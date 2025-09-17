# Sistema Distribuído C++ - Threads, Middleware e Docker

Sistema distribuído implementado em C++ puro seguindo arquitetura mestre-escravo, utilizando threads para paralelismo e containers Docker para orquestração.

## 🏗️ Arquitetura

```
Cliente (Notebook 1)
    ↓ HTTP REST
Mestre (Container 1) ── Thread 1 ──→ Escravo1 (Container 2) [/letras]
    └── Thread 2 ──→ Escravo2 (Container 3) [/numeros]
```

### Componentes

- **Cliente**: Interface para envio de arquivos .txt
- **Mestre**: Coordenador que distribui trabalho entre escravos usando threads
- **Escravo1**: Contador de letras (endpoint `/letras`)
- **Escravo2**: Contador de números (endpoint `/numeros`)

## 🚀 Início Rápido

### Pré-requisitos

```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake git pkg-config libssl-dev libjsoncpp-dev docker docker-compose curl

# Ou use o makefile
make install-deps
```

### Executando o Sistema

1. **Subir os containers**:
```bash
make docker-build
make docker-up
```

2. **Compilar o cliente**:
```bash
make cliente
```

3. **Testar o sistema**:
```bash
make teste
```

## 📁 Estrutura do Projeto

```
├── Cliente.cpp          # Cliente com interface de linha de comando
├── Mestre.cpp           # Servidor mestre (coordenador)
├── Escravo1.cpp         # Escravo contador de letras
├── Escravo2.cpp         # Escravo contador de números
├── Dockerfile.mestre    # Docker para o mestre
├── Dockerfile.escravo   # Docker para os escravos
├── docker-compose.yml   # Orquestração dos containers
├── build_escravo.sh     # Script de build para escravos
├── Makefile            # Automação de build e deploy
└── README.md           # Este arquivo
```

## 🔧 Compilação Manual

### Cliente
```bash
g++ -std=c++17 -o cliente Cliente.cpp -ljsoncpp -lpthread
```

### Servidores (local, para desenvolvimento)
```bash
g++ -std=c++17 -o mestre Mestre.cpp -ljsoncpp -lpthread
g++ -std=c++17 -o escravo1 Escravo1.cpp -ljsoncpp -lpthread  
g++ -std=c++17 -o escravo2 Escravo2.cpp -ljsoncpp -lpthread
```

## 🐳 Comandos Docker

```bash
# Construir images
docker-compose build

# Subir sistema
docker-compose up -d

# Ver logs
docker-compose logs -f

# Parar sistema
docker-compose down

# Health check manual
curl http://localhost:8080/health
```

## 💻 Uso do Cliente

### Linha de Comando
```bash
# Modo básico
./cliente -f arquivo.txt

# Especificar servidor
./cliente -f arquivo.txt -h localhost -p 8080

# Modo interativo
./cliente
```

### Exemplo de Arquivo de Entrada
```text
Teste123 com letras e números 456!
Mais algumas palavras aqui 789.
Final do arquivo com 0 zero.
```

### Exemplo de Saída
```
=== RESULTADO ===
Quantidade de letras: 42
Quantidade de números: 7
Total de caracteres processados: 49
================
```

## 🔄 Funcionamento Interno

1. **Cliente** envia arquivo .txt via POST `/processar` ao **Mestre**
2. **Mestre** verifica saúde dos escravos (`/health`)
3. **Mestre** dispara 2 threads paralelas:
   - Thread 1 → **Escravo1** (`/letras`)
   - Thread 2 → **Escravo2** (`/numeros`)
4. **Mestre** aguarda ambos os resultados com `std::future`
5. **Mestre** consolida resposta em JSON e retorna ao **Cliente**

## 📡 API Endpoints

### Mestre (porta 8080)
- `POST /processar` - Processa texto
- `GET /health` - Status do mestre

### Escravo1 (porta 8081)
- `POST /letras` - Conta letras
- `GET /health` - Status do escravo

### Escravo2 (porta 8081)  
- `POST /numeros` - Conta números
- `GET /health` - Status do escravo

### Exemplo de Request/Response

**Request**: `POST /processar`
```json
{
  "texto": "Olá123 mundo456!"
}
```

**Response**:
```json
{
  "letras": 8,
  "numeros": 6,
  "timestamp": 1637123456
}
```

## 🧪 Testes

### Teste Automatizado
```bash
make teste
```

### Teste Manual
```bash
# 1. Subir containers
make docker-up

# 2. Criar arquivo de teste
echo "Teste 123 com letras e números!" > teste.txt

# 3. Executar cliente
./cliente -f teste.txt

# 4. Verificar logs
make docker-logs
```

## 🔧 Tecnologias Utilizadas

- **C++17**: Linguagem principal
- **std::thread/std::async**: Concorrência
- **cpp-httplib**: Cliente/servidor HTTP
- **jsoncpp**: Processamento JSON
- **Docker & docker-compose**: Containerização
- **REST HTTP**: Comunicação entre processos

## 🛠️ Desenvolvimento

### Executar Localmente (sem Docker)
```bash
# Terminal 1 - Escravo1
make local
./escravo1

# Terminal 2 - Escravo2  
./escravo2

# Terminal 3 - Mestre
./mestre

# Terminal 4 - Cliente
./cliente -f exemplo.txt -h localhost
```

### Debug
```bash
# Logs detalhados
docker-compose logs -f mestre
docker-compose logs -f escravo1
docker-compose logs -f escravo2

# Verificar conectividade
curl -X GET http://localhost:8080/health
curl -X GET http://localhost:8081/health # (dentro do container)
```

## ⚠️ Troubleshooting

### Erro de Conexão
- Verificar se containers estão rodando: `docker ps`
- Verificar logs: `make docker-logs`
- Testar health checks: `curl http://localhost:8080/health`

### Erro de Compilação
- Instalar dependências: `make install-deps`
- Verificar versão do g++: `g++ --version` (necessário ≥ 7.0)
- Verificar bibliotecas: `pkg-config --libs jsoncpp`

### Containers não Sobem
- Limpar sistema: `make clean`
- Reconstruir: `make docker-build`
- Verificar porta ocupada: `sudo netstat -tlnp | grep 8080`

## 📋 Requisitos Atendidos

- ✅ **C++17**: Implementado em C++ puro
- ✅ **Threads**: `std::thread` e `std::async` para paralelismo
- ✅ **REST HTTP**: Comunicação via cpp-httplib
- ✅ **Docker**: Containerização completa com docker-compose
- ✅ **Arquitetura Mestre-Escravo**: Implementada conforme especificação
- ✅ **Health Checks**: Verificação de disponibilidade dos escravos
- ✅ **JSON**: Respostas consolidadas em formato JSON
- ✅ **Interface CLI**: Cliente com interface de linha de comando


**Desenvolvido para o trabalho 04 - Threads, Middleware, Paralelismo e Docker da disciplina Sistemas Distribuídos**