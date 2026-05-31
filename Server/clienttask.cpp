#include "clienttask.h"
#include "mytcpsocket.h"

ClientTask::ClientTask(MyTcpSocket *socket)
{
    m_socket=socket;
}

void ClientTask::run()
{
    // MyTcpSocket 构造函数已连接 readyRead/disconnected 信号
    // 无需重复连接
}
