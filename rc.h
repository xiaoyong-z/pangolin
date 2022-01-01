#ifndef ERRORCODE_H
#define ERRORCODE_H

enum MMAP {
  MMAP = 1,
  MSYNC,
  MUNMAP,
  MREMAP
};

enum MMAPFILE {
  OPEN = 1,
  FSATA,
  FTRUNCATE,
  CLOSE,
  REMOVE,
  MMAP_UNINTIALIZE,
  NOT_ENOUGH_SPACE,
};

enum WAL {
  UNINTIALIZE = 1,
  END_OF_FILE
};


enum SKIPLIST {
  NOT_FOUND,
};

enum LEVELS {
  SSTABLE_FILE_NOT_OPEN,
  KEY_NOT_FOUND_IN_CUR_LEVEL,
  KEY_NOT_FOUND_IN_ALL_LEVELS,
};

enum SSTABLE {
  CRC_FAIL1,
};

enum MEMTABLE {
  MEMTABLE_UNINTIALIZE,
};

enum TABLE {
  EXCEED_MINMAX,
  BLOOM_FILTER_NOT_CONTAIN,
  KEY_NOT_FOUND_IN_BLOCK,
};

enum BLOCK {
  CRC_FAIL2,
};



enum RC {

  SUCCESS = 0, 
  MMAP_ERROR,
  MMAPFILE_ERROR,
  WAL_ERROR,
  SKIPLIST_ERROR,
  LEVELS_ERROR,
  SSTABLE_ERROR,
  BLOCK_ERROR,
  TABLE_ERROR,
  MEMTABLE_ERROR,

  /* mmap pool part */
  MMAP_MMAP = (MMAP_ERROR | (MMAP::MMAP << 8)),
  MMAP_MSYNC = (MMAP_ERROR | (MMAP::MSYNC << 8)),
  MMAP_MUNMAP = (MMAP_ERROR | (MMAP::MUNMAP << 8)),
  MMAP_MREMAP = (MMAP_ERROR | (MMAP::MREMAP << 8)),

  MMAPFILE_OPEN = (MMAPFILE_ERROR | (MMAPFILE::OPEN << 8)),
  MMAPFILE_STAT = (MMAPFILE_ERROR | (MMAPFILE::FSATA << 8)),
  MMAPFILE_TRUNCATE = (MMAPFILE_ERROR | (MMAPFILE::FTRUNCATE << 8)),
  MMAPFILE_CLOSE = (MMAPFILE_ERROR | (MMAPFILE::CLOSE << 8)),
  MMAPFILE_REMOVE = (MMAPFILE_ERROR | (MMAPFILE::REMOVE << 8)),
  MMAPFILE_MMAP_UNINITIALIZE = (MMAPFILE_ERROR | (MMAPFILE::MMAP_UNINTIALIZE << 8)), 
  MMAPFILE_NOT_ENOUGH_SPACE = (MMAPFILE_ERROR | (MMAPFILE::NOT_ENOUGH_SPACE << 8)), 

  WAL_UNINTIALIZE = (WAL_ERROR | (WAL::UNINTIALIZE << 8)),
  WAL_END_OF_FILE_ERROR = (WAL_ERROR | (WAL::END_OF_FILE) << 8),

  SKIPLIST_NOT_FOUND = (SKIPLIST_ERROR | (SKIPLIST::NOT_FOUND << 8)),

  LEVELS_FILE_NOT_OPEN = (LEVELS_ERROR | (LEVELS::SSTABLE_FILE_NOT_OPEN << 8)),
  LEVELS_KEY_NOT_FOUND_IN_CUR_LEVEL = (LEVELS_ERROR | (LEVELS::KEY_NOT_FOUND_IN_CUR_LEVEL << 8)),
  LEVELS_KEY_NOT_FOUND_IN_ALL_LEVELS = (LEVELS_ERROR | (LEVELS::KEY_NOT_FOUND_IN_ALL_LEVELS << 8)),

  SSTABLE_CRC_FAIL = (SSTABLE_ERROR | (SSTABLE::CRC_FAIL1 << 8)),
  
  BLOCK_CRC_FAIL = (BLOCK_ERROR | (BLOCK::CRC_FAIL2 << 8)),

  TABLE_EXCEED_MINMAX = (TABLE_ERROR | (TABLE::EXCEED_MINMAX << 8)),
  
  TABLE_BLOOM_FILTER_NOT_CONTAIN = (TABLE_ERROR | (TABLE::BLOOM_FILTER_NOT_CONTAIN << 8)),
  TABLE_KEY_NOT_FOUND_IN_BLOCK = (TABLE_ERROR | (TABLE::KEY_NOT_FOUND_IN_BLOCK << 8)),

  MEMTABLE_UNINTIALIZE_FAIL = (MEMTABLE_ERROR | (MEMTABLE::MEMTABLE_UNINTIALIZE) << 8),

};

#endif //__OBSERVER_RC_H__
