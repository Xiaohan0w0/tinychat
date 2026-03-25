#ifndef GLOBAL_H
#define GLOBAL_H
#include <QByteArray>
#include <QDir>
#include <QJsonObject>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QSettings>
#include <QString>
#include <QWidget>
#include "QStyle"
#include <iostream>
#include <memory.h>
#include <mutex>

// repolish 用来刷新qss
extern std::function<void(QWidget *)> repolish;

enum ReqId {
    ID_GET_VERIFY_CODE = 1001, // 获取验证码
    ID_REG_USER = 1002,        // 注册模块
};

enum Modules {
    REGISTERMOD = 0,
};

enum ErrorCodes {
    SUCCESS = 0,
    ERR_JSON = 1,    // JSON解析失败
    ERR_NETWORK = 2, // 网络错误
};

extern QString gate_url_prefix;

#endif // GLOBAL_H
