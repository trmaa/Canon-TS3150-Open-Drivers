#include <QApplication>
#include <QFileDialog>
#include <QProcess>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>
#include <QPushButton>

class PrinterAutomation : public QObject {
    Q_OBJECT
public:
    PrinterAutomation(QObject *parent = nullptr) : QObject(parent), currentFileIndex(0) {}

    void startPrinting(const QStringList &files) {
        if (files.isEmpty()) {
            QMessageBox::information(nullptr, "Información", "No se seleccionaron archivos para imprimir.");
            QApplication::quit();
            return;
        }

        pdfFiles = files;
        currentFileIndex = 0;
        printNextFile();
    }

private slots:
    void printNextFile() {
        if (currentFileIndex >= pdfFiles.size()) {
            QMessageBox::information(nullptr, "Completado", "Todos los archivos han sido enviados a imprimir.");
            QApplication::quit();
            return;
        }

        QString file = pdfFiles[currentFileIndex];
        qDebug() << "Imprimiendo:" << file;

        // Paso 1: Enviar a imprimir
        QProcess::startDetached("lp", QStringList() << file);

        // Paso 2: Verificar cola de impresión después de un breve retraso
        QTimer::singleShot(5000, this, &PrinterAutomation::checkPrintQueue);
    }

    void checkPrintQueue() {
        QProcess *process = new QProcess(this);
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &PrinterAutomation::handleQueueCheck);

        process->start("lpstat", QStringList() << "-o");
    }

    void handleQueueCheck(int exitCode, QProcess::ExitStatus exitStatus) {
        Q_UNUSED(exitCode);
        Q_UNUSED(exitStatus);
        
        QProcess *process = qobject_cast<QProcess*>(sender());
        if (!process) return;

        QString output = process->readAllStandardOutput();
        process->deleteLater();

        // Analizar la salida para ver si el trabajo está en cola
        if (output.contains("canon")) { // Ajusta este filtro según tu impresora
            // Mostrar mensaje con botón para continuar
            QMessageBox msgBox;
            msgBox.setWindowTitle("Acción requerida");
            msgBox.setText("Por favor:\n"
                          "1. En la impresora Canon T-3150, presiona cancelar cuando aparezcan las letritas\n"
                          "2. Cuando aparezca el símbolo de carga, presiona imprimir\n"
                          "3. Luego de que imprima, haz clic en Continuar");
            
            QPushButton *continueButton = msgBox.addButton("Continuar", QMessageBox::AcceptRole);
            msgBox.exec();

            if (msgBox.clickedButton() == continueButton) {
                currentFileIndex++;
                printNextFile();
            }
        } else {
            // Trabajo no está en cola, cancelar y reintentar
            QMessageBox::warning(nullptr, "Error", 
                "El trabajo no apareció en la cola de impresión. Se cancelará y se reintentará.");

            // Cancelar el trabajo
            QProcess::startDetached("cancel", QStringList() << "-a");

            // Reintentar el mismo archivo
            QTimer::singleShot(2000, this, &PrinterAutomation::printNextFile);
        }
    }

private:
    QStringList pdfFiles;
    int currentFileIndex;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Configurar el diálogo de selección de archivos
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter("PDF Files (*.pdf)");
    dialog.setWindowTitle("Seleccionar archivos PDF para imprimir");

    if (dialog.exec()) {
        QStringList files = dialog.selectedFiles();
        PrinterAutomation printer;
        printer.startPrinting(files);
    } else {
        return 0;
    }

    return app.exec();
}

#include "main.moc"
