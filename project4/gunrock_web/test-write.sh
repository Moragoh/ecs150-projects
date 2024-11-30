#!/bin/bash

# Print BEFORE message
echo "START\n"

./ds3cat ./tests/disk_images/a2.img 3
echo "\n"
./ds3bits ./tests/disk_images/a2.img 

echo "AFTER LONG WRITE\n"
./ds3mkdir ./tests/disk_images/a2.img 1
echo "\n"
./ds3cat ./tests/disk_images/a2.img 3
echo "\n"
./ds3bits ./tests/disk_images/a2.img 

echo "AFTER SHORT WRITE\n"
./ds3mkdir ./tests/disk_images/a2.img 0
echo "\n"
./ds3cat ./tests/disk_images/a2.img 3
echo "\n"
./ds3bits ./tests/disk_images/a2.img 

