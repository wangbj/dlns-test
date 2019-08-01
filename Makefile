CC	 = clang

CFLAGS	 = -g -O1 -Wall -fPIC -D_GNU_SOURCE=1
CFLAGS_ASAN = -g -O1 -Wall -fPIC -D_GNU_SOURCE=1 -fstack-protector-strong -fsanitize=address -fno-omit-frame-pointer

all: libpreload.so libdlns.so run app app-standalone

libpreload.so: preload.c
	$(CC) -o $@ $< $(CFLAGS) -shared -ldl -Wl,-soname=preload

libdlns.so: dlns.c
	$(CC) -o $@ $< $(CFLAGS) -shared -ldl -lpthread -Wl,-soname=dlns,-as-needed

run: run.c
	$(CC) -o $@ $^ $(CFLAGS)

app: app.c
	$(CC) -o $@ $^ $(CFLAGS) -DWITH_DLNS -pthread

app-standalone: app.c dlns.c
	$(CC) -o $@ $^ $(CFLAGS) -pthread

.PHONY: all clean test

clean:
	$(RM) libpreload.so libdlns.so run run.o dlns.o preload.o app app.o core

test:
	./run libpreload.so libdlns.so ./app
