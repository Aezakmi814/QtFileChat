#include "client.h"
#include "file.h"
#include "ui_file.h"
#include "uploader.h"
#include<QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <qinputdialog.h>

File::File(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::File)
{
    ui->setupUi(this);
    m_strUsePath=(QString("%1/%2").arg(Client::getInstance().m_strRootPath).arg(Client::getInstance().m_strLoginName));
    m_strCurPath=m_strUsePath;
}

File::~File()
{
    delete ui;
}

void File::flushFile()
{
    PDU*pdu=mkPDU(m_strCurPath.toStdString().size()+1);
    pdu->uiType=ENUM_TYPE_FLUSH_FILE_REQUEST;
    memcpy(pdu->caMsg,m_strCurPath.toStdString().c_str(),m_strCurPath.toStdString().size());
    Client::getInstance().sendMsg(pdu);
}

void File::updateFileList(QList<FileInfo *> pFileInfoList)
{
    foreach(FileInfo * pFileInfo,m_pFileInfoList){
        delete pFileInfo;
    }
    m_pFileInfoList.clear();
    m_pFileInfoList = pFileInfoList;

    ui->listWidget->clear();
    foreach(FileInfo* pFileInfo,pFileInfoList){
        QListWidgetItem* pItem=new QListWidgetItem(pFileInfo->caName);
        if(pFileInfo->iFileType==0){
            pItem->setIcon(QIcon(QPixmap(":/dir.png")));
        }else{
            pItem->setIcon(QIcon(QPixmap(":/file.png")));
        }
        ui->listWidget->addItem(pItem);
    }
}



void File::on_mkdir_PB_clicked()
{
    QString strName = QInputDialog::getText(this, "创建文件夹", "文件夹名：");
    if(strName.isEmpty()||strName.toStdString().size()>32)
    {
        QMessageBox::information(this,"提示","文件长度非法");
        return;
    }
    PDU*pdu=mkPDU(m_strCurPath.toStdString().size()+1);
    qDebug()<<"创建文件夹名"<<strName;
    pdu->uiType = ENUM_TYPE_MKFILE_REQUEST;
    memcpy(pdu->caData, strName.toStdString().c_str(), 32);
    memcpy(pdu->caMsg, m_strCurPath.toStdString().c_str(),m_strCurPath.toStdString().size());
    Client::getInstance().sendMsg(pdu);
}

void File::on_flush_PB_clicked()
{
    flushFile();
}

void File::on_del_PB_clicked()
{
    // 获取当前选中的列表项
       QListWidgetItem* pItem = ui->listWidget->currentItem();
       if (!pItem) {
           return; // 没有选中项则直接返回
       }

       // 弹出确认对话框
       int ret = QMessageBox::question(this, "删除文件",
                                       QString("是否确认删除文件 %1? ").arg(pItem->text()));
       if (ret != QMessageBox::Yes) {
           return; // 用户取消删除
       }

       // 构建文件路径：当前路径 + 文件名
       QString strPath = QString("%1/%2").arg(m_strCurPath).arg(pItem->text());

       // 创建PDU（协议数据单元）
       PDU* pdu = mkPDU(strPath.toStdString().size() + 1);
       pdu->uiType = ENUM_TYPE_DEL_FILE_REQUEST;

       // 将文件路径拷贝到PDU的消息部分
       memcpy(pdu->caMsg, strPath.toStdString().c_str(), strPath.toStdString().size());

       // 发送删除请求到服务器
       Client::getInstance().sendMsg(pdu);

}

void File::on_return_PB_clicked()
{
    if(m_strCurPath == m_strUsePath) {
        return;
    }
    int index = m_strCurPath.lastIndexOf('/');
    if (index <= 0) {
        m_strCurPath = m_strUsePath;
    } else {
        m_strCurPath.remove(index, m_strCurPath.size() - index);
    }
    flushFile();
}

void File::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    foreach (FileInfo* pFileInfo, m_pFileInfoList) {
        if (pFileInfo->caName == item->text() &&
            pFileInfo->iFileType != 0) {
            return; 
        }
    }
    m_strCurPath = QString("%1/%2").arg(m_strCurPath).arg(item->text());
    flushFile();
}

void File::on_upload_PB_clicked()
{
    m_strUploadFilePath.clear();
    m_strUploadFilePath = QFileDialog::getOpenFileName();
    qDebug() << "m_strUploadFilePath" << m_strUploadFilePath;
    if (m_strUploadFilePath.isEmpty()) {
        return;
    }
    int index = m_strUploadFilePath.lastIndexOf('/');
    QString fileName = m_strUploadFilePath.mid(index + 1);
    QFile file(m_strUploadFilePath);
    qint64 iFileSize = file.size();
    PDU* pdu = mkPDU(m_strCurPath.toStdString().size() + 1);
    pdu->uiType = ENUM_TYPE_UPLOAD_FILE_INIT_REQUEST;
    memcpy(pdu->caData, fileName.toStdString().c_str(), 32);
    memcpy(pdu->caData + 32, &iFileSize, sizeof(qint64));
    memcpy(pdu->caMsg, m_strCurPath.toStdString().c_str(),
           m_strCurPath.toStdString().size());
    Client::getInstance().sendMsg(pdu);
}

void File::uploadFile()
{
    Uploader*uploader=new Uploader(m_strUploadFilePath);
    connect(uploader,&Uploader::errorMsg,this,&File::uploadErrorBox,Qt::QueuedConnection);
    connect(uploader,&Uploader::uploadPDU,&Client::getInstance(),&Client::sendMsg,Qt::QueuedConnection);
    uploader->start();
}

void File::uploadErrorBox(const QString &msg)
{
    QMessageBox::information(this,"提示",msg);
}
