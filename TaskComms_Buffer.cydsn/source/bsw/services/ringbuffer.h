/**
 * \file 	ringbuffer.h
 * \author	Thomas Barth 				Hochschule Darmstadt - thomas.barth@h-da.de
 * \date 	03.05.2019
 *
 * \brief 	General purpose ringbuffer mechanism for the use with ERIKA os 
 *
 * \todo This helper is WIP, use with caution 
 *
 * ----- Changelog -----
 *
 * \copyright Copyright ?DateTime inactive during development
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
#ifndef RINGBUFFER_H
#define RINGBUFFER_H
    
/*=======================[ Includes ]==============================================================*/
    
#include <project.h>
#include "global.h"
    
/*=======================[ Symbols ]===============================================================*/

/**\brief Indicator if a ringbuffer shall not use ERIKA ressources */
#define RINGBUFFER_NO_ERIKA_RES     uint32_t_MAX
#define RB_SIZE 200
typedef uint8_t RB_buffer_t;
    
/*=======================[ Types ]=================================================================*/

/**
 * \brief Rungbuffer handler structure
 */
typedef struct{
    RB_buffer_t m_buffer[RB_SIZE];
    uint32_t    m_readIndex;        /**< Read index, points to the next byte to be read */
    uint32_t    m_writeIndex;       /**< Write index, points to the next free byte in the buffer */
    uint32_t    m_fillLevel;        /**< Number of bytes stored in the buffer*/
} RB_t;
    
/*****************************************************************************/
/* API functions                                                             */
/*****************************************************************************/

RC_t RB_init(RB_t* const rb);

RC_t RB_write(RB_t* const me, RB_buffer_t const * const data);

RC_t RB_read(RB_t* const me, RB_buffer_t * const data);

uint16_t RB_getCapacity(RB_t * const me);

uint16_t RB_getNumberOfStoredElements(RB_t * const me);

RC_t RB_clear(RB_t * const me);

#endif /*RINGBUFFER_H*/