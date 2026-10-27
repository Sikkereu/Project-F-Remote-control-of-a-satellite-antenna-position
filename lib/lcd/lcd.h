#ifndef _LCD_H_
#define _LCD_H_

#include "stm32f30x_conf.h"

#define CHAR_WIDTH 6
#define LCD_BUFF_SIZE 512
#define LCD_LINE_BUFF_SIZE 256
#define LCD_LINE_SIZE 128

/*****************************/
/*** LCD Control Functions ***/
/*****************************/
void init_spi_lcd();
void lcd_transmit_byte(uint8_t data);
void lcd_push_buffer(uint8_t* buffer);
void lcd_reset();
void generate_line_buff(uint8_t * str, uint8_t * linebuff, uint16_t bufflen);
void write_line_buff(uint8_t * linebuff, uint8_t * lcdbuff, uint8_t xoffset, uint8_t yoffset, uint8_t scrollena);
void lcd_write_string(uint8_t * str, uint8_t * lcdBuff, uint8_t xoffset, uint8_t yoffset);

/*****************************************************************/
/*** Text API with a built-in framebuffer (standard project)   ***/
/*****************************************************************/
/* x in pixels (0..127), line 0..3 (8 pixels each). 21 characters per line.
 * Nothing reaches the display until lcd_update().                         */
void     lcd_init(void);                 // init_spi_lcd() + blank screen
void     lcd_clear(void);
void     lcd_update(void);               // push the buffer, about 512 SPI bytes
void     lcd_print(uint8_t x, uint8_t line, const char *str);
void     lcd_printf(uint8_t x, uint8_t line, const char *fmt, ...);
void     lcd_set_pixel(uint8_t x, uint8_t y, uint8_t on);   // y 0..31
uint8_t *lcd_buffer(void);               // raw 512-byte buffer for drawing


#endif /*! _LCD_H_ */
