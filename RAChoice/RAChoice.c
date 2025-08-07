/*******************************************************************************
	
   NAME

	RACHOICE

   FORMAT

	RACHOICE <title> <body> <gadgets> [PUBSCREEN] [SET] [SETENV] [IMAGE] [FILE] [QUIET] [DEBUG]

   TEMPLATE

	TITLE/A,BODY/A,GADGETS/A/M,PUBSCREEN/K,SET/K,SETENV/K,IMAGE/K,FILE/S,QUIET/S,DEBUG/S

   PATH

	C:RACHOICE

   FUNCTION

	Allows AmigaDOS scripts to use Reaction-based message requesters.

	TITLE       - The text to display in the title bar of the requester.
	BODY        - The text to display in the body of the requester.
	GADGETS     - The text for each of the buttons.
	PUBSCREEN   - The name of the public screen to open the requester upon.
	SET         - The LOCAL environment variable where the result is written to.
	SETENV      - The GLOBAL environment variable where the result is written to.
	FILE        - The BODY argument is a filename. Its content will be used.
	IMAGE       - The pictogram or the image path to display, among :
			INFO       - Display a INFO pictogram.
			WARNING    - Display a WARNING pictogram.
			ERROR      - Display a ERROR pictogram.
			QUESTION   - Display a QUESTION pictogram.
			INSERTDISK - Display a INSERTDISK pictogram.
        		ELSE load any supported datatype image from disk.
	QUIET       - Suppress stdout messages.
	DEBUG       - Outputs debug information.

   RESULT

	Standard DOS return codes.

   NOTES

	This program use the V47 'classes/requester.class', 
	and, if asked, the 'images/bitmap.image' system objects.
	
	To place a newline into the BODY of the requester use *n or *N.
	
	To place a quotation mark in the BODY of the requester use *".

   EXAMPLES

	RAChoice Info Line Ok
	
	RAChoice "Info" "line1*nline2" "ok|cancel"
	RAChoice "Warn" "line1*nline2" "ok|cancel" IMAGE=WARNING
	RAChoice "Warn" "line1*nline2" "ok|cancel" IMAGE=WARNING SET=MYVAR
	RAChoice "Warn" "line1*nline2" "ok|cancel" IMAGE=RAM:WARN.PNG
	RAChoice "Warn" "line1*nline2" "ok|cancel" IMAGE=RAM:WARN.info
	
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
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/pictureclass.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/rdargs.h>
#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <utility/tagitem.h>

#include <clib/alib_protos.h>
#include <clib/datatypes_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/requester_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/datatypes_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>
#include <pragmas/requester_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include <proto/datatypes.h>

#define ALL_REACTION_CLASSES
#define ALL_REACTION_MACROS

#include <reaction/reaction.h>
#include <reaction/reaction_macros.h>

#include <images/bitmap.h>
#include <classes/requester.h>
#include <classes/window.h>

/*****************************************************************************/

#define TEMPLATE "TITLE/A,BODY/A,GADGETS/A,PUBSCREEN/K,SET/K,SETENV/K,FILE/S,IMAGE/K,QUIET/S,DEBUG/S"

/*****************************************************************************/

enum {
    OPT_TITLE,       /* Compatible with CBM's RequestChoice */
    OPT_BODY,        /* Compatible with CBM's RequestChoice */
    OPT_GADGETS,     /* Compatible with CBM's RequestChoice */
    OPT_PUBSCREEN,   /* Compatible with CBM's RequestChoice */
    OPT_SET,         /* Compatible with CBM's RequestChoice */
    OPT_SETENV,      /* NEW: Set Global env variable        */
    OPT_FILE,        /* NEW: Load Body from file            */
    OPT_IMAGE,       /* NEW: Feature from requester.class   */
    OPT_QUIET,       /* NEW: Suppress stdout messages       */
    OPT_DEBUG,       /* NEW: Outputs picture information    */
    OPT_COUNT        /* Number of supported arguments       */
};

/*****************************************************************************/

STRPTR VersionTag = "$VER: RAChoice 1.2 (7.8.2025) Philippe CARPENTIER";

/*****************************************************************************/

extern struct DOSBase  * DOSBase;
extern struct ExecBase * SysBase;
extern struct GfxBase  * GfxBase;
extern struct Library  * IntuitionBase;
extern struct Library  * UtilityBase;
extern struct Library  * DatatypesBase;

struct Library *BitMapBase;
struct Library *RequesterBase;
struct Image   *RequesterImage;

BOOL bDEBUG = FALSE;
BOOL bQUIET = FALSE;

/*****************************************************************************/

Object * LoadPicture(STRPTR name)
{
    Object * picture = NULL;
    struct Screen * screen = NULL;
    
    if (screen = LockPubScreen(NULL))
    {
        picture = NewDTObject(
            name,
            DTA_GroupID,           GID_PICTURE,
            DTA_SourceType,        DTST_FILE,
            PDTA_DestMode,         PMODE_V43,
            PDTA_FreeSourceBitMap, TRUE,
            PDTA_Remap,            TRUE,
            PDTA_Screen,           screen,
            OBP_Precision,         PRECISION_EXACT,
            OBP_FailIfBad,         FALSE,
            TAG_DONE);
        
        if (picture)
        {
            DoDTMethod(picture, NULL, NULL, DTM_PROCLAYOUT, NULL, TRUE);
        }
        
        UnlockPubScreen(NULL, screen);
    }
    
    return (picture);
}

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
        
        if (BitMapBase = OpenLibrary("images/bitmap.image", 0L))
        {
            struct Screen *screen;
            
            if(screen = LockPubScreen(NULL))
            {
                Object * picture;
                
                /*
                struct Image *RequesterImageAlt;
                RequesterImageAlt = BitMapObject,
                    BITMAP_SourceFile, s,
                    BITMAP_Screen, screen,
                    BITMAP_Precision, PRECISION_EXACT,
                    BITMAP_Masking, TRUE,
                    BITMAP_MaskPlane, (APTR)GetReqImage,
                    BITMAP_Transparent, TRUE,
                    EndImage;
                */
                
                if (picture = LoadPicture(s))
                {
                    struct BitMapHeader * bitMapHeader = NULL;
                    
                    GetDTAttrs(picture, PDTA_BitMapHeader, &bitMapHeader, TAG_DONE);
                    
                    if (bitMapHeader)
                    {
                        APTR maskPlane = NULL;
                        struct BitMap * bitMap = NULL;
                        
                        GetDTAttrs(picture, PDTA_BitMap, &bitMap, TAG_DONE);
                        GetDTAttrs(picture, PDTA_MaskPlane, &maskPlane, TAG_DONE);
                        
                        if (bDEBUG)
                        {
                            printf("Picture:   0x%08lx\n", picture);
                            printf("BitMap:    0x%08lx\n", bitMap);
                            printf("BitMap.w:  %u\n",      bitMapHeader->bmh_Width);
                            printf("BitMap.h:  %u\n",      bitMapHeader->bmh_Height);
                            printf("MaskPlane: 0x%08lx\n", maskPlane);
                        }
                        
                        if (!maskPlane && !bQUIET)
                        {
                            printf("WARNING: the picture maskPlane is null.\n");
                        }
                        
                        RequesterImage = BitMapObject,
                            BITMAP_Screen,      screen,
                            BITMAP_Precision,   PRECISION_IMAGE,
                            BITMAP_BitMap,      bitMap,
                            BITMAP_Masking,     ((maskPlane != NULL) ? TRUE : FALSE),
                            BITMAP_MaskPlane,   maskPlane,
                            BITMAP_Width,       bitMapHeader->bmh_Width,
                            BITMAP_Height,      bitMapHeader->bmh_Height,
                            BITMAP_Transparent, ((maskPlane != NULL) ? TRUE : FALSE),
                            EndImage;
                    }
                    else
                    {
                        if (!bQUIET) printf("WARNING: the bitmap header is null.\n");
                    }
                }
                else
                {
                    if (!bQUIET) printf("WARNING: failed to load picture.\n");
                }
                
                UnlockPubScreen(NULL, screen);
            }
            else
            {
                if (!bQUIET) printf("WARNING: cant lock public screen.\n");
            }
        }
        else
        {
            if (!bQUIET) printf("WARNING: can load bitmap.image.\n");
        }
    }
    
    if (RequesterImage)
    {
        return((ULONG)RequesterImage);
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
        
        bDEBUG  = (args[OPT_DEBUG ]  ? TRUE : FALSE);
        bQUIET  = (args[OPT_QUIET ]  ? TRUE : FALSE);
        bQUIET |= (args[OPT_SET   ]) ? TRUE : FALSE;
        bQUIET |= (args[OPT_SETENV]) ? TRUE : FALSE;
        
        if (bDEBUG) bQUIET = FALSE;
        
        if (args[OPT_FILE])
        {
            if (!(Req_File = FileToString(Req_Body)))
            {
                if (!bQUIET)
                {
                    printf("ERROR: failed to open file `%s`.\n", Req_Body);
                }
            }
        }
        
        if (Req_Body && Req_Gadgets)
        {
            failureCode = ERROR_NO_FREE_STORE;
            
            if (RequesterBase = OpenLibrary("requester.class", 0L))
            {
                Object *Req_Object = RequesterObject, 
                    REQ_TitleText, Req_Title, TAG_DONE);
                
                if (Req_Object)
                {
                    UBYTE output[32];
                    
                    failureCode = OpenRequesterTags(Req_Object,
                        REQ_Type,       REQTYPE_INFO,
                        REQ_Image,      GetReqImage(args),
                        REQ_BodyText,   Req_File ? Req_File : Req_Body,
                        REQ_GadgetText, Req_Gadgets,
                        TAG_DONE);
                    
                    failureLevel = RETURN_OK;
                    
                    sprintf(output, "%ld\n", failureCode);
                    
                    if (args[OPT_SET])
                    {
                        ULONG success = SetVar((STRPTR)args[OPT_SET], 
                            output, -1, LV_VAR + GVF_LOCAL_ONLY);
                        
                        if (!success && !bQUIET)
                        {
                            printf("ERROR: failed to set LOCAL variable.\n");
                            failureLevel = RETURN_ERROR;
                        }
                    }
                    else if (args[OPT_SETENV])
                    {
                        ULONG success = SetVar((STRPTR)args[OPT_SETENV], 
                            output, -1, LV_VAR + GVF_GLOBAL_ONLY);
                        
                        if (!success && !bQUIET)
                        {
                            printf("ERROR: failed to set GLOBAL variable.\n");
                            failureLevel = RETURN_ERROR;
                        }
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
                if (!bQUIET)
                {
                    printf("ERROR: failed to open requester.class.\n");
                    failureLevel = RETURN_FAIL;
                }
                
                failureCode = ERROR_OBJECT_NOT_FOUND;
            }
        }
        else
        {
            if (!bQUIET)
            {
                printf("ERROR: missing arguments.\n");
                failureLevel = RETURN_FAIL;
            }
            
            failureCode = ERROR_REQUIRED_ARG_MISSING;
        }
        
        if (Req_File)
        {
            FreeVec(Req_File);
        }
    }
    else
    {
        if (!bQUIET)
        {
            printf("ERROR: bad arguments.\n");
            failureLevel = RETURN_FAIL;
        }
        
        failureCode = IoErr();
    }
    
    if (RequesterImage)
    {
        DisposeObject(RequesterImage);
    }
    
    if (BitMapBase)
    {
        CloseLibrary(BitMapBase);
    }
    
    SetIoErr(failureCode);
    
    if (failureLevel != RETURN_OK)
    {
        PrintFault(failureCode, NULL);
    }
    
    return(failureLevel);
}

/*****************************************************************************/
