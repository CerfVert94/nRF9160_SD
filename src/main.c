/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-identifier: LicenseRef-BSD-5-Clause-Nordic
 */
#include <zephyr.h>
#include <stdio.h>
#include <string.h>
#include <console.h>
#include <misc/reboot.h>
#include <device.h>
#include <spi.h>
#include <gpio.h>
#include <SD.h>
#include <SDAPI.h>

struct device * gpio_dev;
struct device * spi_dev;
#define PIN1 2
#define PIN2 3
#define PIN4 18
#define PIN5 19

static void gpio_init(void)
{
  const char* const gpioName = "GPIO_0";
  gpio_dev = device_get_binding(gpioName);
  
  if (gpio_dev == NULL) {
    printk("Could not get %s device\n", gpioName);
    return;
  }
  printk("Successfully initialised %s device\n", gpioName);
 
  if(gpio_pin_configure(gpio_dev, CS_PIN, GPIO_DIR_OUT) < 0)
    printk("Could not configure PIN P0.%02d\n", CS_PIN);
}
static void spi_init(void)
{
  const char* const spiName = "SPI_3";
  spi_dev = device_get_binding(spiName);  

  if (spi_dev == NULL) {
    printk("Could not get %s device\n", spiName);
    return;
  }
  printk("Successfully initialised %s device\n", spiName);
}
void main(void)
{
  printk("Application started\n");
  gpio_init();
  spi_init();

  //Initialize SD card.
  init(0);
  printf("Card type : ");
  
  switch (get_type()) {
    case SD_CARD_TYPE_SD1:
      printf("SD1\n");
      break;
    case SD_CARD_TYPE_SD2:
      printf("SD2\n ");
      break;
    case SD_CARD_TYPE_SDHC:
      printf("SDHC\n");
      break;
    default:
      printf("Unknown\n");
      break;
  }
  if (!volume_init(1)) {
    //printf("\n");
  }
  else if (!volume_init(0)) {
    //printf("Successful operations : 0\n");
  }
  else {
    printf("Failed initializing the volume\n");
  }
  printf("Fat%d\n", fatType());
  u32_t volumesize;
  volumesize = blocksPerCluster();    // clusters are collections of blocks
  volumesize *= clusterCount();       // we'll have a lot of clusters
  volumesize /= 2;                           // SD card blocks are always 512 bytes (2 blocks are 1KB)
  printf("Volume size (Kb):  ");
  printf("%d\n", volumesize);
  printf("Volume size (Mb):  ");
  volumesize /= 1024;
  printf("%d\n", volumesize);
  printf("Volume size (Gb):  ");
  printf("%f\n", (float)volumesize / 1024.0);
  if(!openRoot()) {
    printf("Failed opening root\n");
  }
  ls(4 | 2);
  printf("Succesful Operations.\n");
  while (1) {
      //printf("%lld (Start : %lld)\n", k_uptime_get() - t0, t0);
      //k_sleesp(1000);
   }
 }