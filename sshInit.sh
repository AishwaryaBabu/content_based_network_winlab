#!/bin/bash

ssh-keygen -t rsa -N "" -f /root/.ssh/id_ccn_rsa
ssh-copy-id -i /root/.ssh/id_ccn_rsa.pub $1
#ssh -o StrictHostKeyChecking=no $1
ssh-keyscan -t rsa >> /root/.ssh/known_hosts
