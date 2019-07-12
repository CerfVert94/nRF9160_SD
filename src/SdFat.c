
#include <zephyr.h>
#include <SdFat.h>
#include <SD.h>
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
printf("Check open\n");
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

    u8_t type(){
      return type_;
    }
    void setUnbufferedRead(void) {
      if (isFile()) {
        flags_ |= F_FILE_UNBUFFERED_READ;
      }
    }
    u8_t unbufferedRead(){
      return flags_ & F_FILE_UNBUFFERED_READ;
    }