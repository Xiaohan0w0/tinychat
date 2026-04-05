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


class RPConPool
{
public:
    RPConPool(size_t poolsize,std::string host,std::string port);
    ~RPConPool();
    void close();
    std::unique_ptr<VerifyService::Stub> getConnection();
    void returnConnection(std::unique_ptr<VerifyService::Stub> context);
private:
    std::atomic<bool> b_stop_;
    size_t poolSize_;
    std::string host_;
    std::string port_;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::queue<std::unique_ptr<VerifyService::Stub>> connections_;
};

class VerifyGrpcClient : public Singleton<VerifyGrpcClient>
{
    friend class Singleton<VerifyGrpcClient>;
public:
    GetVerifyRsp GetVerifyCode(const std::string &email);
    ~VerifyGrpcClient() = default;

private:
    // stub客户端存根，封装了 RPC 调用能力，用于调用服务端的方法，传递消息，依赖channel提供网络连接
    // 好比channel是电话线路，stub是电话听筒
    VerifyGrpcClient();
    std::unique_ptr<RPConPool> pool_;
};