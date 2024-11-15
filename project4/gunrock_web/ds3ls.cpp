#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>

#include "StringUtils.h"
#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;

/*
  Use this function with std::sort for directory entries
bool compareByName(const dir_ent_t& a, const dir_ent_t& b) {
    return std::strcmp(a.name, b.name) < 0;
}
*/

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    cerr << argv[0] << ": diskImageFile directory" << endl;
    cerr << "For example:" << endl;
    cerr << "    $ " << argv[0] << " tests/disk_images/a.img /a/b" << endl;
    return 1;
  }

  // parse command line arguments

  // Utilities do not need the server--it is all local, so it will emulate the Disk and FileSystem
  Disk *disk = new Disk(argv[1], UFS_BLOCK_SIZE);
  LocalFileSystem *fileSystem = new LocalFileSystem(disk); // Use func from this after they are implemented
  string directory = string(argv[2]);

  // // TODO

  // // Go to disk and read contents of superblock (block num 0)
  // /*
  //   How to read inode numbers:
  //   Get the block num by: (inum + sizeof(inode_)) / blockSize
  //   Then allocate a buffer that can accomodate a block
  //   Then use inode index by doing inum % blockInodeCount and access it
  // */
  // int inum = 0; // Placeholder
  // int blockInodeCount = UFS_BLOCK_SIZE / sizeof(inode_t);
  // inode_t blockBuffer[blockInodeCount];

  // // super_t super;
  // // int inodeTableStart = super.inode_region_addr;
  // // int blockOffset = (sizeof(inode_t) * inum) / UFS_BLOCK_SIZE;
  // // int blockNum = blockOffset + inodeTableStart;

  // disk->readBlock(0, blockBuffer);

  // int withinBlockInodeNum = inum % blockInodeCount;
  // inode_t inode = blockBuffer[0]; // Inode for that inum retrived. Now we can get metadata from this

  // cout << inode.type;

  // At this point we have a number of inodes

  // Get inode block start location
  // Read inode (first of which should be root directory)
  // Iterate thru direct pointers

  return 0;
}
