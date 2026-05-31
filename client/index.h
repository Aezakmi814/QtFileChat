#ifndef INDEX_H
#define INDEX_H

#include "file.h"
#include "friend.h"

#include <QWidget>

namespace Ui {
class Index;
}

class Index : public QWidget
{
    Q_OBJECT

public:
    static Index& getInstance();
    ~Index();
    Friend* getFriend();
    File* getFile();

private slots:
    void on_friend_PB_clicked();
    void on_file_PB_clicked();



private:
    explicit Index(QWidget *parent = nullptr);
    Index(const Index&) = delete;
    Index& operator=(const Index&) = delete;

    Ui::Index *ui;
    static Index* instance;  // 静态实例指针
};

#endif // INDEX_H
