Manuel d'utilisation:

Dans un premier temps, il vous faudra compiler les exécutables:
    make server pour compiler le serveur
    make client pour compiler le client
    
Ensuite, lancer le serveur avec ./server puis un client grâce à ./client.
Vous serez ensuite amené à donner l'adresse IP du serveur ainsi que le fichier que vous souhaitez transférer.
Bien qu'il a été prévu de puvoir choisir les options de transfert, notamment la taille de la fenêtre et la taille des blocs,
nous vous conseillons de ne pas les utiliser pusique leurs implémentations n'a pas été réussie.

Le transfert se fait donc sur votre machine dans le dossier "./out/" que vous aurez créé au préalable.
Une indication vous informe de la fin du transfert ainsi que le temps de transfert.
