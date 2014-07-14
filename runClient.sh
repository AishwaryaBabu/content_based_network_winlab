#!/bin/bash

#obtain args from labWiki Experiment
sudo make

sudo mkdir "Host" 
cp host ./"Host"
cd ./"Host"
sudo ./client connectionsList

