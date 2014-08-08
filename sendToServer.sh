#!/bin/bash

file="./file" #the file where you keep your string name
name=$(cat $file)        #the output of 'cat $file' is assigned to the $name variable
echo $name | ssh hostname 'cat >> rtt.txt'
