#!/bin/bash

#obtain args from labWiki Experiment
numContentIds=$#
#contentIdString="$1"

for i in "$@"
do
    contentIdString+="$i "
done
    echo $contentIdString

cd /users/content_based_network/
sudo rm -r "Host/"
sudo make clean
sudo make
sudo ./getIfAddr

sudo mkdir "Host" 
sudo cp host ./"Host"
sudo cp connectionsList ./"Host"
cd ./"Host"
sudo ./host connectionsList $contentIdString 
