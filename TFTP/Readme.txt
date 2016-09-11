Vous pouvez lancer la compilation en deux étapes.
Dans un terminal faite d'abords :
make
Puis:
make Client

Vous pouvez supprimer les .o et les executables avec:
make clean

Pour lancer le serveur, faite
./Server numport
Ou numport est un numéro de port sur lequel le serveur va écouter
Ex : ./Server 5555

Pour lancer le client, faite
./Client ip numport -b blksize -w windowsize fichierorigin fichierdest

Ou ip est l'ip du serveur, numport son port d'ecoute, blksize la taille des blocs, windowsize la taille de la fenetre d'envoie fichierorigin le chemin du fichier d'origine et fichierdest, le chemin du fichier de destination

Ex : ./Client localhost 5555 -b  400 -w 5 test.txt test2.txt
