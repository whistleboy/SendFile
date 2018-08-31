#include "filesender.h"
#include "ui_filesender.h"
#include <QWidget>
#include <QTextCodec>
#include <QFileDialog>
#include <QDebug>
#include <QStringListModel>
#include <QLineEdit>
#include <QPushButton>
#include <QList>
#include <QLabel>

FileSender::FileSender(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileSender)
{
    ui->setupUi(this);

    // init ui of reciever
//    ui->label_recv_status->hide();
//    ui->process_recv->hide();
    // set codec
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    // init ui of sender
//    ui->label_send_status->hide();
//    ui->process_send->hide();
    sendTcpClient = new QTcpSocket(this);
    sendTimes = 0;

    // init send connect
    connect(sendTcpClient, SIGNAL(connected()), this, SLOT(send()));
    connect(sendTcpClient, SIGNAL(bytesWritten(qint64)), this, SLOT(goOnSend(qint64)));
}

void FileSender::acceptConnetion() {

    ui->label_sended_file->setText(tr("Connected. Ready to recieve file!"));
    recievedSocket = recievedServer->nextPendingConnection();
    connect(recievedSocket,SIGNAL(readyRead()), this, SLOT(readClient()));
}

void FileSender::readClient() {
    ui->label_sended_file->setText(tr("file is recieving....."));
    if (0 == recievedByteRecieved) {
        ui->process_recv->setValue(0);
        QDataStream in(recievedSocket);
        in >> recievedTotalSize >> recievedByteRecieved >> recievedFileName;
        recievedFileName = recievedFilePath + "/" + recievedFileName;
        recievedNewFile = new QFile(recievedFileName);
        if (false ==  recievedNewFile->open(QFile::WriteOnly)) {
            qDebug() << "open file failed!!!";
        }
        // init list_view_files
        QStringListModel *recvModel;
        QStringList fileName_list;
        fileName_list.push_back(recievedFileName);
        recvModel = new QStringListModel(this);
        recvModel->setStringList(fileName_list);
        ui->list_sending_file->setModel(recvModel);
    } else {
        recievedInBlock = recievedSocket->readAll();
        recievedByteRecieved += recievedInBlock.size();
        recievedNewFile->write(recievedInBlock);
        recievedNewFile->flush();
    }
    ui->label_recv_status->show();
//    ui->process_recv->show();
    ui->process_recv->setMaximum(recievedTotalSize);
    ui->process_recv->setValue(recievedByteRecieved);

    if (recievedByteRecieved == recievedTotalSize) {
        ui->label_sended_file->setText(tr("file[s] had recieved completed."));

        recievedInBlock.clear();
        recievedTotalSize = 0;
        recievedByteRecieved = 0;

        ui->process_recv->setValue(0);
        ui->process_send->setValue(0);

    }
}

// sender
void FileSender::send(){
    // init file size
    sendByteToWrite = sendLocalFile->size();
    sendTotalSize = sendLocalFile->size();
    sendLoadSize = 4 * 1024;

    // out fileName
    QDataStream out(&sendOutBlock, QIODevice::WriteOnly);
    QString currentFileName = sendFileName.right(sendFileName.size() - sendFileName.lastIndexOf('/') - 1);
    out << qint64(0) << qint64(0) << currentFileName;

    // out totalSize & outBlockSize
    sendTotalSize += sendOutBlock.size();
    sendByteToWrite += sendOutBlock.size();
    out.device()->seek(0);
    out << sendTotalSize << qint64(sendOutBlock.size());

    // write to tcpSocket
    sendTcpClient->write(sendOutBlock);

    // set ui's progress
//    ui->label_send_status->show();
//    ui->process_send->show();
    ui->process_send->setMaximum(sendTotalSize);
    ui->process_send->setValue(sendTotalSize - sendByteToWrite);
}
void FileSender::goOnSend(qint64 numBytes) {
    sendByteToWrite -= numBytes;
    sendOutBlock = sendLocalFile->read(qMin(sendLoadSize, sendByteToWrite));
    sendTcpClient->write(sendOutBlock);
    ui->process_send->setMaximum(sendTotalSize);
    ui->process_send->setValue(sendTotalSize - sendByteToWrite);
    if (sendByteToWrite == 0)
        ui->label_add_send_file->setText("file completed!");
}

void FileSender::addUserIp() {

    sendUserIp = edit_user_ip->text();

    QMap<QString, QString>* mapUserIp = new QMap<QString, QString>();
    mapUserIp->insert(tr("user1"), sendUserIp);

//    qDebug() << "user IP:" << sendUserIp;
    chooseUserDialog->close();

    // init list_view_files
    QStringListModel *userModel;
    QStringList fileName_list3;
    QList<QString> userIPs = mapUserIp->values();
    foreach (QString userIp, userIPs) {
        fileName_list3.push_back(userIp);
    }
    userModel = new QStringListModel(this);
    userModel->setStringList(fileName_list3);
    ui->list_send_user->setModel(userModel);
}

FileSender::~FileSender()
{
    delete ui;
}

void FileSender::on_btn_add_send_file_clicked()
{
    // init ui
    ui->label_add_send_file->setText("opening file......");
    // ui->process_send->setValue(0);
    // init parament
    sendLoadSize = 0;
    sendByteToWrite = 0;
    sendTotalSize = 0;
    sendOutBlock.clear();
    // open files
    sendFileName = QFileDialog::getOpenFileName(this);
    sendLocalFile = new QFile(sendFileName);
    sendLocalFile->open(QFile::ReadOnly);
    ui->label_add_send_file->setText(tr("opened file:"));

    // init list_view_files
    QStringListModel *sendModel;
    QStringList fileName_list2;
    fileName_list2.push_back(sendFileName);
    sendModel = new QStringListModel(this);
    sendModel->setStringList(fileName_list2);
    ui->list_add_send_file->setModel(sendModel);
}

void FileSender::on_btn_send_to_user_clicked()
{
//    QString sendIp = mapUserIp->value(tr("user1"));
    if (sendUserIp.length() == 0) {
        qDebug() << "Ip is empty!!!";
        return;
    }

    if (sendFileName.length() == 0) {
        qDebug() << "file is not selected";
        return;
    }

    // if first send ,then conneting distance ip:port
    if (0 == sendTimes) {
        sendTcpClient->connectToHost(QHostAddress(sendUserIp),8888);
        sendTimes = 1;
    } else {
        send();
    }
    ui->label_add_send_file->setText(tr("file is sending %1").arg(sendFileName));
}

void FileSender::on_btn_recv_save_file_clicked()
{
    recievedFilePath =  QFileDialog::getExistingDirectory(this);

    qDebug() << "File Path:" << recievedFilePath;

    recievedTotalSize = 0;
    recievedByteRecieved = 0;
    recievedServer = new QTcpServer(this);
    recievedServer->listen(QHostAddress::AnyIPv4,8888);
    // init listen
    connect(recievedServer, SIGNAL(newConnection()), this, SLOT(acceptConnetion()));
    ui->process_recv->setValue(0);
    ui->label_sended_file->setText(tr("start listening......"));
}

void FileSender::on_btn_add_send_user_clicked()
{
    chooseUserDialog = new QDialog(this);
    chooseUserDialog->setGeometry(450, 300, 300, 250);

    QLabel* label_recommand = new QLabel(chooseUserDialog);
    label_recommand->setGeometry(30, 80, 250, 30);
    label_recommand->setText(tr("Please input a IP Address Below:"));

    edit_user_ip = new QLineEdit(chooseUserDialog);
    edit_user_ip->setGeometry(30, 120, 200, 20);
    QPushButton* btn_user_ip = new QPushButton(chooseUserDialog);
    btn_user_ip->setGeometry(200, 160, 90, 30);
    btn_user_ip->setText(tr("OK"));

    connect(btn_user_ip, SIGNAL(clicked()), this, SLOT(addUserIp()));

    chooseUserDialog->exec();
}
