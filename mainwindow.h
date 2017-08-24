#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "form.h"
#include "ui_form.h"
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QMediaPlayer>
#include <QStandardItemModel>
#include <QLabel>
#include <QPushButton>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *NAM;
    QString key,downloadDir;
    QMediaPlayer *player;
    QLabel *label_lyric;
    Form *DesktopLyric;

    QPushButton *pushButton_fontcolor;
    int page;
    struct Lyric{
        QTime time;
        QString sentence;
    };
    QList<Lyric> lyrics;
    void getKey();
    QString readSettings(QString path, QString key);
    void writeSettings(QString path, QString key, QString value);
    void closeEvent(QCloseEvent *event);

private slots:
    void on_action_directory_triggered();
    void on_action_settings_triggered();
    void on_action_about_triggered();
    void on_pushButton_pageLast_clicked();
    void on_pushButton_pageNext_clicked();
    void on_pushButton_play_clicked();
    void on_pushButton_skipf_clicked();
    void on_pushButton_skipb_clicked();
    void on_pushButton_lyric_clicked();
    void on_pushButton_download_clicked();
    void initSearch();
    void search();
    void playSong(int,int);
    void positionChange(qint64);
    void durationChange(qint64);
    void setSTime(int);
    void setMPPosition();
    void stateChange(QMediaPlayer::State);
    void updateProgress(qint64, qint64);
    void chooseFont();
    void chooseFontColor();
    void replyAlbumPixmap(QNetworkReply *reply);
    void replyLyrics(QNetworkReply *reply);
};

#endif // MAINWINDOW_H
