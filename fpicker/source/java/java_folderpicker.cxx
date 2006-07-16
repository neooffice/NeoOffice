
#ifndef _JAVA_FOLDERPICKER_HXX_
#include "java_folderpicker.hxx"
#endif

using namespace cppu;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::ui::dialogs;
using namespace com::sun::star::util;
using namespace rtl;

// ========================================================================

JavaFolderPicker::JavaFolderPicker( const Reference< XMultiServiceFactory >& xServiceMgr ) : WeakComponentImplHelper3< XFolderPicker, XServiceInfo, XCancellable >( maMutex )
{
}

// ------------------------------------------------------------------------

JavaFolderPicker::~JavaFolderPicker()
{
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFolderPicker::setTitle( const OUString& aTitle ) throw( RuntimeException )
{
}

// ------------------------------------------------------------------------

sal_Int16 SAL_CALL JavaFolderPicker::execute() throw( RuntimeException )
{
	return 0;
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFolderPicker::setDisplayDirectory( const OUString& rDirectory ) throw( IllegalArgumentException, RuntimeException )
{
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFolderPicker::getDisplayDirectory() throw( RuntimeException )
{
	return OUString();
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFolderPicker::getDirectory() throw( RuntimeException )
{
	return OUString();
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFolderPicker::setDescription( const OUString& rDescription ) throw( RuntimeException )
{
}

// ------------------------------------------------------------------------

OUString SAL_CALL JavaFolderPicker::getImplementationName() throw( RuntimeException )
{
	return OUString();
}

// ------------------------------------------------------------------------

sal_Bool SAL_CALL JavaFolderPicker::supportsService( const OUString& ServiceName ) throw( RuntimeException )
{
	return sal_False;
}

// ------------------------------------------------------------------------

Sequence< OUString > SAL_CALL JavaFolderPicker::getSupportedServiceNames() throw( RuntimeException )
{
	return Sequence< OUString >();
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFolderPicker::cancel() throw( RuntimeException )
{
}

// ------------------------------------------------------------------------

void SAL_CALL JavaFolderPicker::disposing( const EventObject& aEvent ) throw( RuntimeException )
{
}
