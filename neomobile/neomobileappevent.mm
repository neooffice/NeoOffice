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

#include "neomobile.hxx"
#include "neomobileappevent.hxx"
#include <org/neooffice/XNeoOfficeMobile.hpp>
#include <unistd.h>

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


using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::org::neooffice;


NeoMobilExportFileAppEvent::NeoMobilExportFileAppEvent( OUString aSaveUUID, NSFileManager *pFileManager, NSMutableData *pPostBody ) :
	mnErrorCode( 0 ),
	mpFileManager( pFileManager ),
	mbFinished( false ),
	mpPostBody( pPostBody ),
	maSaveUUID( aSaveUUID ),
	mbCanceled( false )
{
}

IMPL_LINK( NeoMobilExportFileAppEvent, ExportFile, void*, EMPTY_ARG )
{
	if ( !mbFinished && mpPostBody )
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
#ifdef DEBUG
				fprintf( stderr, "NeoMobilExportFileAppEvent::ExportFile unable to get NeoOfficeMobile service reference\n" );
#endif	// DEBUG
				return 0;
			}
			
			Reference< XNeoOfficeMobile > neoOfficeMobile(rNeoOfficeMobile, UNO_QUERY);
			if(!neoOfficeMobile.is())
			{
#ifdef DEBUG
				fprintf( stderr, "NeoMobilExportFileAppEvent::ExportFile unable to cast NeoOfficeMobile reference to service\n" );
#endif	// DEBUG
				return 0;
			}
			
			// embed the UUID within the current document.  This is saved 
			// in the opendocument formatted export.
			
			neoOfficeMobile->setPropertyValue(OUString::createFromAscii("uuid"), maSaveUUID);
			
			// get a unique temporary base filename
			
			NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];
			
			try
			{
			
			NSString *basePath = NSTemporaryDirectory();
			NSString *filePath = [basePath stringByAppendingPathComponent:@"_nm_export"];
			while ([[NSFileManager defaultManager] fileExistsAtPath:filePath]) {
				filePath = [basePath stringByAppendingPathComponent:[NSString stringWithFormat:@"_nm_export_%d", rand()]];
			}
			[filePath retain];
			
			OUString oufilePath(NSStringToOUString(filePath));
			
#ifdef DEBUG
			fprintf(stderr, "NeoMobilExportFileAppEvent::ExportFile exporting to '%s'\n", [filePath UTF8String]);
#endif	// DEBUG
			
			// perform an opendocument export
			
			OUString docExtension=neoOfficeMobile->getOpenDocumentExtension();
			
			OUString openDocExportURL=OUString::createFromAscii("file://");
			openDocExportURL+=oufilePath;
			openDocExportURL+=docExtension;
			
			if(!neoOfficeMobile->saveAsOpenDocument(openDocExportURL))
			{
				[pool release];
#ifdef DEBUG
				fprintf( stderr, "NeoMobilExportFileAppEvent::ExportFile unable to perform OpenDocument export\n" );
#endif	// DEBUG
				return 0;
			}
			
			if ( mbCanceled )
				throw this;
			
			// perform a PDF export
			
			OUString pdfExportURL=OUString::createFromAscii("file://");
			pdfExportURL+=oufilePath;
			pdfExportURL+=OUString::createFromAscii(".pdf");
			
			if(!neoOfficeMobile->saveAsPDF(pdfExportURL))
			{
				[pool release];
#ifdef DEBUG
				fprintf( stderr, "NeoMobilExportFileAppEvent::ExportFile unable to perform PDF export\n" );
#endif	// DEBUG
				return 0;
			}
			
			if ( mbCanceled )
				throw this;
			
			// perform an HTML export.  Note that we need to do this in a
			// temporary directory and then zip the directory contents into
			// a single file.  We'll just use our file's base path
			// as the temporary directory name.
			
			if(!mpFileManager || ![mpFileManager createDirectoryAtPath: filePath attributes: nil])
			{
				[pool release];
#ifdef DEBUG
				fprintf( stderr, "NeoMobilExportFileAppEvent::ExportFile unable to create export directory\n" );
#endif	// DEBUG
				return 0;
			}
			
			OUString htmlExportURL=OUString::createFromAscii("file://");
			htmlExportURL+=oufilePath;
			htmlExportURL+=OUString::createFromAscii("/_nm_export.html");
			
			if(!neoOfficeMobile->saveAsHTML(htmlExportURL))
			{
				[pool release];
#ifdef DEBUG
				fprintf( stderr, "NeoMobilExportFileAppEvent::ExportFile unable to perform HTML export\n" );
#endif	// DEBUG
				return 0;
			}
			
			if ( mbCanceled )
				throw this;
			
			OUString htmlExportZipDir(oufilePath);
			OUString htmlExportZipFile(htmlExportZipDir);
			htmlExportZipFile+=OUString::createFromAscii(".zip");
			
			if(!neoOfficeMobile->zipDirectory(htmlExportZipDir, htmlExportZipFile))
			{
				[pool release];
#ifdef DEBUG
				fprintf( stderr, "NeoMobilExportFileAppEvent::ExportFile unable to create HTML zip file\n" );
#endif	// DEBUG
				return 0;
			}
			
			if ( mbCanceled )
				throw this;
			
			// remove temporary directory used to create zip file
			
			[[NSFileManager defaultManager] removeFileAtPath:filePath handler:NULL];
			
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
			
			NSString *stringBoundary = @"neomobileupload";
			
			// add PDF data
			
			[mpPostBody appendData:[[NSString stringWithFormat:@"--%@\r\n",stringBoundary] dataUsingEncoding:NSUTF8StringEncoding]];
			
			OString pdfExportURLutf8 = OUStringToOString(pdfExportURL,RTL_TEXTENCODING_UTF8);
			
			[mpPostBody appendData:[[NSString stringWithString:@"Content-Disposition: form-data; name=\"pdf\"; filename=\"unused.pdf\"\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
			[mpPostBody appendData:[[NSString stringWithString:@"Content-Type: image/pdf\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
						
			[mpPostBody appendData:[NSData dataWithContentsOfURL:[NSURL URLWithString:[NSString stringWithUTF8String: pdfExportURLutf8.getStr()]]]];
						
			// add HTML zip file data
			
			[mpPostBody appendData:[[NSString stringWithFormat:@"\r\n--%@\r\n",stringBoundary] dataUsingEncoding:NSUTF8StringEncoding]];
			
			OString htmlExportZipFileutf8 = OUStringToOString(htmlExportZipFile,RTL_TEXTENCODING_UTF8);
			
			[mpPostBody appendData:[[NSString stringWithString:@"Content-Disposition: form-data; name=\"html\"; filename=\"unused.zip\"\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
			[mpPostBody appendData:[[NSString stringWithString:@"Content-Type: application/zip\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
						
			[mpPostBody appendData:[NSData dataWithContentsOfFile:[NSString stringWithUTF8String: htmlExportZipFileutf8.getStr()]]];

			// add ODF data tagged with the appropriate mime type
			
			[mpPostBody appendData:[[NSString stringWithFormat:@"\r\n--%@\r\n",stringBoundary] dataUsingEncoding:NSUTF8StringEncoding]];
			
			OString odfPartName = OUStringToOString(docExtension.copy(1),RTL_TEXTENCODING_UTF8); // extension with first period stripped off
			
			OString mimeType = OUStringToOString(neoOfficeMobile->getMimeType(),RTL_TEXTENCODING_UTF8);
			
			OString openDocExportURLutf8 = OUStringToOString(openDocExportURL,RTL_TEXTENCODING_UTF8);
			
			[mpPostBody appendData:[[NSString stringWithFormat:@"Content-Disposition: form-data; name=\"%@\"; filename=\"unused.%@\"\r\n\r\n", [NSString stringWithUTF8String: odfPartName.getStr()], [NSString stringWithUTF8String: odfPartName.getStr()]] dataUsingEncoding:NSUTF8StringEncoding]];
			[mpPostBody appendData:[[NSString stringWithFormat:@"Content-Type: %@\r\n\r\n", [NSString stringWithUTF8String: mimeType.getStr()]] dataUsingEncoding:NSUTF8StringEncoding]];
						
			[mpPostBody appendData:[NSData dataWithContentsOfURL:[NSURL URLWithString:[NSString stringWithUTF8String: openDocExportURLutf8.getStr()]]]];

			// add UUID
			
			[mpPostBody appendData:[[NSString stringWithFormat:@"\r\n--%@\r\n",stringBoundary] dataUsingEncoding:NSUTF8StringEncoding]];
			
			OString uuidUtf8 = OUStringToOString(maSaveUUID,RTL_TEXTENCODING_UTF8);
			
			[mpPostBody appendData:[[NSString stringWithString:@"Content-Disposition: form-data; name=\"UUID\"\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
						
			[mpPostBody appendData:[[NSString stringWithUTF8String: uuidUtf8.getStr()] dataUsingEncoding:NSUTF8StringEncoding]];		

			// close out form
			
			[mpPostBody appendData:[[NSString stringWithFormat:@"\r\n--%@--\r\n",stringBoundary] dataUsingEncoding:NSUTF8StringEncoding]];
			
#ifdef DEBUG
			// print post data to stderr
			
			fprintf( stderr, "NeoMobilExportFileAppEvent::ExportFile start of post request\n");
			fprintf( stderr, "%s", (char *)[mpPostBody bytes]);
			fprintf( stderr, "NeoMobilExportFileAppEvent::ExportFile end of post request\n");
#endif	// DEBUG
			
			// remove exported files on disk now that we've finished assembling
			// our post in memory
			
			[[NSFileManager defaultManager] removeFileAtPath:[[NSURL URLWithString:[NSString stringWithUTF8String: pdfExportURLutf8.getStr()]] path] handler:nil];
			[[NSFileManager defaultManager] removeFileAtPath:[NSString stringWithUTF8String: htmlExportZipFileutf8.getStr()] handler:nil];
			[[NSFileManager defaultManager] removeFileAtPath:[[NSURL URLWithString:[NSString stringWithUTF8String: openDocExportURLutf8.getStr()]] path] handler:nil];
			
			// free our autorelease pool

			}
			catch (...)
			{
			}
			
			[pool release];
		}
		else
		{
#ifdef DEBUG
			fprintf( stderr, "NeoMobilExportFileAppEvent::ExportFile : I'm notn Main!\n");
#endif	// DEBUG
		}
		
		mbFinished = true;
	}

	return 0;
}
