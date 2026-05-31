#include "clienttask.h"
#include "mytcpserver.h"
#include "mytcpsocket.h"
#include <QDebug>
#include <string.h>

MyTcpServer* MyTcpServer::instance = nullptr;

MyTcpServer& MyTcpServer::getInstance()
{
    if (instance == nullptr) {
        instance = new MyTcpServer();
    }
    return *instance;
}

MyTcpServer::MyTcpServer(QObject *parent)
    : QTcpServer(parent)
{
    QThreadPool::globalInstance()->setMaxThreadCount(8);
}

MyTcpServer::~MyTcpServer()
{
    // 清理所有socket
    for (MyTcpSocket* socket : m_tcpSocketList) {
        if (socket) {
            socket->close();
            delete socket;
        }
    }
    m_tcpSocketList.clear();
}

void MyTcpServer::incomingConnection(qintptr handle)
{
    qDebug() << "新客户端连接";
    MyTcpSocket* pTcpSocket = new MyTcpSocket();
    pTcpSocket->setSocketDescriptor(handle);

    m_tcpSocketList.append(pTcpSocket);
    ClientTask*task = new ClientTask(pTcpSocket);
    QThreadPool::globalInstance()->start(task);
}

void MyTcpServer::resend(char* caTarName, PDU* pdu)
{
    if (caTarName == NULL || pdu == NULL) {
        if (pdu) free(pdu);
        return;
    }
    QString targetName = QString::fromUtf8(caTarName).trimmed();
    bool found = false;
    for (int i = 0; i < m_tcpSocketList.size(); i++) {
        MyTcpSocket* socket = m_tcpSocketList[i];
        if (!socket) continue;

        QString loginName = socket->m_strLoginName.trimmed();

        if (loginName == targetName && socket->state() == QAbstractSocket::ConnectedState) {
            socket->write((char*)pdu, pdu->uiTotalLen);
            socket->flush();
            free(pdu);
            pdu = nullptr;
            found = true;
            break;
        }
    }
    if (!found && pdu) {
        free(pdu);
        pdu = nullptr;
    }
}

void MyTcpServer::removeSocket(MyTcpSocket* socket)
{
    if (socket == NULL) {
        return;
    }

    // 从列表中移除
    m_tcpSocketList.removeOne(socket);

    // 释放内存
    socket->close();
    delete socket;

    qDebug() << "移除socket，当前连接数:" << m_tcpSocketList.size();
}
