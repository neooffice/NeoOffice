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
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Portions of this Copyright 2000 by Sun Microsystems, Inc.
 *  Rest is Copyright 2002-2003 William Lachance (william.lachance@sympatico.ca)
 *  http://libwpd.sourceforge.net
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
 *  Modified November 2004 by Patrick Luby. SISSL Removed. NeoOffice is
 *  distributed under GPL only under modification term 3 of the LGPL.
 *
 *  Contributor(s): _______________________________________
 *
 ************************************************************************/


#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _RTL_TENCINFO_H_
#include <rtl/tencinfo.h>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_INPUTSOURCE_HPP_
#include <com/sun/star/xml/sax/InputSource.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif

#include "stream/WPXSvStream.h"
#include "FilterInternal.hxx" // for debugging
#include "WordPerfectCollector.hxx"
#include "WordPerfectImportFilter.hxx"

#ifndef _ATTRLIST_HPP_
#include <xmloff/attrlist.hxx>
#endif
#ifndef _XMLKYWD_HPP
#include <xmloff/xmlkywd.hxx>
#endif

using namespace ::rtl;
using rtl::OString;
using rtl::OUString;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::beans::PropertyValue;
using com::sun::star::document::XFilter;

using com::sun::star::io::XInputStream;
using com::sun::star::document::XImporter;
using com::sun::star::xml::sax::InputSource;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::xml::sax::XDocumentHandler;
using com::sun::star::xml::sax::XParser;

void callHandler(Reference < XDocumentHandler > xDocHandler);

sal_Bool SAL_CALL WordPerfectImportFilter::importImpl( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
	throw (RuntimeException)
{
	WRITER_DEBUG_MSG(("WordPerfectImportFilter::importImpl: Got here!\n"));
	
	sal_Int32 nLength = aDescriptor.getLength();
	const PropertyValue * pValue = aDescriptor.getConstArray();
	OUString suFileName;
	Reference < XInputStream > xInputStream;
	for ( sal_Int32 i = 0 ; i < nLength; i++)
	 	{
	 		if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "InputStream" ) ) )
	 			pValue[i].Value >>= xInputStream;
	 		else if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "FileName" ) ) )
	 			pValue[i].Value >>= suFileName;
			rtl_TextEncoding encoding = RTL_TEXTENCODING_INFO_ASCII;
	 	}
	if ( !xInputStream.is() )
	 	{
	 		OSL_ASSERT( 0 );
	 		return sal_False;
	 	}
	OString sFileName;
	sFileName = OUStringToOString(suFileName, RTL_TEXTENCODING_INFO_ASCII);
	
	// WL: an XML import service -- I assume that this is what we want to write to..
	OUString sXMLImportService ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Writer.XMLImporter" ) );
	Reference < XDocumentHandler > xHandler( mxMSF->createInstance( sXMLImportService ), UNO_QUERY );
	mxHandler = xHandler;

	// WL: the XImporter. according to the documentation at xml.openoffice.org, it's supposed to set up
	// an empty target document for XDocumentHandler to write to.. 
	Reference < XImporter > xImporter( xHandler, UNO_QUERY );
	xImporter->setTargetDocument ( mxDoc );

	WPXSvInputStream input( xInputStream );
	WordPerfectCollector collector;
	collector.filter(input, xHandler);

// 	WordPerfectStream stream;
// 	stream.stream = (void *)&xInputStream;
// 	stream.position = 0;
// 	stream.wpd_callback_get_bytes = &getBytes;
// 	stream.wpd_callback_skip_bytes = &skipBytes;
 	//
 	//sal_Bool bRet = collector->filter(&xHandler, &stream);
	
	//WRITER_DEBUG_MSG(("WordPerfectImportFilter::importImpl: Success: %i\n", (int) bRet));
	
	return true; //bRet;
}

sal_Bool SAL_CALL WordPerfectImportFilter::filter( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor ) 
	throw (RuntimeException)
{
	WRITER_DEBUG_MSG(("WordPerfectImportFilter::filter: Got here!\n"));
	return importImpl ( aDescriptor );
}
void SAL_CALL WordPerfectImportFilter::cancel(  ) 
	throw (RuntimeException)
{
	WRITER_DEBUG_MSG(("WordPerfectImportFilter::cancel: Got here!\n"));
}

// XImporter
void SAL_CALL WordPerfectImportFilter::setTargetDocument( const Reference< ::com::sun::star::lang::XComponent >& xDoc )
	throw (::com::sun::star::lang::IllegalArgumentException, RuntimeException)
{
	WRITER_DEBUG_MSG(("WordPerfectImportFilter::getTargetDocument: Got here!\n"));
	meType = FILTER_IMPORT;
	mxDoc = xDoc;
}

// XInitialization
void SAL_CALL WordPerfectImportFilter::initialize( const Sequence< Any >& aArguments ) 
	throw (Exception, RuntimeException)
{
	WRITER_DEBUG_MSG(("WordPerfectImportFilter::initialize: Got here!\n"));
	Sequence < PropertyValue > aAnySeq;
	sal_Int32 nLength = aArguments.getLength();
	if ( nLength && ( aArguments[0] >>= aAnySeq ) )
	{
		const PropertyValue * pValue = aAnySeq.getConstArray();
		nLength = aAnySeq.getLength();
		for ( sal_Int32 i = 0 ; i < nLength; i++)
		{
			if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "Type" ) ) )
			{
				pValue[i].Value >>= msFilterName;
				break;
			}
		}
	}
}
OUString WordPerfectImportFilter_getImplementationName ()
	throw (RuntimeException)
{
	return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Writer.WordPerfectImportFilter" ) );
}
// #define SERVICE_NAME1 "com.sun.star.document.ExportFilter"
#define SERVICE_NAME2 "com.sun.star.document.ImportFilter"
sal_Bool SAL_CALL WordPerfectImportFilter_supportsService( const OUString& ServiceName ) 
	throw (RuntimeException)
{
	return //ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME1 ) ) ||
    	   ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME2 ) );
}
Sequence< OUString > SAL_CALL WordPerfectImportFilter_getSupportedServiceNames(  ) 
	throw (RuntimeException)
{
	Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME2 ) );
//     pArray[1] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME2 ) );
    return aRet;
}
//#undef SERVICE_NAME1
#undef SERVICE_NAME2

Reference< XInterface > SAL_CALL WordPerfectImportFilter_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
	throw( Exception )
{
	return (cppu::OWeakObject*) new WordPerfectImportFilter( rSMgr );
}

// XServiceInfo
OUString SAL_CALL WordPerfectImportFilter::getImplementationName(  ) 
	throw (RuntimeException)
{
	return WordPerfectImportFilter_getImplementationName();
}
sal_Bool SAL_CALL WordPerfectImportFilter::supportsService( const OUString& rServiceName ) 
	throw (RuntimeException)
{
    return WordPerfectImportFilter_supportsService( rServiceName );
}
Sequence< OUString > SAL_CALL WordPerfectImportFilter::getSupportedServiceNames(  ) 
	throw (RuntimeException)
{
    return WordPerfectImportFilter_getSupportedServiceNames();
}
