#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDesktopWidget>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->pushButton_skipb->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));
    ui->pushButton_play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->pushButton_skipf->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
    ui->pushButton_pageLast->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    ui->pushButton_pageNext->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    ui->lineEdit_page->setValidator(new QIntValidator(1,50));
    move((QApplication::desktop()->width()-width())/2,(QApplication::desktop()->height()-height())/2);
    connect(ui->lineEdit_search,SIGNAL(returnPressed()),this,SLOT(initSearch()));
    connect(ui->lineEdit_page,SIGNAL(returnPressed()),this,SLOT(search()));
    ui->tableWidget->setColumnHidden(2,true);
    ui->tableWidget->setColumnWidth(0,ui->tableWidget->width());
    connect(ui->tableWidget,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(playSong(int,int)));
    player = new QMediaPlayer;
    player->setVolume(100);
    connect(player,SIGNAL(positionChanged(qint64)),this,SLOT(positionChange(qint64)));
    connect(player,SIGNAL(durationChanged(qint64)),this,SLOT(durationChange(qint64)));
    connect(player,SIGNAL(stateChanged(QMediaPlayer::State)),this,SLOT(stateChange(QMediaPlayer::State)));
    connect(ui->slider_progress,SIGNAL(valueChanged(int)),this,SLOT(setSTime(int)));
    connect(ui->slider_progress,SIGNAL(sliderReleased()),this,SLOT(setMPPosition()));
    QListWidgetItem *LWI1,*LWI2,*LWI3,*LWI4,*LWI5;
    LWI1 = new QListWidgetItem(QIcon("qqmusic.png"), "音乐馆");
    LWI2 = new QListWidgetItem(QIcon("video.png"), "MV");
    LWI3 = new QListWidgetItem(QIcon("radio.png"), "电台");
    ui->listWidget_online->insertItem(1, LWI1);
    ui->listWidget_online->insertItem(2, LWI2);
    ui->listWidget_online->insertItem(3, LWI3);
    //connect(ui->listViewNav,SIGNAL(clicked(QModelIndex)),this,SLOT(nav(QModelIndex)));
    LWI1 = new QListWidgetItem(QIcon("heart.png"), "我喜欢");
    LWI2 = new QListWidgetItem(QIcon("computer.png"), "本地和下载");
    LWI3 = new QListWidgetItem(QIcon("history.png"), "播放历史");
    LWI4 = new QListWidgetItem(QIcon("LTL.png"), "试听列表");
    LWI5 = new QListWidgetItem(QIcon("trolley.png"), "已购音乐");
    ui->listWidget_mine->insertItem(1, LWI1);
    ui->listWidget_mine->insertItem(2, LWI2);
    ui->listWidget_mine->insertItem(3, LWI3);
    ui->listWidget_mine->insertItem(4, LWI4);
    ui->listWidget_mine->insertItem(5, LWI5);
    downloadDir = QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first();
    NAM = new QNetworkAccessManager;
    getKey();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_about_triggered()
{
    QDialog *dialog=new QDialog;
    dialog->setWindowTitle("关于");
    dialog->setFixedSize(420,340);
    QVBoxLayout *vbox=new QVBoxLayout;
    QHBoxLayout *hbox=new QHBoxLayout;
    QLabel *label=new QLabel;
    label->setPixmap(QPixmap("logo.png"));
    hbox->addWidget(label);
    vbox->addLayout(hbox);
    //vbox->addWidget(label);
    label=new QLabel;
    QFont font;
    font.setPointSize(12);
    label->setFont(font);
    label->setText("    一款基于Qt的QQ音乐播放器，拟补QQ音乐没有Linux客户端的不足，音乐版权归腾讯所有。\n作者：黄颖\nE-mail: sonichy@163.com\n主页：sonichy.96.lt\n参考:\nUI：QQ音乐\nAPI：https://github.com/deepins/qq-music-api");
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignTop);
    vbox->addWidget(label);
    dialog->setLayout(vbox);
    dialog->show();
}

void MainWindow::on_action_directory_triggered()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(downloadDir));
}

void MainWindow::initSearch()
{
    ui->lineEdit_page->setText(QString::number(1));
    search();
}

void MainWindow::search()
{
    QString word = ui->lineEdit_search->text();    
    if(word!=""){
        qDebug() << word;
        QString surl = "http://c.y.qq.com/soso/fcgi-bin/client_search_cp?t=0&aggr=1&lossless=1&cr=1&catZhida=1&format=json&p=" + ui->lineEdit_page->text() + "&n=20&w=" + word;
        qDebug() <<  surl;
        QUrl url = QString(surl);
        QNetworkRequest request(url);
        QNetworkReply *reply = NAM->get(request);
        QEventLoop loop;
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();
        QJsonDocument json;        
        QJsonArray list;
        QByteArray responseText = reply->readAll();
        //qDebug() <<  responseText;
        json = QJsonDocument::fromJson(responseText);
        list = json.object().value("data").toObject().value("song").toObject().value("list").toArray();
        //qDebug() << list;
        ui->tableWidget->setRowCount(0);
        for(int i=0;i<list.size();i++){
            ui->tableWidget->insertRow(i);
            //qDebug() << list[i].toObject().value("songname").toString();
            ui->tableWidget->setItem(i,0,new QTableWidgetItem(list[i].toObject().value("songname").toString()));
            ui->tableWidget->setItem(i,1,new QTableWidgetItem(list[i].toObject().value("singer").toArray()[0].toObject().value("name").toString()));
            ui->tableWidget->setItem(i,2,new QTableWidgetItem("http://dl.stream.qqmusic.qq.com/M500" + list[i].toObject().value("songmid").toString() + ".mp3?vkey=" + key + "&guid=85880580&fromtag=30"));
        }
        ui->tableWidget->resizeColumnsToContents();
        QString singerPic = json.object().value("data").toObject().value("zhida").toObject().value("zhida_singer").toObject().value("singerPic").toString();
        request.setUrl(singerPic);
        reply = NAM->get(request);
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();
        QPixmap pixmap;
        pixmap.loadFromData(reply->readAll());
        ui->label_cover->setPixmap(pixmap.scaled(70,70));
    }
}

void MainWindow::getKey()
{
    QString surl = "https://c.y.qq.com/base/fcgi-bin/fcg_musicexpress.fcg?json=3&format=json&guid=85880580";
    qDebug() <<  surl;
    QUrl url = QString(surl);
    QNetworkRequest request(url);
    QNetworkReply *reply = NAM->get(request);
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    QJsonDocument json;
    QByteArray responseText = reply->readAll();
    //qDebug() <<  responseText;
    json = QJsonDocument::fromJson(responseText);
    //if (!json.isNull()){
    key = json.object().value("key").toString();
    //}
    qDebug() << "key:" << key;
}

void MainWindow::playSong(int r,int c)
{
    ui->label_SongSinger->setText(ui->tableWidget->item(r,0)->text() + " - " + ui->tableWidget->item(r,1)->text());
    QString surl=ui->tableWidget->item(r,2)->text();
    qDebug() << surl;
    player->setMedia(QUrl(surl));
    player->play();
    ui->pushButton_play->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    ui->slider_progress->setMaximum(player->duration());
}

void MainWindow::on_pushButton_play_clicked()
{
    if(player->state()==QMediaPlayer::PlayingState){
        player->pause();
        ui->pushButton_play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    }else if(player->state()==QMediaPlayer::PausedState){
        player->play();
        ui->pushButton_play->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    }else if(player->state()==QMediaPlayer::StoppedState){
        player->play();
        ui->pushButton_play->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    }
}

void MainWindow::on_pushButton_skipf_clicked()
{
    if(ui->tableWidget->currentRow() < ui->tableWidget->rowCount()-1){
        ui->tableWidget->setCurrentCell(ui->tableWidget->currentRow()+1,0);
        playSong(ui->tableWidget->currentRow(),0);
    }
}

void MainWindow::on_pushButton_skipb_clicked()
{
    if(ui->tableWidget->currentRow()>0){
        ui->tableWidget->setCurrentCell(ui->tableWidget->currentRow()-1,0);
        playSong(ui->tableWidget->currentRow(),0);
    }
}

void MainWindow::positionChange(qint64 p)
{
    ui->slider_progress->setValue(p);
}

void MainWindow::durationChange(qint64 d)
{
    ui->slider_progress->setMaximum(d);
    //qDebug() << "player->duration()=" << player->duration() << d;
}

void MainWindow::setMPPosition()
{
    player->setPosition(ui->slider_progress->value());
}

void MainWindow::setSTime(int v)
{
    QTime t(0,0,0);
    t=t.addMSecs(v);
    QString sTimeElapse=t.toString("hh:mm:ss");
    t.setHMS(0,0,0);
    t=t.addMSecs(player->duration());
    QString sTimeTotal=t.toString("hh:mm:ss");
    ui->label_time->setText(sTimeElapse+" / "+sTimeTotal);
}

void MainWindow::stateChange(QMediaPlayer::State state)
{
    qDebug() << state;
//    if(state == QMediaPlayer::StoppedState){
//        if(ui->tableWidget->currentRow() < ui->tableWidget->rowCount()-1){
//            ui->tableWidget->setCurrentCell(ui->tableWidget->currentRow()+1,0);
//            playSong(ui->tableWidget->currentRow(),0);
//        }
//    }
}

void MainWindow::on_pushButton_download_clicked()
{
    QString surl = ui->tableWidget->item(ui->tableWidget->currentRow(),2)->text();
    qDebug() <<  "download -> " + surl;
    QUrl url = QString(surl);
    QNetworkRequest request(url);
    QNetworkReply *reply = NAM->get(request);
    QEventLoop loop;
    connect(reply,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(updateProgress(qint64,qint64)));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    QString filename = ui->tableWidget->item(ui->tableWidget->currentRow(),0)->text() + " - " + ui->tableWidget->item(ui->tableWidget->currentRow(),1)->text() + "." + QFileInfo(surl).suffix().left(QFileInfo(surl).suffix().indexOf("?"));
    QString filepath = downloadDir + "/" + filename;
    qDebug() <<  "path -> " + filepath;
    QFile file(filepath);
    file.open(QIODevice::WriteOnly);
    file.write(reply->readAll());
    file.close();
    ui->pushButton_download->setText("↓");
    ui->pushButton_download->setEnabled(true);
}

void MainWindow::updateProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    ui->pushButton_download->setEnabled(false);
    ui->pushButton_download->setText(QString("%1%").arg(bytesReceived*100/bytesTotal));
}

void MainWindow::on_pushButton_pageLast_clicked()
{
    if(ui->lineEdit_page->text().toInt() > 1){
        page = page - 1;
        ui->lineEdit_page->setText(QString::number(ui->lineEdit_page->text().toInt()-1));
        search();
    }
}

void MainWindow::on_pushButton_pageNext_clicked()
{
    if(ui->lineEdit_page->text().toInt() < 51){
        ui->lineEdit_page->setText(QString::number(ui->lineEdit_page->text().toInt()+1));
        search();
    }
}
