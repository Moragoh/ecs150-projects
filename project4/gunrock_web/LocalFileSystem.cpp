#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <bitset>
#include <algorithm>
#include <assert.h>

#include "LocalFileSystem.h"
#include "ufs.h"

using namespace std;
super_t *super_global = new super_t;

// Prepares inodes with the wanted changed for that specific inode and writes it back using writeBlock
void changeInodeSize(int inum, int newSize, LocalFileSystem &fs)
{
  inode_t *inodes = (inode_t *)malloc(super_global->num_inodes * sizeof(inode_t));
  inode_t *inode = new inode_t;
  // Obtains current state of inodes. Now just have to replace it with inodes
  fs.readInodeRegion(super_global, inodes);

  // Use stat to get specific inode
  fs.stat(inum, inode);
  inode->size = newSize;

  // Find where in inodes the inode specified by inum is
  inodes[inum] = *inode;

  fs.writeInodeRegion(super_global, inodes);
  free(inodes);
  delete inode;
}

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

// Each index is a byte, which represents 8 inodes
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
}

void LocalFileSystem::writeInodeBitmap(super_t *super, unsigned char *inodeBitmap)
{
  // inodeBitMap is the new bitmap you want to write
  // memcpy inodeBitmap into the start of the bitmap (super->inodebitmap_start_addr)
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
  // How to prepare dataBitmap
  // Use readMap to get map
  // Use indexing logic (used in stat) to write specific one

  // dataBitmap is the thing we want to write
  // Get data map start addr
  // Write block
}

void LocalFileSystem::readInodeRegion(super_t *super, inode_t *inodes)
{
  int regionStart = super->inode_region_addr;
  int regionLength = super->inode_region_len;

  char *buffer = (char *)malloc(UFS_BLOCK_SIZE);
  for (int i = 0; i < regionLength; i++)
  {
    disk->readBlock(regionStart + i, buffer);
    // Block obtained. Now we need to read by inode from the block
    int inodesPerBlock = UFS_BLOCK_SIZE / sizeof(inode_t);

    for (int j = 0; j < inodesPerBlock; j++)
    {
      // Get where inode needs to be copied from
      void *inodeStart = buffer + (j * sizeof(inode_t));

      // Get the position in the inodes array it should be copied to
      // inodes is chunked in inode_t and not bytes
      void *dest = inodes + (i * inodesPerBlock) + j;
      memcpy(dest, inodeStart, sizeof(inode_t));
    }
  }
  free(buffer);
}

// Used for updating metadata
void LocalFileSystem::writeInodeRegion(super_t *super, inode_t *inodes)
{
  // Use to persist changes to inodes. How? By preparing inodes with the wanted changes and using writeBlock
  int size = super->num_inodes * sizeof(inode_t); // Number of bytes in inodes

  int blockCount = size / UFS_BLOCK_SIZE;
  if ((size % UFS_BLOCK_SIZE) != 0)
  {
    blockCount += 1;
  }

  int inodesPerBlock = UFS_BLOCK_SIZE / sizeof(inode_t);
  char *tempBuffer = (char *)malloc(UFS_BLOCK_SIZE);
  if (tempBuffer == NULL)
  {
    cerr << "Mem allocation failed" << endl;
    exit(1);
  }

  for (int i = 0; i < blockCount; i++)
  {
    // Iterating through each inode of the ith block
    for (int j = 0; j < inodesPerBlock; j++)
    {
      // Get where inode needs to be copied from
      // inodes is in inode_t, so each count is the size of an inode_t (why we only add j)
      void *inodeStart = inodes + i * inodesPerBlock + j; // decides which inode to copy from

      // Copy the inode into the tempBuffer before using writeBlock©
      void *dest = tempBuffer + (j * sizeof(inode_t));

      // Copy one inode's worth of data into tempBuffer
      memcpy(dest, inodeStart, sizeof(inode_t));
    }
    // Tempbuffer now holds one block's worth of inodes. Ready to be written in using writeBlock
    disk->beginTransaction();
    disk->writeBlock(super->inode_region_addr + i, tempBuffer);
    disk->commit();
  }

  free(tempBuffer);
}

int LocalFileSystem::lookup(int parentInodeNumber, string name)
{
  inode_t *inode = new inode_t;
  int ret = stat(parentInodeNumber, inode);
  if (ret == -EINVALIDINODE)
  {
    delete inode;
    return -EINVALIDINODE;
  }

  // Not a directory
  if (inode->type != 0)
  {
    cerr << "Given parent inode is not a directory" << endl;
    delete inode;
    return -EINVALIDINODE;
  }

  // Get the directory entries of parentInodeNumber
  // Use inode.size to get the size for the read
  int fileSize = inode->size;
  void *buffer[fileSize];
  read(parentInodeNumber, buffer, fileSize);

  // buffer contains directory contents
  dir_ent_t *dirBuffer = (dir_ent_t *)buffer;
  int entryCount = fileSize / sizeof(dir_ent_t);

  int res = 0;
  // Iterate through entryCount and compare the name
  for (int i = 0; i < entryCount; i++)
  {
    char *fileName = (char *)dirBuffer[i].name;
    // cout << fileName << endl;
    if (strcmp(name.c_str(), fileName) == 0)
    {
      // Strings match
      res = 1;
      delete inode;
      // cout << "Inode found:  " << dirBuffer[i].inum << endl;
      return dirBuffer[i].inum;
    }
  }

  if (res == 0)
  {
    // cerr << "Inode of that name is not found" << endl;
    delete inode;
    return -ENOTFOUND;
  }
  delete inode;
  return 0;
}

// Returns metadata of inode
int LocalFileSystem::stat(int inodeNumber, inode_t *inode)
{
  unsigned char *inodeBitmap = (unsigned char *)malloc(super_global->inode_bitmap_len * UFS_BLOCK_SIZE);
  readInodeBitmap(super_global, inodeBitmap);

  inode_t *inodes = new inode_t[super_global->inode_region_len * UFS_BLOCK_SIZE];
  readInodeRegion(super_global, inodes);

  // Interpreting bitmap: each index is a byte (2 hex decimals). Convert to binary then reverse string to read.
  // Reading bitmap: Take inode // 8 to get which byte to read. inode // 8 to get which index to read.
  int byteToRead = inodeNumber / 8;
  int byteInDec = (int)inodeBitmap[byteToRead];
  string byteInBin = bitset<8>(byteInDec).to_string();

  // Reverse string to follow the project's structure
  reverse(byteInBin.begin(), byteInBin.end());
  int byteOffset = inodeNumber % 8;

  char status = byteInBin[byteOffset];
  string statusStr(1, status);
  // TODO: Need to unpack the inodebitmap
  if (statusStr != "1")
  {
    // cerr << "Inode number is null" << endl;
    free(inodeBitmap);
    delete[] inodes;
    return -EINVALIDINODE;
  }
  else
  {
    *inode = inodes[inodeNumber];
  }
  free(inodeBitmap);
  delete[] inodes;
  return 0;
}

int LocalFileSystem::read(int inodeNumber, void *buffer, int size)
{
  if (size < 0)
  {
    return -EINVALIDSIZE;
  }

  // Given inodeNum, retrieve through stat
  inode_t *inode = new inode_t;
  int ret = stat(inodeNumber, inode); // If fileSize was 0, stat would have caught that because bitmap would have been marked
  if (ret == -EINVALIDINODE)
  {
    return -EINVALIDINODE;
  }

  // Get size of inode's file
  int fileSize = inode->size;
  // Get type of inode
  int inodeType = inode->type;

  // if type isn directory, direct pointers point to dir_ent, so cast buffer into dir_ents
  if (inodeType == 0)
  {
    // Directory
    buffer = (dir_ent_t *)buffer;
  }
  // else keep as is.

  // See which is bigger--size of inodeSize
  int amountToRead = min(fileSize, size);
  int remaining = amountToRead;
  int blocksToIterate = amountToRead / UFS_BLOCK_SIZE; // Number of blocks this needs to iterate through
  int resultDest = 0;                                  // Pos in resulting buffer to copy to

  void *tempBuffer = malloc(UFS_BLOCK_SIZE);
  for (int i = 0; i <= blocksToIterate; i++)
  {
    int blockNum = inode->direct[i]; // Get the block number we must read from

    // TODO: Before readblock, check data region
    disk->readBlock(blockNum, tempBuffer); // Read in the whole block

    int currCopyAmount = min(remaining, UFS_BLOCK_SIZE); // This allows us to only copy the part that we need from buffer

    // Only copy rethe necessary bits (may be whole block or less than a block)
    memcpy((char *)buffer + resultDest, tempBuffer, currCopyAmount);

    // Update vars as needed
    remaining -= currCopyAmount;
    resultDest += currCopyAmount;
  }
  delete (inode);
  free(tempBuffer);

  return resultDest;
  //  TODO: IF INCOMPLETE READ, SHOULD IT ONLY RETURN COMPLETE DIR OBJECTS?
}

int LocalFileSystem::create(int parentInodeNumber, int type, string name)
{
  return 0;
}

/**
 * Write the contents of a file.
 *
 * Writes a buffer of size to the file, replacing any content that
 * already exists.
 *
 * Success: number of bytes written
 * Failure: -EINVALIDINODE, -EINVALIDSIZE, -EINVALIDTYPE.
 * Failure modes: invalid inodeNumber, invalid size, not a regular file
 * (because you can't write to directories).
 */
int LocalFileSystem::write(int inodeNumber, const void *buffer, int size)
{
  inode_t *inode = new inode_t;
  int ret = stat(inodeNumber, inode);

  /*ERROR CHECKING*/
  // Check ret to return 1 with error string
  if (ret == -EINVALIDINODE || ret == -EINVALIDSIZE)
  {
    // Invalid inode
    delete inode;
    //-EINVALIDINODE, -EINVALIDSIZE, -EINVALIDTYPE.
    return -EINVALIDINODE;
  }

  if (size < 0)
  {
    delete inode;
    return -EINVALIDTYPE;
  }

  if (inode->type == 0)
  {
    delete inode;
    return -EINVALIDTYPE;
  }

  // Determine if new buffer is less than, the same, or more blocks
  int fileSize = inode->size;
  int currBlockCount = fileSize / UFS_BLOCK_SIZE;
  if ((fileSize % UFS_BLOCK_SIZE) != 0)
  {
    currBlockCount += 1;
  }

  int newBlockCount = size / UFS_BLOCK_SIZE;
  if ((size % UFS_BLOCK_SIZE) != 0)
  {
    newBlockCount += 1;
  }

  // Uses the same number of blocks
  if (newBlockCount == currBlockCount)
  {
    // Iterate through direct and get the current blocks that are in use
    vector<int> blocksInUse;
    for (int i = 0; i < currBlockCount; i++)
    {
      // cout << inode->direct[i] << endl;
      blocksInUse.push_back(inode->direct[i]);
    }

    // If block number same, all we have to do is foreach block, clear out and write
    // then update inode size. Bitmap should remain the same
    // int remaining = size;
    // int copyAmount;
    // int currPos = 0;

    // Buffer of null values to use for emptying out the block
    void *emptyBuffer = malloc(UFS_BLOCK_SIZE);

    for (int i = 0; i < currBlockCount; i++)
    {
      int blockNum = blocksInUse[i];

      // Clear out block
      disk->writeBlock(blockNum, emptyBuffer);
      cout << "Emptied out block" << endl;
      // Updating inode size
      changeInodeSize(inodeNumber, 20, *this);

      // Write to block
    }
    free(emptyBuffer);
  }

  delete inode;
  return 0;
} // Convert the file size to how many blocks of data it would require, then iterate through direct using that count

// When to use transaction?

// First determine if the number of allocated blocks change or not
// Get how many blocks current inode takes up
// Get how many size would take up

// Come up with 3 cases: same, more, less
// Same
// Get currentBlocks (current blocks in use from fileSize / block size) and then do inode->direct
// Iterate through currentBlocks
// Keep track of remaining. If remaining >= blockSize, copyAmount = blockSize. Else remaining
// Make tempbuffer of blockSize

// Before write, always ret = beginTransaction
// if writeblock successful, commit it. Else rollback and therow error
// if ret, then commit transaction. Else rollback

// memcpy buffer into tempbuffer and writeBlock
// Update bufferPos

// To test: first check output, then ds3bits to see that they match

int LocalFileSystem::unlink(int parentInodeNumber, string name)
{
  return 0;
}

/*
Things to test

1. Delete -> Try to access inode and see if it throws error
*/