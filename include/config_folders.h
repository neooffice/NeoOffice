/* config_host/config_folders.h.  Generated from config_folders.h.in by configure.  */
/* Configuration of subfolder names in the installation tree. The
 * values of these macros when configured will be strings. The names
 * of these macros reflect what the value would be in an "ideal" world
 * on Linux, not current reality. (For example LIBO_BIN_FOLDER is
 * actually "program" on Linux and most other Unixes.)
 */

#ifndef CONFIG_FOLDERS_H
#define CONFIG_FOLDERS_H

/* where the soffice executable and other end-user-invoked executables are */
#define LIBO_BIN_FOLDER "MacOS"

/* where the *rc / *.ini file are */
#define LIBO_ETC_FOLDER "MacOS"

/* where helper executables run by soffice are */
#define LIBO_LIBEXEC_FOLDER "MacOS"

/* where dynamic libraries loaded directly or programmatically are */
#define LIBO_LIB_FOLDER "MacOS"

/* where read-only resources are in general */
#define LIBO_SHARE_FOLDER "share"

/* where help files are */
#define LIBO_SHARE_HELP_FOLDER "help"

/* where java jars are */
#define LIBO_SHARE_JAVA_FOLDER "MacOS/classes"

/* the presets folder */
#define LIBO_SHARE_PRESETS_FOLDER "presets"

/* LO's own "resources" */
#define LIBO_SHARE_RESOURCE_FOLDER "MacOS/resource"

/* LO's "shell" artwork */
#define LIBO_SHARE_SHELL_FOLDER "MacOS/shell"

/* URE folders */
#define LIBO_URE_BIN_FOLDER "ure-link/bin"
#define LIBO_URE_ETC_FOLDER "ure-link/lib"
#define LIBO_URE_LIB_FOLDER "ure-link/lib"
#define LIBO_URE_SHARE_FOLDER "ure-link/share"
#define LIBO_URE_SHARE_JAVA_FOLDER "ure-link/share/java"

#endif
