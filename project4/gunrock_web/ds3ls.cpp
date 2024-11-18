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

  super_t *super = new super_t;
  fileSystem->readSuperBlock(super);

  unsigned char *inodeBitmap = (unsigned char *)malloc(super->inode_bitmap_len * UFS_BLOCK_SIZE);
  fileSystem->readInodeBitmap(super, inodeBitmap);

  cout << (int)inodeBitmap[1] << endl;
  free(inodeBitmap);

  // Deallocate to avoid issues
  delete super;
  delete disk;
  delete fileSystem;

  return 0;
}
