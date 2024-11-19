#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <assert.h>

#include "LocalFileSystem.h"
#include "ufs.h"

using namespace std;
super_t *super_global = new super_t;

LocalFileSystem::LocalFileSystem(Disk *disk)
{
  this->disk = disk;

  // Fill super in for other functions to use
  readSuperBlock(super_global);
}
// Read super block and allocate the structure accordingly
void LocalFileSystem::readSuperBlock(super_t *super)
{
  void *buffer = malloc(UFS_BLOCK_SIZE);
  disk->readBlock(0, buffer); // Superblock obtained

  memcpy(super, buffer, sizeof(super_t));
  free(buffer);
}

void LocalFileSystem::readInodeBitmap(super_t *super, unsigned char *inodeBitmap)
{
  // Take super object, which will have already been obtained from readSuperBlock
  // from super block, get the start address of inode bitmap and length
  // Allocate memmory for the bitmap by length * UFS_BLOCK_SIZE
  // memcpy into type int* array
  // Inode bitmap is now ready
  // Its in hex, so must convert to binary string to read

  int mapStart = super->inode_bitmap_addr;
  int mapLength = super->inode_bitmap_len;

  void *buffer = malloc(UFS_BLOCK_SIZE);
  for (int i = 0; i < mapLength; i++)
  {
    disk->readBlock(mapStart + i, buffer); // Read current block of bitmap
    memcpy(inodeBitmap + i * UFS_BLOCK_SIZE, buffer, UFS_BLOCK_SIZE);
  }
  free(buffer);

  // Interpreting bitmap: each index is a byte (2 hex decimals). Convert to binary then reverse string to read.
  // Reading bitmap: Take inode // 8 to get which byte to read. inode // 8 to get which index to read.
}

void LocalFileSystem::writeInodeBitmap(super_t *super, unsigned char *inodeBitmap)
{
}

void LocalFileSystem::readDataBitmap(super_t *super, unsigned char *dataBitmap)
{
  int mapStart = super->data_bitmap_addr;
  int mapLength = super->data_bitmap_len;

  void *buffer = malloc(UFS_BLOCK_SIZE);
  for (int i = 0; i < mapLength; i++)
  {
    disk->readBlock(mapStart + i, buffer); // Read current block of bitmap
    memcpy(dataBitmap + i * UFS_BLOCK_SIZE, buffer, UFS_BLOCK_SIZE);
  }
  free(buffer);
}

void LocalFileSystem::writeDataBitmap(super_t *super, unsigned char *dataBitmap)
{
}

void LocalFileSystem::readInodeRegion(super_t *super, inode_t *inodes)
{
  int regionStart = super->inode_region_addr;
  int regionLength = super->inode_region_len;

  void *buffer = malloc(UFS_BLOCK_SIZE);
  for (int i = 0; i < regionLength; i++)
  {
    disk->readBlock(regionStart + i, buffer);
    memcpy(inodes + i * UFS_BLOCK_SIZE, buffer, UFS_BLOCK_SIZE);
  }
  free(buffer);
  /*
    int inode_bitmap_addr; // block address (in blocks)
    int inode_bitmap_len;  // in blocks
    int data_bitmap_addr;  // block address (in blocks)
    int data_bitmap_len;   // in blocks
    int inode_region_addr; // block address (in blocks)
    int inode_region_len;  // in blocks
    int data_region_addr;  // block address (in blocks)
    int data_region_len;   // in blocks
    int num_inodes;        // just the number of inodes
    int num_data;          // and data blocks...
} super_t;
*/
}

void LocalFileSystem::writeInodeRegion(super_t *super, inode_t *inodes)
{
}

int LocalFileSystem::lookup(int parentInodeNumber, string name)
{
  return 0;
}

// Returns metadata of inode
int LocalFileSystem::stat(int inodeNumber, inode_t *inode)
{
  unsigned char *inodeBitmap = (unsigned char *)malloc(super_global->inode_bitmap_len * UFS_BLOCK_SIZE);
  readInodeBitmap(super_global, inodeBitmap);

  inode_t *inodes = new inode_t[super_global->inode_region_len * UFS_BLOCK_SIZE];
  readInodeRegion(super_global, inodes);

  if (inodeBitmap[inodeNumber] == 0)
  {
    cerr << "Inode number is null" << endl;
    free(inodeBitmap);
    delete[] inodes;
    return -EINVALIDINODE;
  }
  else
  {
    *inode = inodes[inodeNumber];
    cout << "Inode size be the " << inode->size << endl;
  }
  free(inodeBitmap);
  delete[] inodes;
  return 0;

  // Takes in an array of inodes
  // Looks up inode bit map to see if valid
  // Throw error if not

  // If valid, goes into inode region and retrieves the inode to populate the inode_t struct with

  // readSuperBlock in to initialize a super object
  // Use inodebitmap to check
  // Use readinoderegion to populate inode
  // Outisde of the function in ds3ls, can get info about inode by doing inode.size
  // you could do that, i just used stat which in turn used readsuperblock, readinodebitmap and readinoderegion
  return 0;
}

int LocalFileSystem::read(int inodeNumber, void *buffer, int size)
{
  return 0;
}

int LocalFileSystem::create(int parentInodeNumber, int type, string name)
{
  return 0;
}

int LocalFileSystem::write(int inodeNumber, const void *buffer, int size)
{
  return 0;
}

int LocalFileSystem::unlink(int parentInodeNumber, string name)
{
  return 0;
}
