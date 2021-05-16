/******************************************************************************
	
	NAME
		
		GETIPADDRESS
		
	SYNOPSIS
		
		No arguments
		
	LOCATION
		
		C:GETIPADDRESS
		
	FUNCTION
		
		Retrieve the local IP address.
		
	INPUTS
		
		NONE
		
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

BOOL GetIPAddress(VOID)
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
                    PutStr("ioctl(SIOCGIFFLAGS) failed.\n");
                }
            }
        }
        else
        {
            PutStr("ioctl(SIOCGIFCONF) failed.\n");
        }
        
        CloseSocket(sock);
    }
    else
    {
        PutStr("socket() failed.\n");
    }
    
    return(result);
}

/*****************************************************************************/

LONG main(VOID)
{
    LONG result = RETURN_FAIL;
    
    if (SocketBase = OpenLibrary("bsdsocket.library", 0))
    {
        result = RETURN_WARN;
        
        if (GetIPAddress())
        {
            result = RETURN_OK;
        }
        
        CloseLibrary(SocketBase);
    }
    else
    {
        PutStr("No TCP/IP stack.\n");
    }

    return(result);
}

/*****************************************************************************/
