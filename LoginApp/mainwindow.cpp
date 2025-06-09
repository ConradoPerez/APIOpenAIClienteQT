// Inclusión de encabezados propios y librerías necesarias
#include "promptwindow.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "registrowindow.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QFile>
#include <QSettings>
#include <QDebug>

// Constructor de la ventana principal (login)
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , manager(new QNetworkAccessManager(this))  // Crea el gestor de solicitudes HTTP
{
    cargarEstilos();         // Carga el archivo de estilos (QSS)
    ui->setupUi(this);       // Inicializa la interfaz gráfica
    setWindowTitle("Login Procesador de Tareas Técnicas");

    // Leer preferencias guardadas del usuario con QSettings
    QSettings settings("MiEmpresa", "LoginApp");
    QString ultimoCorreo = settings.value("ultimoCorreo").toString();
    QString ultimaContrasena = settings.value("ultimaContrasena").toString();
    bool recordar = settings.value("recordarContrasena", false).toBool();

    // Rellenar los campos si el usuario eligió recordar los datos
    ui->lineCorreo->setText(ultimoCorreo);
    if (recordar) {
        ui->lineContrasena->setText(ultimaContrasena);
        ui->checkRecordar->setChecked(true);
    }

    // Conectar los botones de login y registro a sus respectivas funciones
    connect(ui->btnLogin, &QPushButton::clicked, this, &MainWindow::realizarLogin);
    connect(ui->btnRegistrarse, &QPushButton::clicked, this, [=]() {
        RegistroWindow *ventanaRegistro = new RegistroWindow(this);
        ventanaRegistro->show();
    });
}

// Destructor: libera memoria
MainWindow::~MainWindow()
{
    delete ui;
}

// Carga el archivo de estilos desde recursos (:/styles/styles.qss)
void MainWindow::cargarEstilos()
{
    QFile file(":/styles/styles.qss");  // Estilo embebido en recursos
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qApp->setStyleSheet(file.readAll());  // Aplica el estilo al app
        file.close();
    }
}

// Función que maneja el intento de login
void MainWindow::realizarLogin()
{
    // Obtiene los datos del formulario
    QString correo = ui->lineCorreo->text();
    QString contrasena = ui->lineContrasena->text();
    bool recordar = ui->checkRecordar->isChecked();

    // Prepara el request HTTP al endpoint /login
    QUrl url("http://18.118.208.12:8000/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Arma el cuerpo JSON con los datos ingresados
    QJsonObject json;
    json["correo"] = correo;
    json["contrasena"] = contrasena;

    // Envía la solicitud POST al backend
    QNetworkReply *reply = manager->post(request, QJsonDocument(json).toJson());

    // Procesa la respuesta
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            // Respuesta exitosa del servidor
            QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());
            QJsonObject obj = jsonResponse.object();
            QString mensaje = obj["mensaje"].toString();  // Mensaje opcional
            QString nombre = obj["nombre"].toString();    // Nombre del usuario (opcional)

            // Guarda preferencias si está marcado el checkbox
            QSettings settings("MiEmpresa", "LoginApp");
            settings.setValue("ultimoCorreo", correo);
            settings.setValue("recordarContrasena", recordar);
            if (recordar) {
                settings.setValue("ultimaContrasena", contrasena);
            } else {
                settings.remove("ultimaContrasena");
            }

            // Abre la ventana principal (PromptWindow) y cierra la de login
            PromptWindow *ventanaPrompt = new PromptWindow();
            ventanaPrompt->show();
            this->close();

        } else {
            // Error en la respuesta (fallo de conexión, usuario inválido, etc.)
            qDebug() << "Código de error:" << reply->error();
            qDebug() << "Mensaje de error:" << reply->errorString();
            qDebug() << "Respuesta del servidor:" << reply->readAll();

            QMessageBox::warning(this, "Error", "Fallo de conexión: " + reply->errorString());
        }

        reply->deleteLater();  // Libera la memoria del reply
    });
}
