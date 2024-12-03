#!/bin/bash

# Print BEFORE message
echo "START==================="

./ds3cat ./tests/disk_images/cpy.img 3
echo ""
echo ""
./ds3bits ./tests/disk_images/cpy.img 
echo ""

echo "AFTER==================="
echo ""
./ds3cp ./tests/disk_images/cpy.img ./zTestShort.txt 3
echo ""
./ds3cat ./tests/disk_images/cpy.img 3
echo ""
echo ""
./ds3bits ./tests/disk_images/cpy.img 
