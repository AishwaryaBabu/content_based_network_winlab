#!/bin/bash

echo "Get content number: $1"
#obtain args from labWiki Experiment
sudo make
sudo ./getIfAddr
sudo ./client connectionsList $1 
