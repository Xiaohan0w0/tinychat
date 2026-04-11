#include "registerdialog.h"
#include "global.h"
#include "httpmgr.h"
#include "ui_registerdialog.h"

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
    ui->pass_edit->setEchoMode(QLineEdit::Password);
    ui->confirm_edit->setEchoMode(QLineEdit::Password);
    ui->err_tip->setProperty("state", "normal");
    repolish(ui->err_tip);
    connect(HttpMgr::GetInstance().get(),
            &HttpMgr::sig_reg_mod_finish,
            this,
            &RegisterDialog::slot_reg_mod_finish);
    initHttpHandlers();
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::on_get_code_clicked()
{
    auto email = ui->email_edit->text();
    qDebug() << "邮件地址:" << email;
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool match = regex.match(email).hasMatch();
    if (match) {
        // 发送http验证码
        QJsonObject json_obj;
        json_obj["email"] = email;
        qDebug() << gate_url_prefix;
        HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix + "/get_verifycode"),
                                            json_obj,
                                            ReqId::ID_GET_VERIFY_CODE,
                                            Modules::REGISTERMOD);
    } else {
        showTip(tr("邮箱地址错误"), false);
    }
}

void RegisterDialog::slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if (err != ErrorCodes::SUCCESS) {
        showTip(tr("网络请求错误"), false);
        return;
    }

    // 解析并格式化输出
    QJsonDocument doc = QJsonDocument::fromJson(res.toUtf8());
    if (!doc.isNull()) {
        qDebug().noquote() << "实际收到的响应内容:\n" << doc.toJson(QJsonDocument::Indented);
    }

    // 解析JSON 字符串，res 转换为QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    if (jsonDoc.isNull()) {
        showTip(tr("json解析错误"), false);
        return;
    }
    // json 解析错误
    if (!jsonDoc.isObject()) {
        showTip(tr("json解析错误"), false);
        return;
    }

    _handlers[id](jsonDoc.object());
    return;
}

void RegisterDialog::initHttpHandlers()
{
    // 注册获取验证码回包的逻辑
    _handlers.insert(ReqId::ID_GET_VERIFY_CODE, [this](const QJsonObject &jsonObj) {
        int error = jsonObj["error"].toInt();
        if (error != ErrorCodes::SUCCESS) {
            showTip(tr("参数错误"), false);
            return;
        }
        auto email = jsonObj["email"].toString();
        showTip(tr("验证码已发送至邮箱，请注意查收"), true);
        qDebug() << "邮箱地址:" << email;
    });

    // 注册注册用户回包逻辑
    _handlers.insert(ReqId::ID_REG_USER, [this](QJsonObject jsonObj) {
        int error = jsonObj["error"].toInt();
        if (error != ErrorCodes::SUCCESS) {
            showTip(tr("参数错误"), false);
        }
        auto email = jsonObj["email"].toString();
        showTip(tr("用户注册成功"), true);
        qDebug() << "邮箱地址:" << email;
    });
}

void RegisterDialog::showTip(QString str, bool b_ok)
{
    if (b_ok) {
        ui->err_tip->setProperty("state", "normal");
    } else {
        ui->err_tip->setProperty("state", "err");
    }
    ui->err_tip->setText(str);

    repolish(ui->err_tip);
}

void RegisterDialog::on_sure_btn_clicked()
{
    if (ui->user_edit->text() == "") {
        showTip(tr("用户名不能为空"), false);
        return;
    }
    if (ui->email_edit->text() == "") {
        showTip(tr("邮箱不能为空"), false);
        return;
    }
    if (ui->pass_edit->text() == "") {
        showTip(tr("密码不能为空"), false);
        return;
    }
    if (ui->confirm_edit->text() == "") {
        showTip(tr("确认密码不能为空"), false);
        return;
    }
    if (ui->confirm_edit->text() != ui->pass_edit->text()) {
        showTip(tr("密码和确认密码不匹配"), false);
        return;
    }
    if (ui->verify_edit->text() == "") {
        showTip(tr("验证码不能为空"), false);
        return;
    }

    QJsonObject json_obj;
    json_obj["user"] = ui->user_edit->text();
    json_obj["email"] = ui->email_edit->text();
    json_obj["passwd"] = ui->pass_edit->text();
    json_obj["confirm"] = ui->confirm_edit->text();
    json_obj["verifycode"] = ui->verify_edit->text();
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix + "/user_register"),
                                        json_obj,
                                        ReqId::ID_REG_USER,
                                        Modules::REGISTERMOD);
}
