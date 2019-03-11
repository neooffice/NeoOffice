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
 *  Patrick Luby, June 2003
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 Planamesa Inc.
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

#ifndef _JAVA_CLIPBOARD_HXX_
#define _JAVA_CLIPBOARD_HXX_

#include <unordered_map>
#include <list>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/compbase3.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XSystemClipboard.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <rtl/ustring.hxx>
#include <sal/types.h>

#include "DTransClipboard.hxx"

class JavaClipboard : public ::cppu::WeakComponentImplHelper3< ::com::sun::star::datatransfer::clipboard::XSystemClipboard, ::com::sun::star::datatransfer::clipboard::XFlushableClipboard, ::com::sun::star::lang::XServiceInfo >
{
	::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >	maContents;
	::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner >	maOwner;
	::std::list< ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener > >	maListeners;
	::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >	maPrivateContents;
	::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner >	maPrivateOwner;
	::osl::Mutex			maMutex;
	bool					mbSystemClipboard;
	sal_Bool				mbPrivateClipboard;

public:
							JavaClipboard( bool bSystemClipboard );
	virtual					~JavaClipboard();

	// XFlushableClipboard
    virtual void			SAL_CALL flushClipboard( ) throw() override;

	// XClipboard
	virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >	SAL_CALL getContents() throw() override;
	virtual void			SAL_CALL setContents( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& xTransferable, const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner ) throw() override;
	virtual OUString		SAL_CALL getName() throw() override;

	// XClipboardEx
	virtual sal_Int8		SAL_CALL getRenderingCapabilities() throw() override;

	// XClipboardNotifier
	virtual void			SAL_CALL addClipboardListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener ) throw() override;
	virtual void			SAL_CALL removeClipboardListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener ) throw() override;

	// XServiceInfo
	virtual OUString		SAL_CALL getImplementationName() throw() override;
	virtual sal_Bool		SAL_CALL supportsService( const OUString& ServiceName ) throw() override;
	virtual ::com::sun::star::uno::Sequence< OUString >	SAL_CALL getSupportedServiceNames() throw() override;

	void					setPrivateClipboard( sal_Bool bPrivateClipboard );
};

#endif
