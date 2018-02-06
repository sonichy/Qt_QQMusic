#include "form.h"
#include "ui_form.h"
#include <QMouseEvent>
#include <QDebug>
#include <QDir>
#include <QSettings>

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
    ui->pushButton_close->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
    ui->pushButton_close->hide();
    ui->pushButton_set->hide();
    setAttribute(Qt::WA_TranslucentBackground,true);
    setWindowFlags(Qt::WindowStaysOnTopHint);
    setWindowFlags(Qt::FramelessWindowHint);
    setWindowFlags(Qt::X11BypassWindowManagerHint);
}

Form::~Form()
{
    delete ui;
}

void Form::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_bPressed = true;
        m_point = event->pos();
    }
}

void Form::mouseMoveEvent(QMouseEvent *event)
{
    if(m_bPressed)
        move(event->pos() - m_point + pos());
}

void Form::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_bPressed = false;
    writeSettings(QDir::currentPath() + "/config.ini", "LyricX", QString::number(x()));
    writeSettings(QDir::currentPath() + "/config.ini", "LyricY", QString::number(y()));
}

void Form::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    setStyleSheet("background-color: rgba(255,255,255,30);");
    ui->pushButton_close->show();
    ui->pushButton_set->show();
}

void Form::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    setStyleSheet("");
    ui->pushButton_close->hide();
    ui->pushButton_set->hide();
}

QString Form::readSettings(QString path, QString key)
{
    QSettings setting(path, QSettings::IniFormat);
    setting.beginGroup("config");
    QString value = setting.value(key).toString();
    return value;
}

void Form::writeSettings(QString path, QString key, QString value)
{
    QSettings *config = new QSettings(path, QSettings::IniFormat);
    config->beginGroup("config");
    config->setValue(key, value);
    config->endGroup();
}

void Form::on_pushButton_close_clicked()
{
    hide();
    emit pushButton_lyric_toggle();
}

//void Form::on_pushButton_set_clicked()
//{
//    emit pull_settings();
//}
