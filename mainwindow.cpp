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
#include <QTextCodec>
#include <QFontDialog>
#include <QColorDialog>
#include <QSettings>
#include <QDir>
#include <QFileDialog>
#include <QClipboard>
#include <QTextBlock>
#include <QShortcut>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    guid = "6964837424";
    move((QApplication::desktop()->width()-width())/2,(QApplication::desktop()->height()-height())/2);
    sstyle = styleSheet();
    connect(new QShortcut(QKeySequence(Qt::Key_Escape),this), SIGNAL(activated()),this, SLOT(exitFullscreen()));
    connect(new QShortcut(QKeySequence(Qt::Key_Space),this), SIGNAL(activated()),this, SLOT(on_pushButton_play_clicked()));
    label_cover = new QLabel;
    label_cover->setWindowFlags(Qt::Dialog);
    ui->textBrowser->zoomIn(2);
    ui->pushButton_skipb->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    ui->pushButton_play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->pushButton_skipf->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    ui->pushButton_pageLast->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    ui->pushButton_pageNext->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    ui->lineEdit_page->setValidator(new QIntValidator(1,50));

    connect(ui->lineEdit_search,SIGNAL(returnPressed()),this,SLOT(initSearch()));
    connect(ui->lineEdit_page,SIGNAL(returnPressed()),this,SLOT(search()));
    ui->tableWidget->setColumnHidden(2,true);
    ui->tableWidget->setColumnHidden(3,true);
    ui->tableWidget->setColumnHidden(4,true);
    ui->tableWidget->setColumnWidth(0,ui->tableWidget->width());
    connect(ui->tableWidget,SIGNAL(cellClicked(int,int)),this,SLOT(copy(int,int)));
    connect(ui->tableWidget,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(playSong(int,int)));

    player = new QMediaPlayer;
    player->setVolume(100);
    connect(player,SIGNAL(positionChanged(qint64)),this,SLOT(positionChange(qint64)));
    connect(player,SIGNAL(durationChanged(qint64)),this,SLOT(durationChange(qint64)));
    connect(player,SIGNAL(stateChanged(QMediaPlayer::State)),this,SLOT(stateChange(QMediaPlayer::State)));

    connect(ui->slider_progress,SIGNAL(valueChanged(int)),this,SLOT(setSTime(int)));
    connect(ui->slider_progress,SIGNAL(sliderReleased()),this,SLOT(setMPPosition()));

    QListWidgetItem *LWI;
    LWI = new QListWidgetItem("在线音乐");
    LWI->setFlags(Qt::NoItemFlags);
    ui->listWidget->insertItem(0, LWI);
    LWI = new QListWidgetItem(QIcon(":/qqmusic.png"), "音乐馆");
    ui->listWidget->insertItem(1, LWI);
    LWI = new QListWidgetItem(QIcon(":/video.png"), "MV");
    ui->listWidget->insertItem(2, LWI);
    LWI = new QListWidgetItem(QIcon(":/radio.png"), "电台");
    ui->listWidget->insertItem(3, LWI);
    LWI = new QListWidgetItem("我的音乐");
    LWI->setFlags(Qt::NoItemFlags);
    ui->listWidget->insertItem(4, LWI);
    LWI = new QListWidgetItem(QIcon(":/heart.png"), "我喜欢");
    ui->listWidget->insertItem(5, LWI);
    LWI = new QListWidgetItem(QIcon(":/computer.png"), "本地和下载");
    ui->listWidget->insertItem(6, LWI);
    LWI = new QListWidgetItem(QIcon(":/history.png"), "播放历史");
    ui->listWidget->insertItem(7, LWI);
    LWI = new QListWidgetItem(QIcon(":/LTL.png"), "试听列表");
    ui->listWidget->insertItem(8, LWI);
    LWI = new QListWidgetItem(QIcon(":/trolley.png"), "已购音乐");
    ui->listWidget->insertItem(9, LWI);
    downloadPath = QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first();
    NAM = new QNetworkAccessManager;

    desktopLyric = new Form;
    QString slx = readSettings(QDir::currentPath() + "/config.ini", "config", "LyricX");
    QString sly = readSettings(QDir::currentPath() + "/config.ini", "config", "LyricY");
    if (slx=="" || sly=="" || slx.toInt()>QApplication::desktop()->width() || sly.toInt()>QApplication::desktop()->height()) {
        desktopLyric->move((QApplication::desktop()->width()-desktopLyric->width())/2, QApplication::desktop()->height()-desktopLyric->height());
    } else {
        desktopLyric->move(slx.toInt(),sly.toInt());
    }
    //qDebug() << "歌词坐标" << slx << sly;
    QColor color(readSettings(QDir::currentPath() + "/config.ini", "config", "LyricFontColor"));
    QPalette plt;
    plt.setColor(QPalette::WindowText, color);
    desktopLyric->ui->label_lyric->setPalette(plt);
    QString sfont = readSettings(QDir::currentPath() + "/config.ini", "config", "Font");
    if (sfont != "") {
        QStringList SLFont = sfont.split(",");
        desktopLyric->ui->label_lyric->setFont(QFont(SLFont.at(0),SLFont.at(1).toInt(),SLFont.at(2).toInt(),SLFont.at(3).toInt()));
    }
    connect(desktopLyric, SIGNAL(pushButton_lyric_toggle()), ui->pushButton_lyric, SLOT(toggle()));
    connect(desktopLyric->ui->pushButton_set, SIGNAL(pressed()), this, SLOT(on_action_settings_triggered()));
    desktopLyric->show();

    downloadPath = readSettings(QDir::currentPath() + "/config.ini", "config", "DownloadPath");
    if (downloadPath == "") {
        downloadPath = QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_directory_triggered()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(downloadPath));
}

void MainWindow::on_action_about_triggered()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("关于");
    dialog->setFixedSize(500,360);
    QVBoxLayout *vbox = new QVBoxLayout;
    QLabel *label = new QLabel;
    label->setPixmap(QPixmap("logo.png"));
    label->setAlignment(Qt::AlignCenter);
    vbox->addWidget(label);
    label = new QLabel;
    QFont font;
    font.setPointSize(12);
    label->setFont(font);
    label->setText("QQ音乐 V2.3");
    label->setAlignment(Qt::AlignCenter);
    vbox->addWidget(label);
    label = new QLabel;
    //font.setPointSize(12);
    label->setFont(font);
    label->setText("         一款基于Qt的QQ音乐播放器，拟补QQ音乐没有Linux客户端的不足，音乐版权归腾讯所有。\n作者：黄颖\nE-mail: sonichy@163.com\n主页：sonichy.96.lt\n参考:\nUI：QQ音乐\n参考：https://www.cnblogs.com/songwei1/p/7860758.html");
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignTop);
    vbox->addWidget(label);
    dialog->setLayout(vbox);
    dialog->show();
}

void MainWindow::initSearch()
{
    ui->lineEdit_page->setText(QString::number(1));
    search();
}

void MainWindow::search()
{
    QString word = ui->lineEdit_search->text();
    if (word != "") {
        qDebug() << word;
        QString surl = "http://c.y.qq.com/soso/fcgi-bin/client_search_cp?t=0&aggr=1&lossless=1&cr=1&catZhida=1&format=json&p=" + ui->lineEdit_page->text() + "&n=20&w=" + word;
        qDebug() <<  surl;
        QUrl url = QString(surl);
        QNetworkRequest request(url);
        QNetworkReply *reply = NAM->get(request);
        QEventLoop loop;
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();
        QByteArray BA = reply->readAll();
        //qDebug() << BA;
        // 写log
        QFile file("search.log");
        if (file.open(QFile::WriteOnly)) {
            file.write(QString(BA).toUtf8());
            file.close();
        }

        QJsonDocument json = QJsonDocument::fromJson(BA);
        QJsonArray list = json.object().value("data").toObject().value("song").toObject().value("list").toArray();
        ui->tableWidget->setRowCount(0);
        for (int i=0; i<list.size(); i++) {
            ui->tableWidget->insertRow(i);
            ui->tableWidget->setItem(i,0,new QTableWidgetItem(list[i].toObject().value("songname").toString()));
            ui->tableWidget->setItem(i,1,new QTableWidgetItem(list[i].toObject().value("singer").toArray()[0].toObject().value("name").toString()));
            ui->tableWidget->setItem(i,2,new QTableWidgetItem(list[i].toObject().value("songmid").toString()));
            ui->tableWidget->setItem(i,3,new QTableWidgetItem(list[i].toObject().value("albummid").toString()));
            ui->tableWidget->setItem(i,4,new QTableWidgetItem("http://music.qq.com/miniportal/static/lyric/" + QString::number(list[i].toObject().value("songid").toInt()%100)+ "/" + QString::number(list[i].toObject().value("songid").toInt()) + ".xml"));
        }
        ui->tableWidget->resizeColumnsToContents();
        ui->tableWidget->scrollToTop();
    }
}

void MainWindow::copy(int r, int c)
{
    QApplication::clipboard()->setText(ui->tableWidget->item(r,c)->text());
}

void MainWindow::playSong(int r, int c)
{
    Q_UNUSED(c);
    ui->pushButton_download->setStyleSheet("");
    ui->label_SongSinger->setText(ui->tableWidget->item(r,0)->text() + " - " + ui->tableWidget->item(r,1)->text());
    QString surl = "https://c.y.qq.com/base/fcgi-bin/fcg_music_express_mobile3.fcg?g_tk=1160855065&&loginUin=247990761&hostUin=0&format=json&inCharset=utf8&outCharset=utf-8¬ice=0&platform=yqq&needNewCode=0&cid=205361747&&uin=247990761&songmid=" + ui->tableWidget->item(r,2)->text() + "&filename=C400" + ui->tableWidget->item(r,2)->text() + ".m4a&guid=" + guid;
    qDebug() << surl;
    QNetworkRequest request;
    request.setUrl(QUrl(surl));
    QNetworkReply *reply = NAM->get(request);
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    QByteArray BA = reply->readAll();
    //qDebug() <<  BA;
    QJsonDocument json = QJsonDocument::fromJson(BA);
    QString vkey = json.object().value("data").toObject().value("items").toArray()[0].toObject().value("vkey").toString();
    qDebug() << vkey;
    surl = "http://dl.stream.qqmusic.qq.com/C400" + ui->tableWidget->item(r,2)->text() + ".m4a?vkey=" + vkey + "&fromtag=30&guid=" + guid;
    qDebug() << surl;
    player->setMedia(QUrl(surl));
    player->play();
    ui->pushButton_play->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    ui->slider_progress->setMaximum(player->duration());
    // 专辑封面
    QNetworkAccessManager *NAMAlbumCover = new QNetworkAccessManager;
    QString albumID = ui->tableWidget->item(r,3)->text();
    QString album_cover = QString("https://y.gtimg.cn/music/photo_new/T002R500x500M000%1.jpg").arg(albumID);
    //QString albumpic = QString("http://imgcache.qq.com/music/photo/album/{albumid%100}/albumpic_{albumid}_0.jpg");
    NAMAlbumCover->get(QNetworkRequest(album_cover));
    connect(NAMAlbumCover, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyAlbumPixmap(QNetworkReply*)));
    // 歌词
    ui->textBrowser->setText("");
    desktopLyric->ui->label_lyric->setText("");
    ui->label_lyric->setText("");
    qDebug() << "歌词" << ui->tableWidget->item(r,4)->text();
    QNetworkAccessManager *NAMLirics = new QNetworkAccessManager;
    NAMLirics->get(QNetworkRequest(ui->tableWidget->item(r,4)->text()));
    connect(NAMLirics, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyLyrics(QNetworkReply*)));
}

void MainWindow::on_pushButton_play_clicked()
{
    if (player->state() == QMediaPlayer::PlayingState) {
        player->pause();
        ui->pushButton_play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    } else if (player->state() == QMediaPlayer::PausedState) {
        player->play();
        ui->pushButton_play->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    } else if (player->state() == QMediaPlayer::StoppedState) {
        player->play();
        ui->pushButton_play->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    }
}

void MainWindow::on_pushButton_skipf_clicked()
{
    if (ui->tableWidget->currentRow() < ui->tableWidget->rowCount()-1) {
        ui->tableWidget->setCurrentCell(ui->tableWidget->currentRow()+1,0);
        playSong(ui->tableWidget->currentRow(),0);
    }
}

void MainWindow::on_pushButton_skipb_clicked()
{
    if (ui->tableWidget->currentRow() > 0) {
        ui->tableWidget->setCurrentCell(ui->tableWidget->currentRow()-1,0);
        playSong(ui->tableWidget->currentRow(),0);
    }
}

void MainWindow::positionChange(qint64 p)
{
    ui->slider_progress->setValue(p);

    // 歌词选行
    int hl;
    QTime t(0,0,0);
    t = t.addMSecs(p);
    // 非最后一句
    for (int i=0; i<lyrics.size()-1; i++) {
        //qDebug() << t << lyrics.at(i).time;
        if (t>lyrics.at(i).time && t<lyrics.at(i+1).time) {
            if (desktopLyric->isHidden()) {
                ui->label_lyric->setText(lyrics.at(i).sentence);
            } else {
                ui->label_lyric->setText("");
                desktopLyric->ui->label_lyric->setText(lyrics.at(i).sentence);
            }
            hl = i;
            break;
        }
    }
    //最后一句
    if (lyrics.size()>0) {
        int j = lyrics.size()-1;
        if (t>lyrics.at(j).time) {
            if (desktopLyric->isHidden()) {
                ui->label_lyric->setText(lyrics.at(j).sentence);
            } else {
                ui->label_lyric->setText("");
                desktopLyric->ui->label_lyric->setText(lyrics.at(j).sentence);
            }
            hl = j;
        }
    }
    for (int a=0; a<lyrics.size(); a++) {
        QTextCursor cursor(ui->textBrowser->document()->findBlockByLineNumber(a));
        QTextBlockFormat TBF = cursor.blockFormat();
        TBF.setForeground(QBrush(Qt::black));
        //TBF.setBackground(QBrush(Qt::transparent));
        TBF.clearBackground();
        cursor.setBlockFormat(TBF);
    }
    if (lyrics.size()>0) {
        QTextCursor cursor1(ui->textBrowser->document()->findBlockByLineNumber(hl));
        QTextBlockFormat TBF1 = cursor1.blockFormat();
        TBF1.setForeground(QBrush(Qt::green));
        if (isFullScreen()) {
            TBF1.setBackground(QBrush(QColor(255,255,255,80)));
        } else {
            TBF1.setBackground(QBrush(Qt::yellow));
        }
        cursor1.setBlockFormat(TBF1);
        ui->textBrowser->setTextCursor(cursor1);
    }
}

void MainWindow::durationChange(qint64 d)
{
    ui->slider_progress->setMaximum(d);
}

void MainWindow::setMPPosition()
{
    player->setPosition(ui->slider_progress->value());
}

void MainWindow::setSTime(int v)
{
    QTime t(0,0,0);
    t = t.addMSecs(v);
    QString sTimeElapse = t.toString("hh:mm:ss");
    t.setHMS(0,0,0);
    t = t.addMSecs(player->duration());
    QString sTimeTotal = t.toString("hh:mm:ss");
    ui->label_time->setText(sTimeElapse + " / " + sTimeTotal);
}

void MainWindow::stateChange(QMediaPlayer::State state)
{
    qDebug() << state;
    if (state == QMediaPlayer::StoppedState) {
        ui->pushButton_play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
//        if(ui->tableWidget->currentRow() < ui->tableWidget->rowCount()-1){
//            ui->tableWidget->setCurrentCell(ui->tableWidget->currentRow()+1,0);
//            playSong(ui->tableWidget->currentRow(),0);
//        }
    }
}

void MainWindow::on_pushButton_download_clicked()
{
    if (ui->tableWidget->currentRow() != -1) {
        ui->pushButton_download->setEnabled(false);
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
        QString filepath = downloadPath + "/" + filename;
        //qDebug() <<  "path -> " + filepath;
        QFile file(filepath);
        file.open(QIODevice::WriteOnly);
        file.write(reply->readAll());
        file.close();
        //ui->pushButton_download->setText("↓");
        ui->pushButton_download->setEnabled(true);
    }
}

void MainWindow::updateProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    //ui->pushButton_download->setText(QString("%1%").arg(bytesReceived*100/bytesTotal));
    float p = (float)bytesReceived/bytesTotal;
    ui->pushButton_download->setStyleSheet(QString("background-color: qlineargradient(spread:pad, x1:0, y1:1, x2:0, y2:0,"
                                                   "stop:0 rgba(48, 194, 124, 255), stop:%1 rgba(48, 194, 124, 255),"
                                                   "stop:%2 rgba(255, 255, 255, 255), stop:1 rgba(255, 255, 255, 255));")
                                      .arg(p-0.001)
                                      .arg(p));
    qDebug() << p << ui->pushButton_download->styleSheet();
}

void MainWindow::on_pushButton_pageLast_clicked()
{
    if (ui->lineEdit_page->text().toInt() > 1) {
        page = page - 1;
        ui->lineEdit_page->setText(QString::number(ui->lineEdit_page->text().toInt()-1));
        search();
    }
}

void MainWindow::on_pushButton_pageNext_clicked()
{
    if (ui->lineEdit_page->text().toInt() < 51) {
        ui->lineEdit_page->setText(QString::number(ui->lineEdit_page->text().toInt() + 1));
        search();
    }
}

void MainWindow::on_pushButton_lyric_clicked()
{
    if (desktopLyric->isHidden()) {
        if (desktopLyric->x()>QApplication::desktop()->width() || desktopLyric->y()>QApplication::desktop()->height()) {
            desktopLyric->move((QApplication::desktop()->width()-desktopLyric->width())/2, QApplication::desktop()->height()-desktopLyric->height());
        }
        desktopLyric->show();
    } else {
        desktopLyric->hide();
    }
}

void MainWindow::on_action_settings_triggered()
{
    dialog_settings = new QDialog(this);
    dialog_settings->setWindowTitle("设置");
    dialog_settings->setFixedSize(300,200);
    QVBoxLayout *vbox = new QVBoxLayout;
    QHBoxLayout *hbox = new QHBoxLayout;
    QLabel *label = new QLabel("歌词");
    hbox->addWidget(label);
    QPushButton *pushButton_font = new QPushButton;
    QString sfont = desktopLyric->ui->label_lyric->font().family() + "," + QString::number(desktopLyric->ui->label_lyric->font().pointSize()) + "," + desktopLyric->ui->label_lyric->font().weight() + "," + desktopLyric->ui->label_lyric->font().italic();
    pushButton_font->setText(sfont);
    connect(pushButton_font,SIGNAL(pressed()),this,SLOT(chooseFont()));
    pushButton_font->setFocusPolicy(Qt::NoFocus);
    hbox->addWidget(pushButton_font);
    pushButton_fontcolor = new QPushButton;
    pushButton_fontcolor->setText("■");
    QPalette plt = desktopLyric->ui->label_lyric->palette();
    QBrush brush = plt.color(QPalette::WindowText);
    pushButton_fontcolor->setStyleSheet("color:" + brush.color().name());
    connect(pushButton_fontcolor,SIGNAL(pressed()),this,SLOT(chooseFontColor()));
    pushButton_fontcolor->setFocusPolicy(Qt::NoFocus);
    hbox->addWidget(pushButton_fontcolor);
    vbox->addLayout(hbox);
    hbox = new QHBoxLayout;
    label = new QLabel("保存路径");
    hbox->addWidget(label);
    lineEditDownloadPath = new QLineEdit;
    downloadPath = readSettings(QDir::currentPath() + "/config.ini", "config", "DownloadPath");
    if(downloadPath==""){
        lineEditDownloadPath->setText(QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first());
    }else{
        lineEditDownloadPath->setText(downloadPath);
    }
    QAction *action_browse = new QAction(this);
    action_browse->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
    connect(action_browse,SIGNAL(triggered(bool)),this,SLOT(chooseDownloadPath()));
    lineEditDownloadPath->addAction(action_browse,QLineEdit::TrailingPosition);
    hbox->addWidget(lineEditDownloadPath);
    //QPushButton *pushButton_downloadPath = new QPushButton("选择路径");
    //connect(pushButton_downloadPath,SIGNAL(pressed()),this,SLOT(chooseDownloadPath()));
    //pushButton_downloadPath->setFocusPolicy(Qt::NoFocus);
    //hbox->addWidget(pushButton_downloadPath);
    vbox->addLayout(hbox);
    dialog_settings->setLayout(vbox);
    dialog_settings->show();
}

void MainWindow::chooseFont()
{
    qDebug() << "label_before" << desktopLyric->ui->label_lyric->size();
    bool ok;
    QFont font = QFontDialog::getFont(&ok, desktopLyric->ui->label_lyric->font(), this, "选择字体");
    if(ok){
       desktopLyric->ui->label_lyric->setFont(font);
       QString sfont = font.family() + "," + QString::number(font.pointSize()) + "," + font.weight() + "," + font.italic();
       writeSettings(QDir::currentPath() + "/config.ini", "config", "Font", sfont);
       desktopLyric->ui->label_lyric->adjustSize();
       qDebug() << "label_after" << desktopLyric->ui->label_lyric->size();
       desktopLyric->resize(desktopLyric->ui->label_lyric->size());
       qDebug() << "window" << desktopLyric->size();
    }
}

void MainWindow::chooseFontColor()
{
    QPalette plt = desktopLyric->ui->label_lyric->palette();
    QBrush brush = plt.color(QPalette::WindowText);
    QColor color = QColorDialog::getColor(brush.color(), this);
    if(color.isValid()){
        plt.setColor(QPalette::WindowText, color);
        desktopLyric->ui->label_lyric->setPalette(plt);
        //plt.setColor(QPalette::ButtonText, color);
        //pushButton_fontcolor->setPalette(plt);
        pushButton_fontcolor->setStyleSheet("color:" + color.name());
        writeSettings(QDir::currentPath() + "/config.ini", "config", "LyricFontColor", color.name());
    }
}

QString MainWindow::readSettings(QString path, QString group, QString key)
{
    QSettings setting(path, QSettings::IniFormat);
    setting.beginGroup(group);
    QString value = setting.value(key).toString();
    return value;
}

void MainWindow::writeSettings(QString path, QString group, QString key, QString value)
{
    QSettings *config = new QSettings(path, QSettings::IniFormat);
    config->beginGroup(group);
    config->setValue(key, value);
    config->endGroup();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    player->stop();
    QCoreApplication::exit();
}

void MainWindow::replyAlbumPixmap(QNetworkReply *reply)
{
    pixmap_cover.loadFromData(reply->readAll());
    ui->pushButton_cover->setIcon(QIcon(pixmap_cover));
    pixmap_cover.save(QDir::currentPath() + "/cover.jpg");
}

void MainWindow::replyLyrics(QNetworkReply *reply)
{
    lyrics.clear();
    QByteArray BAReply = reply->readAll();
    if(BAReply.indexOf("GB2312")==-1){
        ui->textBrowser->setText(BAReply);
    }else{
        QString lrc = QTextCodec::codecForName("GBK")->toUnicode(BAReply);
        lrc = lrc.mid(lrc.indexOf("<![CDATA[")+9, lrc.indexOf("]]>")-lrc.indexOf("<![CDATA[")-9).replace("&apos;","'");
        QStringList line = lrc.split("\n");
        for(int i=0;i<line.size();i++){
            if(line.at(i).contains("]") && !line.at(i).contains("ti:") && !line.at(i).contains("ar:") && !line.at(i).contains("al:") && !line.at(i).contains("by:") && !line.at(i).contains("offset:")){
                QStringList strlist = line.at(i).split("]");
                Lyric lyric;
                lyric.time = QTime::fromString(strlist.at(0).mid(1,8) + "0", "mm:ss.zzz");
                lyric.sentence = strlist.at(1);
                lyrics.append(lyric);
            }
        }
        for(int i=0; i<lyrics.size(); i++){
            ui->textBrowser->insertPlainText(lyrics.at(i).sentence + "\n");
        }
        QTextCursor cursor = ui->textBrowser->textCursor();
        cursor.setPosition(0,QTextCursor::MoveAnchor);
        ui->textBrowser->setTextCursor(cursor);
        //cursor.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor);
    }
}

void MainWindow::chooseDownloadPath()
{
    downloadPath = QFileDialog::getExistingDirectory(dialog_settings,"保存路径",downloadPath, QFileDialog::ShowDirsOnly |QFileDialog::DontResolveSymlinks);
    if(downloadPath != ""){
        lineEditDownloadPath->setText(downloadPath);
        writeSettings(QDir::currentPath() + "/config.ini", "config", "DownloadPath", downloadPath);
    }
}

void MainWindow::on_pushButton_cover_clicked()
{
    label_cover->setWindowTitle(ui->label_SongSinger->text());
    label_cover->resize(pixmap_cover.size());
    label_cover->setPixmap(pixmap_cover);
    label_cover->move((QApplication::desktop()->width()-label_cover->width())/2,(QApplication::desktop()->height()-label_cover->height())/2);
    label_cover->show();
}

void MainWindow::on_pushButton_fullscreen_clicked()
{
    if(isFullScreen()){
        exitFullscreen();
    }else{
        enterFullscreen();
    }
}

void MainWindow::enterFullscreen()
{
    showFullScreen();
    setStyleSheet("background-color:black;");
    ui->textBrowser->setStyleSheet("QTextBrowser{border-image:url(cover.jpg);}");
    ui->textBrowser->selectAll();
    ui->textBrowser->setAlignment(Qt::AlignCenter);
    ui->textBrowser->moveCursor(QTextCursor::Start,QTextCursor::MoveAnchor);
    ui->textBrowser->zoomIn(40);
    ui->menuBar->hide();
    ui->navPanel->hide();
    ui->searchBar->hide();
    ui->tableWidget->hide();
    ui->pushButton_cover->hide();
}

void MainWindow::exitFullscreen()
{
    if(isFullScreen()){
        showNormal();
        setStyleSheet(sstyle);
        ui->textBrowser->setStyleSheet("");
        ui->textBrowser->selectAll();
        ui->textBrowser->setAlignment(Qt::AlignLeft);
        ui->textBrowser->moveCursor(QTextCursor::Start,QTextCursor::MoveAnchor);
        ui->textBrowser->zoomOut(40);
        ui->menuBar->show();
        ui->navPanel->show();
        ui->searchBar->show();
        ui->tableWidget->show();
        ui->pushButton_cover->show();
    }
}