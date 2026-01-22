/**
 * \file	TFT.h
 * \author	Limor Fried 	Adafruit Industries
 * \author	Thomas Barth 	Hochschule Darmstadt - thomas.barth@h-da.de
 * \date 	08.08.2017
 * \version	0.1
 *
 * \brief TFT driver for the ST7735 (https://eckstein-shop.de/18-inch-128X160-SPI-Serial-TFT-LCD-Display-Modul-SD-Card), based on the Adafruit GFX library
 *
 *	Changelog:\n
 *	- 0.1 08.08.17 Barth
 *              - Initial documentation
 *
 * \copyright Copyright ©2017
 * Department of electrical engineering and information technology, Hochschule Darmstadt - University of applied sciences (h_da). All Rights Reserved.
 * Permission to use, copy, modify, and distribute this software and its documentation for educational, and research purposes in the context of non-commercial
 * (unless permitted by h_da) and official h_da projects, is hereby granted for enrolled students of h_da, provided that the above copyright notice,
 * this paragraph and the following paragraph appear in all copies, modifications, and distributions.
 * Contact Prof.Dr.-Ing. Peter Fromm, peter.fromm@h-da.de, Birkenweg 8 64295 Darmstadt - GERMANY for commercial requests.
 *
 * \warning This software is a PROTOTYPE version and is not designed or intended for use in production, especially not for safety-critical applications!
 * The user represents and warrants that it will NOT use or redistribute the Software for such purposes.
 * This prototype is for research purposes only. This software is provided "AS IS," without a warranty of any kind.
 */

#ifndef TFT_H
#define TFT_H

#include "global.h"
#include <stdio.h>                                          //sprintf for #TFT_printf


//-------------------------------------[Definitions]

#ifndef PROGMEM
    #define PROGMEM                                         /**< \brief The lib was made for an AVR. Instead of deleting this symbol everywhere, define PROGMEM empty.*/
#endif

#define CFG_TFT_ORIENTATION         TFT_rot_180  
#define CFG_TFT_RB_OUT_SZ           100                     /**< \brief TFT Ringbuffer OUT size for SPI communication*/
#define CFG_TFT_BACKLIGHT           100                     /**< \brief TFT backlight brightness after start (0-100). Can be adjusted with #TFT_setBacklight during runtime*/
#define CFG_TFT_SPRINTF_BUF_L       128                     /**< \brief Buffer length for #TFT_printf */   
#define CFG_TFT_COLOR_TEXT          WHITE                   /**< \brief Default text color */
#define CFG_TFT_COLOR_TEXTBG        BLACK                   /**< \brief Default text background color */

#define CFG_TFT_WIDTH               128                     /**< \brief TFT width, 128px for our CP11001*/
#define CFG_TFT_HEIGHT              160                     /**< \brief TFT height, 160px for our CP11001*/
#define CFG_TFT_SPI_MODULE          TFT_SPI                 /**< \brief Module Name of the SPI, which serves the TFT*/
#define CFG_TFT_LED_MODULE          TFT_BackLight           /**< \brief Module Name of the PWM, which serves the TFT Backlight*/
#define CFG_TFT_DC_PIN              TFT_DC                  /**< \brief Pin Name of the DC pin*/
#define CFG_TFT_RES_PIN             TFT_RES                 /**< \brief Pin Name of the RES pin*/
#define CFG_TFT_DELAY_MS(x)         CyDelay(x)              /**< \brief Macro to implement a delay function (only used in #TFT_init)*/

//-------------------------------------[Types]

typedef uint16_t TFT_color_t;

/** 
* \brief Data stored PER GLYPH
*/
typedef struct {                
	uint16_t bitmapOffset;                                  /**< \brief Pointer into GFXfont->bitmap*/
	uint8_t  width, height;                                 /**< \brief Bitmap dimensions in pixels*/
	uint8_t  xAdvance;                                      /**< \brief Distance to advance cursor (x axis)*/
	int8_t   xOffset, yOffset;                              /**< \brief Dist from cursor pos to UL corner*/
} GFXglyph;

/** 
* \brief Data stored for Font as a whole
*/
typedef struct { 
	uint8_t  *bitmap;                                       /**< \brief Glyph bitmaps, concatenated*/
	GFXglyph *glyph;                                        /**< \brief Glyph array*/
	uint8_t   first, last;                                  /**< \brief ASCII extents*/
	uint8_t   yAdvance;                                     /**< \brief Newline distance (y axis)*/
} GFXfont;

/** 
* \brief Screen rotation
*/
typedef enum{
    TFT_rot_0   =0,                                         /**< \brief Dont rotate, Portrait*/
    TFT_rot_90  =1,                                         /**< \brief Rotate by 90°, Landscape*/
    TFT_rot_180 =2,                                         /**< \brief Rotate by 180°, Portrait*/
    TFT_rot_270 =3,                                         /**< \brief Rotate by 270°, Landscape*/
}TFT_rotation_t;

/**
* \brief TFT Colors
* PF: BITORDER was wrong, had to be inverted
*/
typedef enum{
    WHITE         	 =0xFFFF, //ok
    BLACK         	 =0x0000, //ok	  
    BLUE         	 =0xF800, //0x001F, //red
    BRED             =0xF81F, //ok
    GRED 			 =0x07FF, //0xFFE0,
    GBLUE			 =0xFFE0, //0x07FF,
    RED           	 =0x001F, //0xF800, //blue
    MAGENTA       	 =0xF81F, //ok
    GREEN         	 =0x07E0,
    CYAN          	 =0xFFE0, //0x7FFF,
    YELLOW        	 =0x07FF, //0xFFE0,
    BROWN 			 =0x023D, //0xBC40,
    BRRED 			 =0xE03f, //0xFC07,
    GRAY  			 =0x0C21, //0x8430,
    DARKBLUE      	 =0xF389, //0x01CF,
    LIGHTBLUE      	 =0x3EBE, //0x7D7C,
    GRAYBLUE       	 =0x1A2A, //0x5458,
    LIGHTGREEN     	 =0xF821, //0x841F,
    LGRAY 			 =0x1863, //0xC618,
    LGRAYBLUE        =0x8A65, //0xA651,
    LBBLUE           =0x48D4, //0x2B12
}TFT_colors_t;


//-------------------------------------[Basic API]

/**
 * \brief   Initialize the SPI and PWM given in #TFT_SPI_MODULE and #TFT_LED_MODULE. 
 *          Sends the bootup sequence to the TFT, applies #TFT_ORIENTATION and sets the backlight to #TFT_BACKLIGHT
 * \return  none
 */
void TFT_init();

//-------------------------------------[Screen API]

/**
 * \brief   Sets the TFT Backlight brightness
 * \param   Backlight brightness in % (0-100)
 * \return  none
 */
void TFT_setBacklight(uint8_t b);

/**
 * \brief   Set the rotation of the screen
 * \param   see #TFT_rotation_t
 * \return  none
 */
void TFT_setRotation(TFT_rotation_t m);

/**
 * \brief   get the TFT heigth (per current rotation)
 * \return  TFT heigth
 */
int16_t TFT_height(void);

/**
 * \brief   get the TFT width (per current rotation)
 * \return  TFT width
 */
int16_t TFT_width(void);

/**
 * \brief   get the TFT backlight brigtness
 * \return  backlight brightness in % (0-100)
 */
uint8_t TFT_getBacklight();

/**
 * \brief   get the TFT orientation/rotation
 * \return  see #TFT_rotation_t
 */
TFT_rotation_t TFT_getRotation(void);

/**
 * \brief   Fills the screen with the current background color
 * \return  none
 */
void TFT_clearScreen(void);

/**
 * \brief   Fills the screen with a given color
 * \param   Fill color
 * \return  none
 */
void TFT_fillScreen(TFT_color_t color);

/**
 * \brief   Sets the foreground and background color, needed for #TFT_print, #TFT_write, #TFT_println, #TFT_printf etc.
 * \param   color
 * \param   background color
 * \return  none
 */
void TFT_setColors(TFT_color_t c, TFT_color_t bg);

/**
 * \brief   Gets the background color
 * \return  foreground color
 */
TFT_color_t TFT_getFGcolor(void);


/**
 * \brief   Gets the background color
 * \return  foreground color
 */
TFT_color_t TFT_getBGcolor(void);

//-------------------------------------[draw API]

/**
 * \brief   Draws a single pixel
 * \param   x position of the pixel
 * \param   y position of the pixel
 * \param   color of the pixel
 * \return  none
 */
void TFT_drawPixel(int16_t x, int16_t y, TFT_color_t color);

/**
 * \brief   Draws a filled rectangle
 * \param   x position of the rectangle
 * \param   y position of the rectangle
 * \param   width of the rectangle
 * \param   heigth of the rectangle
 * \param   color of the rectangle
 * \return  none
 */
void TFT_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, TFT_color_t color);

/**
 * \brief   Draws a rectangle
 * \param   x position of the rectangle
 * \param   y position of the rectangle
 * \param   width of the rectangle
 * \param   heigth of the rectangle
 * \param   color of the rectangle
 * \return  none
 */
void TFT_drawRect(int16_t x, int16_t y, int16_t w, int16_t h, TFT_color_t color);

/**
 * \brief   Draws a rectangle with round corners
 * \param   x position of the rectangle
 * \param   y position of the rectangle
 * \param   width of the rectangle
 * \param   heigth of the rectangle
 * \param   radius of the corners
 * \param   color of the rectangle
 * \return  none
 */
void TFT_drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, TFT_color_t color);

/**
 * \brief   Draws a filled rectangle with round corners
 * \param   x position of the rectangle
 * \param   y position of the rectangle
 * \param   width of the rectangle
 * \param   heigth of the rectangle
 * \param   radius of the corners
 * \param   color of the rectangle
 * \return  none
 */
void TFT_fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, TFT_color_t color);

/**
 * \brief   Draws a vertical line
 * \param   x position of the line
 * \param   y position of the line
 * \param   heigth of the line
 * \param   color of the line
 * \return  none
 */
void TFT_fastVLine(int16_t x, int16_t y, int16_t h, TFT_color_t color);

/**
 * \brief   Draws a horizontal line
 * \param   x position of the line
 * \param   y position of the line
 * \param   width of the line
 * \param   color of the line
 * \return  none
 */
void TFT_fastHLine(int16_t x, int16_t y, int16_t w, TFT_color_t color);

/**
 * \brief   Draws a line
 * \param   x position of the line start
 * \param   y position of the line start
 * \param   x position of the line end
 * \param   y position of the line end
 * \param   color of the line
 * \return  none
 */
void TFT_writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, TFT_color_t color);

/**
 * \brief   Inverts the display colors
 * \param   0 if no invertion, else invert
 * \return  none
 */
void TFT_invertDisplay(boolean_t i);

/**
 * \brief   Draws a circle
 * \param   x position of the circle center
 * \param   y position of the circle center
 * \param   radius
 * \param   color of the circle
 * \return  none
 */
void TFT_drawCircle(int16_t x0, int16_t y0, int16_t r, TFT_color_t color);

/**
 * \brief   Draws a filled circle
 * \param   x position of the circle center
 * \param   y position of the circle center
 * \param   radius
 * \param   color of the circle
 * \return  none
 */
void TFT_fillCircle(int16_t x0, int16_t y0, int16_t r, TFT_color_t color);

/**
 * \brief   Draws a triangle
 * \param   x position of the first point
 * \param   y position of the first point
 * \param   x position of the second point
 * \param   y position of the second point
 * \param   x position of the third point
 * \param   y position of the third point
 * \param   color of the triangle
 * \return  none
 */
void TFT_drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, TFT_color_t color);

/**
 * \brief   Draws a filled triangle
 * \param   x position of the first point
 * \param   y position of the first point
 * \param   x position of the second point
 * \param   y position of the second point
 * \param   x position of the third point
 * \param   y position of the third point
 * \param   color of the triangle
 * \return  none
 */
void TFT_fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, TFT_color_t color);

//-------------------------------------[text API]

/**
 * \brief   get current X cursor position 
 * \return  X cursor postition
 */
int16_t TFT_getCursorX(void);

/**
 * \brief   get current Y cursor position 
 * \return  Y cursor postition
 */
int16_t TFT_getCursorY(void);

/**
 * \brief   Draws a single character
 * \param   x position of the character
 * \param   y position of the character
 * \param   the character
 * \param   color of the character
 * \param   background color of the character
 * \param   character size
 * \return  none
 */
void TFT_drawChar(int16_t x, int16_t y, unsigned char c, TFT_color_t color, uint16_t bg, uint8_t size);

/**
 * \brief   Sets the text cursor to a position, needed for #TFT_print, #TFT_write, #TFT_println, #TFT_printf etc.
 * \param   x position of the cursor
 * \param   y position of the cursor
 * \return  none
 */
void TFT_setCursor(int16_t x, int16_t y);

/**
 * \brief   Sets the text color, needed for #TFT_print, #TFT_write, #TFT_println, #TFT_printf etc.
 * \param   color
 * \return  none
 */
void TFT_setTextColor(uint16_t c);

/**
 * \brief   Sets the text wrap(overflow at screen end), needed for #TFT_print, #TFT_write, #TFT_println, #TFT_printf etc.
 * \param   0 no text wrap, else wrap
 * \return  none
 */
void TFT_setTextWrap(boolean_t w);

/**
 * \brief   Sets the text size, needed for #TFT_print, #TFT_write, #TFT_println, #TFT_printf etc.
 * \param   size
 * \return  none
 */
void TFT_setTextSize(uint8_t s);

/**
 * \brief   Sets the font, which shall be used for all text functions.
 * \param   font
 * \return  none
 */
void TFT_setFont(const GFXfont *f);

/**
 * \brief   Calculates the size required by a string and the current configuration of size etc
 * \param   The string to be calculated
 * \param   Start X
 * \param   Start Y
 * \param   Calculated X End
 * \param   Calculated Y End
 * \param   Calculated width
 * \param   Calculated height
 * \return  none
 */
void TFT_getTextBounds(char *string, int16_t x, int16_t y,  int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);

/**
 * \brief   Write a single char with current position and settings
 * \param   char
 * \return  none
 */
void TFT_write(uint8_t);

/**
 * \brief   Write a string with current position and settings
 * \param   char
 * \return  none
 */
void TFT_print(char* txt);

/**
 * \brief   Write a string with current position and settings and add a newline at the end
 * \param   char
 * \return  none
 */
void TFT_println(char* txt); 

void TFT_printInt(uint16_t num);

/**
 * \brief   Write a string with current position usind sprintf format
 * \param   char
 * \return  none
 * \note    Keep the maximum string length defined in #TFT_SPRINTF_BUF_L in mind
 */
#define TFT_printf(...) {char tft_buf[CFG_TFT_SPRINTF_BUF_L]; sprintf(tft_buf,__VA_ARGS__); TFT_print(tft_buf);}

//TODO
/*
void
    TFT_drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color),
    TFT_drawBitmap_w_BG(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg),
    TFT_drawGrayscaleBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h),
    TFT_drawGrayscaleBitmap_masked(int16_t x, int16_t y, uint8_t *bitmap, uint8_t *mask, int16_t w, int16_t h),
    TFT_drawRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h),
    TFT_drawRGBBitmap_masked(int16_t x, int16_t y, uint16_t *bitmap, uint8_t *mask, int16_t w, int16_t h);
 */      
        
#endif

/* [] END OF FILE */