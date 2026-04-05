#include "CServer.h"
#include "AsioIOServicePool.h"
CServer::CServer(boost::asio::io_context &ioc, unsigned short &port)
    : _ioc(ioc),
    _acceptor(ioc, tcp::endpoint(tcp::v4(), port)),
    _socket(ioc)
{

}

void CServer::Start()
{
    auto self = shared_from_this();
    auto& io_context = AsioIOServicePool::GetInstance()->GetIOService();
    std::shared_ptr<HttpConnection> new_con = std::make_shared<HttpConnection>(io_context);
    
    _acceptor.async_accept(new_con->GetSocket(), [self, new_con](boost::system::error_code ec)
    {
        // 无论成功失败，都继续监听下一个连接
        if (!ec)
        {
            new_con->Start();
        }
        else
        {
            std::cerr << "Accept error: " << ec.message() << std::endl;
        }
        self->Start();
    });
}


