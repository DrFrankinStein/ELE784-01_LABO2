#!/bin/bash

read -p "Username > " username
read -p "Email > " email

git config --global user.name "$username"
git config --global user.email "$email"
git config --global credential.helper "cache --timeout=3600" 

git clone https://github.com/DrFrankinStein/ELE784-01_LABO2.git
