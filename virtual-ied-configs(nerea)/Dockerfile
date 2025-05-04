FROM python:3.9

WORKDIR /app

COPY ./virtual-ied-configs /app

COPY ../virtual-circuit-breaker /app/virtual-circuit-breaker

RUN chmod +x /app/entrypoint.sh

CMD ["/app/entrypoint.sh"]