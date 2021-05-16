/******************************************************************************
	
	NAME
		
		GETIPTRAFFIC
		
	SYNOPSIS
		
		U=UP/S,D=DOWN/S,H=HUMAN/S
		
	LOCATION
		
		C:GETIPTRAFFIC
		
	FUNCTION
		
		Retrieve the download/upload network traffic.
		
	INPUTS
		
		UP/S     - Retrieve the upload traffic.
		DOWN/S   - Retrieve the download traffic.
		HUMAN/S  - Display human-readable sizes.
		
	RESULT
		
		Standard DOS return codes.
		
	NOTES
		
		Requires an initialized TCP/IP stack.
		
		This means the "bsdsocket.library" must be available.
		
		Required TCP/IP stack implementation :
                SocketBaseTagList() and SBTC_GET_BYTES_SENT.
                SocketBaseTagList() and SBTC_GET_BYTES_RECEIVED.
		
	EXAMPLES
		
		> GetIPTraffic U D
		U: 81981 bytes, D: 569522 byte
		
		> GetIPTraffic U D H
		U: 80.0 KB, D: 556.2 KB
		
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dos/dos.h>
#include <exec/exec.h>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/socket.h>
#include <proto/utility.h>

#include "GetIPTraffic.h"

/*****************************************************************************/

extern struct ExecBase *SysBase;
extern struct DOSBase  *DOSBase;
extern struct Library  *UtilityBase;

struct Library *SocketBase;

static UBYTE StrBufSizeUL[128];
static UBYTE StrBufSizeDL[128];

static STRPTR VERSION = VERSTAG;

/*****************************************************************************/

STRPTR TEMPLATE = "U=UP/S,D=DOWN/S,H=HUMAN/S";

enum {
    OPT_UP,      /* Retrieve the upload traffic   */
    OPT_DOWN,    /* Retrieve the download traffic */
    OPT_HUMAN,   /* Display human-readable sizes  */
    OPT_COUNT    /* Number of supported arguments */
};

/*****************************************************************************/

UBYTE * PrintSize(ULONG s, UBYTE * buf)
{
    STRPTR u[] = { "B", "KB", "MB", "GB", "TB" };
    ULONG r, i = 0;
    
    while (s > 1024) { r = s % 1024; s >>= 10; i++; }
    if (r > 1000) { r = 0; s++; }
    r /= 100;
    
    sprintf(buf, "%lu.%lu %s", s, r, u[i]);
    
    return(buf);
}

/*****************************************************************************/

LONG main(ULONG argc, char *argv[])
{
    LONG result = RETURN_FAIL;
    LONG args[OPT_COUNT];
    struct RDArgs * rdargs;
    
    memset(args, 0, sizeof(args));
    
    if (rdargs = ReadArgs(TEMPLATE, args, NULL))
    {
        if (SocketBase = OpenLibrary("bsdsocket.library", 4))
        {
            ULONG i = 0;
            SBQUAD_T byteRecv;
            SBQUAD_T byteSent;
            struct TagItem tags[3];
            
            if (args[OPT_DOWN])
            {
                tags[i].ti_Tag  = SBTM_GETREF(SBTC_GET_BYTES_RECEIVED);
                tags[i].ti_Data = (ULONG)&byteRecv;
                i++;
            }
            
            if (args[OPT_UP])
            {
                tags[i].ti_Tag  = SBTM_GETREF(SBTC_GET_BYTES_SENT);
                tags[i].ti_Data = (ULONG)&byteSent;
                i++;
            }
            
            tags[i].ti_Tag = TAG_END;
            
            if (SocketBaseTagList(tags) == 0)
            {
                if (args[OPT_UP])
                {
                    if (args[OPT_HUMAN])
                    {
                        PrintSize(byteSent.sbq_Low, StrBufSizeUL);
                        printf("U: %s", StrBufSizeUL);
                    }
                    else
                    {
                        printf("U: %ld bytes", byteSent.sbq_Low);
                    }
                    
                    if (args[OPT_DOWN])
                        printf(", ");
                }
                
                if (args[OPT_DOWN])
                {
                    if (args[OPT_HUMAN])
                    {
                        PrintSize(byteRecv.sbq_Low, StrBufSizeDL);
                        printf("D: %s", StrBufSizeDL);
                    }
                    else
                    {
                        printf("D: %ld bytes", byteRecv.sbq_Low);
                    }
                }
                
                result = RETURN_OK;
                
                printf("\n");
            }
            
            CloseLibrary(SocketBase);
        }
        
        FreeArgs(rdargs);
    }
    else
    {
        PrintFault(IoErr(), "GetIPTraffic");
    }
    
    return(result);
}

/*****************************************************************************/
