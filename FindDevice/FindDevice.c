/******************************************************************************
 * 
 * FindDevice.c
 * SAS/C 6.59 compiler
 * sc link noicon FindDevice.c
 * 
 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/filehandler.h>
#include <exec/exec.h>
#include <proto/dos.h>
#include <proto/exec.h>

/******************************************************************************
 * 
 * DEFINES
 * 
 *****************************************************************************/

#define APPNAME    "FindDevice"
#define APPVERSION "1.1"
#define APPAUTHOR  "Philippe CARPENTIER"
#define TEMPLATE   "NAME,TYPE,DEVICE,HANDLER,LIST/S,HELP/S"
#define MAX_PATH   4096

/******************************************************************************
 * 
 * ARRAYS
 * 
 *****************************************************************************/

typedef enum {
	OPT_NAME,
	OPT_TYPE,
	OPT_DEVICE,
	OPT_HANDLER,
	OPT_LIST,
	OPT_HELP,
	OPT_COUNT
} OPT_ARGS;

/******************************************************************************
 * 
 * GLOBALS
 * 
 ******************************************************************************/

extern struct DosLibrary * DOSBase;

static UBYTE bufName   [MAX_PATH];
static UBYTE bufType   [MAX_PATH];
static UBYTE bufDevice [MAX_PATH];
static UBYTE bufHandler[MAX_PATH];

STRPTR VERSTRING = "$VER: " APPNAME " " APPVERSION " " __AMIGADATE__ " [SAS/C 6.59] " APPAUTHOR;

/******************************************************************************
 * 
 * FindDevice()
 * 
 ******************************************************************************/

BOOL FindDevice(STRPTR dosName, STRPTR dosType, STRPTR deviceName, STRPTR handlerName, BOOL listAll)
{
	BOOL result = FALSE;
	struct DosList * dosList;
	
	if (dosList = LockDosList(LDF_READ | LDF_DEVICES))
	{
		if (dosName)     strlwr(dosName);
		if (deviceName)  strlwr(deviceName);
		if (handlerName) strlwr(handlerName);
		if (dosType)     strlwr(dosType);
		
		while ((dosList = NextDosEntry(dosList, LDF_DEVICES)))
		{
			if (dosList->dol_Type == DLT_DEVICE)
			{
				struct FileSysStartupMsg * msg;
				
				if (msg = (struct FileSysStartupMsg *)BADDR(dosList->dol_misc.dol_handler.dol_Startup))
				{
					struct DosEnvec * dosEnvec;
					
					if (dosEnvec = (struct DosEnvec *)BADDR(msg->fssm_Environ))
					{
						if (dosEnvec->de_DosType != 0xFFFFFFFF)
						{
							UBYTE * nameStr    = BADDR(dosList->dol_Name);
							UBYTE * handlerStr = BADDR(dosList->dol_misc.dol_handler.dol_Handler);
							UBYTE * deviceStr  = BADDR(msg->fssm_Device);
							
							sprintf(bufType, "%08lx", dosEnvec->de_DosType);
							
							if (nameStr)
							{
								UBYTE byteCount = nameStr[0];
								strncpy(bufName, &nameStr[1], byteCount);
								bufName[byteCount] = '\0';
							}
							
							if (deviceStr)
							{
								UBYTE byteCount = deviceStr[0];
								strncpy(bufDevice, &deviceStr[1], byteCount);
								bufDevice[byteCount] = '\0';
								
								if (!handlerStr)
								{
									strncpy(bufHandler, &deviceStr[1], byteCount);
									bufHandler[byteCount] = '\0';
								}
							}
							
							if (handlerStr)
							{
								UBYTE byteCount = handlerStr[0];
								strncpy(bufHandler, &handlerStr[1], byteCount);
								bufHandler[byteCount] = '\0';
							}
							
							if (listAll)
							{
								printf("NAME=%s, TYPE=%s, DEVICE=%s, HANDLER=%s\n", 
									bufName, bufType, bufDevice, bufHandler);
								
								result = TRUE;
							}
							else
							{
								BOOL a = (!dosName     || strstr(strlwr(bufName),    dosName));
								BOOL b = (!dosType     || strstr(strlwr(bufType),    dosType));
								BOOL c = (!deviceName  || strstr(strlwr(bufDevice),  deviceName));
								BOOL d = (!handlerName || strstr(strlwr(bufHandler), handlerName));
								
								if (a && b && c && d)
								{
									UBYTE byteCount = nameStr[0];
									strncpy(bufName, &nameStr[1], byteCount);
									bufName[byteCount] = '\0';
									PutStr(bufName);
									PutStr(":\n");
									result = TRUE;
								}
							}
						}
					}
				}
			}
		}
		
		UnLockDosList(LDF_READ | LDF_DEVICES);
	}
	
	return (result);
}

/******************************************************************************
 * 
 * ENTRY POINT
 * 
 ******************************************************************************/

ULONG main(int argc, char *argv[])
{
	LONG rc = RETURN_FAIL;
	LONG opts[OPT_COUNT];
	struct RDArgs * rdArgs;
	memset(opts, 0, sizeof(opts));
	
	if (rdArgs = ReadArgs(TEMPLATE, opts, NULL))
	{
		if ((BOOL)opts[OPT_HELP])
		{
			PutStr(VERSTRING+6);
			PutStr("\n\nUsage: " APPNAME " [<options>]\n\n");
			PutStr(" - NAME     - Drive Name (optional)\n");
			PutStr(" - TYPE     - Drive Type (optional)\n");
			PutStr(" - DEVICE   - Device Name (optional)\n");
			PutStr(" - HANDLER  - Handler Name (optional)\n");
			PutStr(" - LIST/S   - List all drives (optional)\n\n");
			PutStr("Example:\n\n");
			PutStr(" > FindDevice LIST\n");
			PutStr(" > FindDevice NAME=SD\n");
			PutStr(" > FindDevice TYPE=46415401\n");
			PutStr(" > FindDevice DEVICE=brcm\n");
			PutStr(" > FindDevice HANDLER=fat\n");
			PutStr(" > FindDevice TYPE=46415401 DEVICE=brcm\n");
			PutStr(" > FindDevice DEVICE=brcm HANDLER=fat\n\n");
			
			rc = RETURN_OK;
		}
		else
		{
			rc = FindDevice(
				(STRPTR)opts[OPT_NAME],
				(STRPTR)opts[OPT_TYPE],
				(STRPTR)opts[OPT_DEVICE],
				(STRPTR)opts[OPT_HANDLER],
				(BOOL)opts[OPT_LIST]) ? RETURN_OK : RETURN_WARN;
		}
		
		FreeArgs(rdArgs);
	}
	
	return (rc);
}

/******************************************************************************
 * 
 * END OF FILE
 * 
 ******************************************************************************/
