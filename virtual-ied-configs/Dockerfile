FROM python:3.11-slim

ARG NODE_NAME
WORKDIR /app

COPY app/ /app/

RUN python3 /app/json_creator.py /app/container_creator.py

CMD ["echo", "Archivo JSON creado. Ejecuta container_creator.py fuera del contenedor."]
