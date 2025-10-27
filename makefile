ifneq ($(wildcard /C/WINDOWS/System32),)
	OS := Windows
else ifneq ($(wildcard /usr/bin/ls),)
	OS := Linux
else
	OS := UNKNOWN
endif

ifeq ($(OS),Windows)

tunnels:server.o tools.o platform.o config.o threading.o entry.o network.o Socks5.o
	gcc -g link/server.o link/tools.o link/platform.o link/config.o link/threading.o link/entry.o link/network.o link/Socks5.o -o tunnels.exe -lws2_32

tunnelc:client.o tools.o platform.o config.o threading.o entry.o network.o Socks5.o
	gcc -g link/client.o link/tools.o link/platform.o link/config.o link/threading.o link/entry.o link/network.o link/Socks5.o -o tunnelc.exe -lws2_32

test: test.o tools.o platform.o
	gcc -g link/test.o link/tools.o link/platform.o -o test -lws2_32

else ifeq ($(OS),Linux)
tunnels:server.o tools.o platform.o config.o threading.o entry.o network.o Socks5.o
	gcc -g link/server.o link/tools.o link/platform.o link/config.o link/threading.o link/entry.o link/network.o link/Socks5.o -o frps

tunnelc:client.o tools.o platform.o config.o threading.o entry.o network.o Socks5.o
	gcc -g link/client.o link/tools.o link/platform.o link/config.o link/threading.o link/entry.o link/network.o link/Socks5.o -o frpc

test: test.o tools.o platform.o
	gcc -g link/test.o link/tools.o link/platform.o -o test

endif

all: tunnels tunnelc

server.o: server/server.c
	gcc -g -c server/server.c -o link/server.o

client.o: client/client.c
	gcc -g -c client/client.c -o link/client.o

test.o: test.c
	gcc -g -c test.c -o link/test.o

config.o: common/config.c 
	gcc -g -c common/config.c -o link/config.o

tools.o: common/tools.c
	gcc -g -c common/tools.c -o link/tools.o

platform.o: common/platform.c
	gcc -g -c common/platform.c -o link/platform.o

threading.o: common/threading.c
	gcc -g -c common/threading.c -o link/threading.o

network.o: network/network.c
	gcc -g -c network/network.c -o link/network.o

Socks5.o: network/Socks5.c
	gcc -g -c network/Socks5.c -o link/Socks5.o

entry.o: network/entry.c
	gcc -g -c network/entry.c -o link/entry.o

clean:
	rm -rf link/*.o