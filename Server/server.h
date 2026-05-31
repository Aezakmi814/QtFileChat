#ifndef SERVER_H
#define SERVER_H

#include "protocol.h"

#include <QWidget>
#include <qtcpsocket.h>


class Server : public QWidget
{
    Q_OBJECT

public:
    QString m_strIP;
    quint16 m_usPort;
    QString m_strRootPath;
    static Server&getInstance();
    ~Server();
    void loadConfig();
    Server(QWidget *parent = nullptr);
private:


};
#endif // SERVER_H
