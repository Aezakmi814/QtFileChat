#include "operatedb.h"
#include "server.h"
#include"stdlib.h"
#include<QDebug>

#include <QApplication>
//定义柔性数组
typedef  struct  ABC{
    int a;
    int b;
    char c[];
}ABC;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OperateDB::getInstance().connectSQL();//数据库连接
    Server w;
   // w.show();
    qDebug() << sizeof (ABC);
    ABC* p=(ABC*)malloc(sizeof (ABC)+12);//12?
    p->a=1;
    p->b=2;
    //复制到哪去 复制的内容 复制的长度
    memcpy(p->c,"hello world",12);
    qDebug() <<p->a<<p->b<<p->c;
    return a.exec();
}
