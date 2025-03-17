#ifndef __JGET_H__
#define __JGET_H__

#include <exec/types.h>
#include <exec/execbase.h>

#define APP_AUTHOR "Philippe CARPENTIER"
#define APP_VERSTRING "$VER: JGet 1.0 (16.3.2025) [SAS/C 6.59] " APP_AUTHOR
#define APP_HELPSTRING ("Usage: JGet <jsonfile> [<jsonpath>] [<options>]\n\n"\
	" HELP            This help.\n"\
	" FILE            The JSON file to parse (mandatory).\n"\
	" PATH            The path of the JSON value to retrieve (optional).\n"\
	" LIST            List all the JSON paths (optional).\n"\
	" ESCAPESLASHES   Escape slashes in the JSON values (optional).\n"\
	" WITHCOMMENTS    For use with commented JSON files (optional).\n\n"\
	"See JGet.help for a more detailed documentation.\n")

#endif /* __JGET_H__ */
