#include <iostream>
#include <string>
#include <sstream>

#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;

int main(int argc, char *argv[])
{
  if (argc != 4)
  {
    cerr << argv[0] << ": diskImageFile src_file dst_inode" << endl;
    cerr << "For example:" << endl;
    cerr << "    $ " << argv[0] << " tests/disk_images/a.img dthread.cpp 3" << endl;
    return 1;
  }

  // Parse command line arguments
  Disk *disk = new Disk(argv[1], UFS_BLOCK_SIZE);
  LocalFileSystem *fileSystem = new LocalFileSystem(disk);
  string srcFile = string(argv[2]);
  int dstInode = stoi(argv[3]);
  /*
  The ds3cp utility
  The ds3cp utility copies a file from your computer into the disk image using your LocalFileSystem.cpp implementation. It takes three command line arguments, the disk image file, the source file from your computer that you want to copy in, and the inode for the destination file within your disk image.

  For all errors, print the string Could not write to dst_file to standard error and exit with return code 1. We will always use files that exist on the computer for testing, so if you encoutner an error opening or reading from the source file you can print any error message that makes sense.
  */
  // What ds3cp does:
  // use srcFile to read in all the data into the buffer (use the built in read utility
  // have that in a buffer, then use the write utility
  int fd = open(srcFile.c_str(), O_RDONLY); // As a referesher--open obtains teh file descriptior, which read can be called on
  if (fd < 0)
  {
    cerr << "Error while trying to open file" << endl;
  }

  stringstream fileData;
  int totalBytesRead = 0;
  int bytesRead = 0;
  char tempBuffer[1024];                               // Read in a block at a time
  while ((bytesRead = read(fd, tempBuffer, 1024)) > 0) // Read will return how many bytes were saved
  {
    totalBytesRead += bytesRead;
    fileData << tempBuffer;
  }
  fileData << '\0'; // Null term what we read

  // Convert stream into string, then string to char*
  string fileInStr = fileData.str();
  const char *fileInChars = fileInStr.c_str();

  cout << fileInChars;

  delete disk;
  delete fileSystem;
  cout << dstInode;

  return 0;
}
