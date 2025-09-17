#!/bin/bash

# Script de setup completo do sistema distribuído

set -e

echo "=== Sistema Distribuído C++ - Setup ==="
echo

# Função para verificar se comando existe
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Verificar dependências
echo "1. Verificando dependências..."

if ! command_exists docker; then
    echo "❌ Docker não encontrado. Instale o Docker primeiro."
    echo "   Ubuntu: sudo apt install docker.io"
    exit 1
fi

if ! command_exists docker-compose; then
    echo "❌ Docker Compose não encontrado. Instale o Docker Compose primeiro."
    echo "   Ubuntu: sudo apt install docker-compose"
    exit 1
fi

if ! command_exists g++; then
    echo "❌ G++ não encontrado. Instalando build tools..."
    sudo apt update
    sudo apt install -y build-essential
fi

echo "✅ Dependências básicas verificadas"

# Instalar dependências de desenvolvimento
echo
echo "2. Instalando dependências de desenvolvimento..."
sudo apt update
sudo apt install -y build-essential cmake git pkg-config libssl-dev libjsoncpp-dev curl

echo "✅ Dependências instaladas"

# Baixar cpp-httplib se necessário
echo
echo "3. Configurando cpp-httplib..."
if [ ! -f /usr/local/include/httplib.h ]; then
    echo "Baixando cpp-httplib..."
    git clone https://github.com/yhirose/cpp-httplib.git /tmp/cpp-httplib
    sudo cp /tmp/cpp-httplib/httplib.h /usr/local/include/
    rm -rf /tmp/cpp-httplib
    echo "✅ cpp-httplib instalado"
else
    echo "✅ cpp-httplib já está instalado"
fi

# Compilar cliente
echo
echo "4. Compilando cliente..."
g++ -std=c++17 -I/usr/local/include -o cliente Cliente.cpp -ljsoncpp -lpthread
echo "✅ Cliente compilado"

# Construir containers Docker
echo
echo "5. Construindo containers Docker..."
docker-compose build
echo "✅ Containers construídos"

# Subir sistema
echo
echo "6. Subindo sistema distribuído..."
docker-compose up -d
echo "✅ Sistema iniciado"

# Aguardar containers ficarem prontos
echo
echo "7. Aguardando containers ficarem prontos..."
sleep 10

# Verificar health checks
echo "Verificando saúde dos serviços..."
for i in {1..30}; do
    if curl -s http://localhost:8080/health > /dev/null; then
        echo "✅ Mestre está rodando"
        break
    fi
    echo "   Aguardando mestre... ($i/30)"
    sleep 2
done

# Criar arquivo de exemplo
echo
echo "8. Criando arquivo de exemplo..."
cat > exemplo.txt << 'EOF'
Teste123 do sistema distribuído com letras e números 456!
Este arquivo contém várias palavras e alguns dígitos 789.
Vamos verificar se o processamento paralelo funciona corretamente.
Números como 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 devem ser contados.
Letras maiúsculas: ABCDEFG e minúsculas: abcdefg também.
Final do arquivo de teste com mais números: 101112.
EOF
echo "✅ Arquivo exemplo.txt criado"

# Teste do sistema
echo
echo "9. Testando sistema..."
echo "Executando: ./cliente -f exemplo.txt"
echo "─────────────────────────────────────"
./cliente -f exemplo.txt
echo "─────────────────────────────────────"

echo
echo "🎉 Setup concluído com sucesso!"
echo
echo "Comandos úteis:"
echo "  ./cliente -f arquivo.txt     # Processar arquivo"
echo "  docker-compose logs -f       # Ver logs"
echo "  docker-compose down          # Parar sistema"
echo "  docker-compose up -d         # Subir sistema"
echo "  curl http://localhost:8080/health  # Health check"
echo
echo "Arquivos criados:"
echo "  - cliente                    # Executável do cliente"
echo "  - exemplo.txt               # Arquivo de teste"
echo
echo "Sistema rodando em:"
echo "  - Mestre: http://localhost:8080"
echo "  - Containers: mestre, escravo1, escravo2"