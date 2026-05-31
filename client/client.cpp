#include "client.h"
#include "index.h"
#include "protocol.h"
#include "reshandler.h"
#include "ui_client.h"
#include <QDebug>
#include <QFile>
#include <QHostAddress>
#include<QMessageBox>
#include "friend.h"



Client::Client(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Client)
{
    ui->setupUi(this);//初始化
    loadConfig();
    m_socket.connectToHost(QHostAddress(m_strIP),m_usPort);//连接服务器
    connect(&m_socket,&QTcpSocket::connected,this,&Client::showConnect);
    connect(&m_socket,&QTcpSocket::readyRead,this,&Client::recvMsg);
    m_prh = new ResHandler();
}

Client::~Client()
{
    delete ui;
}



PDU*Client::readMsg()
{
    qDebug()<<"recvMsg接收信息长度"<<m_socket.bytesAvailable();
    uint uiPDULen=0;
    m_socket.read((char*)&uiPDULen,sizeof (uint));//读取协议总长度
    uint uiMsgLen=uiPDULen-sizeof (PDU);//由总长度得柔性数组长度
    PDU *pdu=mkPDU(uiMsgLen);//构建一个pdu
    m_socket.read((char*)pdu+sizeof (uint),uiPDULen-sizeof(uint));//？读取pdu剩余的成员变量，pdu指针要偏移，读取的长度要减去第一个变量的大小
    return pdu;
}

void Client::handleMsg(PDU *pdu)
{
    qDebug()<<"handleMsg pdu->uiTotalLen"<<pdu->uiTotalLen
            <<"pdu->uiMsgLen"<<pdu->uiMsgLen
            <<"pdu->uiType"<<pdu->uiType
            <<"pdu->caData"<<pdu->caData
            <<"pdu->caData+32"<<pdu->caData+32
            <<"pdu->caMsg"<<pdu->caMsg;
    m_prh->pdu=pdu;
    switch(pdu->uiType){
    case ENUM_TYPE_REGIST_RESPOND:{
        m_prh->regist();
        break;
    }
    case ENUM_TYPE_LOGIN_RESPOND:{
        m_prh->login();
        break;
    }
    case ENUM_TYPE_FIND_USER_RESPOND:{
        m_prh->findUser();
        break;
    }
    case ENUM_TYPE_ONLINE_USER_RESPOND:{
        m_prh->onlineUser();
        break;
    }
    case ENUM_TYPE_ADD_FRIEND_RESPOND:{
        m_prh->addFriend();
        break;
    }
    case ENUM_TYPE_ADD_FRIEND_AGREE_RESPOND:{
        m_prh->addFriendAgree();
        break;
    }
    case ENUM_TYPE_ADD_FRIEND_RESEND:{
        m_prh->addFriendResend();
        break;
    }
    case ENUM_TYPE_FLUSH_FRIEND_RESPONDT:{
        m_prh->flusshFriend();
        break;
    }
    case ENUM_TYPE_DEL_USER_RESPOND:{
        m_prh->delFriend();
        break;
    }
    case ENUM_TYPE_CHAT_RESEND:{
        m_prh->chat();
        break;
    }
    case ENUM_TYPE_MKFILE_RESPOND:{
      m_prh->mkdir();
        break;
    }
    case ENUM_TYPE_FLUSH_FILE_RESPOND:{
      m_prh->flushFile();
        break;
    }
    case ENUM_TYPE_DEL_FILE_RESPOND:{
      m_prh->delFile();
        break;
    }
    case ENUM_TYPE_UPLOAD_FILE_INIT_RESPOND:{
      m_prh->uploadFileInit();
        break;
    }
    case ENUM_TYPE_UPLOAD_FILE_DATA_RESPOND:{
      m_prh->uploadFileData();
        break;
    }

    default:
        break;
    }


   }


void Client::loadConfig()
{
    QFile file(":/connect.config");
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "打开文件失败";
        return;
    }
    QByteArray baData = file.readAll();
    QString strData = QString(baData);
    qDebug()<<"strData"<<strData;
    QStringList strList = strData.split("\r\n");
    m_strIP =strList[0];
    m_usPort =strList[1].toUShort();
    if(strList.size() >= 3) {
        m_strRootPath = strList[2].trimmed();
    }
    qDebug()<<"ip"<<m_strIP<<"port"<<m_usPort<<"rootPath"<<m_strRootPath;

    file.close();
}

void Client::showConnect()
{
    qDebug()<<"连接成功";
}
Client &Client::getInstance()
{
    static Client instance;
    return instance;
}

void Client::on_regist_PB_clicked()
{
    QString strName=ui->name_LE->text();//获取用户名和密码
    QString strPwd=ui->pwd_LE->text();
    if(strName.isEmpty()||strPwd.isEmpty()||strName.toStdString().size()>32||strPwd.toStdString().size()>32)
    {
        QMessageBox::critical(this,"注册","用户名或密码非法");//弹窗注册失败
        return;
    }
    PDU* pdu=mkPDU();
    memcpy(pdu->caData,strName.toStdString().c_str(),32);
    memcpy(pdu->caData+32,strPwd.toStdString().c_str(),32);
    pdu->uiType=ENUM_TYPE_REGIST_REQUEST;//注册的请求枚举值
    m_socket.write((char*)pdu,pdu->uiTotalLen);
    qDebug()<<"send msg pdu->uiTotalen"<<pdu->uiTotalLen
              <<"pdu->uiMsgLen"<<pdu->uiMsgLen
             <<"pdu->uiType"<<pdu->uiType
            <<"pdu->caData"<<pdu->caData
              <<"pdu->caData+32"<<pdu->caData+32
           <<"pdu->caMsg"<<pdu->caMsg;
       free(pdu);
       pdu = NULL;


}

void Client::recvMsg()
{
    qDebug()<<"recvMsg 接收消息长度"<<m_socket.bytesAvailable();

    QByteArray data=m_socket.readAll();
    buffer.append(data);
    while(buffer.size()>=int (sizeof (PDU))){
        PDU*pdu=(PDU*)buffer.data();
        if(buffer.size()<int (pdu->uiTotalLen)){
            break;
        }

        handleMsg(pdu);
        buffer.remove(0,pdu->uiTotalLen);
    }
}

void Client::sendMsg(PDU*pdu)
{
    m_socket.write((char*)pdu,pdu->uiTotalLen);
    qDebug()<<"send msg pdu->uiTotalen"<<pdu->uiTotalLen
              <<"pdu->uiMsgLen"<<pdu->uiMsgLen
             <<"pdu->uiType"<<pdu->uiType
            <<"pdu->caData"<<pdu->caData
              <<"pdu->caData+32"<<pdu->caData+32
           <<"pdu->caMsg"<<pdu->caMsg;
       free(pdu);
       pdu = NULL;
}

void Client::on_login_PB_clicked()
{
    QString strName=ui->name_LE->text();//获取用户名和密码
    QString strPwd=ui->pwd_LE->text();
    if(strName.isEmpty()||strPwd.isEmpty()||strName.toStdString().size()>32||strPwd.toStdString().size()>32)
    {
        QMessageBox::critical(this,"登陆","用户名或密码非法");//弹窗登陆失败
        return;
    }
    m_strLoginName= strName;
    PDU* pdu=mkPDU();

    memcpy(pdu->caData,strName.toStdString().c_str(),32);
    memcpy(pdu->caData+32,strPwd.toStdString().c_str(),32);
    pdu->uiType=ENUM_TYPE_LOGIN_REQUEST;//登陆的请求枚举值
    m_socket.write((char*)pdu,pdu->uiTotalLen);
    qDebug()<<"send msg pdu->uiTotalen"<<pdu->uiTotalLen
              <<"pdu->uiMsgLen"<<pdu->uiMsgLen
             <<"pdu->uiType"<<pdu->uiType
            <<"pdu->caData"<<pdu->caData
              <<"pdu->caData+32"<<pdu->caData+32
           <<"pdu->caMsg"<<pdu->caMsg;
       free(pdu);
       pdu = NULL;
}

