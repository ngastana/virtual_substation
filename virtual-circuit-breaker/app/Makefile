PROJECT_BINARY_NAME = virtcirtbreaker
PROJECT_SOURCES += src/virt_cirt_breaker.c

INCLUDES += -I./
INCLUDES += -Iinclude/
INCLUDES += -I/usr/include/iec61850/

LDFLAGS += -L/usr/lib/iec61850
LDLIBS  += -liec61850
LDLIBS  += -lpthread
LDLIBS += -lm


all: $(PROJECT_BINARY_NAME)

$(PROJECT_BINARY_NAME):	$(PROJECT_SOURCES) $(LIB_NAME)
	$(CC) -g $(CFLAGS) $(LDFLAGS) -o $(PROJECT_BINARY_NAME) $(PROJECT_SOURCES) $(INCLUDES) $(LIB_NAME) $(LDLIBS)

clean:
	rm -f $(PROJECT_BINARY_NAME)
