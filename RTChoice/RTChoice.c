/*******************************************************************************
	
	NAME
		
		RTChoice
		
	SYNOPSIS
		
		Legacy arguments:
		TITLE/A,
		BODY/A,
		GADGETS/A/M,
		PUBSCREEN/K,
		SET/K,
		
		Non-legacy arguments:
		FILE/S,
		DEF=DEFAULT/N/K,
		POS=POSITION/K,
		CENTER=CENTERTEXT/S,
		NORET=NORETURNKEY/S,
		TOP=TOPOFFSET/N/K,
		LEFT=LEFTOFFSET/N/K
		
	LOCATION
		
		SYS:C/
		
	FUNCTION
		
		Allows AmigaDOS scripts to have access to the ReqTools.library
		rtEZRequestA() message requesters.
		
		This program is mostly compatible with the CBM legacy C:RequestChoice
		command, but it also adds some new options, based on the ReqTools 
		features.
		
	INPUTS
		
		TITLE       - The text to display in the title bar of the requester.
		
		BODY        - The text to display in the body of the requester.
		
		GADGETS     - The text for each of the buttons.
		
		PUBSCREEN   - The name of the public screen to open the requester upon.
		
		SET         - The environment variable where the result is written to.
		
		FILE        - The BODY argument is a filename. Its content will be used.
		
		DEFAULT     - The pre-selected gadget, connected to the RETURN key.
		              From 1 to n, or 0 for the most right gadget.
		
		CENTERTEXT  - The BODY lines will be centered in the requester.
		
		NORETURNKEY - Turn off the RETURN key as shortcut for positive response.
		
		POSITION    - The location of the requester, among :
					  
					  POINTER    - requester appears where the mouse pointer is.
					  CENTERSCR  - requester is centered on the screen (default).
					  CENTERWIN  - requester is centered in the window.
					  TOPLEFTSCR - requester appears at the top left of the screen.
					  TOPLEFTWIN - requester appears at the top left of the window.
		
		TOPOFFSET   - Offset of top edge of requester relative to POSITION.
		
		LEFTOFFSET  - Offset of left edge of requester relative to POSITION.
		
	RESULT
		
		Standard DOS return codes.
		
	NOTES
		
		To place a newline into the BODY of the requester use *n or *N.
		
		To place a quotation mark in the BODY of the requester use *".
		
	EXAMPLES
		
		RTChoice Info Line Ok
		
		RTChoice "Info" "line1*nline2" "ok|cancel"
		
		RTChoice "Info" "line1*nline2" "ok" "cancel" default 0
		
		C:Status >T:0
		RTChoice Status T:0 Ok FILE
		
********************************************************************************/


#include <stdio.h>
#include <string.h>

#include <dos/dos.h>
#include <dos/rdargs.h>
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <libraries/reqtools.h>
#include <utility/tagitem.h>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/utility.h>
#include <proto/reqtools.h>

/*****************************************************************************/

#define TEMPLATE "\
TITLE/A,\
BODY/A,\
GADGETS/A/M,\
PUBSCREEN/K,\
SET/K,\
FILE/S,\
DEF=DEFAULT/N/K,\
POS=POSITION/K,\
CENTER=CENTERTEXT/S,\
NORET=NORETURNKEY/S,\
TOP=TOPOFFSET/N/K,\
LEFT=LEFTOFFSET/N/K"

/*****************************************************************************/

enum {
	OPT_TITLE,          /* Compatible with CBM's RequestChoice */
	OPT_BODY,           /* Compatible with CBM's RequestChoice */
	OPT_GADGETS,        /* Compatible with CBM's RequestChoice */
	OPT_PUBSCREEN,      /* Compatible with CBM's RequestChoice */
	OPT_SET,            /* Compatible with CBM's RequestChoice */
	OPT_FILE,           /* NEW: Body from file                 */
	OPT_DEFAULT,        /* Specific to the ReqTools.library    */
	OPT_POSITION,       /* Specific to the ReqTools.library    */
	OPT_CENTERTEXT,     /* Specific to the ReqTools.library    */
	OPT_NORETURNKEY,    /* Specific to the ReqTools.library    */
	OPT_TOPOFFSET,      /* Specific to the ReqTools.library    */
	OPT_LEFTOFFSET,     /* Specific to the ReqTools.library    */
	OPT_COUNT           /* Number of supported arguments       */
};

/*****************************************************************************/

STRPTR VersionTag = "$VER: RTChoice 0.1 (10.5.2021) Philippe CARPENTIER";

/*****************************************************************************/

extern struct Library   *DOSBase;
extern struct ExecBase  *SysBase;
extern struct Library   *IntuitionBase;
extern struct Library   *UtilityBase;

struct ReqToolsBase     *ReqToolsBase;

/*****************************************************************************/

STRPTR GetRtTitle(LONG opts[OPT_COUNT])
{
	return((STRPTR)opts[OPT_TITLE]);
}

/*****************************************************************************/

ULONG GetRtFlags(LONG opts[OPT_COUNT])
{
	ULONG flags = NULL;
	
	if (opts[OPT_CENTERTEXT ]) flags |= EZREQF_CENTERTEXT;
	if (opts[OPT_NORETURNKEY]) flags |= EZREQF_NORETURNKEY;
	
	return(flags);
}

/*****************************************************************************/

ULONG GetRtPosition(LONG opts[OPT_COUNT])
{
	STRPTR s;
	
	if (s = (STRPTR)opts[OPT_POSITION])
	{
		if (!stricmp(s, "POINTER"   )) return(REQPOS_POINTER);
		if (!stricmp(s, "CENTERSCR" )) return(REQPOS_CENTERSCR);
		if (!stricmp(s, "CENTERWIN" )) return(REQPOS_CENTERWIN);
		if (!stricmp(s, "TOPLEFTSCR")) return(REQPOS_TOPLEFTSCR);
		if (!stricmp(s, "TOPLEFTWIN")) return(REQPOS_TOPLEFTWIN);
	}
	
	return(REQPOS_CENTERSCR);
}

/*****************************************************************************/

STRPTR GetRtBody(LONG opts[OPT_COUNT])
{
	STRPTR result = NULL;
	STRPTR s = (STRPTR)opts[OPT_BODY];
	ULONG  i, j, n;
	
	/* Calculate size */
	
	i = 0;
	n = strlen(s) + 1;
	
	while (s[i])
	{
		if (s[i++] == '%')
			n++;
	}
	
	/* Copy all items */
	
	if (n > 0)
	{
		if (result = AllocVec(n + 1, MEMF_ANY))
		{
			i = 0;
			j = 0;
			while (s[i])
			{
				result[j++] = s[i];
				
				if (s[i++] == '%')
					result[j++] = '%';
			}
			
			result[j] = 0;
		}
	}
	
	return(result);
}

/*****************************************************************************/

STRPTR GetRtGadgets(LONG opts[OPT_COUNT])
{
	STRPTR *strings = (STRPTR *)opts[OPT_GADGETS];
	STRPTR *gadgets;
	STRPTR  gadget;
	STRPTR  result = NULL;
	ULONG   n = 0;
	
	/* Calculate size */
	
	gadgets = strings;
	
	while (gadget = *gadgets)
	{
		ULONG i = 0;
		
		n += strlen(gadget) + 1;
		
		while (gadget[i])
		{
			if (gadget[i++] == '%')
				n++;
		}
		
		gadgets++;
	}
	
	/* Copy all items */
	
	if (n > 0)
	{
		if (result = AllocVec(n + 1, MEMF_ANY))
		{
			ULONG j = 0;
			
			gadgets = strings;
			
			while (gadget = *gadgets)
			{
				ULONG i = 0;
				
				while (gadget[i])
				{
					result[j++] = gadget[i];
					
					if (gadget[i++] == '%')
						result[j++] = '%';
				}
				
				gadgets++;
				
				if (*gadgets)
					result[j++] = '|';
			}
			
			result[j] = 0;
		}
	}
	
	return(result);
}

/*****************************************************************************/

struct TagItem * GetRtTagList(LONG opts[OPT_COUNT])
{
	struct TagItem *tags = NULL;
	
	if (tags = AllocateTagItems(10))
	{
		ULONG i = 0;
		
		struct Process *process = (struct Process *)FindTask(NULL);
		
		/* Keyboard Shortkey (char) */
		
		if (TRUE) {
			tags[i].ti_Tag  = RT_Underscore;
			tags[i].ti_Data = '_';
			i++;
		}
		
		/* Default Gadget (integer) */
		
		if (opts[OPT_DEFAULT])
		{
			tags[i].ti_Tag  = RTEZ_DefaultResponse;
			tags[i].ti_Data = *(ULONG *)opts[OPT_DEFAULT];
			i++;
		}
		
		/* Public Screen (string) */
		
		if (opts[OPT_PUBSCREEN]) {
			tags[i].ti_Tag  = RT_PubScrName;
			tags[i].ti_Data = opts[OPT_PUBSCREEN];
			i++;
		}
		
		/* Top Offset (integer) */
		
		if (opts[OPT_TOPOFFSET]) {
			tags[i].ti_Tag  = RT_TopOffset;
			tags[i].ti_Data = *(ULONG *)opts[OPT_TOPOFFSET];
			i++;
		}
		
		/* Left Offset (integer) */
		
		if (opts[OPT_LEFTOFFSET]) {
			tags[i].ti_Tag  = RT_LeftOffset;
			tags[i].ti_Data = *(ULONG *)opts[OPT_LEFTOFFSET];
			i++;
		}
		
		/* Window Address */
		
		if (process && process->pr_WindowPtr) {
			tags[i].ti_Tag  = RT_Window;
			tags[i].ti_Data = (ULONG)process->pr_WindowPtr;
			i++;
		}
		
		tags[i].ti_Tag = TAG_END;
	}
	
	return(tags);
}

/*****************************************************************************/

STRPTR FileToString(STRPTR fileName)
{
	BPTR   file = 0;
	BPTR   lock = 0;
	STRPTR buffer = NULL;
	struct FileInfoBlock* fib = NULL;
	
	if (lock = Lock(fileName, ACCESS_READ))
	{
		if (fib = (struct FileInfoBlock*)AllocMem(
			sizeof(struct FileInfoBlock), 
			MEMF_ANY))
		{
			if (Examine(lock, fib))
			{
				if (buffer = (UBYTE*)AllocVec(
					fib->fib_Size + 1, 
					MEMF_ANY))
				{
					if (file = Open(fileName, MODE_OLDFILE))
					{
						Read(file, buffer, fib->fib_Size);
						buffer[fib->fib_Size] = 0;
						Close(file);
					}
				}
			}
			
			FreeMem(fib, sizeof(struct FileInfoBlock));
		}
		
		UnLock(lock);
	}
	
	return(buffer);
}

/*****************************************************************************/

LONG main(VOID)
{
	LONG           opts[OPT_COUNT];
	struct RDArgs *rdargs;
	LONG           failureCode;
	LONG           failureLevel;
	
	failureCode  = ERROR_INVALID_RESIDENT_LIBRARY;
	failureLevel = RETURN_FAIL;
	
	memset(opts, 0, sizeof(opts));
	
	if (rdargs = ReadArgs(TEMPLATE, opts, NULL))
	{
		struct TagItem * rtTagList  = GetRtTagList (opts);
		ULONG            rtPosition = GetRtPosition(opts);
		ULONG            rtFlags    = GetRtFlags   (opts);
		STRPTR           rtTitle    = GetRtTitle   (opts);
		STRPTR           rtGadgets  = GetRtGadgets (opts);
		STRPTR           rtBody     = NULL;
		
		if (opts[OPT_FILE])
		{
			if (!(rtBody = FileToString((STRPTR)opts[OPT_BODY])))
			{
				printf("Failed to open file `%s`.\n", 
					(STRPTR)opts[OPT_BODY]);
			}
		}
		else
		{
			rtBody = GetRtBody(opts);
		}
		
		if (rtBody && rtGadgets)
		{
			failureCode = ERROR_NO_FREE_STORE;
			
			if (ReqToolsBase = (struct ReqToolsBase *)OpenLibrary(
				REQTOOLSNAME, 
				REQTOOLSVERSION))
			{
				struct rtReqInfo *rtInfo;
				
				if (rtInfo = (struct rtReqInfo *)rtAllocRequest(
					RT_REQINFO, 
					NULL))
				{
					UBYTE output[32];
					
					rtInfo->ReqTitle   = rtTitle;
					rtInfo->ReqPos     = rtPosition;
					rtInfo->Flags      = rtFlags;
					rtInfo->LeftOffset = 0;
					rtInfo->TopOffset  = 0;
					
					failureCode = rtEZRequestA(
						rtBody,      /* bodyfmt  */
						rtGadgets,   /* gadfmt   */
						rtInfo,      /* reqinfo  */
						NULL,        /* argarray */
						rtTagList);  /* taglist  */
					
					failureLevel = RETURN_OK;
					
					sprintf(output, "%ld\n", failureCode);
					
					if (opts[OPT_SET])
					{
						SetVar((STRPTR)opts[OPT_SET], output, -1,
							LV_VAR + GVF_GLOBAL_ONLY);
					}
					else
					{
						PutStr(output);
					}
					
					rtFreeRequest(rtInfo);
				}
				
				CloseLibrary(ReqToolsBase);
			}
			else
			{
				/* Can not open reqtools.library */
				failureCode = ERROR_OBJECT_NOT_FOUND;
			}
		}
		else
		{
			/* Required arguments missing */
			failureCode = ERROR_REQUIRED_ARG_MISSING;
		}
		
		if (rtBody)
		{
			FreeVec(rtBody);
		}
		
		if (rtGadgets)
		{
			FreeVec(rtGadgets);
		}
		
		if (rtTagList)
		{
			FreeTagItems(rtTagList);
		}
	}
	else
	{
		failureCode = IoErr();
	}
	
	SetIoErr(failureCode);
	
	if (failureLevel != RETURN_OK)
	{
		PrintFault(failureCode, NULL);
	}
	
	return(failureLevel);
}

/*****************************************************************************/
