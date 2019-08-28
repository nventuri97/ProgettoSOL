#!/bin/bash
echo 'Inizio fase di test'
for ((i=0;i<5;i++)); do
    ./client.o client$i 1 >>testout.log &
done

wait

for ((i=0;i<3;i++)); do
    ./client.o client$i 2 >>testout.log &
done

for ((i=30;i<2;i++)); do
    ./client.o client$i 3 >>testout.log &
done

wait