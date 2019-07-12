#ifndef _SDAPI_H_
#define _SDAPI_H_

extern u8_t volume_init(u8_t part);
extern u8_t fatType();

extern u8_t blocksPerCluster();
extern u32_t clusterCount();
// SdFile exclusive :
extern bool openRoot();
extern u32_t rootDirEntryCount();
extern u32_t rootDirStart();

extern u8_t blockOfCluster(u32_t position);
extern u32_t clusterStartBlock(u32_t cluster);
extern u32_t blockNumber(u32_t cluster, u32_t position);

extern u8_t chainSize(u32_t cluster, u32_t* size);
extern void ls(u8_t flags, u8_t indent);


extern bool sdfile_open(const char* fileName, u8_t oflag);
extern u8_t sdfile_close(void);
extern void sdfile_rewind(void);
extern int16_t sdfile_read(void* buf, u16_t nbyte);


extern u8_t   flags(); // See above for definition of flags_ bits
extern u8_t   type();  // type of file see above for values
extern u32_t  curCluster();// cluster for current file position
extern u32_t  curPosition();   // current file position in bytes from beginning
extern u32_t  dirBlock();  // SD block that contains directory entry for file
extern u8_t   dirIndex();  // index of entry in dirBlock 0 <= dirIndex_ <= 0XF
extern u32_t  fileSize();  // file size in bytes
extern u32_t  firstCluster();  // first cluster of file
extern int16_t sdfile_read_byte(void);
#endif /* _SDAPI_H_ */