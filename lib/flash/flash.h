#ifndef _FLASH_H_
#define _FLASH_H_

#include "stm32f30x_conf.h"

#define FLASH_START 0x08000000
#define FLASH_END 0x08010000
#define PG31_BASE 0x0800F800
#define PGnn_SIZE 2048

/*****************************/
/*** STM32 FLASH Helper Functions ***/
/*****************************/
void init_page_flash(uint32_t baseaddr);

uint16_t read_hword_flash(uint32_t baseaddr, uint16_t offset);

uint32_t read_word_flash(uint32_t baseaddr, uint16_t offset);

float read_float_flash(uint32_t baseaddr, uint16_t offset);

void write_hword_flash(uint32_t baseaddr, uint16_t offset, uint16_t data);

void write_word_flash(uint32_t baseaddr, uint16_t offset, uint32_t data);

void write_float_flash(uint32_t baseaddr, uint16_t offset, float data);

/*****************************************************************/
/*** Whole-page helpers (standard project)                     ***/
/*****************************************************************/
/* Erase the page at `baseaddr`, then program `n` 32-bit words from `data`
 * (n <= PGnn_SIZE / 4 = 512; flash_write_floats: n <= 16). Unlocks and locks the flash itself.
 * Returns 0 on success, -1 on a bad argument, -2 if a word read back
 * differs from what was written.
 *
 * FLASH can only turn 1s into 0s; erasing the page (all 0xFF) is the only
 * way back, so values are always written as a set. Page 31 (PG31_BASE)
 * is kept out of the program by the linker script (ROM = 62K), so it is
 * safe to use for calibration data.                                      */
int  flash_write_words(uint32_t baseaddr, const uint32_t *data, uint16_t n);
int  flash_write_floats(uint32_t baseaddr, const float *data, uint16_t n);

#endif /*! _FLASH_H_ */
