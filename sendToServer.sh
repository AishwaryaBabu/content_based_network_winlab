#!/bin/bash

#file="./file" #the file where you keep your string name
file="/users/ccn_ababu/Client/rtt.txt"
name=$(cat $file)        #the output of 'cat $file' is assigned to the $name variable
echo $name | ssh -i ~/.ssh/id_ccn_rsa $1 "cat >> /users/ababu/rtt.txt"
#echo $name >> "rtt.txt" 
