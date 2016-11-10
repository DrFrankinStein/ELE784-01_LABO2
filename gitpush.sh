#!/bin/bash

git add *
read -p "Commit Title > " title
read -p "Commit Description (Optional) > " text
git commit -m "$title" -m "$text"
#read -e -i "master" -p "Select branch to commit > " branch
#git push origin ${branch:-master}

while true
do
read -rp "Do you want to send commit to GitHub now? (Y/n) : " key

case $key in
y|Y) git push origin master
     exit;;
n|N) exit;;
esac
done

