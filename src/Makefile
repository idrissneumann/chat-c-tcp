warnings= -Wall -Wextra
gtkdep=`pkg-config --cflags --libs gtk+-2.0`

all:
	make clean
	make server
	make client
	make client_shell
	make clean
	
server: server.o socketfunctions.o iofunctions.o listfunctions.o logfunctions.o
	gcc -pthread server.o socketfunctions.o iofunctions.o listfunctions.o logfunctions.o -o server
	make clean

client: client.o socketfunctions.o iofunctions.o listfunctions.o logfunctions.o
	gcc client.o socketfunctions.o iofunctions.o listfunctions.o logfunctions.o -o client $(gtkdep)
	make clean
	
client_shell: client_shell.o socketfunctions.o iofunctions.o listfunctions.o logfunctions.o
	gcc client_shell.o socketfunctions.o iofunctions.o listfunctions.o logfunctions.o -o client_shell
	make clean

server.o: server.c
	gcc -c server.c $(warnings) -o server.o

client.o: client.c
	gcc -c client.c $(warnings) -o client.o $(gtkdep)

client_shell.o: client_shell.c
	gcc -c client_shell.c $(warnings) -o client_shell.o
	
clienttcp_shell.o: client_shell.c
	gcc -c clienttcp_shell.c $(warnings) -o clienttcp_shell.o

socketfunctions.o: socketfunctions.c
	gcc -c socketfunctions.c $(warnings) -o socketfunctions.o

iofunctions.o: iofunctions.c
	gcc -c iofunctions.c $(warnings) -o iofunctions.o
	
listfunctions.o: listfunctions.c
	gcc -c listfunctions.c $(warnings) -o listfunctions.o
	
logfunctions.o: logfunctions.c
	gcc -c logfunctions.c $(warnings) -o logfunctions.o

clean:
	rm -rf *~ *.o rapport/*~
	
rmlog:
	rm -rf ~/client*.log ~/server*.log client*.log server*.log
	
rmtest:
	rm *.xml *.jpeg *.png