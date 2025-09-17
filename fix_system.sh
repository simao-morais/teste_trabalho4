#!/bin/bash

echo "🔧 Corrigindo sistema distribuído..."

# Parar e remover containers atuais
echo "1. Parando e removendo containers..."
docker-compose down

# Limpar imagens antigas
echo "2. Limpando cache Docker e imagens 'dangling'..."
docker system prune -f
docker rmi -f $(docker images -q --filter "dangling=true") 2>/dev/null || true

# Recompilar cliente
echo "3. Recompilando o cliente..."
g++ -std=c++17 -I/usr/include/jsoncpp -o cliente Cliente.cpp -ljsoncpp -lpthread
if [ $? -ne 0 ]; then
    echo "❌ Erro na compilação do cliente. Por favor, verifique as dependências."
    exit 1
fi

# Reconstruir imagens Docker com cache desabilitado
echo "4. Reconstruindo containers (build --no-cache)..."
docker-compose build --no-cache

if [ $? -ne 0 ]; then
    echo "❌ Erro no build dos containers. Por favor, verifique os Dockerfiles."
    exit 1
fi

# Subir o sistema em modo detached
echo "5. Subindo o sistema corrigido..."
docker-compose up -d

# Aguardar inicialização dos healthchecks
echo "6. Aguardando a inicialização completa dos serviços (até 60s)..."
# Loop para verificar o healthcheck do Mestre
for i in {1..20}; do
    # Tenta obter o status de saúde do Mestre
    health_status=$(docker inspect --format='{{.State.Health.Status}}' mestre 2>/dev/null)
    if [ "$health_status" == "healthy" ]; then
        echo "✅ O Mestre está pronto!"
        break
    fi
    echo "   Aguardando o Mestre... ($i/20)"
    sleep 3
    if [ $i -eq 20 ]; then
        echo "❌ Mestre não ficou saudável a tempo. Abortando."
        docker-compose logs mestre
        exit 1
    fi
done

echo
echo "7. Verificando logs dos escravos..."
echo "--- Escravo1 ---"
docker-compose logs --tail=5 escravo1
echo "--- Escravo2 ---"
docker-compose logs --tail=5 escravo2

echo
echo "8. Testando o sistema completo..."
echo "Executando: ./cliente -f exemplo.txt"
echo "─────────────────────────────────────"
./cliente -f exemplo.txt
echo "─────────────────────────────────────"

echo
echo "🎉 Processo concluído com sucesso!"
echo "🔍 Para inspecionar, use 'docker-compose logs -f' ou 'docker exec -it <nome_do_container> /bin/bash'."