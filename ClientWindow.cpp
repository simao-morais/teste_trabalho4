#include "ClientWindow.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QCoreApplication>
#include <fstream> // Add this line

ClientWindow::ClientWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Sistema Distribuído - Cliente");
    setFixedSize(500, 450);

    // Layout principal
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // --- Seção de Configuração ---
    QGroupBox *configGroup = new QGroupBox("Configuração do Servidor", this);
    mainLayout->addWidget(configGroup);
    QVBoxLayout *configLayout = new QVBoxLayout(configGroup);

    labelHost = new QLabel("Host do Mestre:", this);
    editHost = new QLineEdit("10.180.46.169", this);
    labelPort = new QLabel("Porta do Mestre:", this);
    editPort = new QLineEdit("8080", this);

    configLayout->addWidget(labelHost);
    configLayout->addWidget(editHost);
    configLayout->addWidget(labelPort);
    configLayout->addWidget(editPort);

    // --- Seção de Arquivo ---
    QGroupBox *fileGroup = new QGroupBox("Arquivo de Entrada", this);
    mainLayout->addWidget(fileGroup);
    QVBoxLayout *fileLayout = new QVBoxLayout(fileGroup);

    labelFile = new QLabel("Caminho do Arquivo:", this);
    editFile = new QLineEdit(this);
    buttonSelectFile = new QPushButton("Selecionar Arquivo...", this);
    buttonProcess = new QPushButton("Processar", this);

    fileLayout->addWidget(labelFile);
    fileLayout->addWidget(editFile);
    fileLayout->addWidget(buttonSelectFile);
    fileLayout->addWidget(buttonProcess);

    // --- Seção de Resultado ---
    QGroupBox *outputGroup = new QGroupBox("Resultado", this);
    mainLayout->addWidget(outputGroup);
    QVBoxLayout *outputLayout = new QVBoxLayout(outputGroup);

    textOutput = new QTextEdit(this);
    textOutput->setReadOnly(true);
    outputLayout->addWidget(textOutput);

    // Conecta os botões aos slots
    connect(buttonSelectFile, &QPushButton::clicked, this, &ClientWindow::selectFile);
    connect(buttonProcess, &QPushButton::clicked, this, &ClientWindow::processFile);
}

ClientWindow::~ClientWindow() {}

void ClientWindow::selectFile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Selecionar Arquivo de Texto", "", "Text Files (*.txt)");
    if (!fileName.isEmpty()) {
        editFile->setText(fileName);
    }
}

void ClientWindow::processFile() {
    try {
        std::string host = editHost->text().toStdString();
        int port = editPort->text().toInt();
        std::string nomeArquivo = editFile->text().toStdString();

        if (nomeArquivo.empty()) {
            QMessageBox::warning(this, "Erro", "Por favor, selecione um arquivo.");
            return;
        }

        textOutput->append("Lendo arquivo: " + QString::fromStdString(nomeArquivo));
        std::string conteudo = lerArquivo(nomeArquivo);

        textOutput->append("Enviando para o servidor mestre...");
        Json::Value resultado = enviarArquivo(conteudo, host, port);

        exibirResultado(resultado);

    } catch (const std::exception& e) {
        textOutput->append("<font color=\"red\">Erro: " + QString(e.what()) + "</font>");
        QMessageBox::critical(this, "Erro", e.what());
    }
}

std::string ClientWindow::lerArquivo(const std::string& nomeArquivo) {
    std::ifstream arquivo(nomeArquivo);
    if (!arquivo.is_open()) {
        throw std::runtime_error("Erro ao abrir arquivo: " + nomeArquivo);
    }
    
    std::string conteudo;
    std::string linha;
    while (std::getline(arquivo, linha)) {
        conteudo += linha + "\n";
    }
    
    return conteudo;
}

Json::Value ClientWindow::enviarArquivo(const std::string& conteudo, const std::string& host, int port) {
    httplib::Client client(host, port);
    
    Json::Value requestJson;
    requestJson["texto"] = conteudo;
    
    Json::StreamWriterBuilder builder;
    std::string jsonString = Json::writeString(builder, requestJson);
    
    auto resposta = client.Post("/processar", jsonString, "application/json");
    
    if (!resposta) {
        throw std::runtime_error("Erro na comunicação com o servidor mestre");
    }
    
    if (resposta->status != 200) {
        throw std::runtime_error("Erro do servidor: " + std::to_string(resposta->status));
    }
    
    Json::Value resultado;
    Json::Reader reader;
    if (!reader.parse(resposta->body, resultado)) {
        throw std::runtime_error("Erro ao parsear resposta JSON");
    }
    
    return resultado;
}

void ClientWindow::exibirResultado(const Json::Value& resultado) {
    textOutput->append("\n<font color=\"blue\">=== RESULTADO ===</font>");
    textOutput->append("Quantidade de letras: " + QString::number(resultado["letras"].asInt()));
    textOutput->append("Quantidade de números: " + QString::number(resultado["numeros"].asInt()));
    textOutput->append("Total de caracteres processados: " 
                      + QString::number(resultado["letras"].asInt() + resultado["numeros"].asInt()));
    textOutput->append("<font color=\"blue\">================</font>\n");
}