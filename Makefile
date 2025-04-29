SRC = ./creando_nodos/json_creator.py 
	
#SRC1 = ./creando_nodos/container_creator.py

CC = python3

RESET := \033[0m
RED := \033[1;31m
GREEN := \033[1;32m
YELLOW := \033[1;33m
BLUE := \033[1;34m
CYAN := \033[1;36m

define ART
$(CYAN)  


	     ┓┏•       ┓┳┓   ┓   
	     ┃┃┓┏┓╋┓┏┏┓┃┃┃┏┓┏┫┏┓┏
	     ┗┛┗┛ ┗┗┻┗┻┗┛┗┗┛┗┻┗ ┛
											
$(RED)

	Tecnologías de virtualización y 
automatización de nodos lógicos para subestaciones 
   eléctricas a partir de configuraciones SCL

$(RESET)

endef
export ART

all :
	@docker-compose down --remove-orphans
	@$(CC) $(SRC)
	@echo "$$ART"
	@docker-compose build 
	@docker-compose up -d
#	@$(CC) $(SRC1) 

.PHONY: activity-log dump-net both

# 1) Volcar los logs de un servicio ya en marcha
activity-log:
	@echo "$(CYAN)📝 Iniciando volcado de logs de virtual-circuit-breaker en archivoactividad.log… $(RESET)"
	@# -f para seguir en tiempo real
	@docker logs -f virtual-circuit-breaker >> archivoactividad.log 2>&1 &

# 2) Inyectar tcpdump en el contenedor ya en marcha
dump-net:
	@echo "$(CYAN)📝 Capturando IEC-104 en actividad.txt desde virtual-merging-unit…$(RESET)"
	@docker run --rm \
		--net container:virtual-merging-unit \
		-v "$(PWD)":/output \
		nicolaka/netshoot \
		sh -c "tcpdump -i eth0 -nn -vvX port 102 > /output/actividad.txt & sleep 9999"

# 3) Ambos a la vez
both: activity-log dump-net
	@echo "✅ Logs y dump de red en marcha."
	@echo "   • archivoactividad.log"
	@echo "   • pcap/actividad.txt"
