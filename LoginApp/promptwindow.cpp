#include "promptwindow.h"
#include "ui_promptwindow.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QPrinter>
#include <QTextDocument>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

PromptWindow::PromptWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PromptWindow)
    , manager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);
    setWindowTitle("Procesador de Tareas Técnico");

    // Conecta botones a sus respectivas funciones
    connect(ui->btnProcesar, &QPushButton::clicked, this, &PromptWindow::procesarTexto);
    connect(ui->btnExportar, &QPushButton::clicked, this, &PromptWindow::exportarResultado);

    // Al iniciar, carga el historial SOLO en la tabla
    obtenerHistorial();
}

PromptWindow::~PromptWindow() {
    delete ui;
}

void PromptWindow::procesarTexto() {
    QString descripcion = ui->inputPrompt->toPlainText();

    QUrl url("http://18.118.208.12:8000/procesar_texto");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["descripcion"] = descripcion;

    QNetworkReply *reply = manager->post(request, QJsonDocument(json).toJson());

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
            QJsonArray tareas = response.array();

            QString resultado;
            for (const QJsonValue &tarea : tareas) {
                QJsonObject obj = tarea.toObject();
                QString nombre = obj["tarea"].toString();
                QString categoria = obj["categoria"].toString();
                int orden = obj["orden"].toInt();

                QString color;
                QString etiqueta;
                if (categoria == "frontend") {
                    color = "#1E90FF"; etiqueta = "[Frontend]";
                } else if (categoria == "backend") {
                    color = "#FF8C00"; etiqueta = "[Backend]";
                } else if (categoria == "base de datos") {
                    color = "#228B22"; etiqueta = "[Base de Datos]";
                } else if (categoria == "testing") {
                    color = "#8A2BE2"; etiqueta = "[Testing]";
                } else {
                    color = "#808080"; etiqueta = "[Otro]";
                }
                resultado += QString("<p><b style='color:%1;'>%2</b> %3 - Orden %4</p>")
                             .arg(color, etiqueta, nombre, QString::number(orden));
            }

            // Muestra SOLO el resultado en el QTextBrowser (área de resultados)
            ui->resultadoTexto->setHtml(resultado);
        } else {
            QMessageBox::warning(this, "Error", reply->errorString());
        }

        reply->deleteLater();
        // Refresca SOLO la tabla de historial
        obtenerHistorial();
    });
}

void PromptWindow::exportarResultado() {
    QString texto = ui->resultadoTexto->toPlainText();
    if (texto.isEmpty()) {
        QMessageBox::warning(this, "Exportar", "No hay texto para exportar.");
        return;
    }

    QString archivo = QFileDialog::getSaveFileName(this, "Guardar como...", "", "CSV (*.csv);;PDF (*.pdf)");

    if (archivo.endsWith(".csv")) {
        QFile file(archivo);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "Tarea,Categoría,Orden\n";
            QStringList lineas = texto.split('\n');
            for (const QString &linea : lineas) {
                if (linea.trimmed().isEmpty()) continue;
                QRegularExpression re(R"(• (.+) \((.+)\) - Orden (\d+))");
                QRegularExpressionMatch match = re.match(linea);
                if (match.hasMatch()) {
                    out << QString("\"%1\",\"%2\",%3\n")
                           .arg(match.captured(1), match.captured(2), match.captured(3));
                }
            }
            file.close();
            QMessageBox::information(this, "Exportar", "Archivo CSV guardado con éxito.");
        }
    }
    else if (archivo.endsWith(".pdf")) {
        QTextDocument doc;
        doc.setPlainText(texto);
        QPrinter printer;
        printer.setOutputFileName(archivo);
        printer.setOutputFormat(QPrinter::PdfFormat);
        doc.print(&printer);
        QMessageBox::information(this, "Exportar", "Archivo PDF guardado con éxito.");
    }
}

void PromptWindow::obtenerHistorial() {
    QUrl url("http://18.118.208.12:8000/historial_logs");
    QNetworkRequest request(url);
    QNetworkReply *reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonArray logs = doc.array();

            ui->tablaHistorial->setColumnCount(3);
            ui->tablaHistorial->setHorizontalHeaderLabels({"Respuesta", "Prompt", "Fecha"});
            ui->tablaHistorial->setRowCount(logs.size());

            ui->tablaHistorial->setAlternatingRowColors(true);
            ui->tablaHistorial->setSelectionBehavior(QAbstractItemView::SelectRows);
            ui->tablaHistorial->setSelectionMode(QAbstractItemView::SingleSelection);
            ui->tablaHistorial->setSortingEnabled(true);
            ui->tablaHistorial->setWordWrap(true);
            ui->tablaHistorial->setTextElideMode(Qt::ElideNone);
            ui->tablaHistorial->horizontalHeader()->setVisible(true);
            ui->tablaHistorial->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
            ui->tablaHistorial->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
            ui->tablaHistorial->horizontalHeader()->setStretchLastSection(false);

            ui->tablaHistorial->setStyleSheet(R"(
                QTableWidget {
                    border: none;
                    gridline-color: #dcdcdc;
                    font-size: 12px;
                    alternate-background-color: #f0f0f0;
                    background-color: white;
                }
                QHeaderView::section {
                    background-color: #2d89ef;
                    color: white;
                    padding: 5px;
                    font-weight: bold;
                    border: 1px solid #dcdcdc;
                }
            )");

            for (int i = 0; i < logs.size(); ++i) {
                QJsonObject obj = logs[i].toObject();

                for (int j = 0; j < 3; ++j) {
                    QString contenido;
                    switch (j) {
                        case 0: contenido = obj["respuesta"].toString(); break;
                        case 1: contenido = obj["prompt"].toString(); break;
                        case 2: contenido = obj["timestamp"].toString(); break;
                    }
                    QTableWidgetItem *item = new QTableWidgetItem(contenido);
                    item->setTextAlignment(Qt::AlignLeft | Qt::AlignTop);
                    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                    if (contenido.length() > 100)
                        item->setToolTip(contenido);

                    ui->tablaHistorial->setItem(i, j, item);
                }
                ui->tablaHistorial->resizeRowToContents(i);
            }

            ui->tablaHistorial->setColumnWidth(0, 300); // Respuesta
            ui->tablaHistorial->setColumnWidth(1, 200); // Prompt
            ui->tablaHistorial->setColumnWidth(2, 150); // Fecha
        } else {
            QMessageBox::warning(this, "Error", reply->errorString());
        }
        reply->deleteLater();
    });
}

