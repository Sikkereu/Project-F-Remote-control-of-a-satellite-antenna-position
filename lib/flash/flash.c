#include "flash.h"
#include <string.h>   /* memcpy() */

/*****************************/
/*** STM32 FLASH Helper Functions ***/
/*****************************/

void init_page_flash(uint32_t baseaddr){
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPERR);
    FLASH_ErasePage(baseaddr);
    FLASH_Lock();
}

uint16_t read_hword_flash(uint32_t baseaddr, uint16_t offset){
    return *(uint16_t *)(baseaddr + offset * 2);
}

uint32_t read_word_flash(uint32_t baseaddr, uint16_t offset){
    return *(uint32_t *)(baseaddr + offset * 4);
}

float read_float_flash(uint32_t baseaddr, uint16_t offset){
    return *(float *)(baseaddr + offset * 4);
}

void write_hword_flash(uint32_t baseaddr, uint16_t offset, uint16_t data){
    //FLASH_Unlock();
    FLASH_ProgramHalfWord(baseaddr + offset * 2, data);
    //FLASH_Lock();
}

void write_word_flash(uint32_t baseaddr, uint16_t offset, uint32_t data){
    //FLASH_Unlock();
    FLASH_ProgramWord(baseaddr + offset * 4, data);
    //FLASH_Lock();
}

void write_float_flash(uint32_t baseaddr, uint16_t offset, float data)
{
    //FLASH_Unlock();
    uint32_t word;
    memcpy(&word, &data, sizeof word);   // standard project: was *((uint32_t*)(&data)), a strict-aliasing violation
    FLASH_ProgramWord(baseaddr + offset * 4, word);
    //FLASH_Lock();
}

/*****************************************************************/
/*** Whole-page helpers (standard project)                     ***/
/*****************************************************************/

int flash_write_words(uint32_t baseaddr, const uint32_t *data, uint16_t n){
    uint16_t i;

    if (data == 0 || n > PGnn_SIZE / 4u || (baseaddr % PGnn_SIZE) != 0u){
        return -1;
    }

    init_page_flash(baseaddr);                  /* unlock, erase, lock */

    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
    for (i = 0; i < n; i++){
        FLASH_ProgramWord(baseaddr + i * 4u, data[i]);
    }
    FLASH_Lock();

    for (i = 0; i < n; i++){                    /* verify */
        if (read_word_flash(baseaddr, i) != data[i]){
            return -2;
        }
    }
    return 0;
}

int flash_write_floats(uint32_t baseaddr, const float *data, uint16_t n){
    uint32_t words[16];
    uint16_t i;

    /* memcpy instead of a pointer cast: reinterpreting a float as a
     * uint32_t through a cast breaks C's strict aliasing rule. */
    if (data == 0 || n > sizeof words / sizeof words[0]){
        return -1;                              /* up to 16 floats */
    }
    for (i = 0; i < n; i++){
        memcpy(&words[i], &data[i], sizeof words[i]);
    }
    return flash_write_words(baseaddr, words, n);
}
