/*************************************************************************
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU General Public License Version 2.1.
 *
 *
 *    GNU General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 *    Modified May 2006 by Patrick Luby. NeoOffice is distributed under
 *    GPL only under modification term 3 of the LGPL.
 *
 ************************************************************************/

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif

#ifndef _OSL_FILE_H_
#include <osl/file.h>
#endif

#ifdef IRIX
#ifndef _RLD_INTERFACE_DLFCN_H_DLADDR
#define _RLD_INTERFACE_DLFCN_H_DLADDR
typedef struct DL_INFO {
       const char * dli_fname;
       void       * dli_fbase;
       const char * dli_sname;
       void       * dli_saddr;
       int          dli_version;
       int          dli_reserved1;
       long         dli_reserved[4];
} Dl_info;
#endif
#include <rld_interface.h>
#define _RLD_DLADDR             14
int dladdr(void *address, Dl_info *dl);

int dladdr(void *address, Dl_info *dl)
{
       void *v;
       v = _rld_new_interface(_RLD_DLADDR,address,dl);

       return (int)v;
}
#endif

#include "system.h"

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

/* implemented in file.c */
extern int UnicodeToText(char *, size_t, const sal_Unicode *, sal_Int32);

oslModule SAL_CALL osl_psz_loadModule(const sal_Char *pszModuleName, sal_Int32 nRtldMode);
void* SAL_CALL osl_psz_getSymbol(oslModule hModule, const sal_Char* pszSymbolName);

/*****************************************************************************/
/* osl_loadModule */
/*****************************************************************************/

oslModule SAL_CALL osl_loadModule(rtl_uString *ustrModuleName, sal_Int32 nRtldMode)
{
    oslModule pModule=0;
    rtl_uString* ustrTmp = NULL;

    OSL_ENSURE(ustrModuleName,"osl_loadModule : string is not valid");

    /* ensure ustrTmp hold valid string */
    if (osl_File_E_None != osl_getSystemPathFromFileURL(ustrModuleName, &ustrTmp))
        rtl_uString_assign(&ustrTmp, ustrModuleName);

    if (ustrTmp)
    {
        char buffer[PATH_MAX];

        if (UnicodeToText(buffer, PATH_MAX, ustrTmp->buffer, ustrTmp->length))
            pModule = osl_psz_loadModule(buffer, nRtldMode);
    }
    rtl_uString_release(ustrTmp);

    return pModule;
}

/*****************************************************************************/
/* osl_psz_loadModule */
/*****************************************************************************/

oslModule SAL_CALL osl_psz_loadModule(const sal_Char *pszModuleName, sal_Int32 nRtldMode)
{
	sal_Int32 rtld_mode = (nRtldMode == 0) ? RTLD_LAZY : nRtldMode;
	
	if (pszModuleName)
	{
#ifndef NO_DL_FUNCTIONS
		void* pLib = dlopen(pszModuleName, rtld_mode);
#if defined MACOSX && defined INTEL
		/*
		 * HACK: Fix bug 1508 by implementing a workaround for the OOo 2.0.2
		 * bug in tools/inc/solar.h that causes loading of lib*mxp.dylib files
		 * instead of lib*mxi.dylib files on the Intel platform. Note that this
		 * bug has been fixed in post-OOo 2.0.2 releases and so this hack is
		 * only needed for OOo 2.0.2.
		 */
		if (pLib == 0)
		{
			char buffer[PATH_MAX];
			strcpy(buffer, pszModuleName);
			char *pLastFound = buffer;
			while ((pLastFound = strstr(pLastFound, "mxp.dylib")) != NULL)
				pLastFound[2] = 'i';
			pLib = dlopen(buffer, rtld_mode);
		}
#endif	/* MACOSX && INTEL */
		if ((pLib == 0) && (strchr (pszModuleName, '/') == 0))
		{
			/* module w/o pathname not found, try cwd as last chance */
			char buffer[PATH_MAX];

			buffer[0] = '.', buffer[1] = '/', buffer[2] = '\0';
			strncat (buffer, pszModuleName, sizeof(buffer) - 2);

			pLib = dlopen(buffer, rtld_mode);
		}

#if OSL_DEBUG_LEVEL > 1
		if (pLib == 0)
		{
			/* module not found, give up */
			fprintf (stderr, "Error osl_loadModule: %s\n", dlerror());
		}
#endif /* OSL_DEBUG_LEVEL */

		return ((oslModule)(pLib));

#else   /* NO_DL_FUNCTIONS */
		printf("No DL Functions\n");
#endif  /* NO_DL_FUNCTIONS */
	}
	return NULL;
}

/*****************************************************************************/
/* osl_unloadModule */
/*****************************************************************************/
void SAL_CALL osl_unloadModule(oslModule hModule)
{
	if (hModule)
	{
#ifndef NO_DL_FUNCTIONS
#ifndef GCC
        /*     gcc (2.9.1 (egcs), 295) registers atexit handlers for
         *     static destructors which obviously cannot
         *     be called after dlclose. A compiler "feature". The workaround for now
         *     is not to dlclose libraries. Since most of them are closed at shutdown
         *     this does not make that much a difference
         */
        int nRet = dlclose(hModule);
		
#if OSL_DEBUG_LEVEL > 1
        if (nRet != 0)
        {
			fprintf(stderr, "Error osl_unlocadModule: %s\n", dlerror());
        }
#endif /* if OSL_DEBUG_LEVEL */
#endif /* ifndef GCC */

#endif /* ifndef NO_DL_FUNCTIONS */
	}
}

/*****************************************************************************/
/* osl_getSymbol */
/*****************************************************************************/
void* SAL_CALL osl_getSymbol(oslModule Module, rtl_uString* ustrSymbolName)
{
    void* pHandle = 0;

    OSL_ENSURE(Module,"osl_getSymbol : module handle is not valid");
    OSL_ENSURE(Module,"osl_getSymbol : ustrSymbolName");

    if (Module!= 0 && ustrSymbolName != 0)
    {
        rtl_String* strSymbolName = 0;
        sal_Char* pszSymbolName = 0;

        rtl_uString2String( &strSymbolName,
                            rtl_uString_getStr(ustrSymbolName),
                            rtl_uString_getLength(ustrSymbolName),
                            osl_getThreadTextEncoding(),
                            OUSTRING_TO_OSTRING_CVTFLAGS );

        pszSymbolName = rtl_string_getStr(strSymbolName);

        pHandle=osl_psz_getSymbol(Module,pszSymbolName);

        if (strSymbolName != 0)
        {
            rtl_string_release(strSymbolName);
        }
    }
    return pHandle;
}


/*****************************************************************************/
/* osl_psz_getSymbol */
/*****************************************************************************/
void* SAL_CALL osl_psz_getSymbol(oslModule hModule, const sal_Char* pszSymbolName)
{
	if (hModule && pszSymbolName)
	{
#ifndef NO_DL_FUNCTIONS
        void* pSym = dlsym(hModule, pszSymbolName);

#if OSL_DEBUG_LEVEL > 1
		if (!pSym)
			fprintf(stderr, "Error osl_getSymbol: %s\n", dlerror());
#endif
        return pSym;
#endif
	}
	return NULL;
}

/*****************************************************************************/
/* osl_getFunctionSymbol */
/*****************************************************************************/
oslGenericFunction SAL_CALL osl_getFunctionSymbol( oslModule Module, rtl_uString *ustrFunctionSymbolName )
{
    return (oslGenericFunction)osl_getSymbol( Module, ustrFunctionSymbolName);
}

/*****************************************************************************/
/* osl_getModuleURLFromAddress */
/*****************************************************************************/
sal_Bool SAL_CALL osl_getModuleURLFromAddress(void * addr, rtl_uString ** ppLibraryUrl)
{
	sal_Bool result = sal_False;
	Dl_info dl_info;

	if ((result = dladdr(addr, &dl_info)) != 0)
	{
		rtl_uString * workDir = NULL;
		osl_getProcessWorkingDir(&workDir);

#if OSL_DEBUG_LEVEL > 1
		OSL_TRACE("module.c::osl_getModuleURLFromAddress - %s\n", dl_info.dli_fname);
#endif
		rtl_string2UString(ppLibraryUrl, 
						   dl_info.dli_fname, 
						   strlen(dl_info.dli_fname), 
						   osl_getThreadTextEncoding(), 
						   OSTRING_TO_OUSTRING_CVTFLAGS);
		
        OSL_ASSERT(*ppLibraryUrl != NULL);
		osl_getFileURLFromSystemPath(*ppLibraryUrl, ppLibraryUrl); 
		osl_getAbsoluteFileURL(workDir, *ppLibraryUrl, ppLibraryUrl); 

		result = sal_True;
	}
	return result;
}

/*****************************************************************************/
/* osl_getModuleURLFromFunctionAddress */
/*****************************************************************************/
sal_Bool SAL_CALL osl_getModuleURLFromFunctionAddress(oslGenericFunction addr, rtl_uString ** ppLibraryUrl)
{
    return osl_getModuleURLFromAddress((void*)addr, ppLibraryUrl); 
}
