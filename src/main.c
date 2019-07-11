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
  int i;
  u64_t t0;
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
  //i = 0;
  //t0 = k_uptime_get();
  while (1) {
      //printf("%lld (Start : %lld)\n", k_uptime_get() - t0, t0);
      //k_sleep(1000);
   }
 }