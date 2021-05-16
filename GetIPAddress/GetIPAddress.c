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
		
		For now, this wont work on the UAE TCP/IP implementation,
		because it does not handle the ioctl() flag "SIOCGIFCONF".
		
	EXAMPLES
		
		> GetIPAddress
		192.168.1.10
		
******************************************************************************/

#include <stdio.h>
#include <sys/param.h>
#include <sys/ioctl.h>

#include <dos/dos.h>
#include <exec/exec.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/bsdsocket.h>

/*****************************************************************************/

#ifndef INADDR_LOOPBACK
#define INADDR_LOOPBACK 0x7f000001 /* 127.0.0.1 */
#endif

/*****************************************************************************/

STRPTR VERSTAG = "$VER: GetIPAddress 1.0 (16.5.2021) Philippe CARPENTIER";

extern struct Library  *DOSBase;
extern struct ExecBase *SysBase;

struct Library *SocketBase;

/*****************************************************************************/

#define ERRORMSG "N/A\n"

STRPTR TEMPLATE = "DEBUG/S";

enum {
    OPT_DEBUG,   /* Print the error messages      */
    OPT_COUNT    /* Number of supported arguments */
};

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
            
            for (len = ifc.ifc_len; len; len -= sizeof(ifreq))
            {
                ifreq = *ifr;
                
                if (IoctlSocket(sock, SIOCGIFFLAGS, (char *)&ifreq) >= 0)
                {
                    int slop;
                    
                    if ((ifreq.ifr_flags & IFF_UP) && 
                         ifr->ifr_addr.sa_family == AF_INET)
                    {
                        struct sockaddr_in *sin;
                        
                        sin = (struct sockaddr_in *)&ifr->ifr_addr;
                        
                        if(sin->sin_addr.s_addr != INADDR_ANY && 
                           sin->sin_addr.s_addr != INADDR_LOOPBACK)
                        {
                            ULONG bytes[4];
                            ULONG in = ntohl(sin->sin_addr.s_addr);
                            
                            bytes[0] = (in >> 24) & 255;
                            bytes[1] = (in >> 16) & 255;
                            bytes[2] = (in >>  8) & 255;
                            bytes[3] = (in      ) & 255;
                            
                            VPrintf("%ld.%ld.%ld.%ld\n", &bytes);
                            
                            result = TRUE;
                        }
                        
                        if (!(ifreq.ifr_flags & IFF_LOOPBACK))
                        {
                            break;
                        }
                    }
                    
                    slop = ifr->ifr_addr.sa_len - sizeof(struct sockaddr);
                    
                    if (slop)
                    {
                        ifr = (struct ifreq *) ((caddr_t)ifr + slop);
                        len -= slop;
                    }
                    
                    ifr++;
                }
                else
                {
                    PutStr(args[OPT_DEBUG] ? 
                        "ioctl(SIOCGIFFLAGS) failed.\n" : ERRORMSG);
                }
            }
        }
        else
        {
            PutStr(args[OPT_DEBUG] ? 
                "ioctl(SIOCGIFCONF) failed.\n" : ERRORMSG);
        }
        
        CloseSocket(sock);
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
