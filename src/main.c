/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-BSD-5-Clause-Nordic
 */

#include <zephyr.h>
#include <stdio.h>
#include <string.h>
#include <console.h>
#include <misc/reboot.h>
#include <device.h>
#include <spi.h>
#include <gpio.h>
//#include <SD.h>

struct device * gpio_dev;
struct device * spi_dev;
#define PIN1 0
#define PIN2 1

static void gpio_init(void)
{
  const char* const gpioName = "GPIO_0";
  gpio_dev = device_get_binding(gpioName);
  
  if (gpio_dev == NULL) {
    printk("Could not get %s device\n", gpioName);
    return;
  }
  printk("Successfully initialised %s device\n", gpioName);
 
  if(gpio_pin_configure(gpio_dev, PIN1, GPIO_DIR_OUT) < 0)
    printk("Could not configure pin P0.%02d\n", PIN1);
  if(gpio_pin_configure(gpio_dev, PIN2, GPIO_DIR_OUT ) < 0)
    printk("Could not configure pin P0.%02d\n", PIN2);
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
  int i, j, k, result;
  printk("Application started\n");
  gpio_init();
  spi_init();
  
  
  while (1) {
      if (!gpio_pin_write(gpio_dev, PIN1, 1))
        printf("1. P0.%02d : write 1\n", PIN1);
      if (!gpio_pin_write(gpio_dev, PIN2, 1))
        printf("2. P0.%02d : write 1\n", PIN2);
      k_sleep(100000);
      
     // if (!gpio_pin_write(gpio_dev, PIN_OUT, 0))
     //   printf("0 write\n");
      //k_sleep(1000);
//    spi_test_send();
  //  
/*
    }s
    k_sleep(1000);
    
    if (!gpio_pin_write(gpio_dev, PIN_OUT, 0)){
      printf("0 write\n");
      if (!gpio_pin_read(gpio_dev, PIN_IN, &result))
        printf("Read : %d\n", result);
    }*/
  }

}