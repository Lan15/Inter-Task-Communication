/**
 * \file	TFT.c
 * \author	Limor Fried 	Adafruit Industries
 * \author	Thomas Barth 	Hochschule Darmstadt - thomas.barth@h-da.de
 * \date 	08.08.2017
 * \version	0.1
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

#include "tft.h"
#include "ST7735.h"
#include "glcdfont.h"
#include "math_helper.h"
#include "logging.h"

//-------------------------------------[Definitions]

/** 
* \brief API glue for the SPI module
*/
#define MODULE_SPI(x)       PPCAT(CFG_TFT_SPI_MODULE, x)

/** 
* \brief API glue for the backlight PWM module
*/
#define MODULE_PWM(x)       PPCAT(CFG_TFT_LED_MODULE, x)

// API glue for the Pins
#define MODULE_DC(x)        PPCAT(CFG_TFT_DC_PIN, x)      
#define MODULE_RES(x)       PPCAT(CFG_TFT_RES_PIN, x)  
#define TFT_DC_LOW          MODULE_DC(Write)(0)     /**< \brief Macro to set the DC(A0) pin LOW*/
#define TFT_DC_HIGH         MODULE_DC(Write)(1)     /**< \brief Macro to set the DC(A0) pin HIGH*/
#define TFT_RES_LOW         MODULE_RES(Write)(0)    /**< \brief Macro to set the Reset pin LOW*/
#define TFT_RES_HIGH        MODULE_RES(Write)(1)    /**< \brief Macro to set the Reset pin HIGH*/

/** 
* \brief swap 2 int 16 values
*/
#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif

// The code comes from an AVR. Those are macros to overcome that strange PROGMEM read stuff
#ifndef pgm_read_byte
 #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif
#ifndef pgm_read_word
 #define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif
#ifndef pgm_read_dword
 #define pgm_read_dword(addr) (*(const unsigned long *)(addr))
#endif

// Pointers are a peculiar case...typically 16-bit on AVR boards,
// 32 bits elsewhere.  Try to accommodate both...
#if !defined(__INT_MAX__) || (__INT_MAX__ > 0xFFFF)
 #define pgm_read_pointer(addr) ((void *)pgm_read_dword(addr))
#else
 #define pgm_read_pointer(addr) ((void *)pgm_read_word(addr))
#endif

#define TFT_write_cmd(x)            TFT_send(TFT_CMD,x)
#define TFT_write_data(x)           TFT_send(TFT_DATA,x)
#define TFT_write_stream(x)         TFT_send(TFT_STREAM,x)

//-------------------------------------[types]
/**
* \brief Data send to the TFT is either DATA or COMMAND data.
*/
typedef enum{
    TFT_CMD,                        /**< Payload is command. */
    TFT_DATA,                       /**< Payload is data */
    TFT_STREAM                      /**< D/C has already been set */
}TFT_payload_t;

/**
* \brief Outgoing data to the tft
*/
typedef struct{
    TFT_payload_t   DC;             /**< Determines if this is a a command (#TFT_CMD) or data (#TFT_DATA) */
    uint8_t         payload;
}TFT_data_t;

typedef struct {
    uint16_t    read;
    uint16_t    write;
    uint16_t    fill;
    TFT_data_t  buffer[CFG_TFT_RB_OUT_SZ];
}TFT_RB_out_t;


//-------------------------------------[local variables]

static const char* TAG="TFT";

static uint8_t  colstart;           /**< \brief Column (RAM) start*/
static uint8_t  rowstart;           /**< \brief Row (RAM) start*/
static uint8_t  xstart;             /**< \brief x position start*/
static uint8_t  ystart;             /**< \brief y position start*/

static uint8_t  textsize;           /**< \brief current text size*/
static uint8_t  rotation;           /**< \brief current orientation/rotation*/

static int16_t  _width;             /**< \brief Display width as modified by current rotation*/
static uint16_t _height;            /**< \brief Display heigth as modified by current rotation*/ 
static uint16_t cursor_x;           /**< \brief Cursor X position*/
static uint16_t cursor_y;           /**< \brief Cursor Y position*/
static TFT_color_t color;              /**< \brief Current foreground color*/
static TFT_color_t bgcolor;            /**< \brief Current background color*/

static boolean_t wrap;              /**< \brief If set, 'wrap' text at right edge of display*/ 
    
static GFXfont *gfxFont;            /**< \brief Current Font*/ 

static TFT_RB_out_t TFT_RB_out;     /**< \brief Ringbuffer for SPI communication*/
    
//-------------------------------------[local function prototypes]

static void setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
static void charBounds(char c, int16_t *x, int16_t *y,  int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy);
static void TFT_drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, TFT_color_t color);
static void TFT_fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, TFT_color_t color);

static RC_t TFT_send(TFT_payload_t type, uint8_t payload);
//-------------------------------------[Implementation]

//----------------- Ringbuffer

void __TFT_SPI_TX_ISR(void){
    
    //check if we have data waiting
    if(TFT_RB_out.fill==0)
        return;
    
    //get the next frame to be send
    TFT_data_t* p_next_frame=&TFT_RB_out.buffer[TFT_RB_out.read++];
    
    //adjust read index and fill level
    TFT_RB_out.read%=CFG_TFT_RB_OUT_SZ;
    TFT_RB_out.fill--;
    
    //check payload type
    switch(p_next_frame->DC){
        case TFT_CMD:
            TFT_DC_LOW;
            break;
        
        case TFT_DATA:
            TFT_DC_HIGH;
            break;
        default:
        break;
    }

    MODULE_SPI(WriteTxData)(p_next_frame->payload);    
}

RC_t TFT_send(TFT_payload_t type, uint8_t payload){

    //check if buffer is full
    if(TFT_RB_out.fill>=CFG_TFT_RB_OUT_SZ){
        //TODO: fire event or DET STOP
        LOG_E(TAG,"Buffer overflow");
        return RC_ERROR_BUFFER_FULL;
    }
    
    //add frame to buffer
    TFT_RB_out.buffer[TFT_RB_out.write++]=(TFT_data_t){type,payload};

    //adjust write index and fill level
    TFT_RB_out.write%=CFG_TFT_RB_OUT_SZ;
    TFT_RB_out.fill++;
    
    //check if this is the only entry in the ringbuffer and trigger the ISR
    if(TFT_RB_out.fill==1)
        __TFT_SPI_TX_ISR();

    return RC_SUCCESS;
}


//----------------- CTRL
void TFT_init(){
    
    //init local vars
    _width    = CFG_TFT_WIDTH;
    _height   = CFG_TFT_HEIGHT;
    rotation=0;
    cursor_x=0;
    cursor_y=0;
    textsize=1;
    color=CFG_TFT_COLOR_TEXT;
    bgcolor=CFG_TFT_COLOR_TEXTBG;
    wrap=TRUE;
    gfxFont=NULL;
    ystart = xstart = colstart  = rowstart = 0;
    
    //init ringbuffer
    TFT_RB_out.read=0;
    TFT_RB_out.write=0;
    TFT_RB_out.fill=0;
    
    //Start SPI Interface
    MODULE_SPI(Start)();
    
    //Perform Hardware reset
    TFT_RES_HIGH;
    CFG_TFT_DELAY_MS(100);
    TFT_RES_LOW;
	CFG_TFT_DELAY_MS(100);
	TFT_RES_HIGH;
	CFG_TFT_DELAY_MS(20);
    
    //Software reset
    TFT_write_cmd(ST7735_SWRESET);
    CFG_TFT_DELAY_MS(50);
    
    //sleep out
    TFT_write_cmd(ST7735_SLPOUT);    //Sleep out
	CFG_TFT_DELAY_MS(120);              //Delay 120ms
    
    //send init commands
    uint8_t* addr=(uint8_t*)SST35S_init_seq;    
    uint8_t  numCommands, numArgs;
    numCommands = *(addr++);        // Number of commands to follow
    while(numCommands--) {          // For each command...
        TFT_write_cmd(*(addr++));    // Read, issue command
        numArgs  = *(addr++);       // Number of args to follow
        while(numArgs--) {          // For each argument...
          TFT_write_data(*(addr++));     // Read, issue argument
        }
    }
    
    //set roation
    TFT_setRotation(CFG_TFT_ORIENTATION);
    
    //init RAM
    TFT_fillScreen(BLACK);
    
    //activate backlight
    MODULE_PWM(Start)();
    MODULE_PWM(WriteCompare)(CFG_TFT_BACKLIGHT);
}

void TFT_setBacklight(uint8_t b){
    if(b>100)
        b=100;
    MODULE_PWM(WriteCompare)(b);
}

uint8_t TFT_getBacklight(){
    return MODULE_PWM(ReadCompare)();
};

void setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {

    TFT_write_cmd(ST7735_CASET);   // Column addr set
    TFT_write_data(0x00);
    TFT_write_data(x0+xstart);         // XSTART 
    TFT_write_data(0x00);
    TFT_write_data(x1+xstart);         // XEND

    TFT_write_cmd(ST7735_RASET);   // Row addr set
    TFT_write_data(0x00);
    TFT_write_data(y0+ystart);         // YSTART
    TFT_write_data(0x00);
    TFT_write_data(y1+ystart);         // YEND

    TFT_write_cmd(ST7735_RAMWR);   // write to RAM
}

void TFT_setRotation(TFT_rotation_t m) {

    //Memory data access control entry
    TFT_write_cmd(ST7735_MADCTL);
    rotation = m % 4; // can't be higher than 3
    switch (rotation) {
        case 0:
            TFT_write_data(ST7735_MADCTL_MX | ST7735_MADCTL_MY | ST7735_MADCTL_RGB);
            _height = CFG_TFT_HEIGHT;
            _width  = CFG_TFT_WIDTH;
            xstart = colstart;
            ystart = rowstart;
            break;
        case 1:
            TFT_write_data(ST7735_MADCTL_MY | ST7735_MADCTL_MV | ST7735_MADCTL_RGB);
            _width = CFG_TFT_HEIGHT;
            _height = CFG_TFT_WIDTH;
            ystart = colstart;
            xstart = rowstart;
            break;
        case 2:
            TFT_write_data(ST7735_MADCTL_BGR);
            _height = CFG_TFT_HEIGHT;
            _width  = CFG_TFT_WIDTH;
            xstart = colstart;
            ystart = rowstart;
            break;
        case 3:
            TFT_write_data(ST7735_MADCTL_MX | ST7735_MADCTL_MV | ST7735_MADCTL_RGB);
            _width = CFG_TFT_HEIGHT;
            _height = CFG_TFT_WIDTH;
            ystart = colstart;
            xstart = rowstart;
            break;
    }
}

//----------------- USR CTRL

void TFT_setCursor(int16_t x, int16_t y) {
    cursor_x = x;
    cursor_y = y;
}

void TFT_setTextColor(uint16_t c){
    color   = c;
}

void TFT_setcolor(uint16_t c) {
    // For 'transparent' background, we'll set the bg
    // to the same as fg instead of using a flag
    color = bgcolor = c;
}

void TFT_setColors(uint16_t c, uint16_t b) {
    color   = c;
    bgcolor = b;
}


TFT_color_t TFT_getFGcolor(void){return color;};

TFT_color_t TFT_getBGcolor(void){return bgcolor;};


void TFT_setTextWrap(boolean_t w) {
    wrap = w;
}

void TFT_setTextSize(uint8_t s) {
    textsize = (s > 0) ? s : 1;
}

void TFT_setFont(const GFXfont *f) {
    if(f) {            // Font struct pointer passed in?
        if(!gfxFont) { // And no current font struct?
            // Switching from classic to new font behavior.
            // Move cursor pos down 6 pixels so it's on baseline.
            cursor_y += 6;
        }
    } else if(gfxFont) { // NULL passed.  Current font struct defined?
        // Switching from new to classic font behavior.
        // Move cursor pos up 6 pixels so it's at top-left of char.
        cursor_y -= 6;
    }
    gfxFont = (GFXfont *)f;
}

// Pass string and a cursor position, returns UL corner and W,H.
void TFT_getTextBounds(char *str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
    uint8_t c; // Current character

    *x1 = x;
    *y1 = y;
    *w  = *h = 0;

    int16_t minx = _width, miny = _height, maxx = -1, maxy = -1;

    while((c = *str++))
        charBounds(c, &x, &y, &minx, &miny, &maxx, &maxy);

    if(maxx >= minx) {
        *x1 = minx;
        *w  = maxx - minx + 1;
    }
    if(maxy >= miny) {
        *y1 = miny;
        *h  = maxy - miny + 1;
    }
}

void charBounds(char c, int16_t *x, int16_t *y,  int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy) {

    if(gfxFont) {

        if(c == '\n') { // Newline?
            *x  = 0;    // Reset x to zero, advance y by one line
            *y += textsize * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
        } else if(c != '\r') { // Not a carriage return; is normal char
            uint8_t first = pgm_read_byte(&gfxFont->first),
                    last  = pgm_read_byte(&gfxFont->last);
            if((c >= first) && (c <= last)) { // Char present in this font?
                GFXglyph *glyph = &(((GFXglyph *)pgm_read_pointer(
                  &gfxFont->glyph))[c - first]);
                uint8_t gw = pgm_read_byte(&glyph->width),
                        gh = pgm_read_byte(&glyph->height),
                        xa = pgm_read_byte(&glyph->xAdvance);
                int8_t  xo = pgm_read_byte(&glyph->xOffset),
                        yo = pgm_read_byte(&glyph->yOffset);
                if(wrap && ((*x+(((int16_t)xo+gw)*textsize)) > _width)) {
                    *x  = 0; // Reset x to zero, advance y by one line
                    *y += textsize * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
                }
                int16_t ts = (int16_t)textsize,
                        x1 = *x + xo * ts,
                        y1 = *y + yo * ts,
                        x2 = x1 + gw * ts - 1,
                        y2 = y1 + gh * ts - 1;
                if(x1 < *minx) *minx = x1;
                if(y1 < *miny) *miny = y1;
                if(x2 > *maxx) *maxx = x2;
                if(y2 > *maxy) *maxy = y2;
                *x += xa * ts;
            }
        }

    } else { // Default font

        if(c == '\n') {                     // Newline?
            *x  = 0;                        // Reset x to zero,
            *y += textsize * 8;             // advance y one line
            // min/max x/y unchaged -- that waits for next 'normal' character
        } else if(c != '\r') {  // Normal char; ignore carriage returns
            if(wrap && ((*x + textsize * 6) > _width)) { // Off right?
                *x  = 0;                    // Reset x to zero,
                *y += textsize * 8;         // advance y one line
            }
            int x2 = *x + textsize * 6 - 1, // Lower-right pixel of char
                y2 = *y + textsize * 8 - 1;
            if(x2 > *maxx) *maxx = x2;      // Track max x, y
            if(y2 > *maxy) *maxy = y2;
            if(*x < *minx) *minx = *x;      // Track min x, y
            if(*y < *miny) *miny = *y;
            *x += textsize * 6;             // Advance x one char
        }
    }
}
        
//----------------- USER FCT

void TFT_print(char* txt){
    while(*(txt)!=0){
        TFT_write(*(txt++));
    }
}

void TFT_println(char* txt){
    while(*(txt)!=0){
        TFT_write(*(txt++));
    }
    TFT_write('\n');
}

void TFT_printInt(uint16_t num) {
    if (num == 0) {
        TFT_print("0");
        return;
    }
    char buf[6];  // Max 65535 + null
    int idx = 0;
    while (num > 0) {
        buf[idx++] = '0' + (num % 10);
        num /= 10;
    }
    buf[idx] = '\0';
    // Reverse string
    for (int i = 0; i < idx / 2; i++) {
        char temp = buf[i];
        buf[i] = buf[idx - 1 - i];
        buf[idx - 1 - i] = temp;
    }
    TFT_println(buf);
}

void TFT_write(uint8_t c) {

    if(!gfxFont) {                              // 'Classic' built-in font

        if(c == '\n') {                         // Newline?
            cursor_x  = 0;                      // Reset x to zero,
            cursor_y += textsize * 8;           // advance y one line
        } else if(c != '\r') {                  // Ignore carriage returns
            if(wrap && ((cursor_x + textsize * 6) > _width)) { // Off right?
                cursor_x  = 0;                  // Reset x to zero,
                cursor_y += textsize * 8;       // advance y one line
            }
            TFT_drawChar(cursor_x, cursor_y, c, color, bgcolor, textsize);
            cursor_x += textsize * 6;          // Advance x one char
        }

    } else { // Custom font

        if(c == '\n') {
            cursor_x  = 0;
            cursor_y += (int16_t)textsize *
                        (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
        } else if(c != '\r') {
            uint8_t first = pgm_read_byte(&gfxFont->first);
            if((c >= first) && (c <= (uint8_t)pgm_read_byte(&gfxFont->last))) {
                GFXglyph *glyph = &(((GFXglyph *)pgm_read_pointer(
                  &gfxFont->glyph))[c - first]);
                uint8_t   w     = pgm_read_byte(&glyph->width),
                          h     = pgm_read_byte(&glyph->height);
                if((w > 0) && (h > 0)) { // Is there an associated bitmap?
                    int16_t xo = (int8_t)pgm_read_byte(&glyph->xOffset); // sic
                    if(wrap && ((cursor_x + textsize * (xo + w)) > _width)) {
                        cursor_x  = 0;
                        cursor_y += (int16_t)textsize *
                          (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
                    }
                    TFT_drawChar(cursor_x, cursor_y, c, color, bgcolor, textsize);
                }
                cursor_x += (uint8_t)pgm_read_byte(&glyph->xAdvance) * (int16_t)textsize;
            }
        }

    }
}

void TFT_fillScreen(TFT_color_t color) {
  TFT_fillRect(0, 0,  _width, _height, color);
}

void TFT_clearScreen(void){TFT_fillScreen(TFT_getBGcolor());};

void TFT_fillRect(int16_t x, int16_t y, int16_t w, int16_t h, TFT_color_t color) {
    
    // rudimentary clipping (drawChar w/big text requires this)
    if((x >= _width) || (y >= _height)) return;
    
    if((x + w - 1) >= _width)  w = _width  - x;
    if((y + h - 1) >= _height) h = _height - y;
    
    setAddrWindow(x, y, x+w-1, y+h-1);

    uint8_t hi = color >> 8, lo = color;
    
    TFT_DC_HIGH;
    for(y=h; y>0; y--) {
        for(x=w; x>0; x--) {
          TFT_write_stream(hi);
          TFT_write_stream(lo);
        }
    }
}

void TFT_drawPixel(int16_t x, int16_t y, TFT_color_t color) {

    if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;
    setAddrWindow(x,y,x+1,y+1);

    TFT_DC_HIGH;
    TFT_write_stream(color >> 8);
    TFT_write_stream(color);

}

void TFT_fastVLine(int16_t x, int16_t y, int16_t h, TFT_color_t color) {

  // Rudimentary clipping
  if((x >= _width) || (y >= _height)) return;
  if((y+h-1) >= _height) h = _height-y;
  setAddrWindow(x, y, x, y+h-1);

  uint8_t hi = color >> 8, lo = color;
    
  TFT_DC_HIGH;
  while (h--) {
    TFT_write_stream(hi);
    TFT_write_stream(lo);
  }
}

void TFT_fastHLine(int16_t x, int16_t y, int16_t w,  TFT_color_t color) {

    // Rudimentary clipping
    if((x >= _width) || (y >= _height)) return;
    if((x+w-1) >= _width)  w = _width-x;
    setAddrWindow(x, y, x+w-1, y);

    uint8_t hi = color >> 8, lo = color;

    TFT_DC_HIGH;

    while (w--) {
        TFT_write_stream(hi);
        TFT_write_stream(lo);
    }
}

// Bresenham's algorithm - thx wikpedia
void TFT_writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,  TFT_color_t color) {
    int16_t steep = _abs(y1 - y0) > _abs(x1 - x0);
    if (steep) {
        _swap_int16_t(x0, y0);
        _swap_int16_t(x1, y1);
    }

    if (x0 > x1) {
        _swap_int16_t(x0, x1);
        _swap_int16_t(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = _abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0<=x1; x0++) {
        if (steep) {
            TFT_drawPixel(y0, x0, color);
        } else {
            TFT_drawPixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

void TFT_invertDisplay(boolean_t i) {
  TFT_write_cmd(i ? ST7735_INVON : ST7735_INVOFF);
}

// Draw a circle outline
void TFT_drawCircle(int16_t x0, int16_t y0, int16_t r, TFT_color_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    TFT_drawPixel(x0  , y0+r, color);
    TFT_drawPixel(x0  , y0-r, color);
    TFT_drawPixel(x0+r, y0  , color);
    TFT_drawPixel(x0-r, y0  , color);

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        TFT_drawPixel(x0 + x, y0 + y, color);
        TFT_drawPixel(x0 - x, y0 + y, color);
        TFT_drawPixel(x0 + x, y0 - y, color);
        TFT_drawPixel(x0 - x, y0 - y, color);
        TFT_drawPixel(x0 + y, y0 + x, color);
        TFT_drawPixel(x0 - y, y0 + x, color);
        TFT_drawPixel(x0 + y, y0 - x, color);
        TFT_drawPixel(x0 - y, y0 - x, color);
    }

}

void TFT_drawCircleHelper( int16_t x0, int16_t y0, int16_t r, uint8_t cornername, TFT_color_t color) {
    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;
        if (cornername & 0x4) {
            TFT_drawPixel(x0 + x, y0 + y, color);
            TFT_drawPixel(x0 + y, y0 + x, color);
        }
        if (cornername & 0x2) {
            TFT_drawPixel(x0 + x, y0 - y, color);
            TFT_drawPixel(x0 + y, y0 - x, color);
        }
        if (cornername & 0x8) {
            TFT_drawPixel(x0 - y, y0 + x, color);
            TFT_drawPixel(x0 - x, y0 + y, color);
        }
        if (cornername & 0x1) {
            TFT_drawPixel(x0 - y, y0 - x, color);
            TFT_drawPixel(x0 - x, y0 - y, color);
        }
    }
}

void TFT_fillCircle(int16_t x0, int16_t y0, int16_t r, TFT_color_t color) {

    TFT_fastVLine(x0, y0-r, 2*r+1, color);
    TFT_fillCircleHelper(x0, y0, r, 3, 0, color);
}

// Used to do circles and roundrects
void TFT_fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, TFT_color_t color) {

    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;

        if (cornername & 0x1) {
            TFT_fastVLine(x0+x, y0-y, 2*y+1+delta, color);
            TFT_fastVLine(x0+y, y0-x, 2*x+1+delta, color);
        }
        if (cornername & 0x2) {
            TFT_fastVLine(x0-x, y0-y, 2*y+1+delta, color);
            TFT_fastVLine(x0-y, y0-x, 2*x+1+delta, color);
        }
    }
}

// Draw a triangle
void TFT_drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, TFT_color_t color) {
    TFT_writeLine(x0, y0, x1, y1, color);
    TFT_writeLine(x1, y1, x2, y2, color);
    TFT_writeLine(x2, y2, x0, y0, color);
}

// Fill a triangle
void TFT_fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, TFT_color_t color) {

    int16_t a, b, y, last;

    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1) {
        _swap_int16_t(y0, y1); _swap_int16_t(x0, x1);
    }
    if (y1 > y2) {
        _swap_int16_t(y2, y1); _swap_int16_t(x2, x1);
    }
    if (y0 > y1) {
        _swap_int16_t(y0, y1); _swap_int16_t(x0, x1);
    }

    if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if(x1 < a)      a = x1;
        else if(x1 > b) b = x1;
        if(x2 < a)      a = x2;
        else if(x2 > b) b = x2;
        TFT_fastHLine(a, y0, b-a+1, color);
        return;
    }

    int16_t
    dx01 = x1 - x0,
    dy01 = y1 - y0,
    dx02 = x2 - x0,
    dy02 = y2 - y0,
    dx12 = x2 - x1,
    dy12 = y2 - y1;
    int32_t
    sa   = 0,
    sb   = 0;

    // For upper part of triangle, find scanline crossings for segments
    // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
    // is included here (and second loop will be skipped, avoiding a /0
    // error there), otherwise scanline y1 is skipped here and handled
    // in the second loop...which also avoids a /0 error here if y0=y1
    // (flat-topped triangle).
    if(y1 == y2) last = y1;   // Include y1 scanline
    else         last = y1-1; // Skip it

    for(y=y0; y<=last; y++) {
        a   = x0 + sa / dy01;
        b   = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        /* longhand:
        a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if(a > b) _swap_int16_t(a,b);
        TFT_fastHLine(a, y, b-a+1, color);
    }

    // For lower part of triangle, find scanline crossings for segments
    // 0-2 and 1-2.  This loop is skipped if y1=y2.
    sa = dx12 * (y - y1);
    sb = dx02 * (y - y0);
    for(; y<=y2; y++) {
        a   = x1 + sa / dy12;
        b   = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        /* longhand:
        a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if(a > b) _swap_int16_t(a,b);
        TFT_fastHLine(a, y, b-a+1, color);
    }
}

// Draw a rounded rectangle
void TFT_drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, TFT_color_t color) {
    // smarter version
    TFT_fastHLine(x+r  , y    , w-2*r, color); // Top
    TFT_fastHLine(x+r  , y+h-1, w-2*r, color); // Bottom
    TFT_fastVLine(x    , y+r  , h-2*r, color); // Left
    TFT_fastVLine(x+w-1, y+r  , h-2*r, color); // Right
    // draw four corners
    TFT_drawCircleHelper(x+r    , y+r    , r, 1, color);
    TFT_drawCircleHelper(x+w-r-1, y+r    , r, 2, color);
    TFT_drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
    TFT_drawCircleHelper(x+r    , y+h-r-1, r, 8, color);

}

// Fill a rounded rectangle
void TFT_fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, TFT_color_t color) {
    // smarter version

    TFT_fillRect(x+r, y, w-2*r, h, color);

    // draw four corners
    TFT_fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
    TFT_fillCircleHelper(x+r    , y+r, r, 2, h-2*r-1, color);

}

// Draw a rectangle
void TFT_drawRect(int16_t x, int16_t y, int16_t w, int16_t h,  TFT_color_t color) {
    TFT_fastHLine(x, y, w, color);
    TFT_fastHLine(x, y+h-1, w, color);
    TFT_fastVLine(x, y, h, color);
    TFT_fastVLine(x+w-1, y, h, color);
}

// Draw a character
void TFT_drawChar(int16_t x, int16_t y, unsigned char c,  TFT_color_t color, TFT_color_t bg, uint8_t size) {

    if(!gfxFont) { // 'Classic' built-in font

        if((x >= _width)            || // Clip right
           (y >= _height)           || // Clip bottom
           ((x + 6 * size - 1) < 0) || // Clip left
           ((y + 8 * size - 1) < 0))   // Clip top
            return;

        for(int8_t i=0; i<5; i++ ) { // Char bitmap = 5 columns
            uint8_t line = pgm_read_byte(&font[c * 5 + i]);
            for(int8_t j=0; j<8; j++, line >>= 1) {
                if(line & 1) {
                    if(size == 1)
                        TFT_drawPixel(x+i, y+j, color);
                    else
                        TFT_fillRect(x+i*size, y+j*size, size, size, color);
                } else if(bg != color) {
                    if(size == 1)
                        TFT_drawPixel(x+i, y+j, bg);
                    else
                        TFT_fillRect(x+i*size, y+j*size, size, size, bg);
                }
            }
        }
        if(bg != color) { // If opaque, draw vertical line for last column
            if(size == 1) TFT_fastVLine(x+5, y, 8, bg);
            else          TFT_fillRect(x+5*size, y, size, 8*size, bg);
        }

    } else { // Custom font

        // Character is assumed previously filtered by write() to eliminate
        // newlines, returns, non-printable characters, etc.  Calling
        // drawChar() directly with 'bad' characters of font may cause mayhem!

        c -= (uint8_t)pgm_read_byte(&gfxFont->first);
        GFXglyph *glyph  = &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[c]);
        uint8_t  *bitmap = (uint8_t *)pgm_read_pointer(&gfxFont->bitmap);

        uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
        uint8_t  w  = pgm_read_byte(&glyph->width),
                 h  = pgm_read_byte(&glyph->height);
        int8_t   xo = pgm_read_byte(&glyph->xOffset),
                 yo = pgm_read_byte(&glyph->yOffset);
        uint8_t  xx, yy, bits = 0, bit = 0;
        int16_t  xo16 = 0, yo16 = 0;

        if(size > 1) {
            xo16 = xo;
            yo16 = yo;
        }

        // Todo: Add character clipping here

        // NOTE: THERE IS NO 'BACKGROUND' COLOR OPTION ON CUSTOM FONTS.
        // THIS IS ON PURPOSE AND BY DESIGN.  The background color feature
        // has typically been used with the 'classic' font to overwrite old
        // screen contents with new data.  This ONLY works because the
        // characters are a uniform size; it's not a sensible thing to do with
        // proportionally-spaced fonts with glyphs of varying sizes (and that
        // may overlap).  To replace previously-drawn text when using a custom
        // font, use the getTextBounds() function to determine the smallest
        // rectangle encompassing a string, erase the area with fillRect(),
        // then draw new text.  This WILL infortunately 'blink' the text, but
        // is unavoidable.  Drawing 'background' pixels will NOT fix this,
        // only creates a new set of problems.  Have an idea to work around
        // this (a canvas object type for MCUs that can afford the RAM and
        // displays supporting setAddrWindow() and pushColors()), but haven't
        // implemented this yet.
        for(yy=0; yy<h; yy++) {
            for(xx=0; xx<w; xx++) {
                if(!(bit++ & 7)) {
                    bits = pgm_read_byte(&bitmap[bo++]);
                }
                if(bits & 0x80) {
                    if(size == 1) {
                        TFT_drawPixel(x+xo+xx, y+yo+yy, color);
                    } else {
                        TFT_fillRect(x+(xo16+xx)*size, y+(yo16+yy)*size,
                          size, size, color);
                    }
                }
                bits <<= 1;
            }
        }

    } // End classic vs custom font
}

int16_t TFT_width(void){
    return _width;
}

int16_t TFT_height(void) {
    return _height;
}

TFT_rotation_t TFT_getRotation(void){
    return rotation;
}

int16_t TFT_getCursorX(void){
    return cursor_x;
}

int16_t TFT_getCursorY(void) {
    return cursor_y;
}

/* [] END OF FILE */
