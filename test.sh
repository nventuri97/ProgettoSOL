#!/bin/bash
@echo 'Inizio fase di test'
for ((i=0;i<50;i++)); do
    ./client.o client$i 1 >>testou.log &
done

wait

for ((i=0;i<30;i++)); do
    ./client.o client$i 2 >>testou.log &
done

for ((i=30;i<50;i++)); do
    ./client.o client$i 3 >>testou.log &
done

wait