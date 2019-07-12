/* Arduino SdFat Library
   Copyright (C) 2009 by William Greiman

   This file is part of the Arduino SdFat Library

   This Library is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the Arduino SdFat Library.  If not, see
   <http://www.gnu.org/licenses/>.
*/
#ifndef SdFat_h
#define SdFat_h
/**
   \file
   SdFile and SdVolume classes
*/

//#include "Sd.h
#include "FatStructs.h"


/**
   \brief Cache for an SD data block
*/

union cache_t {
  /* Used to access cached file data blocks.*/ 
  u8_t  data[512];
  /* Used to access cached FAT16 entries. */
  u16_t fat16[256];
  /* Used to access cached FAT32 entries. */
  u32_t fat32[128];
  /* Used to access cached directory entries. */
  dir_t    dir[16];
  /* Used to access a cached MasterBoot Record. */
  mbr_t    mbr;
  /* Used to access to a cached FAT boot sector. */
  fbs_t    fbs;
};

// value for action argument in cacheRawBlock to indicate read from cache
static u8_t const CACHE_FOR_READ = 0;
// value for action argument in cacheRawBlock to indicate cache dirty
static u8_t const CACHE_FOR_WRITE = 1;

static union cache_t cacheBuffer_;        // 512 byte cache for device blocks
static u32_t cacheBlockNumber_ = 0xFFFFFFFF;  // Logical number of block in the cache
//static Sd2Card* sdCard_;            // Sd2Card object for cache
static u8_t cacheDirty_ = 0;         // cacheFlush() will write block if true
static u32_t cacheMirrorBlock_ = 0;  // block number for mirror FAT

    static u8_t cacheFlush(void);


    // Allow SdFile access to SdVolume private data.
//    friend class SdFile;

    // value for action argument in cacheRawBlock to indicate read from cache
    #define CACHE_FOR_READ 0
    // value for action argument in cacheRawBlock to indicate cache dirty
    #define CACHE_FOR_WRITE 1

    //static union cache_t cacheBuffer_;        // 512 byte cache for device blocks
    //static u32_t cacheBlockNumber_;  // Logical number of block in the cache
    //static Sd2Card* sdCard_;            // Sd2Card object for cache
    static u8_t cacheDirty_;         // cacheFlush() will write block if true
    static u32_t cacheMirrorBlock_;  // block number for mirror FAT
    //
    u32_t allocSearchStart_;   // start cluster for alloc search
    u8_t blocksPerCluster_;    // cluster size in blocks
    u32_t blocksPerFat_;       // FAT size in blocks
    u32_t clusterCount_;       // clusters in one FAT
    u8_t clusterSizeShift_;    // shift to convert cluster count to block count
    u32_t dataStartBlock_;     // first data block number
    u8_t fatCount_;            // number of FATs on volume
    u32_t fatStartBlock_;      // start block for first FAT
    u8_t fatType_;             // volume type (12, 16, OR 32)
    u16_t rootDirEntryCount_;  // number of entries in FAT16 root dir
    u32_t rootDirStart_;       // root start block for FAT16, cluster for FAT32
    //----------------------------------------------------------------------------
//------------------------------------------------------------------------------
// forward declaration since SdVolume is used in SdFile
//class SdVolume;

  
//==============================================================================
// SdVolume class

//------------------------------------------------------------------------------
/**
   \class SdVolume
   \brief Access FAT16 and FAT32 volumes on SD and SDHC cards.
*/
//class SdVolume {
//  public:
    /** Create an instance of SdVolume */
    //SdVolume(void) : allocSearchStart_(2), fatType_(0) {}
    /** Clear the cache and returns a pointer to the cache.  Used by the WaveRP
        recorder to do raw write to the SD card.  Not for normal apps.
    */
    
    /*static u8_t* cacheClear(void) {
      cacheFlush();
      cacheBlockNumber_ = 0XFFFFFFFF;
      return cacheBuffer_.data;
    }*/
    /**
       Initialize a FAT volume.  Try partition one first then try super
       floppy format.

       \param[in] dev The Sd2Card where the volume is located.

       \return The value one, true, is returned for success and
       the value zero, false, is returned for failure.  Reasons for
       failure include not finding a valid partition, not finding a valid
       FAT file system or an I/O error.
    */
    //u8_t init(Sd2Card* dev) {
//      return init(dev, 1) ? true : init(dev, 0);
  //  }
    //u8_t init(Sd2Card* dev, u8_t part);

    // inline functions that return volume info
    /** \return The volume's cluster size in blocks. */
    u8_t blocksPerCluster();
    /** \return The number of blocks in one FAT. */
    u32_t blocksPerFat();
    u32_t clusterCount();
    u8_t clusterSizeShift();
    u32_t dataStartBlock();
    u8_t fatCount();
    u32_t fatStartBlock();
    u8_t fatType();
    u32_t rootDirEntryCount();
    u32_t rootDirStart();

    /** \return The logical block number for the start of the root directory
         on FAT16 volumes or the first cluster number on FAT32 volumes. */
    /** return a pointer to the Sd2Card object for this volume */
    /*static Sd2Card* sdCard(void) {
      return sdCard_;
    }*/
    //------------------------------------------------------------------------------
    //#if ALLOW_DEPRECATED_FUNCTIONS
    // Deprecated functions  - suppress cpplint warnings with NOLINT comment
    /** \deprecated Use: u8_t SdVolume::init(Sd2Card* dev); */
    /*u8_t init(Sd2Card& dev) {
      return init(&dev); // NOLINT
    }*/
    /** \deprecated Use: u8_t SdVolume::init(Sd2Card* dev, u8_t vol); */
    /*u8_t init(Sd2Card& dev, u8_t part) {  // NOLINT
      return init(&dev, part);
    }*/
    //#endif  // ALLOW_DEPRECATED_FUNCTIONS
    //------------------------------------------------------------------------------


    u8_t allocContiguous(u32_t count, u32_t* curCluster);
    u8_t blockOfCluster(u32_t position);
    u32_t clusterStartBlock(u32_t cluster);
    u32_t blockNumber(u32_t cluster, u32_t position);
    
    static u8_t cacheRawBlock(u32_t blockNumber, u8_t action);
    static void cacheSetDirty(void);
    static u8_t cacheZeroBlock(u32_t blockNumber);
    u8_t chainSize(u32_t beginCluster, u32_t* size);
    u8_t fatGet(u32_t cluster, u32_t* value);
    u8_t fatPut(u32_t cluster, u32_t value);

    u8_t fatPutEOC(u32_t cluster);
    u8_t freeChain(u32_t cluster);
    u8_t isEOC(u32_t cluster);
    /*
    u8_t readBlock(u32_t block, u8_t* dst) {
      return readBlock(block, dst);
    }
    u8_t readData(u32_t block, u16_t offset,
                     u16_t count, u8_t* dst) {
      return readData(block, offset, count, dst);
    }
    u8_t writeBlock(u32_t block, const u8_t* dst) {
      return writeBlock(block, dst);
    }*/
#endif  // SdFat_h


//SDFILE************************************
//==============================================================================
// SdFile class
#include "FatStructs.h"
/*Should be fixed :*/
    // value for action argument in cacheRawBlock to indicate read from cache
    #define CACHE_FOR_READ 0
    // value for action argument in cacheRawBlock to indicate cache dirty
    #define CACHE_FOR_WRITE 1
    /***********/

static u8_t   flags_; // See above for definition of flags_ bits
static u8_t   type_;  // type of file see above for values
static u32_t  curCluster_;// cluster for current file position
static u32_t  curPosition_;   // current file position in bytes from beginning
static u32_t  dirBlock_;  // SD block that contains directory entry for file
static u8_t   dirIndex_;  // index of entry in dirBlock 0 <= dirIndex_ <= 0XF
static u32_t  fileSize_;  // file size in bytes
static u32_t  firstCluster_;  // first cluster of file

u8_t   flags(); // See above for definition of flags_ bits
u8_t   type();  // type of file see above for values
u32_t  curCluster();// cluster for current file position
u32_t  curPosition();   // current file position in bytes from beginning
u32_t  dirBlock();  // SD block that contains directory entry for file
u8_t   dirIndex();  // index of entry in dirBlock 0 <= dirIndex_ <= 0XF
u32_t  fileSize();  // file size in bytes
u32_t  firstCluster();  // first cluster of file
//static void (*dateTime_)(u16_t* date, u16_t* time);
// flags for ls()
/** ls() flag to print modify date */
#define LS_DATE 1 
/** ls() flag to print file size */
#define LS_SIZE 2 
/** ls() flag for recursive list of subdirectories */
#define LS_R 4 

// use the gnu style oflag in open()
/** open() oflag for reading */
#define O_READ 0X01 
/** open() oflag - same as O_READ */
#define O_RDONLY O_READ 
/** open() oflag for write */
#define O_WRITE 0X02 
/** open() oflag - same as O_WRITE */
#define O_WRONLY O_WRITE 
/** open() oflag for reading and writing */
#define O_RDWR (O_READ | O_WRITE) 
/** open() oflag mask for access modes */
#define O_ACCMODE (O_READ | O_WRITE) 
/** The file offset shall be set to the end of the file prior to each write. */
#define O_APPEND 0X04 
/** synchronous writes - call sync() after each write */
#define O_SYNC 0X08 
/** create the file if nonexistent */
#define O_CREAT 0X10 
/** If O_CREAT and O_EXCL are set, open() shall fail if the file exists */
#define O_EXCL 0X20 
/** truncate the file to zero length */
#define O_TRUNC 0X40 

// flags for timestamp
/** set the file's last access date */
#define T_ACCESS 1 
/** set the file's creation date and time */
#define T_CREATE 2 
/** Set the file's write date and time */
#define T_WRITE 4 
// values for type_
/** This SdFile has not been opened. */
#define FAT_FILE_TYPE_CLOSED 0 
/** SdFile for a file */
#define FAT_FILE_TYPE_NORMAL 1 
/** SdFile for a FAT16 root directory */
#define FAT_FILE_TYPE_ROOT16 2 
/** SdFile for a FAT32 root directory */
#define FAT_FILE_TYPE_ROOT32 3 
/** SdFile for a subdirectory */
#define FAT_FILE_TYPE_SUBDIR 4 
/** Test value for directory type */
#define FAT_FILE_TYPE_MIN_DIR FAT_FILE_TYPE_ROOT16

/** date field for FAT directory entry */
static inline u16_t FAT_DATE(u16_t year, u8_t month, u8_t day) {
  return (year - 1980) << 9 | month << 5 | day;
}
/** year part of FAT directory date field */
static inline u16_t FAT_YEAR(u16_t fatDate) {
  return 1980 + (fatDate >> 9);
}
/** month part of FAT directory date field */
static inline u8_t FAT_MONTH(u16_t fatDate) {
  return (fatDate >> 5) & 0XF;
}
/** day part of FAT directory date field */
static inline u8_t FAT_DAY(u16_t fatDate) {
  return fatDate & 0X1F;
}
/** time field for FAT directory entry */
static inline u16_t FAT_TIME(u8_t hour, u8_t minute, u8_t second) {
  return hour << 11 | minute << 5 | second >> 1;
}
/** hour part of FAT directory time field */
static inline u8_t FAT_HOUR(u16_t fatTime) {
  return fatTime >> 11;
}
/** minute part of FAT directory time field */
static inline u8_t FAT_MINUTE(u16_t fatTime) {
  return (fatTime >> 5) & 0X3F;
}
/** second part of FAT directory time field */
static inline u8_t FAT_SECOND(u16_t fatTime) {
  return 2 * (fatTime & 0X1F);
}
/** Default date for file timestamps is 1 Jan 2000 */
#define FAT_DEFAULT_DATE  ((2000 - 1980) << 9) | (1 << 5) | 1 
/** Default time for file timestamp is 1 am */
#define FAT_DEFAULT_TIME  (1 << 11) 
#define F_OFLAG  (O_ACCMODE | O_APPEND | O_SYNC) 
    // available bits
#define F_UNUSED  0X30 
    // use unbuffered SD read
#define F_FILE_UNBUFFERED_READ  0X40 
    // sync of directory entry required
#define F_FILE_DIR_DIRTY  0X80 
//------------------------------------------------------------------------------
/**
   \class SdFile
   \brief Access FAT16 and FAT32 files on SD and SDHC cards.
*/

    /** Create an instance of SdFile. */
    //SdFile(void) : type_(FAT_FILE_TYPE_CLOSED) {}
    /**
       writeError is set to true if an error occurs during a write().
       Set writeError to false before calling print() and/or write() and check
       for true after calls to print() and/or write().
    */
    //bool writeError;
    /**
       Cancel unbuffered reads for this file.
       See setUnbufferedRead()
    */u8_t addCluster(void);
u8_t addDirCluster(void);
dir_t* cacheDirEntry(u8_t action);
static u8_t make83Name(const char* str, u8_t* name);
u8_t openCachedEntry(u8_t cacheIndex, u8_t oflags);
dir_t* readDirCache(void);

    void clearUnbufferedRead(void);
    u8_t sdfile_close(void);
    u8_t contiguousRange(u32_t* bgnBlock, u32_t* endBlock);
    /*u8_t createContiguous(SdFile* dirFile,
                             const char* fileName, u32_t size);*/
    /** \return The current cluster number for a file or directory. */
    u32_t curCluster(void);
    /** \return The current position for a file or directory. */
    u32_t curPosition(void);
    /**
       Set the date/time callback function

       \param[in] dateTime The user's call back function.  The callback
       function is of the form:

       \code
       void dateTime(u16_t* date, u16_t* time) {
         u16_t year;
         u8_t month, day, hour, minute, second;

         // User gets date and time from GPS or real-time clock here

         // return date using FAT_DATE macro to format fields
     *   *date = FAT_DATE(year, month, day);

         // return time using FAT_TIME macro to format fields
     *   *time = FAT_TIME(hour, minute, second);
       }
       \endcode

       Sets the function that is called when a file is created or when
       a file's directory entry is modified by sync(). All timestamps,
       access, creation, and modify, are set when a file is created.
       sync() maintains the last access date and last modify date/time.

       See the timestamp() function.
    */
    /*static void dateTimeCallback(
      void (*dateTime)(u16_t* date, u16_t* time)) {
      dateTime_ = dateTime;
    }
    
    //   Cancel the date/time callback function.
    */
    /*static void dateTimeCallbackCancel(void) {
      // use explicit zero since NULL is not defined for Sanguino
      dateTime_ = 0;
    }*/
    /** \return Address of the block that contains this file's directory. */
    

    
    u32_t dirBlock();
    u8_t dirEntry(dir_t* dir);
    /** \return Index of this file's directory in the block dirBlock. */
    u8_t dirIndex();
    //static void dirName(const dir_t * dir, char* name);
    /** \return The total number of bytes in a file or directory. */
    u32_t fileSize();
    /** \return The first cluster number for a file or directory. */
    u32_t firstCluster();
    /** \return True if this is a SdFile for a directory else false. */
    u8_t isDir();
    /** \return True if this is a SdFile for a file else false. */
    u8_t isFile();
    /** \return True if this is a SdFile for an open file/directory else false. */
    u8_t isOpen();
    /** \return True if this is a SdFile for a subdirectory else false. */
    u8_t isSubDir();
    /** \return True if this is a SdFile for the root directory. */
    u8_t isRoot();
    void ls(u8_t flags, u8_t indent);
    //u8_t makeDir(SdFile* dir, const char* dirName);
    //u8_t open(u16_t index, u8_t oflag);
    bool sdfile_open(const char* fileName, u8_t oflag);

    //u8_t openRoot(SdVolume* vol);
    static void printDirName(dir_t dir, u8_t width);
  /*  
    static void printFatDate(u16_t fatDate);
    static void printFatTime(u16_t fatTime);
    static void printTwoDigits(u8_t v);*/
    /**
       Read the next byte from a file.

       \return For success read returns the next byte in the file as an int.
       If an error occurs or end of file is reached -1 is returned.
    */
    
    int16_t sdfile_read(void* buf, u16_t nbyte);
    int16_t sdfile_read_byte(void);
    //int8_t readDir(dir_t* dir);
    //static u8_t remove(SdFile* dirFile, const char* fileName);
    u8_t sdfile_remove(void);
    /** Set the file's current position to zero. */
    void sdfile_rewind(void);
    u8_t rmDir(void);
    u8_t rmRfStar(void);
    /** Set the files position to current position + \a pos. See seekSet(). */

    u8_t seekSet(u32_t pos);
    u8_t seekCur(u32_t pos);
    /**
        Set the files current position to end of file.  Useful to position
        a file for append. See seekSet().
    */
    u8_t seekEnd(void);
    /**
       Use unbuffered reads to access this file.  Used with Wave
       Shield ISR.  Used with Sd2Card::partialBlockRead() in WaveRP.

       Not recommended for normal applications.
    */
    void setUnbufferedRead(void);
    u8_t timestamp(u8_t flag, u16_t year, u8_t month, u8_t day,
                      u8_t hour, u8_t minute, u8_t second);
    u8_t sync(void);
    /** Type of this SdFile.  You should use isFile() or isDir() instead of type()
       if possible.

       \return The file or directory type.
    */
    
    u8_t type();
    u8_t truncate(u32_t size);
    /** \return Unbuffered read flag. */
    u8_t unbufferedRead();
    /** \return SdVolume that contains this file. */
    //SdVolume* volume(){
      //return vol_;
    //}
    size_t write_byte(u8_t b);
    size_t write(const void* buf, u16_t nbyte);
    //size_t write(const char* str);
