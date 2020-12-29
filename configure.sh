#!/bin/bash

# create libcMQTT director
mkdir -p libcMQTT libcMQTT/include libcMQTT/lib

if [ ! -f "Makefile" ]; then
touch Makefile
fi

# clear Makefile
echo "" > Makefile

cat <<-EOF > Makefile

CFLAGS = -g -fPIC

HAL_SRC_DIR := hal
HAL_FILES_C := \$(shell find \$(HAL_SRC_DIR) -name "*.c")
HAL_HDRDIR := \$(shell find \$(HAL_SRC_DIR) -type d)
HAL_HDRDIR := \$(addprefix -I,\$(HAL_HDRDIR))

INFRA_SRC_DIR := infra
INFRA_FILES_C := \$(shell find \$(INFRA_SRC_DIR) -name "*.c")
INFRA_HDRDIR := \$(shell find \$(INFRA_SRC_DIR) -type d)
INFRA_HDRDIR := \$(addprefix -I,\$(INFRA_HDRDIR))

MQTT_SRC_DIR := mqtt
MQTT_FILES_C := \$(shell find \$(MQTT_SRC_DIR) -name "*.c")
MQTT_HDRDIR := \$(shell find \$(MQTT_SRC_DIR) -type d)
MQTT_HDRDIR := \$(addprefix -I,\$(MQTT_HDRDIR))

INC = \$(HAL_HDRDIR) \$(INFRA_HDRDIR) \$(MQTT_HDRDIR)

LIBS = -lpthread -lrt

SRC = \$(HAL_FILES_C) \$(INFRA_FILES_C) \$(MQTT_FILES_C)

TARGET = libcMQTT.so

OBJS = \$(SRC:.c=.o)

EOF

if [ -n "$1" ]
then
COMPILE=$1
else
COMPILE=gcc
fi

echo -e "\nCC = `echo $COMPILE`" >> Makefile

echo -e "\n\$(TARGET):\$(OBJS)" >> Makefile
echo -e "\t\$(CC) -o \$@ -shared \$^ \$(LIBS)" >> Makefile

echo -e "\n.PHONY: clean" >> Makefile

echo -e "\nclean:" >> Makefile
echo -e "\trm -f \$(OBJS)" >> Makefile

echo -e "\ninstall: \$(TARGET) clean" >> Makefile
echo -e "\t@echo start compile..." >> Makefile
echo -e "\tcp -r ./hal/*.h ./libcMQTT/include" >> Makefile
echo -e "\tcp -r ./infra/*.h ./libcMQTT/include" >> Makefile
echo -e "\tcp -r ./mqtt/*.h ./libcMQTT/include" >> Makefile
echo -e "\tmv \$(TARGET) ./libcMQTT/lib" >> Makefile
echo -e "\t@echo success" >> Makefile

echo -e "\n%.o:%.c" >> Makefile
echo -e "\t\$(CC) \$(CFLAGS) \$(INC) \$(LIBS) -o \$@ -c \$<" >> Makefile


