/*************************************************************************
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU General Public License Version 2.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *  
 *  =================================================
 *  Modified June 2003 by Patrick Luby. SISSL Removed. NeoOffice is
 *  distributed under GPL only under modification term 3 of the LGPL.
 *
 *  Contributor(s): _______________________________________
 *
 ************************************************************************/

/*
 *   to do:
 *   - Fix: check for corresponding struct sizes in exported functions
 *   - check size/use of oslDirectory
 *   - check size/use of oslDirectoryItem
 *   - check size/use of oslFileStatus
 *   - check size/use of oslVolumeDeviceHandle
 *   - check size/use of oslVolumeInfo
 *   - check size/use of oslFileHandle
 */

#include "system.h"
#include <rtl/alloc.h>
#include <rtl/memory.h>
#include <rtl/byteseq.h>
#include <rtl/uri.h>
#include <osl/file.h>

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

/*  #include <errno.h> */
/*  #include <unistd.h> */
/*  #include <sys/stat.h> */
/*  #include <fcntl.h> */
/*  #include <stdio.h> */
/*  #include <string.h> */
/*  #include <utime.h> */
/*  #include <limits.h> */
/*  #include <sys/types.h> */
/*  #include <dirent.h> */
/*  #include <rtl/string.h> */


#include <osl/thread.h>
#include <osl/diagnose.h>

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

#include <sys/mman.h>

#ifdef HAVE_STATFS_H
#undef HAVE_STATFS_H
#endif

#include <string.h>
#if defined(SOLARIS)
#include <sys/mnttab.h>
#include <sys/statvfs.h>
#define  HAVE_STATFS_H
#include <sys/fs/ufs_quota.h>
static const sal_Char* MOUNTTAB="/etc/mnttab";

#elif defined(LINUX)
#include <mntent.h>
#include <sys/vfs.h>
#define  HAVE_STATFS_H
#include <sys/quota.h>
#include <ctype.h>
static const sal_Char* MOUNTTAB="/etc/mtab";

#elif defined(NETBSD) || defined(FREEBSD)
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#include <ufs/ufs/quota.h>
#include <ctype.h>
#define  HAVE_STATFS_H
/* No mounting table on *BSD
 * This information is stored only in the kernel. */
/* static const sal_Char* MOUNTTAB="/etc/mtab"; */

#elif defined(IRIX)
#include <mntent.h>
#include <sys/mount.h>
#include <sys/statvfs.h>
#define  HAVE_STATFS_H
#include <sys/quota.h>
#include <ctype.h>
static const sal_Char* MOUNTTAB="/etc/mtab";

#elif defined(MACOSX) 
#include <ufs/ufs/quota.h>
#include <ctype.h>
static const sal_Char* MOUNTTAB="/etc/mtab";

#include <sys/param.h>
#include <sys/mount.h>
#define HAVE_STATFS_H

/* All Mac OS X paths are UTF-8 */
#define osl_getThreadTextEncoding() RTL_TEXTENCODING_UTF8
#endif


#if defined(DEBUG)
/*#define DEBUG_OSL_FILE*/
/*#define TRACE_OSL_FILE*/
extern void debug_ustring(rtl_uString*);
#endif

#ifdef DEBUG_OSL_FILE
    #define PERROR( a, b ) perror( a ); fprintf( stderr, b )
#else
    #define PERROR( a, b )
#endif


/******************************************************************************
 *
 *                  Data Type Definition
 *
 ******************************************************************************/

#if 0
/* FIXME: reintroducing this may save some extra bytes per Item */
typedef struct
{
    rtl_uString* ustrFileName;       /* holds native file name */
    rtl_uString* ustrDirPath;        /* holds native dir path */
    sal_uInt32   RefCount;
} oslDirectoryItemImpl;
#endif

typedef struct
{
    rtl_uString* ustrPath;           /* holds native directory path */
    DIR*         pDirStruct;  
} oslDirectoryImpl;


typedef struct
{
    rtl_uString* ustrFilePath;      /* holds native file path */
    int fd;
} oslFileHandleImpl;


typedef struct _oslVolumeDeviceHandleImpl
{
    sal_Char pszMountPoint[PATH_MAX];
    sal_Char pszFilePath[PATH_MAX];
    sal_Char pszDevice[PATH_MAX];
    sal_Char ident[4];
    sal_uInt32   RefCount;
} oslVolumeDeviceHandleImpl;


/******************************************************************************
 *
 *                  static members
 *
 *****************************************************************************/

static const char * pFileLockEnvVar = (char *) -1;

/******************************************************************************
 *
 *                  C-String Function Declarations
 *
 *****************************************************************************/

static oslFileError osl_psz_getVolumeInformation( sal_Char* , oslVolumeInfo* pInfo, sal_uInt32 uFieldMask );
static oslFileError osl_psz_removeFile( sal_Char* pszPath );
static oslFileError osl_psz_createDirectory( sal_Char* pszPath );
static oslFileError osl_psz_removeDirectory( sal_Char* pszPath );
static oslFileError osl_psz_copyFile( sal_Char* pszPath, sal_Char* pszDestPath );
static oslFileError osl_psz_moveFile(sal_Char* pszPath, sal_Char* pszDestPath);
static oslFileError osl_psz_getCanonicalName( sal_Char* pszRequested, sal_Char* pszValid );
static oslFileError osl_psz_getAbsolutePath(sal_Char* pDirBase, sal_Char* pRelative, sal_Char* pszAbsolute);
static oslFileError osl_psz_setFileAttributes( sal_Char* pszFilePath, sal_uInt64 uAttributes );
static oslFileError osl_psz_setFileTime( sal_Char* strFilePath, TimeValue* pCreationTime, TimeValue* pLastAccessTime, TimeValue* pLastWriteTime );


/******************************************************************************
 *
 *                  Static Module Utility Function Declarations
 *
 *****************************************************************************/

static oslFileError  oslIsMountPoint( sal_Char* pszFileName, sal_Bool* bMountPoint );
static sal_Char*     oslSeparatePathEntry( sal_Char* pszUNCPath, sal_Char cSeparator );

static void          oslMakeUniqueName( sal_Char* pszRequested, sal_Char* pszUnique, sal_uInt32 nIndex );

static oslFileError  osl_psz_CheckForExistence(sal_Char* pszFilePath);

static oslFileError  oslDoCopy( sal_Char* pszSourceFileName, sal_Char* pszDestFileName, mode_t nMode, size_t nSourceSize, int DestFileExists );
static oslFileError  oslChangeFileModes( sal_Char* pszFileName, mode_t nMode, time_t nAcTime, time_t nModTime, uid_t nUID, gid_t nGID );
static int           oslDoCopyLink( sal_Char* pszSourceFileName, sal_Char* pszDestFileName );
static int           oslDoCopyFile( sal_Char* pszSourceFileName, sal_Char* pszDestFileName, size_t nSourceSize, mode_t mode );

static oslFileError  oslDoMoveFile(sal_Char* pszPath, sal_Char* pszDestPath);


static sal_Char*     oslMakePszFromUStr(rtl_uString* uStr, sal_Char* pszStr);
static rtl_uString*  oslMakeUStrFromPsz(sal_Char* pszStr,rtl_uString** uStr);

static char * ImplSearchPath( char *, size_t, const char *, const char *, char );

static oslFileError oslTranslateFileError( int nErr );

static sal_Char*            oslGetBaseName( sal_Char* pszPath );
static sal_Char*            oslGetDirName( sal_Char* pszPath );

#if defined(SOLARIS)
/* FIXME: mfe: these should be replaced */
static sal_Char*     osl_getBaseName(sal_Char* pszPath);
static sal_Char*     osl_getDirName(sal_Char* pszPath);
#endif


/* FIXME: mfe: this function is deprecated */
static sal_Char*     oslDoGetAbsolutePath( sal_Char* pszDirBase, sal_Char* pszRelative );

#if 0
static oslFileError  oslCheckForExistence( rtl_uString* strFilePath );
#endif

/******************************************************************************
 *
 *                  Non-Static Utility Function Declarations
 *
 *****************************************************************************/

int UnicodeToText( char *, size_t, const sal_Unicode *, sal_Int32 );
oslFileError FileURLToPath( char *, size_t, rtl_uString* );


/******************************************************************************
 *
 *                  'removeable device' aka floppy functions
 *
 *****************************************************************************/

static oslVolumeDeviceHandle  osl_isFloppyDrive(const sal_Char* pszPath);
static oslFileError   osl_mountFloppy(oslVolumeDeviceHandle hFloppy);
static oslFileError   osl_unmountFloppy(oslVolumeDeviceHandle hFloppy);


#if defined(SOLARIS)
static sal_Bool       osl_isFloppyMounted(sal_Char* pszPath, sal_Char* pszMountPath);
static sal_Bool       osl_getFloppyMountEntry(const sal_Char* pszPath, sal_Char* pBuffer);
static sal_Bool       osl_checkFloppyPath(sal_Char* pszPath, sal_Char* pszFilePath, sal_Char* pszDevicePath);
#endif

#if defined(LINUX)
static sal_Bool       osl_isFloppyMounted(oslVolumeDeviceHandleImpl* pDevice);
static sal_Bool       osl_getFloppyMountEntry(const sal_Char* pszPath, oslVolumeDeviceHandleImpl* pItem);
#endif


#if defined(IRIX)
static sal_Bool       osl_isFloppyMounted(oslVolumeDeviceHandleImpl* pDevice);
static sal_Bool       osl_getFloppyMountEntry(const sal_Char* pszPath, oslVolumeDeviceHandleImpl* pItem);
#endif

#ifdef DEBUG_OSL_FILE
static void           osl_printFloppyHandle(oslVolumeDeviceHandleImpl* hFloppy);
#endif


/******************************************************************************
 *
 *                  Exported Module Functions
 *
 *****************************************************************************/

/* a slightly modified version of Pchar in rtl/source/uri.c */
static const sal_Bool const uriCharClass[128] =  
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* Pchar but without encoding slashes */
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* !"#$%&'()*+,-./*/
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, /*0123456789:;<=>?*/
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*@ABCDEFGHIJKLMNO*/
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, /*PQRSTUVWXYZ[\]^_*/
      0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /*`abcdefghijklmno*/
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0  /*pqrstuvwxyz{|}~ */
    };


/* check for top wrong usage strings */
/*
static sal_Bool findWrongUsage( const sal_Unicode *path, sal_Int32 len )
{
    rtl_uString *pTmp = NULL;
    sal_Bool bRet;
    
    rtl_uString_newFromStr_WithLength( &pTmp, path, len );
    
    rtl_ustr_toAsciiLowerCase_WithLength( pTmp->buffer, pTmp->length );
        
    bRet = ( 0 == rtl_ustr_ascii_shortenedCompare_WithLength( pTmp->buffer, pTmp->length, "ftp://", 6 ) ) ||
           ( 0 == rtl_ustr_ascii_shortenedCompare_WithLength( pTmp->buffer, pTmp->length, "http://", 7 ) ) ||
           ( 0 == rtl_ustr_ascii_shortenedCompare_WithLength( pTmp->buffer, pTmp->length, "vnd.sun.star", 12 ) ) ||
           ( 0 == rtl_ustr_ascii_shortenedCompare_WithLength( pTmp->buffer, pTmp->length, "private:", 8 ) ) ||
           ( 0 == rtl_ustr_ascii_shortenedCompare_WithLength( pTmp->buffer, pTmp->length, "slot:", 5) );
           
    rtl_uString_release( pTmp );
    return bRet;
}
*/

/****************************************************************************/
/*	osl_getSystemPathFromFileURL */
/****************************************************************************/

oslFileError SAL_CALL osl_getSystemPathFromFileURL( rtl_uString *ustrFileURL, rtl_uString **pustrSystemPath )
{
    sal_Int32 nIndex = 0;
    rtl_uString * pTmp = NULL;

    sal_Unicode encodedSlash[3] = { '%', '2', 'F' };

    /* temporary hack: if already system path, return ustrFileURL */
	/*
    if( (sal_Unicode) '/' == ustrFileURL->buffer[0] ) 
    {
        OSL_ENSURE( 0, "osl_getSystemPathFromFileURL: input is already system path" );
        rtl_uString_assign( pustrSystemPath, ustrFileURL );
        return osl_File_E_None;
    }
	*/
        
    /* a valid file url may not start with '/' */
    if( ( 0 == ustrFileURL->length ) || ( (sal_Unicode) '/' == ustrFileURL->buffer[0] ) )
    {
        return osl_File_E_INVAL;
    }

    /* search for encoded slashes (%2F) and decode every single token if we find one */
    if( -1 != rtl_ustr_indexOfStr_WithLength( ustrFileURL->buffer, ustrFileURL->length, encodedSlash, 3 ) )
    {
        rtl_uString * ustrPathToken = NULL;
        sal_Int32 nOffset = 7;
        
        do
        {
            nOffset += nIndex;

            /* break url down in '/' devided tokens tokens */
            nIndex = rtl_ustr_indexOfChar_WithLength( ustrFileURL->buffer + nOffset, ustrFileURL->length - nOffset, (sal_Unicode) '/' );
            
            /* copy token to new string */
            rtl_uString_newFromStr_WithLength( &ustrPathToken, ustrFileURL->buffer + nOffset, 
                -1 == nIndex ? ustrFileURL->length - nOffset : nIndex++ );

            /* decode token */
            rtl_uriDecode( ustrPathToken, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8, &pTmp );
            
            /* the result should not contain any '/' */
            if( -1 != rtl_ustr_indexOfChar_WithLength( pTmp->buffer, pTmp->length, (sal_Unicode) '/' ) )
            {
                rtl_uString_release( pTmp );
                rtl_uString_release( ustrPathToken );

                return osl_File_E_INVAL;
            }
                
        } while( -1 != nIndex );

        /* release temporary string and restore index variable */
        rtl_uString_release( ustrPathToken );
        nIndex = 0;        
    }
 
    /* protocol and server should not be encoded, so decode the whole string */
    rtl_uriDecode( ustrFileURL, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8, &pTmp );
    
    /* check if file protocol specified */    
    /* FIXME: use rtl_ustr_ascii_shortenedCompareIgnoreCase_WithLength when available */
    if( 7 <= pTmp->length )
    {
        rtl_uString * pProtocol = NULL;
        rtl_uString_newFromStr_WithLength( &pProtocol, pTmp->buffer, 7 );
        
        /* protocol is case insensitive */
        rtl_ustr_toAsciiLowerCase_WithLength( pProtocol->buffer, pProtocol->length );
        
        if( 0 == rtl_ustr_ascii_shortenedCompare_WithLength( pProtocol->buffer, pProtocol->length,"file://", 7 ) )
            nIndex = 7;
            
        rtl_uString_release( pProtocol );
    }
     
    /* skip "localhost" or "127.0.0.1" if "file://" is specified */
    /* FIXME: use rtl_ustr_ascii_shortenedCompareIgnoreCase_WithLength when available */
    if( nIndex && ( 10 <= pTmp->length - nIndex ) )
    {
        rtl_uString * pServer = NULL;
        rtl_uString_newFromStr_WithLength( &pServer, pTmp->buffer + nIndex, 10 );

        /* server is case insensitive */
        rtl_ustr_toAsciiLowerCase_WithLength( pServer->buffer, pServer->length );
        
        if( ( 0 == rtl_ustr_ascii_shortenedCompare_WithLength( pServer->buffer, pServer->length,"localhost/", 10 ) ) ||
            ( 0 == rtl_ustr_ascii_shortenedCompare_WithLength( pServer->buffer, pServer->length,"127.0.0.1/", 10 ) ) )
        {
            /* don't exclude the '/' */
            nIndex += 9;
        }

        rtl_uString_release( pServer );
    }
                
    if( nIndex )
        rtl_uString_newFromStr_WithLength( &pTmp, pTmp->buffer + nIndex, pTmp->length - nIndex );

    /* check if system path starts with ~ or ~user and replace it with the appropriate home dir */
    if( (sal_Unicode) '~' == pTmp->buffer[0] )
    {
        /* check if another user is specified */
        if( ( 1 == pTmp->length ) || ( (sal_Unicode)'/' == pTmp->buffer[1] ) )
        {
            rtl_uString *pTmp2 = NULL;

            /* osl_getHomeDir returns file URL */
            osl_getHomeDir( osl_getCurrentSecurity(), &pTmp2 );

            /* remove "file://" prefix */
            rtl_uString_newFromStr_WithLength( &pTmp2, pTmp2->buffer + 7, pTmp2->length - 7 );

            /* replace '~' in original string */             
            rtl_uString_newReplaceStrAt( &pTmp, pTmp, 0, 1, pTmp2 );
            rtl_uString_release( pTmp2 );
        }

        else
        {
            /* FIXME: replace ~user with users home directory */
            return osl_File_E_INVAL;
        }
    }

    /* temporary check for top 5 wrong usage strings (which are valid but unlikly filenames) */
	/*
    OSL_ASSERT( !findWrongUsage( pTmp->buffer, pTmp->length ) );
	*/
        
    *pustrSystemPath = pTmp;
    return osl_File_E_None;
}

/****************************************************************************/
/*	osl_getFileURLFromSystemPath */
/****************************************************************************/

oslFileError SAL_CALL osl_getFileURLFromSystemPath( rtl_uString *ustrSystemPath, rtl_uString **pustrFileURL )
{
    static const sal_Unicode pDoubleSlash[2] = { '/', '/' };
    
    rtl_uString *pTmp = NULL;
    sal_Int32 nIndex;

    if( 0 == ustrSystemPath->length )
        return osl_File_E_INVAL;

    /* temporary hack: if already file url, return ustrSystemPath */
	
    if( 0 == rtl_ustr_ascii_shortenedCompare_WithLength( ustrSystemPath->buffer, ustrSystemPath->length,"file:", 5 ) )
    {
	/*
        if( 0 == rtl_ustr_ascii_shortenedCompare_WithLength( ustrSystemPath->buffer, ustrSystemPath->length,"file://", 7 ) )
        {
            OSL_ENSURE( 0, "osl_getFileURLFromSystemPath: input is already file URL" );
            rtl_uString_assign( pustrFileURL, ustrSystemPath );
        }
        else
        {
            rtl_uString *pTmp2 = NULL;

            OSL_ENSURE( 0, "osl_getFileURLFromSystemPath: input is wrong file URL" );
            rtl_uString_newFromStr_WithLength( pustrFileURL, ustrSystemPath->buffer + 5, ustrSystemPath->length - 5 );
            rtl_uString_newFromAscii( &pTmp2, "file://" );
            rtl_uString_newConcat( pustrFileURL, *pustrFileURL, pTmp2 );
            rtl_uString_release( pTmp2 );
        }
        return osl_File_E_None;
		*/
		return osl_File_E_INVAL;
    }
	
        
    /* check if system path starts with ~ or ~user and replace it with the appropriate home dir */
    if( (sal_Unicode) '~' == ustrSystemPath->buffer[0] )
    {
        /* check if another user is specified */
        if( ( 1 == ustrSystemPath->length ) || ( (sal_Unicode)'/' == ustrSystemPath->buffer[1] ) )
        {
            /* osl_getHomeDir returns file URL */
            osl_getHomeDir( osl_getCurrentSecurity(), &pTmp );

            /* remove "file://" prefix */
            rtl_uString_newFromStr_WithLength( &pTmp, pTmp->buffer + 7, pTmp->length - 7 );

            /* replace '~' in original string */             
            rtl_uString_newReplaceStrAt( &pTmp, ustrSystemPath, 0, 1, pTmp );
        }

        else
        {
            /* FIXME: replace ~user with users home directory */
            return osl_File_E_INVAL;
        }
    }
    
    /* check if initial string contains double instances of '/' */ 
    nIndex = rtl_ustr_indexOfStr_WithLength( ustrSystemPath->buffer, ustrSystemPath->length, pDoubleSlash, 2 );
    if( -1 != nIndex )
    {
        sal_Int32 nSrcIndex;
        sal_Int32 nDeleted = 0;
        
        /* if pTmp is not already allocated, copy ustrSystemPath for modification */
        if( NULL == pTmp )
            rtl_uString_newFromString( &pTmp, ustrSystemPath );
        
        /* adapt index to pTmp */
        nIndex += pTmp->length - ustrSystemPath->length;
        
        /* remove all occurances of '//' */
        for( nSrcIndex = nIndex + 1; nSrcIndex < pTmp->length; nSrcIndex++ )
        {
            if( ((sal_Unicode) '/' == pTmp->buffer[nSrcIndex]) && ((sal_Unicode) '/' == pTmp->buffer[nIndex]) )
                nDeleted++;
            else
                pTmp->buffer[++nIndex] = pTmp->buffer[nSrcIndex];
        }
        
        /* adjust length member */
        pTmp->length -= nDeleted;
    }               
    
    if( NULL == pTmp )
        rtl_uString_assign( &pTmp, ustrSystemPath ); 

    /* temporary check for top 5 wrong usage strings (which are valid but unlikly filenames) */
	/*
    OSL_ASSERT( !findWrongUsage( pTmp->buffer, pTmp->length ) );
	*/
        
    /* file URLs must be URI encoded */
    rtl_uriEncode( pTmp, uriCharClass, rtl_UriEncodeIgnoreEscapes, RTL_TEXTENCODING_UTF8, pustrFileURL );
    
    rtl_uString_release( pTmp );    
    
    /* absolute urls should start with 'file://' */
    if( (sal_Unicode)'/' == (*pustrFileURL)->buffer[0] )
    {
        rtl_uString *pProtocol = NULL;
        
        rtl_uString_newFromAscii( &pProtocol, "file://" );
        rtl_uString_newConcat( pustrFileURL, pProtocol, *pustrFileURL );
        rtl_uString_release( pProtocol );
    }

    return osl_File_E_None;
}


/****************************************************************************/
/*	osl_openDirectory */
/****************************************************************************/

oslFileError SAL_CALL osl_openDirectory( rtl_uString* ustrDirectoryURL, oslDirectory* pDirectory )
{
    rtl_uString* ustrSystemPath = NULL;
    oslFileError eRet;

    char path[PATH_MAX];

    OSL_ASSERT( ustrDirectoryURL );
    OSL_ASSERT( pDirectory );

    if( 0 == ustrDirectoryURL->length )
        return osl_File_E_INVAL;
 
    /* convert file URL to system path */ 
    eRet = osl_getSystemPathFromFileURL( ustrDirectoryURL, &ustrSystemPath );
    if( osl_File_E_None != eRet )
        return eRet;

    /* remove trailing '/' if any */
    if( ( 1 < ustrSystemPath->length ) && ( (sal_Unicode) '/' == ustrSystemPath->buffer[ustrSystemPath->length - 1] ) )
    {
        ustrSystemPath->length--;
        ustrSystemPath->buffer[ustrSystemPath->length] = 0;
    }

    /* FIXME: should be checked in osl_getSystemPathFromFileURL */
    if( (sal_Unicode) '/' != ustrSystemPath->buffer[0] )
    {
        rtl_uString_release( ustrSystemPath );
        return osl_File_E_INVAL;
    }

    /* convert unicode path to text */
    if( UnicodeToText( path, PATH_MAX, ustrSystemPath->buffer, ustrSystemPath->length ) )
    {
        /* open directory */ 
        DIR *pdir = opendir( path );

        if( pdir )
        {
            /* create and initialize impl structure */
            oslDirectoryImpl* pDirImpl = (oslDirectoryImpl*) rtl_allocateMemory( sizeof(oslDirectoryImpl) );
            
            if( pDirImpl )
            {
                pDirImpl->pDirStruct = pdir;
                pDirImpl->ustrPath = ustrSystemPath; 
                
                *pDirectory = (oslDirectory) pDirImpl;
                return osl_File_E_None;
            }
            else
            {
                errno = ENOMEM;
                closedir( pdir );
            }
        }
        else
            /* should be removed by optimizer in product version */
            PERROR( "osl_openDirectory", path );
    }
            
    rtl_uString_release( ustrSystemPath );
    return oslTranslateFileError(errno);
}        

/****************************************************************************/
/*	osl_closeDirectory */
/****************************************************************************/

oslFileError SAL_CALL osl_closeDirectory( oslDirectory Directory )
{
    oslDirectoryImpl* pDirImpl = (oslDirectoryImpl*) Directory;
    oslFileError err = osl_File_E_None;
    
    OSL_ASSERT( Directory );

    if( NULL == pDirImpl )
        return osl_File_E_INVAL;

    /* close directory */
    if( closedir( pDirImpl->pDirStruct ) )
    {
#ifdef OSL_FILE_DEBUG
        char path[PATH_MAX];    

        UnicodeToText( path, PATH_MAX, ustrSystemPath->buffer, ustrSystemPath->length );

        perror( "osl_closeDirectory" );
        fprintf( stderr, path );
#endif
        err = oslTranslateFileError( errno );
    } 
                    
    /* cleanup members */
    rtl_uString_release( pDirImpl->ustrPath );
    rtl_freeMemory( pDirImpl );

    return err;
}


/****************************************************************************/
/*	osl_getNextDirectoryItem */
/****************************************************************************/

oslFileError SAL_CALL osl_getNextDirectoryItem( oslDirectory Directory, oslDirectoryItem* pItem, sal_uInt32 uHint )
{
    oslDirectoryImpl* pDirImpl = (oslDirectoryImpl *) Directory;
    rtl_uString* ustrFileName  = NULL;
    rtl_uString* ustrFilePath  = NULL;
    sal_Int32 capacity;

    struct dirent* pEntry;
    
    OSL_ASSERT( Directory );
    OSL_ASSERT( pItem );
    
    if( ( NULL == Directory ) || ( NULL == pItem ) )
        return osl_File_E_INVAL;
 
    /* FIXME: use readdir_r where available */
    pEntry = readdir( pDirImpl->pDirStruct ); 
    while( pEntry != NULL && pEntry->d_name[0] == '.' && ( pEntry->d_name[1] == '.' || pEntry->d_name[1] == 0 ) )
         pEntry = readdir( pDirImpl->pDirStruct );

    if( NULL == pEntry )
        return osl_File_E_NOENT;

    /* convert file name to unicode */
    rtl_string2UString( &ustrFileName, pEntry->d_name, strlen( pEntry->d_name ),
        osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );
        
    /* concatinate directory and file name */
    capacity = pDirImpl->ustrPath->length + ustrFileName->length + 1;
    rtl_uString_new_WithLength( &ustrFilePath, capacity );
    
    rtl_uStringbuffer_insert( &ustrFilePath, &capacity, 0, pDirImpl->ustrPath->buffer, pDirImpl->ustrPath->length );
    rtl_uStringbuffer_insert_ascii( &ustrFilePath, &capacity, ustrFilePath->length, "/", 1 );
    rtl_uStringbuffer_insert( &ustrFilePath, &capacity, ustrFilePath->length, ustrFileName->buffer, ustrFileName->length );
    
    rtl_uString_release( ustrFileName );
    
    /* use path as directory item */
    *pItem = (oslDirectoryItem) ustrFilePath;

    return osl_File_E_None;
}

/****************************************************************************/
/*	osl_getDirectoryItem */
/****************************************************************************/

oslFileError SAL_CALL osl_getDirectoryItem( rtl_uString* ustrFileURL, oslDirectoryItem* pItem )
{
    rtl_uString* ustrSystemPath = NULL;
    oslFileError eRet = osl_File_E_INVAL;
    
    char path[PATH_MAX];
    
    OSL_ASSERT( ustrFileURL );
    OSL_ASSERT( pItem );

    if( 0 == ustrFileURL->length )
        return osl_File_E_INVAL;
    
    /* convert file URL to system path */ 
    eRet = osl_getSystemPathFromFileURL( ustrFileURL, &ustrSystemPath );
    if( osl_File_E_None != eRet )
        return eRet;

    /* remove trailing '/' if any */
    if( ( 1 < ustrSystemPath->length ) && ( (sal_Unicode) '/' == ustrSystemPath->buffer[ustrSystemPath->length - 1] ) )
    {
        ustrSystemPath->length--;
        ustrSystemPath->buffer[ustrSystemPath->length] = 0;
    }

    /* FIXME: should be checked in osl_getSystemPathFromFileURL */
    if( (sal_Unicode) '/' != ustrSystemPath->buffer[0] )
    {
        rtl_uString_release( ustrSystemPath );
        return osl_File_E_INVAL;
    }
    
    /* convert unicode path to text */
    if( UnicodeToText( path, PATH_MAX, ustrSystemPath->buffer, ustrSystemPath->length ) )
    {
        if ( 0 == access( path, F_OK ) )
        {
            /* use system path as directory item */
            *pItem = (oslDirectoryItem) ustrSystemPath;

            return osl_File_E_None;
        }
        else
            eRet = oslTranslateFileError( errno );
    }
    
    rtl_uString_release( ustrSystemPath );
    return eRet;
}


/****************************************************************************/
/*	osl_acquireDirectoryItem */
/****************************************************************************/

oslFileError osl_acquireDirectoryItem( oslDirectoryItem Item )
{
    rtl_uString* ustrFilePath = (rtl_uString *) Item;

    OSL_ASSERT( Item );
    
    if( ustrFilePath )
        rtl_uString_acquire( ustrFilePath );
        
    return osl_File_E_None;
}
    
/****************************************************************************/
/*	osl_releaseDirectoryItem */
/****************************************************************************/

oslFileError osl_releaseDirectoryItem( oslDirectoryItem Item )
{
    rtl_uString* ustrFilePath = (rtl_uString *) Item;

    OSL_ASSERT( Item );
    
    if( ustrFilePath )
        rtl_uString_release( ustrFilePath );
    
    return osl_File_E_None;
}

/*****************************************
 is_query_extended_file_attributes
 check if extended file attributes are
 queried that require a lstat call
 ****************************************/
 
static int is_query_extended_file_attributes(sal_uInt32 field_mask)
{
    return ((field_mask & osl_FileStatus_Mask_Type) ||
			(field_mask & osl_FileStatus_Mask_Attributes) ||
			(field_mask & osl_FileStatus_Mask_CreationTime) ||
			(field_mask & osl_FileStatus_Mask_AccessTime) ||
			(field_mask & osl_FileStatus_Mask_ModifyTime) ||
			(field_mask & osl_FileStatus_Mask_FileSize) ||
			(field_mask & osl_FileStatus_Mask_LinkTargetURL) ||
			(field_mask & osl_FileStatus_Mask_Validate));                
}

/****************************************************************************/
/*	osl_getFileStatus */
/****************************************************************************/

oslFileError SAL_CALL osl_getFileStatus( oslDirectoryItem Item, oslFileStatus* pStat, sal_uInt32 uFieldMask )
{
    rtl_uString* ustrFilePath = (rtl_uString *) Item;
    
    char path[PATH_MAX];

    OSL_ASSERT( Item );
    OSL_ASSERT( pStat );
    
    if( ( NULL == Item ) || ( NULL == pStat ) )
        return osl_File_E_INVAL;
        
    if( 0 == ustrFilePath->length )
        return osl_File_E_INVAL;
    
    /* convert unicode path to text */
    if( ! UnicodeToText( path, PATH_MAX, ustrFilePath->buffer, ustrFilePath->length ) )
        return osl_File_E_INVAL;

    /* mark all fields as invalid */
    pStat->uValidFields = 0;

    /* urls have to be encoded */
    if (uFieldMask & osl_FileStatus_Mask_FileURL)
    {
        if (osl_File_E_None == osl_getFileURLFromSystemPath(ustrFilePath, &pStat->ustrFileURL))
            pStat->uValidFields |= osl_FileStatus_Mask_FileURL;
    }

    /* file name is the last part of the path */
    if (uFieldMask & osl_FileStatus_Mask_FileName)
    {
        sal_uInt32 nIndex = rtl_ustr_lastIndexOfChar_WithLength(ustrFilePath->buffer, ustrFilePath->length, '/');

        if (0 <= nIndex)
        {
            ++nIndex;
            rtl_uString_newFromStr_WithLength( &pStat->ustrFileName, ustrFilePath->buffer + nIndex, ustrFilePath->length - nIndex );
            pStat->uValidFields |= osl_FileStatus_Mask_FileName;
        }
    }
       
    if (is_query_extended_file_attributes(uFieldMask))
    {
       struct stat aFileStat;
        
       if( 0 != lstat( path, &aFileStat ) )
       {
           PERROR( "osl_getFileStatus", path );
           return oslTranslateFileError(errno);
       }

       /*
        *   File Type
        */

       /* links to directories state also to be a directory */
       if ( S_ISLNK(aFileStat.st_mode) ) 
       { 
           pStat->eType=osl_File_Type_Link;
       }
       
       else if ( S_ISDIR(aFileStat.st_mode) ) 
       { 
           sal_Bool bIsMountPoint;

           oslFileError eRet = oslIsMountPoint( path, &bIsMountPoint ); 

           if ( eRet != osl_File_E_None ) 
               return eRet; 

           pStat->eType = bIsMountPoint ? osl_File_Type_Volume : osl_File_Type_Directory;
       }

       else if ( S_ISREG(aFileStat.st_mode) )
       {
           pStat->eType=osl_File_Type_Regular;
       }

       else if ( S_ISFIFO(aFileStat.st_mode) ) 
       { 
           pStat->eType=osl_File_Type_Fifo;
       }

       else if ( S_ISSOCK(aFileStat.st_mode) ) 
       { 
           pStat->eType=osl_File_Type_Socket;
       }

       else if ( S_ISCHR(aFileStat.st_mode) || S_ISBLK(aFileStat.st_mode) ) 
       { 
           pStat->eType=osl_File_Type_Special;
       }

       else if ( S_ISSOCK(aFileStat.st_mode) ) 
       { 
           pStat->eType=osl_File_Type_Socket;
       }

       else
       { 
           pStat->eType=osl_File_Type_Unknown;
       } 

       pStat->uValidFields|=osl_FileStatus_Mask_Type;


       if ( uFieldMask & osl_FileStatus_Mask_Attributes )
       {
           char *cp;
           
           pStat->uAttributes=0;        
           pStat->uValidFields|=osl_FileStatus_Mask_Attributes;

           /* set hidden flag if name starts with '.' */
           cp = strrchr( path, '/' );
           
           if( NULL ==  cp )
               cp = path;
           else
               cp++;
               
           if( '.' == cp[0] )
               pStat->uAttributes=osl_File_Attribute_Hidden;
           
		   /* #97133 Use gid and uid to determine effective permission right because
		      access() will block or timeout when trying to determine the effective
			  access right for a mount point where the undelying device is no longer 
			  availiable. This can result in wrong access rights because membership
			  of the current user in his secondary group or ACLs are not checked.
			  That's why we only do this for directories */
			  
           if ( S_ISDIR( aFileStat.st_mode ) )
		   {
		       if ( getuid() == aFileStat.st_uid )
			   {
                   if ( 0 == (S_IRUSR & aFileStat.st_mode) )
                       pStat->uValidFields &= ~osl_FileStatus_Mask_Attributes;

                   if ( 0 == (S_IWUSR & aFileStat.st_mode) )
                       pStat->uAttributes|=osl_File_Attribute_ReadOnly;

                   if ( S_IXUSR & aFileStat.st_mode )
                       pStat->uAttributes|=osl_File_Attribute_Executable;
			   }
			   else if ( getgid() == aFileStat.st_gid )
			   {
                   if ( 0 == (S_IRGRP & aFileStat.st_mode) )
                       pStat->uValidFields &= ~osl_FileStatus_Mask_Attributes;

                   if ( 0 == (S_IWGRP & aFileStat.st_mode) )
                       pStat->uAttributes|=osl_File_Attribute_ReadOnly;

                   if ( S_IXGRP & aFileStat.st_mode )
                       pStat->uAttributes|=osl_File_Attribute_Executable;
			   }
			   else
			   {
                   if ( 0 == (S_IROTH & aFileStat.st_mode) )
                       pStat->uValidFields &= ~osl_FileStatus_Mask_Attributes;

                   if ( 0 == (S_IWOTH & aFileStat.st_mode) )
                       pStat->uAttributes|=osl_File_Attribute_ReadOnly;

                   if ( S_IXOTH & aFileStat.st_mode )
                       pStat->uAttributes|=osl_File_Attribute_Executable;
			   }
		   }
		   else
		   {
               if ( 0 > access( path, W_OK ) )
               {
                   if ( errno == EACCES || errno == EROFS )
                   {
                       pStat->uAttributes|=osl_File_Attribute_ReadOnly;
                   }
                   else
                   {
                       pStat->uValidFields&=~osl_FileStatus_Mask_Attributes;
                   }
               }
			   
               if ( 0 == access( path, X_OK ) )
               {
                   pStat->uAttributes|=osl_File_Attribute_Executable;
               }
		   }


           /* user permissions */        
           if ( S_IRUSR & aFileStat.st_mode )
           {
               pStat->uAttributes|=osl_File_Attribute_OwnRead;
           }

           if ( S_IWUSR & aFileStat.st_mode )
           {
               pStat->uAttributes|=osl_File_Attribute_OwnWrite;
           }

           if ( S_IXUSR & aFileStat.st_mode )
           {
               pStat->uAttributes|=osl_File_Attribute_OwnExe;
           }

           /* group permissions */        
           if ( S_IRGRP & aFileStat.st_mode )
           {
               pStat->uAttributes|=osl_File_Attribute_GrpRead;
           }

           if ( S_IWGRP & aFileStat.st_mode )
           {
               pStat->uAttributes|=osl_File_Attribute_GrpWrite;
           }

           if ( S_IXGRP & aFileStat.st_mode )
           {
               pStat->uAttributes|=osl_File_Attribute_GrpExe;
           }

           /* world permissions */
           if ( S_IROTH & aFileStat.st_mode )
           {
               pStat->uAttributes|=osl_File_Attribute_OthRead;
           }

           if ( S_IWOTH & aFileStat.st_mode )
           {
               pStat->uAttributes|=osl_File_Attribute_OthWrite;
           }

           if ( S_IXOTH & aFileStat.st_mode )
           {
               pStat->uAttributes|=osl_File_Attribute_OthExe;
           }
       }

/* this is the "last status change" time, not a creation time */
#if 0
       /* Creation.Time */
       pStat->aCreationTime.Seconds = aFileStat.st_ctime;
       pStat->aCreationTime.Nanosec = 0;
       pStat->uValidFields|=osl_FileStatus_Mask_CreationTime;
#endif

       /* Access.Time */ 
       pStat->aAccessTime.Seconds = aFileStat.st_atime;
       pStat->aAccessTime.Nanosec = 0;
       pStat->uValidFields|=osl_FileStatus_Mask_AccessTime;

       /* Modify.Time */ 
       pStat->aModifyTime.Seconds = aFileStat.st_mtime;
       pStat->aModifyTime.Nanosec = 0;
       pStat->uValidFields|=osl_FileStatus_Mask_ModifyTime;

       /* FileSize */ 
       if ( pStat->eType == osl_File_Type_Regular )
       {
           pStat->uFileSize = aFileStat.st_size;
           pStat->uValidFields |= osl_FileStatus_Mask_FileSize;
       }
      
        /* File Exists semantic of osl_FileStatus_Mask_Validate */
        if ((uFieldMask & osl_FileStatus_Mask_LinkTargetURL) && (osl_File_Type_Link == pStat->eType)) 
        {
            char buffer[PATH_MAX];
            
            if (NULL != realpath(path, buffer))
            {
                rtl_uString* ustrLinkTarget = NULL;
                
                /* convert link target to unicode */
                rtl_string2UString(&ustrLinkTarget, buffer, strlen(buffer), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS);
                
                /* convert unicode path to file URL */
                if (osl_File_E_None == osl_getFileURLFromSystemPath( ustrLinkTarget, &pStat->ustrLinkTargetURL))
                    pStat->uValidFields |= osl_FileStatus_Mask_LinkTargetURL;
                   
                rtl_uString_release(ustrLinkTarget);
            }
            else
            {
                return oslTranslateFileError(errno);
            }
        }            
    }        
    
    return osl_File_E_None;
}

/****************************************************************************/
/*	osl_openFile */
/****************************************************************************/

oslFileHandle osl_createFileHandleFromFD( int fd )
{
	oslFileHandleImpl* pHandleImpl = NULL;

	if ( fd >= 0 )
	{
		pHandleImpl = (oslFileHandleImpl*) rtl_allocateMemory( sizeof(oslFileHandleImpl) );

		if( pHandleImpl )
		{
			pHandleImpl->ustrFilePath = NULL;
			rtl_uString_new( &pHandleImpl->ustrFilePath );
			pHandleImpl->fd = fd;
		}
	}

	return (oslFileHandle)pHandleImpl;
}

oslFileError osl_openFile( rtl_uString* ustrFileURL, oslFileHandle* pHandle, sal_uInt32 uFlags )
{
    oslFileHandleImpl* pHandleImpl = NULL;
    oslFileError eRet;
    rtl_uString* ustrFilePath = NULL;
    
    char buffer[PATH_MAX];
    int  fd;
    int  mode  = S_IRUSR | S_IRGRP | S_IROTH;
    int  flags = O_RDONLY;

    struct flock aflock;
    
    /* locking the complete file */
    aflock.l_type = 0;
	aflock.l_whence = SEEK_SET;
    aflock.l_start = 0;
	aflock.l_len = 0;

    OSL_ASSERT( ustrFileURL );
    OSL_ASSERT( pHandle );

    if( ( 0 == ustrFileURL->length ) )
        return osl_File_E_INVAL;

    /* convert file URL to system path */ 
    eRet = osl_getSystemPathFromFileURL( ustrFileURL, &ustrFilePath );
    if( osl_File_E_None != eRet )
        return eRet;

    /* remove trailing '/' if any */
    if( ( 1 < ustrFilePath->length ) && ( (sal_Unicode) '/' == ustrFilePath->buffer[ustrFilePath->length - 1] ) )
    {
        ustrFilePath->length--;
        ustrFilePath->buffer[ustrFilePath->length] = 0;
    }
    
    /* convert unicode path to text */
    if( UnicodeToText( buffer, PATH_MAX, ustrFilePath->buffer, ustrFilePath->length ) )
    {
        /* we do not open devices or such here */        
        if( !( uFlags & osl_File_OpenFlag_Create ) )
        {
            struct stat aFileStat;

            if( 0 > stat( buffer, &aFileStat ) )
            {
                PERROR( "osl_openFile", buffer );
                eRet = oslTranslateFileError( errno );
            }

            else if( !S_ISREG( aFileStat.st_mode ) )
            {
                eRet = osl_File_E_INVAL;
            }
        }
        
        if( osl_File_E_None == eRet )
        {
            /*
             * set flags and mode
             */

            if ( uFlags & osl_File_OpenFlag_Write )
            {
                mode |= S_IWUSR | S_IWGRP | S_IWOTH;
                flags = O_RDWR;
                aflock.l_type = F_WRLCK;
            }

            if ( uFlags & osl_File_OpenFlag_Create )
            {
                mode |= S_IWUSR | S_IWGRP | S_IWOTH;
                flags = O_CREAT | O_EXCL | O_RDWR;
            }

            /* open the file */
            fd = open( buffer, flags, mode );
            if ( fd >= 0 )
            {
                /* check if file lock is enabled and clear l_type member of flock otherwise */
                if( (char *) -1 == pFileLockEnvVar )
                {
                    /* FIXME: this is not MT safe */
                    pFileLockEnvVar = getenv("SAL_ENABLE_FILE_LOCKING");
                    
                    if( NULL == pFileLockEnvVar)
                        pFileLockEnvVar = getenv("STAR_ENABLE_FILE_LOCKING");
                }
                
                if( NULL == pFileLockEnvVar )
                    aflock.l_type = 0;
                    
                /* lock the file if flock.l_type is set */
                if( F_WRLCK != aflock.l_type || -1 != fcntl( fd, F_SETLK, &aflock ) )
                {
                    /* allocate memory for impl structure */
                    pHandleImpl = (oslFileHandleImpl*) rtl_allocateMemory( sizeof(oslFileHandleImpl) );
                    if( pHandleImpl )
                    {
                        pHandleImpl->ustrFilePath = ustrFilePath;
                        pHandleImpl->fd = fd;

                        *pHandle = (oslFileHandle) pHandleImpl;

                        return osl_File_E_None;
                    }
                    else
                    {
                        errno = ENOMEM;
                    }
                }
                
                close( fd );
            }

            PERROR( "osl_openFile", buffer );
            eRet = oslTranslateFileError( errno );
        }
    }
    else
        eRet = osl_File_E_INVAL;
    
    rtl_uString_release( ustrFilePath );
    return eRet;
}

/****************************************************************************/
/*	osl_closeFile */
/****************************************************************************/
        
oslFileError osl_closeFile( oslFileHandle Handle )
{
    oslFileHandleImpl* pHandleImpl = (oslFileHandleImpl *) Handle;
    oslFileError eRet = osl_File_E_INVAL;

    OSL_ASSERT( Handle );
    
    if( pHandleImpl )
    {
        rtl_uString_release( pHandleImpl->ustrFilePath );

        /* release file lock if locking is enabled */
        if( pFileLockEnvVar )
        {
            struct flock aflock;

            aflock.l_type = F_UNLCK;
            aflock.l_whence = SEEK_SET;
            aflock.l_start = 0;
            aflock.l_len = 0;
            
            /* FIXME: check if file is really locked ?  */
            
            /* release the file share lock on this file */
            if( -1 == fcntl( pHandleImpl->fd, F_SETLK, &aflock ) )
                PERROR( "osl_closeFile", "unlock failed" );
        }
        
        if( 0 > close( pHandleImpl->fd ) )
        {
#ifdef DEBUG_OSL_FILE
            perror("osl_closeFile");
#endif
            eRet = oslTranslateFileError( errno );
        }
        else
            eRet = osl_File_E_None;
        
        rtl_freeMemory( pHandleImpl );
    }
    
    return eRet;
}

/****************************************************************************/
/*	osl_isEndOfFile */
/****************************************************************************/

oslFileError SAL_CALL osl_isEndOfFile( oslFileHandle Handle, sal_Bool *pIsEOF )
{
    oslFileHandleImpl* pHandleImpl = (oslFileHandleImpl *) Handle;
    oslFileError eRet = osl_File_E_INVAL;
	
	if ( pHandleImpl)
	{
		long curPos = lseek( pHandleImpl->fd, 0, SEEK_CUR );
		
		if ( curPos >= 0 )
		{
			long endPos = lseek( pHandleImpl->fd, 0, SEEK_END  );
			
			if ( endPos >= 0 )
			{
				*pIsEOF = ( curPos == endPos );
				curPos = lseek( pHandleImpl->fd, curPos, SEEK_SET );
				
				if ( curPos >= 0 )
					eRet = osl_File_E_None;
				else
					eRet = oslTranslateFileError( errno );
			}
			else
				eRet = oslTranslateFileError( errno );
		}
		else
			eRet = oslTranslateFileError( errno );
	}

	return eRet;	
}

/****************************************************************************/
/*	osl_createDirectoryItemFromHandle */
/****************************************************************************/

oslFileError osl_createDirectoryItemFromHandle( oslFileHandle Handle, oslDirectoryItem* pItem )
{
    oslFileHandleImpl* pHandleImpl = (oslFileHandleImpl *) Handle;
    
    OSL_ASSERT( Handle );
    OSL_ASSERT( pItem );
    
    if( pHandleImpl )
    {
        rtl_uString_acquire( pHandleImpl->ustrFilePath );
        
        *pItem = (oslDirectoryItem) pHandleImpl->ustrFilePath;
        return osl_File_E_None;
    }

    return osl_File_E_INVAL;
}

/****************************************************************************/
/*	osl_moveFile */
/****************************************************************************/

oslFileError osl_moveFile( rtl_uString* ustrFileURL, rtl_uString* ustrDestURL )
{
    char srcPath[PATH_MAX];
    char destPath[PATH_MAX];
    oslFileError eRet;

    OSL_ASSERT( ustrFileURL );
    OSL_ASSERT( ustrDestURL );

    /* convert source url to system path */
    eRet = FileURLToPath( srcPath, PATH_MAX, ustrFileURL );
    if( eRet != osl_File_E_None )
        return eRet;

    /* convert destination url to system path */
    eRet = FileURLToPath( destPath, PATH_MAX, ustrDestURL );
    if( eRet != osl_File_E_None )
        return eRet;

    return oslDoMoveFile( srcPath, destPath );
}

/****************************************************************************/
/*	osl_copyFile */
/****************************************************************************/

oslFileError osl_copyFile( rtl_uString* ustrFileURL, rtl_uString* ustrDestURL )
{
    char srcPath[PATH_MAX];
    char destPath[PATH_MAX];
    oslFileError eRet;

    OSL_ASSERT( ustrFileURL );
    OSL_ASSERT( ustrDestURL );

    /* convert source url to system path */
    eRet = FileURLToPath( srcPath, PATH_MAX, ustrFileURL );
    if( eRet != osl_File_E_None )
        return eRet;

    /* convert destination url to system path */
    eRet = FileURLToPath( destPath, PATH_MAX, ustrDestURL );
    if( eRet != osl_File_E_None )
        return eRet;

    return osl_psz_copyFile( srcPath, destPath );
}

/****************************************************************************/
/*	osl_removeFile */
/****************************************************************************/

oslFileError osl_removeFile( rtl_uString* ustrFileURL )
{
    char path[PATH_MAX];
    oslFileError eRet;

    OSL_ASSERT( ustrFileURL );

    /* convert file url to system path */
    eRet = FileURLToPath( path, PATH_MAX, ustrFileURL );
    if( eRet != osl_File_E_None )
        return eRet;

    return osl_psz_removeFile( path );
}

/****************************************************************************/
/*	osl_getVolumeInformation */
/****************************************************************************/

oslFileError osl_getVolumeInformation( rtl_uString* ustrDirectoryURL, oslVolumeInfo* pInfo, sal_uInt32 uFieldMask )
{
    char path[PATH_MAX];
    oslFileError eRet;

    OSL_ASSERT( ustrDirectoryURL );
    OSL_ASSERT( pInfo );

    /* convert directory url to system path */
    eRet = FileURLToPath( path, PATH_MAX, ustrDirectoryURL );
    if( eRet != osl_File_E_None )
        return eRet;

    return osl_psz_getVolumeInformation( path, pInfo, uFieldMask);
}

/****************************************************************************/
/*	osl_createDirectory */
/****************************************************************************/

oslFileError osl_createDirectory( rtl_uString* ustrDirectoryURL )
{
    char path[PATH_MAX];
    oslFileError eRet;

    OSL_ASSERT( ustrDirectoryURL );

    /* convert directory url to system path */
    eRet = FileURLToPath( path, PATH_MAX, ustrDirectoryURL );
    if( eRet != osl_File_E_None )
        return eRet;

    return osl_psz_createDirectory( path );
}

/****************************************************************************/
/*	osl_removeDirectory */
/****************************************************************************/

oslFileError osl_removeDirectory( rtl_uString* ustrDirectoryURL )
{
    char path[PATH_MAX];
    oslFileError eRet;

    OSL_ASSERT( ustrDirectoryURL );

    /* convert directory url to system path */
    eRet = FileURLToPath( path, PATH_MAX, ustrDirectoryURL );
    if( eRet != osl_File_E_None )
        return eRet;
        
    return osl_psz_removeDirectory( path );
}

/****************************************************************************/
/*	osl_getCanonicalName */
/****************************************************************************/

oslFileError osl_getCanonicalName( rtl_uString* ustrFileURL, rtl_uString** pustrValidURL )
{
    char reqPath[PATH_MAX];
    char valPath[PATH_MAX];
    oslFileError eRet;

    OSL_ASSERT( ustrFileURL );
    OSL_ASSERT( pustrValidURL );

    /* convert file url to system path */
    eRet = FileURLToPath( reqPath, PATH_MAX, ustrFileURL );
    if( eRet != osl_File_E_None )
        return eRet;

    eRet = osl_psz_getCanonicalName( reqPath, valPath );
    if( osl_File_E_None == eRet )
    {
        rtl_uString* ustrValidPath = NULL;
        
        /* convert file name to unicode */
        rtl_string2UString( &ustrValidPath, valPath, strlen( valPath ), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );
        
        /* file urls must be encoded */
        osl_getFileURLFromSystemPath( ustrValidPath, pustrValidURL );
        rtl_uString_release( ustrValidPath );
    }
    
    return eRet;
}

/****************************************************************************/
/*	osl_getAbsoluteFileURL */
/****************************************************************************/
static sal_Bool isLink( const char *source )
{
	struct stat status;
	return
		0 == lstat( source , &status ) && S_ISLNK(status.st_mode);
}

/** strlcpy not stdc ? I had unresolved externals on solaris */
static size_t mystrlcpy( char *target, const char *source , int max )
{
	int i;
	for( i = 0 ; source[i] && i < max ; i ++ )
		target[i] = source[i];
	if( i < max )
		target[i] = 0;
	return i;
}

static oslFileError convertPathToRealPath( const char *source, char *target )
{
	oslFileError eRet = osl_File_E_None;
	sal_Int32 i = 0;
	mystrlcpy( target , source , PATH_MAX );
	while( target[i] )
	{
		if(  '/' == target[i]   &&
			 '.' == target[i+1] &&
			 '.' == target[i+2] &&
			('/' == target[i+3] || ! target[i+3]) )
		{
			/* step one directory back */
			char temp[PATH_MAX];
			target[i] = 0;
			if( isLink( target ) && realpath( target , temp) )
			{
				/* the parent directory was a link, resolve this ! */
				if( target[i+3] )
				{
					/* store the end of the path */
					char temp2[PATH_MAX];
					if( mystrlcpy( temp2, &(target[i+1]) , PATH_MAX ) >= PATH_MAX)
					{
						eRet = osl_File_E_NAMETOOLONG;
						break;
					}

					/* patch the new path */
					strcpy( target , temp );
					i = strlen( target );
					strcat( target , "/" );
					strcat( target, temp2 );
				}
				else
				{
					/* this is the end */
					strcpy( target, temp );
					i = strlen( target );
					strcat( target , "/.." );
				}
			}
			else
			{
				int k;
				int j = i +3;
				
				/* seek the parent dir */
				while( i >= 0 && target[i] != '/' )
				{
					i --;
				}
				if( i < 0 )
				{
					/* .. within root directory , this is an error */
					eRet = osl_File_E_INVAL;
					break;
				}

				for( k = 0; target[j+k] ; k ++ )
				{
					target[i+k] = target[j+k];
				}
				target[i+k] = 0;
				if( 0 == i && 0 == k )
				{
					target[i] = '/';
					i++;
					target[i] = 0;
				}
			}
			
		}
		else if(   '/' == target[i] &&
				   ( 0 == target[i+1] || ('.' == target[i+1] && 0 == target[i+2] ) ) )
		{
			// "/" or "/." at the end of the string, ignore this
			target[i] = 0;
		}
		else if( '/' == target[i] && '.' == target[i+1] && '/' == target[i+2] )
		{
			/* /./ within the path, remove it */
			int j;
			for( j = i ; target[j+2] ; j ++ )
			{
				target[j] = target[j+2];
			}
			target[j] = 0;
		}
		else
		{
			i++;
		}
	}

	return eRet;	
}

oslFileError osl_getAbsoluteFileURL( rtl_uString*  ustrBaseDirURL, 
    rtl_uString* ustrRelativeURL, rtl_uString** pustrAbsoluteURL )
{
    oslFileError eRet = osl_File_E_INVAL;
    rtl_uString* ustrPath = NULL;
    
    char path[PATH_MAX];
    
    eRet = osl_getSystemPathFromFileURL( ustrRelativeURL, &ustrPath );
    if( osl_File_E_None != eRet )
        return eRet;
    
    /* if relative path is absolute, we can ignore the base path */
    if( (sal_Unicode) '/' != ustrPath->buffer[0] )
    {
        rtl_uString* ustrBasePath = NULL;
        
        /* concatinate base and relative path */
        eRet = osl_getSystemPathFromFileURL( ustrBaseDirURL, &ustrBasePath );
        if( osl_File_E_None == eRet )
        {
            rtl_uString* ustrTmp = NULL;
            sal_uInt32 capacity;
            
            capacity = ustrBasePath->length + ustrPath->length + 1;
            rtl_uString_new_WithLength( &ustrTmp, capacity );

            rtl_uStringbuffer_insert( &ustrTmp, &capacity, 0, ustrBasePath->buffer, ustrBasePath->length );
			if( ! ustrTmp->length || ustrBasePath->buffer[ustrTmp->length-1] != '/' )
			{
				rtl_uStringbuffer_insert_ascii(
					&ustrTmp, &capacity, ustrTmp->length, "/", 1 );
			}
            rtl_uStringbuffer_insert( &ustrTmp, &capacity, ustrTmp->length, ustrPath->buffer, ustrPath->length );
    
            rtl_uString_release( ustrBasePath );
            rtl_uString_release( ustrPath );
            ustrPath = ustrTmp;
        }
    }
    
    /* convert unicode path to text */
    if( UnicodeToText( path, PATH_MAX, ustrPath->buffer, ustrPath->length ) )
    {
        char realPath[PATH_MAX];
        
        /* use realpath to determine absolute path */
		if( osl_File_E_None == convertPathToRealPath( path , realPath ) )
        {
            rtl_uString* ustrRealPath = NULL;
        
            /* convert file name to unicode */
            rtl_string2UString( &ustrRealPath, realPath, strlen( realPath ), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );
        
            /* file urls must be encoded */
            osl_getFileURLFromSystemPath( ustrRealPath, pustrAbsoluteURL );
            rtl_uString_release( ustrRealPath );
			eRet = osl_File_E_None;
        }
        else
        {
            PERROR( "osl_getAbsolutePath", path );
            eRet = oslTranslateFileError( errno );
        }
    }
    
    rtl_uString_release( ustrPath );
    return eRet;
}    
    
/****************************************************************************/
/*	osl_searchFileURL */
/****************************************************************************/

oslFileError osl_searchFileURL( rtl_uString* ustrFilePath, rtl_uString* ustrSearchPath, rtl_uString** pustrURL )
{
    rtl_uString* ustrPath = NULL;
    oslFileError eRet;

    char searchPath[PATH_MAX];
    char filePath[PATH_MAX];
    char path[PATH_MAX] = "";

    OSL_ASSERT( ustrFilePath );
    OSL_ASSERT( ustrSearchPath );
    OSL_ASSERT( pustrURL );

    /* file path may also be an URL */
    eRet = FileURLToPath( filePath, PATH_MAX, ustrFilePath );
    if( eRet != osl_File_E_None )
    {
        if( eRet == osl_File_E_INVAL )
        {
            /* seems not to be an URL, so expect it to be a system patrh */
            if( ! UnicodeToText( filePath, PATH_MAX, ustrFilePath->buffer, ustrFilePath->length ) )
                return osl_File_E_INVAL;
        }
        else
            return eRet;
    }

    /* if a search path is specified, it is no file URL */        
    if( ustrSearchPath->length && UnicodeToText( searchPath, PATH_MAX, ustrSearchPath->buffer, ustrSearchPath->length )  )
    {
        if( NULL == ImplSearchPath( path, PATH_MAX, filePath, searchPath, ';' ) )
           *path = '\0';
    }

    /* did we already find something ? */
    if( '\0' == *path )
    {
        char * pEnvPath = getenv("PATH");

        /* fallback to PATH env var */            
        if( ( NULL == pEnvPath ) || ( NULL == ImplSearchPath( path, PATH_MAX, filePath, pEnvPath , ':' ) ) )
        {
            char workdir[PATH_MAX];
                
            /* last try: current working dir */
            if( ( NULL == getcwd( workdir, PATH_MAX ) ) || ( NULL == ImplSearchPath( path, PATH_MAX, filePath, workdir , ':' ) ) )
                return osl_File_E_NOENT;
         }
    }

    /* did we find something, then convert it */        
    if( *path )
    {
        /* convert file name to unicode */
        rtl_string2UString( &ustrPath, path, strlen( path ), osl_getThreadTextEncoding(), OSTRING_TO_OUSTRING_CVTFLAGS );
        
        /* file urls must be encoded */
        osl_getFileURLFromSystemPath( ustrPath, pustrURL );
        rtl_uString_release( ustrPath );
    }
    
    return osl_File_E_None;
}


/****************************************************************************/
/*	osl_setFileAttributes */
/****************************************************************************/

oslFileError osl_setFileAttributes( rtl_uString* ustrFileURL, sal_uInt64 uAttributes )
{
    char path[PATH_MAX];
    oslFileError eRet;

    OSL_ASSERT( ustrFileURL );

    /* convert file url to system path */
    eRet = FileURLToPath( path, PATH_MAX, ustrFileURL );
    if( eRet != osl_File_E_None )
        return eRet;

    return osl_psz_setFileAttributes( path, uAttributes );
}

/****************************************************************************/
/*	osl_setFileTime */
/****************************************************************************/

oslFileError osl_setFileTime( rtl_uString* ustrFileURL, TimeValue* pCreationTime,
                              TimeValue* pLastAccessTime, TimeValue* pLastWriteTime )
{
    char path[PATH_MAX];
    oslFileError eRet;

    OSL_ASSERT( ustrFileURL );

    /* convert file url to system path */
    eRet = FileURLToPath( path, PATH_MAX, ustrFileURL );
    if( eRet != osl_File_E_None )
        return eRet;

    return osl_psz_setFileTime( path, pCreationTime, pLastAccessTime, pLastWriteTime );
}

/******************************************************************************
 *
 *                  Exported Module Functions
 *             (independent of C or Unicode Strings)
 *
 *****************************************************************************/


/*******************************************
    osl_writeFile
********************************************/

oslFileError osl_readFile( oslFileHandle Handle, void* pBuffer, sal_uInt64 uBytesRequested, sal_uInt64* pBytesRead )
{
    ssize_t nBytes = 0;
    int     nRet   = 0;
    oslFileHandleImpl* pHandleImpl = 0;

    /* parameter checking */

    OSL_ASSERT( Handle );
    OSL_ASSERT( pBuffer );
    OSL_ASSERT( pBytesRead );
    
    if ( 0 == Handle || 0 == pBuffer || 0 == pBytesRead )
        return osl_File_E_INVAL;

    pHandleImpl = (oslFileHandleImpl*)Handle;
    
    OSL_ASSERT( pHandleImpl->fd >= 0 );
    
    if ( pHandleImpl->fd < 0 )
        return osl_File_E_INVAL;

    nBytes = read( pHandleImpl->fd, pBuffer, uBytesRequested );
    
    if ( nBytes < 0 )
    {
        nRet = errno;
        return oslTranslateFileError( nRet );
    }

    *pBytesRead = nBytes;
    
    return osl_File_E_None;
}

/*******************************************
    osl_writeFile
********************************************/

oslFileError osl_writeFile( oslFileHandle Handle, const void* pBuffer, sal_uInt64 uBytesToWrite, sal_uInt64* pBytesWritten )
{
    ssize_t nBytes = 0;
    int     nRet   = 0;
    oslFileHandleImpl* pHandleImpl = 0;

    /* parameter checking */

    OSL_ASSERT( Handle );
    OSL_ASSERT( pBuffer );
    OSL_ASSERT( pBytesWritten );

    if ( 0 == Handle || 0 == pBuffer || 0 == pBytesWritten )
        return osl_File_E_INVAL;
        
    pHandleImpl = (oslFileHandleImpl*)Handle;

    OSL_ASSERT( pHandleImpl->fd >= 0 );
    
    if ( pHandleImpl->fd < 0 )
        return osl_File_E_INVAL;

    nBytes = write( pHandleImpl->fd, pBuffer, uBytesToWrite );
    
    if ( nBytes < 0 )
    {
        nRet = errno;
        return oslTranslateFileError(nRet);
    }

    *pBytesWritten = nBytes;
    
    return osl_File_E_None;
}

/*******************************************
    osl_writeFile
********************************************/

oslFileError osl_setFilePos( oslFileHandle Handle, sal_uInt32 uHow, sal_Int64 uPos )
{
    oslFileHandleImpl* pHandleImpl=0;
    int nRet=0;
    off_t nOffset=0;

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  osl_setFilePos\n");
#endif

    pHandleImpl = (oslFileHandleImpl*) Handle;
    if ( pHandleImpl == 0 )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_setFilePos [INVAL]\n");
#endif
        return osl_File_E_INVAL;
    }

    if ( pHandleImpl->fd < 0 )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_setFilePos [INVAL]\n");
#endif
        return osl_File_E_INVAL;
    }

    /* FIXME mfe: setFilePos: Do we have any runtime function to determine LONG_MAX? */
    if ( uPos > LONG_MAX )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_setFilePos [OVERFLOW]\n");
#endif
        return osl_File_E_OVERFLOW;
    }

    nOffset=(off_t)uPos;
    
    switch(uHow)
    {
        case osl_Pos_Absolut:
            nOffset = lseek(pHandleImpl->fd,nOffset,SEEK_SET);            
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_setFilePos Absolut\n");
#endif
            break;

        case osl_Pos_Current:
            nOffset = lseek(pHandleImpl->fd,nOffset,SEEK_CUR);
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_setFilePos Current\n");
#endif
            break;

        case osl_Pos_End:
            nOffset = lseek(pHandleImpl->fd,nOffset,SEEK_END);
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_setFilePos End\n");
#endif
            break;

        default:
#ifdef TRACE_OSL_FILE
            fprintf(stderr,"Out osl_setFilePos [INVAL]\n");
#endif
            return osl_File_E_INVAL;
            break;
    }

    if ( nOffset < 0 )
    {
        nRet=errno;
#ifdef DEBUG_OSL_FILE
        perror("lseek");
#endif
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_setFilePos [lseek]\n");
#endif
        return oslTranslateFileError(nRet);
    }

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out osl_setFilePos [ok]\n");
#endif
    return osl_File_E_None;
}


oslFileError osl_getFilePos( oslFileHandle Handle, sal_uInt64* pPos )
{
    oslFileHandleImpl* pHandleImpl=0;
    off_t nOffset=0;
    int nRet=0;

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  osl_getFilePos\n");
#endif

    pHandleImpl = (oslFileHandleImpl*) Handle;
    if ( pHandleImpl == 0 || pPos == 0)
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_getFilePos [INVAL]\n");
#endif
        return osl_File_E_INVAL;
    }

    if ( pHandleImpl->fd < 0 )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_getFilePos [INVAL]\n");
#endif
        return osl_File_E_INVAL;
    }

    nOffset=lseek(pHandleImpl->fd,0,SEEK_CUR);
    if ( nOffset < 0 )
    {
        nRet=errno;
#ifdef DEBUG_OSL_FILE
        perror("lseek");
#endif
        *pPos=0;
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_getFilePos [lseek]\n");
#endif
        return oslTranslateFileError(nRet);
    }

    *pPos=nOffset;
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out osl_getFilePos [ok]\n");
#endif
    return osl_File_E_None;
}


oslFileError osl_setFileSize( oslFileHandle Handle, sal_uInt64 uSize )
{
    oslFileHandleImpl* pHandleImpl=0;
    off_t nOffset=0;

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  osl_setFileSize\n");
#endif

    pHandleImpl = (oslFileHandleImpl*) Handle;
    if ( pHandleImpl == 0 )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_setFileSize [INVAL]\n");
#endif
        return osl_File_E_INVAL;
    }

    if ( pHandleImpl->fd < 0 )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_setFileSize [INVAL]\n");
#endif
        return osl_File_E_INVAL;
    }

    /* FIXME: mfe: setFileSize: Do we have any runtime function to determine LONG_MAX? */
    if ( uSize > LONG_MAX )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_setFileSize [OVERFLOW]\n");
#endif
        return osl_File_E_OVERFLOW;
    }

    nOffset = (off_t)uSize;
	if (ftruncate (pHandleImpl->fd, nOffset) < 0)
	{
		/* Failure. Try fallback algorithm */
		oslFileError result;
		struct stat  aStat;
		off_t        nCurPos;

		/* Save original result */
		result = oslTranslateFileError (errno);
		PERROR("ftruncate", "Try osl_setFileSize [fallback]\n");

		/* Check against current size. Fail upon 'shrink' */
		if (fstat (pHandleImpl->fd, &aStat) < 0)
		{
			PERROR("ftruncate: fstat", "Out osl_setFileSize [error]\n");
			return (result);
		}
		if ((0 <= nOffset) && (nOffset <= aStat.st_size))
		{
			/* Failure upon 'shrink'. Return original result */
			return (result);
		}

		/* Save current position */
		nCurPos = (off_t)lseek (pHandleImpl->fd, (off_t)0, SEEK_CUR);
		if (nCurPos == (off_t)(-1))
		{
			PERROR("ftruncate: lseek", "Out osl_setFileSize [error]\n");
			return (result);
		}

		/* Try 'expand' via 'lseek()' and 'write()' */
		if (lseek (pHandleImpl->fd, (off_t)(nOffset - 1), SEEK_SET) < 0)
		{
			PERROR("ftruncate: lseek", "Out osl_setFileSize [error]\n");
			return (result);
		}
		if (write (pHandleImpl->fd, (char*)"", (size_t)1) < 0)
		{
			/* Failure. Restore saved position */
			PERROR("ftruncate: write", "Out osl_setFileSize [error]\n");
			if (lseek (pHandleImpl->fd, (off_t)nCurPos, SEEK_SET) < 0)
			{
#ifdef DEBUG_OSL_FILE
				perror("ftruncate: lseek");
#endif /* DEBUG_OSL_FILE */
			}
			return (result);
		}

		/* Success. Restore saved position */
		if (lseek (pHandleImpl->fd, (off_t)nCurPos, SEEK_SET) < 0)
		{
			PERROR("ftruncate: lseek", "Out osl_setFileSize [error]");
			return (result);
		}
	}

#ifdef TRACE_OSL_FILE
	fprintf(stderr, "Out osl_setFileSize [ok]\n");
#endif /* TRACE_OSL_FILE */
	return (osl_File_E_None);
}

/******************************************************************************
 *
 *                  C-String Versions of Exported Module Functions
 *
 *****************************************************************************/

#ifdef HAVE_STATFS_H

#if defined(FREEBSD) || defined(NETBSD) || defined(MACOSX)
#define __OSL_STATFS statfs
#define __OSL_STATFS_FUNC statfs

#define __OSL_STATFS_BLKSIZ(a) ((sal_uInt64)((a).f_bsize))
#define __OSL_STATFS_BLKFREE(a) ((sal_uInt64)((a).f_bavail))
#define __OSL_STATFS_TYPENAME(a) ((a).f_fstypename)

#define __OSL_STATFS_ISREMOTE(a) (((a).f_type & MNT_LOCAL) == 0)

#endif /* FREEBSD || NETBSD */

#if defined(LINUX)
#define __OSL_STATFS statfs
#define __OSL_STATFS_FUNC statfs

#define __OSL_STATFS_BLKSIZ(a) ((sal_uInt64)((a).f_bsize))
#define __OSL_STATFS_BLKFREE(a) ((sal_uInt64)((a).f_bavail))

/* <linux/nfs_fs.h> */
#define __OSL_STATFS_ISNFS(a) ((a).f_type == 0x6969) /* NFS_SUPER_MAGIC */

/* <linux/smb_fs.h> */
#define __OSL_STATFS_ISSMB(a) ((a).f_type == 0x517B) /* SMB_SUPER_MAGIC */

/* MFS = Mosix file system */
#define __OSL_STATFS_ISMFS(a) ((a).f_type ==  0xE08F018) /* MFS_SUPER_MAGIC = 235466776 */

#define __OSL_STATFS_ISREMOTE(a) \
(__OSL_STATFS_ISMFS((a)) || __OSL_STATFS_ISNFS((a)) || __OSL_STATFS_ISSMB((a)))

#endif /* LINUX */

#if defined(IRIX)

#define __OSL_STATFS statvfs
#define __OSL_STATFS_FUNC statvfs

#define __OSL_STATFS_BLKSIZ(a) ((sal_uInt64)((a).f_bsize))
#define __OSL_STATFS_BLKFREE(a) ((sal_uInt64)((a).f_bfree))
#define __OSL_STATFS_TYPENAME(a) ((a).f_basetype)

#define __OSL_STATFS_ISREMOTE(a) (rtl_str_compare((a).f_basetype, "nfs") == 0)

#endif /* IRIX */


#if defined(SOLARIS)
#define __OSL_STATFS statvfs
#define __OSL_STATFS_FUNC statvfs

#define __OSL_STATFS_BLKSIZ(a) ((sal_uInt64)((a).f_frsize))
#define __OSL_STATFS_BLKFREE(a) ((sal_uInt64)((a).f_bavail))
#define __OSL_STATFS_TYPENAME(a) ((a).f_basetype)

#define __OSL_STATFS_ISREMOTE(a) (rtl_str_compare((a).f_basetype, "nfs") == 0)

#endif /* SOLARIS */

#endif /* HAVE_STATFS_H */


static oslFileError osl_psz_getVolumeInformation (
	sal_Char* pszDirectory, oslVolumeInfo* pInfo, sal_uInt32 uFieldMask)
{
#if defined(__OSL_STATFS)

	/* The 'statfs' struct */
	struct __OSL_STATFS aStatFS;

#endif /* __OSL_STATFS */


#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  osl_psz_getVolumeInformation\n");
#endif
    if (!pInfo)
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_psz_getVolumeInformation [pInfo == 0]\n");
#endif
        return osl_File_E_INVAL;
    }


	/* Erase valid fields */
    pInfo->uValidFields = 0;

#if defined(__OSL_STATFS)

	/* The 'statfs' function call */
#ifdef TRACE_OSL_FILE
	fprintf(stderr, "statfs('%s')\n", pszDirectory);
#endif

	memset (&aStatFS, 0, sizeof(aStatFS));

	if (__OSL_STATFS_FUNC (pszDirectory, &aStatFS) < 0)
	{
		/* Failure */
		oslFileError result = oslTranslateFileError (errno);
		PERROR("statfs", "Out osl_psz_getVolumeInformation [statfs]\n");
		return (result);
	}

#endif /* __OSL_STATFS */


	/* Attributes */
	pInfo->uAttributes = 0;
	if (uFieldMask & osl_VolumeInfo_Mask_Attributes)
	{
		/* FIXME: how to detect the kind of storage (fixed, cdrom, ...) */
#if defined(__OSL_STATFS_ISREMOTE)
		if (__OSL_STATFS_ISREMOTE(aStatFS))
		{
			pInfo->uAttributes  |= osl_Volume_Attribute_Remote;
			pInfo->uValidFields |= osl_VolumeInfo_Mask_Attributes;
		}
#endif  /* __OSL_STATFS_ISREMOTE */
	}


	/* TotalSpace */
	pInfo->uTotalSpace = 0;
	if ((uFieldMask & osl_VolumeInfo_Mask_TotalSpace) ||
		(uFieldMask & osl_VolumeInfo_Mask_UsedSpace )    )
	{
#if defined(__OSL_STATFS_BLKSIZ)
		pInfo->uTotalSpace   = __OSL_STATFS_BLKSIZ(aStatFS);
		pInfo->uTotalSpace  *= (sal_uInt64)(aStatFS.f_blocks);
		pInfo->uValidFields |= osl_VolumeInfo_Mask_TotalSpace;
#endif  /* __OSL_STATFS_BLKSIZ */
	}


	/* FreeSpace */
	pInfo->uFreeSpace = 0;
	if ((uFieldMask & osl_VolumeInfo_Mask_FreeSpace) ||
		(uFieldMask & osl_VolumeInfo_Mask_UsedSpace)    )
	{
#if defined(__OSL_STATFS_BLKSIZ)
		pInfo->uFreeSpace = __OSL_STATFS_BLKSIZ(aStatFS);
		if (getuid() == 0)
		{
			/* free space */
			pInfo->uFreeSpace *= (sal_uInt64)(aStatFS.f_bfree);
		}
		else
		{
			/* available space */
			pInfo->uFreeSpace *= __OSL_STATFS_BLKFREE(aStatFS);
		}
		pInfo->uValidFields |= osl_VolumeInfo_Mask_FreeSpace;
#endif  /* __OSL_STATFS_BLKSIZ */
	}
	

	/* UsedSpace */
	pInfo->uUsedSpace = 0;
	if ((pInfo->uValidFields & osl_VolumeInfo_Mask_TotalSpace) &&
		(pInfo->uValidFields & osl_VolumeInfo_Mask_FreeSpace )    )
	{
		pInfo->uUsedSpace    = pInfo->uTotalSpace - pInfo->uFreeSpace;
		pInfo->uValidFields |= osl_VolumeInfo_Mask_UsedSpace;
	}


    /* MaxNameLength */
	pInfo->uMaxNameLength = 0;
	if (uFieldMask & osl_VolumeInfo_Mask_MaxNameLength)
	{
		long nLen = pathconf (pszDirectory, _PC_NAME_MAX);
		if (nLen > 0)
		{
			pInfo->uMaxNameLength = (sal_uInt32)nLen;
			pInfo->uValidFields |= osl_VolumeInfo_Mask_MaxNameLength;
		}
	}


    /* MaxPathLength */
	pInfo->uMaxPathLength = 0;
	if (uFieldMask & osl_VolumeInfo_Mask_MaxPathLength)
	{
		long nLen = pathconf (pszDirectory, _PC_PATH_MAX);
		if (nLen > 0)
		{
			pInfo->uMaxPathLength = (sal_uInt32)nLen;
			pInfo->uValidFields |= osl_VolumeInfo_Mask_MaxPathLength;
		}
	}


	/* FileSystemName */
	if (uFieldMask & osl_VolumeInfo_Mask_FileSystemName)
	{
#if defined(__OSL_STATFS_TYPENAME)
    	rtl_string2UString(
        	&(pInfo->ustrFileSystemName),
        	__OSL_STATFS_TYPENAME(aStatFS),
        	rtl_str_getLength( __OSL_STATFS_TYPENAME(aStatFS) ),
        	osl_getThreadTextEncoding(),
        	OUSTRING_TO_OSTRING_CVTFLAGS );
		pInfo->uValidFields |= osl_VolumeInfo_Mask_FileSystemName;
#endif /* __OSL_STATFS_TYPENAME */
	}


	/* DeviceHandle */
    if (uFieldMask & osl_VolumeInfo_Mask_DeviceHandle)
    {
        /* FIXME: check also entries in mntent for the device
		   and fill it with correct values */
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"osl_getVolumeInformation : checking DeviceHandle\n");
#endif

		OSL_ASSERT(pInfo->pDeviceHandle);
        *(pInfo->pDeviceHandle)=osl_isFloppyDrive(pszDirectory);

        if (*(pInfo->pDeviceHandle))
        {
            pInfo->uValidFields |= osl_VolumeInfo_Mask_DeviceHandle;

            pInfo->uAttributes  |= osl_Volume_Attribute_Removeable;
			pInfo->uValidFields |= osl_VolumeInfo_Mask_Attributes;
        }
    }

	/* Finished */
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out osl_getVolumeInformation [ok]\n");
#endif
    return osl_File_E_None;
}




oslFileError osl_psz_getCanonicalName( sal_Char* pszRequested, sal_Char* pszValid )
{
    int nFD=0;
    int nDone=0;
    sal_Char*  pszRequestedDir = 0;
    sal_Char  szRequestedDir[PATH_MAX];
    sal_Char  szUnique[PATH_MAX];
    sal_uInt32 nIndex=1;

    szRequestedDir[0] = '\0';
    szUnique[0] = '\0';
    
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  osl_psz_getCanonicalName\n");
#endif
    
    nFD = open(pszRequested,O_RDONLY);
    if ( nFD == -1 )
    {
        nFD = errno;
        if ( nFD == ENOENT )
        {
            DIR* pDIR=0;
            
            /* mfe: one of the path entries does not exists ; maybe the file or one of the directories */
            strcpy(szRequestedDir,pszRequested);
            pszRequestedDir=oslGetDirName(szRequestedDir);
            pDIR = opendir(pszRequestedDir);
            if ( pDIR == 0 )
            {
                /* mfe: OK, one of the directories does not exist. Return Error */
                nFD=errno;                
#ifdef TRACE_OSL_FILE
                fprintf(stderr,"Out osl_psz_getCanonicalName [no dir]\n");
#endif
                return oslTranslateFileError(nFD);
            }
            /* mfe: file does not exists, everything is OK */
            closedir(pDIR);

            strcpy(pszValid,pszRequested);
            
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"Valid File Name '%s'\n",pszValid);
#endif
            
            
#ifdef TRACE_OSL_FILE
            fprintf(stderr,"Out osl_psz_getCanonicalName [ok]\n");
#endif
            return osl_File_E_None;
        }

        /* mfe: some other error occurred */
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_psz_getCanonicalName [other error]\n");
#endif
        return oslTranslateFileError(nFD);
    }

    close(nFD);
    
    /* mfe: file exists, make new unique file name */
    while ( nDone == 0 && nIndex < 10000 )
    {
        oslMakeUniqueName(pszRequested,szUnique,nIndex);
        nFD = open(szUnique,O_RDONLY);
        if ( nFD == -1 )
        {
            nFD = errno;
            if ( nFD == ENOENT )
            {
                nDone=1;
            }
        }
        else
        {
            close(nFD);
        }
        nIndex++;
    }

    if ( nIndex == 10000 )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_psz_getCanonicalName [ok]\n");
#endif
        return osl_File_E_EXIST;
    }

    /* mfe: reuse */
    strcat(szRequestedDir,szUnique);
    strcpy(pszValid,szRequestedDir);
   
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out osl_psz_getCanonicalName [ok]\n");
#endif
    return osl_File_E_None;
}


oslFileError osl_psz_setFileAttributes( sal_Char* pszFilePath, sal_uInt64 uAttributes )
{
    int nRet=0;
    mode_t nNewMode=0;
    
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  osl_psz_setFileAttributes\n");
#endif    
    
    if ( uAttributes & osl_File_Attribute_OwnRead )
    {
        nNewMode|=S_IRUSR;
    }
    if ( uAttributes & osl_File_Attribute_OwnWrite )
    {
        nNewMode|=S_IWUSR;
    }
    if  ( uAttributes & osl_File_Attribute_OwnExe  )
    {
        nNewMode|=S_IXUSR;
    }
    if ( uAttributes & osl_File_Attribute_GrpRead  )
    {
        nNewMode|=S_IRGRP;
    }
    if ( uAttributes & osl_File_Attribute_GrpWrite )
    {
        nNewMode|=S_IWGRP;
    }
    if ( uAttributes & osl_File_Attribute_GrpExe   )
    {
        nNewMode|=S_IXGRP;
    }
    if ( uAttributes & osl_File_Attribute_OthRead  )
    {
        nNewMode|=S_IROTH;
    }
    if ( uAttributes & osl_File_Attribute_OthWrite )
    {
        nNewMode|=S_IWOTH;
    }
    if ( uAttributes & osl_File_Attribute_OthExe   )
    {
        nNewMode|=S_IXOTH;        
    }

    nRet = chmod(pszFilePath,nNewMode);
    if ( nRet < 0 )
    {
        nRet=errno;
#ifdef DEBUG_OSL_FILE
        perror("chmod");
#endif

#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_psz_setFileAttributes [chmod]\n");
#endif
        return oslTranslateFileError(nRet);
    }    

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out osl_psz_setFileAttributes [ok]\n");
#endif    
    return osl_File_E_None;
}


oslFileError osl_psz_setFileTime( sal_Char* pszFilePath,
                                  TimeValue* pCreationTime,
                                  TimeValue* pLastAccessTime,
                                  TimeValue* pLastWriteTime )
{
    int nRet=0;
    struct utimbuf aTimeBuffer;
    struct stat aFileStat;
#ifdef DEBUG_OSL_FILE
    struct tm* pTM=0;
#endif
    
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  osl_psz_setFileTime\n");
#endif

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"making lstat with '%s'\n",pszFilePath);
#endif
    
    nRet = lstat(pszFilePath,&aFileStat);
    if ( nRet < 0 )
    {
        nRet=errno;
#ifdef DEBUG_OSL_FILE
        perror("lstat");
#endif

#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_psz_setFileAttributes [lstat]\n");
#endif
        return oslTranslateFileError(nRet);
    }

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"File Times are (in localtime):\n");
    pTM=localtime(&aFileStat.st_ctime);
    fprintf(stderr,"CreationTime is '%s'\n",asctime(pTM));
    pTM=localtime(&aFileStat.st_atime);
    fprintf(stderr,"AccessTime   is '%s'\n",asctime(pTM));
    pTM=localtime(&aFileStat.st_mtime);
    fprintf(stderr,"Modification is '%s'\n",asctime(pTM));

    fprintf(stderr,"File Times are (in UTC):\n");    
    fprintf(stderr,"CreationTime is '%s'\n",ctime(&aFileStat.st_ctime));
    fprintf(stderr,"AccessTime   is '%s'\n",ctime(&aTimeBuffer.actime));
    fprintf(stderr,"Modification is '%s'\n",ctime(&aTimeBuffer.modtime));
#endif
    
    if ( pLastAccessTime != 0 )
    {
#ifdef DEBUG_OSL_FILE
        fprintf(stderr,"Changing Access Time!\n");
#endif
        aTimeBuffer.actime=pLastAccessTime->Seconds;
    }
    else
    {
#ifdef DEBUG_OSL_FILE
        fprintf(stderr,"Access Time unchanged!\n");
#endif
        aTimeBuffer.actime=aFileStat.st_atime;
    }
    
    if ( pLastWriteTime != 0 )
    {
#ifdef DEBUG_OSL_FILE
        fprintf(stderr,"Changing Modification Time!\n");
#endif
        aTimeBuffer.modtime=pLastWriteTime->Seconds;
    }
    else
    {
#ifdef DEBUG_OSL_FILE
        fprintf(stderr,"Modification Time unchanged!\n");
#endif
        aTimeBuffer.modtime=aFileStat.st_mtime;
    }
    
    /* mfe: Creation time not used here! */
    
#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"File Times are (in localtime):\n");    
    pTM=localtime(&aFileStat.st_ctime);
    fprintf(stderr,"CreationTime now '%s'\n",asctime(pTM));
    pTM=localtime(&aTimeBuffer.actime);
    fprintf(stderr,"AccessTime   now '%s'\n",asctime(pTM));
    pTM=localtime(&aTimeBuffer.modtime);
    fprintf(stderr,"Modification now '%s'\n",asctime(pTM));    

    fprintf(stderr,"File Times are (in UTC):\n");    
    fprintf(stderr,"CreationTime now '%s'\n",ctime(&aFileStat.st_ctime));
    fprintf(stderr,"AccessTime   now '%s'\n",ctime(&aTimeBuffer.actime));
    fprintf(stderr,"Modification now '%s'\n",ctime(&aTimeBuffer.modtime));
#endif
    
    nRet=utime(pszFilePath,&aTimeBuffer);
    if ( nRet < 0 )
    {
        nRet=errno;
#ifdef DEBUG_OSL_FILE
        perror("utime");
#endif

#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_psz_setFileTime [utime]\n");
#endif
        return oslTranslateFileError(nRet);
    }

#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_psz_setFileTime [ok]\n");
#endif
    return osl_File_E_None;
}



oslFileError osl_psz_removeFile( sal_Char* pszPath )
{
    int nRet=0;
    struct stat aStat;
    
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  osl_psz_removeFile\n");
#endif

    nRet = stat(pszPath,&aStat);
    if ( nRet < 0 )
    {
        nRet=errno;
#ifdef DEBUG_OSL_FILE
        perror("stat");
#endif
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_psz_removeFile [stat]\n");
#endif
        return oslTranslateFileError(nRet);        
    }
    
    if ( S_ISDIR(aStat.st_mode) )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_psz_removeFile [is dir]\n");
#endif
        return osl_File_E_ISDIR;
    }
    
    nRet = unlink(pszPath);
    if ( nRet < 0 )
    {
        nRet=errno;
#ifdef DEBUG_OSL_FILE
        perror("unlink");
#endif
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_psz_removeFile [unlink]\n");
#endif
        return oslTranslateFileError(nRet);
    }

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out osl_psz_removeFile [ok]\n");
#endif
    return osl_File_E_None;
}


static oslFileError osl_psz_createDirectory( sal_Char* pszPath )
{
    int nRet=0;
    int mode = S_IRWXU | S_IRWXG | S_IRWXO;

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  osl_psz_createDirectory\n");
#endif


#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"dir to create '%s'\n",pszPath);
#endif
    
    nRet = mkdir(pszPath,mode);
    if ( nRet < 0 )
    {
        nRet=errno;
#ifdef DEBUG_OSL_FILE
        perror("mkdir");
#endif
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_psz_createDirectory [mkdir]\n");
#endif
        return oslTranslateFileError(nRet);
    }

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out osl_psz_createDirectory [ok]\n");
#endif
    return osl_File_E_None;
}


static oslFileError osl_psz_removeDirectory( sal_Char* pszPath )
{
    int nRet=0;
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  osl_psz_removeDirectory\n");
#endif
    
#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"removing dir '%s'\n",pszPath);
#endif

    nRet = rmdir(pszPath);
    if ( nRet < 0 )
    {
        nRet=errno;
#ifdef DEBUG_OSL_FILE
        perror("rmdir");
#endif
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_psz_removeDirectory [rmdir]\n");
#endif
        return oslTranslateFileError(nRet);
    }

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out osl_psz_removeDirectory [ok]\n");
#endif
    return osl_File_E_None;
}


static oslFileError oslDoMoveFile(sal_Char* pszPath, sal_Char* pszDestPath)
{
    oslFileError tErr=osl_File_E_invalidError;

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  oslDoMoveFile\n");
#endif
    
    tErr = osl_psz_moveFile(pszPath,pszDestPath);
    if ( tErr == osl_File_E_None )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out oslDoMoveFile [rename succeeded]\n");
#endif
        return tErr;
    }    
        
    if ( tErr != osl_File_E_XDEV )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out oslDoMoveFile [something went wrong]\n");
#endif
        return tErr;
    }
    
    tErr=osl_psz_copyFile(pszPath,pszDestPath);
    if ( tErr != osl_File_E_None )
    {
        oslFileError tErrRemove;
        tErrRemove=osl_psz_removeFile(pszDestPath);
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out oslDoMoveFile [copy failed]\n");
#endif
        return tErr;
    }

    tErr=osl_psz_removeFile(pszPath);

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out oslDoMoveFile [ok]\n");
#endif
    return tErr;
}


oslFileError osl_psz_moveFile(sal_Char* pszPath, sal_Char* pszDestPath)
{

    int nRet = 0;

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  osl_moveFile\n");
#endif
    
#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"moving '%s' to '%s'\n",pszPath,pszDestPath);
#endif
    
    nRet = rename(pszPath,pszDestPath);
    if ( nRet < 0 )
    {
        nRet=errno;
#ifdef DEBUG_OSL_FILE
        fprintf(stderr,"rename failed : '%s'\n",strerror(nRet));
#endif
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_moveFile [rename]\n");
#endif
        return oslTranslateFileError(nRet);
    }
    
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out osl_moveFile [ok]\n");
#endif
    
    return osl_File_E_None;
}

/*******************************************
 osl_psz_copyFile
 ******************************************/
 
oslFileError osl_psz_copyFile( sal_Char* pszPath, sal_Char* pszDestPath )
{
/*      sal_Char* pszSourceFileName=0; */
/*      sal_Char* pszDestFileName=0; */
/*      sal_Char* pszUNCPath=0; */
    time_t nAcTime=0;
    time_t nModTime=0;
    uid_t nUID=0;
    gid_t nGID=0;
    int nRet=0;
    mode_t nMode=0;
    struct stat aFileStat;
    oslFileError tErr=osl_File_E_invalidError;
    size_t nSourceSize=0;
    int DestFileExists=1;

    /* mfe: does the source file really exists? */
    nRet = lstat(pszPath,&aFileStat);
    if ( nRet < 0 )
    {
        nRet=errno;
        return oslTranslateFileError(nRet);
    }

    /* mfe: we do only copy files here! */
    if ( S_ISDIR(aFileStat.st_mode) )
    {
        return osl_File_E_ISDIR;
    }

    nSourceSize=(size_t)aFileStat.st_size;
    nMode=aFileStat.st_mode;
    nAcTime=aFileStat.st_atime;
    nModTime=aFileStat.st_mtime;
    nUID=aFileStat.st_uid;
    nGID=aFileStat.st_gid;

    nRet = stat(pszDestPath,&aFileStat);
    if ( nRet < 0 ) 
    { 
        nRet=errno;
        if ( nRet == ENOENT )
        {
            DestFileExists=0;
        }
/*        return oslTranslateFileError(nRet);*/
    } 

    /* mfe: the destination file must not be a directory! */
    if ( nRet == 0 && S_ISDIR(aFileStat.st_mode) )
    {
#if 0
        return osl_File_E_ISDIR;
#else
		/* #98083# Targets must be files but we do not change previous behavior */
		return osl_File_E_None;
#endif
    }
    else
    {
        /* mfe: file does not exists or is no dir */
    }
    
    tErr = oslDoCopy(pszPath,pszDestPath,nMode,nSourceSize,DestFileExists);
    
    if ( tErr != osl_File_E_None )
    {
        return tErr;
    }

    /*
     *   mfe: ignore return code
     *        since only  the success of the copy is
     *        important
     */
    oslChangeFileModes(pszDestPath,nMode,nAcTime,nModTime,nUID,nGID);
    
    return tErr;
}


/******************************************************************************
 *
 *                  Utility Functions
 *
 *****************************************************************************/

static char * ImplSearchPath( char * buffer, size_t buflen, const char * filePath, const char * searchPath, char separator )
{
    char *pPathItem;
    char *pc;
    size_t nFilePathLen;
    
    pPathItem = searchPath;
    nFilePathLen = strlen( filePath );
    
    do
    {
        char path[PATH_MAX];
        size_t len;
        
        /* extract path item */
        pc = strchr( pPathItem, separator );
        len = pc ? pc - pPathItem : strlen( pPathItem );
        
        /* copy path entry to buffer and append file Path */
        if( PATH_MAX > len + nFilePathLen + 1 )
        {
            strncpy( path, pPathItem, len );
            path[len++] = '/';
            strcpy( path + len, filePath );
            
            if( 0 <= access( path, F_OK ) )
                return realpath( path, buffer );
        }
        
        pPathItem = pc + 1;
    }        
    while( NULL != pc ); 
    
    return NULL;
} 


sal_Char* oslDoGetAbsolutePath( sal_Char* pszBaseDir, sal_Char* pszRelDir )
{
    sal_Char* pszAbsDir=0;
    sal_Char* pszEntry=0;
    sal_Char* pszEntryNext=0;
    sal_Char* pszDirBase=0;
    int nRet=0;
    struct stat aFileStat;
    
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  oslDoGetAbsolutePath\n");
#endif

    if ( pszBaseDir[0] == '.' && pszBaseDir[1] == '\0' )
    {
        sal_Char pszCWD[PATH_MAX + 1];
        pszDirBase=getcwd(pszCWD,PATH_MAX+1);
        pszDirBase = (sal_Char*) rtl_allocateMemory(strlen(pszCWD)+8);
        if ( pszDirBase == 0 )
        {
#ifdef TRACE_OSL_FILE
            fprintf(stderr,"Out oslDoGetAbsolutePath [nomem]\n");
#endif
            return 0;            
        }
        strcpy(pszDirBase,pszCWD);
    }
    else
    {
        pszDirBase = (sal_Char*) rtl_allocateMemory(strlen(pszBaseDir)+8);
        if ( pszDirBase == 0 )
        {
#ifdef TRACE_OSL_FILE
            fprintf(stderr,"Out oslDoGetAbsolutePath [nomem]\n");
#endif
            return 0;            
        }
        strcpy(pszDirBase,pszBaseDir);        
    }
    
    
    pszAbsDir= (sal_Char*) rtl_allocateMemory(strlen(pszDirBase) + strlen(pszRelDir) + 4 );

    if ( pszAbsDir == 0 )
    {
        rtl_freeMemory(pszDirBase);
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out oslDoGetAbsolutePath [nomem]\n");
#endif
        return 0;
    }
    

    pszAbsDir[0]=0;
    pszEntry=pszDirBase+1;
    pszEntryNext=strchr(pszDirBase+1,'/');
    while ( pszEntry != 0 )
    {
        if ( pszEntryNext != 0 )
        {
            *pszEntryNext=0;
            ++pszEntryNext;
        }
        strcat(pszAbsDir,"/");
        strcat(pszAbsDir,pszEntry);
#ifdef DEBUG_OSL_FILE
        fprintf(stderr,"pszEntry == '%s'\n",pszEntry);
#endif
        if ( pszEntryNext != 0 )
        {
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"pszEntryNext == '%s'\n",pszEntryNext);
#endif
        }
#ifdef DEBUG_OSL_FILE
        fprintf(stderr,"pszAbsDir == '%s'\n",pszAbsDir);
#endif

        nRet = stat(pszAbsDir,&aFileStat);
        if (nRet < 0 )
        {
            nRet=errno;
#ifdef DEBUG_OSL_FILE
            perror("stat");
#endif
            rtl_freeMemory(pszAbsDir);
            rtl_freeMemory(pszDirBase);
            
#ifdef TRACE_OSL_FILE
            fprintf(stderr,"Out oslDoGetAbsolutePath [stat]\n");
#endif
            return 0;
            
/*            return oslTranslateFileError(nRet);*/
        }

        if ( ! S_ISDIR(aFileStat.st_mode) )
        {
            rtl_freeMemory(pszAbsDir);
            rtl_freeMemory(pszDirBase);
#ifdef TRACE_OSL_FILE
            fprintf(stderr,"Out oslDoGetAbsolutePath [not dir]\n");
#endif
            return 0;
            
/*            return osl_File_E_NOTDIR;*/
        }
        pszEntry=pszEntryNext;
        if ( pszEntry != 0 )
        {
            pszEntryNext=strchr(pszEntry,'/');
        }
    }


    pszEntry=pszRelDir;
    pszEntryNext=strchr(pszRelDir+1,'/');
    while ( pszEntry != 0 )
    {
        if ( pszEntryNext != 0 )
        {
            *pszEntryNext=0;
            ++pszEntryNext;
        }
#ifdef DEBUG_OSL_FILE
        fprintf(stderr,"pszEntry == '%s'\n",pszEntry);
#endif
        if ( pszEntryNext != 0 )
        {
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"pszEntryNext == '%s'\n",pszEntryNext);
#endif
        }

        if ( strcmp(pszEntry,"..") == 0 || strcmp(pszEntry,".") == 0 || pszEntry[0] == '/' )
        {
            if ( strcmp(pszEntry,"..") == 0 )
            {
				sal_Char* pChar = strrchr(pszAbsDir,'/');
				*pChar = 0;
            }
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"pszAbsDir == '%s'\n",pszAbsDir);
#endif
            
            pszEntry=pszEntryNext;
            if ( pszEntry != 0 )
            {
                pszEntryNext=strchr(pszEntry,'/');
            }
            continue;
        }
        strcat(pszAbsDir,"/");
        strcat(pszAbsDir,pszEntry);
#ifdef DEBUG_OSL_FILE
        fprintf(stderr,"pszAbsDir == '%s'\n",pszAbsDir);
#endif

        nRet = stat(pszAbsDir,&aFileStat);
        if ( nRet < 0 )
        {
            nRet=errno;
#ifdef DEBUG_OSL_FILE
            perror("stat");
#endif
/*            rtl_freeMemory(pszBaseDir);*/
            rtl_freeMemory(pszAbsDir);
            rtl_freeMemory(pszDirBase);

#ifdef TRACE_OSL_FILE
            fprintf(stderr,"Out oslDoGetAbsolutePath [stat]\n");
#endif
            return 0;
            
/*            return oslTranslateFileError(nRet);*/
        }

        if ( ! S_ISDIR(aFileStat.st_mode) && pszEntryNext != 0 )
        {
/*            rtl_freeMemory(pszBaseDir);*/
            rtl_freeMemory(pszAbsDir);
            rtl_freeMemory(pszDirBase);

#ifdef TRACE_OSL_FILE
            fprintf(stderr,"Out oslDoGetAbsolutePath [not dir]\n");
#endif
            return 0;
            
/*            return osl_File_E_NOTDIR;*/
        }

        pszEntry=pszEntryNext;
        if ( pszEntry != 0 )
        {
            pszEntryNext=strchr(pszEntry,'/');
        }
        
    }

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"pszAbsDir == '%s'\n",pszAbsDir);
#endif

    rtl_freeMemory(pszDirBase);
    
    
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out oslDoGetAbsolutePath [ok]\n");
#endif    
    return pszAbsDir;

}

/****************************************
 oslDoCopy
 ***************************************/
 
static oslFileError oslDoCopy(
    sal_Char* pszSourceFileName, 
    sal_Char* pszDestFileName, 
    mode_t nMode, 
    size_t nSourceSize, 
    int DestFileExists)
{
    int nRet=0;
    sal_Char pszTmpDestFile[PATH_MAX];
/*    sal_Char* pC;*/

    pszTmpDestFile[0] = '\0';
    
    if ( DestFileExists )
    {
        strcpy(pszTmpDestFile,pszDestFileName);
        strcat(pszTmpDestFile,".osl-tmp");
        /* FIXME: what if pszTmpDestFile already exists? */
        /*        with getcanonical??? */
        nRet=rename(pszDestFileName,pszTmpDestFile);
    }

    /* mfe: should be S_ISREG */
    if ( !S_ISLNK(nMode) )
    {
        /* copy SourceFile to DestFile */
        nRet = oslDoCopyFile(pszSourceFileName,pszDestFileName,nSourceSize, nMode);
    }
    /* mfe: OK redundant at the moment */
    else if ( S_ISLNK(nMode) )
    {
        nRet = oslDoCopyLink(pszSourceFileName,pszDestFileName);
    }
    else
    {
        /* mfe: what to do here? */
        nRet=ENOSYS;
    }

    if ( nRet > 0 && DestFileExists == 1 )
    {
        unlink(pszDestFileName);
        rename(pszTmpDestFile,pszDestFileName);
    }

    if ( nRet > 0 )
    {
        return oslTranslateFileError(nRet);
    }
    
    if ( DestFileExists == 1 )
    {
        unlink(pszTmpDestFile);
    }
    
    return osl_File_E_None;

}


static oslFileError oslChangeFileModes(sal_Char* pszFileName, mode_t nMode, time_t nAcTime, time_t nModTime, uid_t nUID, gid_t nGID)
{
    int nRet=0;
    struct utimbuf aTimeBuffer;

    nRet = chmod(pszFileName,nMode);
    if ( nRet < 0 )
    {
        nRet=errno;
#ifdef DEBUG_OSL_FILE
        perror("chmod");
#endif
        return oslTranslateFileError(nRet);
    }

    aTimeBuffer.actime=nAcTime;
    aTimeBuffer.modtime=nModTime;
    nRet=utime(pszFileName,&aTimeBuffer);
    if ( nRet < 0 )
    {
        nRet=errno;
#ifdef DEBUG_OSL_FILE
        perror("utime");
#endif
        return oslTranslateFileError(nRet);
    }

    if ( nUID != getuid() )
    {
        nUID=getuid();
    }

    nRet=chown(pszFileName,nUID,nGID);
    if ( nRet < 0 )
    {
        nRet=errno;
#ifdef DEBUG_OSL_FILE
        perror("chown");
#endif
        /* mfe: do not return an error here! */
        /* return oslTranslateFileError(nRet);*/
    }

    return osl_File_E_None;
}


static int oslDoCopyLink(sal_Char* pszSourceFileName, sal_Char* pszDestFileName)
{
    int nRet=0;

    /* mfe: if dest file is symbolic link remove the link and place the file instead (hro says so) */
    /* mfe: if source is a link copy the link and not the file it points to (hro says so) */
    sal_Char pszLinkContent[PATH_MAX];

    pszLinkContent[0] = '\0';

    nRet = readlink(pszSourceFileName,pszLinkContent,PATH_MAX);
    if ( nRet < 0 )
    {
        nRet=errno;
#ifdef DEBUG_OSL_FILE
        perror("readlink");
#endif
        return nRet;
    }
	else pszLinkContent[ nRet ] = 0;
    
    nRet = symlink(pszLinkContent,pszDestFileName);
    if ( nRet < 0 )
    {
        nRet=errno;
#ifdef DEBUG_OSL_FILE
        perror("symlink");
#endif
        return nRet;
    }
    return 0;
}

/****************************************
 oslDoCopyFile
 ***************************************/
 
static int oslDoCopyFile(
    sal_Char* pszSourceFileName, 
    sal_Char* pszDestFileName, 
    size_t nSourceSize,
    mode_t mode)
{
    oslVolumeInfo aVInfo;
    sal_Char* pC=0;
    rtl_uString* strVolInfo=0;
    oslFileError tErr=osl_File_E_invalidError;
    int SourceFileFD=0;
    int DestFileFD=0;
    int nRet=0;
    void* pSourceFile=0;

    
    /* mfe: simple check if we have enough space on the destination */
    memset(&aVInfo,0,sizeof(oslVolumeInfo));
    aVInfo.uStructSize=sizeof(oslVolumeInfo);

    pC = strrchr( pszDestFileName, '/' );
    if( 1 < pC )
    {
        char buffer[PATH_MAX];
        size_t len = pC - pszDestFileName;
        strncpy( buffer, pszDestFileName, len );
        buffer[len] = '\0';
		
    	rtl_string2UString(
        	&strVolInfo,
        	buffer,
        	len,
        	osl_getThreadTextEncoding(),
        	OUSTRING_TO_OSTRING_CVTFLAGS );
    }
    else
    	rtl_string2UString(
        	&strVolInfo,
        	pszDestFileName,
        	rtl_str_getLength( pszDestFileName ),
        	osl_getThreadTextEncoding(),
        	OUSTRING_TO_OSTRING_CVTFLAGS );
        
    osl_getFileURLFromSystemPath( strVolInfo, &strVolInfo );
    tErr = osl_getVolumeInformation(strVolInfo,&aVInfo,osl_VolumeInfo_Mask_FreeSpace);

/* 	#104294# On Suse Linux 7.3, 8.0 there's abug that mmap on NFS mounted volumes only works with a previous statfs.
	So we roll back the code from previous revision but do not return in case of an error. Bug in Linux kernel is fixed in Suse 8.1
	*/

/*		
    if ( tErr != osl_File_E_None )
    {
        return EIO;
    }
    
    if ( nSourceSize > aVInfo.uFreeSpace )
    {
        return ENOSPC;
    }

*/

    SourceFileFD=open(pszSourceFileName,O_RDONLY);
    if ( SourceFileFD < 0 )
    {
        nRet=errno;
        return nRet;
    }
    
    DestFileFD=open(pszDestFileName,O_WRONLY|O_CREAT, mode);
    if ( DestFileFD < 0 )
    {
        nRet=errno;
        close(SourceFileFD);
        return nRet;
    }

    /* HACK: because memory mapping fails on various 
	   platforms if the size of the source file is 
	   0 byte 
	*/
	if (0 == nSourceSize)
	{				
		fsync(DestFileFD);
		close(SourceFileFD);
		close(DestFileFD);
		return 0;
	}
	
    /* FIXME doCopy: fall back code for systems not having mmap */
    /* mmap file -- open dest file -- write once -- fsync it */
    pSourceFile=mmap(0,nSourceSize,PROT_READ,MAP_PRIVATE,SourceFileFD,0);
    if ( pSourceFile == MAP_FAILED )
    {
        nRet=errno;
        close(SourceFileFD);
        close(DestFileFD);
        return nRet;
    }

    nRet=write(DestFileFD,pSourceFile,nSourceSize);
    if ( nRet < 0 )
    {
        nRet=errno;
        close(SourceFileFD);
        close(DestFileFD);
        nRet = munmap((char*)pSourceFile,nSourceSize);
        return nRet;
    }

    nRet = fsync(DestFileFD);
    if ( nRet < 0 )
    {
        nRet=errno;
        close(SourceFileFD);
        close(DestFileFD);
        nRet = munmap((char*)pSourceFile,nSourceSize);
        return nRet;
    }

    nRet = munmap((char*)pSourceFile,nSourceSize);
    if ( nRet < 0 )
    {
        nRet=errno;
        close(SourceFileFD);
        return nRet;
    }

    close(SourceFileFD);
    close(DestFileFD);

    return 0;
}


static rtl_uString* oslMakeUStrFromPsz(sal_Char* pszStr, rtl_uString** ustrValid)
{

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"Enter - oslMakeUStrFromPsz\n");
#endif
    
    rtl_string2UString(
        ustrValid,
        pszStr,
        rtl_str_getLength( pszStr ),
        osl_getThreadTextEncoding(),
        OUSTRING_TO_OSTRING_CVTFLAGS );

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"oslMakeUStrFromPsz : new UString is ");
    debug_ustring(*ustrValid);
#endif

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"Leave - oslMakeUStrFromPsz\n");
#endif
    
    return *ustrValid;
}


static sal_Char* oslMakePszFromUStr(rtl_uString* uStr, sal_Char* pszStr)
{
    rtl_String* aStr=0;
    sal_Char* pszPath=0;
    
    rtl_uString2String( &aStr,
                        rtl_uString_getStr(uStr),
                        rtl_uString_getLength(uStr),
                        osl_getThreadTextEncoding(),
                        OUSTRING_TO_OSTRING_CVTFLAGS );
    if ( aStr != 0 )
    {    
        pszPath = rtl_string_getStr(aStr);
        OSL_ENSURE(pszPath,"oslMakePszFromUStr : Could not convert UString to char*!!!\n");
        strcpy(pszStr,pszPath);
        rtl_string_release(aStr);
        return pszStr;
    }
    
    return 0;
}


static oslFileError oslTranslateFileError(int nErr)
{
    oslFileError tErr = osl_File_E_invalidError;
#ifdef TRACE_OSL_FILE
/*    fprintf(stderr,"In  oslTranslateFileError (err == %i [%s])\n",nErr,sys_errlist[nErr]);*/
#endif
    
    switch(nErr)
    {
        case 0:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_None");
#endif
            tErr=osl_File_E_None;
            break;
            
        case EPERM:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_PERM");
#endif
            tErr = osl_File_E_PERM;
            break;

        case ENOENT:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_NOENT");
#endif
            tErr = osl_File_E_NOENT;
            break;

        case ESRCH:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_SRCH");
#endif
            tErr = osl_File_E_SRCH;
            break;

        case EINTR:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_INTR");
#endif
            tErr = osl_File_E_INTR;
            break;

        case EIO:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_IO");
#endif
            tErr = osl_File_E_IO;
            break;

        case ENXIO:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_NXIO");
#endif
            tErr = osl_File_E_NXIO;
            break;
            
        case E2BIG:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_2BIG");
#endif
            tErr = osl_File_E_2BIG;
            break;
            
        case ENOEXEC:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_NOEXEC");
#endif
            tErr = osl_File_E_NOEXEC;
            break;

        case EBADF:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_BADF");
#endif
            tErr = osl_File_E_BADF;
            break;

        case ECHILD:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_CHILD");
#endif
            tErr = osl_File_E_CHILD;
            break;

        case EAGAIN:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_AGAIN");
#endif
            tErr = osl_File_E_AGAIN;
            break;

        case ENOMEM:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_NOMEM");
#endif
            tErr = osl_File_E_NOMEM;
            break;

        case EACCES:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_ACCES");
#endif
            tErr = osl_File_E_ACCES;
            break;

        case EFAULT:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_FAULT");
#endif
            tErr = osl_File_E_FAULT;
            break;

        case EBUSY:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_BUSY");
#endif
            tErr = osl_File_E_BUSY;
            break;

        case EEXIST:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_EXIST");
#endif
            tErr = osl_File_E_EXIST;
            break;
            
        case EXDEV:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_XDEV");
#endif
            tErr = osl_File_E_XDEV;
            break;
            
        case ENODEV:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_NODEV");
#endif
            tErr = osl_File_E_NODEV;
            break;
            
        case ENOTDIR:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_NOTDIR");
#endif
            tErr = osl_File_E_NOTDIR;
            break;

        case EISDIR:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_ISDIR");
#endif
            tErr = osl_File_E_ISDIR;
            break;

        case EINVAL:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_INVAL");
#endif
            tErr = osl_File_E_INVAL;
            break;
            
        case ENFILE:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_NFILE");
#endif
            tErr = osl_File_E_NFILE;
            break;

        case EMFILE:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_MFILE");
#endif
            tErr = osl_File_E_MFILE;
            break;

        case ENOTTY:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_NOTTY");
#endif
            tErr = osl_File_E_NOTTY;
            break;

        case EFBIG:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_FBIG");
#endif
            tErr = osl_File_E_FBIG;
            break;

        case ENOSPC:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_NOSPC");
#endif
            tErr = osl_File_E_NOSPC;
            break;

        case ESPIPE:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_SPIPE");
#endif
            tErr = osl_File_E_SPIPE;
            break;
            
        case EROFS:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_ROFS");
#endif
            tErr = osl_File_E_ROFS;
            break;

        case EMLINK:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_MLINK");
#endif
            tErr = osl_File_E_MLINK;
            break;
            
        case EPIPE:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_PIPE");
#endif
            tErr = osl_File_E_PIPE;
            break;

        case EDOM:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_DOM");
#endif
            tErr = osl_File_E_DOM;
            break;

        case ERANGE:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_RANGE");
#endif
            tErr = osl_File_E_RANGE;
            break;

        case EDEADLK:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_DEADLK");
#endif
            tErr = osl_File_E_DEADLK;
            break;
            
        case ENAMETOOLONG:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_NAMETOOLONG");
#endif
            tErr = osl_File_E_NAMETOOLONG;
            break;
            
        case ENOLCK:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_NOLCK");
#endif
            tErr = osl_File_E_NOLCK;
            break;

        case ENOSYS: 
#ifdef DEBUG_OSL_FILE
           fprintf(stderr,"osl_File_E_NOSYS");
#endif
           tErr = osl_File_E_NOSYS;
            break;

        case ENOTEMPTY:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_NOTEMPTY");
#endif
            tErr = osl_File_E_NOTEMPTY;
            break;

        case ELOOP:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_LOOP");
#endif
            tErr = osl_File_E_LOOP;
            break;

#if !(defined(MACOSX) || defined(NETBSD) || defined(FREEBSD))
        case EILSEQ:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_ILSEQ");
#endif
            tErr = osl_File_E_ILSEQ;
            break;
#endif /* MACOSX */
            
#if !(defined(MACOSX) || defined(NETBSD) || defined(FREEBSD))
        case ENOLINK:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_NOLINK");
#endif
            tErr = osl_File_E_NOLINK;
            break;
#endif /* MACOSX */
            
#if !(defined(MACOSX) || defined(NETBSD) || defined(FREEBSD))
        case EMULTIHOP:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_MULTIHOP");
#endif
            tErr = osl_File_E_MULTIHOP;
            break;
#endif /* MACOSX */

        case EUSERS:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_USERS");
#endif
            tErr = osl_File_E_USERS;
            break;

        case EOVERFLOW:
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_OVERFLOW");
#endif
            tErr = osl_File_E_OVERFLOW;
            break;
            
        default:
            /* FIXME translateFileError: is this alright? Or add a new one: osl_File_E_Unknown? */
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"osl_File_E_Unknown");
#endif
            tErr = osl_File_E_invalidError;
            break;
    }

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"\n");
#endif
    
#ifdef TRACE_OSL_FILE
/*    fprintf(stderr,"Out oslTranslateFileError\n");*/
#endif
    return tErr;
}


/****************************************************************************/
/* UnicodeToText */
/****************************************************************************/

/* 
 * converting unicode to text manually saves us the penalty of a temporary 
 * rtl_String object.
 */

int UnicodeToText( char * buffer, size_t bufLen, const sal_Unicode * uniText, sal_Int32 uniTextLen )
{
    rtl_UnicodeToTextConverter hConverter;
    sal_Size   nInfo;
    sal_Size   nSrcChars, nDestBytes;
    
    /* stolen from rtl/string.c */
    hConverter = rtl_createUnicodeToTextConverter( osl_getThreadTextEncoding() );
    
	nDestBytes = rtl_convertUnicodeToText( hConverter, 0, uniText, uniTextLen,
                                           buffer, bufLen,
                                           OUSTRING_TO_OSTRING_CVTFLAGS | RTL_UNICODETOTEXT_FLAGS_FLUSH,
                                           &nInfo, &nSrcChars );
    
    rtl_destroyUnicodeToTextConverter( hConverter );
   
    if( nInfo & RTL_UNICODETOTEXT_INFO_DESTBUFFERTOSMALL )
    {
        errno = EOVERFLOW;
        return 0;
    }
        
    /* ensure trailing '\0' */
    buffer[nDestBytes] = '\0';
    
    return nDestBytes;
}


/****************************************************************************/
/* FileURLToPath */
/****************************************************************************/

oslFileError FileURLToPath( char * buffer, size_t bufLen, rtl_uString* ustrFileURL )
{
    rtl_uString* ustrSystemPath = NULL;
    oslFileError eRet = osl_File_E_None;
    
    /* convert file URL to system path */ 
    eRet = osl_getSystemPathFromFileURL( ustrFileURL, &ustrSystemPath );
    if( osl_File_E_None != eRet )
        return eRet;

    /* remove trailing '/' if any */
    if( ( 1 < ustrSystemPath->length ) && ( (sal_Unicode) '/' == ustrSystemPath->buffer[ustrSystemPath->length - 1] ) )
    {
        ustrSystemPath->length--;
        ustrSystemPath->buffer[ustrSystemPath->length] = 0;
    }

    /* convert unicode path to text */
    if( ! UnicodeToText( buffer, bufLen, ustrSystemPath->buffer, ustrSystemPath->length ) )
        eRet = osl_File_E_OVERFLOW;
        
    rtl_uString_release( ustrSystemPath );
    return eRet;
}


#if 0
/* mfe: this one is not used at all */
oslFileError osl_getQuota(oslDirectoryItem Item, sal_uInt32* SoftLimit, sal_uInt32* HardLimit, sal_uInt32* TimeLeft)
{
    int nRet=0;
    sal_Char* pszFileName=0;

#if (defined(LINUX) || defined(NETBSD) || defined(FREEBSD))
    struct dqblk aQuota;
    int QuotaCommand;
#endif /* LINUX */

#if defined(SOLARIS)
    int fd;
    struct quotctl aQuotaCtl;
    struct dqblk aQuota;
#endif /* SOLARIS */

#if defined(MACOSX)
    struct dqblk aQuota;
    int QuotaCommand;
#endif /* MACOSX */

#if defined(IRIX)
    struct dqblk aQuota;
    int QuotaCommand;
#endif /* IRIX */

    oslDirectoryItemImpl* pDirItemImpl = (oslDirectoryItemImpl*) Item;

    if ( pDirItemImpl == 0 )
    {
        *SoftLimit=0;
        *HardLimit=0;
        *TimeLeft=0;
        return osl_File_E_INVAL;
    }

    if ( pDirItemImpl->pszFileName == 0 )
    {
        *SoftLimit=0;
        *HardLimit=0;
        *TimeLeft=0;
        return osl_File_E_INVAL;
    }

/*      rtl_uString_acquire(pDirItemImpl->osl_FileName); */
/*      pszFileName=oslGetNativeName(pDirItemImpl->osl_FileName); */
/*      if ( pszFileName == 0 ) */
/*      { */
/*          rtl_uString_release(pDirItemImpl->osl_FileName); */
/*          *SoftLimit=0; */
/*          *HardLimit=0; */
/*          *TimeLeft=0; */
/*          return osl_File_E_INVAL; */
/*      } */

#if defined(LINUX)
    QuotaCommand = QCMD(Q_GETQUOTA,USRQUOTA);
    nRet = quotactl (QuotaCommand, pszFileName, getuid(), (caddr_t)&aQuota);
#endif /* LINUX */

#if defined(FREEBSD)
    QuotaCommand = QCMD(Q_GETQUOTA,USRQUOTA);
    nRet = quotactl (pszFileName, QuotaCommand, getuid(), (caddr_t)&aQuota);
#endif /* FREEBSD */

#if defined(SOLARIS)
    fd = open(pszFileName,O_RDONLY);
    if ( fd < 0 )
    {
        nRet=errno;        
#ifdef DEBUG_OSL_FILE
        perror("open");
#endif
/*          rtl_uString_release(pDirItemImpl->osl_FileName); */
        rtl_freeMemory(pszFileName);
        
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_getQuota [open]\n");
#endif        
        return oslTranslateFileError(nRet);
    }

    aQuotaCtl.op=Q_GETQUOTA;
    aQuotaCtl.uid=getuid();
    aQuotaCtl.addr=(char*)&aQuota;
    nRet = ioctl(fd, Q_QUOTACTL, &aQuota);
    close(fd);
#endif /* SOLARIS */

#if defined(IRIX)
    nRet = quotactl (Q_GETQUOTA, pszFileName, getuid(), (caddr_t)&aQuota);
#endif /* IRIX */

/*      rtl_uString_release(pDirItemImpl->osl_FileName); */

    if ( nRet != 0 )
    {
        nRet=errno;
#ifdef DEBUG_OSL_FILE
        perror("quota");
#endif
        *SoftLimit=0;
        *HardLimit=0;
        *TimeLeft=0;
        rtl_freeMemory(pszFileName);
        
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_getQuota [quota]\n");
#endif        
        return oslTranslateFileError(nRet);
    }

    *SoftLimit=aQuota.dqb_bsoftlimit;
    *HardLimit=aQuota.dqb_bhardlimit;
#if defined(LINUX)
    *TimeLeft=aQuota.dqb_btime;
#endif /* LINUX */

#if defined (SOLARIS)
    *TimeLeft=aQuota.dqb_btimelimit;
#endif /* SOLARIS */

    return osl_File_E_None;
}

#endif

#ifdef FREEBSD

#include<fstab.h>

static oslFileError oslIsMountPoint(sal_Char* pszFileName, sal_Bool* bMountPoint )
{
 struct fstab *fs;
		 
 setfsent();
 fs = getfsfile(pszFileName);
 endfsent();

 if (fs == NULL)
     *bMountPoint=sal_False;
 else
     *bMountPoint=sal_True;

 return osl_File_E_None;
}

#else

#ifdef MACOSX
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#endif

static oslFileError oslIsMountPoint(sal_Char* pszFileName, sal_Bool* bMountPoint )
{
    int nRet=0;
    FILE* mntfile=0;
    sal_Char buffer[PATH_MAX];
    
#if defined(LINUX) || defined(IRIX)
    struct mntent* pMountEnt=0;
    mntfile = setmntent(MOUNTTAB,"r");
#endif /* LINUX || IRIX */

#if defined(SOLARIS)
    struct mnttab MountEnt;
    mntfile = fopen(MOUNTTAB,"r");
#endif /* SOLARIS */

#if defined(NETBSD) || defined(FREEBSD) || defined(MACOSX)
    struct statfs *mntbufp;
    int mntentries,i;
    mntentries = getmntinfo(&mntbufp,MNT_WAIT);

    buffer[0] = '\0';
    
    if(mntentries == 0)
#else    

    buffer[0] = '\0';

    if ( mntfile == 0 )
#endif
    {
        nRet=errno;
#ifdef DEBUG_OSL_FILE
        perror("mounttab");
#endif
        return oslTranslateFileError(nRet);
    }

    strcpy(buffer,pszFileName);
#ifdef DEBUG_OSL_FILE
/*    fprintf(stderr,"Checking mount of %s\n",buffer);*/
#endif

    /* FIXME isMountPoint: THIS IS NOT THREAD SAFE ! */
#if defined(LINUX)
    pMountEnt=getmntent(mntfile);
    while ( pMountEnt != 0 )
    {
#ifdef DEBUG_OSL_FILE
/*        fprintf(stderr,"mnt_fsname : %s\n",pMountEnt->mnt_fsname);*/
/*        fprintf(stderr,"mnt_dir    : %s\n",pMountEnt->mnt_dir);*/
/*        fprintf(stderr,"mnt_type   : %s\n",pMountEnt->mnt_type);*/
#endif
        if ( strcmp(pMountEnt->mnt_dir,buffer) == 0 )
        {
            fclose(mntfile);
            *bMountPoint=sal_True;
            return osl_File_E_None;
        }
#ifdef DEBUG_OSL_FILE
/*        fprintf(stderr,"=================\n");*/
#endif
        pMountEnt=getmntent(mntfile);
    }
#endif /* LINUX */

#if defined(SOLARIS)
    nRet=getmntent(mntfile,&MountEnt);
    while ( nRet == 0 )
    {
#ifdef DEBUG_OSL_FILE
/*        fprintf(stderr,"mnt_special : %s\n",MountEnt.mnt_special);*/
/*        fprintf(stderr,"mnt_mountp  : %s\n",MountEnt.mnt_mountp);*/
/*        fprintf(stderr,"mnt_type    : %s\n",MountEnt.mnt_fstype);*/
#endif
        if ( strcmp(MountEnt.mnt_mountp,buffer) == 0 )
        {
            fclose(mntfile);
            *bMountPoint=sal_True;
            return osl_File_E_None;
        }
#ifdef DEBUG_OSL_FILE
/*        fprintf(stderr,"=================\n");*/
#endif
        nRet=getmntent(mntfile,&MountEnt);
    }
#endif /* SOLARIS */

#if defined(NETBSD) || defined(FREEBSD) || defined(MACOSX)
    i=0;
    while ( i < mntentries )
    {
       if ( strcmp(mntbufp[i].f_mntonname,buffer) == 0 )
       {
           *bMountPoint=sal_True;
           return osl_File_E_None;
       }
       i++;
    }
#else
    fclose(mntfile);
#endif
    *bMountPoint=sal_False;
    return osl_File_E_None;
}
#endif

static sal_Char* oslSeparatePathEntry(sal_Char* pszPath, sal_Char cSeparator)
{
    sal_Char* pC=0;
    
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  oslSeparatePathEntry\n");
#endif

    pC=strchr(pszPath,cSeparator);
    
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out oslSeparatePathEntry [ok]\n");
#endif
    return pC;
}


static oslFileError osl_psz_CheckForExistence(sal_Char* pszFilePath)
{
    int nRet=0;
    
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  osl_psz_CheckForExistence\n");
#endif

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"making access test with '%s'\n",pszFilePath);
#endif
    
    nRet=access(pszFilePath,F_OK);
    if ( nRet < 0 )
    {
        nRet=errno;
#ifdef DEBUG_OSL_FILE
        perror("access");
#endif

#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_psz_CheckForExistence [access]\n");
#endif
        return oslTranslateFileError(nRet);
    }
    
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out osl_psz_CheckForExistence [ok]\n");
#endif
    
    return osl_File_E_None;
}


static void oslMakeUniqueName(sal_Char* pszRequested, sal_Char* pszUnique, sal_uInt32 nIndex)
{
    sal_Char pszTmpRequested[PATH_MAX];
    sal_Char* pC=0;

    pszTmpRequested[0] = '\0';
    
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  oslMakeUniqueName\n");
#endif
    
    if ( pszUnique == 0 )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out oslMakeUniqueName [pszUnique==0]\n");
#endif
        return;
    }

    if ( pszRequested == 0 )
    {
        pszUnique[0]=0;
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out oslMakeUniqueName [pszRequested==0]\n");
#endif
        return;
    }

    strcpy(pszTmpRequested,pszRequested);
    
    if ( ( pC = strrchr(pszRequested,'.') ) != 0 )
    {
        *pC=0;
        ++pC;
        sprintf(pszUnique,"%s-%lu.%s",pszTmpRequested,nIndex,pC);
    }
    else
    {
        sprintf(pszUnique,"%s-%lu",pszRequested,nIndex);
    }
    
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out oslMakeUniqueName [%s]\n",pszUnique);
#endif
    return;
}


static sal_Char* oslGetDirName(sal_Char* pszPath)
{
    sal_Char* pC=0;

    if ( (pC=strrchr(pszPath,'/')) == 0 )
    {
        return 0;
    }
    else
    {
        *(pC+1)=0;
    }
    return pszPath;
}


static sal_Char* oslGetBaseName(sal_Char* pszPath)
{
    sal_Char* pC=0;

    if ( (pC=strrchr(pszPath,'/')) == 0 )
    {
        return 0;
    }
    return pC+1;
}


#if defined(SOLARIS)
static sal_Char* osl_getDirName(sal_Char* pszPath)
{
    sal_Char* pC;

    if ( (pC=strrchr(pszPath,'/')) == 0 )
    {
        if ( pszPath[1] == 0 )
        {
            *pC = 0;
        }
        
        return 0;
    }
    else
    {
        *pC=0;
    }
    return pszPath;
}


static sal_Char* osl_getBaseName(sal_Char* pszPath)
{
    sal_Char* pC;

    if ( (pC=strrchr(pszPath,'/')) == 0 )
    {
        return 0;
    }
    return (pC+1);
}
#endif


/******************************************************************************
 *
 *                  GENERIC FLOPPY FUNCTIONS
 *
 *****************************************************************************/

oslFileError osl_unmountVolumeDevice( oslVolumeDeviceHandle Handle )
{
    oslFileError tErr = osl_File_E_NOSYS;

    tErr = osl_unmountFloppy(Handle);
	
 	/* Perhaps current working directory is set to mount point */
	
 	if ( tErr )
	{
		sal_Char *pszHomeDir = getenv("HOME");

		if ( pszHomeDir && strlen( pszHomeDir ) && 0 == chdir( pszHomeDir ) )
		{
			/* try again */	
			
    		tErr = osl_unmountFloppy(Handle);
			
			OSL_ENSURE( tErr, "osl_unmountvolumeDevice: CWD was set to volume mount point" );
		}
	}

    return tErr;
}


oslFileError osl_automountVolumeDevice( oslVolumeDeviceHandle Handle )
{
    oslFileError tErr = osl_File_E_NOSYS;

    tErr = osl_mountFloppy(Handle);

    return tErr;
}


oslFileError osl_getVolumeDeviceMountPath( oslVolumeDeviceHandle Handle, rtl_uString **pstrPath )
{
    oslVolumeDeviceHandleImpl* pItem = (oslVolumeDeviceHandleImpl*) Handle;
    sal_Char Buffer[PATH_MAX];

    Buffer[0] = '\0';
    
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"returning mount path: %s\n", pItem->pszMountPoint);
#endif    
     
    if ( pItem == 0 || pstrPath == 0 )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_getVolumeDeviceMountPath [INVAL]\n");
#endif     
        return osl_File_E_INVAL;
    }

    if ( pItem->ident[0] != 'O' || pItem->ident[1] != 'V' || pItem->ident[2] != 'D' || pItem->ident[3] != 'H' )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_getVolumeDeviceMountPath [INVAL]\n");
#endif     
        return osl_File_E_INVAL;
    }

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"Handle is:\n");
    osl_printFloppyHandle(pItem);
#endif

    sprintf(Buffer,"file://%s",pItem->pszMountPoint);

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"Mount Point is: '%s'\n",Buffer);
#endif
    
    oslMakeUStrFromPsz(Buffer, pstrPath);

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out osl_getVolumeDeviceMountPath [ok]\n");
#endif     
    
    return osl_File_E_None;
}


oslFileError SAL_CALL osl_acquireVolumeDeviceHandle( oslVolumeDeviceHandle Handle )
{
    oslVolumeDeviceHandleImpl* pItem =(oslVolumeDeviceHandleImpl*) Handle;

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  osl_acquireVolumeDeviceHandle\n");
#endif     
    
    if ( pItem == 0 )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_acquireVolumeDeviceHandle [INVAL]\n");
#endif     
        return osl_File_E_INVAL;
    }
    
    if ( pItem->ident[0] != 'O' || pItem->ident[1] != 'V' || pItem->ident[2] != 'D' || pItem->ident[3] != 'H' )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_acquireVolumeDeviceHandle [INVAL]\n");
#endif        
        return osl_File_E_INVAL;
    }

    ++pItem->RefCount;

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out osl_acquireVolumeDeviceHandle [ok]\n");
#endif     
    
    return osl_File_E_None;
}


oslFileError osl_releaseVolumeDeviceHandle( oslVolumeDeviceHandle Handle )
{
    oslVolumeDeviceHandleImpl* pItem =(oslVolumeDeviceHandleImpl*) Handle;

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  osl_releaseVolumeDeviceHandle\n");
#endif
    
    if ( pItem == 0 )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_releaseVolumeDeviceHandle [INVAL]\n");
#endif
        return osl_File_E_INVAL;
    }
    
    if ( pItem->ident[0] != 'O' || pItem->ident[1] != 'V' || pItem->ident[2] != 'D' || pItem->ident[3] != 'H' )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_releaseVolumeDeviceHandle [INVAL]\n");
#endif
        return osl_File_E_INVAL;
    }

    --pItem->RefCount;

    if ( pItem->RefCount < 0 )
    {
#ifdef DEBUG_OSL_FILE
        fprintf(stderr,"negativ RefCount in Handle!!!\n");
#endif
    }
    
    if ( pItem->RefCount == 0 )
    {
#ifdef DEBUG_OSL_FILE
        fprintf(stderr,"freeing!!!\n");
#endif
        rtl_freeMemory(pItem);
    }

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out osl_releaseVolumeDeviceHandle [ok]\n");
#endif
    
    return osl_File_E_None;
}

/*
 * ctor/dtor for oslVolumeDeviceHandleImpl
 */

static oslVolumeDeviceHandleImpl*
osl_newVolumeDeviceHandleImpl()
{
    oslVolumeDeviceHandleImpl* pHandle;
    const size_t               nSizeOfHandle = sizeof(oslVolumeDeviceHandleImpl);
 
    pHandle = (oslVolumeDeviceHandleImpl*) rtl_allocateMemory (nSizeOfHandle);
    if (pHandle != NULL)
    {
        pHandle->ident[0]         = 'O';
        pHandle->ident[1]         = 'V';
        pHandle->ident[2]         = 'D';
        pHandle->ident[3]         = 'H';
        pHandle->pszMountPoint[0] = '\0';
        pHandle->pszFilePath[0]   = '\0';
        pHandle->pszDevice[0]     = '\0';
        pHandle->RefCount         = 1;
    }
    return pHandle;   
}

static void
osl_freeVolumeDeviceHandleImpl (oslVolumeDeviceHandleImpl* pHandle)
{
    if (pHandle != NULL)
        rtl_freeMemory (pHandle);
}

/******************************************************************************
 *
 *                  SOLARIS FLOPPY FUNCTIONS
 *
 *****************************************************************************/

#if defined(SOLARIS)
/* compare a given devicename with the typical device names on a Solaris box */
static sal_Bool
osl_isAFloppyDevice (const char* pDeviceName)
{
    const char* pFloppyDevice [] = { 
        "/dev/fd",           "/dev/rfd", 
        "/dev/diskette",     "/dev/rdiskette", 
        "/vol/dev/diskette", "/vol/dev/rdiskette" 
    };

    int i;
    for (i = 0; i < (sizeof(pFloppyDevice)/sizeof(pFloppyDevice[0])); i++)
    {
        if (strncmp(pDeviceName, pFloppyDevice[i], strlen(pFloppyDevice[i])) == 0)
            return sal_True;
    }
    return sal_False;
}

/* compare two directories whether the first may be a parent of the second. this
 * does not realpath() resolving */
static sal_Bool
osl_isAParentDirectory (const char* pParentDir, const char* pSubDir)
{
    return strncmp(pParentDir, pSubDir, strlen(pParentDir)) == 0; 
}

/* the name of the routine is obviously silly. But anyway create a 
 * oslVolumeDeviceHandle with correct mount point, device name and a resolved filepath
 * only if pszPath points to file or directory on a floppy */
static oslVolumeDeviceHandle 
osl_isFloppyDrive(const sal_Char* pszPath)
{
    FILE*                       pMountTab;
    struct mnttab               aMountEnt;
    oslVolumeDeviceHandleImpl*  pHandle;

    if ((pHandle = osl_newVolumeDeviceHandleImpl()) == NULL)
    {
        return NULL;
    }
    if (realpath(pszPath, pHandle->pszFilePath) == NULL)
    {
        osl_freeVolumeDeviceHandleImpl (pHandle);
        return NULL;
    }
    if ((pMountTab = fopen (MOUNTTAB, "r")) == NULL)
    {
        osl_freeVolumeDeviceHandleImpl (pHandle);
        return NULL;
    }

    while (getmntent(pMountTab, &aMountEnt) == 0)
    {
        const char *pMountPoint = aMountEnt.mnt_mountp;
        const char *pDevice     = aMountEnt.mnt_special; 
        if (   osl_isAParentDirectory (aMountEnt.mnt_mountp, pHandle->pszFilePath)
            && osl_isAFloppyDevice    (aMountEnt.mnt_special))
        {
            /* skip the last item for it is the name of the disk */
            char * pc = strrchr( aMountEnt.mnt_special, '/' );
            
            if ( NULL != pc )
            {
                int len = pc - aMountEnt.mnt_special;
                                
                strncpy( pHandle->pszDevice, aMountEnt.mnt_special, len );
                pHandle->pszDevice[len] = '\0';
            }
            else
               strcpy( pHandle->pszDevice, aMountEnt.mnt_special );
            
            /* remember the mount point */
            strcpy(pHandle->pszMountPoint, aMountEnt.mnt_mountp);
            
            fclose (pMountTab);
            return pHandle;
        }
    }

    fclose (pMountTab);
    osl_freeVolumeDeviceHandleImpl (pHandle);
    return NULL;
}

static oslFileError osl_mountFloppy(oslVolumeDeviceHandle hFloppy)
{
    FILE*                       pMountTab;
    struct mnttab               aMountEnt;
    oslVolumeDeviceHandleImpl*  pHandle = (oslVolumeDeviceHandleImpl*) hFloppy;

    int nRet=0;
    sal_Char pszCmd[512] = "";
   
    if ( pHandle == 0 )
        return osl_File_E_INVAL;

    /* FIXME: don't know what this is good for */    
    if ( pHandle->ident[0] != 'O' || pHandle->ident[1] != 'V' || pHandle->ident[2] != 'D' || pHandle->ident[3] != 'H' )
        return osl_File_E_INVAL;

    sprintf( pszCmd,"eject -q %s > /dev/null 2>&1", pHandle->pszDevice );
    nRet = system( pszCmd );

    switch ( WEXITSTATUS(nRet) )
    {
    case 0:        
        {
            /* lookup the device in mount tab again */
            if ((pMountTab = fopen (MOUNTTAB, "r")) == NULL)
                return osl_File_E_BUSY;

            while (getmntent(pMountTab, &aMountEnt) == 0)
            {
                const char *pMountPoint = aMountEnt.mnt_mountp;
                const char *pDevice     = aMountEnt.mnt_special; 
                if ( 0 == strncmp( pHandle->pszDevice, aMountEnt.mnt_special, strlen(pHandle->pszDevice) ) )
                {
                    strcpy (pHandle->pszMountPoint, aMountEnt.mnt_mountp);

                    fclose (pMountTab);
                    return osl_File_E_None;
                }
            }

            fclose (pMountTab);
            return osl_File_E_BUSY;
        }
        break;
        
    case 1:
        return osl_File_E_BUSY;
        break;

    default:
        return osl_File_E_BUSY;
        break;
    }

    return osl_File_E_BUSY;
}

static oslFileError osl_unmountFloppy(oslVolumeDeviceHandle hFloppy)
{
    FILE*                       pMountTab;
    struct mnttab               aMountEnt;
    oslVolumeDeviceHandleImpl*  pHandle = (oslVolumeDeviceHandleImpl*) hFloppy;    
    
    int nRet=0;
    sal_Char pszCmd[512] = "";
    
    if ( pHandle == 0 )
        return osl_File_E_INVAL;

    /* FIXME: don't know what this is good for */    
    if ( pHandle->ident[0] != 'O' || pHandle->ident[1] != 'V' || pHandle->ident[2] != 'D' || pHandle->ident[3] != 'H' )
        return osl_File_E_INVAL;
    
    sprintf( pszCmd,"eject %s > /dev/null 2>&1", pHandle->pszDevice );
    nRet = system( pszCmd );

    switch ( WEXITSTATUS(nRet) )
    {
    case 0:        
        {
            FILE*         pMountTab;
            struct mnttab aMountEnt;
            
            /* lookup if device is still in mount tab */
            if ((pMountTab = fopen (MOUNTTAB, "r")) == NULL)
                return osl_File_E_BUSY;

            while (getmntent(pMountTab, &aMountEnt) == 0)
            {
                const char *pMountPoint = aMountEnt.mnt_mountp;
                const char *pDevice     = aMountEnt.mnt_special; 
                if ( 0 == strncmp( pHandle->pszDevice, aMountEnt.mnt_special, strlen(pHandle->pszDevice) ) )
                {
                    fclose (pMountTab);
                    return osl_File_E_BUSY;
                }
            }

            fclose (pMountTab);
            pHandle->pszMountPoint[0] = 0;
            return osl_File_E_None;
        }
        
        break;
        
    case 1:
        return osl_File_E_NODEV;
        break;

    case 4:
        pHandle->pszMountPoint[0] = 0;
        return osl_File_E_None;
        break;

    default:
        return osl_File_E_BUSY;
        break;
    }

    return osl_File_E_BUSY;
}

#endif /* SOLARIS */

/******************************************************************************
 *
 *                  LINUX FLOPPY FUNCTIONS
 *
 *****************************************************************************/

#if defined(LINUX)
static oslVolumeDeviceHandle 
osl_isFloppyDrive (const sal_Char* pszPath)
{
    oslVolumeDeviceHandleImpl* pItem = osl_newVolumeDeviceHandleImpl(); 
    if (osl_getFloppyMountEntry(pszPath, pItem))
        return (oslVolumeDeviceHandle) pItem;
    
    osl_freeVolumeDeviceHandleImpl (pItem);
    return 0;
}
#endif /* LINUX */

#if defined(LINUX)
static oslFileError osl_mountFloppy(oslVolumeDeviceHandle hFloppy)
{       
    sal_Bool bRet = sal_False;
    oslVolumeDeviceHandleImpl* pItem=0;
    int nRet;
    sal_Char  pszCmd[PATH_MAX];
    sal_Char* pszMountProg = "mount";
    sal_Char* pszSuDo = 0;
    sal_Char* pszTmp = 0;

    pszCmd[0] = '\0';
    
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  osl_mountFloppy\n");
#endif

    pItem = (oslVolumeDeviceHandleImpl*) hFloppy;
    
    if ( pItem == 0 )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_mountFloppy [pItem == 0]\n");
#endif
        
        return osl_File_E_INVAL;
    }

    if ( pItem->ident[0] != 'O' || pItem->ident[1] != 'V' || pItem->ident[2] != 'D' || pItem->ident[3] != 'H' )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_mountFloppy [invalid handle]\n");
#endif
        return osl_File_E_INVAL;
    }
    
    bRet = osl_isFloppyMounted(pItem);
    if ( bRet == sal_True )
    {
#ifdef DEBUG_OSL_FILE
        fprintf(stderr,"detected mounted floppy at '%s'\n",pItem->pszMountPoint);
#endif
        return osl_File_E_BUSY;
    }

    /* mfe: we can't use the mount(2) system call!!!   */
    /*      even if we are root                        */
    /*      since mtab is not updated!!!               */
    /*      but we need it to be updated               */
    /*      some "magic" must be done                  */

/*      nRet = mount(pItem->pszDevice,pItem->pszMountPoint,0,0,0); */
/*      if ( nRet != 0 ) */
/*      { */
/*          nRet=errno; */
/*  #ifdef DEBUG_OSL_FILE */
/*          perror("mount"); */
/*  #endif */
/*      } */

    pszTmp = getenv("SAL_MOUNT_MOUNTPROG");
    if ( pszTmp != 0 )
    {
        pszMountProg=pszTmp;
    }

    pszTmp=getenv("SAL_MOUNT_SU_DO");
    if ( pszTmp != 0 )
    {
        pszSuDo=pszTmp;
    }
    
    if ( pszSuDo != 0 )
    {
        sprintf(pszCmd,"%s %s %s %s",pszSuDo,pszMountProg,pItem->pszDevice,pItem->pszMountPoint);
    }
    else
    {
        sprintf(pszCmd,"%s %s",pszMountProg,pItem->pszMountPoint);        
    }
    
    
#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"executing '%s'\n",pszCmd);
#endif
    
    nRet = system(pszCmd);    

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"call returned '%i'\n",nRet);
    fprintf(stderr,"exit status is '%i'\n", WEXITSTATUS(nRet));
#endif
    
    
    switch ( WEXITSTATUS(nRet) )
    {
    case 0:
        nRet=0;
        break;
        
    case 2:
        nRet=EPERM;
        break;

    case 4:
        nRet=ENOENT;
        break;

    case 8:
        nRet=EINTR;
        break;

    case 16:
        nRet=EPERM;
        break;

    case 32:
        nRet=EBUSY;
        break;

    case 64:
        nRet=EAGAIN;
        break;

    default:
        nRet=EBUSY;
        break;
    }
    
    return oslTranslateFileError(nRet);
}
#endif /* LINUX */


#if defined(LINUX)
static oslFileError osl_unmountFloppy(oslVolumeDeviceHandle hFloppy)
{
    oslVolumeDeviceHandleImpl* pItem=0;
    int nRet=0;
    sal_Char pszCmd[PATH_MAX];
    sal_Char* pszTmp = 0;
    sal_Char* pszSuDo = 0;
    sal_Char* pszUmountProg = "umount";

    pszCmd[0] = '\0';
    
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  osl_unmountFloppy\n");
#endif    

    pItem = (oslVolumeDeviceHandleImpl*) hFloppy;
    
    if ( pItem == 0 )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_unmountFloppy [pItem==0]\n");
#endif
        return osl_File_E_INVAL;
    }

    if ( pItem->ident[0] != 'O' || pItem->ident[1] != 'V' || pItem->ident[2] != 'D' || pItem->ident[3] != 'H' )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_unmountFloppy [invalid handle]\n");
#endif
        return osl_File_E_INVAL;
    }

    /* mfe: we can't use the umount(2) system call!!!  */
    /*      even if we are root                        */
    /*      since mtab is not updated!!!               */
    /*      but we need it to be updated               */
    /*      some "magic" must be done                  */

/*      nRet=umount(pItem->pszDevice); */
/*      if ( nRet != 0 ) */
/*      { */
/*          nRet = errno; */
        
/*  #ifdef DEBUG_OSL_FILE */
/*          perror("mount"); */
/*  #endif */
/*      } */
    
    
    pszTmp = getenv("SAL_MOUNT_UMOUNTPROG");
    if ( pszTmp != 0 )
    {
        pszUmountProg=pszTmp;
    }

    pszTmp = getenv("SAL_MOUNT_SU_DO");
    if ( pszTmp != 0 )
    {
        pszSuDo=pszTmp;
    }

    if ( pszSuDo != 0 )
    {    
        sprintf(pszCmd,"%s %s %s",pszSuDo,pszUmountProg,pItem->pszMountPoint);        
    }
    else
    {
        sprintf(pszCmd,"%s %s",pszUmountProg,pItem->pszMountPoint);
    }
    

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"executing '%s'\n",pszCmd);
#endif
    
    nRet = system(pszCmd);    

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"call returned '%i'\n",nRet);
    fprintf(stderr,"exit status is '%i'\n", WEXITSTATUS(nRet));
#endif
    
    switch ( WEXITSTATUS(nRet) )
    {
    case 0:
        nRet=0;
        break;

    default:
        nRet=EBUSY;
        break;
    }
    
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out osl_unmountFloppy [ok]\n");
#endif    

    return oslTranslateFileError(nRet);
    
/*    return osl_File_E_None;*/
}

#endif /* LINUX */

#if defined(LINUX)
static sal_Bool 
osl_getFloppyMountEntry(const sal_Char* pszPath, oslVolumeDeviceHandleImpl* pItem)
{
    struct mntent* pMountEnt;
    FILE*          pMountTab;

    pMountTab = setmntent (MOUNTTAB, "r");
    if (pMountTab == 0)
        return sal_False;

    while ((pMountEnt = getmntent(pMountTab)) != 0)
    {
        if (   strncmp(pMountEnt->mnt_dir,    pszPath,   strlen(pMountEnt->mnt_dir)) == 0 
            && strncmp(pMountEnt->mnt_fsname, "/dev/fd", strlen("/dev/fd")) == 0)
        {
            strcpy (pItem->pszMountPoint, pMountEnt->mnt_dir);
            strcpy (pItem->pszFilePath,   pMountEnt->mnt_dir);
            strcpy (pItem->pszDevice,     pMountEnt->mnt_fsname);
           
            endmntent (pMountTab);
            return sal_True; 
        }
    }    

    endmntent (pMountTab);
    return sal_False;
}
#endif /* LINUX */

#if defined(LINUX)
static sal_Bool 
osl_isFloppyMounted (oslVolumeDeviceHandleImpl* pDevice)
{
    oslVolumeDeviceHandleImpl aItem;    

    if (   osl_getFloppyMountEntry (pDevice->pszMountPoint, &aItem)
        && strcmp (aItem.pszMountPoint, pDevice->pszMountPoint) == 0 
        && strcmp (aItem.pszDevice,     pDevice->pszDevice) == 0)
    {
        return sal_True;
    }
    return sal_False;    
}
#endif /* LINUX */

/******************************************************************************
 *
 *                  IRIX FLOPPY FUNCTIONS
 *
 *****************************************************************************/

#if defined(IRIX)
static oslVolumeDeviceHandle osl_isFloppyDrive(const sal_Char* pszPath)
{
    oslVolumeDeviceHandleImpl* pItem = osl_newVolumeDeviceHandleImpl ();    
    sal_Bool bRet = sal_False;
    
#ifdef TRACE_OSL_FILE   
    fprintf(stderr,"In  osl_isFloppyDrive\n");
#endif

    bRet=osl_getFloppyMountEntry(pszPath,pItem);

    if ( bRet == sal_False )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_isFloppyDrive [not a floppy]\n");
#endif
        rtl_freeMemory(pItem);
        return 0;
    }

    
#ifdef DEBUG_OSL_FILE
    osl_printFloppyHandle(pItem);
#endif
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out osl_isFloppyDrive [ok]\n");
#endif
    
    return (oslVolumeDeviceHandle) pItem;
}

#endif /* IRIX */


#if defined(IRIX)

static oslFileError osl_mountFloppy(oslVolumeDeviceHandle hFloppy)
{       
    sal_Bool bRet = sal_False;
    oslVolumeDeviceHandleImpl* pItem=0;
    int nRet;
    sal_Char  pszCmd[PATH_MAX];
    sal_Char* pszMountProg = "mount";
    sal_Char* pszSuDo = 0;
    sal_Char* pszTmp = 0;

    pszCmd[0] = '\0';
    
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  osl_mountFloppy\n");
#endif

    pItem = (oslVolumeDeviceHandleImpl*) hFloppy;
    
    if ( pItem == 0 )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_mountFloppy [pItem == 0]\n");
#endif
        
        return osl_File_E_INVAL;
    }

    if ( pItem->ident[0] != 'O' || pItem->ident[1] != 'V' || pItem->ident[2] != 'D' || pItem->ident[3] != 'H' )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_mountFloppy [invalid handle]\n");
#endif
        return osl_File_E_INVAL;
    }
    
    bRet = osl_isFloppyMounted(pItem);
    if ( bRet == sal_True )
    {
#ifdef DEBUG_OSL_FILE
        fprintf(stderr,"detected mounted floppy at '%s'\n",pItem->pszMountPoint);
#endif
        return osl_File_E_BUSY;
    }

    /* mfe: we can't use the mount(2) system call!!!   */
    /*      even if we are root                        */
    /*      since mtab is not updated!!!               */
    /*      but we need it to be updated               */
    /*      some "magic" must be done                  */

/*      nRet = mount(pItem->pszDevice,pItem->pszMountPoint,0,0,0); */
/*      if ( nRet != 0 ) */
/*      { */
/*          nRet=errno; */
/*  #ifdef DEBUG_OSL_FILE */
/*          perror("mount"); */
/*  #endif */
/*      } */

    pszTmp = getenv("SAL_MOUNT_MOUNTPROG");
    if ( pszTmp != 0 )
    {
        pszMountProg=pszTmp;
    }

    pszTmp=getenv("SAL_MOUNT_SU_DO");
    if ( pszTmp != 0 )
    {
        pszSuDo=pszTmp;
    }
    
    if ( pszSuDo != 0 )
    {
        sprintf(pszCmd,"%s %s %s %s",pszSuDo,pszMountProg,pItem->pszDevice,pItem->pszMountPoint);
    }
    else
    {
        sprintf(pszCmd,"%s %s",pszMountProg,pItem->pszMountPoint);        
    }
    
    
#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"executing '%s'\n",pszCmd);
#endif
    
    nRet = system(pszCmd);    

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"call returned '%i'\n",nRet);
    fprintf(stderr,"exit status is '%i'\n", WEXITSTATUS(nRet));
#endif
    
    
    switch ( WEXITSTATUS(nRet) )
    {
    case 0:
        nRet=0;
        break;
        
    case 2:
        nRet=EPERM;
        break;

    case 4:
        nRet=ENOENT;
        break;

    case 8:
        nRet=EINTR;
        break;

    case 16:
        nRet=EPERM;
        break;

    case 32:
        nRet=EBUSY;
        break;

    case 64:
        nRet=EAGAIN;
        break;

    default:
        nRet=EBUSY;
        break;
    }
    
    return oslTranslateFileError(nRet);
}
#endif /* IRIX */


#if defined(IRIX)
static oslFileError osl_unmountFloppy(oslVolumeDeviceHandle hFloppy)
{
    oslVolumeDeviceHandleImpl* pItem=0;
    int nRet=0;
    sal_Char pszCmd[PATH_MAX];
    sal_Char* pszTmp = 0;
    sal_Char* pszSuDo = 0;
    sal_Char* pszUmountProg = "umount";

    pszCmd[0] = '\0';
    
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  osl_unmountFloppy\n");
#endif    

    pItem = (oslVolumeDeviceHandleImpl*) hFloppy;
    
    if ( pItem == 0 )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_unmountFloppy [pItem==0]\n");
#endif
        return osl_File_E_INVAL;
    }

    if ( pItem->ident[0] != 'O' || pItem->ident[1] != 'V' || pItem->ident[2] != 'D' || pItem->ident[3] != 'H' )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_unmountFloppy [invalid handle]\n");
#endif
        return osl_File_E_INVAL;
    }

    /* mfe: we can't use the umount(2) system call!!!  */
    /*      even if we are root                        */
    /*      since mtab is not updated!!!               */
    /*      but we need it to be updated               */
    /*      some "magic" must be done                  */

/*      nRet=umount(pItem->pszDevice); */
/*      if ( nRet != 0 ) */
/*      { */
/*          nRet = errno; */
        
/*  #ifdef DEBUG_OSL_FILE */
/*          perror("mount"); */
/*  #endif */
/*      } */
    
    
    pszTmp = getenv("SAL_MOUNT_UMOUNTPROG");
    if ( pszTmp != 0 )
    {
        pszUmountProg=pszTmp;
    }

    pszTmp = getenv("SAL_MOUNT_SU_DO");
    if ( pszTmp != 0 )
    {
        pszSuDo=pszTmp;
    }

    if ( pszSuDo != 0 )
    {    
        sprintf(pszCmd,"%s %s %s",pszSuDo,pszUmountProg,pItem->pszMountPoint);        
    }
    else
    {
        sprintf(pszCmd,"%s %s",pszUmountProg,pItem->pszMountPoint);
    }
    

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"executing '%s'\n",pszCmd);
#endif
    
    nRet = system(pszCmd);    

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"call returned '%i'\n",nRet);
    fprintf(stderr,"exit status is '%i'\n", WEXITSTATUS(nRet));
#endif
    
    switch ( WEXITSTATUS(nRet) )
    {
    case 0:
        nRet=0;
        break;

    default:
        nRet=EBUSY;
        break;
    }
    
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out osl_unmountFloppy [ok]\n");
#endif    

    return oslTranslateFileError(nRet);
    
/*    return osl_File_E_None;*/
}

#endif /* IRIX */

#if defined(IRIX)
static sal_Bool osl_getFloppyMountEntry(const sal_Char* pszPath, oslVolumeDeviceHandleImpl* pItem)
{
    struct mntent* pMountEnt=0;
    sal_Char buffer[PATH_MAX];
    FILE* mntfile=0;
    int nRet=0;

    buffer[0] = '\0';
    
    mntfile = setmntent(MOUNTTAB,"r");

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  osl_getFloppyMountEntry\n");
#endif

    strcpy(buffer,pszPath);
#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"Checking mount of %s\n",buffer);
#endif

    
    if ( mntfile == 0 )
    {
        nRet=errno;
#ifdef DEBUG_OSL_FILE
        perror("mounttab");
#endif
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_getFloppyMountEntry [mntfile]\n");
#endif
        return sal_False;
    }

    pMountEnt=getmntent(mntfile);
    while ( pMountEnt != 0 )
    {
#ifdef DEBUG_OSL_FILE
/*           fprintf(stderr,"mnt_fsname : %s\n",pMountEnt->mnt_fsname); */
/*           fprintf(stderr,"mnt_dir    : %s\n",pMountEnt->mnt_dir); */
/*        fprintf(stderr,"mnt_type   : %s\n",pMountEnt->mnt_type);*/
#endif
        if ( strcmp(pMountEnt->mnt_dir,buffer) == 0 &&
             strncmp(pMountEnt->mnt_fsname,"/dev/fd",strlen("/dev/fd")) == 0 )
        {
            strcpy(pItem->pszMountPoint,pMountEnt->mnt_dir);
            strcpy(pItem->pszFilePath,pMountEnt->mnt_dir);
            strcpy(pItem->pszDevice,pMountEnt->mnt_fsname);
            
            fclose(mntfile);
#ifdef DEBUG_OSL_FILE
            fprintf(stderr,"Mount Point found '%s'\n",pItem->pszMountPoint);
#endif
#ifdef TRACE_OSL_FILE
            fprintf(stderr,"Out osl_getFloppyMountEntry [found]\n");
#endif
            return sal_True;
        }
#ifdef DEBUG_OSL_FILE
/*        fprintf(stderr,"=================\n");*/
#endif
        pMountEnt=getmntent(mntfile);
    }    

#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out osl_getFloppyMountEntry [not found]\n");
#endif

    fclose(mntfile);
    return sal_False;
}

#endif /* IRIX */

#if defined(IRIX)
static sal_Bool osl_isFloppyMounted(oslVolumeDeviceHandleImpl* pDevice)
{
    sal_Char buffer[PATH_MAX];
    oslVolumeDeviceHandleImpl* pItem=0;    
    sal_Bool bRet=0;

    buffer[0] = '\0';
    
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"In  osl_isFloppyMounted\n");
#endif

    pItem = osl_newVolumeDeviceHandleImpl ();
    if ( pItem == 0 )
        return osl_File_E_NOMEM;
    
    strcpy(buffer,pDevice->pszMountPoint);

#ifdef DEBUG_OSL_FILE
    fprintf(stderr,"Checking mount of %s\n",buffer);
#endif

    bRet = osl_getFloppyMountEntry(buffer,pItem);

    if ( bRet == sal_False )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_isFloppyMounted [not mounted]\n");
#endif
        return sal_False;
    }

    if (strcmp(pItem->pszMountPoint, pDevice->pszMountPoint) == 0 &&
        strcmp(pItem->pszDevice,pDevice->pszDevice) == 0)
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Out osl_isFloppyMounted [is mounted]\n");
#endif
        rtl_freeMemory(pItem);
        return sal_True;
    }
    
#ifdef TRACE_OSL_FILE
    fprintf(stderr,"Out osl_isFloppyMounted [may be EBUSY]\n");
#endif

    rtl_freeMemory(pItem);
    return sal_False;    
}
#endif /* IRIX */


/* NetBSD floppy functions have to be added here. Until we have done that, 
 * we use the MACOSX definitions for nonexistent floppy.
 * */

/******************************************************************************
 *
 *                  MAC OS X FLOPPY FUNCTIONS
 *
 *****************************************************************************/

#if (defined(MACOSX) || defined(NETBSD) || defined(FREEBSD))
static oslVolumeDeviceHandle osl_isFloppyDrive(const sal_Char* pszPath)
{
    return NULL;
}
#endif /* MACOSX */

#if ( defined(MACOSX) || defined(NETBSD) || defined(FREEBSD))
static oslFileError osl_mountFloppy(oslVolumeDeviceHandle hFloppy)
{
    return osl_File_E_BUSY;
}
#endif /* MACOSX */

#if ( defined(MACOSX) || defined(NETBSD) || defined(FREEBSD))
static oslFileError osl_unmountFloppy(oslVolumeDeviceHandle hFloppy)
{
    return osl_File_E_BUSY;
}
#endif /* MACOSX */

#if ( defined(MACOSX) || defined(NETBSD) || defined(FREEBSD))
static sal_Bool osl_getFloppyMountEntry(const sal_Char* pszPath, oslVolumeDeviceHandleImpl* pItem)
{
    return sal_False;
}
#endif /* MACOSX */

#if ( defined(MACOSX) || defined(NETBSD) || defined(FREEBSD))
static sal_Bool osl_isFloppyMounted(oslVolumeDeviceHandleImpl* pDevice)
{
    return sal_False;
}
#endif /* MACOSX */


#ifdef DEBUG_OSL_FILE
static void osl_printFloppyHandle(oslVolumeDeviceHandleImpl* pItem)
{
    if (pItem == 0 )
    {
        fprintf(stderr,"NULL Handle\n");
        return;
    }
    if ( pItem->ident[0] != 'O' || pItem->ident[1] != 'V' || pItem->ident[2] != 'D' || pItem->ident[3] != 'H' )
    {
#ifdef TRACE_OSL_FILE
        fprintf(stderr,"Invalid Handle]\n");
#endif
        return;
    }
    

    fprintf(stderr,"MountPoint : '%s'\n",pItem->pszMountPoint);
    fprintf(stderr,"FilePath   : '%s'\n",pItem->pszFilePath);
    fprintf(stderr,"Device     : '%s'\n",pItem->pszDevice);

    return;    
}
#endif

