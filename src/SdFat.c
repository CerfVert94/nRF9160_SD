
#include <zephyr.h>
#include <SdFat.h>
#include <SD.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <SDAPI.h>

u8_t volume_init(u8_t part) {
	
  u32_t volumeStartBlock = 0;
  
  if (part) {
    if (part > 4) {
      return false;
    }

    if (!cacheRawBlock(volumeStartBlock, CACHE_FOR_READ)) {
      return false;
    }
    
  part_t* p = &cacheBuffer_.mbr.part[part - 1];
    
    if ((p->boot & 0X7F) != 0  ||
        p->totalSectors < 100 ||
        p->firstSector == 0) {
      // not a valid partition
      return false;
    }
    volumeStartBlock = p->firstSector;
  }
  
  if (!cacheRawBlock(volumeStartBlock, CACHE_FOR_READ)) {
    return false;
  }
  
  bpb_t* bpb = &cacheBuffer_.fbs.bpb;
  if (bpb->bytesPerSector != 512 ||
      bpb->fatCount == 0 ||
      bpb->reservedSectorCount == 0 ||
      bpb->sectorsPerCluster == 0) {
    // not valid FAT volume
    return false;
  }
  fatCount_ = bpb->fatCount;
  blocksPerCluster_ = bpb->sectorsPerCluster;

  // determine shift that is same as multiply by blocksPerCluster_
  clusterSizeShift_ = 0;
  while (blocksPerCluster_ != (1 << clusterSizeShift_)) {
    // error if not power of 2
    if (clusterSizeShift_++ > 7) {
      return false;
    }
  }
  blocksPerFat_ = bpb->sectorsPerFat16 ?
                  bpb->sectorsPerFat16 : bpb->sectorsPerFat32;

  fatStartBlock_ = volumeStartBlock + bpb->reservedSectorCount;

  // count for FAT16 zero for FAT32
  rootDirEntryCount_ = bpb->rootDirEntryCount;

  // directory start for FAT16 dataStart for FAT32
  rootDirStart_ = fatStartBlock_ + bpb->fatCount * blocksPerFat_;

  // data start for FAT16 and FAT32
  dataStartBlock_ = rootDirStart_ + ((32 * bpb->rootDirEntryCount + 511) / 512);

  // total blocks for FAT16 or FAT32
  u32_t totalBlocks = bpb->totalSectors16 ?
                         bpb->totalSectors16 : bpb->totalSectors32;
  // total data blocks
  clusterCount_ = totalBlocks - (dataStartBlock_ - volumeStartBlock);

  // divide by cluster size to get cluster count
  clusterCount_ >>= clusterSizeShift_;

  // FAT type is determined by cluster count
  if (clusterCount_ < 4085) {
    fatType_ = 12;
  } else if (clusterCount_ < 65525) {
    fatType_ = 16;
  } else {
    rootDirStart_ = bpb->fat32RootCluster;
    fatType_ = 32;
  }
  return true;
}


//------------------------------------------------------------------------------
static u8_t cacheRawBlock(u32_t blockNumber, u8_t action) {
  if (cacheBlockNumber_ != blockNumber) {
    if (!cacheFlush()) {
      return false;
    }
    if (!readBlock(blockNumber, cacheBuffer_.data)) {
      return false;
    }
    cacheBlockNumber_ = blockNumber;
  }
  cacheDirty_ |= action;
  return true;
}

//------------------------------------------------------------------------------
static u8_t cacheFlush(void) {
  if (cacheDirty_) {
    if (!writeBlock(cacheBlockNumber_, cacheBuffer_.data)) {
      return false;
    }
    // mirror FAT tables
    if (cacheMirrorBlock_) {
      if (!writeBlock(cacheMirrorBlock_, cacheBuffer_.data)) {
        return false;
      }
      cacheMirrorBlock_ = 0;
    }
    cacheDirty_ = 0;
  }
  return true;
}

u8_t chainSize(u32_t cluster, u32_t* size) {
  u32_t s = 0;
  do {
    if (!fatGet(cluster, &cluster)) {
      return false;
    }
    s += 512UL << clusterSizeShift_;
  } while (!isEOC(cluster));
  *size = s;
  return true;
}


u8_t fatGet(u32_t cluster, u32_t* value) {
  if (cluster > (clusterCount_ + 1)) {
    return false;
  }
  u32_t lba = fatStartBlock_;
  lba += fatType_ == 16 ? cluster >> 8 : cluster >> 7;
  if (lba != cacheBlockNumber_) {
    if (!cacheRawBlock(lba, CACHE_FOR_READ)) {
      return false;
    }
  }
  if (fatType_ == 16) {
    *value = cacheBuffer_.fat16[cluster & 0XFF];
  } else {
    *value = cacheBuffer_.fat32[cluster & 0X7F] & FAT32MASK;
  }
  return true;
}


    u8_t blocksPerCluster(){
      return blocksPerCluster_;
    }
    /** \return The number of blocks in one FAT. */
    u32_t blocksPerFat(){
      return blocksPerFat_;
    }
    /** \return The total number of clusters in the volume. */
    u32_t clusterCount(){
      return clusterCount_;
    }
    /** \return The shift count required to multiply by blocksPerCluster. */
    u8_t clusterSizeShift(){
      return clusterSizeShift_;
    }
    /** \return The logical block number for the start of file data. */
    u32_t dataStartBlock(){
      return dataStartBlock_;
    }
    /** \return The number of FAT structures on the volume. */
    u8_t fatCount(){
      return fatCount_;
    }
    /** \return The logical block number for the start of the first FAT. */
    u32_t fatStartBlock(){
      return fatStartBlock_;
    }
    /** \return The FAT type of the volume. Values are 12, 16 or 32. */
    u8_t fatType(){
      return fatType_;
    }
    /** \return The number of entries in the root directory for FAT16 volumes. */
    u32_t rootDirEntryCount(){
      return rootDirEntryCount_;
    }
    u32_t rootDirStart(){
      return rootDirStart_;
    }
    u8_t blockOfCluster(u32_t position) {
      return (position >> 9) & (blocksPerCluster_ - 1);
    }
    u32_t clusterStartBlock(u32_t cluster) {
      return dataStartBlock_ + ((cluster - 2) << clusterSizeShift_);
    }
    u32_t blockNumber(u32_t cluster, u32_t position) {
      return clusterStartBlock(cluster) + blockOfCluster(position);
    }
    
    u8_t fatPutEOC(u32_t cluster) {
      return fatPut(cluster, 0x0FFFFFFF);
    }
    u8_t isEOC(u32_t cluster) {
      return  cluster >= (fatType_ == 16 ? FAT16EOC_MIN : FAT32EOC_MIN);
    }


    // SDFILE************

    
void sdfile_init(){
  type_ = FAT_FILE_TYPE_CLOSED;
}
bool openRoot() {
  // error if file is already open
printf("Check open : %d\n",type_);
  if (isOpen()) {
    return false;
  }
printf("Check fat type\n");
  if (fatType() == 16) {
    type_ = FAT_FILE_TYPE_ROOT16;
    firstCluster_ = 0;
    fileSize_ = 32 * rootDirEntryCount();
  } else if (fatType() == 32) {
    type_ = FAT_FILE_TYPE_ROOT32;
    firstCluster_ = rootDirStart();
printf("Check chainsize\n");
    if (!chainSize(firstCluster_, &fileSize_)) {
      return false;
    }
  } else {
    // volume is not initialized or FAT12
    return false;
  }
  // read only
  flags_ = O_READ;

  // set to start of file
  curCluster_ = 0;
  curPosition_ = 0;

  // root has no directory entry
  dirBlock_ = 0;
  dirIndex_ = 0;
  return true;
}

void ls(u8_t flags, u8_t indent) {
  dir_t* p;

  sdfile_rewind();
  while ((p = readDirCache())) {
    // done if past last used entry
    if (p->name[0] == DIR_NAME_FREE) {
      break;
    }

    // skip deleted entry and entries for . and  ..
    if (p->name[0] == DIR_NAME_DELETED || p->name[0] == '.') {
      continue;
    }

    // only list subdirectories and files
    if (!DIR_IS_FILE_OR_SUBDIR(p)) {
      continue;
    }

    // print any indent spaces
    for (int8_t i = 0; i < indent; i++) {
      printf(" ");
    }

    // print file name with possible blank fill
    printDirName(*p, flags & (LS_DATE | LS_SIZE) ? 14 : 0);

    // print modify date/time if requested
    if (flags & LS_DATE) {
      //printf("", p->lastWriteDate);
      printf(" (Date)");
      //printFatTime(p->lastWriteTime);
      printf(" (Time)");
    }
    // print size if requested
    if (!DIR_IS_SUBDIR(p) && (flags & LS_SIZE)) {
      printf(" ");
      printf("%d", p->fileSize);
    }
    printf("\n");

    // list subdirectory content if requested
    /*if ((flags & LS_R) && DIR_IS_SUBDIR(p)) {
      u16_t index = curPosition() / 32 - 1;
      SdFile s;
      if (s.open(this, index, O_READ)) {
        s.ls(flags, indent + 2);
      }
      seekSet(32 * (index + 1));
    }*/
  }
}
void printDirName(dir_t dir, u8_t width) {
  u8_t w = 0;
  for (u8_t i = 0; i < 11; i++) {
    if (dir.name[i] == ' ') {
      continue;
    }
    if (i == 8) {
      printf(".");
      w++;
    }
    printf("%c", dir.name[i]);
    w++;
  }
  if (DIR_IS_SUBDIR(&dir)) {
    printf("/");
    w++;
  }
  while (w < width) {
    printf(" ");
    w++;
  }
 }

 dir_t* readDirCache(void) {
  // error if not directory
  if (!isDir()) {
    return NULL;
  }

  // index of entry in cache
  u8_t i = (curPosition_ >> 5) & 0XF;

  // use read to locate and cache block
  if (sdfile_read_byte() < 0) {
    printf("cannot read : NULL\n");
    return NULL;
  }

  // advance to next entry
  curPosition_ += 31;

  // return pointer to entry
  return (cacheBuffer_.dir + i);
}


int16_t sdfile_read(void* buf, u16_t nbyte) {
  u8_t* dst = (u8_t*)buf;

  // error if not open or write only
  if (!isOpen() || !(flags_ & O_READ)) {
    return -1;
  }

  // max bytes left in file
  if (nbyte > (fileSize_ - curPosition_)) {
    nbyte = fileSize_ - curPosition_;
  }

  // amount left to read
  u16_t toRead = nbyte;
  while (toRead > 0) {
    u32_t block;  // raw device block number
    u16_t offset = curPosition_ & 0X1FF;  // offset in block
    if (type_ == FAT_FILE_TYPE_ROOT16) {
      block = rootDirStart() + (curPosition_ >> 9);
    } else {
      u8_t var_blockOfCluster = blockOfCluster(curPosition_);
      if (offset == 0 && var_blockOfCluster == 0) {
        // start of new cluster
        if (curPosition_ == 0) {
          // use first cluster in file
          curCluster_ = firstCluster_;
        } else {
          // get next cluster from FAT
          if (!fatGet(curCluster_, &curCluster_)) {
            return -1;
          }
        }
      }
      block = clusterStartBlock(curCluster_) + var_blockOfCluster;
    }
    u16_t n = toRead;

    // amount to be read from current block
    if (n > (512 - offset)) {
      n = 512 - offset;
    }

    // no buffering needed if n == 512 or user requests no buffering
    if ((unbufferedRead() || n == 512) &&
        block != cacheBlockNumber_) {
      if (!readData(block, offset, n, dst)) {
        return -1;
      }
      dst += n;
    } else {
      // read block to cache and copy data to caller
      if (!cacheRawBlock(block, CACHE_FOR_READ)) {
        return -1;
      }
      u8_t* src = cacheBuffer_.data + offset;
      u8_t* end = src + n;
      while (src != end) {
        *dst++ = *src++;
      }
    }
    curPosition_ += n;
    toRead -= n;
  }
  return nbyte;
}

    void clearUnbufferedRead(void) {
      flags_ &= ~F_FILE_UNBUFFERED_READ;
    }
    u32_t curCluster(void) {
      return curCluster_;
    }
    u32_t curPosition(void) {
      return curPosition_;
    }
    u32_t dirBlock(){
      return dirBlock_;
    }
    u8_t dirIndex(){
      return dirIndex_;
    }
    u32_t fileSize(){
      return fileSize_;
    }
    /** \return The first cluster number for a file or directory. */
    u32_t firstCluster(){
      return firstCluster_;
    }
    /** \return True if this is a SdFile for a directory else false. */
    u8_t isDir(){
      return type_ >= FAT_FILE_TYPE_MIN_DIR;
    }
    /** \return True if this is a SdFile for a file else false. */
    u8_t isFile(){
      return type_ == FAT_FILE_TYPE_NORMAL;
    }
    /** \return True if this is a SdFile for an open file/directory else false. */
    u8_t isOpen(){
      return type_ != FAT_FILE_TYPE_CLOSED;
    }
    /** \return True if this is a SdFile for a subdirectory else false. */
    u8_t isSubDir(){
      return type_ == FAT_FILE_TYPE_SUBDIR;
    }
    /** \return True if this is a SdFile for the root directory. */
    u8_t isRoot(){
      return type_ == FAT_FILE_TYPE_ROOT16 || type_ == FAT_FILE_TYPE_ROOT32;
    }
    int16_t sdfile_read_byte(void) {
      u8_t b;
      return sdfile_read(&b, 1) == 1 ? b : -1;
    }
    void sdfile_rewind(void) {
      curPosition_ = curCluster_ = 0;
    }
    u8_t seekCur(u32_t pos) {
      return seekSet(curPosition_ + pos);
    }
    u8_t seekEnd(void) {
      return seekSet(fileSize_);
    }

    void setUnbufferedRead(void) {
      if (isFile()) {
        flags_ |= F_FILE_UNBUFFERED_READ;
      }
    }
    u8_t unbufferedRead(){
      return flags_ & F_FILE_UNBUFFERED_READ;
    }


    /**
   Open a file or directory by name.

   \param[in] dirFile An open SdFat instance for the directory containing the
   file to be opened.

   \param[in] fileName A valid 8.3 DOS name for a file to be opened.

   \param[in] oflag Values for \a oflag are constructed by a bitwise-inclusive
   OR of flags from the following list

   O_READ - Open for reading.

   O_RDONLY - Same as O_READ.

   O_WRITE - Open for writing.

   O_WRONLY - Same as O_WRITE.

   O_RDWR - Open for reading and writing.

   O_APPEND - If set, the file offset shall be set to the end of the
   file prior to each write.

   O_CREAT - If the file exists, this flag has no effect except as noted
   under O_EXCL below. Otherwise, the file shall be created

   O_EXCL - If O_CREAT and O_EXCL are set, open() shall fail if the file exists.

   O_SYNC - Call sync() after each write.  This flag should not be used with
   write(u8_t), write_P(PGM_P), writeln_P(PGM_P), or the Arduino Print class.
   These functions do character at a time writes so sync() will be called
   after each byte.

   O_TRUNC - If the file exists and is a regular file, and the file is
   successfully opened and is not read only, its length shall be truncated to 0.

   \note Directory files must be opened read only.  Write and truncation is
   not allowed for directory files.

   \return The value one, true, is returned for success and
   the value zero, false, is returned for failure.
   Reasons for failure include this SdFile is already open, \a difFile is not
   a directory, \a fileName is invalid, the file does not exist
   or can't be opened in the access mode specified by oflag.
*/
bool sdfile_open(const char* fileName, u8_t oflag) {
  u8_t dname[11];
  dir_t* p;

  // error if already open
  //printf("Check open : %d\n",type_);
  /*
  if (isOpen()) {
    return false;
  }*/

  //printf("Check 83\n");
  if (!make83Name(fileName, dname)) {
    return false;
  }
  sdfile_rewind();

  // bool for empty entry found
  u8_t emptyFound = false;

  // search for file
  //printf("Cur pos : %d\n", curPosition_);
  //printf("File Size : %d\n", fileSize_);
  while (curPosition_ < fileSize_) {
    u8_t index = 0XF & (curPosition_ >> 5);
    p = readDirCache();
    //printf("Check p\n");
    if (p == NULL) {
      return false;
    }
    else {
    //printf("%s\n", p->name);
    }

    //printf("P not null\n");
    if (p->name[0] == DIR_NAME_FREE || p->name[0] == DIR_NAME_DELETED) {
      // remember first empty slot
      if (!emptyFound) {
        emptyFound = true;
        dirIndex_ = index;
        dirBlock_ = cacheBlockNumber_;
      }
      // done if no entries follow
      if (p->name[0] == DIR_NAME_FREE) {
        break;
      }
    } else if (!memcmp(dname, p->name, 11)) {
      // don't open existing file if O_CREAT and O_EXCL
      
      //printf("Check flags\n");
      if ((oflag & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL)) {
        return false;
      }

      // open found file
      return openCachedEntry(0XF & index, oflag);
    }
  }
  // only create file if O_CREAT and O_WRITE
  if ((oflag & (O_CREAT | O_WRITE)) != (O_CREAT | O_WRITE)) {
    return false;
  }

  // cache found slot or add cluster if end of file
  if (emptyFound) {
    p = cacheDirEntry(CACHE_FOR_WRITE);
    if (!p) {
      return false;
    }
  } else {
    if (type_ == FAT_FILE_TYPE_ROOT16) {
      return false;
    }

    // add and zero cluster for dirFile - first cluster is in cache for write
    if (!addDirCluster()) {
      return false;
    }

    // use first entry in cluster
    dirIndex_ = 0;
    p = cacheBuffer_.dir;
  }
  // initialize as empty file
  memset(p, 0, sizeof(dir_t));
  memcpy(p->name, dname, 11);

  // set timestamps
  /*
  if (dateTime_) {
    // call user function
    dateTime_(&p->creationDate, &p->creationTime);
  } else {
    // use default date/time
    p->creationDate = FAT_DEFAULT_DATE;
    p->creationTime = FAT_DEFAULT_TIME;
  }*/
  p->lastAccessDate = p->creationDate;
  p->lastWriteDate = p->creationDate;
  p->lastWriteTime = p->creationTime;

  // force write of entry to SD
  if (!cacheFlush()) {
    return false;
  }

  // open entry in cache
  return openCachedEntry(dirIndex_, oflag);
}


u8_t make83Name(const char* str, u8_t* name) {
  u8_t c;
  u8_t n = 7;  // max index for part before dot
  u8_t i = 0;
  // blank fill name and extension
  while (i < 11) {
    name[i++] = ' ';
  }
  i = 0;
  while ((c = *str++) != '\0') {
    if (c == '.') {
      if (n == 10) {
        return false;  // only one dot allowed
      }
      n = 10;  // max index for full 8.3 name
      i = 8;   // place for extension
    } else {
      // illegal FAT characters
      u8_t b;
      const u8_t valid[] = "|<>^+=?/[];,*\"\\";
      const u8_t *p = valid;
      while ((b = *p++)) if (b == c) {
          return false;
        }

      // check size and only allow ASCII printable characters
      if (i > n || c < 0X21 || c > 0X7E) {
        return false;
      }
      // only upper case allowed in 8.3 names - convert lower to upper
      name[i++] = c < 'a' || c > 'z' ?  c : c + ('A' - 'a');
    }
  }
  // must have a file name, extension is optional
  return name[0] != ' ';
}
u8_t openCachedEntry(u8_t dirIndex, u8_t oflag) {
  // location of entry in cache
  dir_t* p = cacheBuffer_.dir + dirIndex;

  // write or truncate is an error for a directory or read-only file
  if (p->attributes & (DIR_ATT_READ_ONLY | DIR_ATT_DIRECTORY)) {
    if (oflag & (O_WRITE | O_TRUNC)) {
      return false;
    }
  }
  // remember location of directory entry on SD
  dirIndex_ = dirIndex;
  dirBlock_ = cacheBlockNumber_;

  // copy first cluster number for directory fields
  firstCluster_ = (u32_t)p->firstClusterHigh << 16;
  firstCluster_ |= p->firstClusterLow;

  // make sure it is a normal file or subdirectory
  if (DIR_IS_FILE(p)) {
    fileSize_ = p->fileSize;
    type_ = FAT_FILE_TYPE_NORMAL;
  } else if (DIR_IS_SUBDIR(p)) {
    if (!chainSize(firstCluster_, &fileSize_)) {
      return false;
    }
    type_ = FAT_FILE_TYPE_SUBDIR;
  } else {
    return false;
  }
  // save open flags for read/write
  flags_ = oflag & (O_ACCMODE | O_SYNC | O_APPEND);

  // set to start of file
  curCluster_ = 0;
  curPosition_ = 0;

  // truncate file to zero length if requested
  if (oflag & O_TRUNC) {
    return truncate(0);
  }
  return true;
}


u8_t   flags() {
  return flags_;
}

u8_t truncate(u32_t length) {
  // error if not a normal file or read-only
  if (!isFile() || !(flags_ & O_WRITE)) {
    return false;
  }

  // error if length is greater than current size
  if (length > fileSize_) {
    return false;
  }

  // fileSize and length are zero - nothing to do
  if (fileSize_ == 0) {
    return true;
  }

  // remember position for seek after truncation
  u32_t newPos = curPosition_ > length ? length : curPosition_;

  // position to last cluster in truncated file
  if (!seekSet(length)) {
    return false;
  }

  if (length == 0) {
    // free all clusters
    if (!freeChain(firstCluster_)) {
      return false;
    }
    firstCluster_ = 0;
  } else {
    u32_t toFree;
    if (!fatGet(curCluster_, &toFree)) {
      return false;
    }

    if (!isEOC(toFree)) {
      // free extra clusters
      if (!freeChain(toFree)) {
        return false;
      }

      // current cluster is end of chain
      if (!fatPutEOC(curCluster_)) {
        return false;
      }
    }
  }
  fileSize_ = length;

  // need to update directory entry
  flags_ |= F_FILE_DIR_DIRTY;

  if (!sync()) {
    return false;
  }

  // set file to correct position
  return seekSet(newPos);
}

u8_t sync(void) {
  // only allow open files and directories
  if (!isOpen()) {
    return false;
  }

  if (flags_ & F_FILE_DIR_DIRTY) {
    dir_t* d = cacheDirEntry(CACHE_FOR_WRITE);
    if (!d) {
      return false;
    }

    // do not set filesize for dir files
    if (!isDir()) {
      d->fileSize = fileSize_;
    }

    // update first cluster fields
    d->firstClusterLow = firstCluster_ & 0XFFFF;
    d->firstClusterHigh = firstCluster_ >> 16;

    // set modify time if user supplied a callback date/time function
    /*if (dateTime_) {
      dateTime_(&d->lastWriteDate, &d->lastWriteTime);
      d->lastAccessDate = d->lastWriteDate;
    }*/
    // clear directory dirty
    flags_ &= ~F_FILE_DIR_DIRTY;
  }
  return cacheFlush();
}
//----------------------------------------------
// Store a FAT entry
u8_t fatPut(u32_t cluster, u32_t value) {
  // error if reserved cluster
  if (cluster < 2) {
    return false;
  }

  // error if not in FAT
  if (cluster > (clusterCount_ + 1)) {
    return false;
  }

  // calculate block address for entry
  u32_t lba = fatStartBlock_;
  lba += fatType_ == 16 ? cluster >> 8 : cluster >> 7;

  if (lba != cacheBlockNumber_) {
    if (!cacheRawBlock(lba, CACHE_FOR_READ)) {
      return false;
    }
  }
  // store entry
  if (fatType_ == 16) {
    cacheBuffer_.fat16[cluster & 0XFF] = value;
  } else {
    cacheBuffer_.fat32[cluster & 0X7F] = value;
  }
  cacheSetDirty();

  // mirror second FAT
  if (fatCount_ > 1) {
    cacheMirrorBlock_ = lba + blocksPerFat_;
  }
  return true;
}

static void cacheSetDirty(void) {
  cacheDirty_ |= CACHE_FOR_WRITE;
}
//------------------------------------------------------------------------------
// cache a file's directory entry
// return pointer to cached entry or null for failure
dir_t* cacheDirEntry(u8_t action) {
  if (!cacheRawBlock(dirBlock_, action)) {
    return NULL;
  }
  return cacheBuffer_.dir + dirIndex_;
}


u8_t seekSet(u32_t pos) {
  // error if file not open or seek past end of file
  if (!isOpen() || pos > fileSize_) {
    return false;
  }

  if (type_ == FAT_FILE_TYPE_ROOT16) {
    curPosition_ = pos;
    return true;
  }
  if (pos == 0) {
    // set position to start of file
    curCluster_ = 0;
    curPosition_ = 0;
    return true;
  }
  // calculate cluster index for cur and new position
  u32_t nCur = (curPosition_ - 1) >> (clusterSizeShift_ + 9);
  u32_t nNew = (pos - 1) >> (clusterSizeShift_ + 9);

  if (nNew < nCur || curPosition_ == 0) {
    // must follow chain from first cluster
    curCluster_ = firstCluster_;
  } else {
    // advance from curPosition
    nNew -= nCur;
  }
  while (nNew--) {
    if (!fatGet(curCluster_, &curCluster_)) {
      return false;
    }
  }
  curPosition_ = pos;
  return true;
}
//------------------------------------------------------------------------------
// free a cluster chain
u8_t freeChain(u32_t cluster) {
  // clear free cluster location
  allocSearchStart_ = 2;

  do {
    u32_t next;
    if (!fatGet(cluster, &next)) {
      return false;
    }

    // free cluster
    if (!fatPut(cluster, 0)) {
      return false;
    }

    cluster = next;
  } while (!isEOC(cluster));

  return true;
}
u8_t addDirCluster(void) {
  if (!addCluster()) {
    return false;
  }

  // zero data in cluster insure first cluster is in cache
  u32_t block = clusterStartBlock(curCluster_);
  for (u8_t i = blocksPerCluster_; i != 0; i--) {
    if (!cacheZeroBlock(block + i - 1)) {
      return false;
    }
  }
  // Increase directory file size by cluster size
  fileSize_ += 512UL << clusterSizeShift_;
  return true;
}
u8_t cacheZeroBlock(u32_t blockNumber) {
  if (!cacheFlush()) {
    return false;
  }

  // loop take less flash than memset(cacheBuffer_.data, 0, 512);
  for (u16_t i = 0; i < 512; i++) {
    cacheBuffer_.data[i] = 0;
  }
  cacheBlockNumber_ = blockNumber;
  cacheSetDirty();
  return true;
}
u8_t addCluster() {
  if (!allocContiguous(1, &curCluster_)) {
    return false;
  }

  // if first cluster of file link to directory entry
  if (firstCluster_ == 0) {
    firstCluster_ = curCluster_;
    flags_ |= F_FILE_DIR_DIRTY;
  }
  return true;
}
u8_t allocContiguous(u32_t count, u32_t* curCluster) {
  // start of group
  u32_t bgnCluster;

  // flag to save place to start next search
  u8_t setStart;

  // set search start cluster
  if (*curCluster) {
    // try to make file contiguous
    bgnCluster = *curCluster + 1;

    // don't save new start location
    setStart = false;
  } else {
    // start at likely place for free cluster
    bgnCluster = allocSearchStart_;

    // save next search start if one cluster
    setStart = 1 == count;
  }
  // end of group
  u32_t endCluster = bgnCluster;

  // last cluster of FAT
  u32_t fatEnd = clusterCount_ + 1;

  // search the FAT for free clusters
  for (u32_t n = 0;; n++, endCluster++) {
    // can't find space checked all clusters
    if (n >= clusterCount_) {
      return false;
    }

    // past end - start from beginning of FAT
    if (endCluster > fatEnd) {
      bgnCluster = endCluster = 2;
    }
    u32_t f;
    if (!fatGet(endCluster, &f)) {
      return false;
    }

    if (f != 0) {
      // cluster in use try next cluster as bgnCluster
      bgnCluster = endCluster + 1;
    } else if ((endCluster - bgnCluster + 1) == count) {
      // done - found space
      break;
    }
  }
  // mark end of chain
  if (!fatPutEOC(endCluster)) {
    return false;
  }

  // link clusters
  while (endCluster > bgnCluster) {
    if (!fatPut(endCluster - 1, endCluster)) {
      return false;
    }
    endCluster--;
  }
  if (*curCluster != 0) {
    // connect chains
    if (!fatPut(*curCluster, bgnCluster)) {
      return false;
    }
  }
  // return first cluster number to caller
  *curCluster = bgnCluster;

  // remember possible next free cluster
  if (setStart) {
    allocSearchStart_ = bgnCluster + 1;
  }

  return true;
}

u8_t sdfile_close(void) {
  if (!sync()) {
    return false;
  }
  type_ = FAT_FILE_TYPE_CLOSED;
  return true;
}