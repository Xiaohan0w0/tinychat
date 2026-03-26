// 引入 nodemailer 库（专门用来发邮件的）
const nodemailer = require('nodemailer');
// 引入你自己的配置文件（里面存邮箱账号、授权码）
const config_module = require("./config")

/**
 * 创建发送邮件的代理，发邮件的发送器
 */
let transport = nodemailer.createTransport({
    host: 'smtp.qq.com',
    port: 465,
    secure: true,
    auth: {
        user: config_module.email_user, // 发送方邮箱地址
        pass: config_module.email_pass // 邮箱授权码
    }
});

/**
 * 发送邮件的函数
 * @param {*} mailOptions_ 发送邮件的参数：收件人、标题、内容等
 * @returns Promise 对象
 */
//把 “发邮件” 这件事包装成一个可以等待、可以捕获成功 / 失败的异步任务
function SendMail(mailOptions_){
    return new Promise(function (resolve, reject) {
        //我要返回一个 “可以等待发邮件结果” 的对象
        transport.sendMail(mailOptions_, function(error, info){
            if (error) {
                console.log(error);
                reject(error);
            } else {
                console.log('邮件已成功发送：' + info.response);
                resolve(info.response)
            }
        });
    })

}

module.exports.SendMail = SendMail