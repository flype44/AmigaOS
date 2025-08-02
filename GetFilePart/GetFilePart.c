/******************************************************************************
 * 
 * File:  GetFilePart.c.
 * Short: Get the part(s) of a filename.
 * Build: sc LINK GetFilePart.c
 * 
 *****************************************************************************/

#include <string.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <exec/types.h>
#include <proto/exec.h>
#include <proto/dos.h>

/******************************************************************************
 * 
 * DEFINES
 * 
 *****************************************************************************/

#define APPNAME    "GetFilePart"
#define APPVERSION "1.0"
#define APPAUTHOR  "Philippe CARPENTIER"
#define TEMPLATE   "FILE/A,D=DRIVE/S,P=PATH/S,N=NODE/S,E=EXTENSION/S"
#define MAX_PATH   4096

typedef enum {
	OPT_FILE,
	OPT_DRIVE,
	OPT_PATH,
	OPT_NODE,
	OPT_EXTENSION,
	OPT_COUNT
} OPT_ARGS;

/******************************************************************************
 * 
 * GLOBALS
 * 
 *****************************************************************************/

static UBYTE D[MAX_PATH]; // Drive
static UBYTE P[MAX_PATH]; // Path
static UBYTE N[MAX_PATH]; // Node
static UBYTE E[MAX_PATH]; // Extension

STRPTR VERSTRING = "$VER: " APPNAME " " APPVERSION " " __AMIGADATE__ " [SAS/C 6.59] " APPAUTHOR;

/******************************************************************************
 * 
 * MAIN
 * 
 *****************************************************************************/

LONG main(VOID)
{
	LONG rc = RETURN_FAIL;
	LONG opts[OPT_COUNT];
	struct DosLibrary *DOSBase;
	struct RDArgs *rdArgs;
	memset(opts, 0, sizeof(opts));
	
	if ((DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 37)))
	{
		rc = RETURN_WARN;
		
		if (rdArgs = ReadArgs(TEMPLATE, opts, NULL))
		{
			strsfn((STRPTR)opts[OPT_FILE], D, P, N, E);
			
			if ((BOOL)opts[OPT_DRIVE] && strlen(D))
			{
				rc = RETURN_OK;
				PutStr(D);
			}
			
			if ((BOOL)opts[OPT_PATH] && strlen(P))
			{
				rc = RETURN_OK;
				PutStr(P);
			}
			
			if ((BOOL)opts[OPT_NODE] && strlen(N))
			{
				rc = RETURN_OK;
				PutStr(N);
			}
			
			if ((BOOL)opts[OPT_EXTENSION] && strlen(E))
			{
				rc = RETURN_OK;
				PutStr(".");
				PutStr(E);
			}
			
			if (rc == RETURN_OK)
			{
				PutStr("\n");
			}
			
			FreeArgs(rdArgs);
		}
		else
		{
			PutStr(VERSTRING+6);
			PutStr("\n\nUsage: " APPNAME " <file> [<options>]\n\n");
			PutStr(" FILE/A        - The filename to parse (mandatory).\n");
			PutStr(" D=DRIVE/S     - The file DRIVE part (optional).\n");
			PutStr(" P=PATH/S      - The file PATH part (optional).\n");
			PutStr(" N=NODE/S      - The file NODE part (optional).\n");
			PutStr(" E=EXTENSION/S - The file EXTENSION part (optional).\n\n");
			PutStr("Example:\n\n");
			PutStr("1> GetFilePart Drive:Path/Node.Extension D P N E\n\n");
		}
	}
	
	return (rc);
}

/******************************************************************************
 * 
 * END OF FILE
 * 
 *****************************************************************************/
