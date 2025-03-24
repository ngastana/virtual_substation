FROM python:3.11-slim

WORKDIR /app

COPY app/ /app/

RUN python3 /app/json_creator.py

CMD ["cat", "/app/ied-config.json"]