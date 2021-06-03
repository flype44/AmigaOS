/******************************************************************************
 * 
 * UpdateIcon.c
 * 
 ******************************************************************************/

#include <stdio.h>

#include <dos/dos.h>
#include <exec/exec.h>
#include <workbench/icon.h>

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/icon.h>

/*****************************************************************************/

STRPTR VERSTAG = "$VER: UpdateIcon 1.0 (3.6.2021) Philippe CARPENTIER";

STRPTR APPNAME = "UpdateIcon";

extern struct ExecBase *SysBase;
extern struct DosLibrary *DOSBase;

struct Library *IconBase;

/*****************************************************************************/

STRPTR TEMPLATE = "FILE/A,DEFTOOL/A,QUIET/S";

enum
{
    OPT_FILE,      /* The file to update            */
    OPT_DEFTOOL,   /* The new Default Tool          */
    OPT_QUIET,     /* Do not printout the old value */
    OPT_COUNT      /* Number of supported arguments */
};

/*****************************************************************************/

LONG main(VOID)
{
    LONG result = RETURN_FAIL;
    LONG args[OPT_COUNT];
    struct RDArgs * rdargs;
    
    args[OPT_FILE   ] = NULL;
    args[OPT_DEFTOOL] = NULL;
    args[OPT_QUIET  ] = NULL;
    
    if (rdargs = ReadArgs(TEMPLATE, args, NULL))
    {
        if (IconBase = OpenLibrary(ICONNAME, 36))
        {
            struct DiskObject *object;
            
            result = RETURN_WARN;
            
            if (object = GetDiskObject((STRPTR)args[OPT_FILE]))
            {
                if (!args[OPT_QUIET])
                {
                    PutStr(object->do_DefaultTool);
                    PutStr("\n");
                }
                
                object->do_DefaultTool = (STRPTR)args[OPT_DEFTOOL];
                
                if (PutDiskObject((STRPTR)args[OPT_FILE], object))
                {
                    result = RETURN_OK;
                }
                
                FreeDiskObject(object);
            }
            else
            {
                PrintFault(IoErr(), APPNAME);
            }
            
            CloseLibrary(IconBase);
        }
        
        FreeArgs(rdargs);
    }
    else
    {
        PrintFault(IoErr(), APPNAME);
    }
    
    return(result);
}

/*****************************************************************************/
