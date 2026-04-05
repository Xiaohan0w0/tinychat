#include "VerifyGrpcClient.h"
#include "ConfigMgr.h"

VerifyGrpcClient::VerifyGrpcClient()
{
    auto &gCfgMgr = ConfigMgr::GetInstance();
    std::string host = gCfgMgr["VerifyServer"]["Host"];
    std::string port = gCfgMgr["VerifyServer"]["Port"];
    pool_.reset(new RPConPool(5, host, port));
}

GetVerifyRsp VerifyGrpcClient::GetVerifyCode(const std::string &email)
{
    ClientContext context;
    GetVerifyReq request;
    GetVerifyRsp response;
    request.set_email(email);
    auto stub=pool_->getConnection();
    Status status = stub->GetVerifyCode(&context, request, &response);
    if (status.ok())
    {
        pool_->returnConnection(std::move(stub));
        return response;
    }
    else
    {
        pool_->returnConnection(std::move(stub));
        response.set_error(ErrorCodes::RPCFailed);
        return response;
    }
}

RPConPool::RPConPool(size_t poolsize, std::string host, std::string port)
:poolSize_(poolsize)
,host_(host)
,port_(port)
,b_stop_(false)
{
    // 不使用TLS，采用明文传输，连接 gRPC 服务器地址 0.0.0.0:50051
    // channel 管理与服务端的连接，包括连接的创建、维护和关闭等
    // 一个channel可以创建多个stub，每个stub可以并行调用服务端的方法，但是每个stub只能调用一个服务
    for (size_t i = 0; i < poolSize_;i++)
    {
        std::shared_ptr<Channel> channel = grpc::CreateChannel(host_+":"+port_, grpc::InsecureChannelCredentials());
        connections_.push(VerifyService::NewStub(channel));
    }
}

RPConPool::~RPConPool()
{
    std::lock_guard<std::mutex> lock(mutex_);
    close();
    while (!connections_.empty())
    {
        connections_.pop();
    } 
}

void RPConPool::close()
{
    b_stop_ = true;
    cond_.notify_all();
}

std::unique_ptr<VerifyService::Stub> RPConPool::getConnection()
{
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this]() {
        if (b_stop_)
            return true;
        return !connections_.empty();
    });
    if (b_stop_)
        return nullptr;
    auto context = std::move(connections_.front());
    connections_.pop();
    return context;
}

void RPConPool::returnConnection(std::unique_ptr<VerifyService::Stub> context)
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    connections_.push(std::move(context));
    cond_.notify_one();
}
