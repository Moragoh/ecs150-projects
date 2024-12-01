#!/bin/bash

# Print BEFORE message
echo "START\n"

./ds3ls ./tests/disk_images/a3.img /a/b
echo "\n"
./ds3bits ./tests/disk_images/a3.img 

echo "AFTER Create\n"
./ds3touch ./tests/disk_images/a3.img 2 test
./ds3ls ./tests/disk_images/a3.img /a/b
echo "\n"
./ds3bits ./tests/disk_images/a3.img 
