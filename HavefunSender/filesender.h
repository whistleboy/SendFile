#ifndef FILESENDER_H
#define FILESENDER_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QString>
#include <QDialog>
#include <QLineEdit>
#include <QMap>

namespace Ui {
class FileSender;
}

class FileSender : public QWidget
{
    Q_OBJECT

public:
    explicit FileSender(QWidget *parent = 0);
    ~FileSender();

private:
    Ui::FileSender *ui;

    //init reciever socket & params recieved
    QTcpServer* recievedServer;
    QTcpSocket* recievedSocket;
    QFile* recievedNewFile;
    QByteArray recievedInBlock;
    QString recievedFilePath;
    QString recievedFileName;
    qint64 recievedTotalSize;
    qint64 recievedByteRecieved;

    // init sender socket & params
    QTcpSocket* sendTcpClient;
    QFile* sendLocalFile;
    QString sendFileName;
    QByteArray sendOutBlock;
    qint64 sendLoadSize;
    qint64 sendByteToWrite;
    qint64 sendTotalSize;
    int sendTimes;

    //sender | reciever params
    QString sendUserIp;
    QMap<QString, QString>* mapUserIp;

    // dialog
    QDialog* chooseUserDialog;
    QLineEdit* edit_user_ip;

private slots:
    // reciever slots declare
    void acceptConnetion();
    void readClient();

    // sender slots declare
    void send();
    void goOnSend(qint64);
    void addUserIp();

    void on_btn_add_send_file_clicked();
    void on_btn_send_to_user_clicked();
    void on_btn_add_send_user_clicked();
    void on_btn_recv_save_file_clicked();
};


#endif // FILESENDER_H
