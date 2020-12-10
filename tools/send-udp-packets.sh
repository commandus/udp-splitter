#!/bin/bash
A=84.237.104.128
P=2000
for (( i = 1; i < 100; i++ ))
do
echo "$i"
echo -n "A$i" | nc -u -q 1 $A $P
sleep 1
done
