#ifndef __SHOWSANADEV_H__
#define __SHOWSANADEV_H__

/******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <exec/exec.h>
#include <exec/types.h>

/******************************************************************************
 * DEFINES FOR APP
 ******************************************************************************/

#define APP_NAME     "ShowSanaDev"
#define APP_DESCR    "A simple SANA2 commandline tool"
#define APP_AUTHOR   "Philippe CARPENTIER"
#define APP_VERSION  "0"
#define APP_REVISION "2alpha"
#define APP_DATE     "13.5.2024"
#define APP_TITLE    APP_NAME " by " APP_AUTHOR
#define VERSTAG      "$VER: " APP_NAME " " APP_VERSION "." APP_REVISION " (" APP_DATE ") (C) " APP_AUTHOR "\0"

/******************************************************************************
 * DEFINES FOR ARGUMENTS
 ******************************************************************************/

#define DEFAULT_DEVICE_NAME "prism2.device"
#define DEFAULT_DEVICE_UNIT 0

/******************************************************************************
 * END OF FILE
 ******************************************************************************/

#endif // __SHOWSANADEV_H__
