#include <iostream>
#include <string>
#include <vector>

#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;

int main(int argc, char *argv[])
{
  if (argc != 4)
  {
    cerr << argv[0] << ": diskImageFile parentInode fileName" << endl;
    cerr << "For example:" << endl;
    cerr << "    $ " << argv[0] << " a.img 0 a.txt" << endl;
    return 1;
  }

  // Parse command line arguments

  Disk *disk = new Disk(argv[1], UFS_BLOCK_SIZE);
  LocalFileSystem *fileSystem = new LocalFileSystem(disk);
  int parentInode = stoi(argv[2]);
  string fileName = string(argv[3]);

  // What to do now? First write the ds3touch utility so taht it works, test it, and upload it to gradescope
  string ERRMSG = "Error creating file";

  // Create a file of name under parentInode
  int ret = fileSystem->create(parentInode, 1, fileName);
  if (ret < 0)
  {
    delete disk;
    delete fileSystem;
    cerr << ERRMSG << endl;
    return 1;
  }

  // Disk *disk = new Disk("./tests/disk_images/a3.img", UFS_BLOCK_SIZE);
  // LocalFileSystem *fileSystem = new LocalFileSystem(disk);
  // string fileName = "createTest";

  // // inode_t *inode = new inode_t;
  // // fileSystem->stat(parentInode, inode);

  // // Temporarily use to test create
  // // Problem: It's saying that inode 2 is an invalid inode. Find out why
  // fileSystem->create(2, 0, "CoffeeBean");

  delete disk;
  delete fileSystem;
  return 0;
}

/*
DEBVUGGER: MUST RUN CODE AS YOU USUALLY WOULD LIKE THIS
ROSETTA_DEBUGSERVER_PORT=1234 ./ds3touch
*/