#!/bin/bash

REPO_PATH="${PROJECT_HOME}/chat-c-tcp/"

cd "${REPO_PATH}" && git pull origin master || :
git push github master 
git push pgitlab master
git push bitbucket master
exit 0
