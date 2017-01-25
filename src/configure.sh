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

buildBin() {
    make

    if [[ ! -d ~/bin ]] && [[ $SHELL =~ ^.*/bash ]]; then
        echo "Creating ~/bin..."
        mkdir ~/bin 
        echo "export PATH=$PATH:~/bin" >> ~/.bashrc
        source ~/.bashrc
    fi

    rm -rf ~/bin/chat-*
    cp server ~/bin/chat-server
    cp client ~/bin/chat-client
    cp client_shell ~/bin/chat-shell-client
}

usage() {
    echo "Usage: ./configure.sh [opts]"
    echo "-h: display helps"
    echo "-b: build binaries"
    echo "-c: clean binaries"
}

opts=$(getopt -o h,b,c -l help,clean,build -- "$@") 
set -- $opts
while true; do 
    case "$1" in 
        -h|--help) usage ; shift ;;
        -b|--build) cleanBin ; buildBin ; shift ;;
        -c|--clean) cleanBin ; shift ;;
        --) shift ; break ;;
        *) error ; shift ;;
    esac 
done
