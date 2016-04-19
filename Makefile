CC = gcc
CFLAGS = -Wall -Werror -Wextra -pedantic -std=c11 -D_XOPEN_SOURCE 

# définition des bibliothèques à utiliser lors de l'édition de lien de
# droite à gauche par ordre de dépendance. Les bibliothèques sont
# recherchées dans les répertoires système standard, plus les
# répertoires indiqués avec l'option -L dans les LDFLAGS.  L'éditeur
# de lien utilisera, en fonction des fichiers trouvés, soit: les
# bibliothèques dynamiques (libAdresseInternet.so), statiques
# (libAdresseInternet.a) ou les fichiers objets (AdresseInternet.o)
LDLIBS = -lAdresseInternet
LIBS = libsocketUDP.a

# non standard, mais on peut définir une macro contenant tous les
# exécutables du projet:
#PRGS = test

# la première cible est celle qui sera exécutée par défaut si on
# exécute "make" sans argument.
all: $(LIBS)

libsocketUDP.a: socketUDP.o
	ar r $@ $<
	
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
	
clean:
	$(RM) -vf *~ *.o

dist-clean:
	$(RM) -vf *~ *.o *.so $(LIBS)
