#ifndef RESHANDLER_H
#define RESHANDLER_H

#include "protocol.h"



class ResHandler
{
public:
    ResHandler();
    PDU*pdu;
    void regist();
    void login();
    void findUser();
    void onlineUser();
    void addFriend();
    void addFriendResend();
    void addFriendAgree();
    void flusshFriend();
    void delFriend();
    void chat();
    void mkdir();
    void flushFile();
    void delFile();
    void uploadFileInit();
    void uploadFileData();
};

#endif // RESHANDLER_H
