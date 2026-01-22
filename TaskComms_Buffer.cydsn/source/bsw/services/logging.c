/**
 * \file 	logging.c
 * \author 	Thomas Barth	- Hochschule Darmstadt thomas.barth@h-da.de
 * \date 	26.11.2016
 * \version 0.1
 *
 * \brief  Logging mechanism
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
/*=======================[ Includes ]===============================================================*/
#include <project.h>
#include "logging.h"


/*=======================[ constant data ]===============================================================*/

const char logLevel2String[6][CFG_LOGGING_LEVEL_L+1]={
    "Verbose",
    "Info",
    "Warning",
    "Error",
    "Debug",
    "Off"
};

/*=======================[ variable data ]===============================================================*/

/*=======================[ Function implementation ]=====================================================*/

RC_t LOG_Init(void){
#if (CFG_LOGGING_ENABLE==ON)   
    //Start logging service HW module
    MODULE_LOG(Start)();
    
    //indicate DEBUG Start
    MODULE_LOG(PutString)(CFG_LOGGING_NL_CHAR);
    MODULE_LOG(PutString)(CFG_LOGGING_NL_CHAR);
    MODULE_LOG(PutString)("UART started");
    MODULE_LOG(PutString)(CFG_LOGGING_NL_CHAR);

#else
    return RC_ERROR_INVALID_STATE;
#endif
}

RC_t LOG_send(logging_t* entry){
    
    //result buffer
    RC_t result=RC_ERROR_INVALID_STATE;
#if (CFG_LOGGING_ENABLE==ON)     
    MODULE_LOG(PutString)(entry->sender);
    MODULE_LOG(PutString)(" : ");
    MODULE_LOG(PutString)(entry->message);
    MODULE_LOG(PutString)(CFG_LOGGING_NL_CHAR);
    
    result = RC_SUCCESS;
    
#endif

    return result;

}

/* [] END OF FILE */
