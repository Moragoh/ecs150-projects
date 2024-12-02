#include <iostream>
#include <string>
#include <vector>
#include <cstring>

#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;

int main(int argc, char *argv[])
{
  if (argc != 4)
  {
    cerr << argv[0] << ": diskImageFile parentInode directory" << endl;
    cerr << "For example:" << endl;
    cerr << "    $ " << argv[0] << " a.img 0 a" << endl;
    return 1;
  }

  // Parse command line arguments
  Disk *disk = new Disk(argv[1], UFS_BLOCK_SIZE);
  LocalFileSystem *fileSystem = new LocalFileSystem(disk);
  int parentInode = stoi(argv[2]);
  string directory = string(argv[3]);

  string ERRMSG = "Error creating directory";
  // Create a file of name under parentInode
  int ret = fileSystem->create(parentInode, 0, directory);
  if (ret < 0)
  {
    delete disk;
    delete fileSystem;
    cerr << ERRMSG << endl;
    return 1;
  }

  delete disk;
  delete fileSystem;
  return 0;
}
