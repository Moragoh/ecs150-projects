#include <iostream>
#include <cstring>
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
    delete disk;
    delete fileSystem;
    cerr << "Error while trying to open file" << endl;
    return 1;
  }

  stringstream fileData;
  int totalBytesRead = 0;
  int bytesRead = 0;
  char tempBuffer[1024];                                             // Read in a block at a time
  while ((bytesRead = read(fd, tempBuffer, sizeof(tempBuffer))) > 0) // Read will return how many bytes were saved
  {
    // use write call
    fileData.write(tempBuffer, bytesRead);
    totalBytesRead += bytesRead;
    // fileData << tempBuffer; // Says it broke here
  }

  // Error check read
  if (bytesRead < 0)
  {
    delete disk;
    delete fileSystem;
    cerr << "Error while reading file" << endl;
    return 1;
  }
  // Convert stream into string, then string to char*
  string fileInStr = fileData.str(); // this may get rid of the null character
  fileInStr += '\0';                 // Null termiante what we are about to write, since write() in our case never appends and always replaces everything
  const char *fileInChars = fileInStr.c_str();
  size_t length = strlen(fileInChars);

  // Now we have what we read in. Time to write using the LocalFileSystem write() function
  fileSystem->write(dstInode, fileInChars, length + 1); // Because write always wipes clean and rewrites, we do length+1 make sure the null terminator is included

  delete disk;
  delete fileSystem;
  return 0;
}
