SRC = ./creando_nodos/json_creator.py 
	
SRC1 = ./creando_nodos/container_creator.py

CC = python3

CLR_RMV := \033[0m
RED := \033[1;31m
GREEN := \033[1;32m
YELLOW := \033[1;33m
BLUE := \033[1;34m
CYAN := \033[1;36m

define ART
$(GREEN)

QUE EMPIEZE $(RED) LA MARCHA

$(BLUE) By Nerea Gastañaga

$(YELLOW)SIN MIEDO AL EXITO:
$(CYAN) 
1.- Hacer que pille cualquier archivo .xml que se le ponga
2.- Que levante no solo XCBR si no que tambien LPDH, GGIO, PDIS, MMXU..
3.- Conecxión entre ellos
4.- Estructurar la información del .json
5.- poder poner las caracteristicas de cada uno en su contenedor correspondiente

$(CLR_RMV)
endef
export ART

all : 	
	@$(CC) $(SRC)
	@echo "$$ART"
	@$(CC) $(SRC1) 
