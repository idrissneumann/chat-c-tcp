# Procédure d'installation et d'utilisation

## I - Installation standard

### Prérequis

* Linux ou Mac OS
* libgtk2.0-dev (sous Ubuntu, équivalents chez les autres distributions)
* bash comme shell par défaut
* gcc, make (ou build-essential pour Ubuntu)

### Vérifications des prérequis
    
    [ ~] echo $SHELL # vérifier que à renvoit bien /bin/bash
    [ ~] sudo dpkg -l | grep build-essential # vérifier que le package est bien installé
    [ ~] sudo dpkg -l | grep libgtk2 # vérifier que le package est bien installé

### Installation des prérequis en cas de besoin pour Ubuntu

    [ ~] apt-get update
    [ ~] apt-get install build-essential
    [ ~] apt-get install libgtk2.0-dev

### Installation des prérequis sur Mac OS X

    $ brew update
    $ brew install gtk

### Installation

     # Cloner le repo
     cd src/
     [ ~/chat $] chmod +x configure.sh
     [ ~/chat $] ./configure.sh -b

## Lancement

    [ ~ $] chat-server
    [ ~ $] chat-client
    [ ~ $] chat-shell-client

## II - Installation dégradée (Si vous n'avez pas Gtk)

### Prérequis

* Linux
* gcc, make (ou build-essential pour Ubuntu)

### Vérifications des prérequis
   
    [ ~] sudo dpkg -l | grep build-essential # vérifier que le package est bien installé

### Installation des prérequis en cas de besoin pour Ubuntu

    [ ~] apt-get update
    [ ~] apt-get install build-essential

### Installation

    # cloner le repo
    [ ~ $] cd src
    [ ~/chat $] make server
    [ ~/chat $] make client_shell

### Lancement

    [ ~/chat $] ./server
    [ ~/chat $] ./client_shell

#### Remarque

Si vous avez Gtk d'installé et que vous souhaitez recourir à l'installation dégradée, vous pouvez également faire

    [ ~/chat $] make client # ou make tout court pour tout construire
    [ ~/chat $] ./client # pour lancer le client en mode graphique

## III - utilisation

Pour les différents modes d'installation choisis, il faut :
* Lancer en premier le server (comme indiqué dans les parties précédentes)
* Attendre que le server renvoi "Server is ready". 
* Le serveur indique le port utilisé (1111 si libre, sinon 1112 si libre, sinon 1113 et ainsi de suite)
* Lancer un client et se connecter avec un pseudo et l'ip 127.0.0.1 et le port utilisé par le serveur pour une installation locale
* Si le client renvoit "Connect error", vérifier que le server est bien lancé

Les différentes commandes du chat :
* /QUIT : quitter le chat (l'info de déconnexion est envoyée au server) ... cela revient à fermer l'IHM sur la version graphique
* /STOP : stop le server à distance

Des fichiers de logs sont disponibles pour chaque utilisation. 
Les fichiers concernés se trouvent dans le répertoire dans lequel vous lancez les commandes, ils ont pour noms :

* server-<date>.log : fichier de log server
* client-<date>.log : fichier de log client
* client_shell-<date>.log : fichier de log pour le client en mode shell

Des levels vous permettent de cibler vos recherche dans ces fichiers de logs à l'aide de la commande grep par exemple.

Les différents levels sont les suivants :
* LOG_DEBUG : pour mettre des traces pour debugger
* LOG_ERR : pour les erreurs
* LOG_INFO : garde trace des échanges
* LOG_WARNING : avertissements

## IV - Avertissements

* Ne pas utiliser les exécutables déjà présents (fonctionnent pour une Ubuntu 12.04 LTS en 64 bits)
* Une fois le script configure.sh lancé, l'archive .tar.gz est reconstruite avec les nouveaux exécutables compilés chez vous

