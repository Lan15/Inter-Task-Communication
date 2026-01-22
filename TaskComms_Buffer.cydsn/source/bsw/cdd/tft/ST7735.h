/**
 * \file	ST7735.h
 * \author	Thomas Barth 	Hochschule Darmstadt - thomas.barth@h-da.de
 * \date 	08.08.2017
 * \version	0.1
 *
 * \brief Low level information for the ST7735 (https://eckstein-shop.de/18-inch-128X160-SPI-Serial-TFT-LCD-Display-Modul-SD-Card)
 *
 *	Changelog:\n
 *	- 0.1 08.08.17 Barth
 *              - Initial documentation
 *
 * \copyright Copyright ©2016
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
#include "global.h"

/**
* \brief ST7735 Commands
*/
typedef enum{
    ST7735_NOP=0x00,
    ST7735_SWRESET=0x01,
    ST7735_RDDID   =0x04,
    ST7735_RDDST   =0x09, 
    ST7735_SLPIN   =0x10,
    ST7735_SLPOUT  =0x11,
    ST7735_PTLON   =0x12,
    ST7735_NORON   =0x13,
    ST7735_INVOFF  =0x20,
    ST7735_INVON   =0x21,
    ST7735_DISPOFF =0x28,
    ST7735_DISPON  =0x29,
    ST7735_CASET   =0x2A,
    ST7735_RASET   =0x2B,
    ST7735_RAMWR   =0x2C,
    ST7735_RAMRD   =0x2E,
    ST7735_PTLAR   =0x30,
    ST7735_COLMOD  =0x3A,
    ST7735_MADCTL  =0x36,
    ST7735_FRMCTR1 =0xB1,
    ST7735_FRMCTR2 =0xB2,
    ST7735_FRMCTR3 =0xB3,
    ST7735_INVCTR  =0xB4,
    ST7735_DISSET5 =0xB6,
    ST7735_PWCTR1  =0xC0,
    ST7735_PWCTR2  =0xC1,
    ST7735_PWCTR3  =0xC2,
    ST7735_PWCTR4  =0xC3,
    ST7735_PWCTR5  =0xC4,
    ST7735_VMCTR1  =0xC5,
    ST7735_RDID1   =0xDA,
    ST7735_RDID2   =0xDB,
    ST7735_RDID3   =0xDC,
    ST7735_RDID4   =0xDD,
    ST7735_PWCTR6  =0xFC,
    ST7735_GMCTRP1 =0xE0,
    ST7735_GMCTRN1 =0xE1
}ST7735_cmd_t;

/**
* \brief Bits within #ST7735_MADCTL
*/
typedef enum{
    ST7735_MADCTL_MY   = BIT(7),
    ST7735_MADCTL_MX   = BIT(6),
    ST7735_MADCTL_MV   = BIT(5),
    ST7735_MADCTL_ML   = BIT(4),
    ST7735_MADCTL_BGR  = BIT(3),
    ST7735_MADCTL_MH   = BIT(2),
    ST7735_MADCTL_RGB  = BIT(1)
}ST7735_MADCTL_BITS_t;

/**
* \brief Initialization sequence for 7735S screens
*/
static const uint8_t  SST35S_init_seq[] = {                     
    15,                     // 15 commands in list:
	
	//------------------------------------ST7735S Frame Rate-----------------------------------------//
	ST7735_FRMCTR1, 3,
	0x05,0x3C,0x3C,
	
	ST7735_FRMCTR2, 3,
	0x05, 0x3C, 0x3C,
	
	ST7735_FRMCTR3, 6,
	0x05, 0x3C, 0x3C, 0x05, 0x3C, 0x3C,
	
	//------------------------------------ ST7735S Power Sequence-----------------------------------------//
	ST7735_INVCTR, 1,
	0x03,
	
	ST7735_PWCTR1, 3,
	0x28, 0x08, 0x04,
	
	ST7735_PWCTR2, 1,
	0XC0,
	
	ST7735_PWCTR3, 2,
	0x0D, 0x00,
	
	ST7735_PWCTR4, 2,
	0x8D, 0x2A,
	
	ST7735_PWCTR5, 2,
	0x8D, 0xEE,
	
	//---------------------------------End ST7735S Power Sequence-------------------------------------//
	ST7735_VMCTR1, 1,
	0x1A,
	
	ST7735_MADCTL, 1,
	0xC0,
	
	//------------------------------------ST7735S Gamma Sequence-----------------------------------------//
	ST7735_GMCTRP1, 16,
	0x04,	0x22,	0x07,	0x0A,	0x2E,	0x30,	0x25,	0x2A,	0x28,	0x26,	0x2E,	0x3A,	0x00,	0x01,	0x03,	0x13,
	
	ST7735_GMCTRN1, 15,
	0x16,0x06,0x0D,0x2D,0x26,0x23,0x27,0x27,0x25,0x2D,0x3B,0x00,0x01,0x04,0x13,
	
	ST7735_COLMOD, 1,   //Set color mode
	0x05,               //16-bit color
	
	ST7735_DISPON, 0
	
};

/* [] END OF FILE */