# 
# Name:        cdt-prefs-make-all.py
# Version:     1.0
# Description: Preferences files builder for the AmigaOS C-datatype by Stefan Ruppert.
# Usage:       python3 ./cdt-prefs-make-all.py
# Requires:    https://aminet.net/util/dtype/cdt_39.15.lha
#

import os, json

# function
def format_multiline(prefix, items, max_per_line=10):
    lines = []
    for i in range(0, len(items), max_per_line):
        chunk = items[i:i + max_per_line]
        lines.append(f"{prefix} " + " ".join(chunk))
    lines.append("")
    return lines

# Load JSON file
filename = "cdt-prefs.json"
print(f"Loading catalog {filename}...")
with open(filename, "r") as f:
    data = json.load(f)
print(f"Loaded.\n")

databaseName           = data.get("name", "")
databaseUrl            = data.get("databaseUrl", "")
datatypeUrl            = data.get("datatypeUrl", "")
destinationFile        = data.get("destinationFile", "")
global_settings        = data.get("global", {})
global_tab_length      = global_settings.get("tabLength", 4)
global_nested_comments = global_settings.get("nestedComments", True)
global_debug           = global_settings.get("debug", False)
user_keywords          = sorted(data.get("userKeywords", []))
user_storage           = sorted(data.get("userStorage", []))
user_types             = sorted(data.get("userTypes", []))
user_keywords_len      = len(user_keywords)
user_storage_len       = len(user_storage)
user_types_len         = len(user_types)
catalog_items_len      = len(data["catalog"])

# Print catalog informations
print(f"Catalog informations:")
print(f"databaseName:           {databaseName}")
print(f"databaseUrl:            {databaseUrl}")
print(f"datatypeUrl:            {datatypeUrl}")
print(f"destinationFile:        {destinationFile}")
print(f"global_tab_length:      {global_tab_length}")
print(f"global_nested_comments: {global_nested_comments}")
print(f"global_debug:           {global_debug}")
print(f"user_keywords:          {user_keywords_len} items")
print(f"user_storage:           {user_storage_len} items")
print(f"user_types:             {user_types_len} items")
print(f"catalog_items:          {catalog_items_len} items\n")

# Make one file per item in catalog
print(f"Exporting catalog items:")
for theme in data["catalog"]:

    # Make HEADER
    lines = []
    lines.append(f"; HEADER")
    lines.append(f"; cdt_{theme['id']}.prefs")
    lines.append(f"; {theme['name']} color scheme for the AmigaOS C-DataType by Stefan Ruppert")
    lines.append(f"; {theme['description']}")
    lines.append(f"; Save this file to {destinationFile}")
    lines.append(f"; Download and install this Aminet package:")
    lines.append(f"; {datatypeUrl}")
    lines.append(f"; {databaseUrl}\n")

    # Make GLOBAL
    lines.append(f"; GLOBAL")
    global_line = f"GLOBAL TABLENGTH={global_tab_length}"
    if not global_nested_comments:
        global_line += " NONESTEDCOMMENTS"
    if global_debug:
        global_line += " DEBUG"
    lines.append(global_line)
    lines.append("")

    # Make CPART
    lines.append(f"; CPART")
    bgColor = theme.get("bgColor", [0, 0, 0])
    for part in [ "standard", "comment", "cpp", "header", "keyword", "storage", "types", "typename", "string", "number" ]:
        part_key = part.lower().strip()
        style = theme.get(part_key) or theme.get(part_key + " ")
        if not style:
            continue
        fgColor = style.get("color", [255, 255, 255])
        modifiers = []
        if style.get("b", False):
            modifiers.append("BOLD")
        if style.get("u", False):
            modifiers.append("UNDERLINED")
        if style.get("i", False):
            modifiers.append("ITALIC")
        if style.get("text", False):
            modifiers.append("TEXT")
        line = (
            f"CPART={part.upper():<10} "
            f"R={fgColor[0]:<3} G={fgColor[1]:<3} B={fgColor[2]:<3} "
            f"BGR={bgColor[0]:<3} BGG={bgColor[1]:<3} BGB={bgColor[2]:<3}"
        )
        if modifiers:
            line += " " + " ".join(modifiers)
        lines.append(line)
    lines.append("")

    # Make USERTYPES
    if user_types:
        lines.append(f"; USERTYPES")
        lines.extend(format_multiline("USERTYPES", user_types))

    # Make USERKEYWORDS
    if user_keywords:
        lines.append(f"; USERKEYWORDS")
        lines.extend(format_multiline("USERKEYWORDS", user_keywords))

    # Make USERSTORAGE
    if user_storage:
        lines.append(f"; USERSTORAGE")
        lines.extend(format_multiline("USERSTORAGE", user_storage))

    # Write to file
    filename = f"cdt_{theme['id']}.prefs"
    with open(filename, "w") as outf:
        outf.write("\n".join(lines))
    print(f"Exported: {filename}")

print(f"\nDone.")
