#!/bin/sh

server="localhost:8778"
conference="def1"
play_file_path="../../Output/ShClnt/"
count=3

for i in `seq 1 $count`
do
    ./ShClnt $server test${i} 1 ${conference} ${play_file_path}p${i}.bmp &
done
