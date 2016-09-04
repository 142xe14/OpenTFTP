Programme : main.o AdresseInternet.o SocketUDP.o
		gcc AdresseInternet.o SocketUDP.o main.o -o Programme

main.o : main.c AdresseInternet.c
		gcc -c main.c -o main.o

AdresseInternet.o : AdresseInternet.c
		gcc -c AdresseInternet.c -o AdresseInternet.o

SocketUDP.o : SocketUDP.c
		gcc -c SocketUDP.c -o SocketUDP.o