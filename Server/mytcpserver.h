#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include <QList>
#include <QThreadPool>
#include "protocol.h"

class MyTcpSocket;

class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    static MyTcpServer& getInstance();
    void removeSocket(MyTcpSocket* socket);
    void incomingConnection(qintptr handle) override;
    void resend(char *caTarName, PDU *pdu);
    void resend(const char *caTarName, PDU *pdu);

private:
    explicit MyTcpServer(QObject *parent = nullptr);
    ~MyTcpServer();
    MyTcpServer(const MyTcpServer&) = delete;
    MyTcpServer& operator=(const MyTcpServer&) = delete;
    QList<MyTcpSocket*> m_tcpSocketList;
    static MyTcpServer* instance;
};

#endif // MYTCPSERVER_H
