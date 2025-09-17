#!/bin/bash

# --- Passo 1: Instalar dependências ---
# Assume-se que o sistema é baseado em Debian/Ubuntu.
echo "Atualizando a lista de pacotes e instalando dependências..."
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    qtbase5-dev \
    libjsoncpp-dev \
    libpthread-stubs0-dev

# --- Passo 2: Compilar o cliente ---
echo "Compilando o cliente..."
# Navega até o diretório onde os arquivos do cliente estão.
# Substitua 'caminho/para/o/diretorio/do/cliente' pelo caminho real.
cd caminho/para/o/diretorio/do/cliente

# Gera o Makefile usando qmake (o arquivo .pro já está disponível)
qmake cliente.pro

# Compila o projeto
make

echo "Automação concluída. O executável 'cliente' deve ter sido criado."

# --- Passo 3 (Opcional): Executar o cliente ---
# O cliente agora pode ser executado. Pode ser necessário ajustar o IP do servidor
# no código-fonte antes de compilar.
# ./cliente
