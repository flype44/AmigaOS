# cdt-prefs

This folder contains **color schemes** for the AmigaOS `C-DataType` by **Stefan Ruppert**. This datatype enables viewing C and C++ source code (`.c`, `.h`, `.cpp`) in `MultiView` with enhanced syntax highlighting.

To be honest, for a program written in 1995, it does in an elegant way, by using a clean and robust `Bison`/`Yacc` implementation, with an efficient grammar, that still is OK nowadays (but could be improved to support the `C99` grammar), also the rendering was well designed by the author, by offering the end-users to customise the preferences file, in `ENV:Datatypes/c.prefs`. That's where this project started, the original archive only provides a basic preferences file, and i wanted to see what some modern well-known color-schemes would looks like with it, particularily on beefed systems. And yeah it does very well! so i thought having a database of nice color-schemes could be useful for the Amiga community, and also might be welcomed for other projects.

## Previews
![Preview](cdt-prefs-1.png?raw=true)
![Preview](cdt-prefs-2.png?raw=true)

## Installation

1. **Install C-DataType**  
   Download from [Aminet](https://aminet.net/util/dtype/cdt_39.15.lha) and follow the installation instructions.

2. **Install cdt-prefs**  
   Download this project's archive (links coming soon) and copy all `cdt_` files to `ENVARC:Datatypes/cdt-prefs`.

3. **Install a Fixed-Width Font (Recommended)**  
   [Dina font](https://aminet.net/text/bfont/Dina.lha) is recommended for best viewing.

## Usage

1. Update your C-DataType preferences:
```sh
C:Copy FROM=ENVARC:Datatypes/cdt-prefs/cdt_atom_onedark.prefs TO=ENV:Datatypes/c.prefs
```

2. Open a C source file in MultiView:
```sh
Run >NIL: Multiview <file> FONTNAME=Dina FONTSIZE=13
```

Or set these as ToolTypes in the MultiView icon.

## About

This project includes a custom `JSON` catalog of color schemes from popular editors and a Python script (`cdt-prefs-make-all.py`) to generate preference files.

You can add new schemes or modify keywords and types as needed.

See `cdt-prefs.json` and the script for details.

The catalog contains a collection of various `Light` and `Dark` color-schemes coming from Atom, GitHub, IntelliJ, KWrite, VSCode, SublimeText, and some other ones.

The `JSON` __schema__ supports all the `C-DataType` preferences, as described in the Datatype documentation (see `cdt_39.15\cdt\doc\c_dtc.doc`).

Then there is an __own-made__ `Python` script (see `cdt-prefs-make-all.py`) to generate all the preference files, one per color-scheme.

The preferences files are already generated and provided in this repository but you can add more to the collection, and/or modify the `user-keywords`, the `user-types`, and the `user-storage` tokens.

The `Python` script as been used and tested under `WSL/Ubuntu`, but can be used on any correctely installed `Python3` setup.

Not mandatory but if you need to regenerate the preferences files, __execute the script__ like this :

### Generate files

```sh
python3 ./cdt-prefs-make-all.py
```

### The output should be :

```sh
Loading catalog cdt-prefs.json...
Loaded.

Catalog informations:
databaseName:           cdt-prefs.json
databaseUrl:            https://aminet.net/util/dtype/cdt_prefs.lha
datatypeUrl:            https://aminet.net/util/dtype/cdt_39.15.lha
destinationFile:        ENV:Datatypes/c.prefs
global_tab_length:      4
global_nested_comments: False
global_debug:           False
user_keywords:          46 items
user_storage:           62 items
user_types:             46 items
catalog_items:          36 items

Exporting catalog items:
Exported: cdt_atom_onelight.prefs
Exported: cdt_atom_onedark.prefs
Exported: cdt_borland_turbovision.prefs
Exported: cdt_cobalt.prefs
Exported: cdt_craftex_nightfall.prefs
Exported: cdt_dracula.prefs
Exported: cdt_dusk.prefs
Exported: cdt_gameboy.prefs
Exported: cdt_github_light.prefs
Exported: cdt_github_light_colorblind.prefs
Exported: cdt_github_light_high_contrast.prefs
Exported: cdt_github_dark.prefs
Exported: cdt_github_dark_colorblind.prefs
Exported: cdt_github_dark_high_contrast.prefs
Exported: cdt_github_dark_dimmed.prefs
Exported: cdt_gruvbox_dark.prefs
Exported: cdt_intellij_light.prefs
Exported: cdt_intellij_dark.prefs
Exported: cdt_kwrite_catppuccin_mocha.prefs
Exported: cdt_midnight.prefs
Exported: cdt_midnight_sea.prefs
Exported: cdt_modus_alt_syntax.prefs
Exported: cdt_monokai.prefs
Exported: cdt_obsidian.prefs
Exported: cdt_plainfocus.prefs
Exported: cdt_qbasic_blue.prefs
Exported: cdt_solarized_light.prefs
Exported: cdt_solarized_dark.prefs
Exported: cdt_sublimetext_monokai.prefs
Exported: cdt_twilight.prefs
Exported: cdt_vscode_light.prefs
Exported: cdt_vscode_dark.prefs
Exported: cdt_vt100_amber.prefs
Exported: cdt_vt100_green.prefs
Exported: cdt_yaru.prefs
Exported: cdt_zenburn.prefs

Done.
```

## Testing

Additionally this C test sample is provided to see the syntax colorisation :

```c
/* C-Language Syntax Highlighting */

#include <stdio.h>
#include "main.h"

#define D(...) \
    KPrintF(__VA_ARGS__)

#ifndef PI
#define PI 3.141592653589793
#endif

static const double pi = PI;

/* struct */
typedef struct Point {
    int x, y; // coordinates
} Point;

/* entry point */
int main(int argc, char ** argv)
{
    while (--argc > 0)
    {
        printf("%s ", argv[argc]);
    }
    
    Point p = { 1, 2 };
    printf("x=%d y=%d\n", p.x, p.y);
    
    return 0;
}
```
