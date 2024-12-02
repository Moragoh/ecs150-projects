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

/*
MAIN ISSUES: MUST HANDLE CASE WHERE DIRECTORY IS BRAND NEW AND THEREFORE HAS NO BLOCKS. HANDLE CASE WHERE FILESIZE = 0, ELSE KEEP SAME
*/
// Updates contents of a directory
int writeToDirectory(int parentInodeNumber, const void *newDirBuffer, int size, LocalFileSystem &fs)
{
  // There is only a case where we would have to allcoate one extra block

  inode_t *parentInode = new inode_t;
  fs.stat(parentInodeNumber, parentInode); // Get the parent directory that we have to update

  // Determine if new buffer is less than, the same, or more blocks
  int fileSize = parentInode->size; // This corerctly retrieves the new inode, which a directory with a size of 0 with non updated direct points
  int total = 0;
  delete parentInode;
  if (fileSize == 0)
  {
    parentInode = new inode_t;
    fs.stat(parentInodeNumber, parentInode);

    // Brand new inode. Must assign it blocks for its direct[] and write
    unsigned char *dataBitmap = (unsigned char *)malloc(super_global->data_bitmap_len * UFS_BLOCK_SIZE);
    fs.readDataBitmap(super_global, dataBitmap);

    int newBlock;
    int enoughSpace = 0;
    int startBlock = super_global->data_region_addr;
    int endBlock = super_global->data_region_addr + super_global->data_region_len;
    for (int i = startBlock; i < endBlock; i++)
    {
      // Remember: i here is the absolute block number
      int bitToCheck = (i - (super_global->data_region_addr));
      int byteNum = bitToCheck / 8;
      int byteToCheck = (int)dataBitmap[byteNum];

      string byteInBin = bitset<8>(byteToCheck).to_string();
      int byteOffset = bitToCheck % 8;

      char status = byteInBin[byteInBin.size() - 1 - byteOffset];
      string statusStr(1, status);
      if (statusStr != "1")
      {
        newBlock = i;
        enoughSpace = 1;
        break;
      }
    }

    // We know which block we can use
    if (enoughSpace == 0)
    {
      cerr << "Not enough space!" << endl;
    }

    // We know which free block the new inode can use, so give it assign its first direct pointer that inode and write to it
    // We need to copy newDirBuffer into a block buffer, then  write that block into newBlock

    void *blockBuffer = malloc(UFS_BLOCK_SIZE);
    memcpy(blockBuffer, newDirBuffer, size); // In this case, it is copying dot and dotDot

    // Clear out block
    void *emptyBuffer = malloc(UFS_BLOCK_SIZE);
    memset(emptyBuffer, '\0', UFS_BLOCK_SIZE);
    fs.disk->writeBlock(newBlock, emptyBuffer);
    free(emptyBuffer);

    // Write block with new data
    fs.disk->writeBlock(newBlock, blockBuffer);
    total += size;
    free(blockBuffer);

    cout << "parentInode before size: " << parentInode->size << endl;

    /*
    Inode size update
    */
    cout << "Updating inode size to " << total << endl;
    changeInodeSize(parentInodeNumber, total, fs);

    /*
    Now we must update the parentInode's direct
    */
    // Since it's a new inode, we always write to first
    parentInode->direct[0] = newBlock;

    // Persist direct pointer change to disk
    inode_t *inodes = (inode_t *)malloc(super_global->num_inodes * sizeof(inode_t));
    // Obtains current state of inodes. Now just have to replace it with inodes
    fs.readInodeRegion(super_global, inodes);

    // Find where in inodes the inode specified by inum is
    // Check if it persisted
    parentInode = new inode_t;
    fs.stat(parentInodeNumber, parentInode);  // From this point, parentInode is the one witht he updated size
    inodes[parentInodeNumber] = *parentInode; // Issue: this grabs the old parentInode with size of 0. We must grab one that has persisted through stat
    fs.writeInodeRegion(super_global, inodes);
    free(inodes);

    // Check if it persisted
    inode_t *testInode = new inode_t;
    fs.stat(parentInodeNumber, testInode);

    cout << "Testinode block " << testInode->direct[0] << endl;
    cout << "testinode size " << testInode->size << endl;
    delete testInode;

    /*
    Update data bitmap
    */

    int bitToCheck = (newBlock - (super_global->data_region_addr));
    int byteNum = bitToCheck / 8;
    int byteToCheck = (int)dataBitmap[byteNum];
    int byteOffset = bitToCheck % 8;
    string byteInBin = bitset<8>(byteToCheck).to_string();

    // Access that byte and modify the value
    byteInBin[byteInBin.size() - 1 - byteOffset] = '1';

    // cout << "changed byteInBin: " << byteInBin << endl;

    // Change that byteInBin to int again, then write it back as dataBitmap[byteNum]
    int byteInInt = stoi(byteInBin, nullptr, 2);
    unsigned char byteInChar = (unsigned char)byteInInt;
    // cout << byteInInt << endl;
    // cout << (int)byteInChar << endl;
    dataBitmap[byteNum] = byteInChar;

    // Write new dataBitmap using writeDatabitmap
    fs.writeDataBitmap(super_global, dataBitmap);
    free(dataBitmap);
    delete parentInode;
  }
  else
  {
    parentInode = new inode_t;
    fs.stat(parentInodeNumber, parentInode);

    int currBlockCount = fileSize / UFS_BLOCK_SIZE;
    if ((fileSize % UFS_BLOCK_SIZE) != 0)
    {
      currBlockCount += 1;
    }

    int sizeToWrite = size;
    int newBlockCount = sizeToWrite / UFS_BLOCK_SIZE;
    if ((fileSize % UFS_BLOCK_SIZE) != 0)
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

        // HANDLE CORNER CASE WHERE FILESIZE 0, MEANING THAT THE DIRECTORY IS BRAND NEW AND NEW ONES MUST BE ASSIGNED
        // This does not work when the size is 0--if so, there is nothign written, so we need to go tghrough the logic iof assigning direct pointers here
        blocksInUse.push_back(parentInode->direct[i]);
      }

      // If block number same, all we have to do is foreach block, clear out and write
      // then update inode size. Bitmap should remain the same
      int remaining = sizeToWrite;
      int copyAmount;
      // Buffer of null values to use for emptying out the block
      void *emptyBuffer = malloc(UFS_BLOCK_SIZE);
      memset(emptyBuffer, '\0', UFS_BLOCK_SIZE);
      // Clear and write each block at a time
      for (int i = 0; i < currBlockCount; i++)
      {
        // Determine how much to write: a full block's worth or less
        if (remaining > UFS_BLOCK_SIZE)
        {
          copyAmount = UFS_BLOCK_SIZE;
        }
        else
        {
          copyAmount = remaining;
        }

        void *blockBuffer = malloc(UFS_BLOCK_SIZE);
        void *currBlockToCopy = (char *)newDirBuffer + i * UFS_BLOCK_SIZE; // Determines which block's worth of data from buffer should be copied in
        memcpy(blockBuffer, currBlockToCopy, copyAmount);

        // Reuse existing blokcs
        int blockNum = blocksInUse[i];
        // Clear out block
        fs.disk->writeBlock(blockNum, emptyBuffer); // writeBlock must ALWAYS write a block at a time

        // Write block with new data
        fs.disk->writeBlock(blockNum, blockBuffer);
        remaining -= copyAmount;
        total += copyAmount;
        free(blockBuffer);
      }
      // Updating inode size
      // At first pass, this updates inode size to 0
      cout << "Updating inode size to " << total << endl;
      changeInodeSize(parentInodeNumber, total, fs);
      free(emptyBuffer);
    }
    delete parentInode;
  }
  return total;
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
  int mapStart = super->inode_bitmap_addr;
  int mapLength = super->inode_bitmap_len; // The bitmap is mapLegth blocks long

  void *buffer = malloc(UFS_BLOCK_SIZE);
  for (int i = 0; i < mapLength; i++)
  {
    // Gets a block of dataBitmap into buffer
    memcpy(buffer, inodeBitmap + i * UFS_BLOCK_SIZE, UFS_BLOCK_SIZE);
    disk->writeBlock(mapStart + i, buffer);
  }
  free(buffer);
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
  int mapStart = super->data_bitmap_addr;
  int mapLength = super->data_bitmap_len; // The bitmap is mapLegth blocks long

  void *buffer = malloc(UFS_BLOCK_SIZE);
  for (int i = 0; i < mapLength; i++)
  {
    // Gets a block of dataBitmap into buffer
    memcpy(buffer, dataBitmap + i * UFS_BLOCK_SIZE, UFS_BLOCK_SIZE);
    disk->writeBlock(mapStart + i, buffer);
  }
  free(buffer);
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
  cerr << "This should not be printed at the end of lookup" << endl;
  return 0;
  delete inode;
}

// Returns metadata of inode
int LocalFileSystem::stat(int inodeNumber, inode_t *inode)
{
  unsigned char *inodeBitmap = (unsigned char *)malloc(super_global->inode_bitmap_len * UFS_BLOCK_SIZE);
  readInodeBitmap(super_global, inodeBitmap);

  inode_t *inodes = new inode_t[super_global->inode_region_len * UFS_BLOCK_SIZE];
  readInodeRegion(super_global, inodes);

  // Interpreting bitmap: each index is a byte (2 hex decimals). Convert to binary then reverse string to read.
  // Reading bitmap: Take inode // 8 to get which byte to read. inode % 8 to get which index to read.
  int byteToRead = inodeNumber / 8;
  int byteInDec = (int)inodeBitmap[byteToRead]; //  This is a byte.
  string byteInBin = bitset<8>(byteInDec).to_string();

  int byteOffset = inodeNumber % 8;

  // Bits go from right to left, so must index from the right
  // char status = byteInBin[byteInBin.size() - byteOffset];
  char status = byteInBin[byteInBin.size() - 1 - byteOffset];

  string statusStr(1, status);
  // TODO: Need to unpack the inodebitmap
  if (statusStr != "1")
  {
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

  int blocksToIterate = amountToRead / UFS_BLOCK_SIZE;
  if ((amountToRead % UFS_BLOCK_SIZE) != 0)
  {
    blocksToIterate += 1;
  }
  int resultDest = 0;

  void *tempBuffer = malloc(UFS_BLOCK_SIZE);
  for (int i = 0; i < blocksToIterate; i++)
  {
    int blockNum = inode->direct[i]; // Get the block number we must read from
    // cout << blockNum << endl;
    // FOr ds3, fails here--it seems that the blockNum is not updating properly
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

  /* Error Checking */
  inode_t *inode = new inode_t;
  int ret = stat(parentInodeNumber, inode);

  if (ret != 0)
  {
    delete inode;
    return -EINVALIDINODE;
  }

  if (inode->type != 0)
  {
    return -EINVALIDINODE;
  }

  if (name.length() > DIR_ENT_NAME_SIZE)
  {
    return -EINVALIDNAME;
  }

  // Check if inode already exists
  inode_t *target = new inode_t;
  int lookupRet = lookup(parentInodeNumber, name);

  // Inode not found, so now look if there is enough space to create one
  if (lookupRet == -ENOTFOUND)
  {
    // Check if there is enough space by checking the inodeBitmap
    // DEBUG: Entered here correctly under not finding test. Where does it segfault?
    unsigned char *inodeBitmap = (unsigned char *)malloc(super_global->inode_bitmap_len * UFS_BLOCK_SIZE);
    readInodeBitmap(super_global, inodeBitmap);

    // Similar to checking the data bitmap for write. Iterate through the inode bitMap and pick out the free one
    int enoughSpaceToCreate = 0;
    int inodeNumToCreate;

    for (int i = 0; i < super_global->num_inodes; i++)
    {
      int byteNum = i / 8;
      int byteToCheck = (int)inodeBitmap[byteNum];

      int byteOffset = i % 8;
      string byteInBin = bitset<8>(byteToCheck).to_string();

      char targetBit = byteInBin[byteInBin.size() - 1 - byteOffset];
      if (targetBit == '0')
      { // Corerctly gets that i=4 is the first free inode
        enoughSpaceToCreate = 1;
        inodeNumToCreate = i;
        break;
      }
    }
    free(inodeBitmap);

    if (enoughSpaceToCreate)
    {
      // Create the new inode
      inode_t *newInode = new inode_t;

      if (type == 0)
      {
        // Creating a new inode
        newInode->type = 0;
        newInode->size = 0;

        /*
        Writing new inode to disk
        */
        inode_t *inodes = (inode_t *)malloc(super_global->num_inodes * sizeof(inode_t));
        // Obtains current state of inodes. Now just have to replace it with inodes
        readInodeRegion(super_global, inodes);

        // Find where in inodes the inode specified by inum is
        inodes[inodeNumToCreate] = *newInode; // inode here is the aprent

        writeInodeRegion(super_global, inodes);
        free(inodes);

        /*
        Updating inode bitmap
        */
        inodeBitmap = (unsigned char *)malloc(super_global->inode_bitmap_len * UFS_BLOCK_SIZE);
        readInodeBitmap(super_global, inodeBitmap);

        // Update the inodeBitmap to set inodeNumToCreate to 1
        int byteNum = inodeNumToCreate / 8;
        int byteToCheck = (int)inodeBitmap[byteNum];

        int byteOffset = inodeNumToCreate % 8;
        string byteInBin = bitset<8>(byteToCheck).to_string();
        byteInBin[byteInBin.size() - 1 - byteOffset] = '1';
        // Change that byteInBin to int again, then write it back
        int byteInInt = stoi(byteInBin, nullptr, 2);
        unsigned char byteInChar = (unsigned char)byteInInt;
        // cout << byteInInt << endl;
        // cout << (int)byteInChar << endl;
        inodeBitmap[byteNum] = byteInChar;

        // We have new inodeBitmap, so now write it
        writeInodeBitmap(super_global, inodeBitmap);

        /*
        Filling up contents of the new directory
        */

        // Prepare dir_ent_t entries of . and .. (write one at a time)
        dir_ent_t *dot = new dir_ent_t;
        strncpy(dot->name, ".\0", DIR_ENT_NAME_SIZE);
        dot->inum = inodeNumToCreate;

        dir_ent_t *dotDot = new dir_ent_t;
        strncpy(dotDot->name, "..\0", DIR_ENT_NAME_SIZE);
        dotDot->inum = parentInodeNumber;

        // These 2 dir_ent_ts need to be written to the newInode's direct
        // Get a buffer ready, memcpy in the 2 dir ents, then use the write() call
        int sizeOfEnts = sizeof(*dot) + sizeof(*dotDot);
        void *dirEntBuffer = malloc(sizeOfEnts);
        int pos = 0;

        memcpy((char *)dirEntBuffer, dot, sizeof(*dot));
        pos += sizeof(*dot);
        memcpy((char *)dirEntBuffer + pos, dotDot, sizeof(*dotDot));

        writeToDirectory(inodeNumToCreate, dirEntBuffer, sizeOfEnts, *this);
        // write(inodeNumToCreate, dirEntBuffer, sizeOfEnts); // Wrote . and .. to the direct array of the newInode. This should also update the size automatically

        // inode_t *testInode = new inode_t;
        // stat(inodeNumToCreate, testInode);

        // cout << "Testinode type " << testInode->type << "Testinode size " << testInode->size << endl;
        // delete testInode;

        free(dirEntBuffer);
        delete dot;
        delete dotDot;

        /*
        Updating parentInode, which is a directory, with information about the new inode
        */
        inode_t *parentInode = new inode_t;
        stat(parentInodeNumber, parentInode);

        // Create new directory entry for the newly created inode
        dir_ent_t *newDirEnt = new dir_ent_t;
        strncpy(newDirEnt->name, name.c_str(), DIR_ENT_NAME_SIZE);
        newDirEnt->inum = inodeNumToCreate;

        /*
        Writing to direct pointers of the parentInode
        */
        int fileSize = parentInode->size;
        int newFileSize = fileSize + sizeof(*newDirEnt);
        void *newDirBuffer = malloc(newFileSize);
        read(parentInodeNumber, newDirBuffer, fileSize); // Read what exists at inode currently
        // Copy in data of newDirEnt at the end of buffer
        memcpy((char *)newDirBuffer + fileSize, newDirEnt, sizeof(*newDirEnt));

        writeToDirectory(parentInodeNumber, newDirBuffer, newFileSize, *this);

        free(inodeBitmap);
        free(newDirBuffer);
        delete newDirEnt;
        delete parentInode;
      }
      // else file
      delete newInode;
    }
    else
    {
      return -ENOTENOUGHSPACE;
    }
  }
  else
  {
    // Inode exists
    // Check if the type is correct
    if ((int)type == target->type)
    {
      delete inode;
      delete target;
      cout << "That alr exist" << endl;
      return lookupRet;
    }
    else
    {
      cout << "original type: " << type << endl;
      cout << "target type: " << target->type << endl;

      cout << "Wrong type" << endl;
      // Exists but is the wrong type
      delete inode;
      delete target;
      return -EINVALIDTYPE;
    }
  }

  delete target;
  delete inode;

  /*
  Issues:
  1)target not being updated right
  2) new directory still has c.txt in it
  3) What if we need more blocks?

  */
  return 0;
}

int LocalFileSystem::write(int inodeNumber, const void *buffer, int size)
{
  inode_t *inode = new inode_t;
  int ret = stat(inodeNumber, inode);

  /*
  ERROR CHECKING
  */
  // Check ret to return 1 with error string
  if (ret == -EINVALIDINODE || ret == -EINVALIDSIZE)
  {
    // Invalid inode
    delete inode;
    //-EINVALIDINODE, -EINVALIDSIZE, -EINVALIDTYPE.
    return -EINVALIDINODE;
  }

  if (size < 0 || size > MAX_FILE_SIZE)
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

  int sizeToWrite = size; // Makes sure that only the buffer size is written if the specified size is bigger
  int newBlockCount = sizeToWrite / UFS_BLOCK_SIZE;
  if ((size % UFS_BLOCK_SIZE) != 0)
  {
    newBlockCount += 1;
  }

  // Uses the same number of blocks
  int total = 0;
  if (newBlockCount == currBlockCount)
  {
    // cout << "Is same\n";
    // Iterate through direct and get the current blocks that are in use
    vector<int> blocksInUse;
    for (int i = 0; i < currBlockCount; i++)
    {
      blocksInUse.push_back(inode->direct[i]);
    }

    // If block number same, all we have to do is foreach block, clear out and write
    // then update inode size. Bitmap should remain the same
    int remaining = sizeToWrite;
    int copyAmount;
    // Buffer of null values to use for emptying out the block
    void *emptyBuffer = malloc(UFS_BLOCK_SIZE);
    memset(emptyBuffer, '\0', UFS_BLOCK_SIZE);
    // Clear and write each block at a time
    for (int i = 0; i < currBlockCount; i++)
    {
      // Determine how much to write: a full block's worth or less
      if (remaining > UFS_BLOCK_SIZE)
      {
        copyAmount = UFS_BLOCK_SIZE;
      }
      else
      {
        copyAmount = remaining;
      }

      void *blockBuffer = malloc(UFS_BLOCK_SIZE);
      void *currBlockToCopy = (char *)buffer + i * UFS_BLOCK_SIZE; // Determines which block's worth of data from buffer should be copied in
      memcpy(blockBuffer, currBlockToCopy, copyAmount);

      // Reuse existing blokcs
      int blockNum = blocksInUse[i];

      // Clear out block
      disk->writeBlock(blockNum, emptyBuffer); // writeBlock must ALWAYS write a block at a time

      // Write block with new data
      disk->writeBlock(blockNum, blockBuffer);
      remaining -= copyAmount;
      total += copyAmount;
      free(blockBuffer);
    }
    // Updating inode size
    changeInodeSize(inodeNumber, total, *this);
    free(emptyBuffer);
  }
  else if (newBlockCount > currBlockCount)
  {

    /*
    CASE WHERE WE NEED TO ALLOCATE MORE BLOCKS
    */
    vector<int> blocksToUse; // vector that holds both blocks that are currently in use + extra blocks to allocate
    for (int i = 0; i < currBlockCount; i++)
    {
      blocksToUse.push_back(inode->direct[i]);
    }

    unsigned char *dataBitmap = (unsigned char *)malloc(super_global->data_bitmap_len * UFS_BLOCK_SIZE);
    readDataBitmap(super_global, dataBitmap);

    vector<int> newBlocks;
    int enoughSpace = 0;
    int collectedBlocks = 0;
    int newBlocksNum = newBlockCount - currBlockCount;
    int startBlock = super_global->data_region_addr;
    int endBlock = super_global->data_region_addr + super_global->data_region_len;
    int remaining;
    for (int i = startBlock; i < endBlock; i++)
    {
      // Remember: i here is the absolute block number
      int bitToCheck = (i - (super_global->data_region_addr));
      int byteNum = bitToCheck / 8;
      int byteToCheck = (int)dataBitmap[byteNum];

      string byteInBin = bitset<8>(byteToCheck).to_string();
      int byteOffset = bitToCheck % 8;

      char status = byteInBin[byteInBin.size() - 1 - byteOffset];
      string statusStr(1, status);
      if (statusStr != "1")
      {
        collectedBlocks += 1;
        // Is free--add the block to newBlocks vector)
        newBlocks.push_back(i); // Append that block number
        // cout << "Just added " << i << endl;
        if (collectedBlocks == newBlocksNum)
        {
          enoughSpace = 1;
          break;
        }
      }
    }

    int totalBlockCount = 0;
    // Check if we have enough new blocks to allcoate
    if (enoughSpace != 1)
    {
      // Not enough space, so determine how many block's worth of data can be written
      blocksToUse.insert(blocksToUse.end(), newBlocks.begin(), newBlocks.end());
      totalBlockCount = blocksToUse.size();
      remaining = totalBlockCount * UFS_BLOCK_SIZE; // update the size that is to be written to the max number of blocks we can write
    }
    else
    {
      // By this point, we have the new blocks we can write to. Append to blocksToUse and then write the buffer as you do in the same blocks case
      blocksToUse.insert(blocksToUse.end(), newBlocks.begin(), newBlocks.end());
      totalBlockCount = blocksToUse.size();
      // We have enough blocks, so copyAmount is size
      remaining = size;
    }

    int copyAmount;
    // Buffer of null values to use for emptying out the block
    void *emptyBuffer = malloc(UFS_BLOCK_SIZE);
    memset(emptyBuffer, '\0', UFS_BLOCK_SIZE);
    // Clear and write each block at a time
    for (int i = 0; i < totalBlockCount; i++)
    {
      // Determine how much to write: a full block's worth or less
      if (remaining > UFS_BLOCK_SIZE)
      {
        copyAmount = UFS_BLOCK_SIZE;
      }
      else
      {
        copyAmount = remaining;
      }

      void *blockBuffer = malloc(UFS_BLOCK_SIZE);
      void *currBlockToCopy = (char *)buffer + i * UFS_BLOCK_SIZE; // Determines which block's worth of data from buffer should be copied in
      memcpy(blockBuffer, currBlockToCopy, copyAmount);

      // Start writing to the blocks
      int blockNum = blocksToUse[i];
      // cout << "Writing to block " << blockNum << endl;

      // Clear out block
      disk->writeBlock(blockNum, emptyBuffer);

      // Write block with new data
      disk->writeBlock(blockNum, blockBuffer);
      remaining -= copyAmount;
      total += copyAmount;
      free(blockBuffer);
    }

    // After writing, 3 things need to be done: inode size update, direct array update, data bitmap update

    /*
    Inode size update
    */
    changeInodeSize(inodeNumber, total, *this);
    free(emptyBuffer);

    /*
    Updating direct pointer of the inodeNumber inode
    */
    inode_t *inode = new inode_t;
    int ret = stat(inodeNumber, inode);
    if (ret != 0)
    {
      cerr << "Error while getting inode to update its direct pointers" << endl;
    }

    // Iterate through blocksToUse and update the direct[] entries. We can check this with ds3cart
    for (int i = 0; i < (int)blocksToUse.size(); i++)
    {
      inode->direct[i] = blocksToUse[i];
    }

    // Persist to disk
    inode_t *inodes = (inode_t *)malloc(super_global->num_inodes * sizeof(inode_t));
    // Obtains current state of inodes. Now just have to replace it with inodes
    readInodeRegion(super_global, inodes);

    // Find where in inodes the inode specified by inum is
    inodes[inodeNumber] = *inode;

    writeInodeRegion(super_global, inodes);
    free(inodes);
    delete inode;

    /*
    Update data bitmap
    */
    for (int blockNum : blocksToUse)
    {
      // Remember: i here is the absolute block number
      int bitToCheck = (blockNum - (super_global->data_region_addr));
      int byteNum = bitToCheck / 8;
      int byteToCheck = (int)dataBitmap[byteNum];
      int byteOffset = bitToCheck % 8;
      string byteInBin = bitset<8>(byteToCheck).to_string();

      // Access that byte and modify the value
      byteInBin[byteInBin.size() - 1 - byteOffset] = '1';

      // cout << "changed byteInBin: " << byteInBin << endl;

      // Change that byteInBin to int again, then write it back as dataBitmap[byteNum]
      int byteInInt = stoi(byteInBin, nullptr, 2);
      unsigned char byteInChar = (unsigned char)byteInInt;
      // cout << byteInInt << endl;
      // cout << (int)byteInChar << endl;
      dataBitmap[byteNum] = byteInChar;
      // int numDataInBytes = super_global->num_data / 8;
      // Printing byte by byte
      // for (int i = 0; i < numDataInBytes; i++)
      // {
      //   cout << (unsigned int)dataBitmap[i] << " ";
      // }
      // cout << "\n";
    }
    // Write new dataBitmap using writeDatabitmap
    writeDataBitmap(super_global, dataBitmap);
    free(dataBitmap);
  }
  else if (newBlockCount < currBlockCount)
  {
    // LESS BLOCKS NEEDED
    // Iterate through the current blocks that are in use
    // Separate into two bins: blocks that are still needed, and blocks that are no longer needed
    vector<int> blocksInUse;
    for (int i = 0; i < currBlockCount; i++)
    {
      // cout << inode->direct[i] << endl;
      blocksInUse.push_back(inode->direct[i]);
    }

    vector<int> blocksToKeep;
    vector<int> blocksToFree;
    int bCount = 0;

    cout << currBlockCount << endl;
    cout << newBlockCount << endl;
    for (int i = 0; i < currBlockCount; i++)
    {
      if (bCount < newBlockCount)
      {
        blocksToKeep.push_back(inode->direct[i]);
        bCount += 1;
      }
      else
      {
        blocksToFree.push_back(inode->direct[i]);
        bCount += 1;
      }
    }

    // // For testing reduced block writes
    // for (int blockNum : blocksToKeep)
    // {
    //   cout << "Blocks to keep is: " << blockNum << endl;
    // }

    // for (int blockNum : blocksToFree)
    // {
    //   cout << "Blocks to bye bye is: " << blockNum << endl;
    // }

    // Iterate through blocksToKeep and write buffer to those blocks
    // If block number same, all we have to do is foreach block, clear out and write
    // then update inode size. Bitmap should remain the same
    int remaining = sizeToWrite;
    int copyAmount;
    // Buffer of null values to use for emptying out the block
    void *emptyBuffer = malloc(UFS_BLOCK_SIZE);
    memset(emptyBuffer, '\0', UFS_BLOCK_SIZE);
    // Clear and write each block at a time
    for (int i = 0; i < newBlockCount; i++)
    {
      // Determine how much to write: a full block's worth or less
      if (remaining > UFS_BLOCK_SIZE)
      {
        copyAmount = UFS_BLOCK_SIZE;
      }
      else
      {
        copyAmount = remaining;
      }

      void *blockBuffer = malloc(UFS_BLOCK_SIZE);
      void *currBlockToCopy = (char *)buffer + i * UFS_BLOCK_SIZE; // Determines which block's worth of data from buffer should be copied in
      memcpy(blockBuffer, currBlockToCopy, copyAmount);

      // Reuse existing blokcs
      int blockNum = blocksToKeep[i];

      // Clear out block
      disk->writeBlock(blockNum, emptyBuffer);
      // Write block with new data
      disk->writeBlock(blockNum, blockBuffer);
      remaining -= copyAmount;
      total += copyAmount;
      free(blockBuffer);
    }

    /*
    Inode size update
    */
    changeInodeSize(inodeNumber, total, *this);
    free(emptyBuffer);

    /*
    Update inode direct with blocksToKeep and blocksToBeFree as 0 (this part may not be needed since we change the size)
    */
    inode_t *inode = new inode_t;
    stat(inodeNumber, inode);
    // Iterate through blocksToUse and update the direct[] entries. We can check this with ds3cart
    for (int i = 0; i < (int)blocksToKeep.size(); i++)
    {
      inode->direct[i] = blocksToKeep[i];
    }

    // Persist to disk
    inode_t *inodes = (inode_t *)malloc(super_global->num_inodes * sizeof(inode_t));
    // Obtains current state of inodes. Now just have to replace it with inodes
    readInodeRegion(super_global, inodes);

    // Find where in inodes the inode specified by inum is
    inodes[inodeNumber] = *inode;

    writeInodeRegion(super_global, inodes);
    free(inodes);
    delete inode;

    /*
    Update the dataBitmap to set the freed blocks to 0
    */
    unsigned char *dataBitmap = (unsigned char *)malloc(super_global->data_bitmap_len * UFS_BLOCK_SIZE);
    readDataBitmap(super_global, dataBitmap);
    for (int blockNum : blocksToFree)
    {
      // Remember: i here is the absolute block number
      int bitToCheck = (blockNum - (super_global->data_region_addr));
      int byteNum = bitToCheck / 8;
      int byteToCheck = (int)dataBitmap[byteNum];
      int byteOffset = bitToCheck % 8;
      string byteInBin = bitset<8>(byteToCheck).to_string();

      // Access that byte and modify the value back to 0
      byteInBin[byteInBin.size() - 1 - byteOffset] = '0';

      // cout << "changed byteInBin: " << byteInBin << endl;

      // Change that byteInBin to int again, then write it back as dataBitmap[byteNum]
      int byteInInt = stoi(byteInBin, nullptr, 2);
      unsigned char byteInChar = (unsigned char)byteInInt;
      // cout << byteInInt << endl;
      // cout << (int)byteInChar << endl;
      dataBitmap[byteNum] = byteInChar;
      // int numDataInBytes = super_global->num_data / 8;
      // // Printing byte by byte
      // for (int i = 0; i < numDataInBytes; i++)
      // {
      //   cout << (unsigned int)dataBitmap[i] << " ";
      // }
      // cout << "\n";
    }
    // Write new dataBitmap using writeDatabitmap
    writeDataBitmap(super_global, dataBitmap);
    free(dataBitmap);
  }
  else
  {
    cerr << "Uncaught case when comparing newBlockCount and currBlockCount" << endl;
  }
  delete inode;
  return total;
}

int LocalFileSystem::unlink(int parentInodeNumber, string name)
{
  return 0;
}

/*
Things to test

1. Delete -> Try to access inode and see if it throws error
2. Actually check if total in write matches up with size in less than, same, more (check)
3. Test omega big file (easier to do with cpy, probably)

Create tests
1) Simple create
2) Create when existing
3) Create when exsiting, wrong type
4) Not enough space test

TODO:
- Create
- Unlink
- WriteInodeBitmap (prolly needed for both of the above)
*/