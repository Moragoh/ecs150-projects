#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>

#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    cerr << argv[0] << ": diskImageFile inodeNumber" << endl;
    return 1;
  }

  // Parse command line arguments
  Disk *disk = new Disk(argv[1], UFS_BLOCK_SIZE);
  LocalFileSystem *fileSystem = new LocalFileSystem(disk);
  int inodeNumber = stoi(argv[2]);

  // Use stat
  inode_t *inode = new inode_t;
  int ret = fileSystem->stat(inodeNumber, inode);

  // Check ret to return 1 with error string
  if (ret == -EINVALIDINODE || ret == -EINVALIDSIZE)
  {
    delete inode;
    delete disk;
    delete fileSystem;
    cerr << "Error reading file" << endl;
    return 1;
  }

  if (inode->type == UFS_DIRECTORY)
  {
    // Is directory
    delete inode;
    delete disk;
    delete fileSystem;
    cerr << "Error reading file" << endl;
    return 1;
  }

  // inode obtained by this point
  cout << "File blocks" << endl;

  // Convert the file size to how many blocks of data it would require, then iterate through direct using that count
  int fileSize = inode->size;

  // cout << "Filesize " << fileSize << endl;

  int blockCount = fileSize / UFS_BLOCK_SIZE;
  if ((fileSize % UFS_BLOCK_SIZE) != 0)
  {
    blockCount += 1;
  }

  for (int i = 0; i < blockCount; i++)
  {
    cout << inode->direct[i] << endl;
  }

  // File Data
  cout << "\n";
  cout << "File data" << endl;
  if (fileSize == 0)
  {
    return 0;
  }
  // Use read() utility and print out the buffer
  void *buffer[fileSize + 1];
  fileSystem->read(inodeNumber, buffer, fileSize);
  char *charBuffer = (char *)buffer;
  charBuffer[fileSize + 1] = '\0';
  cout << charBuffer; // Make sure to null terminate anything read in

  delete inode;
  delete disk;
  delete fileSystem;

  return 0;
}
