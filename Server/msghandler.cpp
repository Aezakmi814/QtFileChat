#include "msghandler.h"
#include "mytcpserver.h"
#include "mytcpsocket.h"
#include "operatedb.h"
#include <QDir>
#include <QFile>
#include<string.h>
#include "server.h"

MsgHandler::MsgHandler(MyTcpSocket* socket)
    : m_socket(socket)
{

}


PDU *MsgHandler::regist()
{
    char caName[32]={'\0'};
    memcpy(caName,pdu->caData,32);
    char caPwd[32]={'\0'};
    memcpy(caPwd,pdu->caData+32,32);
    qDebug()<<"regist cName"<<caName<<"caPwd"<<caPwd;
    bool ret = OperateDB::getInstance().handleRegist(caName,caPwd);
    qDebug()<<"regist ret"<<ret;
    if(ret){
        QDir dir;
        bool res=
                dir.mkdir(QString("%1/%2").arg(Server::getInstance().m_strRootPath).arg(caName));
        qDebug()<<"创建用户文件夹res"<<res;

    }
    PDU*respdu=mkPDU();
    memcpy(respdu->caData,&ret,sizeof(bool));
    respdu->uiType=ENUM_TYPE_REGIST_RESPOND;
    return respdu;
}

PDU *MsgHandler::handleLogin( QString &m_strLoginName)
{
    char caName[32] = {'\0'};
    memcpy(caName,pdu->caData,32);
    char caPwd[32] = {'\0'};
    memcpy(caPwd,pdu->caData+32,32);
    qDebug()<<"login caName"<<caName<<"caPwd"<<caPwd;
    bool ret = OperateDB::getInstance().handleLogin(caName,caPwd);
    qDebug()<<"login ret"<<ret;
    if(ret){
        m_strLoginName = caName;
    }
    PDU*respdu=mkPDU();
    memcpy(respdu->caData,&ret,sizeof (bool));
    respdu->uiType = ENUM_TYPE_LOGIN_RESPOND;
    return respdu;
}

PDU *MsgHandler::findUser()
{
    char caName[32] = {'\0'};
    memcpy(caName,pdu->caData,32);
    qDebug()<<"find user caName"<<caName;
    int ret = OperateDB::getInstance().handleFindUser(caName);
    qDebug()<<"find user ret"<<ret;
    PDU*respdu=mkPDU();
    memcpy(respdu->caData,&ret,sizeof (int));
    respdu->uiType=ENUM_TYPE_FIND_USER_RESPOND;
    return respdu;
}

PDU *MsgHandler::onlineUser()
{
    QStringList res = OperateDB::getInstance().handleOnlineUser();
    PDU*respdu=mkPDU(res.size()*32);
    respdu->uiType=ENUM_TYPE_ONLINE_USER_RESPOND;
    for(int i=0;i<res.size();i++){
        memcpy(respdu->caMsg+i*32,res[i].toStdString().c_str(),32);

    }
    return respdu;
}



PDU* MsgHandler::addFriend()
{
    char caCurName[32] = {'\0'};
    char caTarName[32] = {'\0'};
    memcpy(caCurName, pdu->caData, 32);
    memcpy(caTarName, pdu->caData + 32, 32);

    qDebug() << "addFriend: 请求方:" << caCurName << "目标:" << caTarName;

    int ret = OperateDB::getInstance().handleADDFriend(caCurName, caTarName);
    qDebug() << "数据库返回:" << ret;

    if(ret == 1) {
        // 创建新的PDU进行转发（不要修改原pdu！）
        PDU* forwardPdu = mkPDU(0);
        memcpy(forwardPdu->caData, caCurName, 32);  // 只放请求方的名字
        forwardPdu->uiType = ENUM_TYPE_ADD_FRIEND_RESEND;

        qDebug() << "转发好友请求给:" << caTarName;

        // 转发给目标用户
        MyTcpServer::getInstance().resend(caTarName, forwardPdu);

        // 返回响应给请求方
        PDU* respdu = mkPDU(0);
        int status = 2;  // 请求已发送
        memcpy(respdu->caData, &status, sizeof(int));
        respdu->uiType = ENUM_TYPE_ADD_FRIEND_RESPOND;
        return respdu;
    }
    else {
        // 直接返回结果
        PDU* respdu = mkPDU(0);
        memcpy(respdu->caData, &ret, sizeof(int));
        respdu->uiType = ENUM_TYPE_ADD_FRIEND_RESPOND;
        return respdu;
    }
}

PDU* MsgHandler::addFriendAgree()
{

    char caCurName[32] = {'\0'};
    char caTarName[32] = {'\0'};
    memcpy(caCurName,pdu->caData,32);
    memcpy(caTarName,pdu->caData+32,32);
    qDebug() << "处理同意添加好友 - 同意者:" << caCurName << "发起者:" << caTarName;
    bool ret = OperateDB::getInstance().handleAddFriendAgree(caCurName, caTarName);
    qDebug() << "添加好友关系结果:" << ret;
    PDU* respdu = mkPDU();
    respdu->uiType = ENUM_TYPE_ADD_FRIEND_AGREE_RESPOND;
    memcpy(respdu->caData, &ret, sizeof(bool));
    // 创建独立PDU转发给发起者（resend会free掉pdu）
    PDU* forwardPdu = mkPDU();
    memcpy(forwardPdu, respdu, sizeof(PDU));
    MyTcpServer::getInstance().resend(caTarName, forwardPdu);
    // 返回响应给同意者
    return respdu;
}

PDU *MsgHandler::flushFriend()
{
    QStringList res =
               OperateDB::getInstance().handleFlushFriend(pdu->caData);
       PDU* respdu = mkPDU(res.size()*32);
       respdu->uiType =ENUM_TYPE_FLUSH_FRIEND_RESPONDT;
       for(int i= 0;i<res.size();i++)
       {
           memcpy(respdu->caMsg+i*32,res[i].toStdString().c_str(),32);
       }
       return respdu;
}

PDU *MsgHandler::delFriend()
{
    char curName[32] = {'\0'};
    char tarName[32] = {'\0'};
    memcpy(curName,pdu->caData,32);
    memcpy(tarName,pdu->caData+32,32);
    bool ret = OperateDB::getInstance().DelFriend(curName,tarName);
    qDebug()<<"delFriend ret:"<<ret;
    PDU* respdu = mkPDU(0);
    respdu->uiType = ENUM_TYPE_DEL_USER_RESPOND;
    memcpy(respdu->caData, &ret, sizeof(bool));
    return respdu;
}

PDU *MsgHandler::chat()
{
    if (!pdu) return nullptr;
    uint msgLen = pdu->uiMsgLen;
    PDU* forwardPdu = mkPDU(msgLen);
    memcpy(forwardPdu, pdu, sizeof(PDU) + msgLen);
    forwardPdu->uiType = ENUM_TYPE_CHAT_RESEND;
    char tarName[32] = {'\0'};
    memcpy(tarName, forwardPdu->caData + 32, 32);
    MyTcpServer::getInstance().resend(tarName, forwardPdu);
    return NULL;
}


PDU *MsgHandler::handleMkfile()
{
   QString strPath=QString("%1/%2").arg(pdu->caMsg).arg(pdu->caData);
    qDebug()<<"mkdir strPath"<<strPath;
    QDir dir;
    bool ret= dir.mkdir(strPath);
    qDebug()<<"创建文件夹 ret"<<ret;
    PDU*respdu=mkPDU();
    respdu->uiType=ENUM_TYPE_MKFILE_RESPOND;
    memcpy(respdu->caData,&ret,sizeof (bool));
    return respdu;
}

PDU *MsgHandler::flushFile()
{
    QDir dir(pdu->caMsg);
    QFileInfoList fileInfoList =dir.entryInfoList();

    PDU*respdu = mkPDU(fileInfoList.size()*sizeof (FileInfo));
    respdu->uiType = ENUM_TYPE_FLUSH_FILE_RESPOND;
    for(int i=0; i<fileInfoList.size();i++){
        FileInfo *pFileInfo = (FileInfo*)respdu->caMsg+i;
        if(fileInfoList[i].isDir()){
            pFileInfo->iFileType = 0;
        }else
        {
            pFileInfo->iFileType = 1;
        }
        memcpy(pFileInfo->caName,fileInfoList[i].fileName().toStdString().c_str(),32);
        qDebug()<<"pFileInfo->caName"<<pFileInfo->caName;
    }
    return respdu;
}

PDU *MsgHandler::delFile()
{
    // 从PDU中获取文件路径并创建QFileInfo对象
        QFileInfo fileInfo(pdu->caMsg);
        int ret;

        // 判断是目录还是文件，分别使用不同的删除方法
        if (fileInfo.isDir()) {
            QDir dir(pdu->caMsg);
            ret = dir.removeRecursively(); // 递归删除目录
        } else {
            QFile file(pdu->caMsg);
            ret = file.remove(); // 删除文件
        }

        qDebug() << "delFile ret" << ret; // 调试输出删除结果

        // 创建响应PDU
        PDU* respdu = mkPDU();
        respdu->uiType = ENUM_TYPE_DEL_FILE_RESPOND;

        // 将删除结果拷贝到PDU的caData中
        memcpy(respdu->caData, &ret, sizeof(bool));

        return respdu; // 返回响应PDU
}

PDU* MsgHandler::uploadFileInit()
{
    // 提取文件名
    char caFileName[32] = {'\0'};
    memcpy(caFileName, pdu->caData, 32);
    // 提取文件大小（实际仅用于日志）
    qint64 iFileSize = 0;
    memcpy(&iFileSize, pdu->caData + 32, sizeof(qint64));
    // 提取保存路径
    QString strPath = QString("%1/%2").arg(pdu->caMsg).arg(caFileName);

    qDebug() << "uploadFileInit: 文件名" << caFileName << "大小" << iFileSize << "路径" << strPath;

    bool ret = false;
    if (m_socket) {
        // 关闭之前的未完成上传
        if (m_socket->m_uploadFile) {
            if (m_socket->m_uploadFile->isOpen())
                m_socket->m_uploadFile->close();
            delete m_socket->m_uploadFile;
        }
        m_socket->m_uploadFile = new QFile(strPath);
        m_socket->m_uploadFilePath = strPath;
        if (m_socket->m_uploadFile->open(QIODevice::WriteOnly)) {
            ret = true;
        } else {
            qDebug() << "上传文件创建失败:" << strPath;
        }
    }

    PDU* respdu = mkPDU();
    respdu->uiType = ENUM_TYPE_UPLOAD_FILE_INIT_RESPOND;
    memcpy(respdu->caData, &ret, sizeof(bool));
    return respdu;
}

PDU* MsgHandler::uploadFileData()
{
    bool ret = false;
    if (m_socket && m_socket->m_uploadFile && m_socket->m_uploadFile->isOpen()) {
        qint64 written = m_socket->m_uploadFile->write(pdu->caMsg, pdu->uiMsgLen);
        if (written == pdu->uiMsgLen) {
            ret = true;
        }
        // 数据长度小于PDU缓冲区(4096)说明是最后一块，关闭文件
        if (pdu->uiMsgLen < 4096) {
            m_socket->m_uploadFile->close();
            qDebug() << "上传完成:" << m_socket->m_uploadFilePath;
        }
    }

    PDU* respdu = mkPDU();
    respdu->uiType = ENUM_TYPE_UPLOAD_FILE_DATA_RESPOND;
    memcpy(respdu->caData, &ret, sizeof(bool));
    return respdu;
}
