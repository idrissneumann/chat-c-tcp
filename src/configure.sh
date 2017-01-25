#!/bin/bash

error() {
    echo "Error : invalid parameter !" >&2
    echo "Use -h opt to get more informations" >&2
    exit 1
}

cleanBin() {
    rm -rf server
    rm -rf client
    rm -rf client_shell
    rm -rf *.log
    make clean
}

createDirIfNeeded() {
    if [[ ! -d ~/bin ]] && [[ $SHELL =~ ^.*/bash ]]; then
        echo "Creating ~/bin..."
        mkdir ~/bin
        echo "export PATH=$PATH:~/bin" >> ~/.bashrc
        source ~/.bashrc
    fi
}

buildShell() {
    cleanBin
    make server
    make client_shell
    createDirIfNeeded
    rm -rf ~/bin/chat-*
    cp server ~/bin/chat-server
    cp client_shell ~/bin/chat-shell-client  
}

buildBin() {
    cleanBin
    make
    createDirIfNeeded
    rm -rf ~/bin/chat-*
    cp server ~/bin/chat-server
    cp client ~/bin/chat-client
    cp client_shell ~/bin/chat-shell-client
}

usage() {
    echo "Usage: ./configure.sh [opts]"
    echo "-h|--help: display helps"
    echo "-b|--build: build binaries"
    echo "-s|--shell: build shell binaries only" 
    echo "-c|--clean: clean binaries"
}

opts=$(getopt -o h,b,s,c -l help,clean,build,shell -- "$@") 
set -- $opts
while true; do 
    case "$1" in 
        -h|--help) usage ; shift ;;
        -b|--build) buildBin ; shift ;;
        -s|--shell) buildShell ; shift ;;   
        -c|--clean) cleanBin ; shift ;;
        --) shift ; break ;;
        *) error ; shift ;;
    esac 
done
