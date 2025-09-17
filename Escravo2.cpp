#include <iostream>
#include <cctype>
#include <algorithm>
#include <httplib.h>
#include <jsoncpp/json/json.h>

class Escravo2 {
private:
    httplib::Server servidor;
    
public:
    Escravo2() {
        configurarRotas();
    }
    
    void configurarRotas() {
        // Endpoint para contar números
        servidor.Post("/numeros", [this](const httplib::Request& req, httplib::Response& res) {
            this->contarNumeros(req, res);
        });
        
        // Health check
        servidor.Get("/health", [](const httplib::Request&, httplib::Response& res) {
            Json::Value resposta;
            resposta["status"] = "ok";
            resposta["servico"] = "escravo2-numeros";
            resposta["funcionalidade"] = "contador de números";
            
            Json::StreamWriterBuilder builder;
            res.set_content(Json::writeString(builder, resposta), "application/json");
        });
    }
    
    int contarNumerosTexto(const std::string& texto) {
        return std::count_if(texto.begin(), texto.end(), [](char c) {
            return std::isdigit(static_cast<unsigned char>(c));
        });
    }
    
    void contarNumeros(const httplib::Request& req, httplib::Response& res) {
        try {
            // Parse do JSON
            Json::Value requestJson;
            Json::Reader reader;
            if (!reader.parse(req.body, requestJson)) {
                res.status = 400;
                res.set_content("{\"erro\": \"JSON inválido\"}", "application/json");
                return;
            }
            
            std::string texto = requestJson["texto"].asString();
            std::cout << "Escravo2: Contando números em texto de " 
                     << texto.length() << " caracteres..." << std::endl;
            
            int quantidade = contarNumerosTexto(texto);
            
            // Constrói resposta
            Json::Value resposta;
            resposta["quantidade"] = quantidade;
            resposta["tipo"] = "numeros";
            resposta["processado_por"] = "escravo2";
            resposta["timestamp"] = std::time(nullptr);
            
            Json::StreamWriterBuilder builder;
            res.set_content(Json::writeString(builder, resposta), "application/json");
            
            std::cout << "Escravo2: Encontrados " << quantidade << " números" << std::endl;
            
        } catch (const std::exception& e) {
            std::cerr << "Escravo2 - Erro: " << e.what() << std::endl;
            
            Json::Value erro;
            erro["erro"] = e.what();
            erro["servico"] = "escravo2";
            
            Json::StreamWriterBuilder builder;
            res.status = 500;
            res.set_content(Json::writeString(builder, erro), "application/json");
        }
    }
    
    void iniciar(int porta = 8082) { // Porta alterada para 8082
        std::cout << "Escravo2 (Contador de Números) iniciando na porta " << porta << std::endl;
        servidor.listen("0.0.0.0", porta);
    }
    
    void parar() {
        servidor.stop();
    }
};

int main() {
    try {
        Escravo2 escravo;
        
        // Tratamento de sinais
        std::signal(SIGINT, [](int) {
            std::cout << "\nEncerrando Escravo2..." << std::endl;
            exit(0);
        });
        
        escravo.iniciar(8082); // Chamada com a porta 8082
        
    } catch (const std::exception& e) {
        std::cerr << "Erro fatal no Escravo2: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}