# Getting started

## Standard installation

### Required dependencies

* Linux or Mac OS
* Gtk
* bash v3+ as default shell
* gcc, make

### Verifying dependencies on Ubuntu
    
```shell
$ sudo dpkg -l | grep build-essential
$ sudo dpkg -l | grep libgtk2
```

### Installing dependencies on Ubuntu

```shell
$ apt-get update
$ apt-get install build-essential
$ apt-get install libgtk2.0-dev
```

### Installating dependencies on Mac OS X

```shell
$ brew update
$ brew install gtk
```

### Installation

```shell
# clone repo or download
$ cd src/
$ chmod +x configure.sh
$ ./configure.sh -b
```

## Running apps

```shell
$ chat-server
$ chat-client
$ chat-shell-client
```

## Installing without gtk

### Required dependencies

* Linux
* bash v3+ as default shell
* gcc, make

### Verifying dependencies on Ubuntu

```shell
$ sudo dpkg -l | grep build-essential
```

### Installing dependencies on Ubuntu

```shell
$ apt-get update
$ apt-get install build-essential
```

### Installation

```shell
# clone repo or download
$ cd src
$ chmod +x configure.sh
$ ./configure.sh -s
```

### Running apps

```shell
$ chat-server
$ chat-shell-client
```

## Using apps

Do the following steps:
* Run server first ;
* Wait until the server display this message : "Server is ready" ;
* The server will display the TCP port to use (1111 or 1112 or 1113 until the port is open and free)
* Run the client with the host "127.0.0.1" and the previous port 

The chat client commands :
* `/QUIT`: quit the chat
* `/STOP`: stop the server

There is log files for each apps with following log messages levels:
* `LOG_DEBUG`: debugging messages
* `LOG_ERR`: error messages
* `LOG_INFO`: informations messages
* `LOG_WARNING`: warning messages

You can use grep on this levels

Enjoy !
