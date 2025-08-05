# cdt-prefs

This folder contains __color-schemes__ for the AmigaOS `C-DataType` by __Stefan Ruppert__.

The `C-DataType` offers to view `C-Language` and `CPP-Language` source codes in `MultiView`.

![Preview](https://github.com/flype44/AmigaOS/blob/main/cdt-prefs/cdt-prefs-2.png?raw=true)


# usage on amigaos

This package requires to download and install on your AmigaOS setup the `C-DataType` archive in `Aminet` :

* https://aminet.net/package/util/dtype/cdt_39.15
* https://aminet.net/util/dtype/cdt_39.15.lha

Then, copy all the `cdt_` files of this repository, preferably into the `ENVARC:Datatypes/cdt-prefs` folder.

Then, you can change your `C-DataType` preference file like this :

```bash
C:Copy FROM=ENVARC:Datatypes/cdt-prefs/cdt_atom_onedark.prefs TO=ENV:Datatypes/c.prefs
```

Then, open a `C-Language` or `CPP-Language` (.c or .h or .cpp) source file in `MultiView` :

```bash
Run >NIL: Multiview <file> FONTNAME=Dina FONTSIZE=13
```

Please note, like any source code, it is best-viewed with a `fixed-length` font, such as the `Dina.font`, or any other fixed-font.
https://aminet.net/package/text/bfont/Dina

# more about this project

This project consists of an __own-made__ `JSON` catalog file (see `cdt-prefs.json`) which contains various color-schemes from famous world-wide text-editors.

In this catalog, you will find a collection of various `Light` and/or `Dark` color-schemes as used in :

* Atom, 
* GitHub, 
* IntelliJ, 
* KWrite,
* VSCode, 
* SublimeText,
* And many other ones.

The `JSON` __schema__ supports all the `C-DataType` preferences, as described in the Datatype documentation (see `cdt_39.15\cdt\doc\c_dtc.doc`).

Then there is a `Python` script (see `cdt-prefs-make-all.py`) to generate all the preference files, one per color-scheme.

The preferences files are already generated and provided in this repository but you can add more to the collection, and/or modify the `user-keywords`, `user-types`, and the `user-storage` tokens.

The `Python` script as been used and tested under `WSL/Ubuntu`, but can be used on any correctely installed `Python3` setup.

Not mandatory but if you need to regenerate the preferences files, __execute the script__ like this :

```bash
python3 ./cdt-prefs-make-all.py
```

```bash
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
Exported : cdt_atom_onelight.prefs
Exported : cdt_atom_onedark.prefs
Exported : cdt_borland_turbovision.prefs
Exported : cdt_cobalt.prefs
Exported : cdt_craftex_nightfall.prefs
Exported : cdt_dracula.prefs
Exported : cdt_dusk.prefs
Exported : cdt_gameboy.prefs
Exported : cdt_github_light.prefs
Exported : cdt_github_light_colorblind.prefs
Exported : cdt_github_light_high_contrast.prefs
Exported : cdt_github_dark.prefs
Exported : cdt_github_dark_colorblind.prefs
Exported : cdt_github_dark_high_contrast.prefs
Exported : cdt_github_dark_dimmed.prefs
Exported : cdt_gruvbox_dark.prefs
Exported : cdt_intellij_light.prefs
Exported : cdt_intellij_dark.prefs
Exported : cdt_kwrite_catppuccin_mocha.prefs
Exported : cdt_midnight.prefs
Exported : cdt_midnight_sea.prefs
Exported : cdt_modus_alt_syntax.prefs
Exported : cdt_monokai.prefs
Exported : cdt_obsidian.prefs
Exported : cdt_plainfocus.prefs
Exported : cdt_qbasic_blue.prefs
Exported : cdt_solarized_light.prefs
Exported : cdt_solarized_dark.prefs
Exported : cdt_sublimetext_monokai.prefs
Exported : cdt_twilight.prefs
Exported : cdt_vscode_light.prefs
Exported : cdt_vscode_dark.prefs
Exported : cdt_vt100_amber.prefs
Exported : cdt_vt100_green.prefs
Exported : cdt_yaru.prefs
Exported : cdt_zenburn.prefs

Done.
```
