#ifndef FORM_H
#define FORM_H

#include <QWidget>

namespace Ui {
class Form;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = 0);
    ~Form();
    Ui::Form *ui;

private:
    QPoint m_point;
    bool m_bPressed;
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void enterEvent(QEvent*);
    void leaveEvent(QEvent*);
    QString readSettings(QString path, QString key);
    void writeSettings(QString path, QString key, QString value);

signals:
    void pushButton_lyric_toggle();
    //void pull_settings();

private slots:
    void on_pushButton_close_clicked();
    //void on_pushButton_set_clicked();
};

#endif // FORM_H
