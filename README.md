# Getting started

## Standard installation

### Required dependancies

* Linux or Mac OS
* Gtk
* bash v3+ as default shell
* gcc, make

### Verifying dependancies on Ubuntu
    
    [ ~] echo $SHELL # check if it's bash
    [ ~] sudo dpkg -l | grep build-essential
    [ ~] sudo dpkg -l | grep libgtk2

### Installing dependancies on Ubuntu

    [ ~] apt-get update
    [ ~] apt-get install build-essential
    [ ~] apt-get install libgtk2.0-dev

### Installating dependancies on Mac OS X

    $ brew update
    $ brew install gtk

### Installation

     # clone repo or download
     cd src/
     [ ~/chat $] chmod +x configure.sh
     [ ~/chat $] ./configure.sh -b

## Running apps

    [ ~ $] chat-server
    [ ~ $] chat-client
    [ ~ $] chat-shell-client

## Installing without gtk

### Required dependancies

* Linux
* bash v3+ as default shell
* gcc, make

### Verifying dependancies on Ubuntu
   
    [ ~] echo $SHELL # check if it's bash
    [ ~] sudo dpkg -l | grep build-essential

### Installing dependancies on Ubuntu

    [ ~] apt-get update
    [ ~] apt-get install build-essential

### Installation

    # clone repo or download
    [ ~ $] cd src
    [ ~/chat $] chmod +x configure.sh
    [ ~/chat $] ./configure.sh -s

### Running apps

    [ ~/chat $] chat-server
    [ ~/chat $] chat-shell-client


## Using apps

Do the following steps:
* Run server first ;
* Wait until the server display this message : "Server is ready" ;
* The server will display the TCP port to use (1111 or 1112 or 1113 until the port is open and free)
* Run the client with the host "127.0.0.1" and the previous port 

The chat client commands :
* /QUIT : quit the chat
* /STOP : stop the server


Des fichiers de logs sont disponibles pour chaque utilisation. 
Les fichiers concernés se trouvent dans le répertoire dans lequel vous lancez les commandes, ils ont pour noms :

* server-<date>.log : fichier de log server
* client-<date>.log : fichier de log client
* client_shell-<date>.log : fichier de log pour le client en mode shell

There is log files for each apps with following log messages levels:
* LOG_DEBUG : debugging messages
* LOG_ERR : error messages
* LOG_INFO : informations messages
* LOG_WARNING : warning messages

You can use grep on this levels

Enjoy !

