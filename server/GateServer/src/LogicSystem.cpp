#include "LogicSystem.h"
#include "VerifyGrpcClient.h"

bool LogicSystem::HandleGet(std::string url, std::shared_ptr<HttpConnection> connection)
{
    if(_get_handlers.find(url) == _get_handlers.end())
    {
        return false;
    }
    _get_handlers[url](connection);
        return true;
}

bool LogicSystem::HandlePost(std::string url, std::shared_ptr<HttpConnection> connection)
{
    if(_post_handlers.find(url) == _post_handlers.end())
    {
        return false;
    }
    _post_handlers[url](connection);
        return true;
}

void LogicSystem::RegGet(std::string url, HttpHandler handler)
{
    _get_handlers[url] = handler;
}
void LogicSystem::RegPost(std::string url, HttpHandler handler)
{
    _post_handlers[url] = handler;
}

LogicSystem::LogicSystem()
{
    RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
        beast::ostream(connection->_response.body())<<"recceive get_test req"<<std::endl;
        int i = 0;
        for(auto& elem : connection->_get_params)
        {
            i++;
            beast::ostream(connection->_response.body()) << "param " << i << " key is " << elem.first << std::endl;
            beast::ostream(connection->_response.body()) << "param " << i << " value is " << elem.second << std::endl;
        }
        connection->_response.set(http::field::content_type, "text/plain;charset=utf-8");
    });

    RegPost("/get_verifycode", [](std::shared_ptr<HttpConnection> connection)
            {
                auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
                std::cout << "receive body is " << body_str << std::endl;
                connection->_response.set(http::field::content_type, "text/plain;charset=utf-8");
                // 存放 JSON 对象，返回给客户端
                Json::Value root;
                // 字符串转换为 JSON 对象
                Json::Reader reader;
                // 存放 JSON 数据，客户端发送的 JSON 数据
                Json::Value src_root;
                bool parse_success = reader.parse(body_str, src_root);
                if (!parse_success)
                {
                    std::cout << "Failed to parse JSON data!" << std::endl;
                    root["error"] = ErrorCodes::Error_Json;
                    std::string jsonstr = root.toStyledString();
                    beast::ostream(connection->_response.body()) << jsonstr << std::endl;
                    return true;
                }

                if (!src_root.isMember("email"))
                {
                    std::cout << "Failed to parse JSON data!" << std::endl;
                    root["error"] = ErrorCodes::Error_Json;
                    std::string jsonstr = root.toStyledString();
                    beast::ostream(connection->_response.body()) << jsonstr << std::endl;
                    return true;
                }

                auto email = src_root["email"].asString();
                // auto rsp = VerifyGrpcClient::GetInstance()->GetVerifyCode(email);
                std::cout << "email is " << email << std::endl;
                // root["error"] = rsp.error();
                root["email"] = src_root["email"];
                std::string jsonstr = root.toStyledString();
                beast::ostream(connection->_response.body()) << jsonstr << std::endl;
                return true;
            });
}
