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
 *  Copyright 2003 by Patrick Luby (patrick.luby@planamesa.com)
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
#include "java_clipboard.hxx"
#endif
#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#include <stdio.h>

using namespace	::rtl;
using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace	::com::sun::star::lang;

Reference< XTransferable > rXTransfRead;

class TestTransferable : public WeakImplHelper2< XClipboardOwner, XTransferable >
{
public:	
	TestTransferable();
	virtual Any SAL_CALL getTransferData( const DataFlavor& aFlavor ) throw(UnsupportedFlavorException, IOException, RuntimeException);
    virtual Sequence< DataFlavor > SAL_CALL getTransferDataFlavors() throw(RuntimeException);
	virtual sal_Bool SAL_CALL isDataFlavorSupported( const DataFlavor& aFlavor ) throw(RuntimeException);
	virtual void SAL_CALL lostOwnership( const Reference< XClipboard >& xClipboard, const Reference< XTransferable >& xTrans ) throw(RuntimeException);
	
private:
	Sequence< DataFlavor > maDataFlavors;
	OUString               maData;
};

TestTransferable::TestTransferable() :
	maDataFlavors( 1 ),
	maData( RTL_CONSTASCII_USTRINGPARAM( "This is a test string" ) )
{
	DataFlavor df;
	
	df.MimeType = OUString::createFromAscii( "text/html" );
	df.DataType = getCppuType( ( Sequence< sal_Int8 >* )0 );

	maDataFlavors[0] = df;	
}

Any SAL_CALL TestTransferable::getTransferData( const DataFlavor& aFlavor ) 
	throw(UnsupportedFlavorException, IOException, RuntimeException)
{	
	Any anyData;

	if ( aFlavor.MimeType == maDataFlavors[0].MimeType )
	{
		OString aStr( maData.getStr(), maData.getLength(), 1252 );
		Sequence< sal_Int8 > sOfChars( aStr.getLength() );
		sal_Int32 lenStr = aStr.getLength();

		for ( sal_Int32 i = 0; i < lenStr; ++i )
			sOfChars[i] = aStr[i];

		anyData = makeAny( sOfChars );
	}

	return anyData;
}

Sequence< DataFlavor > SAL_CALL TestTransferable::getTransferDataFlavors() 
	throw(RuntimeException)
{
	return maDataFlavors;
}

sal_Bool SAL_CALL TestTransferable::isDataFlavorSupported( const DataFlavor& aFlavor ) 
	throw(RuntimeException)
{
	sal_Int32 nLength = maDataFlavors.getLength();
	sal_Bool bRet     = sal_False;

	for ( sal_Int32 i = 0; i < nLength; ++i )
	{
		if ( maDataFlavors[i].MimeType == aFlavor.MimeType )
		{
			bRet = sal_True;
			break;
		}
	}

	return bRet;
}

void SAL_CALL TestTransferable::lostOwnership( const Reference< XClipboard >& xClipboard, const Reference< XTransferable >& xTrans ) 
	throw(RuntimeException)
{
}

int SAL_CALL main( int argc, char** argv )
{
	if ( argc != 2 )
	{
		fprintf( stderr, "usage: %s <my rdb file>\n", argv[0] );
		return 1;
	}

	OUString rdbName = OUString::createFromAscii( argv[1] );
	Reference< XMultiServiceFactory > g_xFactory( createRegistryServiceFactory( rdbName ) );

	// Print a message if an error occured.
	if ( !g_xFactory.is() )
	{
		OSL_ENSURE( sal_False, "Can't create RegistryServiceFactory" );
		return -1;
	}

	Reference< XTransferable > rXTransf( static_cast< XTransferable* >( new TestTransferable ) );

	Reference< XClipboard > xClipboard( g_xFactory->createInstance( OUString::createFromAscii( JAVA_CLIPBOARD_SERVICE_NAME ) ), UNO_QUERY );
	if ( !xClipboard.is() )
	{
		OSL_ENSURE( sal_False, "Error creating clipboard service" );
		return -1;
	}

	Reference< XTypeProvider > rXTypProv( xClipboard, UNO_QUERY );

	if ( rXTypProv.is() )
	{
		Sequence< Type >     seqType = rXTypProv->getTypes();
		sal_Int32 nLen = seqType.getLength();
		for ( sal_Int32 i = 0; i < nLen; i++ )
		{
			Type nxtType = seqType[i];
		}

		Sequence< sal_Int8 > seqInt8 = rXTypProv->getImplementationId();
	}

	xClipboard->setContents( rXTransf, Reference< XClipboardOwner >( rXTransf, UNO_QUERY )  );

	rXTransfRead = xClipboard->getContents();

	// destroy the transferable explicitly
	rXTransfRead = Reference< XTransferable>();

	// destroy the clipboard
	xClipboard = Reference< XClipboard >();

	// Cast factory to XComponent
	Reference< XComponent > xComponent( g_xFactory, UNO_QUERY );

	if ( !xComponent.is() )
		OSL_ENSURE( sal_False, "Error shutting down" );
	
	// Dispose and clear factory
	xComponent->dispose();
	g_xFactory.clear();
	g_xFactory = Reference< XMultiServiceFactory >();

	return 0;	
}
