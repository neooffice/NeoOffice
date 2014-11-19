/*************************************************************************
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * $RCSfile$
 * $Revision$
 *
 * This file is part of NeoOffice.
 *
 * NeoOffice is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * NeoOffice is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 3 along with NeoOffice.  If not, see
 * <http://www.gnu.org/licenses/gpl-3.0.txt>
 * for a copy of the GPLv3 License.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Portions of this file are part of the LibreOffice project.
 *
 *   This Source Code Form is subject to the terms of the Mozilla Public
 *   License, v. 2.0. If a copy of the MPL was not distributed with this
 *   file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Modified August 2014 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/

#ifndef _SFX_SFXBASEMODEL_HXX_
#define _SFX_SFXBASEMODEL_HXX_

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/frame/XModule.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <com/sun/star/frame/XTitleChangeBroadcaster.hpp>
#include <com/sun/star/frame/XUntitledNumbers.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/document/XDocumentInfo.hpp>
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/document/EventObject.hpp>
#include <com/sun/star/document/XDocumentSubStorageSupplier.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/frame/DoubleInitializationException.hpp>
#include <com/sun/star/util/XModifiable2.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/view/XPrintable.hpp>
#include <com/sun/star/view/XPrintJobBroadcaster.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XStorable2.hpp>
#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/script/provider/XScriptProviderSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/script/XStarBasicAccess.hpp>
#include <osl/mutex.hxx>

#ifndef	_LINK_HXX_
#include <tools/link.hxx>
#endif

#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <com/sun/star/task/XInteractionHandler.hpp>

//________________________________________________________________________________________________________
#if ! defined(INCLUDED_COMPHELPER_IMPLBASE_VAR_HXX_28)
#define INCLUDED_COMPHELPER_IMPLBASE_VAR_HXX_28
#define COMPHELPER_IMPLBASE_INTERFACE_NUMBER 28
#include <comphelper/implbase_var.hxx>
#endif

//	include of my own project
//________________________________________________________________________________________________________
#include <svtools/lstner.hxx>

//________________________________________________________________________________________________________
//	defines
//________________________________________________________________________________________________________

// Some defines to write better code :-)
#define	XCHILD					::com::sun::star::container::XChild
#define	XNAMECONTAINER			::com::sun::star::container::XNameContainer
#define	XCONTROLLER				::com::sun::star::frame::XController
#define	XEVENTLISTENER			::com::sun::star::lang::XEventListener
#define	XINTERFACE				::com::sun::star::uno::XInterface
#define XCOMPONENT              ::com::sun::star::lang::XComponent
#define	XMODEL					::com::sun::star::frame::XModel
#define	XMODEL2					::com::sun::star::frame::XModel2
#define	XMODIFIABLE				::com::sun::star::util::XModifiable
#define	XMODIFIABLE2			::com::sun::star::util::XModifiable2
#define XMODIFYBROADCASTER      ::com::sun::star::util::XModifyBroadcaster
#define	XMODIFYLISTENER			::com::sun::star::util::XModifyListener
#define XCLOSEABLE              ::com::sun::star::util::XCloseable
#define XCLOSEBROADCASTER       ::com::sun::star::util::XCloseBroadcaster
#define XCLOSELISTENER          ::com::sun::star::util::XCloseListener
#define	XPRINTABLE				::com::sun::star::view::XPrintable
#define	XPRINTJOBBROADCASTER	::com::sun::star::view::XPrintJobBroadcaster
#define	XSTORABLE				::com::sun::star::frame::XStorable
#define	XSTORABLE2				::com::sun::star::frame::XStorable2
#define	XLOADABLE				::com::sun::star::frame::XLoadable
#define	XTYPEPROVIDER			::com::sun::star::lang::XTypeProvider
#define	XSTARBASICACCESS		::com::sun::star::script::XStarBasicAccess
#define	NOSUCHELEMENTEXCEPTION	::com::sun::star::container::NoSuchElementException
#define	ELEMENTEXISTEXCEPTION	::com::sun::star::container::ElementExistException
#define XNAMEREPLACE			::com::sun::star::container::XNameReplace
#define XINDEXCONTAINER         ::com::sun::star::container::XIndexContainer
#define DISPOSEDEXCEPTION       ::com::sun::star::lang::DisposedException
#define	DOCEVENTOBJECT			::com::sun::star::document::EventObject
#define	XDOCEVENTLISTENER		::com::sun::star::document::XEventListener
#define	XDOCUMENTINFO			::com::sun::star::document::XDocumentInfo
#define	XDOCUMENTINFOSUPPLIER	::com::sun::star::document::XDocumentInfoSupplier
#define	XEVENTBROADCASTER		::com::sun::star::document::XEventBroadcaster
#define	XEVENTSSUPPLIER			::com::sun::star::document::XEventsSupplier
#define XEMBEDDEDSCRIPTS        ::com::sun::star::document::XEmbeddedScripts
#define XSCRIPTINVOCATIONCONTEXT    ::com::sun::star::document::XScriptInvocationContext

#define	NOSUPPORTEXCEPTION		::com::sun::star::lang::NoSupportException
#define	RUNTIMEEXCEPTION		::com::sun::star::uno::RuntimeException
#define CLOSEVETOEXCEPTION      ::com::sun::star::util::CloseVetoException
#define DOUBLEINITIALIZATIONEXCEPTION      ::com::sun::star::frame::DoubleInitializationException
#define ILLEGALARGUMENTEXCEPTION ::com::sun::star::lang::IllegalArgumentException
#define IOEXCEPTION				::com::sun::star::io::IOException
#define EXCEPTION				::com::sun::star::uno::Exception

#define	ANY						::com::sun::star::uno::Any
#define	EVENTOBJECT				::com::sun::star::lang::EventObject
#define	PROPERTYVALUE			::com::sun::star::beans::PropertyValue
#define	REFERENCE				::com::sun::star::uno::Reference
#define	SEQUENCE				::com::sun::star::uno::Sequence
#define	MUTEX					::osl::Mutex
#define	OUSTRING				::rtl::OUString
#define	UNOTYPE					::com::sun::star::uno::Type
#define XVIEWDATASUPPLIER       ::com::sun::star::document::XViewDataSupplier
#define XINDEXACCESS		  	::com::sun::star::container::XIndexAccess
#define XTRANSFERABLE		  	::com::sun::star::datatransfer::XTransferable
#define DATAFLAVOR				::com::sun::star::datatransfer::DataFlavor

#define	XUNOTUNNEL		 		::com::sun::star::lang::XUnoTunnel

#define XDOCUMENTSUBSTORAGESUPPLIER  ::com::sun::star::document::XDocumentSubStorageSupplier
#define XSTORAGEBASEDDOCUMENT	::com::sun::star::document::XStorageBasedDocument
#define XSTORAGE				::com::sun::star::embed::XStorage
#define XVISUALOBJECT           ::com::sun::star::embed::XVisualObject

#define XSCRIPTPROVIDERSUPPLIER	::com::sun::star::script::provider::XScriptProviderSupplier
#define XSCRIPTPROVIDER			::com::sun::star::script::provider::XScriptProvider

#define XUICONFIGURATIONMANAGERSUPPLIER ::com::sun::star::ui::XUIConfigurationManagerSupplier
#define XUICONFIGURATIONMANAGER ::com::sun::star::ui::XUIConfigurationManager
#define XMODULE ::com::sun::star::frame::XModule
#define XTITLE ::com::sun::star::frame::XTitle
#define XTITLECHANGEBROADCASTER ::com::sun::star::frame::XTitleChangeBroadcaster
#define XUNTITLEDNUMBERS ::com::sun::star::frame::XUntitledNumbers

//________________________________________________________________________________________________________
//	namespace
//________________________________________________________________________________________________________
#ifdef css
    #error "class is using css as namespace define .-)"
#else
    #define css ::com::sun::star
#endif

//________________________________________________________________________________________________________
//	forwards
//________________________________________________________________________________________________________
class	SfxPrinter;
class	SfxViewShell;
class	SfxObjectShell						;
class	SfxEventHint;
struct	IMPL_SfxBaseModel_DataContainer		;	// impl. struct to hold member of class SfxBaseModel

//________________________________________________________________________________________________________
//	class declarations
//________________________________________________________________________________________________________

struct IMPL_SfxBaseModel_MutexContainer
{
	MUTEX m_aMutex ;
} ;

/**_______________________________________________________________________________________________________
	@short		-

	@descr		-

	@implements	XChild
				XComponent
				XDocumentInfoSupplier
				XDocumentPropertiesSupplier
				XEventListener
				XModel
				XModifiable2
				XPrintable
				XStorable2
				document::XEventBroadcaster
				document::XEventsSupplier
                document::XEmbeddedScripts
                document::XScriptInvocationContext
                XCloseable
                XCloseBroadcaster

	@base		IMPL_MutexContainer
 				SfxListener
*/

typedef ::comphelper::WeakImplHelper28  <   XCHILD
                                        ,   XDOCUMENTINFOSUPPLIER
                                        ,   ::com::sun::star::document::XDocumentPropertiesSupplier
                                        ,   XEVENTBROADCASTER
                                        ,   XEVENTLISTENER
                                        ,   XEVENTSSUPPLIER
                                        ,   XEMBEDDEDSCRIPTS
                                        ,   XSCRIPTINVOCATIONCONTEXT
                                        ,   XMODEL2
                                        ,   XMODIFIABLE2
                                        ,   XPRINTABLE
                                        ,   XPRINTJOBBROADCASTER
                                        ,   XSTORABLE2
                                        ,   XLOADABLE
                                        ,   XSTARBASICACCESS
                                        ,   XVIEWDATASUPPLIER
                                        ,   XCLOSEABLE           // => XCLOSEBROADCASTER
                                        ,   XTRANSFERABLE
                                        ,   XDOCUMENTSUBSTORAGESUPPLIER
                                        ,   XSTORAGEBASEDDOCUMENT
                                        ,   XSCRIPTPROVIDERSUPPLIER
                                        ,   XUICONFIGURATIONMANAGERSUPPLIER
                                        ,   XVISUALOBJECT
                                        ,   XUNOTUNNEL
                                        ,   XMODULE
                                        ,   XTITLE
                                        ,   XTITLECHANGEBROADCASTER
                                        ,   XUNTITLEDNUMBERS
                                        >   SfxBaseModel_Base;

class SFX2_DLLPUBLIC SfxBaseModel	:	public SfxBaseModel_Base
					                ,	public IMPL_SfxBaseModel_MutexContainer
					                ,	public SfxListener
{

//________________________________________________________________________________________________________
//	public methods
//________________________________________________________________________________________________________

public:

	//____________________________________________________________________________________________________
	//	constructor/destructor
	//____________________________________________________________________________________________________

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	SfxBaseModel( SfxObjectShell *pObjektShell = NULL ) ;

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual ~SfxBaseModel() ;

	//____________________________________________________________________________________________________
	//	XInterface
	//____________________________________________________________________________________________________

	/**___________________________________________________________________________________________________
		@short		give answer, if interface is supported
		@descr		The interfaces are searched by type.

		@seealso	XInterface

		@param      "rType" is the type of searched interface.

		@return		Any		information about found interface

		@onerror	A RuntimeException is thrown.
	*/

	virtual ANY SAL_CALL queryInterface( const UNOTYPE& rType ) throw( RUNTIMEEXCEPTION ) ;

	/**___________________________________________________________________________________________________
		@short		increment refcount
		@descr		-

		@seealso	XInterface
		@seealso	release()

		@param		-

		@return		-

		@onerror	A RuntimeException is thrown.
	*/

    virtual void SAL_CALL acquire() throw() ;

	/**___________________________________________________________________________________________________
		@short		decrement refcount
		@descr		-

		@seealso	XInterface
		@seealso	acquire()

		@param		-

		@return		-

		@onerror	A RuntimeException is thrown.
	*/

    virtual void SAL_CALL release() throw() ;

	//____________________________________________________________________________________________________
	//	XTypeProvider
	//____________________________________________________________________________________________________

	/**___________________________________________________________________________________________________
		@short		get information about supported interfaces
		@descr      -

		@seealso	XTypeProvider

		@param      -

		@return		Sequence of types of all supported interfaces

		@onerror	A RuntimeException is thrown.
	*/

	virtual SEQUENCE< UNOTYPE > SAL_CALL getTypes() throw( RUNTIMEEXCEPTION ) ;

	/**___________________________________________________________________________________________________
		@short		get implementation id
		@descr      This ID is neccessary for UNO-caching. If there no ID, cache is disabled.
					Another way, cache is enabled.

		@seealso	XTypeProvider

		@param		-

		@return		ID as Sequence of byte

		@onerror	A RuntimeException is thrown.
	*/

	virtual SEQUENCE< sal_Int8 > SAL_CALL getImplementationId() throw( RUNTIMEEXCEPTION ) ;


	//____________________________________________________________________________________________________
	//	XStarBasicAccess
	//____________________________________________________________________________________________________

	/**___________________________________________________________________________________________________
		@seealso	XStarBasicAccess
	*/
    virtual REFERENCE< XNAMECONTAINER > SAL_CALL getLibraryContainer() throw( RUNTIMEEXCEPTION );

	/**___________________________________________________________________________________________________
		@seealso	XStarBasicAccess
	*/
    virtual void SAL_CALL createLibrary( const OUSTRING& LibName, const OUSTRING& Password,
		const OUSTRING& ExternalSourceURL, const OUSTRING& LinkTargetURL )
			throw(ELEMENTEXISTEXCEPTION, RUNTIMEEXCEPTION);

	/**___________________________________________________________________________________________________
		@seealso	XStarBasicAccess
	*/
    virtual void SAL_CALL addModule( const OUSTRING& LibraryName, const OUSTRING& ModuleName,
		const OUSTRING& Language, const OUSTRING& Source )
			throw( NOSUCHELEMENTEXCEPTION, RUNTIMEEXCEPTION);

	/**___________________________________________________________________________________________________
		@seealso	XStarBasicAccess
	*/
    virtual void SAL_CALL addDialog( const OUSTRING& LibraryName, const OUSTRING& DialogName,
		const ::com::sun::star::uno::Sequence< sal_Int8 >& Data )
			throw(NOSUCHELEMENTEXCEPTION, RUNTIMEEXCEPTION);


	//____________________________________________________________________________________________________
	//	XChild
	//____________________________________________________________________________________________________

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

    virtual REFERENCE< XINTERFACE > SAL_CALL getParent() throw( RUNTIMEEXCEPTION ) ;

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

    virtual void SAL_CALL setParent( const REFERENCE< XINTERFACE >& xParent ) throw(	NOSUPPORTEXCEPTION	,
																						RUNTIMEEXCEPTION	) ;

	//____________________________________________________________________________________________________
	//	XComponent
	//____________________________________________________________________________________________________

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual void SAL_CALL addEventListener(const REFERENCE< XEVENTLISTENER >& aListener) throw (::com::sun::star::uno::RuntimeException);

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual void SAL_CALL removeEventListener(const REFERENCE< XEVENTLISTENER >& aListener ) throw (::com::sun::star::uno::RuntimeException);

	//____________________________________________________________________________________________________
	//	XDocumentInfoSupplier
	//____________________________________________________________________________________________________

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual REFERENCE< XDOCUMENTINFO > SAL_CALL getDocumentInfo() throw (::com::sun::star::uno::RuntimeException);

    // XDocumentPropertiesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentProperties >
        SAL_CALL getDocumentProperties()
        throw (::com::sun::star::uno::RuntimeException);

	//____________________________________________________________________________________________________
	//	XEventListener
	//____________________________________________________________________________________________________

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual void SAL_CALL disposing( const EVENTOBJECT& aEvent ) throw (::com::sun::star::uno::RuntimeException);

	//____________________________________________________________________________________________________
	//	XModel
	//____________________________________________________________________________________________________

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual sal_Bool SAL_CALL attachResource(	const	OUSTRING&					sURL	,
												const	SEQUENCE< PROPERTYVALUE >&	aArgs	)
        throw (::com::sun::star::uno::RuntimeException);

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual OUSTRING SAL_CALL getURL() throw (::com::sun::star::uno::RuntimeException);

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual SEQUENCE< PROPERTYVALUE > SAL_CALL getArgs() throw (::com::sun::star::uno::RuntimeException);

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual void SAL_CALL connectController( const REFERENCE< XCONTROLLER >& xController ) throw (::com::sun::star::uno::RuntimeException);

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual void SAL_CALL disconnectController( const REFERENCE< XCONTROLLER >& xController ) throw (::com::sun::star::uno::RuntimeException);

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual void SAL_CALL lockControllers() throw (::com::sun::star::uno::RuntimeException);

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual void SAL_CALL unlockControllers() throw (::com::sun::star::uno::RuntimeException);

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual sal_Bool SAL_CALL hasControllersLocked() throw (::com::sun::star::uno::RuntimeException);

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual REFERENCE< XCONTROLLER > SAL_CALL getCurrentController() throw (::com::sun::star::uno::RuntimeException);

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual void SAL_CALL setCurrentController( const REFERENCE< XCONTROLLER >& xController )
        throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual REFERENCE< XINTERFACE > SAL_CALL getCurrentSelection() throw (::com::sun::star::uno::RuntimeException);

	//____________________________________________________________________________________________________
	//	XModel2
	//____________________________________________________________________________________________________
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL getControllers()
        throw (css::uno::RuntimeException);
        
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableViewControllerNames()
        throw (css::uno::RuntimeException);
        
    virtual css::uno::Reference< css::frame::XController2 > SAL_CALL createDefaultViewController(const css::uno::Reference< css::frame::XFrame >& Frame )
        throw (css::uno::RuntimeException         ,
               css::lang::IllegalArgumentException,
               css::uno::Exception                );
               
    virtual css::uno::Reference< css::frame::XController2 > SAL_CALL createViewController(const ::rtl::OUString&                                 ViewName       ,
                                                                                          const css::uno::Sequence< css::beans::PropertyValue >& Arguments      ,
                                                                                          const css::uno::Reference< css::frame::XFrame >&       Frame          )
        throw (css::uno::RuntimeException         ,
               css::lang::IllegalArgumentException,
               css::uno::Exception                );
    
	//____________________________________________________________________________________________________
	//	XModifiable2
	//____________________________________________________________________________________________________

    virtual ::sal_Bool SAL_CALL disableSetModified(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL enableSetModified(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isSetModifiedEnabled(  ) throw (::com::sun::star::uno::RuntimeException);

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual sal_Bool SAL_CALL isModified() throw (::com::sun::star::uno::RuntimeException);

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual void SAL_CALL setModified( sal_Bool bModified )
        throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException);

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

    virtual void SAL_CALL addModifyListener( const REFERENCE< XMODIFYLISTENER >& xListener ) throw( RUNTIMEEXCEPTION ) ;

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

    virtual void SAL_CALL removeModifyListener(const REFERENCE< XMODIFYLISTENER > & xListener) throw( RUNTIMEEXCEPTION ) ;

	//____________________________________________________________________________________________________
    //  XCloseable
	//____________________________________________________________________________________________________

    virtual void SAL_CALL close( sal_Bool bDeliverOwnership ) throw (CLOSEVETOEXCEPTION, RUNTIMEEXCEPTION);

	//____________________________________________________________________________________________________
    //  XCloseBroadcaster
	//____________________________________________________________________________________________________

    virtual void SAL_CALL addCloseListener( const REFERENCE< XCLOSELISTENER >& xListener ) throw (RUNTIMEEXCEPTION);
    virtual void SAL_CALL removeCloseListener( const REFERENCE< XCLOSELISTENER >& xListener ) throw (RUNTIMEEXCEPTION);

	//____________________________________________________________________________________________________
	//	XPrintJobBroadcaster
	//____________________________________________________________________________________________________

    virtual void SAL_CALL addPrintJobListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XPrintJobListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePrintJobListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XPrintJobListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

	//____________________________________________________________________________________________________
	//	XPrintable
	//____________________________________________________________________________________________________

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual SEQUENCE< PROPERTYVALUE > SAL_CALL getPrinter() throw (::com::sun::star::uno::RuntimeException);

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual void SAL_CALL setPrinter( const SEQUENCE< PROPERTYVALUE >& seqPrinter )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual void SAL_CALL print( const SEQUENCE< PROPERTYVALUE >& seqOptions )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

	//____________________________________________________________________________________________________
	//	XStorable2
	//____________________________________________________________________________________________________

	virtual void SAL_CALL storeSelf( const	SEQUENCE< PROPERTYVALUE >&	seqArguments	)
		throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

	//____________________________________________________________________________________________________
	//	XStorable
	//____________________________________________________________________________________________________

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual sal_Bool SAL_CALL hasLocation() throw (::com::sun::star::uno::RuntimeException);

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual OUSTRING SAL_CALL getLocation() throw (::com::sun::star::uno::RuntimeException);

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual sal_Bool SAL_CALL isReadonly() throw (::com::sun::star::uno::RuntimeException);

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual void SAL_CALL store() throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual void SAL_CALL storeAsURL(	const	OUSTRING&					sURL			,
										const	SEQUENCE< PROPERTYVALUE >&	seqArguments	)
        throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException) ;

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual void SAL_CALL storeToURL(	const	OUSTRING&					sURL			,
										const	SEQUENCE< PROPERTYVALUE >&	seqArguments	)
        throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);



	//____________________________________________________________________________________________________
	//	XLoadable
	//____________________________________________________________________________________________________

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual void SAL_CALL initNew()
		throw (::com::sun::star::frame::DoubleInitializationException,
			   ::com::sun::star::io::IOException,
			   ::com::sun::star::uno::RuntimeException,
			   ::com::sun::star::uno::Exception);

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	virtual void SAL_CALL load(	const	SEQUENCE< PROPERTYVALUE >&	seqArguments )
		throw (::com::sun::star::frame::DoubleInitializationException,
			   ::com::sun::star::io::IOException,
			   ::com::sun::star::uno::RuntimeException,
			   ::com::sun::star::uno::Exception);

	//____________________________________________________________________________________________________
	//	XDocumentSubStorageSupplier
	//____________________________________________________________________________________________________

    virtual REFERENCE< XSTORAGE > SAL_CALL getDocumentSubStorage( const ::rtl::OUString& aStorageName, sal_Int32 nMode )
		throw ( RUNTIMEEXCEPTION );

	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getDocumentSubStoragesNames()
		throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException );

	//____________________________________________________________________________________________________
	//	XStorageBasedDocument
	//____________________________________________________________________________________________________

    virtual void SAL_CALL loadFromStorage( const REFERENCE< XSTORAGE >& xStorage,
											const SEQUENCE< PROPERTYVALUE >& aMediaDescriptor )
		throw ( ILLEGALARGUMENTEXCEPTION,
				DOUBLEINITIALIZATIONEXCEPTION,
				IOEXCEPTION,
				EXCEPTION,
				RUNTIMEEXCEPTION );

    virtual void SAL_CALL storeToStorage( const REFERENCE< XSTORAGE >& xStorage,
											const SEQUENCE< PROPERTYVALUE >& aMediaDescriptor )
		throw ( ILLEGALARGUMENTEXCEPTION,
				IOEXCEPTION,
				EXCEPTION,
				RUNTIMEEXCEPTION );

    virtual void SAL_CALL switchToStorage( const REFERENCE< XSTORAGE >& xStorage )
		throw ( ILLEGALARGUMENTEXCEPTION,
				IOEXCEPTION,
				EXCEPTION,
				RUNTIMEEXCEPTION );

    virtual REFERENCE< XSTORAGE > SAL_CALL getDocumentStorage()
		throw ( IOEXCEPTION,
				EXCEPTION,
				RUNTIMEEXCEPTION );

    virtual void SAL_CALL addStorageChangeListener(
			const REFERENCE< ::com::sun::star::document::XStorageChangeListener >& xListener )
		throw ( RUNTIMEEXCEPTION );

    virtual void SAL_CALL removeStorageChangeListener(
			const REFERENCE< ::com::sun::star::document::XStorageChangeListener >& xListener )
		throw ( RUNTIMEEXCEPTION );

	//____________________________________________________________________________________________________
	//	XVisualObject
	//____________________________________________________________________________________________________

    virtual void SAL_CALL setVisualAreaSize( sal_Int64 nAspect, const ::com::sun::star::awt::Size& aSize )
		throw ( ::com::sun::star::lang::IllegalArgumentException,
				::com::sun::star::embed::WrongStateException,
				::com::sun::star::uno::Exception,
				::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::awt::Size SAL_CALL getVisualAreaSize( sal_Int64 nAspect )
		throw ( ::com::sun::star::lang::IllegalArgumentException,
				::com::sun::star::embed::WrongStateException,
				::com::sun::star::uno::Exception,
				::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::embed::VisualRepresentation SAL_CALL getPreferredVisualRepresentation( ::sal_Int64 nAspect )
		throw ( ::com::sun::star::lang::IllegalArgumentException,
				::com::sun::star::embed::WrongStateException,
				::com::sun::star::uno::Exception,
				::com::sun::star::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL getMapUnit( sal_Int64 nAspect )
		throw ( ::com::sun::star::uno::Exception,
				::com::sun::star::uno::RuntimeException );

	//____________________________________________________________________________________________________
	//	XScriptProviderSupplier
	//____________________________________________________________________________________________________

	virtual REFERENCE< XSCRIPTPROVIDER > SAL_CALL getScriptProvider() throw ( RUNTIMEEXCEPTION );

	//____________________________________________________________________________________________________
	//	XUIConfigurationManagerSupplier
	//____________________________________________________________________________________________________
    virtual REFERENCE< XUICONFIGURATIONMANAGER > SAL_CALL getUIConfigurationManager() throw ( RUNTIMEEXCEPTION );

    //____________________________________________________________________________________________________
	//	XTransferable
	//____________________________________________________________________________________________________

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

    virtual ANY SAL_CALL getTransferData( const DATAFLAVOR& aFlavor )
		throw (::com::sun::star::datatransfer::UnsupportedFlavorException,
			   ::com::sun::star::io::IOException,
			   ::com::sun::star::uno::RuntimeException);

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/


    virtual SEQUENCE< DATAFLAVOR > SAL_CALL getTransferDataFlavors()
		throw (::com::sun::star::uno::RuntimeException);

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

    virtual sal_Bool SAL_CALL isDataFlavorSupported( const DATAFLAVOR& aFlavor )
		throw (::com::sun::star::uno::RuntimeException);


	//____________________________________________________________________________________________________
	//	XEventsSupplier
	//____________________________________________________________________________________________________

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-	offers a list of event handlers which are be bound to events of
						this object.
		@seealso	-

		@param		-

		@return		-	an <type>Events</type> object.

		@onerror	-
	*/

    virtual REFERENCE< XNAMEREPLACE > SAL_CALL getEvents() throw( RUNTIMEEXCEPTION );


	//____________________________________________________________________________________________________
	//	XEmbeddedScripts
	//____________________________________________________________________________________________________

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::script::XStorageBasedLibraryContainer > SAL_CALL getBasicLibraries() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::script::XStorageBasedLibraryContainer > SAL_CALL getDialogLibraries() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getAllowMacroExecution() throw (::com::sun::star::uno::RuntimeException);

	//____________________________________________________________________________________________________
	//	XScriptInvocationContext
	//____________________________________________________________________________________________________

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::document::XEmbeddedScripts > SAL_CALL getScriptContainer() throw (::com::sun::star::uno::RuntimeException);

    //____________________________________________________________________________________________________
	//	XEventBroadcaster
	//____________________________________________________________________________________________________

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-	registers the given <type>XEventListener</type>.

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

    virtual void SAL_CALL addEventListener( const REFERENCE< XDOCEVENTLISTENER >& xListener ) throw( RUNTIMEEXCEPTION );

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-	unregisters the given <type>XEventListener</type>.

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

    virtual void SAL_CALL removeEventListener( const REFERENCE< XDOCEVENTLISTENER >& xListener ) throw( RUNTIMEEXCEPTION );

	virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    // css.frame.XModule
    virtual void SAL_CALL setIdentifier(const ::rtl::OUString& sIdentifier)
        throw (css::uno::RuntimeException);
        
    // css.frame.XModule
    virtual ::rtl::OUString SAL_CALL getIdentifier()
        throw (css::uno::RuntimeException);
    
    // css.frame.XTitle
    virtual ::rtl::OUString SAL_CALL getTitle()
        throw (css::uno::RuntimeException);
    
    // css.frame.XTitle
    virtual void SAL_CALL setTitle( const ::rtl::OUString& sTitle )
        throw (css::uno::RuntimeException);    
        
    // css.frame.XTitleChangeBroadcaster
    virtual void SAL_CALL addTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener )
        throw (css::uno::RuntimeException);
        
    // css.frame.XTitleChangeBroadcaster
    virtual void SAL_CALL removeTitleChangeListener( const css::uno::Reference< css::frame::XTitleChangeListener >& xListener )
        throw (css::uno::RuntimeException);
        
    // css.frame.XUntitledNumbers
    virtual ::sal_Int32 SAL_CALL leaseNumber( const css::uno::Reference< css::uno::XInterface >& xComponent )
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException         );
               
    // css.frame.XUntitledNumbers
    virtual void SAL_CALL releaseNumber( ::sal_Int32 nNumber )
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException         );
               
    // css.frame.XUntitledNumbers
    virtual void SAL_CALL releaseNumberForComponent( const css::uno::Reference< css::uno::XInterface >& xComponent )
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException         );
               
    // css.frame.XUntitledNumbers
    virtual ::rtl::OUString SAL_CALL getUntitledPrefix()
        throw (css::uno::RuntimeException);
    
    //____________________________________________________________________________________________________
	//	SfxListener
	//____________________________________________________________________________________________________

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	void Notify(			SfxBroadcaster&	aBC		,
					const	SfxHint&		aHint	) ;

	//____________________________________________________________________________________________________
	//	public IMPL?
	//____________________________________________________________________________________________________

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	void changing() ;
	SAL_DLLPRIVATE void impl_change() ;

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	SfxObjectShell* GetObjectShell() const ;
	SAL_DLLPRIVATE SfxObjectShell* impl_getObjectShell() const ;

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

	SAL_DLLPRIVATE sal_Bool impl_isDisposed() const ;
	sal_Bool IsDisposed() const ;

    ::com::sun::star::uno::Reference < ::com::sun::star::container::XIndexAccess > SAL_CALL getViewData() throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL setViewData( const ::com::sun::star::uno::Reference < ::com::sun::star::container::XIndexAccess >& aData ) throw (::com::sun::star::uno::RuntimeException);

	/** calls all XEventListeners */
	void notifyEvent( const ::com::sun::star::document::EventObject& aEvent ) const;

	/** returns true if someone added a XEventListener to this XEventBroadcaster */
	sal_Bool hasEventListeners() const;

#if defined USE_JAVA && defined MACOSX
	void setURL( const OUSTRING& sURL );
#endif	// USE_JAVA && MACOSX

protected:

    /* returns a unique id for the model that is valid as long as the document
       is loaded. The id is not saved across document close/reload. */
    rtl::OUString getRuntimeUID() const;

    /* returns true if the document signatures are valid, otherwise false */
    sal_Bool hasValidSignatures() const;

    void setDocumentProperties( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentProperties >& );

#if SUPD == 310
    /* GrabBagItem for interim interop purposes */
    void getGrabBagItem(com::sun::star::uno::Any& rVal) const;

    void setGrabBagItem(const com::sun::star::uno::Any& rVal);
#endif	// SUPD == 310
//________________________________________________________________________________________________________
//	private methods
//________________________________________________________________________________________________________

private:

	/**___________________________________________________________________________________________________
		@short		-
		@descr		-

		@seealso	-

		@param		-

		@return		-

		@onerror	-
	*/

    bool impl_getPrintHelper();
	SAL_DLLPRIVATE void ListenForStorage_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );
	SAL_DLLPRIVATE ::rtl::OUString GetMediumFilterName_Impl();

	SAL_DLLPRIVATE void impl_store(	const	OUSTRING&					sURL			,
						const	SEQUENCE< PROPERTYVALUE >&	seqArguments	,
								sal_Bool					bSaveTo			) ;
	SAL_DLLPRIVATE void postEvent_Impl( ULONG nEventID );
	SAL_DLLPRIVATE String getEventName_Impl( long nID );
	SAL_DLLPRIVATE void NotifyStorageListeners_Impl();
   	SAL_DLLPRIVATE bool QuerySaveSizeExceededModules( const com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler >& xHandler );

    SAL_DLLPRIVATE css::uno::Reference< css::frame::XTitle > impl_getTitleHelper ();
    SAL_DLLPRIVATE css::uno::Reference< css::frame::XUntitledNumbers > impl_getUntitledHelper ();
    
//________________________________________________________________________________________________________
//	private variables and methods
//________________________________________________________________________________________________________

private:

	IMPL_SfxBaseModel_DataContainer*	m_pData	;
    // cannot be held in m_pData, since it needs to be accessed in non-threadsafe context
    const bool                          m_bSupportEmbeddedScripts;

} ;	// class SfxBaseModel

#undef css

#endif // _SFX_SFXBASEMODEL_HXX_
