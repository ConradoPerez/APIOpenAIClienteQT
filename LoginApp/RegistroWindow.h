#ifndef REGISTROWINDOW_H
#define REGISTROWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>

namespace Ui {
class RegistroWindow;
}

class RegistroWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit RegistroWindow(QWidget *parent = nullptr);
    ~RegistroWindow();

private slots:
    void registrarUsuario();

private:
    Ui::RegistroWindow *ui;
    QNetworkAccessManager *manager;
};

#endif // REGISTROWINDOW_H
