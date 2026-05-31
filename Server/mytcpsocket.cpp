#include "mytcpsocket.h"
#include "protocol.h"
#include "operatedb.h"
#include "mytcpserver.h"
#include <QDebug>
#include <QMessageBox>
#include <qdir.h>
#include "msghandler.h"
#include "server.h"

MyTcpSocket::MyTcpSocket()
    : m_pmh(new MsgHandler(this)), m_uploadFile(nullptr)
{
    connect(this, &QTcpSocket::readyRead, this, &MyTcpSocket::recvMsg);
    connect(this, &QTcpSocket::disconnected, this, &MyTcpSocket::clientOffline);
}

MyTcpSocket::~MyTcpSocket()
{
    if (m_pmh) {
        delete m_pmh;
        m_pmh = NULL;
    }
    if (m_uploadFile) {
        if (m_uploadFile->isOpen()) {
            m_uploadFile->close();
        }
        delete m_uploadFile;
        m_uploadFile = nullptr;
    }
}

PDU* MyTcpSocket::readMsg()
{
    qDebug() << "readMsg: 接收消息长度 =" << this->bytesAvailable();
    uint uiPDULen = 0;
    this->read((char*)&uiPDULen, sizeof(uint));
    uint uiMsgLen = uiPDULen-sizeof (PDU);
    PDU* pdu = mkPDU(uiMsgLen);
    this->read((char*)pdu+sizeof(uint),uiPDULen-sizeof (uint));
    return pdu;
}

PDU* MyTcpSocket::handleMsg(PDU* pdu)
{


    qDebug() << "handleMsg pdu->uiTotalLen" << pdu->uiTotalLen
             << "pdu->uiMsgLen" << pdu->uiMsgLen
             << "pdu->uiType" << pdu->uiType
             <<"pdu->caData"<<pdu->caData
            <<"pdu->caData+32"<<pdu->caData+32
           <<"pdu->caMsg"<<pdu->caMsg;
    PDU* respdu = NULL;  // 响应PDU
    m_pmh->pdu=pdu;
    switch(pdu->uiType) {
    case ENUM_TYPE_REGIST_REQUEST: {
        respdu=m_pmh->regist();
        break;
    }
    case ENUM_TYPE_LOGIN_REQUEST: {
        respdu=m_pmh->handleLogin(m_strLoginName);
        break;
    }
    case ENUM_TYPE_FIND_USER_REQUEST: {
        respdu = m_pmh->findUser();
        break;
    }
    case ENUM_TYPE_ONLINE_USER_REQUEST: {
        respdu=m_pmh->onlineUser();
        break;
    }
    case ENUM_TYPE_ADD_FRIEND_REQUEST:{
        respdu = m_pmh->addFriend();
        break;
    }

    case ENUM_TYPE_ADD_FRIEND_AGREE_REQUEST:{
        respdu = m_pmh->addFriendAgree();
        break;
    }
    case ENUM_TYPE_DEL_USER_REQUEST: {
    respdu=m_pmh->delFriend();
    break;
    }
    case ENUM_TYPE_FLUSH_FRIEND_REQUEST: {
    respdu=m_pmh->flushFriend();
    break;
    }
    case ENUM_TYPE_CHAT_REQUEST: {
    respdu=m_pmh->chat();
    break;
}
    case ENUM_TYPE_MKFILE_REQUEST: {
    respdu=m_pmh->handleMkfile();
    break;
    }
    case ENUM_TYPE_FLUSH_FILE_REQUEST: {
    respdu=m_pmh->flushFile();
    break;
    }
    case ENUM_TYPE_UPLOAD_FILE_INIT_REQUEST: {
    respdu=m_pmh->uploadFileInit();
    break;
    }
    case ENUM_TYPE_UPLOAD_FILE_DATA_REQUEST: {
    respdu=m_pmh->uploadFileData();
    break;
    }
    default:
        break;
    }
    return respdu;
}

void MyTcpSocket::recvMsg()
{
    QByteArray data = this->readAll();
    buffer.append(data);
    while(buffer.size() >= (int)sizeof(PDU)) {
        PDU* pdu = (PDU*)buffer.data();
        if(buffer.size() < (int)pdu->uiTotalLen) {
            break;
        }
        // 拷贝出完整的PDU（buffer.data()会被后续操作覆盖）
        PDU* pduCopy = (PDU*)malloc(pdu->uiTotalLen);
        memcpy(pduCopy, pdu, pdu->uiTotalLen);
        buffer.remove(0, pdu->uiTotalLen);

        qDebug() << "recvMsg: 处理消息类型" << pduCopy->uiType;
        PDU* respdu = handleMsg(pduCopy);
        if (respdu) {
            sendMsg(respdu);
        }
        free(pduCopy);
    }
}

void MyTcpSocket::sendMsg(PDU *pdu)
{
    this->write((char*)pdu, pdu->uiTotalLen);
    qDebug() << "send Msg respdu->uiTotalLen" << pdu->uiTotalLen
             << "pdu->uiMsgLen" << pdu->uiMsgLen
             << "pdu->uiType" << pdu->uiType
             << "pdu->caData" << pdu->caData
             << "pdu->caData+32" << pdu->caData+32
             << "pdu->caMsg" << pdu->caMsg;
    free(pdu);
    pdu = NULL;
}


void MyTcpSocket::clientOffline()
{
    if (!m_strLoginName.isEmpty()) {
        OperateDB::getInstance().handleOffline(m_strLoginName.toStdString().c_str());
    }
    MyTcpServer::getInstance().removeSocket(this);
}

