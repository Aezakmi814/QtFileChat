#ifndef FILE_H
#define FILE_H

#include "protocol.h"

#include <QListWidget>
#include <QWidget>

namespace Ui {
class File;
}

class File : public QWidget
{
    Q_OBJECT

public:
    explicit File(QWidget *parent = nullptr);
    ~File();
    QString m_strUsePath;
    QString m_strCurPath;
    QString m_strUserPath;
    QList<FileInfo*>m_pFileInfoList;
    QString m_strUploadFilePath;
    void flushFile();
    void updateFileList(QList< FileInfo * > pFileInfoList);

    void on_upload_PB_clicked();
    void uploadFile();
    void uploadErrorBox(const QString &msg);
private slots:
    void on_mkdir_PB_clicked();

    void on_flush_PB_clicked();

    void on_del_PB_clicked();

    void on_return_PB_clicked();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);


private:
    Ui::File *ui;
};

#endif // FILE_H
