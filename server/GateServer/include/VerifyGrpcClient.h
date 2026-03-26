#pragma once
#include "Const.h"
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "Singleton.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using message::GetVerifyReq;
using message::GetVerifyRsp;
using message::VerifyService;

class VerifyGrpcClient : public Singleton<VerifyGrpcClient>
{
    friend class Singleton<VerifyGrpcClient>;
public:
    GetVerifyRsp GetVerifyCode(const std::string& email)
    {
        ClientContext context;
        GetVerifyReq request;
        GetVerifyRsp response;
        request.set_email(email);

        Status status = stub_->GetVerifyCode(&context, request, &response);
        if (status.ok())
        {
            return response;
        }
        else
        {
            response.set_error(ErrorCodes::RPCFailed);
            return response;
        }
    }
private:
    VerifyGrpcClient()
    {
        // 不使用TLS，采用明文传输，连接 gRPC 服务器地址 0.0.0.0:50051
        // channel 管理与服务端的连接，包括连接的创建、维护和关闭等
        // 一个channel可以创建多个stub，每个stub可以并行调用服务端的方法，但是每个stub只能调用一个服务
        std::shared_ptr<Channel> channel=grpc::CreateChannel("0.0.0.0:50051",grpc::InsecureChannelCredentials());
        stub_=VerifyService::NewStub(channel);
    }
    ~VerifyGrpcClient()=default;
    // stub客户端存根，封装了 RPC 调用能力，用于调用服务端的方法，传递消息，依赖channel提供网络连接
    // 好比channel是电话线路，stub是电话听筒
    std::unique_ptr<VerifyService::Stub> stub_;
};