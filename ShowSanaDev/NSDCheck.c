/******************************************************************************
 * 
 * NSDCheck.c
 * 
 ******************************************************************************/

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

#include <dos/dos.h>
#include <dos/rdargs.h>
#include <exec/exec.h>
#include <exec/types.h>
#include <exec/errors.h>
#include <exec/memory.h>
#include <exec/devices.h>
#include <utility/utility.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <clib/alib_protos.h>

#include "NSDCheck.h"

/******************************************************************************
 * 
 * Defines
 * 
 ******************************************************************************/

#define NSDEV_EXTRA_BYTES 128

#ifndef NSDEVTYPE_AUDIO
#define NSDEVTYPE_AUDIO NSDEVTYPE_AUDIOARD // typo
#endif

/******************************************************************************
 * 
 * Supported commands CODES
 * 
 ******************************************************************************/

UWORD NSDEVTYPE_CODES[] = {
	NSDEVTYPE_UNKNOWN,
	NSDEVTYPE_GAMEPORT,
	NSDEVTYPE_TIMER,
	NSDEVTYPE_KEYBOARD,
	NSDEVTYPE_INPUT,
	NSDEVTYPE_TRACKDISK,
	NSDEVTYPE_CONSOLE,
	NSDEVTYPE_SANA2,
	NSDEVTYPE_AUDIO,
	NSDEVTYPE_CLIPBOARD,
	NSDEVTYPE_PRINTER,
	NSDEVTYPE_SERIAL,
	NSDEVTYPE_PARALLEL
};

UWORD CMD_IO_CODES[] = {
	CMD_RESET,
	CMD_READ,
	CMD_WRITE,
	CMD_UPDATE,
	CMD_CLEAR,
	CMD_STOP,
	CMD_START,
	CMD_FLUSH,
	NSCMD_DEVICEQUERY,
	NULL
};

UWORD CMD_SANA2_CODES[] = {
	S2_DEVICEQUERY, 
	S2_GETSTATIONADDRESS, 
	S2_CONFIGINTERFACE,
	S2_ADDMULTICASTADDRESS, 
	S2_DELMULTICASTADDRESS, 
	S2_MULTICAST, 
	S2_BROADCAST, 
	S2_TRACKTYPE, 
	S2_UNTRACKTYPE,
	S2_GETTYPESTATS, 
	S2_GETSPECIALSTATS, 
	S2_GETGLOBALSTATS,
	S2_ONEVENT, 
	S2_READORPHAN, 
	S2_ONLINE, 
	S2_OFFLINE, 
	S2_ADDMULTICASTADDRESSES, 
	S2_DELMULTICASTADDRESSES, 
	S2_GETPEERADDRESS, 
	S2_GETDNSADDRESS, 
	S2_GETEXTENDEDGLOBALSTATS, 
	S2_CONNECT, 
	S2_DISCONNECT, 
	S2_SAMPLE_THROUGHPUT, 
	S2_GETSIGNALQUALITY, 
	S2_GETNETWORKS, 
	S2_SETOPTIONS, 
	S2_SETKEY, 
	S2_GETNETWORKINFO, 
	S2_READMGMT, 
	S2_WRITEMGMT, 
	S2_GETRADIOBANDS, 
	S2_GETCRYPTTYPES, 
	NULL
};

/******************************************************************************
 * 
 * Supported commands NAMES
 * 
 ******************************************************************************/

CONST_STRPTR NSDEVTYPE_NAMES[] = {
	"NSDEVTYPE_UNKNOWN",
	"NSDEVTYPE_GAMEPORT",
	"NSDEVTYPE_TIMER",
	"NSDEVTYPE_KEYBOARD",
	"NSDEVTYPE_INPUT",
	"NSDEVTYPE_TRACKDISK",
	"NSDEVTYPE_CONSOLE",
	"NSDEVTYPE_SANA2",
	"NSDEVTYPE_AUDIO",
	"NSDEVTYPE_CLIPBOARD",
	"NSDEVTYPE_PRINTER",
	"NSDEVTYPE_SERIAL",
	"NSDEVTYPE_PARALLEL"
};

CONST_STRPTR CMD_IO_NAMES[] = {
	"CMD_RESET", 
	"CMD_READ", 
	"CMD_WRITE", 
	"CMD_UPDATE", 
	"CMD_CLEAR", 
	"CMD_STOP", 
	"CMD_START", 
	"CMD_FLUSH",
	"NSCMD_DEVICEQUERY"
};

CONST_STRPTR CMD_SANA2_NAMES[] = {
	"S2_DEVICEQUERY", 
	"S2_GETSTATIONADDRESS", 
	"S2_CONFIGINTERFACE",
	"S2_ADDMULTICASTADDRESS", 
	"S2_DELMULTICASTADDRESS", 
	"S2_MULTICAST", 
	"S2_BROADCAST", 
	"S2_TRACKTYPE", 
	"S2_UNTRACKTYPE",
	"S2_GETTYPESTATS", 
	"S2_GETSPECIALSTATS", 
	"S2_GETGLOBALSTATS",
	"S2_ONEVENT", 
	"S2_READORPHAN", 
	"S2_ONLINE", 
	"S2_OFFLINE", 
	"S2_ADDMULTICASTADDRESSES", 
	"S2_DELMULTICASTADDRESSES", 
	"S2_GETPEERADDRESS", 
	"S2_GETDNSADDRESS", 
	"S2_GETEXTENDEDGLOBALSTATS", 
	"S2_CONNECT", 
	"S2_DISCONNECT", 
	"S2_SAMPLE_THROUGHPUT", 
	"S2_GETSIGNALQUALITY", 
	"S2_GETNETWORKS", 
	"S2_SETOPTIONS", 
	"S2_SETKEY", 
	"S2_GETNETWORKINFO", 
	"S2_READMGMT", 
	"S2_WRITEMGMT", 
	"S2_GETRADIOBANDS", 
	"S2_GETCRYPTTYPES"
};

/******************************************************************************
 * 
 * NSDCheck_PrintError()
 * 
 ******************************************************************************/

VOID NSDCheck_PrintError(struct IOSana2Req *req)
{
	printf("\nDevice Error : ");
	
	switch (req->ios2_Req.io_Error)
	{
		case IOERR_OPENFAIL:   PutStr("IOERR_OPENFAIL"  ); break;
		case IOERR_ABORTED:    PutStr("IOERR_ABORTED"   ); break;
		case IOERR_NOCMD:      PutStr("IOERR_NOCMD"     ); break;
		case IOERR_BADLENGTH:  PutStr("IOERR_BADLENGTH" ); break;
		case IOERR_BADADDRESS: PutStr("IOERR_BADADDRESS"); break;
		case IOERR_UNITBUSY:   PutStr("IOERR_UNITBUSY"  ); break;
		case IOERR_SELFTEST:   PutStr("IOERR_SELFTEST"  ); break;
		default:
			Printf("%ld", req->ios2_Req.io_Error);
			break;
	}
}

/******************************************************************************
 * 
 * NSDCheck_CountCommand()
 * 
 ******************************************************************************/

ULONG NSDCheck_CountCommands(UWORD * commands)
{
	ULONG result = 0;
	
	while (*commands) {
		commands++;
		result++;
	}
	
	return (result);
}

/******************************************************************************
 * 
 * NSDCheck_FindCommand()
 * 
 ******************************************************************************/

BOOL NSDCheck_FindCommand(UWORD command, UWORD * commands)
{
	while (*commands)
	{
		if (*commands == command)
		{
			return (TRUE);
		}
		
		commands++;
	}
	
	return (FALSE);
}

/******************************************************************************
 * 
 * NSDCheck_PrintCommands()
 * 
 ******************************************************************************/

VOID NSDCheck_PrintCommands(UWORD * list, UWORD * known_codes, CONST_STRPTR * known_names)
{
	while (*known_codes)
	{
		printf("[%s] 0x%04x %s\n", 
			NSDCheck_FindCommand(*known_codes, list) ? "X" : " ",
			*known_codes, *known_names);
		
		known_codes++;
		known_names++;
	}
}

/******************************************************************************
 * 
 * NSDCheck_PrintSupportedCommands()
 * 
 ******************************************************************************/

VOID NSDCheck_PrintSupportedCommands(UWORD devType, UWORD devSubType, UWORD *list)
{
	// Generic IO commands
	
	NSDCheck_PrintCommands(list, CMD_IO_CODES, CMD_IO_NAMES);
	
	// Specific DeviceType commands
	
	switch (devType)
	{
		case NSDEVTYPE_SANA2:
			NSDCheck_PrintCommands(list, CMD_SANA2_CODES, CMD_SANA2_NAMES);
			break;
	}
}

/******************************************************************************
 * 
 * NSDCheck_CheckDevice()
 * 
 ******************************************************************************/

BOOL NSDCheck_CheckDevice(STRPTR devName, LONG devUnit)
{
	BOOL result = FALSE;
	struct MsgPort *msgPort;
	struct IOStdReq *ioStdReq;
	struct IORequest *ioRequest;
	struct NSDeviceQueryResult nsd;
	
	if (msgPort = CreateMsgPort())
	{
		if (ioStdReq = CreateIORequest(msgPort, sizeof(struct IOStdReq) + NSDEV_EXTRA_BYTES))
		{
			ioRequest = (struct IORequest *)ioStdReq;
			
			if (OpenDevice(devName, devUnit, ioRequest, 0) == 0)
			{
				BYTE error;
				
				memset(&nsd, 0, sizeof(nsd));
				ioStdReq->io_Command = NSCMD_DEVICEQUERY;
				ioStdReq->io_Data    = &nsd;
				ioStdReq->io_Length  = sizeof(nsd);
				
				error = DoIO(ioRequest);
				
				if (error == 0)
				{
					Printf("NSCMD_DEVICEQUERY Response :\n");
					Printf("io_Actual               = %ld bytes\n", ioStdReq->io_Actual);
					Printf("io_Length               = %ld bytes\n", ioStdReq->io_Length);
					Printf("nsdqr_DevQueryFormat    = %ld\n",       nsd.nsdqr_DevQueryFormat);
					Printf("nsdqr_SizeAvailable     = %ld bytes\n", nsd.nsdqr_SizeAvailable);
					Printf("nsdqr_DeviceType        = %s (%ld)\n",  NSDEVTYPE_NAMES[nsd.nsdqr_DeviceType], nsd.nsdqr_DeviceType);
					Printf("nsdqr_DeviceSubType     = %ld\n",       nsd.nsdqr_DeviceSubType);
					Printf("nsdqr_SupportedCommands = %ld commands\n\n", NSDCheck_CountCommands(nsd.nsdqr_SupportedCommands));
					Printf("NSCMD_DEVICEQUERY Supported commands :\n");
					
					NSDCheck_PrintSupportedCommands(
						nsd.nsdqr_DeviceType, 
						nsd.nsdqr_DeviceSubType, 
						nsd.nsdqr_SupportedCommands);
					
					result = TRUE;
				}
				
				CloseDevice(ioRequest);
			}
			
			DeleteIORequest(ioRequest);
		}
		
		DeleteMsgPort(msgPort);
	}
	
	return (result);
}

/******************************************************************************
 * 
 * END OF FILE
 * 
 ******************************************************************************/
