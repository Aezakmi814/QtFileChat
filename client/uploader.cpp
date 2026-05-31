#include "client.h"
#include "uploader.h"
#include <QFile>
#include <QIoDevice>
#include <QThread>


Uploader::Uploader(QString strFilePath)
{
    m_strUploadFilePath = strFilePath;
}

void Uploader::start()
{
    QThread* thread=new QThread;
    this->moveToThread(thread);
    connect(thread,&QThread::started,this,&Uploader::UploaderFile,Qt::QueuedConnection);
    connect(this,&Uploader::finished,thread,&QThread::quit);
    connect(thread,&QThread::finished,thread,&QThread::deleteLater);
    thread->start();
}

void Uploader::UploaderFile()
{
    QFile file(m_strUploadFilePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        emit errorMsg("打开文件失败");
        emit finished();
        return;
    }
    while(true)
    {
        PDU* datapdu = mkPDU(4096);
        datapdu->uiType = ENUM_TYPE_UPLOAD_FILE_DATA_REQUEST;
        qint64 ret = file.read(datapdu->caMsg, 4096);
        if(ret <= 0) {
            free(datapdu);
            if(ret < 0) {
                emit errorMsg("读取文件失败");
            }
            break;
        }
        datapdu->uiMsgLen=ret;
        datapdu->uiTotalLen=ret+sizeof (PDU);
        emit uploadPDU(datapdu);
    }
    file.close();
    emit finished();
}

