#ifndef __ufs_h__
#define __ufs_h__

#define UFS_DIRECTORY (0)
#define UFS_REGULAR_FILE (1)

#define UFS_ROOT_DIRECTORY_INODE_NUMBER (0) // Start by reading 0
#define UFS_BLOCK_SIZE (4096) // 4kB = 4 * 1024 bits. K iks kibi is 1024 bytes here
#define DIRECT_PTRS (30) // No indirect poiners

#define MAX_FILE_SIZE (DIRECT_PTRS * UFS_BLOCK_SIZE) 

// Note: Bitmap indexes identify disk blocks relative to the start of a region.

// Struct for inode (which includes metadata)
typedef struct {
    int type;   // UFS_DIRECTORY or UFS_REGULAR
    int size;   // bytes
    unsigned int direct[DIRECT_PTRS];
} inode_t;
// SUMMARY: Each inode has a type, size, and 30 direct pointers. 

// Struct for directory entry. Has name and an inum
// By reading this, you can list out information about each entry
#define DIR_ENT_NAME_SIZE (28)
typedef struct {
    char name[DIR_ENT_NAME_SIZE];  // up to 28 bytes of name in directory (including \0)
    int  inum;      // inode number of entry
} dir_ent_t;

// Struct for super block
// presumed: block 0 is the super block
typedef struct __super {
    int inode_bitmap_addr; // block address (in blocks)
    int inode_bitmap_len;  // in blocks
    int data_bitmap_addr;  // block address (in blocks)
    int data_bitmap_len;   // in blocks
    int inode_region_addr; // block address (in blocks)
    int inode_region_len;  // in blocks
    int data_region_addr;  // block address (in blocks)
    int data_region_len;   // in blocks
    int num_inodes;        // just the number of inodes
    int num_data;          // and data blocks...
} super_t;


#endif // __ufs_h__
