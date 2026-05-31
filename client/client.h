#ifndef CLIENT_H
#define CLIENT_H

#include "protocol.h"
#include "reshandler.h"
#include <QTcpSocket>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Client; }
QT_END_NAMESPACE

class Client : public QWidget
{
    Q_OBJECT

public:
    static Client& getInstance();
    ~Client();
    QString m_strLoginName;
    QString m_strRootPath;
    QByteArray buffer;


private:
    explicit Client(QWidget *parent = nullptr);  // 构造函数私有化
    Client(const Client &instance) = delete;
    Client& operator=(const Client&) = delete;

    Ui::Client *ui;
    QString m_strIP;
    quint16 m_usPort;
    QTcpSocket m_socket;
    ResHandler *m_prh;
//    QString m_strLoginName;//原生

public slots:
    void showConnect();
    void recvMsg();
    void sendMsg(PDU*pdu);
private slots:
    void on_regist_PB_clicked();
    void on_login_PB_clicked();

public:
    void loadConfig();
//    void sendMsg(PDU *pdu);
    PDU *readMsg();
    void handleMsg(PDU* pdu);
    void on_send_PB_clicked();
};

#endif // CLIENT_H
