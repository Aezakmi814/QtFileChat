#include "client.h"
#include "index.h"
#include "reshandler.h"
#include <QMessageBox>
#include <QDebug>

ResHandler::ResHandler()
    : pdu(nullptr)
{
}

void ResHandler::login()
{
    if (pdu == nullptr) {
        qDebug() << "ResHandler::login: pdu is null!";
        return;
    }

    bool ret;
    memcpy(&ret, pdu->caData, sizeof(bool));
    qDebug() << "login ret" << ret;

    if(ret){
        Index& index = Index::getInstance();  //创建实例
        index.show();
        Client::getInstance().hide();

    } else {
        QMessageBox::information(&Client::getInstance(), "提示", "登陆失败");
    }
}

void ResHandler::regist() {
    bool ret;
    memcpy(&ret, pdu->caData, sizeof(bool));
    qDebug() << "注册ret" << ret;

    if(ret){
        QMessageBox::information(&Client::getInstance(), "提示", "注册成功");
    } else {
        QMessageBox::information(&Client::getInstance(), "提示", "注册失败");
    }

}

void ResHandler::findUser() {
    int ret;
    memcpy(&ret, pdu->caData, sizeof(int));
    qDebug() << "find user ret" << ret;

    if (ret == 0) {
        QMessageBox::information(&Index::getInstance(), "提示", "该用户不在线");
    } else if (ret == 1) {
        QMessageBox::information(&Index::getInstance(), "提示", "该用户在线");
    } else if (ret == 2) {
        QMessageBox::information(&Index::getInstance(), "提示", "该用户不存在");
    } else if (ret == -1) {
        QMessageBox::information(&Index::getInstance(), "提示", "查找失败");
    }

}

void ResHandler::onlineUser() {
    uint uiSize = pdu->uiMsgLen/32;
    char caTmp[32] = {'\0'};
    QStringList userList;
    for (uint i=0; i<uiSize; i++) {
        memcpy(caTmp, pdu->caMsg+i*32, 32);
        userList.append(caTmp);
    }
    Index::getInstance().getFriend()->m_pOnlineUser->updateLW(userList);

}

void ResHandler::addFriend()
{
    if (!pdu) return;

    int ret;
    memcpy(&ret, pdu->caData, sizeof(int));
    qDebug() << "addFriend ret:" << ret;

    if (ret == 0) {
        QMessageBox::information(&Index::getInstance(), "提示", "该用户不在线");
    } else if (ret == -2) {
        QMessageBox::information(&Index::getInstance(), "提示", "该用户已经是好友");
    } else if (ret == -1) {
        QMessageBox::information(&Index::getInstance(), "提示", "服务器错误：联系开发人员");
    } else if (ret == 2) {
        QMessageBox::information(&Index::getInstance(), "提示", "好友请求已发送，等待对方同意");
    }
}

void ResHandler::addFriendResend()
{
    if (!pdu) return;

       char caName[32] = {'\0'};
       memcpy(caName, pdu->caData, 32);

       QString userName = QString(caName);
       int ret = QMessageBox::question(&Index::getInstance(),
                                      "添加好友",
                                      QString("是否同意 %1 的添加好友请求？").arg(caName),
                                      QMessageBox::Yes | QMessageBox::No);

       if (ret == QMessageBox::Yes) {

           // 同意请求，发送同意消息
           PDU* respdu = mkPDU();
           QString currentUser = Client::getInstance().m_strLoginName;

           strncpy(respdu->caData, currentUser.toStdString().c_str(), 32);
           strncpy(respdu->caData + 32, caName, 32);
           respdu->uiType = ENUM_TYPE_ADD_FRIEND_AGREE_REQUEST;

           // 获取Client单例并发送
           Client::getInstance().sendMsg(respdu);
       }
       if(ret != QMessageBox::Yes){
           return;
       }
}

void ResHandler::addFriendAgree()
{
    if (!pdu) return;
    bool ret;
    memcpy(&ret, pdu->caData, sizeof(bool));
    qDebug() << "addFriendAgree ret:" << ret;
    if (ret) {
        QMessageBox::information(&Index::getInstance(), "提示", "添加好友成功");
    } else {
        QMessageBox::information(&Index::getInstance(), "提示", "添加好友失败");
    }
}

void ResHandler::flusshFriend()
{
    QStringList friendList;
    int iSize =pdu->uiMsgLen/32;
    char caTmp[32]={'\0'};
    for(int i=0; i<iSize;i++)
    {
        memcpy(caTmp,pdu->caMsg+i*32,32);
        friendList.append(caTmp);
    }
    Index::getInstance().getFriend()->update_LW(friendList);
}

void ResHandler::delFriend()
{
    bool ret;
    memcpy(&ret,pdu->caData,sizeof (bool));
    qDebug()<<"delFriend ret"<<ret;
    if(ret){
        Index::getInstance().getFriend()->flushFriend();

    }else {
    QMessageBox::information(&Index::getInstance(),"提示","删除好友失败");
}
}

void ResHandler::chat()
{
    Chat*c = Index::getInstance().getFriend()->m_pChat;
    if(c->isHidden()){
        c->show();
    }
    char caChatName[32] = {'\0'};
    memcpy(caChatName,pdu->caData,32);
    c->updateShow_TE(QString("%1:%2").arg(caChatName).arg(pdu->caMsg));
    c->m_strChatName = caChatName;
}

void ResHandler::mkdir()
{
    bool ret;
    memcpy(&ret,pdu->caData,sizeof (bool));
    qDebug()<<"mkdir ret"<<ret;
    if(ret)
    {
        QMessageBox::information(&Index::getInstance(),"提示","创建文件夹成功");

    }else{
        QMessageBox::information(&Index::getInstance(),"提示","创建文件夹失败");

    }
}

void ResHandler::flushFile()
{
    int iCount=pdu->uiMsgLen/sizeof (FileInfo);
    QList<FileInfo*>pFileInfoList;
    for(int i=0;i<iCount;i++ ){
        FileInfo*pFileInfo = new FileInfo;
        memcpy(pFileInfo,pdu->caMsg+i*sizeof (FileInfo),sizeof (FileInfo));
        pFileInfoList.append(pFileInfo);

    }
    Index::getInstance().getFile()->updateFileList(pFileInfoList);
}

void ResHandler::delFile()
{
    bool ret;
       // 从PDU的caData中提取删除结果
       memcpy(&ret, pdu->caData, sizeof(bool));

       qDebug() << "delFile ret" << ret; // 调试输出

       if (ret) {
           // 删除成功，刷新文件列表
           Index::getInstance().getFile()->flushFile();
       } else {
           // 删除失败，弹出提示框
           QMessageBox::information(&Index::getInstance(), "提示", "删除文件失败");
       }
}

void ResHandler::uploadFileInit()
{
    if (!pdu) return;
    bool ret;
    memcpy(&ret, pdu->caData, sizeof(bool));
    qDebug() << "uploadFileInit ret" << ret;
    if (ret) {
        Index::getInstance().getFile()->uploadFile();
    } else {
        QMessageBox::information(&Index::getInstance(), "提示", "文件上传初始化失败");
    }
}

void ResHandler::uploadFileData()
{
    if (!pdu) return;
    bool ret;
    memcpy(&ret, pdu->caData, sizeof(bool));
    qDebug() << "uploadFileData ret" << ret;
    if (!ret) {
        QMessageBox::information(&Index::getInstance(), "提示", "文件数据上传失败");
    }
}
