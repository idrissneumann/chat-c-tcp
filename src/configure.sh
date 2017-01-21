#!/bin/bash

make

if [ ! -d ~/bin ] && [ "$SHELL" = "/bin/bash" ]; then
    mkdir ~/bin 
    echo "PATH=$PATH:~/bin" >> ~/.bashrc
    source ~/.bashrc
fi

rm -rf ~/bin/chat-*
cp server ~/bin/chat-server
cp client ~/bin/chat-client
cp client_shell ~/bin/chat-shell-client
tar cvfz ../chat_neumann_lavergne.tar.gz ../chat