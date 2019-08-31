#!/bin/bash

clients=$(grep "Connect: OK" testout.log | wc -l)
echo Client totali connessi : $clients

echo ----------

storecl=$(grep -Pzo "Connect: OK \nStore:" testout.log | wc -l)
echo Client che hanno effettuato store: $storecl

store_ex=$(grep "Store: " testout.log | wc -l)
echo Store totali eseguite: $store_ex

store_s=$(grep "Store: dati salvati" testout.log | wc -l)
echo Store con successo: $store_s

store_f=$(grep "Store: dati non" testout.log | wc -l)
echo Store fallite: $store_f

echo ----------

retrievecl=$(grep -Pzo "Connect: OK \nRetrieve: " testout.log | wc -l)
echo Client che hanno effettuato retrieve: $retrievecl

retrieve_ex=$(grep "Retrieve: " testout.log | wc -l)
echo Retrieve totali eseguite: $retrieve_ex

retrieve_s=$(grep "Retrieve: recupero dati riuscito" testout.log | wc -l)
echo Retrieve con successo: $retrieve_s

retrieve_f=$(grep "Retrieve: recupero dati fallito" testout.log | grep "Retrieve: dati" testout.log | wc -l)
echo Retrieve fallite: $retrieve_f

echo ----------

deletecl=$(grep -Pzo "Connect: OK \nDelete" testout.log | wc -l)
echo Client che hanno effettuato retrieve: $deletecl

delete_ex=$(grep "Delete: " testout.log | wc -l)
echo Delete totali eseguite: $delete_ex

delete_s=$(grep "Delete: dati rimossi" testout.log | wc -l)
echo Delete con successo: $delete_s

delete_f=$(grep "Delete: dati non" testout.log | wc -l)
echo Delete fallite: $delete_f

echo ----------