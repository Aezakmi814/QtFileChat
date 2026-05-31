#ifndef OPERATEDB_H
#define OPERATEDB_H


#include <QObject>
#include<QSqlDatabase>


class OperateDB : public QObject
{
    Q_OBJECT
public:
    static OperateDB& getInstance();
    void connectSQL();
    bool handleRegist(const char *caName,const char*caPwd);
    ~OperateDB();

    bool handleLogin(const char *caName, const char *caPwd);
    void handleOffline(const char *caName);
    int handleFindUser(const char *caName);
    QStringList handleOnlineUser();
    int handleADDFriend(const char *caCurName, const char *caTarName);
    bool handleAddFriendAgree(const char *caCurName, const char *caTarName);
    int handleAddFriend(const char *caCurName, const char *caTarName);
    QStringList handleFlushFriend(const char *caCurName);
    bool DelFriend(const char *caCurName, const char *caTarName);
private://转成单例模式把这些删了
    explicit OperateDB(QObject *parent = nullptr);
    OperateDB(const OperateDB& instance)=delete ;
    OperateDB& operator=(const OperateDB)=delete ;
    QSqlDatabase m_db;
signals:

};

#endif // OPERATEDB_H
