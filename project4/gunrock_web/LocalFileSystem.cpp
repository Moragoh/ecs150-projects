#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <assert.h>

#include "LocalFileSystem.h"
#include "ufs.h"

using namespace std;

LocalFileSystem::LocalFileSystem(Disk *disk)
{
  this->disk = disk;
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
  // Take super objectm, which will have already been obtained from readSuperBlock
  // from super block, get the start address of inode bitmap and length
  // Allocate memmory for the bitmap by length * UFS_BLOCK_SIZE
  // memcpy into type int* array
  // Inode bitmap can now be ready like a usual array

  int mapStart = super->inode_bitmap_addr;
  int mapLength = super->inode_bitmap_len;

  // disk->readBlock(start of inode map, buffer); // Superblock obtained

  for (int i = 0; i < mapLength; i++)
  {
    void *buffer = malloc(UFS_BLOCK_SIZE);
    disk->readBlock(mapStart + i, buffer); // Read current block of bitmap
    memcpy(inodeBitmap + i * UFS_BLOCK_SIZE, buffer, UFS_BLOCK_SIZE);
    free(buffer);
  }
}

void LocalFileSystem::writeInodeBitmap(super_t *super, unsigned char *inodeBitmap)
{
}

void LocalFileSystem::readDataBitmap(super_t *super, unsigned char *dataBitmap)
{
}

void LocalFileSystem::writeDataBitmap(super_t *super, unsigned char *dataBitmap)
{
}

void LocalFileSystem::readInodeRegion(super_t *super, inode_t *inodes)
{
  // Read inode
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
