#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void realizarLogin();          // Maneja el intento de login

private:
    void cargarEstilos();          // Aplica el archivo QSS si existe
    void cargarDatosGuardados();   // Carga correo y contraseña si fueron guardados

    Ui::MainWindow *ui;
    QNetworkAccessManager *manager;
};

#endif // MAINWINDOW_H
