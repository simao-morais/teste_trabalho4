#include <iostream>
#include <thread>
#include <future>
#include <httplib.h>
#include <jsoncpp/json/json.h>

class Mestre {
private:
    httplib::Server servidor;
    std::string escravo1Host = "escravo1";
    std::string escravo2Host = "escravo2";
    int escravo1Port = 8081; // Porta para o Escravo de letras
    int escravo2Port = 8082; // Porta para o Escravo de números
    
public:
    Mestre() {
        configurarRotas();
    }
    
    void configurarRotas() {
        // Rota para receber arquivos do cliente
        servidor.Post("/processar", [this](const httplib::Request& req, httplib::Response& res) {
            this->processarTexto(req, res);
        });
        
        // Rota de health check
        servidor.Get("/health", [](const httplib::Request&, httplib::Response& res) {
            Json::Value resposta;
            resposta["status"] = "ok";
            resposta["servico"] = "mestre";
            
            Json::StreamWriterBuilder builder;
            res.set_content(Json::writeString(builder, resposta), "application/json");
        });
    }
    
    bool verificarSaudeEscravo(const std::string& host, int port) {
        httplib::Client client(host, port);
        auto resposta = client.Get("/health");
        
        if (!resposta || resposta->status != 200) {
            std::cout << "Escravo " << host << " não está disponível!" << std::endl;
            return false;
        }
        
        std::cout << "Escravo " << host << " está saudável" << std::endl;
        return true;
    }
    
    std::future<int> enviarParaEscravoLetras(const std::string& texto) {
        return std::async(std::launch::async, [this, texto]() -> int {
            if (!verificarSaudeEscravo(escravo1Host, escravo1Port)) {
                throw std::runtime_error("Escravo1 (letras) não disponível");
            }
            
            httplib::Client client(escravo1Host, escravo1Port);
            
            Json::Value requestJson;
            requestJson["texto"] = texto;
            
            Json::StreamWriterBuilder builder;
            std::string jsonString = Json::writeString(builder, requestJson);
            
            auto resposta = client.Post("/letras", jsonString, "application/json");
            
            if (!resposta || resposta->status != 200) {
                throw std::runtime_error("Erro na comunicação com Escravo1");
            }
            
            Json::Value resultado;
            Json::Reader reader;
            if (!reader.parse(resposta->body, resultado)) {
                throw std::runtime_error("Erro ao parsear resposta do Escravo1");
            }
            
            return resultado["quantidade"].asInt();
        });
    }
    
    std::future<int> enviarParaEscravoNumeros(const std::string& texto) {
        return std::async(std::launch::async, [this, texto]() -> int {
            if (!verificarSaudeEscravo(escravo2Host, escravo2Port)) {
                throw std::runtime_error("Escravo2 (números) não disponível");
            }
            
            httplib::Client client(escravo2Host, escravo2Port);
            
            Json::Value requestJson;
            requestJson["texto"] = texto;
            
            Json::StreamWriterBuilder builder;
            std::string jsonString = Json::writeString(builder, requestJson);
            
            auto resposta = client.Post("/numeros", jsonString, "application/json");
            
            if (!resposta || resposta->status != 200) {
                throw std::runtime_error("Erro na comunicação com Escravo2");
            }
            
            Json::Value resultado;
            Json::Reader reader;
            if (!reader.parse(resposta->body, resultado)) {
                throw std::runtime_error("Erro ao parsear resposta do Escravo2");
            }
            
            return resultado["quantidade"].asInt();
        });
    }
    
    void processarTexto(const httplib::Request& req, httplib::Response& res) {
        try {
            // Parse do JSON recebido
            Json::Value requestJson;
            Json::Reader reader;
            if (!reader.parse(req.body, requestJson)) {
                res.status = 400;
                res.set_content("{\"erro\": \"JSON inválido\"}", "application/json");
                return;
            }
            
            std::string texto = requestJson["texto"].asString();
            std::cout << "Processando texto de " << texto.length() << " caracteres..." << std::endl;
            
            // Dispara as duas threads em paralelo
            auto futureLetras = enviarParaEscravoLetras(texto);
            auto futureNumeros = enviarParaEscravoNumeros(texto);
            
            // Aguarda os resultados
            int quantidadeLetras = futureLetras.get();
            int quantidadeNumeros = futureNumeros.get();
            
            // Constrói resposta consolidada
            Json::Value resposta;
            resposta["letras"] = quantidadeLetras;
            resposta["numeros"] = quantidadeNumeros;
            resposta["timestamp"] = std::time(nullptr);
            
            Json::StreamWriterBuilder builder;
            res.set_content(Json::writeString(builder, resposta), "application/json");
            
            std::cout << "Processamento concluído: " << quantidadeLetras 
                     << " letras, " << quantidadeNumeros << " números" << std::endl;
            
        } catch (const std::exception& e) {
            std::cerr << "Erro no processamento: " << e.what() << std::endl;
            
            Json::Value erro;
            erro["erro"] = e.what();
            
            Json::StreamWriterBuilder builder;
            res.status = 500;
            res.set_content(Json::writeString(builder, erro), "application/json");
        }
    }
    
    void iniciar(int porta = 8080) {
        std::cout << "Servidor Mestre iniciando na porta " << porta << std::endl;
        std::cout << "Escravos configurados: " << escravo1Host << ":" << escravo1Port << " e " 
                 << escravo2Host << ":" << escravo2Port << std::endl;
        
        servidor.listen("0.0.0.0", porta);
    }
    
    void parar() {
        servidor.stop();
    }
};

int main() {
    try {
        Mestre mestre;
        
        // Configura tratamento de sinais para shutdown graceful
        std::signal(SIGINT, [](int) {
            std::cout << "\nEncerrando servidor mestre..." << std::endl;
            exit(0);
        });
        
        mestre.iniciar(8080);
        
    } catch (const std::exception& e) {
        std::cerr << "Erro fatal: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}