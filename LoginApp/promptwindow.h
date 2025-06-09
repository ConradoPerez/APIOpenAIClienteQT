#ifndef PROMPTWINDOW_H
#define PROMPTWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace Ui {
class PromptWindow;
}

class PromptWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PromptWindow(QWidget *parent = nullptr);
    ~PromptWindow();

private slots:
    void procesarTexto();
    void exportarResultado();
    void obtenerHistorial();


private:
    Ui::PromptWindow *ui;
    QNetworkAccessManager *manager;
};

#endif // PROMPTWINDOW_H
