FROM alpine:latest

# Instala gcc, musl-dev y make
RUN apk add --no-cache gcc musl-dev make

WORKDIR /app

COPY src/virtual_goose.c ./src/
COPY include/goose_publisher.h ./include/
COPY goose_publisher.c .
COPY Makefile .

RUN make

CMD ["./virtual_goose"]
