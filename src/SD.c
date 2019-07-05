#include <stdio.h>
#include <string.h>
#include <console.h>
#include <spi.h>
#include <SD.h>
#include <SdInfo.h>
#include <gpio.h>
static const struct spi_cs_control cs = {
  .gpio_dev = NULL
};

static const struct spi_config spi_cfg = {
  .operation =  SPI_WORD_SET(8) | SPI_TRANSFER_MSB |
                SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_CS_ACTIVE_HIGH,
  .frequency = 250000,
  .slave = 0,
  //.cs = &cs
};
void spi_test_send(void)
{
	int err, result;
	static u8_t tx_buffer[1];
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

        if (!gpio_pin_read(gpio_dev, PIN_IN, &result))
          printf("Before Transceive, CS : %d\n", result);
	err = spi_transceive(spi_dev, &spi_cfg, &tx, &rx);
        
        if (!gpio_pin_read(gpio_dev, PIN_IN, &result))
          printf("Before Transceive, CS : %d\n", result);
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
	static u8_t rx_buffer[1];

        tx_buffer[0] = data;

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
	}	
}


/*
uint8_t Sd2Card::cardCommand(uint8_t cmd, u32_t arg) {

  // wait up to 300 ms if busy
  k_sleep(300);

  // send command
  spiSend(cmd | 0x40);

  // send argument
  for (int8_t s = 24; s >= 0; s -= 8) {
    spiSend(arg >> s);
  }

  // send CRC
  uint8_t crc = 0XFF;
  if (cmd == CMD0) {
    crc = 0X95;  // correct crc for CMD0 with arg 0
  }
  if (cmd == CMD8) {
    crc = 0X87;  // correct crc for CMD8 with arg 0X1AA
  }
  spiSend(crc);

  // wait for response
  for (uint8_t i = 0; ((status_ = spiRec()) & 0X80) && i != 0XFF; i++);
  return status_;
}

static uint8_t spiRec(void) {

  spiSend(0XFF);
  return 0;
}

*/
/*
uint8_t Sd2Card::init(uint8_t sckRateID, uint8_t chipSelectPin) {
//  errorCode_ = inBlock_ = partialBlockRead_ = type_ = 0;
//  chipSelectPin_ = chipSelectPin;
  // 16-bit init start time allows over a minute
  unsigned int t0 = millis();
  u32_t arg;

  // set pin modes

  spi_context_cs_control(ctx, true);
  for (int i = 0; i < 10; i++) {
    spiSend(0XFF);
  }

  spi_context_cs_control(ctx, false);
  chipSelectLow();

  // command to go idle in SPI mode
  while ((status_ = cardCommand(CMD0, 0)) != R1_IDLE_STATE) {
    unsigned int d = millis() - t0;
    if (d > SD_INIT_TIMEOUT) {
      error(SD_CARD_ERROR_CMD0);
      goto fail;
    }
  }
  // check SD version
  if ((cardCommand(CMD8, 0x1AA) & R1_ILLEGAL_COMMAND)) {
    type(SD_CARD_TYPE_SD1);
  } else {
    // only need last byte of r7 response
    for (uint8_t i = 0; i < 4; i++) {
      status_ = spiRec();
    }
    if (status_ != 0XAA) {
      error(SD_CARD_ERROR_CMD8);
      goto fail;
    }
    type(SD_CARD_TYPE_SD2);
  }
  // initialize card and send host supports SDHC if SD2
  arg = type() == SD_CARD_TYPE_SD2 ? 0X40000000 : 0;

  while ((status_ = cardAcmd(ACMD41, arg)) != R1_READY_STATE) {
    // check for timeout
    unsigned int d = millis() - t0;
    if (d > SD_INIT_TIMEOUT) {
      error(SD_CARD_ERROR_ACMD41);
      goto fail;
    }
  }
  // if SD2 read OCR register to check for SDHC card
  if (type() == SD_CARD_TYPE_SD2) {
    if (cardCommand(CMD58, 0)) {
      error(SD_CARD_ERROR_CMD58);
      goto fail;
    }
    if ((spiRec() & 0XC0) == 0XC0) {
      type(SD_CARD_TYPE_SDHC);
    }
    // discard rest of ocr - contains allowed voltage range
    for (uint8_t i = 0; i < 3; i++) {
      spiRec();
    }
  }
  chipSelectHigh();

  #ifndef SOFTWARE_SPI
  return setSckRate(sckRateID);
  #else  // SOFTWARE_SPI
  return true;
  #endif  // SOFTWARE_SPI

fail:
  chipSelectHigh();
  return false;
}*/