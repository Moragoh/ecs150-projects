#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cstring>

#include "StringUtils.h"
#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;

// Use this function with std::sort for directory entries
bool compareByName(const dir_ent_t &a, const dir_ent_t &b)
{
  return std::strcmp(a.name, b.name) < 0;
}

int main(int argc, char *argv[])
{

  if (argc != 3)
  {
    cerr << argv[0] << ": diskImageFile directory" << endl;
    cerr << "For example:" << endl;
    cerr << "    $ " << argv[0] << " tests/disk_images/a.img /a/b" << endl;
    return 1;
  }

  // Utilities do not need the server--it is all local, so it will emulate the Disk and FileSystem
  Disk *disk = new Disk(argv[1], UFS_BLOCK_SIZE);
  LocalFileSystem *fileSystem = new LocalFileSystem(disk); // Use func from this after they are implemented
  string directory = string(argv[2]);

  // Use getline instead
  stringstream dirStream(directory);
  string nextName;
  int currInodeNum;
  int iterCount = 0;

  while (getline(dirStream, nextName, '/'))
  {
    // cout << nextName << endl;
    // Special case of when just / is inputted as path: should immediately go to printing our directories
    if (iterCount == 0 && nextName == "")
    {
      // For handling root directory
      currInodeNum = 0;
    }
    else
    {
      // Find inode num with the next name to search, and update once found
      currInodeNum = fileSystem->lookup(currInodeNum, nextName);
    }
  }

  // Directory num found by this point; time to print our directory contents
  inode_t *target = new inode_t;
  int ret = fileSystem->stat(currInodeNum, target);

  // Check ret to return 1 with error string
  if (ret == -EINVALIDINODE || ret == -EINVALIDSIZE)
  {
    delete target;
    delete disk;
    delete fileSystem;
    cerr << "Directory not found" << endl;
    return 1;
  }

  // Otherwise print out contents
  // Check if file or directory
  if (target->type == 0)
  {

    // Directory, so print out entries
    vector<dir_ent_t> dirEnts;

    // Collect directories
    // Use inode.size to get the size for the read
    int fileSize = target->size;
    void *buffer[fileSize];
    fileSystem->read(currInodeNum, buffer, fileSize); // read contents of currInodeNum *(inode num of target)

    // Buffer contains directory contents
    dir_ent_t *dirBuffer = (dir_ent_t *)buffer;
    int entryCount = fileSize / sizeof(dir_ent_t);

    // Collect directory elements
    for (int i = 0; i < entryCount; i++)
    {
      // Append each entry to vector
      dirEnts.push_back(dirBuffer[i]);
    }

    // Sort entries by name
    sort(dirEnts.begin(), dirEnts.end(), compareByName);

    // Print relevant info
    for (auto ent : dirEnts)
    {
      int inodeNum = ent.inum;
      char *fileName = (char *)ent.name;
      cout << inodeNum << "\t" << fileName << "\n";
    }
  }
  else
  {
    // File, so print out the name
    // If it is a file, nextName would simply be its name
    cout << currInodeNum << "\t" << nextName << "\n";
  }

  delete target;
  delete disk;
  delete fileSystem;
  return 0;
}