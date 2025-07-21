/******************************************************************************
 * 
 * NSDCheck.h
 * 
 ******************************************************************************/

#ifndef __NSDCHECK_H__
#define __NSDCHECK_H__

/******************************************************************************
 * 
 * Includes
 * 
 ******************************************************************************/

#include <exec/exec.h>
#include <exec/types.h>

#include <devices/newstyle.h>
#include <devices/audio.h>
#include <devices/clipboard.h>
#include <devices/console.h>
#include <devices/input.h>
#include <devices/keyboard.h>
#include <devices/gameport.h>
#include <devices/parallel.h>
#include <devices/printer.h>
#include <devices/scsidisk.h>
#include <devices/serial.h>
#include <devices/timer.h>
#include <devices/trackdisk.h>
#include <devices/sana2.h>
#include <devices/sana2specialstats.h>
#include <devices/sana2wireless.h>

/******************************************************************************
 * 
 * Prototypes
 * 
 ******************************************************************************/

VOID NSDCheck_PrintError(struct IOSana2Req *req);
BOOL NSDCheck_CheckDevice(STRPTR devName, LONG devUnit);
ULONG NSDCheck_CountCommands(UWORD * commands);
BOOL NSDCheck_FindCommand(UWORD command, UWORD * commands);
VOID NSDCheck_PrintSupportedCommands(UWORD devType, UWORD devSubType, UWORD *list);
VOID NSDCheck_PrintCommands(UWORD * list, UWORD * known_codes, CONST_STRPTR * known_names);

#endif // __NSDCHECK_H__
