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
 *		 - GNU General Public License Version 2.1
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2008 by Planamesa Inc.
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
 *************************************************************************/

#include "neomobileappevent.hxx"
#include <org/neooffice/XNeoOfficeMobile.hpp>
#include <unistd.h>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx> // helper for queryInterface() impl
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx> // helper for component factory
#endif
#ifndef _CPPUHELPER_IMPLEMENATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif

// generated c++ interfaces

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#include <comphelper/processfactory.hxx>


#import <premac.h>
#import "NSDataAdditions.h"
#import <postmac.h>


using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::org::neooffice;


NeoMobilExportFileAppEvent::NeoMobilExportFileAppEvent( OUString aSaveUUID ) :
	mnErrorCode( 0 ),
	mbFinished( false ),
	mpPostBody( nil ),
	maSaveUUID( aSaveUUID )
{
}

NeoMobilExportFileAppEvent::~NeoMobilExportFileAppEvent()
{
	if ( mpPostBody )
		[mpPostBody release];
}

IMPL_LINK( NeoMobilExportFileAppEvent, ExportFile, void*, EMPTY_ARG )
{
	if ( !mbFinished )
	{
		if ( Application::IsInMain() )
		{
			// get reference to our NeoOfficeMobile service to be used to
			// perform our conversions
			
			Reference< XComponentContext > component( comphelper_getProcessComponentContext() );
			Reference< XMultiComponentFactory > rServiceManager = component->getServiceManager();
			Reference< XInterface > rNeoOfficeMobile = rServiceManager->createInstanceWithContext(OUString::createFromAscii("org.neooffice.NeoOfficeMobile"), component);
			
			if(!rNeoOfficeMobile.is())
			{
				fprintf( stderr, "NeoMobilExportFileAppEvent::ExportFile unable to get NeoOfficeMobile service reference\n" );
				return 0;
			}
			
			Reference< XNeoOfficeMobile > neoOfficeMobile(rNeoOfficeMobile, UNO_QUERY);
			if(!neoOfficeMobile.is())
			{
				fprintf( stderr, "NeoMobilExportFileAppEvent::ExportFile unable to cast NeoOfficeMobile reference to service\n" );
				return 0;
			}
			
			// embed the UUID within the current document.  This is saved 
			// in the opendocument formatted export.
			
			neoOfficeMobile->setPropertyValue(OUString::createFromAscii("uuid"), maSaveUUID);
			
			// get a unique temporary base filename
			
			NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];
			
			NSString *basePath = NSTemporaryDirectory();
			NSString *filePath = [basePath stringByAppendingPathComponent:@"_nm_export"];
			while ([[NSFileManager defaultManager] fileExistsAtPath:filePath]) {
				filePath = [basePath stringByAppendingPathComponent:[NSString stringWithFormat:@"_nm_export_%d", rand()]];
			}
			
			fprintf(stderr, "NeoMobilExportFileAppEvent::ExportFile exporting to '%s'\n", [filePath UTF8String]);
			
			// perform an opendocument export
			
			OUString docExtension=neoOfficeMobile->getOpenDocumentExtension();
			
			OUString openDocExportURL=OUString::createFromAscii("file://");
			openDocExportURL+=OUString([filePath UTF8String], [filePath length], RTL_TEXTENCODING_UTF8);
			openDocExportURL+=docExtension;
			
			if(!neoOfficeMobile->saveAsOpenDocument(openDocExportURL))
			{
				[pool release];
				fprintf( stderr, "NeoMobilExportFileAppEvent::ExportFile unable to perform OpenDocument export\n" );
				return 0;
			}
			
			// perform a PDF export
			
			OUString pdfExportURL=OUString::createFromAscii("file://");
			pdfExportURL+=OUString([filePath UTF8String], [filePath length], RTL_TEXTENCODING_UTF8);
			pdfExportURL+=OUString::createFromAscii(".pdf");
			
			if(!neoOfficeMobile->saveAsPDF(pdfExportURL))
			{
				[pool release];
				fprintf( stderr, "NeoMobilExportFileAppEvent::ExportFile unable to perform PDF export\n" );
				return 0;
			}
			
			// perform an HTML export.  Note that we need to do this in a
			// temporary directory and then zip the directory contents into
			// a single file.  We'll just use our file's base path
			// as the temporary directory name.
			
			if(![[NSFileManager defaultManager] createDirectoryAtPath: filePath attributes: nil])
			{
				[pool release];
				fprintf( stderr, "NeoMobilExportFileAppEvent::ExportFile unable to create export directory\n" );
				return 0;
			}
			
			OUString htmlExportURL=OUString::createFromAscii("file://");
			htmlExportURL+=OUString([filePath UTF8String], [filePath length], RTL_TEXTENCODING_UTF8);
			htmlExportURL+=OUString::createFromAscii("/_nm_export.html");
			
			if(!neoOfficeMobile->saveAsHTML(htmlExportURL))
			{
				[pool release];
				fprintf( stderr, "NeoMobilExportFileAppEvent::ExportFile unable to perform HTML export\n" );
				return 0;
			}
			
			OUString htmlExportZipFile([filePath UTF8String], [filePath length], RTL_TEXTENCODING_UTF8);
			htmlExportZipFile+=OUString::createFromAscii(".zip");
			
			if(!neoOfficeMobile->zipDirectory(OUString([filePath UTF8String], [filePath length], RTL_TEXTENCODING_UTF8), htmlExportZipFile))
			{
				[pool release];
				fprintf( stderr, "NeoMobilExportFileAppEvent::ExportFile unable to create HTML zip file\n" );
				return 0;
			}
			
			// construct post data for uploading files to server
			
			// Note that at this point the exported trio is in the following
			// OUStrings:
			//
			//	PDF (as file:// URL) - pdfExportURL
			//	HTML (as full absolute path) - htmlExportZipFile
			//	OpenDocument (as file:// URL) - openDocExportURL
			
			// assemble the data for the multiple part mime form.  The
			// mime form takes in the name part "pdf" for the pdf data,
			// "html" for the zipfile, and the named section for the
			// opendocument format section is named after the extension.
			
			NSMutableData *postBody = [NSMutableData data];
			NSString *stringBoundary = [NSString stringWithString:@"neomobileupload"];
			
			// add PDF data
			
			[postBody appendData:[[NSString stringWithFormat:@"--%@\r\n",stringBoundary] dataUsingEncoding:NSUTF8StringEncoding]];
			
			OString pdfExportURLutf8;
			pdfExportURL.convertToString(&pdfExportURLutf8, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
			
			[postBody appendData:[[NSString stringWithString:@"Content-Disposition: form-data; name=\"pdf\"; filename=\"unused.pdf\"\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
			[postBody appendData:[[NSString stringWithString:@"Content-Type: image/pdf\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
			[postBody appendData:[[NSString stringWithString:@"Content-Transfer-Encoding: base64\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
						
			[postBody appendData:[[[NSData dataWithContentsOfURL:[NSURL URLWithString:[NSString stringWithUTF8String: pdfExportURLutf8.getStr()]]] base64EncodingWithLineLength: 80] dataUsingEncoding:NSASCIIStringEncoding]];
						
			// add HTML zip file data
			
			[postBody appendData:[[NSString stringWithFormat:@"\r\n--%@\r\n",stringBoundary] dataUsingEncoding:NSUTF8StringEncoding]];
			
			OString htmlExportZipFileutf8;
			htmlExportZipFile.convertToString(&htmlExportZipFileutf8, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
			
			[postBody appendData:[[NSString stringWithString:@"Content-Disposition: form-data; name=\"html\"; filename=\"unused.zip\"\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
			[postBody appendData:[[NSString stringWithString:@"Content-Type: application/zip\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
			[postBody appendData:[[NSString stringWithString:@"Content-Transfer-Encoding: base64\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
						
			[postBody appendData:[[[NSData dataWithContentsOfFile:[NSString stringWithUTF8String: htmlExportZipFileutf8.getStr()]] base64EncodingWithLineLength: 80] dataUsingEncoding:NSASCIIStringEncoding]];

			// add ODF data tagged with the appropriate mime type
			
			[postBody appendData:[[NSString stringWithFormat:@"\r\n--%@\r\n",stringBoundary] dataUsingEncoding:NSUTF8StringEncoding]];
			
			OString odfPartName;
			docExtension.copy(1).convertToString(&odfPartName, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS); // extension with first period stripped off
			
			OString mimeType;
			neoOfficeMobile->getMimeType().convertToString(&mimeType, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
			
			OString openDocExportURLutf8;
			openDocExportURL.convertToString(&openDocExportURLutf8, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
			
			[postBody appendData:[[NSString stringWithFormat:@"Content-Disposition: form-data; name=\"%@\"; filename=\"unused.%@\"\r\n\r\n", [NSString stringWithUTF8String: odfPartName.getStr()], [NSString stringWithUTF8String: odfPartName.getStr()]] dataUsingEncoding:NSUTF8StringEncoding]];
			[postBody appendData:[[NSString stringWithFormat:@"Content-Type: %@\r\n\r\n", [NSString stringWithUTF8String: mimeType.getStr()]] dataUsingEncoding:NSUTF8StringEncoding]];
			[postBody appendData:[[NSString stringWithString:@"Content-Transfer-Encoding: base64\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
						
			[postBody appendData:[[[NSData dataWithContentsOfURL:[NSURL URLWithString:[NSString stringWithUTF8String: openDocExportURLutf8.getStr()]]] base64EncodingWithLineLength: 80] dataUsingEncoding:NSASCIIStringEncoding]];

			// add UUID
			
			[postBody appendData:[[NSString stringWithFormat:@"\r\n--%@\r\n",stringBoundary] dataUsingEncoding:NSUTF8StringEncoding]];
			
			OString uuidUtf8;
			maSaveUUID.convertToString(&uuidUtf8, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
			
			[postBody appendData:[[NSString stringWithString:@"Content-Disposition: form-data; name=\"UUID\"\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
						
			[postBody appendData:[[NSString stringWithUTF8String: uuidUtf8.getStr()] dataUsingEncoding:NSUTF8StringEncoding]];		

			// mark that data is binhexed
			
			[postBody appendData:[[NSString stringWithFormat:@"\r\n--%@\r\n",stringBoundary] dataUsingEncoding:NSUTF8StringEncoding]];
			
			[postBody appendData:[[NSString stringWithString:@"Content-Disposition: form-data; name=\"base64\"\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
						
			[postBody appendData:[[NSString stringWithString:@"true"] dataUsingEncoding:NSUTF8StringEncoding]];
			
			// close out form
			
			[postBody appendData:[[NSString stringWithFormat:@"\r\n--%@--\r\n",stringBoundary] dataUsingEncoding:NSUTF8StringEncoding]];
			
			// print post data to stderr
			
			fprintf( stderr, "NeoMobilExportFileAppEvent::ExportFile start of post request\n");
			fprintf( stderr, "%s", (char *)[postBody bytes]);
			fprintf( stderr, "NeoMobilExportFileAppEvent::ExportFile end of post request\n");
			
			// Save post body
			[postBody retain];
			mpPostBody = postBody;

			// free our autorelease pool
			
			[pool release];
		}
		else
		{
			fprintf( stderr, "NeoMobilExportFileAppEvent::ExportFile : I'm notn Main!\n");
		}

		fprintf( stderr, "NeoMobilExportFileAppEvent::ExportFile not implemented\n" );
		mbFinished = true;
	}

	return 0;
}
