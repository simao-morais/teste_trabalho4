#include <iostream>
#include <cctype>
#include <algorithm>
#include <httplib.h>
#include <jsoncpp/json/json.h>

class Escravo1 {
private:
    httplib::Server servidor;
    
public:
    Escravo1() {
        configurarRotas();
    }
    
    void configurarRotas() {
        // Endpoint para contar letras
        servidor.Post("/letras", [this](const httplib::Request& req, httplib::Response& res) {
            this->contarLetras(req, res);
        });
        
        // Health check
        servidor.Get("/health", [](const httplib::Request&, httplib::Response& res) {
            Json::Value resposta;
            resposta["status"] = "ok";
            resposta["servico"] = "escravo1-letras";
            resposta["funcionalidade"] = "contador de letras";
            
            Json::StreamWriterBuilder builder;
            res.set_content(Json::writeString(builder, resposta), "application/json");
        });
    }
    
    int contarLetrasTexto(const std::string& texto) {
        return std::count_if(texto.begin(), texto.end(), [](char c) {
            return std::isalpha(static_cast<unsigned char>(c));
        });
    }
    
    void contarLetras(const httplib::Request& req, httplib::Response& res) {
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
            std::cout << "Escravo1: Contando letras em texto de " 
                     << texto.length() << " caracteres..." << std::endl;
            
            int quantidade = contarLetrasTexto(texto);
            
            // Constrói resposta
            Json::Value resposta;
            resposta["quantidade"] = quantidade;
            resposta["tipo"] = "letras";
            resposta["processado_por"] = "escravo1";
            resposta["timestamp"] = std::time(nullptr);
            
            Json::StreamWriterBuilder builder;
            res.set_content(Json::writeString(builder, resposta), "application/json");
            
            std::cout << "Escravo1: Encontradas " << quantidade << " letras" << std::endl;
            
        } catch (const std::exception& e) {
            std::cerr << "Escravo1 - Erro: " << e.what() << std::endl;
            
            Json::Value erro;
            erro["erro"] = e.what();
            erro["servico"] = "escravo1";
            
            Json::StreamWriterBuilder builder;
            res.status = 500;
            res.set_content(Json::writeString(builder, erro), "application/json");
        }
    }
    
    void iniciar(int porta = 8081) {
        std::cout << "Escravo1 (Contador de Letras) iniciando na porta " << porta << std::endl;
        servidor.listen("0.0.0.0", porta);
    }
    
    void parar() {
        servidor.stop();
    }
};

int main() {
    try {
        Escravo1 escravo;
        
        // Tratamento de sinais
        std::signal(SIGINT, [](int) {
            std::cout << "\nEncerrando Escravo1..." << std::endl;
            exit(0);
        });
        
        escravo.iniciar(8081);
        
    } catch (const std::exception& e) {
        std::cerr << "Erro fatal no Escravo1: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}