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
  if (argc != 2)
  {
    cerr << argv[0] << ": diskImageFile" << endl;
    return 1;
  }

  // Parse command line arguments
  Disk *disk = new Disk(argv[1], UFS_BLOCK_SIZE);
  LocalFileSystem *fileSystem = new LocalFileSystem(disk);

  // Run super
  super_t *super = new super_t;
  fileSystem->readSuperBlock(super);

  // Print super block stuff
  cout << "Super" << endl;
  cout << "inode_region_addr " << super->inode_region_addr << endl;
  cout << "inode_region_len " << super->inode_region_len << endl;
  cout << "num_inodes " << super->num_inodes << endl;
  cout << "data_region_addr " << super->data_region_addr << endl;
  cout << "data_region_len " << super->data_region_len << endl;
  cout << "num_data " << super->num_data << endl;
  cout << "\n";

  // Inode bitmap stuff
  // Inode bitmap only needs the size of how many inodes ther will be
  unsigned char *inodeBitmap = (unsigned char *)malloc(super->inode_bitmap_len * UFS_BLOCK_SIZE);
  fileSystem->readInodeBitmap(super, inodeBitmap);

  cout << "Inode bitmap" << endl;

  // numInodesInBytes: There are inode_num inodes, and a byte is 8 bits, so numInodesInBytes = num_inodes / 8
  int numInodesInBytes = super->num_inodes / 8;
  // Printing byte by byte
  for (int i = 0; i < numInodesInBytes; i++)
  {
    cout << (unsigned int)inodeBitmap[i] << " ";
  }
  cout << "\n\n";

  cout << "Data bitmap" << endl;

  // Data bitmap stuff
  // unsigned char *dataBitmap = (unsigned char *)malloc(super->num_data * sizeof(unsigned char *))
  unsigned char *dataBitmap = (unsigned char *)malloc(super->data_bitmap_len * UFS_BLOCK_SIZE);
  fileSystem->readDataBitmap(super, dataBitmap);

  int numDataInBytes = super->num_data / 8;
  // Printing byte by byte
  for (int i = 0; i < numDataInBytes; i++)
  {
    cout << (unsigned int)dataBitmap[i] << " ";
  }
  cout << "\n";

  free(inodeBitmap);
  free(dataBitmap);
  delete super;
  delete disk;
  delete fileSystem;

  return 0;
}
