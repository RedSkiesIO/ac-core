#!/bin/bash


while [ true ]; do

n=`bin/botnet_ctl random_node`
#n="10.84.172.50"
echo "transfer to $n:"
us-wallet -whost $n transfer 3cjyGYdhH9qBZNss7B9xWdmrt8L 1
#sleep 0.1


done
