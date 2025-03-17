/******************************************************************************
 * 
 * File:    JGet.c
 * Short:   Gets a value from a JSON file
 * Author:  Philippe CARPENTIER, aka flype.
 * Date:    March 2025
 * 
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dos/dos.h>
#include <exec/exec.h>
#include <proto/dos.h>
#include <proto/exec.h>

#include "JGet.h"
#include "parson.h"

/******************************************************************************
 * 
 * DEFINES
 * 
 ******************************************************************************/

#define MAXDEPTH   64
#define MAXNAMELEN 256

#define TEMPLATE "H=HELP/S,F=FILE/A,P=PATH,L=LIST/S,E=ESCAPESLASHES/S,W=WITHCOMMENTS/S"

typedef enum {
	OPT_HELP,
	OPT_FILE,
	OPT_PATH,
	OPT_LIST,
	OPT_ESCAPE_SLASHES,
	OPT_WITH_COMMENTS,
	OPT_COUNT
} OPT_ARGS;

/******************************************************************************
 * 
 * GLOBALS
 * 
 ******************************************************************************/

static BOOL   optList = FALSE;
static STRPTR optPath = NULL;
static STRPTR verstring = APP_VERSTRING;
static UBYTE  tokens[MAXDEPTH][MAXNAMELEN];
static UBYTE  tmpPath[MAXDEPTH * MAXNAMELEN];

extern struct ExecBase * SysBase;
extern struct DosLibrary * DOSBase;

/******************************************************************************
 * 
 * PROTOTYPES
 * 
 ******************************************************************************/

VOID JGet_PrintHelp   (VOID);
VOID JGet_PrintValue  (JSON_Value * value);
BOOL JGet_ParseFile   (LONG * options);
VOID JGet_ParseArray  (JSON_Array * array, CONST_STRPTR name, ULONG depth);
VOID JGet_ParseObject (JSON_Object * object, ULONG depth, ULONG index);
VOID JGet_ParseValue  (JSON_Value * value, CONST_STRPTR name, ULONG depth, ULONG index);
BOOL JGet_ParseMatch  (CONST_STRPTR name, ULONG depth, ULONG index);

/******************************************************************************
 * 
 * JGet_PrintHelp()
 * 
 ******************************************************************************/

VOID JGet_PrintHelp(VOID)
{
	printf(APP_HELPSTRING);
}

/******************************************************************************
 * 
 * JGet_PrintValue()
 * 
 ******************************************************************************/

VOID JGet_PrintValue(JSON_Value * value)
{
	STRPTR pretty, start;
	
	if (optList)
		return;
	
	if (start = pretty = json_serialize_to_string_pretty(value))
	{
		size_t length = strlen(start);
		
		if (json_type(value) == JSONString)
		{
			length -= 2;
			start += 1;
		}
		
		printf("%.*s\n", length, start);
		
		json_free_serialized_string(pretty);
	}
}

/******************************************************************************
 * 
 * JGet_ParseMatch()
 * 
 ******************************************************************************/

BOOL JGet_ParseMatch(CONST_STRPTR name, ULONG depth, ULONG index)
{
	ULONG i;
	
	// Build the JSON path
	
	sprintf(tokens[depth], (index == -1) ? "%s" : "%s[%i]", name, index);
	
	tmpPath[0] = '\0';
	
	for (i = 0; i < depth; i++)
	{
		strncat(tmpPath, ".", 1);
		strncat(tmpPath, tokens[i + 1], MAXNAMELEN);
	}
	
	// Display the JSON path
	
	if (optList && depth)
	{
		printf("%s\n", tmpPath);
	}
	
	// JSON path match ?
	
	return (BOOL)(strnicmp(optPath, tmpPath, MAXNAMELEN) == 0);
}

/******************************************************************************
 * 
 * JGet_ParseArray()
 * 
 ******************************************************************************/

VOID JGet_ParseArray(JSON_Array * array, CONST_STRPTR name, ULONG depth)
{
	ULONG i, count = json_array_get_count(array);
	
	for (i = 0; i < count; i++)
	{
		JGet_ParseValue(json_array_get_value(array, i), name, depth, i);
	}
}

/******************************************************************************
 * 
 * JGet_ParseObject()
 * 
 ******************************************************************************/

VOID JGet_ParseObject(JSON_Object * object, ULONG depth, ULONG index)
{
	ULONG i, count = json_object_get_count(object);
	
	for (i = 0; i < count; i++)
	{
		JGet_ParseValue(json_object_get_value_at(object, i), 
			json_object_get_name(object, i), depth + 1, index);
	}
}

/******************************************************************************
 * 
 * JGet_ParseValue()
 * 
 ******************************************************************************/

VOID JGet_ParseValue(JSON_Value * value, CONST_STRPTR name, ULONG depth, ULONG index)
{
	if (JGet_ParseMatch(name, depth, index))
	{
		JGet_PrintValue(value);
	}
	
	switch (json_type(value))
	{
	case JSONArray:
		JGet_ParseArray(json_array(value), name, depth);
		break;
	case JSONObject:
		JGet_ParseObject(json_object(value), depth, -1);
		break;
	}
}

/******************************************************************************
 * 
 * JGet_ParseFile()
 * 
 ******************************************************************************/

BOOL JGet_ParseFile(LONG * opts)
{
	JSON_Value * root;
	
	optList = (BOOL)opts[OPT_LIST];
	optPath = (STRPTR)opts[OPT_PATH];
	
	root = opts[OPT_WITH_COMMENTS] ? 
		json_parse_file_with_comments((STRPTR)opts[OPT_FILE]) :
		json_parse_file((STRPTR)opts[OPT_FILE]);
	
	if (root)
	{
		json_set_escape_slashes(opts[OPT_ESCAPE_SLASHES]);
		JGet_ParseValue(root, NULL, 0, -1);
		json_value_free(root);
		return TRUE;
	}
	
	return FALSE;
}

/******************************************************************************
 * 
 * Entry point
 * 
 ******************************************************************************/

ULONG main(ULONG argc, STRPTR * argv)
{
	ULONG result = RETURN_FAIL;
	LONG opts[OPT_COUNT];
	struct RDArgs * rdArgs;
	
	memset(opts, 0, sizeof(opts));
	
	if (rdArgs = (struct RDArgs *)ReadArgs(TEMPLATE, opts, NULL))
	{
		result = RETURN_WARN;
		
		if (opts[OPT_HELP])
		{
			JGet_PrintHelp();
			
			result = RETURN_OK;
		}
		else
		{
			if (JGet_ParseFile(opts))
			{
				result = RETURN_OK;
			}
		}
		
		FreeArgs(rdArgs);
	}
	else
	{
		JGet_PrintHelp();
	}
	
	return result;
}

/******************************************************************************
 * 
 * End of file
 * 
 ******************************************************************************/
