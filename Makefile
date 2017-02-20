CC = xtensa-lx106-elf-gcc
ESP_OPEN_SDK_DIR = ../esp-open-sdk/sdk
HTTP_PARSER_DIR = ../http-parser
CFLAGS = -I. -mlongcalls -DICACHE_FLASH \
	-I$(ESP_SDK_PATH)/include \
	-I$(ESP_SDK_PATH)/driver_lib/include/driver
LDLIBS = -nostdlib -Wl,--start-group -lmain -lnet80211 -lwpa -llwip \
	-lpp -lcirom -lphy -Wl,--end-group -lgcc -ldriver
LDFLAGS = -Teagle.app.v6.ld
COPY_FILES = $(ESP_SDK_PATH)/bin/blank.bin

.PHONY: rebuild
rebuild: clean build

.PHONY: build
build: esp8266HttpServer.bin blank.bin

esp8266HttpServer.bin: esp8266HttpServer
	esptool.py elf2image $<

blank.bin: $(ESP_SDK_PATH)/bin/blank.bin
	cp $< $@

esp8266HttpServer.bin: esp8266HttpServer.o

esp8266HttpServer.o: esp8266HttpServer.c

flash: esp8266HttpServer.bin
	esptool.py -p /dev/ttyUSB0 write_flash --flash_mode dio \
		0x00000 esp8266HttpServer-0x00000.bin \
		0x10000 esp8266HttpServer-0x10000.bin \
		0x7e000 blank.bin \
		0x3fe000 blank.bin

reset:
	esptool.py run

clean:
	rm -f \
		esp8266HttpServer \
		esp8266HttpServer.o \
		esp8266HttpServer-0x00000.bin \
		esp8266HttpServer-0x10000.bin \
		blank.bin