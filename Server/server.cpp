#include "server.h"
#include"mytcpserver.h"

#include <qfile.h>

#include "mytcpsocket.h"
#include "operatedb.h"
#include "operatedb.h"
#include "protocol.h"

Server::Server(QWidget *parent)
    : QWidget(parent)
{
    loadConfig();
    // 检查是否已经在监听
        if (!MyTcpServer::getInstance().isListening()) {
            MyTcpServer::getInstance().listen(QHostAddress(m_strIP), m_usPort);
            qDebug() << "服务器开始监听:" << m_strIP << ":" << m_usPort;
        } else {
            qDebug() << "服务器已经在监听";
        }
}

Server &Server::getInstance()
{
    static Server instance;
    return instance;
}

Server::~Server()
{

}

void Server::loadConfig()
{
    QFile file(":/connect.config");
    if(!file.open(QIODevice::ReadOnly))
        {
            qDebug() << "服务器打开文件失败";
            return;
        }

        QByteArray baData = file.readAll();
        QString strData = QString(baData);
        qDebug() << "服务器配置:" << strData;

        QStringList strList = strData.split("\n");
        for(auto& line : strList) {
            line = line.trimmed();
        }

        if(strList.size() >= 2) {
            m_strIP = strList[0];
            m_usPort = strList[1].toUShort();
            m_strRootPath = strList[2];
            qDebug() << "服务器IP:" << m_strIP << "端口:" << m_usPort<<"文件路径:"<<m_strRootPath;
        }
        file.close();
}




