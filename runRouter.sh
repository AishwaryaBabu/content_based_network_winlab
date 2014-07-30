#!/bin/bash

cd /users/content_based_network/
sudo make clean
sudo make
sudo ./getIfAddr
sudo ./router connectionsList
