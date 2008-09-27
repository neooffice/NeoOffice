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
 *  Patrick Luby, December 2007
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2007 Planamesa Inc.
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#include <plugin/impl.hxx>

// ============================================================================

JavaPluginComm::JavaPluginComm( const rtl::OUString& rMIME, const rtl::OUString& rName, void *pView ) : PluginComm( OUStringToOString( rName, RTL_TEXTENCODING_UTF8 ) )
{
#ifdef DEBUG
	fprintf( stderr, "JavaPluginComm::JavaPluginComm not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

JavaPluginComm::~JavaPluginComm()
{
}

// ----------------------------------------------------------------------------

NPError JavaPluginComm::NPP_Destroy( NPP instance, NPSavedData **save )
{
#ifdef DEBUG
	fprintf( stderr, "JavaPluginComm::NPP_Destroy not implemented\n" );
#endif
	return NPERR_GENERIC_ERROR;
}

// ----------------------------------------------------------------------------

NPError JavaPluginComm::NPP_DestroyStream( NPP instance, NPStream *stream, NPError reason )
{
#ifdef DEBUG
	fprintf( stderr, "JavaPluginComm::NPP_DestroyStream not implemented\n" );
#endif
	return NPERR_GENERIC_ERROR;
}

// ----------------------------------------------------------------------------

void *JavaPluginComm::NPP_GetJavaClass()
{
#ifdef DEBUG
	fprintf( stderr, "JavaPluginComm::NPP_GetJavaClass not implemented\n" );
#endif
	return NULL;
}

// ----------------------------------------------------------------------------

NPError JavaPluginComm::NPP_Initialize()
{
#ifdef DEBUG
	fprintf( stderr, "JavaPluginComm::NPP_Initialize not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

NPError JavaPluginComm::NPP_New( NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc, char *argn[], char *argv[], NPSavedData *saved )
{
#ifdef DEBUG
	fprintf( stderr, "JavaPluginComm::NPP_New not implemented\n" );
#endif
	return NPERR_GENERIC_ERROR;
}

// ----------------------------------------------------------------------------

NPError JavaPluginComm::NPP_NewStream( NPP instance, NPMIMEType type, NPStream *stream, NPBool seekable, uint16 *stype )
{
#ifdef DEBUG
	fprintf( stderr, "JavaPluginComm::NPP_NewStream not implemented\n" );
#endif
	return NPERR_GENERIC_ERROR;
}

// ----------------------------------------------------------------------------

void JavaPluginComm::NPP_Print( NPP instance, NPPrint *platformPrint )
{
#ifdef DEBUG
	fprintf( stderr, "JavaPluginComm::NPP_Print not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

NPError JavaPluginComm::NPP_SetWindow( NPP instance, NPWindow *window )
{
#ifdef DEBUG
	fprintf( stderr, "JavaPluginComm::NPP_SetWindow not implemented\n" );
#endif
	return NPERR_GENERIC_ERROR;
}

// ----------------------------------------------------------------------------

void JavaPluginComm::NPP_Shutdown()
{
#ifdef DEBUG
	fprintf( stderr, "JavaPluginComm::NPP_Shutdown not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

void JavaPluginComm::NPP_StreamAsFile( NPP instance, NPStream *stream, const char *fname )
{
#ifdef DEBUG
	fprintf( stderr, "JavaPluginComm::NPP_StreamAsFile not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

void JavaPluginComm::NPP_URLNotify( NPP instance, const char *url, NPReason reason, void *notifyData )
{
#ifdef DEBUG
	fprintf( stderr, "JavaPluginComm::NPP_URLNotify not implemented\n" );
#endif
}

// ----------------------------------------------------------------------------

int32 JavaPluginComm::NPP_Write( NPP instance, NPStream *stream, int32 offset, int32 len, void *buffer )
{
#ifdef DEBUG
	fprintf( stderr, "JavaPluginComm::NPP_Write not implemented\n" );
#endif
	return 0;
}

// ----------------------------------------------------------------------------

int32 JavaPluginComm::NPP_WriteReady( NPP instance, NPStream *stream )
{
#ifdef DEBUG
	fprintf( stderr, "JavaPluginComm::NPP_WriteReady not implemented\n" );
#endif
	return 0;
}

// ----------------------------------------------------------------------------

NPError JavaPluginComm::NPP_GetValue( NPP instance, NPPVariable  variable, void *value )
{
#ifdef DEBUG
	fprintf( stderr, "JavaPluginComm::NPP_GetValue not implemented\n" );
#endif
	return NPERR_GENERIC_ERROR;
}

// ----------------------------------------------------------------------------

NPError JavaPluginComm::NPP_SetValue( NPP instance, NPNVariable variable, void *value)
{
#ifdef DEBUG
	fprintf( stderr, "JavaPluginComm::NPP_SetValue not implemented\n" );
#endif
	return NPERR_GENERIC_ERROR;
}
