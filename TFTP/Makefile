LDFLAGS = -lpthread

Server : AdresseInternet.o SocketUDP.o tftp.o server.o
		gcc AdresseInternet.o SocketUDP.o tftp.o server.o -o Server $(LDFLAGS)

Client : AdresseInternet.o SocketUDP.o tftp.o client.o
		gcc AdresseInternet.o SocketUDP.o tftp.o client.o -o Client $(LDFLAGS)

AdresseInternet.o : AdresseInternet.c
		gcc -c AdresseInternet.c -o AdresseInternet.o $(LDFLAGS)

SocketUDP.o : SocketUDP.c
		gcc -c SocketUDP.c -o SocketUDP.o $(LDFLAGS)

tftp.o : tftp.c
		gcc -c tftp.c -o tftp.o $(LDFLAGS)

server.o : server.c
		gcc -c server.c -o server.o $(LDFLAGS)

client.o : client.c
		gcc -c client.c -o client.o $(LDFLAGS)