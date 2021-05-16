/*******************************************************************************
	
	NAME
		
		GETIP
		
	SYNOPSIS
		
		ALL/S,FULL/S
		
	LOCATION
		
		C:GETIP
		
	FUNCTION
		
		Retrieve the local IP address(es).
		
	INPUTS
		
		ALL/S   - Retrieve all the IP addresses from the available
		          network interfaces (if there are more than one).
		
		FULL/S  - Print the network interface name. 
		
	RESULT
		
		Standard DOS return codes.
		
	NOTES
		
		Requirements:
		bsdsocket.library minimum version is >= 4.
		bsdsocket.library -> QueryInterfaceTags() method.
		
		AmiTCP?, Miami?, and RoadShow TCP/IP stacks supports it.
		For now, year 2021, the UAE bsdsocket does not support it.
		
	EXAMPLES
		
		> GetIP
		192.168.1.10
		
		> GetIP ALL
		192.168.1.10
		192.168.1.11
		192.168.1.12
		
        	> GetIP FULL
		192.168.1.10 on interface X
		
        	> GetIP FULL ALL
		192.168.1.10 on interface X
		192.168.1.11 on interface Y
		192.168.1.12 on interface Z
		
********************************************************************************/

#include <stdio.h>
#include <dos/dos.h>
#include <exec/exec.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/bsdsocket.h>

/*****************************************************************************/

STRPTR VERSTAG = "$VER: GetIP 1.0 (16.5.2021) Philippe CARPENTIER";

extern struct Library  *DOSBase;
extern struct ExecBase *SysBase;

struct Library *SocketBase;

/*****************************************************************************/

STRPTR TEMPLATE = "ALL/S,FULL/S";

enum {
    OPT_ALL,     /* Retrieve all IP addresses        */
    OPT_FULL,    /* Print the network interface name */
    OPT_COUNT    /* Number of supported arguments    */
};

/*****************************************************************************/

BOOL ProcessInterface(LONG args[OPT_COUNT], struct Node *node)
{
    BOOL result = FALSE;
    
    if (node && node->ln_Name)
    {
        struct sockaddr_in sin;
        STRPTR name = node->ln_Name;
        
        memset(&sin, 0, sizeof(sin));
        
        if (QueryInterfaceTags(name, IFQ_Address, &sin, TAG_DONE) == 0)
        {
            if(sin.sin_addr.s_addr != INADDR_ANY)
            {
                ULONG values[4];
                ULONG in = ntohl(sin.sin_addr.s_addr);
                
                values[0] = (in >> 24) & 255;
                values[1] = (in >> 16) & 255;
                values[2] = (in >>  8) & 255;
                values[3] = (in >>  0) & 255;
                
                VPrintf("%ld.%ld.%ld.%ld", &values);
                
                if (args[OPT_FULL])
                {
                    PutStr(" on interface ");
                    PutStr(name);
                }
                
                PutStr("\n");
                
                result = TRUE;
            }
        }
    }
    
    return(result);
}

/*****************************************************************************/

ULONG ProcessInterfaces(LONG args[OPT_COUNT])
{
    ULONG result = 0;
    struct List *list;
    
    if (list = ObtainInterfaceList())
    {
        struct Node *node;
        
        for (node = list->lh_Head; node->ln_Succ != NULL; node = node->ln_Succ)
        {
            if (ProcessInterface(args, node))
            {
                result++;
                
                if (!args[OPT_ALL])
                {
                    break;
                }
            }
            else
            {
                PutStr("QueryInterfaceTags() failed.\n");
            }
        }
        
        ReleaseInterfaceList(list);
    }
    else
    {
        PutStr("ObtainInterfaceList() failed.\n");
    }
    
    return(result);
}

/*****************************************************************************/

LONG main(VOID)
{
    LONG result = RETURN_FAIL;
    LONG args[OPT_COUNT];
    struct RDArgs * rdargs;
    
    struct Process * pr = (struct Process *)FindTask(NULL);
    
    if(pr->pr_CLI)
    {
        memset(args, 0, sizeof(args));
        
        if (rdargs = ReadArgs(TEMPLATE, args, NULL))
        {
            if (SocketBase = OpenLibrary("bsdsocket.library", 4))
            {
                struct TagItem tags[2];
                ULONG haveInterfaceAPI = 0;
                
                result = RETURN_WARN;
                
                tags[0].ti_Tag  = SBTM_GETREF(SBTC_HAVE_INTERFACE_API);
                tags[0].ti_Data = (ULONG)&haveInterfaceAPI;
                tags[1].ti_Tag  = TAG_END;
                
                if (SocketBaseTagList(tags) == 0 && haveInterfaceAPI)
                {
                    if (ProcessInterfaces(args))
                    {
                        result = RETURN_OK;
                    }
                }
                else
                {
                    PutStr("No Network Interface API.\n");
                }
                
                CloseLibrary(SocketBase);
            }
            else
            {
                PutStr("No TCP/IP stack.\n");
            }
            
            FreeArgs(rdargs);
        }
        else
        {
            PrintFault(IoErr(), "GetIP");
        }
    }
    else
    {
        struct MsgPort * mp;
        struct Message * mn;
        mp = &pr->pr_MsgPort;
        WaitPort(mp);
        mn = GetMsg(mp);
        Forbid();
        ReplyMsg(mn);
    }
    
    return(result);
}

/*****************************************************************************/
