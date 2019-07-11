#include <stdio.h>
#include <string.h>
#include <console.h>
#include <spi.h>
#include <SD.h>
#include <SdInfo.h>
#include <gpio.h>

#define SD_PROTECT_BLOCK_ZERO 1




u32_t block_ = 0;
u8_t chipSelectPin_ = 0;
u8_t errorCode_ = 0;
u8_t inBlock_ = 0;
u16_t offset_ = 0;
u8_t partialBlockRead_ = 0;
u8_t status_ = 0;
u8_t type_ = 0;

static const struct spi_cs_control cs = {
  .gpio_dev = NULL
};

static const struct spi_config spi_cfg = {
  .operation =  SPI_WORD_SET(8) | SPI_TRANSFER_MSB |
                SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_CS_ACTIVE_HIGH,
  .frequency = 250000,
  .slave = 0,
  .cs = &cs
};


void set_type(u8_t value) {
      type_ = value;
}

u8_t get_type(void){
      return type_;
}
void set_error(u8_t code) {
      errorCode_ = code;
}


void spi_test_send(void)
{
	int err;
	static u8_t tx_buffer[1] = {0xAA};
	static u8_t rx_buffer[1];

	const struct spi_buf tx_buf = {
		.buf = tx_buffer,
		.len = sizeof(tx_buffer)
	};
	const struct spi_buf_set tx = {
		.buffers = &tx_buf,
		.count = 1
	};

	struct spi_buf rx_buf = {
		.buf = rx_buffer,
		.len = sizeof(rx_buffer),
	};  
	const struct spi_buf_set rx = {
		.buffers = &rx_buf,
		.count = 1
	};
	err = spi_transceive(spi_dev, &spi_cfg, &tx, &rx);

	if (err) {
		printk("SPI error: %d\n", err);
	} else {
                // Connect MISO to MOSI for loopback
		printk("TX sent: %x\n", tx_buffer[0]);
		printk("RX recv: %x\n", rx_buffer[0]);
		tx_buffer[0]++;
	}	
}


void spiSend(u8_t data)
{
	int err;
	static u8_t tx_buffer[1];
	
        tx_buffer[0] = data;

	const struct spi_buf tx_buf = {
		.buf = tx_buffer,
		.len = sizeof(tx_buffer)
	};
	const struct spi_buf_set tx = {
		.buffers = &tx_buf,
		.count = 1
	};

        //chipSelectLow();
	err = spi_transceive(spi_dev, &spi_cfg, &tx, NULL);
        //chipSelectHigh();
	
        if (err) {
		printk("SPI error: %d\n", err);
	} else {
		//printk("TX sent: %x\n", tx_buffer[0]);
	}	
}

u8_t spiRec()
{
	int err;
	static u8_t rx_buffer[1];
	
        struct spi_buf rx_buf = {
		.buf = rx_buffer,
		.len = sizeof(rx_buffer),
	};  
	const struct spi_buf_set rx = {
		.buffers = &rx_buf,
		.count = 1
	};
        //chipSelectHigh();
	err = spi_transceive(spi_dev, &spi_cfg, NULL, &rx);
        //chipSelectLow();
	
        if (err) {
		printk("SPI error: %d\n", err);
	} else {
		//printk("RX recv: %x\n", rx_buffer[0]);
	}	
        return rx_buffer[0];
}

void chipSelectLow() 
{
  gpio_pin_write(gpio_dev, CS_PIN, 0);
  chipSelectPin_ = false;
}

void chipSelectHigh() 
{
  gpio_pin_write(gpio_dev, CS_PIN, 1);
  chipSelectPin_ = true;
}

u8_t init(u8_t sckRateID) {
  errorCode_ = inBlock_ = partialBlockRead_ = type_ = 0;
  // 16-bit init start time allows over a minute

  u64_t t0 = k_uptime_get();
  u32_t arg;

  // set pin modes
  chipSelectHigh();
  
  // must supply min of 74 clock cycles with CS high.
  for (u8_t i = 0; i < 10; i++) {
    spiSend(0XFF);
  }

  chipSelectLow();
  printf("CMD0\n");
  // command to go idle in SPI mode
  while ((status_ = cardCommand(CMD0, 0)) != R1_IDLE_STATE) {
    u64_t d = k_uptime_get() - t0;
    if (d > SD_INIT_TIMEOUT) {
      set_error(SD_CARD_ERROR_CMD0);
      goto fail;
    }
  }
  // check SD version
  printf("CMD8\n");
  if ((cardCommand(CMD8, 0x1AA) & R1_ILLEGAL_COMMAND)) {
    type_ = (SD_CARD_TYPE_SD1);
  } else {
    // only need last byte of r7 response
    for (u8_t i = 0; i < 4; i++) {
      status_ = spiRec();
    }
    if (status_ != 0XAA) {
      set_error(SD_CARD_ERROR_CMD8);
      goto fail;
    }
    type_ = (SD_CARD_TYPE_SD2);
  }
  // initialize card and send host supports SDHC if SD2
  arg = (type_ == SD_CARD_TYPE_SD2 ? 0X40000000 : 0);
  printf("ACMD41\n");

  while ((status_ = cardAcmd(ACMD41, arg)) != R1_READY_STATE) {
    // check for timeout
    u64_t d = k_uptime_get() - t0;
    if (d > SD_INIT_TIMEOUT) {
      set_error(SD_CARD_ERROR_ACMD41);
      goto fail;
    }
  }
  // if SD2 read OCR register to check for SDHC card
  if (type_ == SD_CARD_TYPE_SD2) {
    if (cardCommand(CMD58, 0)) {
      set_error(SD_CARD_ERROR_CMD58);
      goto fail;
    }
    if ((spiRec() & 0XC0) == 0XC0) {
      type_ = (SD_CARD_TYPE_SDHC);
    }
    printf("Recv\n");
    // discard rest of ocr - contains allowed voltage range
    for (u8_t i = 0; i < 3; i++) {
      spiRec();
    }
  }
  chipSelectHigh();
  

//Note : In case of failure, maybe set to the full speed
  return true;

fail:
  printf("Error occured in retrieving card info.\n");
  chipSelectHigh();
  return false;
}


//------------------------------------------------------------------------------
// wait for card to go not busy
u8_t waitNotBusy(unsigned int timeoutMillis) {
  u64_t t0 = k_uptime_get();
  u64_t d;
  do {
    if (spiRec() == 0XFF) {
      return true;
    }
    d = k_uptime_get() - t0;
    printf("d / t0 : %lld / %lld \n", d ,t0);
  } while (d < timeoutMillis);
  return false;
}

//------------------------------------------------------------------------------
// send command and return error code.  Return zero for OK
u8_t cardCommand(u8_t cmd, u32_t arg) {
  // end read if in partialBlockRead mode
  readEnd();

  // select card
  chipSelectLow();

  // wait up to 300 ms if busy
  // send command
  spiSend(cmd | 0x40);

  // send argument
  for (int s = 24; s >= 0; s -= 8) {
    spiSend(arg >> s);
  }

  // send CRC
  u8_t crc = 0XFF;
  if (cmd == CMD0) {
    crc = 0X95;  // correct crc for CMD0 with arg 0
  }
  if (cmd == CMD8) {
    crc = 0X87;  // correct crc for CMD8 with arg 0X1AA
  }
  spiSend(crc);

  
  // wait for response
  for (u8_t i = 0; ((status_ = spiRec()) & 0X80) && i != 0XFF; i++);
  return status_;
}

u8_t cardAcmd(u8_t cmd, u32_t arg) {
  cardCommand(CMD55, 0);
  return cardCommand(cmd, arg);
}

























//------------------------------------------------------------------------------
/**
   Enable or disable partial block reads.

   Enabling partial block reads improves performance by allowing a block
   to be read over the SPI bus as several sub-blocks.  Errors may occur
   if the time between reads is too long since the SD card may timeout.
   The SPI SS line will be held low until the entire block is read or
   readEnd() is called.

   Use this for applications like the Adafruit Wave Shield.

   \param[in] value The value TRUE (non-zero) or FALSE (zero).)
*/
void partialBlockRead(u8_t value) {
  readEnd();
  partialBlockRead_ = value;
}
//------------------------------------------------------------------------------
/**
   Read a 512 byte block from an SD card device.

   \param[in] block Logical block to be read.
   \param[out] dst Pointer to the location that will receive the data.

   \return The value one, true, is returned for success and
   the value zero, false, is returned for failure.
*/
u8_t readBlock(u32_t block, u8_t* dst) {
  return readData(block, 0, 512, dst);
}
//------------------------------------------------------------------------------
/**
   Read part of a 512 byte block from an SD card.

   \param[in] block Logical block to be read.
   \param[in] offset Number of bytes to skip at start of block
   \param[out] dst Pointer to the location that will receive the data.
   \param[in] count Number of bytes to read
   \return The value one, true, is returned for success and
   the value zero, false, is returned for failure.
*/
u8_t readData(u32_t block, u16_t offset, u16_t count, u8_t* dst) {

  if (count == 0) {
    return true;
  }

  if ((count + offset) > 512) {
    goto fail;
  }

  if (!inBlock_ || block != block_ || offset < offset_) {
    block_ = block;
    // use address if not SDHC card
    if (type_ != SD_CARD_TYPE_SDHC) {
      block <<= 9;
    }
    if (cardCommand(CMD17, block)) {
      set_error(SD_CARD_ERROR_CMD17);
      goto fail;
    }
    if (!waitStartBlock()) {
      goto fail;
    }
    offset_ = 0;
    inBlock_ = 1;
  }

  // skip data before offset
  for (; offset_ < offset; offset_++) {
    spiRec();
  }
  // transfer data
  for (u16_t i = 0; i < count; i++) {
    dst[i] = spiRec();
  }

  offset_ += count;
  if (!partialBlockRead_ || offset_ >= 512) {
    // read rest of data, checksum and set chip select high
    readEnd();
  }
  return true;

fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** Skip remaining data in a block when in partial block read mode. */
void readEnd(void) {
  if (inBlock_) {
    // skip data and crc
    while (offset_++ < 514) {
      spiRec();
    }
    chipSelectHigh();
    inBlock_ = 0;
  }
}
//------------------------------------------------------------------------------
/** Wait for start block token */
u8_t waitStartBlock(void) {
  u64_t t0 = k_uptime_get();
  while ((status_ = spiRec()) == 0XFF) {
    u64_t d = k_uptime_get() - t0;
    if (d > SD_READ_TIMEOUT) {
      set_error(SD_CARD_ERROR_READ_TIMEOUT);
      goto fail;
    }
  }
  if (status_ != DATA_START_BLOCK) {
    set_error(SD_CARD_ERROR_READ);
    goto fail;
  }
  return true;

fail:
  chipSelectHigh();
  return false;
}



//------------------------------------------------------------------------------
/** Write one data block in a multiple block write sequence */
/*u8_t writeBlock(const u8_t* src) {
  // wait for previous write to finish
  if (!waitNotBusy(SD_WRITE_TIMEOUT)) {
    set_error(SD_CARD_ERROR_WRITE_MULTIPLE);
    chipSelectHigh();
    return false;
  }
  return writeData(WRITE_MULTIPLE_TOKEN, src);
}*/
//------------------------------------------------------------------------------
// send one block of data for write block or write multiple blocks
u8_t writeData(u8_t token, const u8_t* src) {

  spiSend(token);
  for (u16_t i = 0; i < 512; i++) {
    spiSend(src[i]);
  }
 
  spiSend(0xff);  // dummy crc
  spiSend(0xff);  // dummy crc

  status_ = spiRec();
  if ((status_ & DATA_RES_MASK) != DATA_RES_ACCEPTED) {
    set_error(SD_CARD_ERROR_WRITE);
    chipSelectHigh();
    return false;
  }
  return true;
}
//------------------------------------------------------------------------------
/** Start a write multiple blocks sequence.

   \param[in] blockNumber Address of first block in sequence.
   \param[in] eraseCount The number of blocks to be pre-erased.

   \note This function is used with writeData() and writeStop()
   for optimized multiple block writes.

   \return The value one, true, is returned for success and
   the value zero, false, is returned for failure.
*/
u8_t writeStart(u32_t blockNumber, u32_t eraseCount) {
  #if SD_PROTECT_BLOCK_ZERO
  // don't allow write to first block
  if (blockNumber == 0) {
    set_error(SD_CARD_ERROR_WRITE_BLOCK_ZERO);
    goto fail;
  }
  #endif  // SD_PROTECT_BLOCK_ZERO

  // send pre-erase count
  if (cardAcmd(ACMD23, eraseCount)) {
    set_error(SD_CARD_ERROR_ACMD23);
    goto fail;
  }
  // use address if not SDHC card
  if (type_ != SD_CARD_TYPE_SDHC) {
    blockNumber <<= 9;
  }
  if (cardCommand(CMD25, blockNumber)) {
    set_error(SD_CARD_ERROR_CMD25);
    goto fail;
  }
  return true;

fail:
  chipSelectHigh();
  return false;
}
//------------------------------------------------------------------------------
/** End a write multiple blocks sequence.

  \return The value one, true, is returned for success and
   the value zero, false, is returned for failure.
*/
u8_t writeStop(void) {
  if (!waitNotBusy(SD_WRITE_TIMEOUT)) {
    goto fail;
  }
  spiSend(STOP_TRAN_TOKEN);
  if (!waitNotBusy(SD_WRITE_TIMEOUT)) {
    goto fail;
  }
  chipSelectHigh();
  return true;

fail:
  set_error(SD_CARD_ERROR_STOP_TRAN);
  chipSelectHigh();
  return false;
}

u8_t writeBlock(u32_t blockNumber, const u8_t* src) {
  #if SD_PROTECT_BLOCK_ZERO
  // don't allow write to first block
  if (blockNumber == 0) {
    set_error(SD_CARD_ERROR_WRITE_BLOCK_ZERO);
    goto fail;
  }
  #endif  // SD_PROTECT_BLOCK_ZERO

  // use address if not SDHC card
  if (type_ != SD_CARD_TYPE_SDHC) {
    blockNumber <<= 9;
  }
  if (cardCommand(CMD24, blockNumber)) {
    set_error(SD_CARD_ERROR_CMD24);
    goto fail;
  }
  if (!writeData(DATA_START_BLOCK, src)) {
    goto fail;
  }

  // wait for flash programming to complete
  if (!waitNotBusy(SD_WRITE_TIMEOUT)) {
    set_error(SD_CARD_ERROR_WRITE_TIMEOUT);
    goto fail;
  }
  // response is r2 so get and check two bytes for nonzero
  if (cardCommand(CMD13, 0) || spiRec()) {
    set_error(SD_CARD_ERROR_WRITE_PROGRAMMING);
    goto fail;
  }
  chipSelectHigh();
  return true;

fail:
  chipSelectHigh();
  return false;
}
