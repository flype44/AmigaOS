/******************************************************************************
	
	NAME
		
		GETIPADDRESS
		
	SYNOPSIS
		
		DEBUG/S
		
	LOCATION
		
		C:GETIPADDRESS
		
	FUNCTION
		
		Retrieve the local IP address.
		
	INPUTS
		
		DEBUG/S  - Print the error messages, instead of "N/A".
		
	RESULT
		
		Standard DOS return codes.
		
	NOTES
		
		Requires an initialized TCP/IP stack.
		
		This means the "bsdsocket.library" must be available.
		
		The address is read with the ioctl() flags "SIOCGIFCONF" and
		"SIOCGIFFLAGS" (AmiTCP, Roadshow). Stacks without them, such
		as lwip-amiga, are asked through the Roadshow interface query
		API (ObtainInterfaceList, QueryInterfaceTagList) instead.
		The UAE TCP/IP implementation has neither.
		
	EXAMPLES
		
		> GetIPAddress
		192.168.1.10
		
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/ioctl.h>

#include <dos/dos.h>
#include <exec/exec.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/bsdsocket.h>
#include <utility/tagitem.h>

/*****************************************************************************/

#ifndef INADDR_LOOPBACK
#define INADDR_LOOPBACK 0x7f000001 /* 127.0.0.1 */
#endif

/* Roadshow interface query API (Roadshow SDK libraries/bsdsocket.h) */

#ifndef IFQ_BASE
#define IFQ_BASE    (TAG_USER + 1900)
#define IFQ_Address (IFQ_BASE + 14) /* struct sockaddr * */
#define IFQ_State   (IFQ_BASE + 19) /* LONG * (SM_*)     */
#define SM_Up       3
#endif

/* Last LVO of the query API: older stacks have shorter vector tables */

#define LVO_QUERYINTERFACETAGLIST 468

/*****************************************************************************/

STRPTR VERSTAG = "$VER: GetIPAddress 1.1 (28.10.2026) Philippe CARPENTIER";

#ifdef __SASC
extern struct Library  *DOSBase;
extern struct ExecBase *SysBase;
#endif

struct Library *SocketBase;

/*****************************************************************************/

#define ERRORMSG "N/A\n"

STRPTR TEMPLATE = "DEBUG/S";

enum {
    OPT_DEBUG,   /* Print the error messages      */
    OPT_COUNT    /* Number of supported arguments */
};

/*****************************************************************************/

BOOL PrintAddress(ULONG addr)
{
    ULONG bytes[4];
    ULONG in = ntohl(addr);
    
    if (in == INADDR_ANY || (in >> 24) == 127)
    {
        return(FALSE);
    }
    
    bytes[0] = (in >> 24) & 255;
    bytes[1] = (in >> 16) & 255;
    bytes[2] = (in >>  8) & 255;
    bytes[3] = (in      ) & 255;
    
    VPrintf("%ld.%ld.%ld.%ld\n", &bytes);
    
    return(TRUE);
}

/*****************************************************************************/

BOOL QueryInterfaces(LONG args[OPT_COUNT])
{
    BOOL result = FALSE;
    struct List *list;
    struct Node *node;
    
    if (SocketBase->lib_NegSize < LVO_QUERYINTERFACETAGLIST)
    {
        PutStr(args[OPT_DEBUG] ? 
            "No interface query API.\n" : ERRORMSG);
        return(FALSE);
    }
    
    if ((list = ObtainInterfaceList()))
    {
        for (node = list->lh_Head; node->ln_Succ && !result; node = node->ln_Succ)
        {
            struct sockaddr_in sin;
            LONG state = SM_Up;
            struct TagItem tags[3];
            
            memset(&sin, 0, sizeof(sin));
            
            tags[0].ti_Tag  = IFQ_Address;
            tags[0].ti_Data = (ULONG)&sin;
            tags[1].ti_Tag  = IFQ_State;
            tags[1].ti_Data = (ULONG)&state;
            tags[2].ti_Tag  = TAG_DONE;
            
            if (QueryInterfaceTagList(node->ln_Name, tags) == 0 && 
                state == SM_Up && sin.sin_family == AF_INET)
            {
                result = PrintAddress(sin.sin_addr.s_addr);
            }
        }
        
        ReleaseInterfaceList(list);
    }
    
    if (!result)
    {
        PutStr(args[OPT_DEBUG] ? 
            "No interface is up.\n" : ERRORMSG);
    }
    
    return(result);
}

/*****************************************************************************/

BOOL GetIPAddress(LONG args[OPT_COUNT])
{
    BOOL result = FALSE;
    
    int sock;
    
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
    {
        char buf[512];
        struct ifconf ifc;
        
        ifc.ifc_buf = buf;
        ifc.ifc_len = sizeof(buf);
        
        if (IoctlSocket(sock, SIOCGIFCONF, (char *)&ifc) >= 0)
        {
            int len;
            struct ifreq ifreq;
            struct ifreq *ifr = ifc.ifc_req;
            int slop;
            
            for (len = ifc.ifc_len; len >= (int)sizeof(ifreq); len -= sizeof(ifreq))
            {
                ifreq = *ifr;
                
                if (IoctlSocket(sock, SIOCGIFFLAGS, (char *)&ifreq) >= 0)
                {
                    if ((ifreq.ifr_flags & IFF_UP) && 
                         ifr->ifr_addr.sa_family == AF_INET)
                    {
                        struct sockaddr_in *sin;
                        
                        sin = (struct sockaddr_in *)&ifr->ifr_addr;
                        
                        if (PrintAddress(sin->sin_addr.s_addr))
                        {
                            result = TRUE;
                        }
                        
                        if (!(ifreq.ifr_flags & IFF_LOOPBACK))
                        {
                            break;
                        }
                    }
                }
                else if (args[OPT_DEBUG])
                {
                    PutStr("ioctl(SIOCGIFFLAGS) failed.\n");
                }
                
                slop = ifr->ifr_addr.sa_len - sizeof(struct sockaddr);
                
                if (slop > 0)
                {
                    ifr = (struct ifreq *) ((caddr_t)ifr + slop);
                    len -= slop;
                }
                
                ifr++;
            }
            
            CloseSocket(sock);
        }
        else
        {
            if (args[OPT_DEBUG])
            {
                PutStr("ioctl(SIOCGIFCONF) failed, trying the interface query API.\n");
            }
            
            CloseSocket(sock);
            
            result = QueryInterfaces(args);
        }
    }
    else
    {
        PutStr(args[OPT_DEBUG] ? 
            "socket() failed.\n" : ERRORMSG);
    }
    
    return(result);
}

/*****************************************************************************/

LONG main(VOID)
{
    LONG result = RETURN_FAIL;
    LONG args[OPT_COUNT];
    struct RDArgs * rdargs;
    
    memset(args, 0, sizeof(args));
    
    if (rdargs = ReadArgs(TEMPLATE, args, NULL))
    {
        if (SocketBase = OpenLibrary("bsdsocket.library", 0))
        {
            result = RETURN_WARN;
            
            if (GetIPAddress(args))
            {
                result = RETURN_OK;
            }
            
            CloseLibrary(SocketBase);
        }
        else
        {
            PutStr(args[OPT_DEBUG] ? 
                "No TCP/IP stack.\n" : ERRORMSG);
        }
        
        FreeArgs(rdargs);
    }
    else
    {
        PrintFault(IoErr(), "GetIPAddress");
    }
    
    return(result);
}

/*****************************************************************************/
