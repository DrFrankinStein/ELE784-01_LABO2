#!/bin/bash

git add *
read -p "Message Commit > " text
git commit -m "$text"
#read -e -i "master" -p "Select branch to commit > " branch
#git push origin ${branch:-master}
git push origin master