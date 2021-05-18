/*******************************************************************************
	
	NAME
		
		RACHOICE
		
	SYNOPSIS
		
		TITLE/A,BODY/A,GADGETS/A/M,PUBSCREEN/K,SET/K,IMAGE/K,FILE/S
		
	LOCATION
		
		C:RACHOICE
		
	FUNCTION
		
		Allows AmigaDOS scripts to use Reaction-based message requesters.
		
	INPUTS
		
		TITLE       - The text to display in the title bar of the requester.
		
		BODY        - The text to display in the body of the requester.
		
		GADGETS     - The text for each of the buttons.
		
		PUBSCREEN   - The name of the public screen to open the requester upon.
		
		SET         - The environment variable where the result is written to.
		
		FILE        - The BODY argument is a filename. Its content will be used.
		
		IMAGE       - The image pictogram to display, among :
					  
				INFO       - Display a INFO pictogram.
				WARNING    - Display a WARNING pictogram.
				ERROR      - Display a ERROR pictogram.
				QUESTION   - Display a QUESTION pictogram.
				INSERTDISK - Display a INSERTDISK pictogram.
		
	RESULT
		
		Standard DOS return codes.
		
	NOTES
		This program use the V47 system 'requester.class'.
		
		To place a newline into the BODY of the requester use *n or *N.
		
		To place a quotation mark in the BODY of the requester use *".
		
	EXAMPLES
		
		RAChoice Info Line Ok
		
		RAChoice "Info" "line1*nline2" "ok|cancel"
                
		RAChoice "Warn" "line1*nline2" "ok|cancel" IMAGE=WARNING
                
		RAChoice "Warn" "line1*nline2" "ok|cancel" IMAGE=WARNING SET=MYVAR
		
		C:Status >T:0
		RAChoice Status T:0 Ok FILE
		
********************************************************************************/

#define USE_BUILTIN_MATH
#define USE_SYSBASE

#include <stdio.h>
#include <string.h>
#include <math.h>

#define	INTUI_V36_NAMES_ONLY

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/types.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/rdargs.h>
#include <utility/tagitem.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>
#include <clib/requester_protos.h>

#include <pragmas/intuition_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/requester_pragmas.h>

#define ALL_REACTION_CLASSES
#define ALL_REACTION_MACROS

#include <reaction/reaction.h>
#include <classes/requester.h>
#include <classes/window.h>

/*****************************************************************************/

#define TEMPLATE "TITLE/A,BODY/A,GADGETS/A,PUBSCREEN/K,SET/K,FILE/S,IMAGE/K"

/*****************************************************************************/

enum {
    OPT_TITLE,       /* Compatible with CBM's RequestChoice */
    OPT_BODY,        /* Compatible with CBM's RequestChoice */
    OPT_GADGETS,     /* Compatible with CBM's RequestChoice */
    OPT_PUBSCREEN,   /* Compatible with CBM's RequestChoice */
    OPT_SET,         /* Compatible with CBM's RequestChoice */
    OPT_FILE,        /* NEW: Load Body from file            */
    OPT_IMAGE,       /* NEW: Feature from requester.class   */
    OPT_COUNT        /* Number of supported arguments       */
};

/*****************************************************************************/

STRPTR VersionTag = "$VER: RAChoice 1.0 (18.5.2021) Philippe CARPENTIER";

/*****************************************************************************/

extern struct DOSBase  *DOSBase;
extern struct ExecBase *SysBase;
extern struct Library  *IntuitionBase;
extern struct Library  *UtilityBase;

struct Library *RequesterBase;

/*****************************************************************************/

ULONG OpenRequesterTags(Object *obj, ULONG Tag1, ...)
{
    struct Process *process = (struct Process *)FindTask(NULL);
    
    struct orRequest msg[1];
    
    msg->MethodID = RM_OPENREQ;
    msg->or_Window = process->pr_WindowPtr;
    msg->or_Screen = NULL;
    msg->or_Attrs = (struct TagItem *)&Tag1;
    
    return(DoMethodA(obj, (Msg)msg));
}

/*****************************************************************************/

ULONG GetReqImage(LONG opts[OPT_COUNT])
{
    STRPTR s;
    
    if (s = (STRPTR)opts[OPT_IMAGE])
    {
        if (!stricmp(s, "INFO"      )) return(REQIMAGE_INFO      );
        if (!stricmp(s, "WARNING"   )) return(REQIMAGE_WARNING   );
        if (!stricmp(s, "ERROR"     )) return(REQIMAGE_ERROR     );
        if (!stricmp(s, "QUESTION"  )) return(REQIMAGE_QUESTION  );
        if (!stricmp(s, "INSERTDISK")) return(REQIMAGE_INSERTDISK);
    }
    
    return(REQIMAGE_DEFAULT);
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
        if (fib = (struct FileInfoBlock*)AllocMem(sizeof(struct FileInfoBlock), MEMF_ANY))
        {
            if (Examine(lock, fib))
            {
                if (buffer = (UBYTE*)AllocVec(fib->fib_Size + 1, MEMF_ANY))
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
    LONG           args[OPT_COUNT];
    struct RDArgs *rdargs;
    LONG           failureCode;
    LONG           failureLevel;
    
    failureCode  = ERROR_INVALID_RESIDENT_LIBRARY;
    failureLevel = RETURN_FAIL;
    
    memset(args, 0, sizeof(args));
    
    if (rdargs = ReadArgs(TEMPLATE, args, NULL))
    {
        STRPTR Req_File    = NULL;
        STRPTR Req_Title   = (STRPTR)args[OPT_TITLE  ];
        STRPTR Req_Body    = (STRPTR)args[OPT_BODY   ];
        STRPTR Req_Gadgets = (STRPTR)args[OPT_GADGETS];
        ULONG  Req_Image   = GetReqImage(args);
        
        if (args[OPT_FILE])
        {
            if (!(Req_File = FileToString(Req_Body)))
            {
                printf("Failed to open file `%s`.\n", Req_Body);
            }
        }
	
        if (Req_Body && Req_Gadgets)
        {
            failureCode = ERROR_NO_FREE_STORE;
            
            if (RequesterBase = OpenLibrary("requester.class", 0L))
            {
                Object *Req_Object = NewObject(REQUESTER_GetClass(), NULL, 
                    REQ_TitleText, Req_Title, TAG_DONE);
                
                if (Req_Object)
                {
                    UBYTE output[32];
                    
                    failureCode = OpenRequesterTags(Req_Object,
                        REQ_Type,       REQTYPE_INFO,
                        REQ_Image,      Req_Image,
                        REQ_BodyText,   Req_File ? Req_File : Req_Body,
                        REQ_GadgetText, Req_Gadgets,
                        TAG_DONE);
                    
                    failureLevel = RETURN_OK;
                    
                    sprintf(output, "%ld\n", failureCode);
                    
                    if (args[OPT_SET])
                    {
                        SetVar((STRPTR)args[OPT_SET], output, -1,
                            LV_VAR + GVF_GLOBAL_ONLY);
                    }
                    else
                    {
                        PutStr(output);
                    }
                    
                    DisposeObject(Req_Object);
                }
                
                CloseLibrary((struct Library *)RequesterBase);
            }
            else
            {
                failureCode = ERROR_OBJECT_NOT_FOUND;
            }
        }
        else
        {
            failureCode = ERROR_REQUIRED_ARG_MISSING;
        }
        
        if (Req_File)
        {
            FreeVec(Req_File);
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
