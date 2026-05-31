#include "index.h"
#include "ui_index.h"
#include <QDebug>
#include <QMessageBox>

Index* Index::instance = nullptr;

// 实现单例方法
Index& Index::getInstance()
{
    if (instance == nullptr) {
        instance = new Index();
    }
    return *instance;
}

Index::Index(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Index)
{
    ui->setupUi(this);
//    setWindowTitle("主界面");

}

Index::~Index()
{
    delete ui;
    instance = nullptr;  // 防止悬空指针
}

Friend *Index::getFriend()
{
    return ui->friendPage;

}

File *Index::getFile()
{
    return ui->filePage;

}

void Index::on_friend_PB_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void Index::on_file_PB_clicked()
{

    ui->stackedWidget->setCurrentIndex(1);

}


