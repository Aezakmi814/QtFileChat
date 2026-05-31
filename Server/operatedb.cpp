#include "operatedb.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

OperateDB &OperateDB::getInstance()
{
    static OperateDB instance;
    return instance;
}

void OperateDB::connectSQL()
{
    m_db.setHostName("localhost");
    m_db.setPort(3306);
    m_db.setUserName("root");
    m_db.setPassword("123456");
    m_db.setDatabaseName("test");
    if(m_db.open()){
        qDebug() << "数据库连接成功";
    } else {
        qDebug() << "数据库连接失败" << m_db.lastError().text();
    }
}

void OperateDB::handleOffline(const char *caName)
{
    if(caName == NULL)
    {
        return;
    }
    QString sql = QString("update user_info set online=0 where name='%1'").arg(caName);
    qDebug() << "将用户的online字段置为0 sql" << sql;
    QSqlQuery q;
    bool ret = q.exec(sql);
    qDebug() << "handleOffline ret" << ret;
}

int OperateDB::handleFindUser(const char *caName)
{
    if (caName == NULL) {
        return -1;
    }
    QString sql = QString("select online from user_info where name='%1'").arg(caName);
    qDebug() << "查找用户的online字段 sql" << sql;
    QSqlQuery q;
    if (!q.exec(sql)) {
        return -1;
    }
    if (q.next()) {
        return q.value(0).toInt();
    }
    return 2;
}

int OperateDB::handleADDFriend(const char *caCurName, const char *caTarName)
{
    if(caCurName == NULL || caTarName == NULL)
    {
        return -1;
    }
    if (strcmp(caCurName, caTarName) == 0) {
            return -3;  // 不能添加自己为好友
        }

    // 1. 检查是否已经是好友
    QString sql = QString(R"(
        select * from friend where
        (user_id=(select id from user_info where name='%1')
         and
        friend_id=(select id from user_info where name='%2'))
        or
        (user_id=(select id from user_info where name='%2')
         and friend_id=(select id from user_info where name='%1'))
    )").arg(caCurName).arg(caTarName);

    qDebug() << "判断是否是好友 sql" << sql;

    QSqlQuery q;
    q.exec(sql);

    if(!q.exec(sql)) {
        qDebug() << "SQL执行失败:" << q.lastError().text();
        return -1;
    }

    if(q.next()){
        return -2;  // 已经是好友
    }

    // 2. 检查目标用户是否在线
    sql = QString("select online from user_info where name='%1'").arg(caTarName);  // 修复：online 不是 omline
    qDebug() << "查询用户是否在线 sql" << sql;

    if(!q.exec(sql)) {
        qDebug() << "SQL执行失败:" << q.lastError().text();
        return -1;
    }

    if(q.next()){
        int online = q.value(0).toInt();
        if(online == 1){
            return 1;  // 用户在线，可以转发
        } else {
            return 0;  // 用户不在线
        }
    }

    return -3;  // 用户不存在
}

bool OperateDB::handleAddFriendAgree(const char *caCurName, const char *caTarName)
{
    if(caCurName == NULL || caTarName == NULL)
    {
        return false;
    }

    QString sql = QString(R"(
        insert into friend(user_id, friend_id)
        select u1.id, u2.id
        from user_info u1, user_info u2
        where u1.name='%1' and u2.name='%2'
    )").arg(caCurName).arg(caTarName);

    qDebug() << "添加好友关系 sql" << sql;

    QSqlQuery q;
    return q.exec(sql);
}

bool OperateDB::handleRegist(const char *caName, const char *caPwd)
{
    if(caName == NULL || caPwd == NULL)
    {
        return false;
    }

    // 1. 检查用户名是否已存在
    QString checkSql = QString("select id from user_info where name='%1'").arg(caName);
    qDebug() << "检查用户名是否存在 sql" << checkSql;

    QSqlQuery checkQuery;
    if(checkQuery.exec(checkSql) && checkQuery.next()) {
        qDebug() << "用户名已存在";
        return false;
    }

    // 2. 插入新用户
    QString sql = QString("insert into user_info(name, pwd, online) values('%1', '%2', 0)").arg(caName).arg(caPwd);
    qDebug() << "注册用户 sql" << sql;

    QSqlQuery q;
    bool ret = q.exec(sql);
    qDebug() << "handleRegist 结果:" << ret;
    return ret;
}

bool OperateDB::handleLogin(const char *caName, const char *caPwd)
{
    if(caName == NULL || caPwd == NULL)
    {
        return false;
    }

    QString sql = QString("select * from user_info where name='%1' and pwd='%2'").arg(caName).arg(caPwd);
    qDebug() << "查找用户名和密码是否存在 sql" << sql;

    QSqlQuery q;
    if(!q.exec(sql) || !q.next()) {  // 通过.next函数判断是否有结果
        qDebug() << "用户名或密码错误";
        return false;
    }

    sql = QString("update user_info set online=1 where name='%1'").arg(caName);
    qDebug() << "将用户的online字段设置1 sql" << sql;

    bool ret = q.exec(sql);
    qDebug() << "登录状态更新结果:" << ret;
    return ret;
}

QStringList OperateDB::handleOnlineUser()
{
    QString sql = QString("select name from user_info where online=1");
       QSqlQuery q;
       q.exec(sql);
       QStringList res;
       while (q.next()) {
           res.append(q.value(0).toString());
       }
       return res;

}



QStringList OperateDB::handleFlushFriend(const char *caCurName)
{
    QStringList res;
    if(caCurName == NULL)
    {
        return res;

    }
    QString sql = QString(R"(
                          SELECT name FROM user_info
                                  WHERE id IN (
                                      -- 我添加的好友
                                      SELECT friend_id FROM friend
                                      WHERE user_id = (SELECT id FROM user_info WHERE name='%1')
                                      UNION
                                      -- 添加我的好友
                                      SELECT user_id FROM friend
                                      WHERE friend_id = (SELECT id FROM user_info WHERE name='%1')
                                  )
                                  -- 关键：排除当前用户自己
                                  AND name != '%1'
                                  AND online = 1
                          )").arg(caCurName);
    QSqlQuery q;
    q.exec(sql);
    while(q.next()){
        res.append(q.value(0).toString());
    }
    return res;


}

bool OperateDB::DelFriend(const char *caCurName, const char *caTarName)
{
    if(caCurName == NULL || caTarName == NULL){
        return false;
    }



    QString sql = QString(R"(
                          delete from friend where
                          (
                          user_id=(select id from user_info where name='%1')
                          and
                          friend_id=(select id from user_info where name='%2')
                            )
                                  or
                            (
                          friend_id=(select id from user_info where name='%1')
                                      and
                          user_id=(select id from user_info where name='%2')
                            );
                           )").arg(caCurName).arg(caTarName);
    QSqlQuery q;
    return q.exec(sql);
}


OperateDB::~OperateDB()
{
    m_db.close();
}

OperateDB::OperateDB(QObject *parent) : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QMYSQL");
}
