
#define PIN_OUT 0
#define PIN_IN 1

extern struct device *gpio_dev;
extern struct device *spi_dev;

//void spi_init(void);
void spi_test_send(void);
void spiSend(u8_t data);