/**
* \file ringbuffer.c
* \author V.S. Agilan
* \date 28.12.25
*
* \brief Ringbuffer
*
* Copyright YOUR COMPANY, THE YEAR
* All Rights Reserved
* UNPUBLISHED, LICENSED SOFTWARE.
*
* CONFIDENTIAL AND PROPRIETARY INFORMATION
* WHICH IS THE PROPERTY OF your company.
*
* ========================================
*/

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include <project.h>
#include "global.h"
#include "ringbuffer.h"

/*****************************************************************************/
/* API functions                                                             */
/*****************************************************************************/

/**
 * \brief initializes a ringbuffer handle / Clearing of a ringbuffer
 * @param RB_t * const me : IN/OUT - pointer to ringbuffer object 
 * @return RC_SUCCESS or error code
 */
RC_t RB_init(RB_t* const me)
{
    if (NULL_PTR == me)
    {
        return RC_ERROR_NULL;
    }
    
    me->m_fillLevel     = 0;
    me->m_readIndex     = 0;
    me->m_writeIndex    = 0;
    
    for (uint16_t i = 0; i < RB_SIZE; i++)
    {
        me->m_buffer[i] = 0;
    }
    
    return RC_SUCCESS;
}

/**
 * Write one element to the ringbuffer
 * @param RB_t * const me : IN/OUT - pointer to ringbuffer object 
 * @param RB_buffer_t const * const data : IN - Data to be written
 * @return RC_SUCCESS or error code
 */
RC_t RB_write(RB_t* const me, RB_buffer_t const * const data)
{
    //GetResource(rb->erika_ressource);
    
    //we check if there is enough memory left to store the new data
    if (me->m_fillLevel == RB_SIZE)
    {
        return RC_ERROR_BUFFER_FULL;
    } else if (me->m_fillLevel < RB_SIZE)
    {
        me->m_buffer[me->m_writeIndex++] = *data;
        me->m_writeIndex %= RB_SIZE;
        me->m_fillLevel++;
        
        return RC_SUCCESS;
    }
    
    //ReleaseResource(rb->erika_ressource);
    
    return RC_SUCCESS;
}

/**
 * Read one element from the ringbuffer
 * @param RB_t * const me : IN/OUT - pointer to ringbuffer object 
 * @param RB_buffer_t * const data : OUT - Data read
 * @return RC_SUCCESS or error code
 */
RC_t RB_read(RB_t* const me, RB_buffer_t * const data)
{
    //GetResource(rb->erika_ressource);
    
    //we check if the requested size is actually present in the buffer
    if (me->m_fillLevel == 0)
    {
        return RC_ERROR_BUFFER_EMTPY;
    } else if (me->m_fillLevel > 0)
    {
        *data = me->m_buffer[me->m_readIndex++];
        
        me->m_readIndex %= RB_SIZE;
        me->m_fillLevel--;
        
        return RC_SUCCESS;
    }
    
    //ReleaseResource(rb->erika_ressource);
    
    return RC_SUCCESS;
}

/**
 * Provide available space in buffer
 * @param RB_t * const me : IN/OUT - pointer to ringbuffer object 
 * @return Available number of elements to be stored
 */
uint16_t RB_getCapacity(RB_t * const me)
{
    //Todo: No error handling considered, assume all other functions work as intended
    return RB_SIZE - me->m_fillLevel;
}

/**
 * Check how much data is stored
 * @param RB_t * const me : IN/OUT - pointer to ringbuffer object 
 * @return number of byte stored
 */
uint16_t RB_getNumberOfStoredElements(RB_t * const me)
{
    return me->m_fillLevel;
}

/* [ringBuffer.c] END OF FILE */
