#ifndef MSGHANDLER_H
#define MSGHANDLER_H

#include "protocol.h"
#include <QObject>

class MyTcpSocket;  // 前向声明

class MsgHandler : public QObject
{
    Q_OBJECT
public:
    explicit MsgHandler(MyTcpSocket* socket = nullptr);
    PDU* regist();
    PDU* handleLogin(QString&m_strLoginName);
    PDU* findUser();
    PDU* onlineUser();
    PDU* addFriend();
    PDU* addFriendAgree();
    PDU* flushFriend();
    PDU* delFriend();
    PDU* chat();
    PDU* handleMkfile();
    PDU* flushFile();
    PDU* delFile();
    PDU* uploadFileInit();
    PDU* uploadFileData();
    PDU* m_pdu;  // 当前处理的PDU
    PDU*pdu;
    MyTcpSocket* m_socket;  // 所属socket

private:

};

#endif // MSGHANDLER_H
