// Inclusión de encabezados propios y librerías necesarias
#include "registrowindow.h"
#include "ui_registrowindow.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>

// Constructor de la ventana RegistroWindow
RegistroWindow::RegistroWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::RegistroWindow)
    , manager(new QNetworkAccessManager(this))  // Crea el gestor de solicitudes HTTP
{
    ui->setupUi(this);  // Inicializa la interfaz gráfica
    setWindowTitle("Registro de Usuario");  // Título de la ventana

    // Conecta el botón de registrar con la función que maneja el registro
    connect(ui->btnRegistrar, &QPushButton::clicked, this, &RegistroWindow::registrarUsuario);
}

// Destructor: libera recursos
RegistroWindow::~RegistroWindow()
{
    delete ui;
}

// Función que se ejecuta cuando se presiona el botón "Registrar"
void RegistroWindow::registrarUsuario()
{
    // Obtiene los datos del formulario
    QString nombre = ui->lineNombre->text();
    QString correo = ui->lineCorreo->text();
    QString contrasena = ui->lineContrasena->text();

    // Validación básica de campos vacíos
    if (nombre.isEmpty() || correo.isEmpty() || contrasena.isEmpty()) {
        QMessageBox::warning(this, "Validación", "Por favor completá todos los campos.");
        return;
    }

    // Define la URL del endpoint para el registro de usuario
    QUrl url("http://18.118.208.12:8000/registrarse");  // Reemplazá si cambia el servidor o endpoint
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Crea el cuerpo JSON con los datos del usuario
    QJsonObject json;
    json["nombre"] = nombre;
    json["correo"] = correo;
    json["contrasena"] = contrasena;

    // Envía la solicitud POST con los datos del usuario
    QNetworkReply *reply = manager->post(request, QJsonDocument(json).toJson());

    // Procesa la respuesta cuando termina la solicitud
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            // Registro exitoso
            QMessageBox::information(this, "Éxito", "Usuario registrado correctamente.");
            this->close();  // Cierra la ventana de registro
        } else {
            // Intenta obtener el mensaje de error desde la respuesta JSON
            QJsonDocument errorDoc = QJsonDocument::fromJson(reply->readAll());
            QString mensaje = errorDoc["detail"].toString();

            // Si hay mensaje personalizado lo muestra, si no muestra el mensaje del sistema
            QMessageBox::warning(this, "Error", mensaje.isEmpty() ? reply->errorString() : mensaje);
        }

        reply->deleteLater();  // Libera la memoria de la respuesta
    });
}
