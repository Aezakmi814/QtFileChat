#include "client.h"
#include "index.h"
#include <QApplication>
#include <QProcess>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 获取单例并显示
    Client::getInstance().show();

    return a.exec();
}
