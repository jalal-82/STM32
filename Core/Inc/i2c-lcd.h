#include "stm32f4xx_hal.h"

void lcd_init (uint16_t addr_7);   // initialize lcd

void lcd_send_cmd (char cmd, int i2c_frame_size);  // send command to the lcd

void lcd_send_data (char data);  // send data to the lcd

void lcd_send_string (char *str);  // send string to the lcd


void lcd_clear (void);

// LCD commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// LCD line addresses
#define LCD_LINE1 0x80 // First line address
#define LCD_LINE2 0xC0 // Second line address
