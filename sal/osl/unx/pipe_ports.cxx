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
 *  Patrick Luby, January 2013
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2013 Planamesa Inc.
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
 ************************************************************************/

#include <config_folders.h>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <rtl/bootstrap.hxx>

#include "pipe_ports.hxx"

using namespace osl;

static rtl::OUString ImplParentURL( const rtl::OUString& aURL )
{
	sal_Int32 nLastIndex = aURL.lastIndexOf( sal_Unicode( '/' ) );
	rtl::OUString aParentURL = aURL.copy( 0, nLastIndex );

	if ( aParentURL.getLength() == 6 && aParentURL[ aParentURL.getLength() - 1 ] == sal_Unicode(':') )
		aParentURL += "/";

	if ( aParentURL == "file://" )
		aParentURL = "file:///";

	return aParentURL;
}

static sal_Bool ImplEnsureDirURL( const rtl::OUString& rURL )
{
	if ( !rURL.getLength() )
		return sal_False;

	// Remove trailing slash
	rtl::OUString aURL;
	if ( rURL[ rURL.getLength() - 1 ] == sal_Unicode( '/' ) )
		aURL = rURL.copy( 0, rURL.getLength() - 1 );
	else
		aURL = rURL;

	Directory aDirectory( aURL );

	// Temporarily set umask to read/write only for the user when creating
	// directory
	mode_t nOldMode = umask( 077 );
	FileBase::RC nError = aDirectory.open();
	umask( nOldMode );
	aDirectory.close();
	if ( nError == File::E_None )
		return sal_True;

	// Try to create the directory
	nError = Directory::create( aURL );
	sal_Bool bRet = ( nError == File::E_None || nError == FileBase::E_EXIST );
	if ( !bRet )
	{
		rtl::OUString aParentURL = ImplParentURL( aURL );
		if ( aParentURL != aURL )
		{
			// Try to create parent directories and then try again
			bRet = ImplEnsureDirURL( aParentURL );
			if ( bRet )
			{
				nError = Directory::create( aURL );
				bRet = ( nError == File::E_None || nError == FileBase::E_EXIST );
			}
		}
	}

	return bRet;
}

static rtl::OUString ImplPipePortFileURLForName( const rtl::OUString& rName )
{
	static bool bUserInstallInitialized = false;
	static rtl::OUString aUserInstallURL;
	static Mutex aUserInstallMutex;

	rtl::OUString aRet;

	if ( !bUserInstallInitialized )
	{
		Guard< Mutex > aGuard( aUserInstallMutex );

		if ( !bUserInstallInitialized )
		{
			rtl::OUString aURL( "${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}" );
			rtl::Bootstrap::expandMacros( aURL );

			// Make sure that the user installation directory is created
			bUserInstallInitialized = ImplEnsureDirURL( aURL );
			if ( bUserInstallInitialized )
				aUserInstallURL = aURL;
		}
	}

	if ( rName.getLength() && aUserInstallURL.getLength() )
		aRet = aUserInstallURL + "/.pipe_port_" + rName;

	return aRet;
}

sal_Bool osl_createPortFileForPipe( oslPipe pPipe )
{
	sal_Bool bRet = sal_False;

	if ( !pPipe || pPipe->m_bClosed )
		return bRet;

	rtl::OUString aName( pPipe->m_Name, strlen( pPipe->m_Name ), osl_getThreadTextEncoding() );
	if ( !aName.getLength() )
		return bRet;

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	socklen_t len = sizeof(addr);
	if (getsockname(pPipe->m_Socket, reinterpret_cast< struct sockaddr* >( &addr ), &len) < 0 || addr.sin_family != AF_INET || ntohs( addr.sin_port ) == 0 )
		return bRet;

	rtl::OString aPort = rtl::OString::number( static_cast< sal_Int32 >( ntohs( addr.sin_port ) ) );
	if ( !aPort.getLength() )
		return bRet;

	rtl::OUString aPipePortFileURL( ImplPipePortFileURLForName( aName ) );
	if ( !aPipePortFileURL.getLength() )
		return bRet;

	// Create the port file
	File aFile( aPipePortFileURL );
	FileBase::RC nError = aFile.open( osl_File_OpenFlag_Write | osl_File_OpenFlag_Create );
	if ( nError != FileBase::E_None )
		nError = aFile.open( osl_File_OpenFlag_Write );
	if ( nError == FileBase::E_None )
	{
		sal_uInt64 nWritten;
		aFile.setSize( 0 );
		if ( aFile.write( static_cast< const void* >( aPort.getStr() ), aPort.getLength(), nWritten ) == FileBase::E_None && nWritten == static_cast< sal_uInt64 >( aPort.getLength() ) )
			bRet = sal_True;
		else
			aFile.setSize( 0 );
		aFile.close();
	}

	return bRet;
}

sal_uInt16 osl_getPortForPipeName( const sal_Char *pName )
{
	sal_uInt16 nRet = 0;

	if ( !pName )
		return nRet;

	rtl::OUString aName( pName, strlen( pName ), osl_getThreadTextEncoding() );
	if ( !aName.getLength() )
		return nRet;

	rtl::OUString aPipePortFileURL( ImplPipePortFileURLForName( aName ) );
	if ( !aPipePortFileURL.getLength() )
		return nRet;

	// Read the port file if it exists
	File aFile( aPipePortFileURL );
	FileBase::RC nError = aFile.open( osl_File_OpenFlag_Read );
	if ( nError == FileBase::E_None )
	{
		rtl::ByteSequence aBytes;
		if ( aFile.readLine( aBytes ) == FileBase::E_None )
		{
			rtl::OString aLine( reinterpret_cast< const sal_Char* >( aBytes.getArray() ), aBytes.getLength() );
			sal_Int32 nPort = aLine.toInt32();
			if ( nPort > 0 && nPort <= SAL_MAX_UINT16 )
				nRet = static_cast< sal_uInt16 >( nPort );
		}
		aFile.close();
	}

	return nRet;
}

void osl_unlinkPortFileForPipeName( const sal_Char *pName )
{
	if ( !pName )
		return;

	rtl::OUString aName( pName, strlen( pName ), osl_getThreadTextEncoding() );
	if ( !aName.getLength() )
		return;

	rtl::OUString aPipePortFileURL( ImplPipePortFileURLForName( aName ) );
	if ( !aPipePortFileURL.getLength() )
		return;

	rtl::OUString aPipePortFilePath;
	FileBase::getSystemPathFromFileURL( aPipePortFileURL, aPipePortFilePath );
	if ( !aPipePortFilePath.getLength() )
		return;

	unlink( OUStringToOString( aPipePortFilePath, osl_getThreadTextEncoding() ).getStr() );
}
