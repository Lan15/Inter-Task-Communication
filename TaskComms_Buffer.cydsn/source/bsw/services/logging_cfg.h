/**
 * \file 	system_cfg.h
 * \author 	Thomas Barth	- Hochschule Darmstadt thomas.barth@h-da.de
 * \date 	09.07.2017
 * \version	0.2
 *
 * \brief Global configuration of the LiDAR project
 *
 * Within this file, the task configuration, as well as configuration of the sub modules is defined.
 * Please see the documentation of defined symbols.
 *
 *  Changelog:\n
 *  - 0.2   09.07.2017 Barth
 *            - added CFG_SYS_STR
 *  - 0.1   09.11.2016 Barth
 *            - Initial documentation
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
 
#ifndef SYSTEM_CFG_H_
#define SYSTEM_CFG_H_
    
//-------------------------------------------------------------------- [System String]
#define CFG_SYS_STR                 "Engine (c) H-DA, P. Fromm"    
        
//-------------------------------------------------------------------- [TFT]
/** 
* \brief orientation of the TFT after startup
*
* See #TFT_rotation_t for possible values.
* After startup, #TFT_setRotation can be used to rotate the screen.
*/

#define CFG_TFT_ORIENTATION         TFT_rot_180  
#define CFG_TFT_RB_OUT_SZ           100                     /**< \brief TFT Ringbuffer OUT size for SPI communication*/
#define CFG_TFT_BACKLIGHT           100                     /**< \brief TFT backlight brightness after start (0-100). Can be adjusted with #TFT_setBacklight during runtime*/
#define CFG_TFT_SPRINTF_BUF_L       128                     /**< \brief Buffer length for #TFT_printf */   
#define CFG_TFT_COLOR_TEXT          WHITE                   /**< \brief Default text color */
#define CFG_TFT_COLOR_TEXTBG        BLACK                   /**< \brief Default text background color */
//More parameters in #TFT.h
        
//-------------------------------------------------------------------- [Logging]  
    
#define CFG_LOGGING_ENABLE          ON
    
#define CFG_LOGGING_LEVEL           LOG_VERBOSE                /**< \brief Defines the logging-level. See #logging_level_t */
#define CFG_LOGGING_NL_CHAR         "\n"                    /**< \brief Newline characters for logging output */

    
#define CFG_LOGGING_DT_T            LOGGING_USE_N_T         /**< \brief Defines if no date or time #LOGGING_USE_N_T, the time #LOGGING_USE_T_T or date AND time #LOGGING_USE_DT_T are safed within a logging message. */    
#define CFG_LOGGING_L_M             150                     /**< \brief Number of Characters, that can be used within an logging message */
#define CFG_LOGGING_LEVEL_L         7                       /**< \brief Number of Characters, that can be used for the logging message level */
#define CFG_LOGGING_SENDER_L        15                      /**< \brief Number of Characters, that can be used for the logging message sender */
#define CFG_LOGGING_FR_L            30                      /**< \brief Number of Characters, that can be used additionally for wrapping the information into a readable format  */
 
#define CFG_LOGGING_MODULE          UART_LOG                /**< \brief Name of the UART Module, which is used for logging messages */   
   
#endif //SYSTEM_CFG_H_
/* [] END OF FILE */
