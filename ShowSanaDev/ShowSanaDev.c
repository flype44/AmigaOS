/******************************************************************************
 * 
 * ShowSanaDev.c
 * 
 * ShowSanaDev is a simple AmigaOS3.x SANA2 information tool.
 * It provides as much information as possible on a SANA2 network device,
 * It supports latest AmigaOS SANA2 standards, such as the Wireless command set, 
 * and also brings some support for the AmigaOS NSD standard (NewStyleDevice).
 * 
 * This program may help SANA2 driver developers to implement the SANA2
 * function set, and end-users to check how behave their network device.
 * 
 * See official documentation here for more information about NSD :
 * https://wiki.amigaos.net/wiki/NSD_Standard
 * 
 * See official documentation here for more information about SANA2 :
 * https://wiki.amigaos.net/wiki/SANA-II_Standard
 * 
 * ShowSanaDev is more or less inspired from :
 * SanaUtil by Timo Rossi.
 * http://aminet.net/package/comm/net/sanautil.
 * 
 * Written by Philippe CARPENTIER, 2024.
 * Compiled with SAS/C 6.58 for AmigaOS/M68K.
 * Freely distributed for non-commercial purposes.
 * 
 ******************************************************************************
 * 
 * SYNTAX :
 * C:ShowSanaDev DEVICE=prism2.device UNIT=0 DEVICEQUERY
 * C:ShowSanaDev DEVICE=prism2.device UNIT=0 NSDEVICEQUERY
 * C:ShowSanaDev DEVICE=prism2.device UNIT=0 GETCRYPTTYPES
 * C:ShowSanaDev DEVICE=prism2.device UNIT=0 GETDNSADDRESS
 * C:ShowSanaDev DEVICE=prism2.device UNIT=0 GETEXTENDEDGLOBALSTATS
 * C:ShowSanaDev DEVICE=prism2.device UNIT=0 GETGLOBALSTATS
 * C:ShowSanaDev DEVICE=prism2.device UNIT=0 GETNETWORKINFO
 * C:ShowSanaDev DEVICE=prism2.device UNIT=0 GETNETWORKS
 * C:ShowSanaDev DEVICE=prism2.device UNIT=0 GETPEERADDRESS
 * C:ShowSanaDev DEVICE=prism2.device UNIT=0 GETRADIOBANDS
 * C:ShowSanaDev DEVICE=prism2.device UNIT=0 GETSIGNALQUALITY
 * C:ShowSanaDev DEVICE=prism2.device UNIT=0 GETSPECIALSTATS
 * C:ShowSanaDev DEVICE=prism2.device UNIT=0 GETSTATIONADDRESS
 * C:ShowSanaDev DEVICE=prism2.device UNIT=0 GETVERSION
 * 
 ******************************************************************************
 * 
 * HISTORY :
 * 0.1alpha 2024-05-11 First version
 * 
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <devices/newstyle.h>
#include <devices/sana2.h>
#include <devices/sana2specialstats.h>
#include <devices/sana2wireless.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <exec/exec.h>
#include <exec/types.h>
#include <exec/errors.h>
#include <exec/memory.h>
#include <exec/devices.h>
#include <utility/utility.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>
#include <clib/alib_protos.h>

#include "NSDCheck.h"
#include "ShowSanaDev.h"

/******************************************************************************
 * Prototypes
 ******************************************************************************/

VOID HexDump(UBYTE * buf, LONG length);

STRPTR FormatBand(ULONG band);
STRPTR FormatBSSID(UBYTE * addr);
STRPTR FormatTIMEVAL(struct TimeVal * timeVal);

VOID SanaDev_Help(VOID);
VOID SanaDev_NSDeviceQuery(VOID);
VOID SanaDev_DeviceOpen(STRPTR name, ULONG unit, ULONG flags);
VOID SanaDev_DeviceClose(STRPTR msg, LONG code);
VOID SanaDev_DeviceError(struct IOSana2Req * req);
VOID SanaDev_DeviceVersion(VOID);
VOID SanaDev_DeviceQuery(VOID);
VOID SanaDev_GetCryptTypes(VOID);
VOID SanaDev_GetDnsAddress(VOID);
VOID SanaDev_GetExtendedGlobalStats(VOID);
VOID SanaDev_GetGlobalStats(VOID);
VOID SanaDev_GetNetworkInfo(VOID);
VOID SanaDev_GetNetworkInfoPrint(APTR tagList);
VOID SanaDev_GetNetworks(STRPTR ssid);
VOID SanaDev_GetPeerAddress(VOID);
VOID SanaDev_GetRadioBands(VOID);
VOID SanaDev_GetSignalQuality(VOID);
VOID SanaDev_GetSpecialStats(VOID);
VOID SanaDev_GetStationAddress(VOID);

/******************************************************************************
 * Defines
 ******************************************************************************/

#define exit XCEXIT
#define POOLPUDDLESIZE 32768
#define POOLTHRESHSIZE 32768

UBYTE HELP[] = APP_NAME " usage :\n"
"DN=DEVICE/K                 -- The SANA Device Name (Default: " DEFAULT_DEVICE_NAME ").\n"
"DU=UNIT/N/K                 -- The SANA Device Unit (Default: 0).\n"
"DQ=DEVICEQUERY/S            -- Print the DeviceQuery.\n"
"NS=NSDEVICEQUERY/S          -- Print the NewStyle DeviceQuery.\n"
"CT=GETCRYPTTYPES/S          -- Print the supported Crypt Types.\n"
"DA=GETDNSADDRESS/S          -- Print the DNS Address.\n"
"ES=GETEXTENDEDGLOBALSTATS/S -- Print the Extended Global Stats.\n"
"GS=GETGLOBALSTATS/S         -- Print the Global Stats.\n"
"NI=GETNETWORKINFO/S         -- Print the current Wireless network.\n"
"NW=GETNETWORKS/S            -- Print the available Wireless networks.\n"
"PA=GETPEERADDRESS/S         -- Print the Peer Address.\n"
"RB=GETRADIOBANDS/S          -- Print the supported Radio Bands.\n"
"SQ=GETSIGNALQUALITY/S       -- Print the Signal Quality.\n"
"SS=GETSPECIALSTATS/S        -- Print the Special Stats.\n"
"SA=GETSTATIONADDRESS/S      -- Print the Station Address.\n"
"VE=GETVERSION/S             -- Print the Device version String.\n"
"HELP/S                      -- Print this help.\n";

UBYTE TEMPLATE[] = "\
DN=DEVICE/K,\
DU=UNIT/N/K,\
DQ=DEVICEQUERY/S,\
NS=NSDEVICEQUERY/S,\
CT=GETCRYPTTYPES/S,\
DA=GETDNSADDRESS/S,\
ES=GETEXTENDEDGLOBALSTATS/S,\
GS=GETGLOBALSTATS/S,\
NI=GETNETWORKINFO/S,\
NW=GETNETWORKS/S,\
PA=GETPEERADDRESS/S,\
RB=GETRADIOBANDS/S,\
SQ=GETSIGNALQUALITY/S,\
SS=GETSPECIALSTATS/S,\
SA=GETSTATIONADDRESS/S,\
VE=GETVERSION/S,\
HELP/S";

enum OPT_ARGS
{
	OPT_DEVICE_NAME,
	OPT_DEVICE_UNIT,
	OPT_DEVICEQUERY,
	OPT_NSDEVICEQUERY,
	OPT_GETCRYPTTYPES,
	OPT_GETDNSADDRESS,
	OPT_GETEXTENDEDGLOBALSTATS,
	OPT_GETGLOBALSTATS,
	OPT_GETNETWORKINFO,
	OPT_GETNETWORKS,
	OPT_GETPEERADDRESS,
	OPT_GETRADIOBANDS,
	OPT_GETSIGNALQUALITY,
	OPT_GETSPECIALSTATS,
	OPT_GETSTATIONADDRESS,
	OPT_GETVERSION,
	OPT_HELP,
	OPT_COUNT
};

/******************************************************************************
 * Globals
 ******************************************************************************/

extern struct ExecBase   * SysBase;
extern struct DosLibrary * DOSBase;
extern struct Library    * UtilityBase;

struct RDArgs     * rdargs   = NULL;
struct MsgPort    * SanaPort = NULL;
struct IOSana2Req * SanaReq  = NULL;
struct Device     * SanaDev  = NULL;

static UBYTE StrBand[16];
static UBYTE StrBSSID[(2 * 6) + (6 - 1)];
static UBYTE StrTIMEVAL[LEN_DATSTRING*4];

static STRPTR VERSION = VERSTAG;

/******************************************************************************
 * BinDump()
 ******************************************************************************/

VOID BinDump(ULONG value)
{
	ULONG i;
	UBYTE buf[128];
	UBYTE * p = buf;
	
	for (i = 0; i < 32; i++)
	{
		sprintf(p, "%s ", value & 1 ? "1" : "0");
		p += 2;
		
		if ((i + 1) % 8 == 0)
		{
			sprintf(p, "\n");
			p += 1;
		}
		
		value >>= 1;
	}
	
	Printf(buf);
}

/******************************************************************************
 * HexDump()
 ******************************************************************************/

VOID HexDump(UBYTE * buffer, LONG length)
{
	UBYTE * buf = buffer;
	LONG l2, offset = 0;
	UBYTE linebuf[128];
	ULONG i;
	
	if (!buffer) { printf("NULL\n"); return; }
	if (!length) { printf("EMPTY\n"); return; }
	
	while (length)
	{
		UBYTE * line = linebuf;
		
		l2 = length;
		
		if (l2 > 16)
			l2 = 16;
		
		sprintf(line, "| %08lx |", buf + offset);
		line += 12;
		
		for (i = 0; i < l2; i++)
		{
			sprintf(line, " %02lx", buffer[i]);
			line += 3;
		}
		
		sprintf(line, " | ");
		line += 3;
		
		for (i = 0; i < l2; i++)
		{
			char c = buffer[i];
			sprintf(line, "%c", (c >= 32 && c <= 126) ? c : '.');
			line++;
		}
		
		sprintf(line, " |\n");
		PutStr(linebuf);
		buffer += l2;
		length -= l2;
		offset += l2;
	}
}

/******************************************************************************
 * FormatBand()
 ******************************************************************************/

STRPTR FormatBand(ULONG band)
{
	switch (band) {
	case 0:  strcpy(StrBand, "5 GHz"  ); break;
	case 1:  strcpy(StrBand, "2.4 GHz"); break;
	default: strcpy(StrBand, "? GHz" ); break;
	}
	return StrBand;
}

/******************************************************************************
 * FormatBSSID()
 ******************************************************************************/

STRPTR FormatBSSID(UBYTE * addr)
{
	sprintf(StrBSSID, "%02lx:%02lx:%02lx:%02lx:%02lx:%02lx", 
		addr[0], addr[1], addr[2], 
		addr[3], addr[4], addr[5]);
	
	return StrBSSID;
}

/******************************************************************************
 * FormatTIMEVAL()
 ******************************************************************************/

STRPTR FormatTIMEVAL(struct TimeVal *tv)
{
	static struct DateTime dt;
	static UBYTE strDay[LEN_DATSTRING];
	static UBYTE strDate[LEN_DATSTRING];
	static UBYTE strTime[LEN_DATSTRING];
	
	StrTIMEVAL[0] = 0;
	
	if (tv)
	{
		dt.dat_Format  = FORMAT_INT;
		dt.dat_Flags   = 0;
		dt.dat_StrDay  = strDay;
		dt.dat_StrDate = strDate;
		dt.dat_StrTime = strTime;
		
		dt.dat_Stamp.ds_Days   = (tv->tv_secs / 86400);
		dt.dat_Stamp.ds_Minute = (tv->tv_secs % 86400) / 60;
		dt.dat_Stamp.ds_Tick   = (tv->tv_secs % 60) * 50;
		
		if (DateToStr(&dt))
		{
			sprintf(StrTIMEVAL, "%s %s %s", strDay, strDate, strTime);
		}
	}
	
	return StrTIMEVAL;
}

/******************************************************************************
 * SanaDev_Help()
 ******************************************************************************/

VOID SanaDev_Help(VOID)
{
	PutStr(HELP);
}

/******************************************************************************
 * SanaDev_DeviceVersion()
 ******************************************************************************/

VOID SanaDev_DeviceVersion(VOID)
{
	if (SanaDev)
	{
		struct Library lib = SanaDev->dd_Library;
		
		Printf("\DEVICE VERSION:\n");
		Printf("Device.dd_Library.lib_OpenCnt  = %ld\n", lib.lib_OpenCnt);
		Printf("Device.dd_Library.lib_Version  = %ld\n", lib.lib_Version);
		Printf("Device.dd_Library.lib_Revision = %ld\n", lib.lib_Revision);
		Printf("Device.dd_Library.lib_IdString = %s",    lib.lib_IdString);
	}
}

/******************************************************************************
 * SanaDev_DeviceOpen()
 ******************************************************************************/

VOID SanaDev_DeviceOpen(STRPTR name, ULONG unit, ULONG flags)
{
	if ((SanaPort = CreateMsgPort()) == NULL)
	{
		SanaDev_DeviceClose("[" APP_NAME "] Can't create MsgPort\n", 0);
	}
	
	if ((SanaReq = (struct IOSana2Req *)CreateIORequest(SanaPort, sizeof(struct IOSana2Req))) == NULL)
	{
		SanaDev_DeviceClose("[" APP_NAME "] Can't create IORequest\n", 0);
	}
	
	if (OpenDevice(name, unit, (struct IORequest *)SanaReq, flags) != 0)
	{
		Printf("[" APP_NAME "] Can't open device %s unit %ld", name, unit);
		SanaDev_DeviceError(SanaReq);
		SanaDev_DeviceClose("", 0);
	}
	
	SanaDev = SanaReq->ios2_Req.io_Device;
}

/******************************************************************************
 * SanaDev_DeviceClose()
 ******************************************************************************/

VOID SanaDev_DeviceClose(STRPTR msg, LONG code)
{
	if (msg != NULL)
	{
		Printf("%s: ", APP_NAME);
		PutStr(msg);
	}
	
	if (code != 0)
		PrintFault(code, APP_NAME);
	
	if (rdargs != NULL)
		FreeArgs(rdargs);
	
	if (SanaDev != NULL)
		CloseDevice((struct IORequest *)SanaReq);
	
	if (SanaReq != NULL)
		DeleteIORequest(SanaReq);
	
	if (SanaPort != NULL)
		DeleteMsgPort(SanaPort);
	
	exit(msg == NULL && code == 0 ? RETURN_OK : RETURN_ERROR);
}

/******************************************************************************
 * SanaDev_DeviceError()
 ******************************************************************************/

VOID SanaDev_DeviceError(struct IOSana2Req *req)
{
	PutStr("\n[" APP_NAME "] SANA2 Error: ");
	
	switch (req->ios2_Req.io_Error)
	{
	case IOERR_OPENFAIL:      PutStr("IOERR_OPENFAIL"     ); break;
	case IOERR_ABORTED:       PutStr("IOERR_ABORTED"      ); break;
	case IOERR_NOCMD:         PutStr("IOERR_NOCMD"        ); break;
	case IOERR_BADLENGTH:     PutStr("IOERR_BADLENGTH"    ); break;
	case IOERR_BADADDRESS:    PutStr("IOERR_BADADDRESS"   ); break;
	case IOERR_UNITBUSY:      PutStr("IOERR_UNITBUSY"     ); break;
	case IOERR_SELFTEST:      PutStr("IOERR_SELFTEST"     ); break;
	case S2ERR_NO_ERROR:      PutStr("S2ERR_NO_ERROR"     ); break;
	case S2ERR_NO_RESOURCES:  PutStr("S2ERR_NO_RESOURCES" ); break;
	case S2ERR_BAD_ARGUMENT:  PutStr("S2ERR_BAD_ARGUMENT" ); break;
	case S2ERR_BAD_STATE:     PutStr("S2ERR_BAD_STATE"    ); break;
	case S2ERR_BAD_ADDRESS:   PutStr("S2ERR_BAD_ADDRESS"  ); break;
	case S2ERR_MTU_EXCEEDED:  PutStr("S2ERR_MTU_EXCEEDED" ); break;
	case S2ERR_NOT_SUPPORTED: PutStr("S2ERR_NOT_SUPPORTED"); break;
	case S2ERR_SOFTWARE:      PutStr("S2ERR_SOFTWARE"     ); break;
	case S2ERR_OUTOFSERVICE:  PutStr("S2ERR_OUTOFSERVICE" ); break;
	default:
		Printf("%ld", req->ios2_Req.io_Error);
		break;
	}
	
	if (1) //req->ios2_WireError != S2WERR_GENERIC_ERROR)
    {
		PutStr("\n[" APP_NAME "] SANA2 WireError: ");
		
		switch(req->ios2_WireError)
		{
		case S2WERR_GENERIC_ERROR:   PutStr("S2WERR_GENERIC_ERROR"  ); break;
		case S2WERR_NOT_CONFIGURED:  PutStr("S2WERR_NOT_CONFIGURED" ); break;
		case S2WERR_UNIT_ONLINE:     PutStr("S2WERR_UNIT_ONLINE"    ); break;
		case S2WERR_UNIT_OFFLINE:    PutStr("S2WERR_UNIT_OFFLINE"   ); break;
		case S2WERR_ALREADY_TRACKED: PutStr("S2WERR_ALREADY_TRACKED"); break;
		case S2WERR_NOT_TRACKED:     PutStr("S2WERR_NOT_TRACKED"    ); break;
		case S2WERR_BUFF_ERROR:      PutStr("S2WERR_BUFF_ERROR"     ); break;
		case S2WERR_SRC_ADDRESS:     PutStr("S2WERR_SRC_ADDRESS"    ); break;
		case S2WERR_DST_ADDRESS:     PutStr("S2WERR_DST_ADDRESS"    ); break;
		case S2WERR_BAD_BROADCAST:   PutStr("S2WERR_BAD_BROADCAST"  ); break;
		case S2WERR_BAD_MULTICAST:   PutStr("S2WERR_BAD_MULTICAST"  ); break;
		case S2WERR_MULTICAST_FULL:  PutStr("S2WERR_MULTICAST_FULL" ); break;
		case S2WERR_BAD_EVENT:       PutStr("S2WERR_BAD_EVENT"      ); break;
		case S2WERR_BAD_STATDATA:    PutStr("S2WERR_BAD_STATDATA"   ); break;
		case S2WERR_IS_CONFIGURED:   PutStr("S2WERR_IS_CONFIGURED"  ); break;
		case S2WERR_NULL_POINTER:    PutStr("S2WERR_NULL_POINTER"   ); break;
		default:
			Printf("%ld", req->ios2_WireError);
			break;
		}
	}
	
	PutStr("\n");
}

/******************************************************************************
 * SanaDev_DeviceQuery()
 ******************************************************************************/

VOID SanaDev_DeviceQuery(VOID)
{
	if (SanaDev)
	{
		static struct Sana2DeviceQuery __aligned query;
		
		// Query the SANA2 device
		
		SanaReq->ios2_Req.io_Command = S2_DEVICEQUERY;
		SanaReq->ios2_StatData = &query;
		query.SizeAvailable = sizeof(struct Sana2DeviceQuery);
		
		if (DoIO((struct IORequest *)SanaReq) == S2ERR_NO_ERROR)
		{
			// Success
			
			Printf("\nS2_DEVICEQUERY:\n");
			Printf("%-31s = %ld bytes\n",   "Sana2DeviceQuery.SizeAvailable",  query.SizeAvailable);
			Printf("%-31s = %ld bytes\n",   "Sana2DeviceQuery.SizeSupplied",   query.SizeSupplied);
			Printf("%-31s = %ld\n",         "Sana2DeviceQuery.DevQueryFormat", query.DevQueryFormat);
			Printf("%-31s = %ld\n",         "Sana2DeviceQuery.DeviceLevel",    query.DeviceLevel);
			Printf("%-31s = %ld bits (%ld bytes)\n", "Sana2DeviceQuery.AddrFieldSize",  query.AddrFieldSize,  query.AddrFieldSize / 8);
			Printf("%-31s = %ld bytes\n",   "Sana2DeviceQuery.MTU",            query.MTU);
			Printf("%-31s = %ld Mbits/s\n", "Sana2DeviceQuery.BPS",            query.BPS / 1000000);
			Printf("%-31s = %s\n",          "Sana2DeviceQuery.HardwareType",   query.HardwareType == 1 ? "Ethernet" : "Not-Ethernet");
			
			if (query.SizeSupplied >= query.SizeAvailable)
				Printf("%-31s = %ld bytes\n", "Sana2DeviceQuery.RawMTU", query.RawMTU);
			else
				Printf("%-31s = n/a\n", "Sana2DeviceQuery.RawMTU");
		}
		else
		{
			// Failure
			
			PutStr("[" APP_NAME "] S2_DEVICEQUERY failed");
			SanaDev_DeviceError(SanaReq);
		}
	}
}


/******************************************************************************
 * SanaDev_NSDeviceQuery()
 ******************************************************************************/

static BYTE * NSDEVTYPE_STR[] = {
	"NSDEVTYPE_UNKNOWN",
	"NSDEVTYPE_GAMEPORT",
	"NSDEVTYPE_TIMER",
	"NSDEVTYPE_KEYBOARD",
	"NSDEVTYPE_INPUT",
	"NSDEVTYPE_TRACKDISK",
	"NSDEVTYPE_CONSOLE",
	"NSDEVTYPE_SANA2",
	"NSDEVTYPE_AUDIO",
	"NSDEVTYPE_CLIPBOARD",
	"NSDEVTYPE_PRINTER",
	"NSDEVTYPE_SERIAL",
	"NSDEVTYPE_PARALLEL",
};

VOID SanaDev_NSDeviceQuery(VOID)
{
	if (SanaDev)
	{
		// Query the SANA2 device
		
		static struct NSDeviceQueryResult query;
		
		query.nsdqr_DevQueryFormat    = 0;
		query.nsdqr_SizeAvailable     = sizeof(struct NSDeviceQueryResult) + 4L;
		query.nsdqr_DeviceType        = 0;
		query.nsdqr_DeviceSubType     = 0;
		query.nsdqr_SupportedCommands = NULL;
		
		SanaReq->ios2_Req.io_Command = NSCMD_DEVICEQUERY;
		SanaReq->ios2_Data = &query;
		
		Printf("\DoIO(NSCMD_DEVICEQUERY):\n");
		if (DoIO((struct IORequest *)SanaReq) == S2ERR_NO_ERROR)
		{
			// Success
			
			Printf("\NSCMD_DEVICEQUERY:\n");
			
			Printf("%-31s = %d (%s)\n", "NSDeviceQuery.DeviceType", 
				query.nsdqr_DeviceType, query.nsdqr_DeviceType <= 12 ? 
					NSDEVTYPE_STR[query.nsdqr_DeviceType] : "Unknown DeviceType");
			
			Printf("%-31s = %d\n", "NSDeviceQuery.DeviceSubType", 
				query.nsdqr_DeviceSubType);
			
			Printf("%-31s = 0x%08lx\n", "NSDeviceQuery.SupportedCommands", 
				query.nsdqr_SupportedCommands);
			
			// HexDump(query.nsdqr_SupportedCommands, 512);
		}
		else
		{
			// Failure
			
			PutStr("[" APP_NAME "] NSCMD_DEVICEQUERY failed");
			SanaDev_DeviceError(SanaReq);
		}
	}
}

/******************************************************************************
 * SanaDev_GetCryptTypes()
 ******************************************************************************/

VOID SanaDev_GetCryptTypes(VOID)
{
	// Allocate memory pool
	
	APTR poolHeader = CreatePool(MEMF_PUBLIC|MEMF_CLEAR, POOLPUDDLESIZE, POOLTHRESHSIZE);
	
	if (poolHeader)
	{
		// Query the SANA2 device
		
		SanaReq->ios2_Req.io_Command = S2_GETCRYPTTYPES;
		SanaReq->ios2_Data = poolHeader;
		
		if (DoIO((struct IORequest *)SanaReq) == S2ERR_NO_ERROR)
		{
			UBYTE * types = (UBYTE *)SanaReq->ios2_StatData;
			ULONG i;
			
			Printf("\nS2_GETCRYPTTYPES:\n");
			Printf("%-26s = %ld supported Crypt Types.\n", 
				"IOSana2Req.ios2_DataLength", 
				SanaReq->ios2_DataLength);
			
			Printf("%-26s = 0x%08lx.\n", 
				"IOSana2Req.ios2_Data", 
				SanaReq->ios2_StatData);
			
			for (i = 0; i < SanaReq->ios2_DataLength; i++)
			{
				Printf("IOSana2Req.ios2_Data[%ld]    = ", i);
				switch (types[i]) {
				case S2ENC_NONE: Printf("0 (S2ENC_NONE)"); break;
				case S2ENC_WEP:  Printf("1 (S2ENC_WEP)" ); break;
				case S2ENC_TKIP: Printf("2 (S2ENC_TKIP)"); break;
				case S2ENC_CCMP: Printf("3 (S2ENC_CCMP)"); break;
				default: Printf("S2ENC_?"); break;
				}
				PutStr(".\n");
			}
		}
		else
		{
			Printf("[" APP_NAME "] S2_GETCRYPTTYPES failed");
			SanaDev_DeviceError(SanaReq);
		}
		
		// Release the memory pool
		
		DeletePool(poolHeader);
	}
}

/******************************************************************************
 * SanaDev_GetDnsAddress()
 ******************************************************************************/

VOID SanaDev_GetDnsAddress(VOID)
{
	SanaReq->ios2_Req.io_Command = S2_GETDNSADDRESS;
	SanaReq->ios2_Req.io_Flags = SANA2IOB_QUICK;
	
	if (DoIO((struct IORequest *)SanaReq) == S2ERR_NO_ERROR)
	{
		Printf("\nS2_GETDNSADDRESS:\n");
		
		Printf("%-21s = %s\n", "DnsAddress.Primary", 
			FormatBSSID(SanaReq->ios2_SrcAddr));
		
		Printf("%-21s = %s\n", "DnsAddress.Secondary", 
			FormatBSSID(SanaReq->ios2_DstAddr));
	}
	else
	{
		Printf("[" APP_NAME "] S2_GETDNSADDRESS failed");
		SanaDev_DeviceError(SanaReq);
	}
}

/******************************************************************************
 * SanaDev_GetExtendedGlobalStats()
 ******************************************************************************/

VOID SanaDev_GetExtendedGlobalStats(VOID)
{
	static struct Sana2ExtDeviceStats __aligned stats;
	
	stats.s2xds_Length = sizeof(struct Sana2ExtDeviceStats);
	
	SanaReq->ios2_Req.io_Command = S2_GETEXTENDEDGLOBALSTATS;
	SanaReq->ios2_StatData = &stats;
	
	if (DoIO((struct IORequest *)SanaReq) == S2ERR_NO_ERROR)
	{
		Printf("\nS2_GETEXTENDEDGLOBALSTATS:\n");
		Printf("%-42s = %ld bytes\n",         "Sana2ExtDeviceStats.Length",               stats.s2xds_Length);
		Printf("%-42s = %ld bytes\n",         "Sana2ExtDeviceStats.Actual",               stats.s2xds_Actual);
		Printf("%-42s = %ld %ld packets\n",   "Sana2ExtDeviceStats.PacketsReceived",      stats.s2xds_PacketsReceived.s2q_High, stats.s2xds_PacketsReceived.s2q_Low);
		Printf("%-42s = %ld %ld packets\n",   "Sana2ExtDeviceStats.PacketsSent",          stats.s2xds_PacketsSent.s2q_High, stats.s2xds_PacketsSent.s2q_Low);
		Printf("%-42s = %ld %ld packets\n",   "Sana2ExtDeviceStats.BadData",              stats.s2xds_BadData.s2q_High, stats.s2xds_BadData.s2q_Low);
		Printf("%-42s = %ld %ld hardware misses\n", "Sana2ExtDeviceStats.Overruns",             stats.s2xds_Overruns.s2q_High, stats.s2xds_Overruns.s2q_Low);
		Printf("%-42s = %ld %ld orphans\n",   "Sana2ExtDeviceStats.UnknownTypesReceived", stats.s2xds_UnknownTypesReceived.s2q_High, stats.s2xds_UnknownTypesReceived.s2q_Low);
		Printf("%-42s = %ld %ld orphans\n",   "Sana2ExtDeviceStats.Reconfigurations",     stats.s2xds_Reconfigurations.s2q_High, stats.s2xds_Reconfigurations.s2q_Low);
		Printf("%-42s = %s\n",                "Sana2ExtDeviceStats.LastStart",            FormatTIMEVAL(&stats.s2xds_LastStart));
		Printf("%-42s = %s\n",                "Sana2ExtDeviceStats.LastConnected",        FormatTIMEVAL(&stats.s2xds_LastConnected));
		Printf("%-42s = %s\n",                "Sana2ExtDeviceStats.LastDisconnected",     FormatTIMEVAL(&stats.s2xds_LastDisconnected));
		Printf("%-42s = %s\n",                "Sana2ExtDeviceStats.TimeConnected",        FormatTIMEVAL(&stats.s2xds_TimeConnected));
	}
	else
	{
		Printf("[" APP_NAME "] S2_GETEXTENDEDGLOBALSTATS failed");
		SanaDev_DeviceError(SanaReq);
	}
}

/******************************************************************************
 * SanaDev_GetGlobalStats()
 ******************************************************************************/

VOID SanaDev_GetGlobalStats(VOID)
{
	static struct Sana2DeviceStats __aligned stats;
	
	SanaReq->ios2_Req.io_Command = S2_GETGLOBALSTATS;
	SanaReq->ios2_StatData = &stats;
	
	if (DoIO((struct IORequest *)SanaReq) == S2ERR_NO_ERROR)
	{
		Printf("\nS2_GETGLOBALSTATS:\n");
		Printf("%-37s = %ld packets\n",         "Sana2DeviceStats.PacketsReceived",      stats.PacketsReceived);
		Printf("%-37s = %ld packets\n",         "Sana2DeviceStats.PacketsSent",          stats.PacketsSent);
		Printf("%-37s = %ld packets\n",         "Sana2DeviceStats.BadData",              stats.BadData);
		Printf("%-37s = %ld hardware misses\n", "Sana2DeviceStats.Overruns",             stats.Overruns);
		Printf("%-37s = %ld\n",                 "Sana2DeviceStats.Unused",               stats.Unused);
		Printf("%-37s = %ld orphans\n",         "Sana2DeviceStats.UnknownTypesReceived", stats.UnknownTypesReceived);
		Printf("%-37s = %ld\n",                 "Sana2DeviceStats.Reconfigurations",     stats.Reconfigurations);
		Printf("%-37s = %s\n",                  "Sana2DeviceStats.LastStart",            FormatTIMEVAL(&stats.LastStart));
	}
	else
	{
		PutStr("[" APP_NAME "] S2_GETGLOBALSTATS failed");
		SanaDev_DeviceError(SanaReq);
	}
}

/******************************************************************************
 * SanaDev_GetNetworkInfo()
 ******************************************************************************/

VOID SanaDev_GetNetworkInfo(VOID)
{
	// Allocate memory pool
	
	APTR poolHeader = CreatePool(MEMF_PUBLIC|MEMF_CLEAR, POOLPUDDLESIZE, POOLTHRESHSIZE);
	
	if (poolHeader)
	{
		// Query the SANA2 device
		
		SanaReq->ios2_Req.io_Command = S2_GETNETWORKINFO;
		SanaReq->ios2_Data = poolHeader;
		
		if (DoIO((struct IORequest *)SanaReq) == S2ERR_NO_ERROR)
		{
			Printf("\nS2_GETNETWORKINFO:\n");
			SanaDev_GetNetworkInfoPrint(SanaReq->ios2_StatData);
		}
		else
		{
			PutStr("[" APP_NAME "] S2_GETNETWORKINFO failed");
			SanaDev_DeviceError(SanaReq);
		}
		
		// Release the memory pool
		
		DeletePool(poolHeader);
	}
}

/******************************************************************************
 * SanaDev_GetNetworks()
 ******************************************************************************/

VOID SanaDev_GetNetworks(STRPTR ssid)
{
	// Allocate memory pool
	
	APTR poolHeader = CreatePool(MEMF_PUBLIC|MEMF_CLEAR, POOLPUDDLESIZE, POOLTHRESHSIZE);
	
	if (poolHeader)
	{
		// Input arguments

		static struct TagItem args[4];
		
		args[0].ti_Tag = TAG_DONE;
		
		if (ssid)
		{
			args[0].ti_Tag  = S2INFO_SSID;
			args[0].ti_Data = (ULONG)ssid;
			args[1].ti_Tag  = TAG_DONE;
		}
		
		// Query the SANA2 device
		
		SanaReq->ios2_Req.io_Command = S2_GETNETWORKS;
		SanaReq->ios2_Data = poolHeader;
		SanaReq->ios2_StatData = &args;
		
		if (DoIO((struct IORequest *)SanaReq) == S2ERR_NO_ERROR)
		{
			ULONG i;
			
			APTR * array = (APTR *)SanaReq->ios2_StatData;

			Printf("\nS2_GETNETWORKS:\n");
			
			for (i = 0; i < SanaReq->ios2_DataLength; i++)
			{
				PutStr("\n");
				PutStr("------------------------------------------------\n");
				Printf("Network[%ld]\n", i);
				PutStr("------------------------------------------------\n");
				PutStr("\n");
				SanaDev_GetNetworkInfoPrint(array[i]);
			}
		}
		else
		{
			PutStr("[" APP_NAME "] S2_GETNETWORKS failed");
			SanaDev_DeviceError(SanaReq);
			return;
		}
		
		// Release the memory pool
		
		DeletePool(poolHeader);
	}
}

/******************************************************************************
 * SanaDev_GetNetworkInfoPrint()
 ******************************************************************************/

VOID SanaDev_GetNetworkInfoPrint(APTR tagList)
{
	struct TagItem * tags = (struct TagItem *)tagList;
	
	if (tags != NULL)
	{
		ULONG capabilities = 0;//GetTagData(S2INFO_Capabilities, 0, tags);
		UBYTE * info_elements = 0;//(UBYTE *)GetTagData(S2INFO_InfoElements, 0, tags);
		UBYTE * wpa_info = 0;//(UBYTE *)GetTagData(S2INFO_WPAInfo, 0, tags);
		
		Printf("%-21s = %s\n",       "S2INFO_SSID",           GetTagData(S2INFO_SSID, 0, tags));
		Printf("%-21s = %s\n",       "S2INFO_BSSID",          FormatBSSID((UBYTE *)GetTagData(S2INFO_BSSID, 0, tags)));
		Printf("%-21s = %ld\n",      "S2INFO_AuthTypes",      GetTagData(S2INFO_AuthTypes, 0, tags));
		Printf("%-21s = %ld\n",      "S2INFO_AssocID",        GetTagData(S2INFO_AssocID, 0, tags));
		Printf("%-21s = %ld\n",      "S2INFO_Encryption",     GetTagData(S2INFO_Encryption, 0, tags));
		Printf("%-21s = %ld\n",      "S2INFO_PortType",       GetTagData(S2INFO_PortType, 0, tags));
		Printf("%-21s = %ld\n",      "S2INFO_BeaconInterval", GetTagData(S2INFO_BeaconInterval, 0, tags));
		Printf("%-21s = %ld\n",      "S2INFO_Channel",        GetTagData(S2INFO_Channel, 0, tags));
		Printf("%-21s = %ld dBm\n",  "S2INFO_Signal",         GetTagData(S2INFO_Signal, -90, tags));
		Printf("%-21s = %ld dbM\n",  "S2INFO_Noise",          GetTagData(S2INFO_Noise, -90, tags));
		Printf("%-21s = %ld\n",      "S2INFO_Capabilities",   GetTagData(S2INFO_Capabilities, 0, tags));
		Printf("%-21s = 0x%08lx\n",  "S2INFO_InfoElements",   GetTagData(S2INFO_InfoElements, 0, tags));
		Printf("%-21s = 0x%08lx\n",  "S2INFO_WPAInfo",        GetTagData(S2INFO_WPAInfo, 0, tags));
		Printf("%-21s = %s (%ld)\n", "S2INFO_Band",           FormatBand(GetTagData(S2INFO_Band, 0, tags)), GetTagData(S2INFO_Band, 0, tags));
		Printf("%-21s = %ld\n",      "S2INFO_DefaultKeyNo",   GetTagData(S2INFO_DefaultKeyNo, 0, tags));
		
		if (capabilities)
		{
			PutStr("\nBinDump for S2INFO_Capabilities :\n");
			BinDump(capabilities);
		}
		
		if (info_elements)
		{
			PutStr("\nHexDump for S2INFO_InfoElements :\n");
			HexDump(info_elements, 128);
		}
		
		if (wpa_info)
		{
			PutStr("\nHexDump for S2INFO_WPAInfo :\n");
			HexDump(wpa_info, 128);
		}
	}
}

/******************************************************************************
 * SanaDev_GetPeerAddress()
 ******************************************************************************/

VOID SanaDev_GetPeerAddress(VOID)
{
	SanaReq->ios2_Req.io_Command = S2_GETPEERADDRESS;
	SanaReq->ios2_Req.io_Flags = SANA2IOB_QUICK;
	
	if (DoIO((struct IORequest *)SanaReq) == S2ERR_NO_ERROR)
	{
		Printf("\nS2_GETPEERADDRESS:\n");
		
		Printf("%-40s = %s\n", "PeerAddress.Client", 
			FormatBSSID(SanaReq->ios2_SrcAddr));
		
		Printf("%-40s = %s\n", "PeerAddress.Server", 
			FormatBSSID(SanaReq->ios2_DstAddr));
	}
	else
	{
		Printf("[" APP_NAME "] S2_GETPEERADDRESS failed");
		SanaDev_DeviceError(SanaReq);
	}
}

/******************************************************************************
 * SanaDev_GetRadioBands()
 ******************************************************************************/

VOID SanaDev_GetRadioBands(VOID)
{
	// Allocate memory pool
	
	APTR poolHeader = CreatePool(MEMF_PUBLIC|MEMF_CLEAR, POOLPUDDLESIZE, POOLTHRESHSIZE);
	
	if (poolHeader)
	{
		// Query the SANA2 device
		
		SanaReq->ios2_Req.io_Command = S2_GETRADIOBANDS;
		SanaReq->ios2_Data = poolHeader;
		
		if (DoIO((struct IORequest *)SanaReq) == S2ERR_NO_ERROR)
		{
			UBYTE * types = (UBYTE *)SanaReq->ios2_StatData;
			ULONG i;
			
			Printf("\nS2_GETRADIOBANDS:\n");
			Printf("%-26s = %ld supported Radio Bands.\n", 
				"IOSana2Req.ios2_DataLength", 
				SanaReq->ios2_DataLength);
			
			Printf("%-26s = 0x%08lx.\n", 
				"IOSana2Req.ios2_Data", 
				SanaReq->ios2_StatData);
			
			for (i = 0; i < SanaReq->ios2_DataLength; i++)
			{
				Printf("IOSana2Req.ios2_Data[%ld]    = ", i);
				switch (types[i]) {
				case S2BAND_A: Printf("0 (S2BAND_A)"); break;
				case S2BAND_B: Printf("1 (S2BAND_B)" ); break;
				case S2BAND_G: Printf("2 (S2BAND_G)"); break;
				case S2BAND_N: Printf("3 (S2BAND_N)"); break;
				default: Printf("S2BAND_?"); break;
				}
				PutStr(".\n");
			}
		}
		else
		{
			Printf("[" APP_NAME "] S2_GETRADIOBANDS failed");
			SanaDev_DeviceError(SanaReq);
		}
		
		// Release the memory pool
		
		DeletePool(poolHeader);
	}
}

/******************************************************************************
 * SanaDev_GetSignalQuality()
 ******************************************************************************/

VOID SanaDev_GetSignalQuality(VOID)
{
	static struct Sana2SignalQuality __aligned stat;
	
	SanaReq->ios2_Req.io_Command = S2_GETSIGNALQUALITY;
	SanaReq->ios2_StatData = &stat;
	
	if (DoIO((struct IORequest *)SanaReq) == S2ERR_NO_ERROR)
	{
		LONG level = stat.SignalLevel - stat.NoiseLevel;
		
		Printf("\nS2_GETSIGNALQUALITY:\n");
		Printf("%-30s = %ld dBm\n", "Sana2SignalQuality.SignalLevel", stat.SignalLevel);
		Printf("%-30s = %ld dBm\n", "Sana2SignalQuality.NoiseLevel", stat.NoiseLevel);
		Printf("%-30s = %ld dB ",   "Sana2SignalQuality.Quality", level);
		
		if (level < 0)
			Printf("[------] [Abnormal]");
		else if (level >=  0 && level <=  5)
			Printf("[*-----] [Too low]");
		else if (level >=  6 && level <= 11)
			Printf("[-*----] [Very low]");
		else if (level >= 12 && level <= 17)
			Printf("[--*---] [Low]");
		else if (level >= 18 && level <= 23)
			Printf("[---*--] [Good]");
		else if (level >= 24 && level <= 29)
			Printf("[----*-] [Very good]");
		else if (level >= 30)
			Printf("[-----*] [Excellent]");
		
		Printf("\n");
	}
	else
	{
		PutStr("[" APP_NAME "] S2_GETSIGNALQUALITY failed");
		SanaDev_DeviceError(SanaReq);
	}
}

/******************************************************************************
 * SanaDev_GetSpecialStats()
 ******************************************************************************/

#define RECORDCOUNTMAX 16

struct Sana2SpecialStat
{
    ULONG RecordCountMax;
    ULONG RecordCountSupplied;
    struct Sana2SpecialStatRecord Record[RECORDCOUNTMAX];
};

VOID SanaDev_GetSpecialStats(VOID)
{
	static struct Sana2SpecialStat __aligned stats;
	
	stats.RecordCountMax = RECORDCOUNTMAX;
	
	SanaReq->ios2_Req.io_Command = S2_GETSPECIALSTATS;
	SanaReq->ios2_StatData = &stats;
	
	if (DoIO((struct IORequest *)SanaReq) == S2ERR_NO_ERROR)
	{
		ULONG i;
		
		Printf("\nS2_GETSPECIALSTATS:\n");
		
		Printf("%-36s = %ld records\n", 
			"Sana2SpecialStat.RecordCountMax", 
			stats.RecordCountMax);
		
		Printf("%-36s = %ld records\n", 
			"Sana2SpecialStat.RecordCountSupplied", 
			stats.RecordCountSupplied);
		
		for (i = 0; i < stats.RecordCountSupplied; i++)
		{
			struct Sana2SpecialStatRecord * record = &stats.Record[i];
			
			Printf("Sana2SpecialStat.Record[%02ld].Type     = 0x08%lx (", i, record->Type);
			
			switch (record->Type)
			{
			case S2SS_ETHERNET_BADMULTICAST:
				Printf("S2SS_ETHERNET_BADMULTICAST");
				break;
			case S2SS_ETHERNET_RETRIES:
				Printf("S2SS_ETHERNET_RETRIES");
				break;
			case S2SS_ETHERNET_FIFO_UNDERRUNS:
				Printf("S2SS_ETHERNET_FIFO_UNDERRUNS");
				break;
			default:
				Printf("S2SS_?");
				break;
			}
			
			Printf(")\n");

			Printf("Sana2SpecialStat.Record[%02ld].String   = %s\n", 
				i, record->String);
			
			Printf("Sana2SpecialStat.Record[%02ld].Count    = %ld\n", 
				i, record->Count);
		}
	}
	else
	{
		Printf("[" APP_NAME "] S2_GETSPECIALSTATS failed");
		SanaDev_DeviceError(SanaReq);
	}
}

/******************************************************************************
 * SanaDev_GetStationAddress()
 ******************************************************************************/

VOID SanaDev_GetStationAddress(VOID)
{
	SanaReq->ios2_Req.io_Command = S2_GETSTATIONADDRESS;
	
	if (DoIO((struct IORequest *)SanaReq) == S2ERR_NO_ERROR)
	{
		Printf("\nS2_GETSTATIONADDRESS:\n");
		
		Printf("%-22s = %s\n", "StationAddress.Current", 
			FormatBSSID(SanaReq->ios2_SrcAddr));
		
		Printf("%-22s = %s\n", "StationAddress.Default", 
			FormatBSSID(SanaReq->ios2_DstAddr));
	}
	else
	{
		Printf("[" APP_NAME "] S2_GETSTATIONADDRESS failed");
		SanaDev_DeviceError(SanaReq);
	}
}

/******************************************************************************
 * main()
 ******************************************************************************/

ULONG main(char *cmdline)
{
	static ULONG  args[OPT_COUNT];
	static UBYTE  deviceName[256];
	static STRPTR pDeviceName = DEFAULT_DEVICE_NAME;
	static LONG   deviceUnit = DEFAULT_DEVICE_UNIT;
	static LONG   deviceFlags = 0;
	
	//--------------------------------------------------------
	// Read arguments
	//--------------------------------------------------------
	
	deviceName[0] = '\0';
	memset(args, 0, sizeof(args));
	
	if ((rdargs = ReadArgs(TEMPLATE, (LONG *)args, NULL)) == NULL)
	{
		SanaDev_Help();
		SanaDev_DeviceClose(NULL, IoErr());
	}
	
	if (args[OPT_HELP])
	{
		SanaDev_Help();
		SanaDev_DeviceClose(NULL, 0);
	}
	
	if (args[OPT_DEVICE_UNIT])
	{
		deviceUnit = *((LONG *)args[OPT_DEVICE_UNIT]);
	}
	
	if (args[OPT_DEVICE_NAME])
	{
		pDeviceName = (STRPTR)args[OPT_DEVICE_NAME];
	}
	
	if (!strchr(pDeviceName, ':') && !strchr(pDeviceName, '/'))
	{
		strcpy(deviceName, "networks/");
	}
	
	strcat(deviceName, pDeviceName);
	
	//--------------------------------------------------------
	// Open the SANA2 device
	//--------------------------------------------------------
	
	SanaDev_DeviceOpen(deviceName, deviceUnit, deviceFlags);
	
	//--------------------------------------------------------
	// Proceed arguments
	//--------------------------------------------------------
	
	if (SanaDev != NULL)
	{
		//----------------------------------------------------
		// SANA2 CMD in newstyle.h
		//----------------------------------------------------
		
		if (args[OPT_GETVERSION])
			SanaDev_DeviceVersion();
		
		if (args[OPT_NSDEVICEQUERY])
			 NSDCheck_CheckDevice(deviceName, deviceUnit);
		//	SanaDev_NSDeviceQuery();
		
		//----------------------------------------------------
		// SANA2 CMD in sana2.h
		//----------------------------------------------------
		
		if (args[OPT_DEVICEQUERY])
			SanaDev_DeviceQuery();
		
		if (args[OPT_GETDNSADDRESS])
			SanaDev_GetDnsAddress();
		
		if (args[OPT_GETPEERADDRESS])
			SanaDev_GetPeerAddress();
		
		if (args[OPT_GETSTATIONADDRESS])
			SanaDev_GetStationAddress();
		
		if (args[OPT_GETGLOBALSTATS])
			SanaDev_GetGlobalStats();
		
		if (args[OPT_GETEXTENDEDGLOBALSTATS])
			SanaDev_GetExtendedGlobalStats();
		
		//----------------------------------------------------
		// SANA2 CMD in sana2specialstats.h
		//----------------------------------------------------
		
		if (args[OPT_GETSPECIALSTATS])
			SanaDev_GetSpecialStats();
		
		//----------------------------------------------------
		// SANA2 CMD in sana2wireless.h
		//----------------------------------------------------
		
		if (args[OPT_GETCRYPTTYPES])
			SanaDev_GetCryptTypes();
		
		if (args[OPT_GETNETWORKINFO])
			SanaDev_GetNetworkInfo();
		
		if (args[OPT_GETNETWORKS])
			SanaDev_GetNetworks(NULL);
		
		if (args[OPT_GETRADIOBANDS])
			SanaDev_GetRadioBands();
		
		if (args[OPT_GETSIGNALQUALITY])
			SanaDev_GetSignalQuality();
	}
	
	//----------------------------------------------------
	// Clean Exit
	//----------------------------------------------------
	
	SanaDev_DeviceClose(NULL, 0);
}

/******************************************************************************
 * END OF FILE
 ******************************************************************************/
