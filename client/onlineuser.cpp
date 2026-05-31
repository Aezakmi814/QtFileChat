#include "client.h"
#include "onlineuser.h"
#include "ui_onlineuser.h"
#include "protocol.h"  // 添加 PDU 头文件
#include <QDebug>
#include <QMessageBox>

OnlineUser::OnlineUser(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OnlineUser)
{
    ui->setupUi(this);
}

OnlineUser::~OnlineUser()
{
    delete ui;
}

void OnlineUser::updateLW(QStringList userList)
{
    ui->listWidget->clear();
    ui->listWidget->addItems(userList);
}

void OnlineUser::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    QString strCurName = Client::getInstance().m_strLoginName;
    QString strTarName = item->text();

    qDebug() << "添加好友请求 - 当前用户:" << strCurName << "目标用户:" << strTarName;
    if (strCurName == strTarName) {
            QMessageBox::warning(this, "提示", "不能添加自己为好友");
            return;
        }
    PDU* pdu = mkPDU();
    pdu->uiType = ENUM_TYPE_ADD_FRIEND_REQUEST;
    memcpy(pdu->caData, strCurName.toStdString().c_str(), 32);
    memcpy(pdu->caData + 32, strTarName.toStdString().c_str(), 32);

    Client::getInstance().sendMsg(pdu);
}
