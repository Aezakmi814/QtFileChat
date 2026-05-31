#include "client.h"
#include "index.h"
#include "sharefile.h"
#include "ui_sharefile.h"

#include <QDebug>

ShareFile::ShareFile(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShareFile)
{
    ui->setupUi(this);
}

ShareFile::~ShareFile()
{
    delete ui;
}

void ShareFile::updateFriendList()
{
    ui->friend_LW->clear();
    QListWidget* friendLW = Index::getInstance().getFriend()->getFriendLW();
    for (int i = 0; i < friendLW->count(); i++) {
        QListWidgetItem* friendItem = friendLW->item(i);
        QListWidgetItem* newItem = new QListWidgetItem(*friendItem);
        ui->friend_LW->addItem(newItem);
    }
}

void ShareFile::on_allSelect_PB_clicked()
{
    for (int i = 0; i < ui->friend_LW->count(); ++i) {
        ui->friend_LW->item(i)->setSelected(true);
    }
}

void ShareFile::on_cancelAllSelect_PB_clicked()
{
    for (int i = 0; i < ui->friend_LW->count(); ++i) {
        ui->friend_LW->item(i)->setSelected(false);
    }
}

void ShareFile::on_ok_PB_clicked()
{
    QString strCurName = Client::getInstance().m_strLoginName;
    QString strCurPath = Index::getInstance().getFile()->m_strCurPath;
    QString strShareFileName = Index::getInstance().getFile()->m_strShareFileName;
    QString strPath = strCurPath + "/" + strShareFileName;

    QList<QListWidgetItem*> pItems = ui->friend_LW->selectedItems();
    int friendNum = pItems.size();

    if (friendNum == 0) {
        return;
    }

    // caData: 当前用户名(32) + 好友数量(int)
    // caMsg: 好友名列表(每个32字节) + 文件路径
    PDU* pdu = mkPDU(friendNum * 32 + strPath.toStdString().size() + 1);
    pdu->uiType = ENUM_TYPE_SHARE_FILE_REQUEST;
    memcpy(pdu->caData, strCurName.toStdString().c_str(), 32);
    memcpy(pdu->caData + 32, &friendNum, sizeof(int));

    for (int i = 0; i < friendNum; i++) {
        memcpy((char*)(pdu->caMsg) + i * 32,
               pItems.at(i)->text().toStdString().c_str(), 32);
    }
    memcpy((char*)(pdu->caMsg) + friendNum * 32,
           strPath.toStdString().c_str(), strPath.toStdString().size());

    Client::getInstance().sendMsg(pdu);
    this->hide();
}
