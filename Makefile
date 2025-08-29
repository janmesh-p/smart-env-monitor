# Makefile for smart-env-monitor
# Run from smart-env-monitor/src/
# Requires: libgpiod-dev, libcurl4-openssl-dev
# Ensure common_dht_read.h and bbb_dht_read.h are in ../include/

CC = gcc
CFLAGS = -Iinclude -Wall -g
LDFLAGS = -lgpiod -lcurl -pthread
SOURCES =src/ main.c src/dht22.c src/ssd1306.c src/fan.c src/led.c src/ldr.c src/uart.c src/button.c src/thingspeak.c
EXEC = smart-env-monitor

all: $(EXEC)

$(EXEC): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LDFLAGS)
	chmod +x $@

clean:
	rm -f $(EXEC)

.PHONY: all clean
