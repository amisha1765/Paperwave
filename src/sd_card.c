//initializations
#include "sd_card.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

//define variables
#define SD_SPI spi1
#define SD_CS_PIN   9
#define SD_MISO_PIN 8
#define SD_MOSI_PIN 11
#define SD_SCK_PIN  10

static bool sd_check_cmd8(void);

// SPI initialization.
static void sd_spi_init(void){
    //400kHZ for now, can increase later this is for testing purposes
    spi_init(SD_SPI, 400 * 1000);
    spi_set_format(SD_SPI, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    // Configure SPI GPIO pins
    gpio_set_function(SD_MISO_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SD_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SD_SCK_PIN, GPIO_FUNC_SPI);

    // Configure chip select as a normal GPIO output
    gpio_init(SD_CS_PIN);
    gpio_set_dir(SD_CS_PIN, GPIO_OUT);
    gpio_put(SD_CS_PIN, 1);
}

// Send one byte and receive one byte
static uint8_t sd_spi_transfer(uint8_t data){
    uint8_t received;
    spi_write_read_blocking(SD_SPI, &data, &received, 1);
    return received;
}

// Receive multiple bytes
static void sd_spi_receive(uint8_t *buffer, uint32_t length){
    spi_read_blocking(SD_SPI, 0xFF, buffer, length);
}

//chip select functions
static void sd_select(void){
    //when CS signal LOW, communicate with SD card
    gpio_put(SD_CS_PIN, 0);
}

static void sd_deselect(void){
    //when CS signal HIGH, stop selecting SD card
    gpio_put(SD_CS_PIN, 1);
    // Send extra clock cycles while deselected
    sd_spi_transfer(0xFF);
}

//SD card intitalization
static uint8_t sd_send_command(uint8_t command, uint32_t argument, uint8_t crc){
    uint8_t response;
    sd_select(); // Select the SD card
    sd_spi_transfer(0xFF); // Give the card an extra clock cycle
    sd_spi_transfer(0x40 | command); // Send command packet
    // Use this bc the starting has 01 start bits and 6 bit command number, the OR basically adds the starting bits 

    // Send 32-bit argument, one byte at a time
    sd_spi_transfer((argument >> 24) & 0xFF);
    sd_spi_transfer((argument >> 16) & 0xFF);
    sd_spi_transfer((argument >> 8) & 0xFF);
    sd_spi_transfer(argument & 0xFF);

    // Send CRC
    sd_spi_transfer(crc); //Fixed at the end of CMD0

    // Wait for the card's response
    for (int i = 0; i < 10; i++)
    {
        response = sd_spi_transfer(0xFF);

        // Bit 7 becomes zero when an R1 response arrives
        if ((response & 0x80) == 0) {return response;}
    }
    // No valid response received
    return 0xFF;
}

// Initialize the SD card
bool sd_card_init(void){
    sd_spi_init();

    // Send at least 74 clock cycles with CS HIGH
    for (int i = 0; i < 10; i++) {sd_spi_transfer(0xFF);}

    // CMD0: Reset SD card
    uint8_t response = sd_send_command(0, 0, 0x95);
    sd_deselect();
    if (response != 0x01) {return false;}

    // CMD8: Check SD version and voltage
    if (!sd_check_cmd8()) {return false;}
    return true;
}

//implement CMD 8
static bool sd_check_cmd8(void) {
    // Send CMD8 with voltage and check-pattern argument
    uint8_t response = sd_send_command(8, 0x000001AA, 0x87);

    // CMD8 should return 0x01 for an SD v2 card in idle state
    if (response != 0x01) {
        sd_deselect();
        return false;
    }

    // CMD8 returns four additional bytes (R7 response)
    uint8_t r7[4];

    for (int i = 0; i < 4; i++) {
        r7[i] = sd_spi_transfer(0xFF);
    }
    sd_deselect();

    // Check that the card accepted the voltage and check pattern
    if (r7[2] != 0x01 || r7[3] != 0xAA) {return false;}
    return true;
}