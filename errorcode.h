#ifndef ERRORCODE_H
#define ERRORCODE_H

enum MMAP {
  MMAP = 1,
  MSYNC,
  MUNMAP,
  MREMAP
};

enum ERRORCODE {

  SUCCESS = 0, 
  MMAP_ERROR,

  /* mmap pool part */
  MMAP_MMAP = (MMAP_ERROR | (MMAP::MMAP << 8)),
  MMAP_MSYNC = (MMAP_ERROR | (MMAP::MSYNC << 8)),
  MMAP_MUNMAP = (MMAP_ERROR | (MMAP::MUNMAP << 8)),
  MMAP_MREMAP = (MMAP_ERROR | (MMAP::MREMAP << 8)),
};

#endif //__OBSERVER_RC_H__
