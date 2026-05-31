#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include <QFile>
#include "protocol.h"

class MsgHandler;  // 前向声明

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    MyTcpSocket();
    ~MyTcpSocket();
     QString m_strLoginName;  // 登录用户名
    QString getLoginName() const { return m_strLoginName; }
    void setLoginName(const QString& name) { m_strLoginName = name; }
    QByteArray buffer;
    PDU* readMsg();          // 读取一个完整的PDU
    void sendMsg(PDU *pdu);
    PDU* handleMsg(PDU* pdu);
    MsgHandler* m_pmh;
    void recvMsg();
    void clientOffline();
    // 上传文件跟踪
    QFile* m_uploadFile;
    QString m_uploadFilePath;
private slots:

private:

};

#endif // MYTCPSOCKET_H
