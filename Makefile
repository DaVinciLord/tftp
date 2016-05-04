CC = gcc
CFLAGS = -Wall -Werror -Wextra -pedantic -std=c11 -D_XOPEN_SOURCE=700 -pthread
#CFLAGS = -D_XOPEN_SOURCE=700

# définition des bibliothèques à utiliser lors de l'édition de lien de
# droite à gauche par ordre de dépendance. Les bibliothèques sont
# recherchées dans les répertoires système standard, plus les
# répertoires indiqués avec l'option -L dans les LDFLAGS.  L'éditeur
# de lien utilisera, en fonction des fichiers trouvés, soit: les
# bibliothèques dynamiques (libAdresseInternet.so), statiques
# (libAdresseInternet.a) ou les fichiers objets (AdresseInternet.o)
LDLIBS = -lAdresseInternet -lSocketUDP -lTFTP -pthread
LIBS =  libTFTP.a libSocketUDP.a libAdresseInternet.a

# non standard, mais on peut définir une macro contenant tous les
# exécutables du projet:
PRGS = server client test_socket_udp test_tftp_make

# la première cible est celle qui sera exécutée par défaut si on
# exécute "make" sans argument.
all: $(LIBS)

client: client.c all
	gcc $(CFLAGS) $< $(LIBS)  -o client
	
server: server.c all
	gcc $(CFLAGS) $< $(LIBS) -o server
	
test_socket_udp: test_socket_udp.c all
	gcc $(CFLAGS) $< $(LIBS)  -o test_socket_udp
	
test_tftp_make: test_tftp_make.c all
	gcc $(CFLAGS) $< $(LIBS)  -o test_tftp_make

libTFTP.a: tftp.o
	ar r $@ $<

libSocketUDP.a: socketUDP.o
	ar r $@ $<

libAdresseInternet.a: AdresseInternet.o
	ar r $@ $<
		
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
	
clean:
	$(RM) -vf *~ *.o client server test_tftp_make test_socket_udp

dist-clean:
	$(RM) -vf *~ *.o *.so $(LIBS) $(PRGS)
