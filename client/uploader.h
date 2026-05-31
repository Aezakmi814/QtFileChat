#ifndef UPLOADER_H
#define UPLOADER_H

#include "protocol.h"

#include <QObject>

class Uploader : public QObject
{
    Q_OBJECT
public:
    //explicit Uploader(QObject *parent = nullptr);
    QString m_strUploadFilePath;
    Uploader(QString strFilePath);
    void start();


public slots:
    void UploaderFile();
signals:
    void errorMsg(const  QString& msg);
    void uploadPDU(PDU*pdu);
    void finished();
};

#endif // UPLOADER_H
