#!/bin/bash

# Print BEFORE message
echo "BEFORE"

# Execute the command to read the disk image
./ds3cat ./tests/disk_images/a2.img 3

./ds3mkdir ./tests/disk_images/a2.img 0 q

# Print AFTER message
echo "AFTER"
./ds3cat ./tests/disk_images/a2.img 3

