PROJECT_BINARY_NAME = virtualied
PROJECT_SOURCES += src/virtualied.c
PROJECT_SOURCES += src/virtual_ied_model.c


ifeq ($(WITH_GPIO), true)
PROJECT_SOURCES += src/gpio_ctrl.c
endif

INCLUDES += -I./
INCLUDES += -Iinclude/
INCLUDES += -I/usr/include/iec61850/

ifeq ($(WITH_GPIO), true)
CFLAGS += -DGPIO_SUPPORT_ENABLED
endif

LDFLAGS += -L/usr/lib/iec61850
LDLIBS  += -liec61850
LDLIBS  += -lpthread


all: $(PROJECT_BINARY_NAME)

$(PROJECT_BINARY_NAME):	$(PROJECT_SOURCES) $(LIB_NAME)
	$(CC) -g $(CFLAGS) $(LDFLAGS) -o $(PROJECT_BINARY_NAME) $(PROJECT_SOURCES) $(INCLUDES) $(LIB_NAME) $(LDLIBS)

clean:
	rm -f $(PROJECT_BINARY_NAME)
