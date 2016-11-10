#!/bin/bash

git add *
read -p "Commit Title > " title
read -p "Commit Description (Optional) > " text
git commit -m "$title" -m "$text"
#read -e -i "master" -p "Select branch to commit > " branch
#git push origin ${branch:-master}

read -rp "Do you want to send commit to GitHub now? (Y/n) : " -ei "Y" key

if ($key)
	then git push origin master
fi

