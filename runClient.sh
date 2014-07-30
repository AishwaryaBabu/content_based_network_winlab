#!/bin/bash

cd /users/content_based_network/
echo "Get content number: $1"
#obtain args from labWiki Experiment
sudo make clean
sudo make
sudo ./getIfAddr
sudo mkdir "Client" 
sudo cp client ./"Client"
sudo cp connectionsList ./"Client"
cd ./"Client"
sudo ./client connectionsList $1 
