CC = xtensa-lx106-elf-gcc
ESP_SDK_PATH = ../esp-open-sdk/sdk
CFLAGS = -I. -mlongcalls -DICACHE_FLASH \
	-I$(ESP_SDK_PATH)/include \
	-I$(ESP_SDK_PATH)/driver_lib/include/driver
LDLIBS = -nostdlib -Wl,--start-group -lmain -lnet80211 -lwpa -llwip \
	-lpp -lcirom -lphy -Wl,--end-group -lgcc -ldriver
LDFLAGS = -Teagle.app.v6.ld
# This variable controls the name of generated binaries
ESP_IMAGE_PREFIX = application-
ESP_TOOL_PORT = /dev/ttyUSB0
ESP_TOOL_BAUD = 115200
OBJ_DIR = ./obj
BIN_DIR = ./bin

ESP_BOOT_ADDRESS = 0x00000
ESP_USER_ADDRESS = 0x10000
ESP_BOOT_IMAGE = ${BIN_DIR}/${ESP_IMAGE_PREFIX}${ESP_BOOT_ADDRESS}.bin
ESP_USER_IMAGE = ${BIN_DIR}/${ESP_IMAGE_PREFIX}${ESP_USER_ADDRESS}.bin
ESP_BLANK_IMAGE = ${BIN_DIR}/blank.bin

# The esptool.py script elf2image command will output two images: 
#	${ESP_IMAGE_PREFIX}0x00000.bin and
#	${ESP_IMAGE_PREFIX}0x10000.bin
# Thes should be loaded at the cooresponding addresses during flash
${ESP_BOOT_IMAGE} ${ESP_USER_IMAGE}: ${OBJ_DIR}/esp8266HttpServer ${BIN_DIR}
	esptool.py elf2image -o ${BIN_DIR}/${ESP_IMAGE_PREFIX} $<

${ESP_BLANK_IMAGE}:
	cp $(ESP_SDK_PATH)/bin/blank.bin $@

${BIN_DIR}:
	mkdir $@

${OBJ_DIR}/esp8266HttpServer: ${OBJ_DIR}/esp8266HttpServer.o
	${CC} -o $@ $< ${LDFLAGS} ${LDLIBS}

${OBJ_DIR}/esp8266HttpServer.o: esp8266HttpServer.c ${OBJ_DIR}
	${CC} -o $@ -c $< ${CFLAGS} 

${OBJ_DIR}:
	mkdir $@

flash: ${ESP_BOOT_IMAGE} ${ESP_USER_IMAGE} ${ESP_BLANK_IMAGE}
	esptool.py -p ${ESP_TOOL_PORT} -b ${ESP_TOOL_BAUD} \
		write_flash --flash_mode dio \
			${ESP_BOOT_ADDRESS} ${ESP_BOOT_IMAGE} \
			${ESP_USER_ADDRESS} ${ESP_USER_IMAGE} \
			0x7e000 ${ESP_BLANK_IMAGE} \
			0x3fe000 ${ESP_BLANK_IMAGE}

reset:
	esptool.py -p ${ESP_TOOL_PORT} -b ${ESP_TOOL_BAUD} run

clean:
	rm -rf ${OBJ_DIR} ${BIN_DIR}
		
# TODO: This Makefile will not work outside of eclipse unless the ESP_SDK_PATH 
#	environment variable is set. Should probably do something to fix that.