#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QMediaPlayer>
#include <QStandardItemModel>

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
    void getKey();
    QMediaPlayer *player;
    int page;

private slots:
    void on_action_directory_triggered();
    void on_action_about_triggered();
    void on_pushButton_play_clicked();
    void on_pushButton_skipf_clicked();
    void on_pushButton_skipb_clicked();
    void on_pushButton_download_clicked();
    void on_pushButton_pageLast_clicked();
    void on_pushButton_pageNext_clicked();
    void initSearch();
    void search();
    void playSong(int,int);
    void positionChange(qint64);
    void durationChange(qint64);
    void setSTime(int);
    void setMPPosition();
    void stateChange(QMediaPlayer::State);
    void updateProgress(qint64, qint64);
};

#endif // MAINWINDOW_H
