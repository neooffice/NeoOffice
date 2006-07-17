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

#ifndef _JAVA_FILEPICKER_HXX_
#include "java_filepicker.hxx"
#endif

using namespace cppu;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::ui::dialogs;
using namespace rtl;
using namespace java;

// ========================================================================

namespace java {

Reference< XInterface > SAL_CALL JavaFilePicker_createInstance( const Reference< XMultiServiceFactory >& xMultiServiceFactory )
{
	return Reference< XInterface >( static_cast< OWeakObject* >( new JavaFilePicker( xMultiServiceFactory ) ) );
}

}

// ========================================================================

JavaFilePicker::JavaFilePicker( const Reference< XMultiServiceFactory >& xServiceMgr ) : WeakComponentImplHelper9< XFilterManager, XFilterGroupManager, XFilePickerControlAccess, XFilePickerNotifier, XFilePreview, XInitialization, XCancellable, XEventListener, XServiceInfo >( maMutex )
{
}

// ------------------------------------------------------------------------

JavaFilePicker::~JavaFilePicker()
{
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::addFilePickerListener( const Reference< XFilePickerListener >& xListener ) throw( RuntimeException )
{
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::removeFilePickerListener( const Reference< XFilePickerListener >& xListener ) throw( RuntimeException )
{
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::setTitle( const OUString& aTitle ) throw( RuntimeException )
{
}

// ------------------------------------------------------------------------

sal_Int16 SAL_CALL JavaFilePicker::execute() throw( RuntimeException )
{
	return 0;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::setMultiSelectionMode( sal_Bool bMode ) throw( RuntimeException )
{
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::setDefaultName( const OUString& aName ) throw( RuntimeException )
{
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::setDisplayDirectory( const OUString& aDirectory ) throw( com::sun::star::lang::IllegalArgumentException, RuntimeException )
{
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFilePicker::getDisplayDirectory() throw( RuntimeException )
{
	return OUString();
}

// ------------------------------------------------------------------------

Sequence< OUString > SAL_CALL JavaFilePicker::getFiles() throw( RuntimeException )
{
	return Sequence< OUString >();
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::appendFilter( const OUString& aTitle, const OUString& aFilter ) throw( IllegalArgumentException, RuntimeException )
{
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::setCurrentFilter( const OUString& aTitle ) throw( IllegalArgumentException, RuntimeException )
{
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFilePicker::getCurrentFilter() throw( RuntimeException )
{
	return OUString();
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::appendFilterGroup( const OUString& sGroupTitle, const Sequence< StringPair >& aFilters ) throw( IllegalArgumentException, RuntimeException )
{
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::setValue( sal_Int16 nControlId, sal_Int16 nControlAction, const Any& aValue ) throw( RuntimeException )
{
}

// ------------------------------------------------------------------------

Any SAL_CALL JavaFilePicker::getValue( sal_Int16 aControlId, sal_Int16 aControlAction ) throw( RuntimeException )
{
	return Any();
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::enableControl( sal_Int16 nControlId, sal_Bool bEnable ) throw( RuntimeException )
{
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::setLabel( sal_Int16 nControlId, const OUString& aLabel ) throw( RuntimeException )
{
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFilePicker::getLabel( sal_Int16 nControlId ) throw( RuntimeException )
{
	return OUString();
}

// ------------------------------------------------------------------------

Sequence< sal_Int16 > SAL_CALL JavaFilePicker::getSupportedImageFormats() throw( RuntimeException )
{
	return Sequence< sal_Int16 >();
}

// ------------------------------------------------------------------------

sal_Int32 SAL_CALL JavaFilePicker::getTargetColorDepth() throw( RuntimeException )
{
	return 0;
}

// ------------------------------------------------------------------------

sal_Int32 SAL_CALL JavaFilePicker::getAvailableWidth() throw( RuntimeException )
{
	return 0;
}

// ------------------------------------------------------------------------

sal_Int32 SAL_CALL JavaFilePicker::getAvailableHeight() throw( RuntimeException )
{
	return 0;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::setImage( sal_Int16 aImageFormat, const Any& aImage ) throw( IllegalArgumentException, RuntimeException )
{
}

// ------------------------------------------------------------------------

sal_Bool SAL_CALL JavaFilePicker::setShowState( sal_Bool bShowState ) throw( RuntimeException )
{
	return sal_False;
}

// ------------------------------------------------------------------------

sal_Bool SAL_CALL JavaFilePicker::getShowState() throw( RuntimeException )
{
	return sal_False;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::initialize( const Sequence< Any >& aArguments ) throw( Exception, RuntimeException )
{
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::cancel() throw( RuntimeException )
{
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::disposing( const EventObject& aEvent ) throw( RuntimeException )
{
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFilePicker::getImplementationName() throw( RuntimeException )
{
	return OUString();
}

// ------------------------------------------------------------------------

sal_Bool SAL_CALL JavaFilePicker::supportsService( const OUString& ServiceName ) throw( RuntimeException )
{
	return sal_False;
}

// ------------------------------------------------------------------------

Sequence< OUString > SAL_CALL JavaFilePicker::getSupportedServiceNames() throw( RuntimeException )
{
	return Sequence< OUString >();
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::fileSelectionChanged( FilePickerEvent aEvent )
{
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::directoryChanged( FilePickerEvent aEvent )
{
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFilePicker::helpRequested( FilePickerEvent aEvent ) const
{
	return OUString();
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::controlStateChanged( FilePickerEvent aEvent )
{
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFilePicker::dialogSizeChanged()
{
}
