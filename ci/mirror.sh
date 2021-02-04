#!/bin/bash

REPO_PATH="/home/centos/chat-c-tcp/"

cd "${REPO_PATH}" && git pull origin master || :
git push github master 
git push pgitlab master
exit 
0
