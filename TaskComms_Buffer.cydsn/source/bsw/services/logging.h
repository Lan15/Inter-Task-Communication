/**
 * \file 	logging.h
 * \author 	Thomas Barth	- Hochschule Darmstadt thomas.barth@h-da.de
 * \date 	12.09.2019
 * \version	2.0
 *
 * \brief Logging mechanism
 *
 *
 *  Changelog:\n
 *  - 1.0 06.07.18 Barth
 *  	- BaseSys V3
 *  - 0.1   21.09.2016  Barth
 *            - Initial documentation
 *
 * \copyright Copyright ©2018
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

#ifndef _LOGGING_H_
#define _LOGGING_H_

/*=======================[ Includes ]==============================================================*/

#include "global.h"
#include "logging_cfg.h"
#include "ringbuffer.h"
    
#include <stdio.h>

/*=======================[ Symbols ]===============================================================*/

#define LOGGING_USE_N_T     0               /**< \brief Do not add date or time to logging messages*/
#define LOGGING_USE_T_T     1               /**< \brief Add time to logging messages*/
#define LOGGING_USE_DT_T    2               /**< \brief Add date and time to logging messages*/
    
/**\brief Module API concationation with #CFG_LOGGING_MODULE. */
#define MODULE_LOG(x)       PPCAT(CFG_LOGGING_MODULE, x)    

/**\brief Reference to the logging queue */
extern RB_t    rb_log;
    
/**\brief Stringification of debug levels */
extern const char logLevel2String[6][CFG_LOGGING_LEVEL_L+1];
        
/*=======================[ Types ]=================================================================*/
    
/**
 * \brief Debugging Level.
 *
 * The debugging level defines the character of an event and is used to define which events are send to the log using #LOGGING_LEVEL
 */
typedef enum{
    LOG_VERBOSE=0,											/**< \brief Low Level Information */
    LOG_INFO=1,												/**< \brief High level information during normal operation */
    LOG_WARNING=2,											/**< \brief abnormal operation, which might be fixed */
    LOG_ERROR=3,											/**< \brief abnormal operation, which can not be fixed */
    LOG_DEBUG=4,											/**< \brief high priority for debugging messages (use only temporarily for debugging) */
    LOG_OFF=5												/**< \brief Event logging is switched off */
}logging_level_t;
 
/**
 * \brief Logging structure
 *
 * Each entry in the log has the following structure
 */
typedef struct{
    logging_level_t     level;                              /**< \brief Logging level, see #logging_level_t */
    char                sender [CFG_LOGGING_SENDER_L+1];    /**< \brief sender of the message */
    char                message[CFG_LOGGING_L_M+1];         /**< \brief message content */
}logging_t;

/*=======================[ logging interface ]=================================================================*/

 /** 
 * \brief Macro for sending messages to the logging system
 * \param x     Logging level, see #logging_level_t
 * \param y     Sender identification string
 * \param ...   The message to be printed, can be used in #sprintf fassion.
 * \warning     There is no check if the message was send to the queue. If the user wants to be sure that the message is send,
 *              then the macro below can be modified to retry the queue send operation or another mechanism has to be used.
 */
#define LOG(x,y,...)                                \
{                                                   \
    if(x>=CFG_LOGGING_LEVEL){                       \
        logging_t msg;                              \
        msg.level=x;                                \
        sprintf (msg.sender, "%s",y);               \
        sprintf (msg.message, __VA_ARGS__);         \
        LOG_send(&msg);                             \
    }                                               \
}                                                   \

/**
* \brief Macro for sending verbose messages to the logging system
* \param y     Sender identification string
* \param ...   The message to be printed, can be used in #sprintf fassion.
*/
#define LOG_V(tag,...)    	LOG(LOG_VERBOSE,tag, __VA_ARGS__)

/**
* \brief Macro for sending information messages to the logging system
* \param y     Sender identification string
* \param ...   The message to be printed, can be used in #sprintf fassion.
*/
#define LOG_I(tag,...)    	LOG(LOG_INFO,tag, __VA_ARGS__)

/**
* \brief Macro for sending warning messages to the logging system
* \param y     Sender identification string
* \param ...   The message to be printed, can be used in #sprintf fassion.
*/
#define LOG_W(tag,...)    	LOG(LOG_WARNING,tag, __VA_ARGS__)

/**
* \brief Macro for sending error messages to the logging system
* \param y     Sender identification string
* \param ...   The message to be printed, can be used in #sprintf fassion.
*/
#define LOG_E(tag,...)    	LOG(LOG_ERROR,tag, __VA_ARGS__)

/**
* \brief Macro for sending debug messages to the logging system
* \param y     Sender identification string
* \param ...   The message to be printed, can be used in #sprintf fassion.
*/
#define LOG_D(tag,...)    	LOG(LOG_DEBUG,tag, __VA_ARGS__)
    
/*=======================[ Prototypes ]============================================================*/

/**
 * \brief Initializes the logging mechanism
 *
 * Starts the hardware module specified if #system_config and initializes the internal message buffer.
 * \return The result of #Ringbuffer_Init
 */
RC_t LOG_Init(void);

/**
 * \brief   Send a new logging message to the logging-queue. It is recommended to use #LOGGING_SEND instead of this function directly.
 * \param   entry   Logging message, see #logging_t.
 * \return  #RC_ERROR_INVALID_STATE if the logging service is not available.
 *          #RC_SUCCESS if the message was added to the queue.
 *          #RC_ERROR_BUFFER_FULL if the queue is full and the message was not added.
 */
RC_t LOG_send(logging_t* entry);

#endif /* LOGGING_H_ */

/* [] END OF FILE */