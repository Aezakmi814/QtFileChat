#include "client.h"
#include "friend.h"
#include "onlineuser.h"
#include "protocol.h"
#include "ui_friend.h"
#include<QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include "chat.h"

Friend::Friend(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Friend)
{
    ui->setupUi(this);
    flushFriend();
    m_pOnlineUser = new OnlineUser;
    m_pChat = new Chat;
}

Friend::~Friend()
{
    delete ui;
    delete m_pOnlineUser;
    delete m_pChat;

}

void Friend::update_LW(QStringList friendList)
{
    ui->listWidget->clear();
    ui->listWidget->addItems(friendList);
}

void Friend::flushFriend()
{
    PDU*pdu=mkPDU();
    pdu->uiType = ENUM_TYPE_FLUSH_FRIEND_REQUEST;
    memcpy(pdu->caData, Client::getInstance().m_strLoginName.toStdString().c_str(), 32);
    Client::getInstance().sendMsg(pdu);

}

void Friend::on_findUser_PB_clicked()
{
    QString strName = QInputDialog::getText(this, "提示", "用户名：");
        if (strName.isEmpty() || strName.toStdString().size() > 32) {
            QMessageBox::critical(this, "提示", "长度非法");
            return;
        }
        PDU* pdu = mkPDU();
        memcpy(pdu->caData, strName.toStdString().c_str(), 32);
        pdu->uiType = ENUM_TYPE_FIND_USER_REQUEST;
        Client::getInstance().sendMsg(pdu);

}

void Friend::on_onlineUser_PB_clicked()
{
    if (m_pOnlineUser->isHidden()) {
           m_pOnlineUser->show();
       }
       PDU* pdu = mkPDU();
       pdu->uiType = ENUM_TYPE_ONLINE_USER_REQUEST;
       Client::getInstance().sendMsg(pdu);

}

void Friend::on_flush_PB_clicked()
{
    flushFriend();
}

void Friend::on_del_PB_clicked()
{
    QListWidgetItem *pItem =ui->listWidget->currentItem();
    if(!pItem)
    {
        return;
    }
     QString strTarName = pItem->text();
    int ret = QMessageBox::question(this,"删除好友",QString("是否确定删除好友%1").arg(strTarName));

    QString strCurName =Client::getInstance().m_strLoginName;
    PDU* pdu = mkPDU(0);
    pdu->uiType = ENUM_TYPE_DEL_USER_REQUEST;
    memcpy(pdu->caData, strCurName.toStdString().c_str(), strCurName.size());
    memcpy(pdu->caData+32, strTarName.toStdString().c_str(), strTarName.size());
    qDebug()<<"删除好友-当前用户"<<Client::getInstance().m_strLoginName
           <<"删除好友-目标用户"<<strTarName;
    Client::getInstance().sendMsg(pdu);
}

void Friend::on_chat_PB_clicked()
{
    QListWidgetItem *pItem =ui->listWidget->currentItem();
    if(!pItem)
    {
        return;
    }
    if(m_pChat->isHidden()){
        m_pChat->show();
    }
    m_pChat->m_strChatName = pItem->text();
}
