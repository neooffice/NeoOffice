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
 *  Patrick Luby, July 2006
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2006 by Patrick Luby (patrick.luby@planamesa.com)
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

#include <stdio.h>

#ifndef _JAVA_FILEPICKER_HXX_
#include "java_filepicker.hxx"
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_NULLPOINTEREXCEPTION_HPP_
#include <com/sun/star/lang/NullPointerException.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_TEMPLATEDESCRIPTION_HPP_
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#include "svtools/svtools.hrc"
#include "cocoa_dialog.h"

using namespace cppu;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::ui::dialogs;
using namespace osl;
using namespace rtl;
using namespace java;

// ========================================================================

namespace java {

Sequence< OUString > SAL_CALL JavaFilePicker_getSupportedServiceNames()
{
	Sequence< OUString > aRet( 2 );
	aRet[0] = OUString::createFromAscii( "com.sun.star.ui.dialogs.FilePicker" );
	aRet[1] = OUString::createFromAscii( "com.sun.star.ui.dialogs.SystemFilePicker" );
	return aRet;
}

// ------------------------------------------------------------------------

Reference< XInterface > SAL_CALL JavaFilePicker_createInstance( const Reference< XMultiServiceFactory >& xMultiServiceFactory )
{
	return Reference< XInterface >( static_cast< XFilePicker* >( new JavaFilePicker( xMultiServiceFactory ) ) );
}

}

// ========================================================================

JavaFilePicker::JavaFilePicker( const Reference< XMultiServiceFactory >& xServiceMgr ) : WeakComponentImplHelper9< XFilterManager, XFilterGroupManager, XFilePickerControlAccess, XFilePickerNotifier, XFilePreview, XInitialization, XCancellable, XEventListener, XServiceInfo >( maMutex ), mpDialog( NULL )
{
}

// ------------------------------------------------------------------------

JavaFilePicker::~JavaFilePicker()
{
	if ( mpDialog )
		NSFileDialog_release( mpDialog );

	if ( mpResMgr )
		delete mpResMgr;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::addFilePickerListener( const Reference< XFilePickerListener >& xListener ) throw( RuntimeException )
{
    Guard< Mutex > aGuard( maMutex );

    maListeners.push_back( xListener );
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::removeFilePickerListener( const Reference< XFilePickerListener >& xListener ) throw( RuntimeException )
{
    Guard< Mutex > aGuard( maMutex );

    maListeners.remove( xListener );
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::setTitle( const OUString& aTitle ) throw( RuntimeException )
{
    Guard< Mutex > aGuard( maMutex );

	CFStringRef aString = CFStringCreateWithCharacters( NULL, aTitle.getStr(), aTitle.getLength() );
	if ( aString )
	{
		NSFileDialog_setTitle( mpDialog, aString );
		CFRelease( aString );
	}
}

// ------------------------------------------------------------------------

sal_Int16 SAL_CALL JavaFilePicker::execute() throw( RuntimeException )
{
	// Don't lock mutex as we expect callbacks to this object from a
	// a different thread while the dialog is showing
	ULONG nCount = Application::ReleaseSolarMutex();
	sal_Int16 nRet = NSFileDialog_showFileDialog( mpDialog );
	Application::AcquireSolarMutex( nCount );

	return nRet;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::setMultiSelectionMode( sal_Bool bMode ) throw( RuntimeException )
{
    Guard< Mutex > aGuard( maMutex );

	NSFileDialog_setMultiSelectionMode( mpDialog, bMode ? TRUE : FALSE );
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::setDefaultName( const OUString& aName ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "JavaFilePicker::setDefaultName not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::setDisplayDirectory( const OUString& aDirectory ) throw( com::sun::star::lang::IllegalArgumentException, RuntimeException )
{
    Guard< Mutex > aGuard( maMutex );

	OUString aPath;
	File::getSystemPathFromFileURL( aDirectory, aPath );
	if ( aPath.getLength() )
	{
		CFStringRef aString = CFStringCreateWithCharacters( NULL, aPath.getStr(), aPath.getLength() );
		if ( aString )
		{
			NSFileDialog_setDirectory( mpDialog, aString );
			CFRelease( aString );
		}
	}
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFilePicker::getDisplayDirectory() throw( RuntimeException )
{
    Guard< Mutex > aGuard( maMutex );

	OUString aRet;

	CFStringRef aString = NSFileDialog_directory( mpDialog );
	if ( aString )
	{
		CFIndex nLen = CFStringGetLength( aString );
		CFRange aRange = CFRangeMake( 0, nLen );
		sal_Unicode pBuffer[ nLen + 1 ];
		CFStringGetCharacters( aString, aRange, pBuffer );
		pBuffer[ nLen ] = 0;
		CFRelease( aString );
		OUString aPath( pBuffer );

		OUString aURL;
		File::getFileURLFromSystemPath( aPath, aRet );
	}

	return aRet;
}

// ------------------------------------------------------------------------

Sequence< OUString > SAL_CALL JavaFilePicker::getFiles() throw( RuntimeException )
{
    Guard< Mutex > aGuard( maMutex );

	Sequence< OUString > aRet;

	CFStringRef *pFileNames = NSFileDialog_fileNames( mpDialog );
	if ( pFileNames )
	{
		int nCount = 0;
		for ( ; pFileNames[ nCount ]; nCount++ )
			;

		if ( nCount )
		{
			aRet = Sequence< OUString >( nCount );
			for ( int i = 0; i < nCount; i++ )
			{
				CFStringRef aString = pFileNames[ i ];
				CFIndex nLen = CFStringGetLength( aString );
				CFRange aRange = CFRangeMake( 0, nLen );
				sal_Unicode pBuffer[ nLen + 1 ];
				CFStringGetCharacters( aString, aRange, pBuffer ); 
				pBuffer[ nLen ] = 0;
				OUString aPath( pBuffer );
				File::getFileURLFromSystemPath( aPath, aRet[ i ] );
			}
		}

		NSFontManager_releaseFileNames( pFileNames );
	}

	return aRet;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::appendFilter( const OUString& aTitle, const OUString& aFilter ) throw( IllegalArgumentException, RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "JavaFilePicker::appendFilter: %s not implemented\n", OUStringToOString( aTitle, RTL_TEXTENCODING_UTF8 ).getStr() );
#endif
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::setCurrentFilter( const OUString& aTitle ) throw( IllegalArgumentException, RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "JavaFilePicker::setCurrentFilter: %s not implemented\n", OUStringToOString( aTitle, RTL_TEXTENCODING_UTF8 ).getStr() );
#endif
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFilePicker::getCurrentFilter() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "JavaFilePicker::getCurrentFilter not implemented\n" );
#endif
	return OUString();
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::appendFilterGroup( const OUString& sGroupTitle, const Sequence< StringPair >& aFilters ) throw( IllegalArgumentException, RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "JavaFilePicker::appendFilterGroup: %s not implemented\n", OUStringToOString( sGroupTitle, RTL_TEXTENCODING_UTF8 ).getStr() );
	int nCount = aFilters.getLength();
	for ( int i = 0; i < nCount; i++ )
		fprintf( stderr, "    %s : %s\n", aFilters[ i ].First.getStr(), aFilters[ i ].Second.getStr() );
#endif
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const Any& aValue ) throw( RuntimeException )
{
    Guard< Mutex > aGuard( maMutex );

	switch ( nControlId )
	{
		case ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION:
			sal_Bool bChecked;
			aValue >>= bChecked;
			NSFileDialog_setExtensionHidden( mpDialog, bChecked ? TRUE : FALSE );
			break;
		default:
#ifdef DEBUG
			fprintf( stderr, "JavaFilePicker::setValue: %i not implemented\n", nControlId );
#endif
			break;
	}
}

// ------------------------------------------------------------------------

Any SAL_CALL JavaFilePicker::getValue( sal_Int16 nControlId, sal_Int16 nControlAction ) throw( RuntimeException )
{
    Guard< Mutex > aGuard( maMutex );

	Any aRet;

	switch ( nControlId )
	{
		case ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION:
			aRet <<= (sal_Bool)NSFileDialog_isExtensionHidden( mpDialog );
			break;
		default:
#ifdef DEBUG
			fprintf( stderr, "JavaFilePicker::getValue: %i not implemented\n", nControlId );
#endif
			break;
	}

	return aRet;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::enableControl( sal_Int16 nControlId, sal_Bool bEnable ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "JavaFilePicker::enableControl not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::setLabel( sal_Int16 nControlId, const OUString& aLabel ) throw( RuntimeException )
{
    Guard< Mutex > aGuard( maMutex );

	CocoaControlID nCocoaControlId = MAX_COCOA_CONTROL_ID;
	switch ( nControlId )
	{
		case ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION:
			// This is not changeable since we are using a control that is
			// already in Mac OS X's native file dialog
			break;
		case ExtendedFilePickerElementIds::CHECKBOX_READONLY:
			nCocoaControlId = COCOA_CONTROL_ID_READONLY;
			break;
		default:
#ifdef DEBUG
			fprintf( stderr, "JavaFilePicker::setLabel: %i not implemented\n", nControlId );
#endif
			break;
	}

	if ( nCocoaControlId < MAX_COCOA_CONTROL_ID )
	{
		XubString aRealLabel( aLabel );
		aRealLabel.EraseAllChars('~');
		CFStringRef aString = CFStringCreateWithCharacters( NULL, aRealLabel.GetBuffer(), aRealLabel.Len() );
		if ( aString )
		{
			NSFileDialog_setLabel( mpDialog, nCocoaControlId, aString );
			CFRelease( aString );
		}
	}
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFilePicker::getLabel( sal_Int16 nControlId ) throw( RuntimeException )
{
    Guard< Mutex > aGuard( maMutex );

	OUString aRet;

	CocoaControlID nCocoaControlId = MAX_COCOA_CONTROL_ID;
	switch ( nControlId )
	{
		case ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION:
			// This is not changeable since we are using a control that is
			// already in Mac OS X's native file dialog
			break;
		case ExtendedFilePickerElementIds::CHECKBOX_READONLY:
			nCocoaControlId = COCOA_CONTROL_ID_READONLY;
			break;
		default:
#ifdef DEBUG
			fprintf( stderr, "JavaFilePicker::getLabel: %i not implemented\n", nControlId );
#endif
			break;
	}

	if ( nCocoaControlId < MAX_COCOA_CONTROL_ID )
	{
		CFStringRef aString = NSFileDialog_label( mpDialog, nCocoaControlId );
		if ( aString )
		{
			CFIndex nLen = CFStringGetLength( aString );
			CFRange aRange = CFRangeMake( 0, nLen );
			sal_Unicode pBuffer[ nLen + 1 ];
			CFStringGetCharacters( aString, aRange, pBuffer );
			pBuffer[ nLen ] = 0;
			CFRelease( aString );
			aRet = OUString( pBuffer );
		}
	}

	return aRet;
}

// ------------------------------------------------------------------------

Sequence< sal_Int16 > SAL_CALL JavaFilePicker::getSupportedImageFormats() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "JavaFilePicker::getSupportedImageFormats not implemented\n" );
#endif
	return Sequence< sal_Int16 >();
}

// ------------------------------------------------------------------------

sal_Int32 SAL_CALL JavaFilePicker::getTargetColorDepth() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "JavaFilePicker::getTargetColorDepth not implemented\n" );
#endif
	return 0;
}

// ------------------------------------------------------------------------

sal_Int32 SAL_CALL JavaFilePicker::getAvailableWidth() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "JavaFilePicker::getAvailableWidth not implemented\n" );
#endif
	return 0;
}

// ------------------------------------------------------------------------

sal_Int32 SAL_CALL JavaFilePicker::getAvailableHeight() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "JavaFilePicker::getAvailableHeight not implemented\n" );
#endif
	return 0;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::setImage( sal_Int16 aImageFormat, const Any& aImage ) throw( IllegalArgumentException, RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "JavaFilePicker::setImage not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

sal_Bool SAL_CALL JavaFilePicker::setShowState( sal_Bool bShowState ) throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "JavaFilePicker::setShowState not implemented\n" );
#endif
	return sal_False;
}

// ------------------------------------------------------------------------

sal_Bool SAL_CALL JavaFilePicker::getShowState() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "JavaFilePicker::getShowState not implemented\n" );
#endif
	return sal_False;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::initialize( const Sequence< Any >& aArguments ) throw( Exception, RuntimeException )
{
	Any aAny;
	if ( !aArguments.getLength() )
		throw IllegalArgumentException( OUString::createFromAscii( "no arguments" ), static_cast< XFilePicker* >( this ), 1 );

	aAny = aArguments[0];
	if ( ( aAny.getValueType() != getCppuType( (sal_Int16*)0) ) && ( aAny.getValueType() != getCppuType( (sal_Int8*)0 ) ) )
		throw IllegalArgumentException( OUString::createFromAscii( "invalid argument type" ), static_cast< XFilePicker* >( this ), 1 );

	mpResMgr = SimpleResMgr::Create( CREATEVERSIONRESMGR_NAME( fps_office ) );
	if ( !mpResMgr )
		throw NullPointerException();

	BOOL bUseFileOpenDialog = TRUE;
    BOOL bShowAutoExtension = FALSE;
    BOOL bShowFilterOptions = FALSE;
    BOOL bShowImageTemplate = FALSE;
    BOOL bShowLink = FALSE;
    BOOL bShowPassword = FALSE;
    BOOL bShowPreview = FALSE;
    BOOL bShowReadOnly = FALSE;
    BOOL bShowSelection = FALSE;
    BOOL bShowTemplate = FALSE;
    BOOL bShowVersion = FALSE;

	sal_Int16 nType = -1;
	aAny >>= nType;
	switch ( nType )
	{
		case TemplateDescription::FILEOPEN_SIMPLE:
			break;
		case TemplateDescription::FILESAVE_SIMPLE:
			bUseFileOpenDialog = FALSE;
			break;
		case TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD:
			bUseFileOpenDialog = FALSE;
    		bShowAutoExtension = TRUE;
    		bShowPassword = TRUE;
			break;
		case TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS:
			bUseFileOpenDialog = FALSE;
    		bShowAutoExtension = TRUE;
    		bShowFilterOptions = TRUE;
    		bShowPassword = TRUE;
			break;
		case TemplateDescription::FILESAVE_AUTOEXTENSION_SELECTION:
			bUseFileOpenDialog = FALSE;
    		bShowAutoExtension = TRUE;
    		bShowSelection = TRUE;
			break;
		case TemplateDescription::FILESAVE_AUTOEXTENSION_TEMPLATE:
			bUseFileOpenDialog = FALSE;
    		bShowAutoExtension = TRUE;
			bShowTemplate = TRUE;
			break;
		case TemplateDescription::FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE:
			bShowLink = TRUE;
			bShowImageTemplate = TRUE;
			bShowPreview = TRUE;
			break;
		case TemplateDescription::FILEOPEN_PLAY:        
			break;
		case TemplateDescription::FILEOPEN_READONLY_VERSION:
			bShowReadOnly = TRUE;
			bShowVersion = TRUE;
			break;
		case TemplateDescription::FILEOPEN_LINK_PREVIEW:
			bShowLink = TRUE;
			bShowPreview = TRUE;
			break;
		case TemplateDescription::FILESAVE_AUTOEXTENSION:
			bUseFileOpenDialog = FALSE;
    		bShowAutoExtension = TRUE;
			break;
		default:
			throw IllegalArgumentException( OUString::createFromAscii( "Unknown template" ), static_cast< XFilePicker* >( this ), 1 );
    }

	mpDialog = NSFileDialog_create( bUseFileOpenDialog, TRUE, bShowAutoExtension, bShowFilterOptions, bShowImageTemplate, bShowLink, bShowPassword, bShowPreview, bShowReadOnly, bShowSelection, bShowTemplate, bShowVersion );
	if ( !mpDialog )
		throw NullPointerException();

	// Set initial values
	if ( bShowReadOnly )
	{
		OUString aLabel( mpResMgr->ReadString( STR_SVT_FILEPICKER_READONLY ) );
		setLabel( ExtendedFilePickerElementIds::CHECKBOX_READONLY, aLabel );
	}
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::cancel() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "JavaFilePicker::cancel not implemented\n" );
#endif
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::disposing( const EventObject& aEvent ) throw( RuntimeException )
{
	Reference< XFilePickerListener > xListener( aEvent.Source, UNO_QUERY );
	if ( xListener.is() )
		removeFilePickerListener( xListener );
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFilePicker::getImplementationName() throw( RuntimeException )
{
#ifdef DEBUG
	fprintf( stderr, "JavaFilePicker::getImplementationName not implemented\n" );
#endif
	return OUString();
}

// ------------------------------------------------------------------------

sal_Bool SAL_CALL JavaFilePicker::supportsService( const OUString& ServiceName ) throw( RuntimeException )
{
	Sequence < OUString > aSupportedServicesNames = JavaFilePicker_getSupportedServiceNames();
 
	for ( sal_Int32 n = aSupportedServicesNames.getLength(); n--; )
		if ( aSupportedServicesNames[n].compareTo(ServiceName) == 0 )
			return sal_True;
 
	return sal_False;
}

// ------------------------------------------------------------------------

Sequence< OUString > SAL_CALL JavaFilePicker::getSupportedServiceNames() throw( RuntimeException )
{
	return JavaFilePicker_getSupportedServiceNames();
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::fileSelectionChanged( FilePickerEvent aEvent )
{
    ClearableMutexGuard aGuard( maMutex );

	::std::list< Reference< XFilePickerListener > > listeners( maListeners );

	aGuard.clear();

    for ( ::std::list< Reference< XFilePickerListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
    {
        if ( (*it).is() )
			(*it)->fileSelectionChanged( aEvent );
    }
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::directoryChanged( FilePickerEvent aEvent )
{
    ClearableMutexGuard aGuard( maMutex );

	::std::list< Reference< XFilePickerListener > > listeners( maListeners );

	aGuard.clear();

    for ( ::std::list< Reference< XFilePickerListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
    {
        if ( (*it).is() )
			(*it)->directoryChanged( aEvent );
    }
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFilePicker::helpRequested( FilePickerEvent aEvent ) const
{
#ifdef DEBUG
	fprintf( stderr, "JavaFilePicker::helpRequested not implemented\n" );
#endif
	return OUString();
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::controlStateChanged( FilePickerEvent aEvent )
{
    ClearableMutexGuard aGuard( maMutex );

	::std::list< Reference< XFilePickerListener > > listeners( maListeners );

	aGuard.clear();

    for ( ::std::list< Reference< XFilePickerListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
    {
        if ( (*it).is() )
			(*it)->controlStateChanged( aEvent );
    }
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::dialogSizeChanged()
{
    ClearableMutexGuard aGuard( maMutex );

	::std::list< Reference< XFilePickerListener > > listeners( maListeners );

	aGuard.clear();

    for ( ::std::list< Reference< XFilePickerListener > >::const_iterator it = listeners.begin(); it != listeners.end(); ++it )
    {
        if ( (*it).is() )
			(*it)->dialogSizeChanged();
    }
}
