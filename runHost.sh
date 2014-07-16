#!/bin/bash

#obtain args from labWiki Experiment
sudo make
sudo ./getIfAddr
sudo mkdir "Host" 
sudo cp host ./"Host"
sudo cp connectionsList ./"Host"
cd ./"Host"
sudo ./host connectionsList

