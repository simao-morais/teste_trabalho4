#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <httplib.h>
#include <jsoncpp/json/json.h>

class ClientWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();

private slots:
    void selectFile();
    void processFile();

private:
    // Widgets da interface
    QLabel* labelHost;
    QLineEdit* editHost;
    QLabel* labelPort;
    QLineEdit* editPort;
    QLabel* labelFile;
    QLineEdit* editFile;
    QPushButton* buttonSelectFile;
    QPushButton* buttonProcess;
    QTextEdit* textOutput;

    // Lógica do cliente
    std::string lerArquivo(const std::string& nomeArquivo);
    Json::Value enviarArquivo(const std::string& conteudo, const std::string& host, int port);
    void exibirResultado(const Json::Value& resultado);
};

#endif // CLIENTWINDOW_H