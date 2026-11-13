#ifndef SD_CARD_H
#define SD_CARD_H

#include <stdint.h>
#include <stdbool.h>

bool sd_card_init(void);

bool sd_card_read_sector(uint32_t sector, uint8_t *buffer);

uint32_t sd_card_get_sector_count(void);

#endif

//check