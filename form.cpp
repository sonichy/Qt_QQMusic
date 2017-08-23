#include "form.h"
#include "ui_form.h"
#include <QMouseEvent>
#include <QDebug>

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
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
    if (event->button() == Qt::LeftButton)
    {
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
}

void Form::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    ui->label_lyric->setStyleSheet("border:1px solid gray;");
}

void Form::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    ui->label_lyric->setStyleSheet("");
}
