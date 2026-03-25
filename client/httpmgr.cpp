#include "httpmgr.h"

HttpMgr::HttpMgr()
{
    connect(this, &HttpMgr::sig_http_finish, this, &HttpMgr::slot_http_finish);
}

void HttpMgr::PostHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod)
{
    // 将内存中的 JSON 对象转换为可传输的字节流（字符串格式）
    QByteArray data = QJsonDocument(json).toJson();
    // 构建请求对象
    QNetworkRequest request(url);
    // 设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(data.length()));

    // 发起异步POST请求，捕获智能指针（保证回调时对象存活）
    // 通过网络管理器发送请求，立即返回 reply 对象
    auto self = shared_from_this();
    QNetworkReply *reply = _manager.post(request, data);

    // 注册回调函数，等待网络请求完成后被触发
    QObject::connect(reply, &QNetworkReply::finished, [self, reply, req_id, mod]() {
        // 处理错误情况
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << reply->errorString();
            // 发送失败信号
            emit self->sig_http_finish(req_id, "", ErrorCodes::ERR_NETWORK, mod);
            // 释放资源
            reply->deleteLater();
            return;
        }

        // 无错误，读取数据
        QString res = reply->readAll();
        // 发送成功信号，通知完成
        emit self->sig_http_finish(req_id, res, ErrorCodes::SUCCESS, mod);
        // 释放资源
        reply->deleteLater();
    });
}

void HttpMgr::slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod)
{
    if (mod == Modules::REGISTERMOD) {
        // 发送信号通知指定模块 http 的响应结束了
        emit sig_reg_mod_finish(id, res, err);
    }
}

HttpMgr::~HttpMgr() {}