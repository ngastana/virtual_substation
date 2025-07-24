#!/bin/bash

set -e  # Termina el script si ocurre un error
set -o pipefail

BASE_DIR=$(pwd)

CARPETAS=("terraform" "terraform_artifactory")

echo "🚨 Iniciando destrucción de recursos..."

for carpeta in "${CARPETAS[@]}"
do
  echo ""
  echo " Destruyendo en carpeta: $carpeta"
  cd "$BASE_DIR/$carpeta" || { echo "❌ No se pudo entrar a $carpeta"; exit 1; }

  terraform init -input=false >/dev/null
  terraform destroy -auto-approve

  echo " Borrando IEDs anteriores"
  find . -maxdepth 1 -type d -name "ied*" -exec rm -rf {} +
  cd "$BASE_DIR"

  echo " Borrando docker-compose anteriores"
  find . -maxdepth 1 -type f -name "docker-compose-[0-9]*.yml" -exec rm -f {} +

done

echo ""
echo " Borrando keypairs manualmente si existen..."

KEYPAIRS=("ssh-key-imgartifactory" "ssh-key-imglocal")

for key in "${KEYPAIRS[@]}"; do
  if openstack --insecure keypair show "$key" &>/dev/null; then
    echo "Eliminando keypair: $key"
    openstack --insecure keypair delete "$key" && echo " ✅ $key eliminado."
  else
    echo " ℹ️  El keypair '$key' no existe. Nada que borrar."
  fi
done


echo "✅ Recursos destruidos correctamente."
