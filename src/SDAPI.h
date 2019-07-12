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

u8_t chainSize(u32_t cluster, u32_t* size);

extern void sdfile_rewind(void);


#endif /* _SDAPI_H_ */