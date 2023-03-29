/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#import "neomobile.hxx"
#import "neomobileappevent.hxx"
#import "neomobilei18n.hxx"

#import <com/sun/star/frame/XDesktop.hpp>
#import <com/sun/star/frame/XFrame.hpp>
#import <com/sun/star/registry/XRegistryKey.hpp>
#import <comphelper/processfactory.hxx>
#import <cppuhelper/factory.hxx>
#import <cppuhelper/implementationentry.hxx>
#import <cppuhelper/queryinterface.hxx>
#import <org/neooffice/XNeoOfficeMobile.hpp>
#import <vcl/svapp.hxx>
#import <vos/module.hxx>
#import <vos/mutex.hxx>

#ifndef DLLPOSTFIX
#error DLLPOSTFIX must be defined in makefile.mk
#endif

#define DOSTRING( x )           #x
#define STRING( x )             DOSTRING( x )

typedef void ShowOnlyMenusForWindow_Type( void*, sal_Bool );

static ::vos::OModule aModule;
static ShowOnlyMenusForWindow_Type *pShowOnlyMenusForWindow = NULL;

using namespace rtl;
using namespace osl;
using namespace cppu;
using namespace com::sun::star::frame;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace com::sun::star::uno;
using namespace org::neooffice;
using namespace vos;

static bool IsRunningInNeoOffice()
{
	// Locate libvcl and invoke the ShowOnlyMenusForWindow function
	if ( !pShowOnlyMenusForWindow )
	{
		::rtl::OUString aLibName = ::rtl::OUString::createFromAscii( "libvcl" );
#if SUPD == 680
		aLibName += ::rtl::OUString::valueOf( (sal_Int32)SUPD, 10 );
#endif	// SUPD == 680
		aLibName += ::rtl::OUString::createFromAscii( STRING( DLLPOSTFIX ) );
		aLibName += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".dylib" ) );
		if ( aModule.load( aLibName ) )
			pShowOnlyMenusForWindow = (ShowOnlyMenusForWindow_Type *)aModule.getSymbol( ::rtl::OUString::createFromAscii( "ShowOnlyMenusForWindow" ) );
	}

	return(pShowOnlyMenusForWindow ? true : false);
}

IMPL_LINK( NeoMobileExportFileAppEvent, ExportFile, void*, EMPTY_ARG )
{
	if ( !mbFinished && mpPostBody )
	{
		if ( Application::IsInMain() )
		{
			// get reference to our NeoOfficeMobile service to be used to
			// perform our conversions
			
			Reference< XInterface > rNeoOfficeMobile = ::comphelper::getProcessServiceFactory()->createInstance(OUString::createFromAscii("org.neooffice.NeoOfficeMobile"));
			
			if(!rNeoOfficeMobile.is())
			{
#ifdef DEBUG
				fprintf( stderr, "NeoMobileExportFileAppEvent::ExportFile unable to get NeoOfficeMobile service reference\n" );
#endif	// DEBUG
				return 0;
			}
			
			Reference< XNeoOfficeMobile > neoOfficeMobile(rNeoOfficeMobile, UNO_QUERY);
			if(!neoOfficeMobile.is())
			{
#ifdef DEBUG
				fprintf( stderr, "NeoMobileExportFileAppEvent::ExportFile unable to cast NeoOfficeMobile reference to service\n" );
#endif	// DEBUG
				return 0;
			}
			
			// get current frame
			Reference< XDesktop > rDesktop(::comphelper::getProcessServiceFactory()->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop"))), UNO_QUERY);
			if(!rDesktop.is())
			{
#ifdef DEBUG
				fprintf( stderr, "NeoMobileExportFileAppEvent::ExportFile unable to get XDesktop service reference\n" );
#endif	// DEBUG
				return 0;
			}

			Reference< XFrame > rFrame=rDesktop->getCurrentFrame();
			if(!rFrame.is())
			{
#ifdef DEBUG
				fprintf( stderr, "NeoMobileExportFileAppEvent::ExportFile unable to get current frame reference\n" );
#endif	// DEBUG
				return 0;
			}
			
			// check that we have a supported document type.  If not, return without
			// attempting to assemble the post or perform an export.
			
			OString mimeType = OUStringToOString(neoOfficeMobile->getMimeType(rFrame),RTL_TEXTENCODING_UTF8);
			if(!mimeType.getLength())
			{
				mbUnsupportedComponentType = true;
				mbFinished = true;
				return(0);
			}
			
			// Some ODF file formats do not have a matching Office file format
			// so it is not a problem if there is no matching Office mime type
			OString officeMimeType = OUStringToOString(neoOfficeMobile->getOfficeMimeType(rFrame),RTL_TEXTENCODING_UTF8);
			if(mpMimeTypes && officeMimeType.getLength() && ![mpMimeTypes containsObject: [NSString stringWithUTF8String: officeMimeType.getStr()]])
				officeMimeType = OString();
			
			// embed the UUID within the current document.  This is saved 
			// in the opendocument formatted export.
			
			neoOfficeMobile->setPropertyValue(rFrame, OUString::createFromAscii("uuid"), maSaveUUID);
			

			// Check if the current document is a password protected. If so,
			// display an alert and if the alert is cancelled, cancel this event
			if(neoOfficeMobile->isPasswordProtected(rFrame))
			{
				NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];
				NeoMobileRunPasswordProtectionAlertOnMainThread *passwordProtection=[[NeoMobileRunPasswordProtectionAlertOnMainThread alloc] init];
				[passwordProtection performSelectorOnMainThread:@selector(runModal:) withObject:passwordProtection waitUntilDone:YES modes:NeoMobileGetPerformSelectorOnMainThreadModes()];
				if([passwordProtection cancelled])
					mbCanceled = true;
				[passwordProtection release];
				[pool release];
			}

			if ( mbCanceled )
				return(0);

			// get a unique temporary base filename
			
			NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];
			NeoMobileDoFileManagerOnMainThread *fileMgr=nil;
			
			OString pdfExportURLutf8;
			OString openDocExportURLutf8;
			OString officeDocExportURLutf8;
			OString htmlExportZipFileutf8;
			
			try
			{
			fileMgr=[[NeoMobileDoFileManagerOnMainThread alloc] init];
			[fileMgr performSelectorOnMainThread:@selector(makeBasePath:) withObject:fileMgr waitUntilDone:YES modes:NeoMobileGetPerformSelectorOnMainThreadModes()];
			
			NSString *filePath=[fileMgr filePath];
			OUString oufilePath(NeoMobileNSStringToOUString(filePath));
			
#ifdef DEBUG
			fprintf(stderr, "NeoMobileExportFileAppEvent::ExportFile exporting to '%s'\n", [filePath UTF8String]);
#endif	// DEBUG
			
			// perform an opendocument export
			
			OUString docExtension=neoOfficeMobile->getOpenDocumentExtension(rFrame);
			
			OUString openDocExportURL=OUString::createFromAscii("file://");
			openDocExportURL+=oufilePath;
			openDocExportURL+=docExtension;
			
			openDocExportURLutf8 = OUStringToOString(openDocExportURL,RTL_TEXTENCODING_UTF8);

			if(!neoOfficeMobile->saveAsOpenDocument(rFrame, openDocExportURL))
			{
#ifdef DEBUG
				fprintf( stderr, "NeoMobileExportFileAppEvent::ExportFile unable to perform OpenDocument export\n" );
#endif	// DEBUG
				mnErrorCode=1;
				throw this;
			}
			
			if ( mbCanceled )
				throw this;
			
			// perform an Office document export but skip if there is no
			// matchin Office document mime type

			OUString officeDocExtension=neoOfficeMobile->getOfficeDocumentExtension(rFrame);
			
			if(officeMimeType.getLength())
			{
				OUString officeDocExportURL=OUString::createFromAscii("file://");
				officeDocExportURL+=oufilePath;
				officeDocExportURL+=officeDocExtension;
				
				officeDocExportURLutf8 = OUStringToOString(officeDocExportURL,RTL_TEXTENCODING_UTF8);

				if(!neoOfficeMobile->saveAsOfficeDocument(rFrame, officeDocExportURL))
				{
#ifdef DEBUG
					fprintf( stderr, "NeoMobileExportFileAppEvent::ExportFile unable to perform Office document export\n" );
#endif	// DEBUG
					mnErrorCode=1;
					throw this;
				}
			
				if ( mbCanceled )
					throw this;
			}
			
			// perform a PDF export
			
			OString pdfMimeType( "image/pdf" );
			if(mpMimeTypes && pdfMimeType.getLength() && ![mpMimeTypes containsObject: [NSString stringWithUTF8String: pdfMimeType.getStr()]])
				pdfMimeType = OString();

			if(pdfMimeType.getLength())
			{
				OUString pdfExportURL=OUString::createFromAscii("file://");
				pdfExportURL+=oufilePath;
				pdfExportURL+=OUString::createFromAscii(".pdf");
				
				pdfExportURLutf8 = OUStringToOString(pdfExportURL,RTL_TEXTENCODING_UTF8);

				if(!neoOfficeMobile->saveAsPDF(rFrame, pdfExportURL))
				{
#ifdef DEBUG
					fprintf( stderr, "NeoMobileExportFileAppEvent::ExportFile unable to perform PDF export\n" );
#endif	// DEBUG
					mnErrorCode=1;
					throw this;
				}
			}
			
			if ( mbCanceled )
				throw this;
			
			// perform an HTML export.  Note that we need to do this in a
			// temporary directory and then zip the directory contents into
			// a single file.  We'll just use our file's base path
			// as the temporary directory name.
			
			OString htmlMimeType( "application/zip" );
			if(mpMimeTypes && htmlMimeType.getLength() && ![mpMimeTypes containsObject: [NSString stringWithUTF8String: htmlMimeType.getStr()]])
				htmlMimeType = OString();

			if(htmlMimeType.getLength())
			{
				[fileMgr performSelectorOnMainThread:@selector(createDir:) withObject:filePath waitUntilDone:YES modes:NeoMobileGetPerformSelectorOnMainThreadModes()];
							
				OUString htmlExportURL=OUString::createFromAscii("file://");
				htmlExportURL+=oufilePath;
				htmlExportURL+=OUString::createFromAscii("/_nm_export.html");
				
				if(!neoOfficeMobile->saveAsHTML(rFrame, htmlExportURL))
				{
#ifdef DEBUG
					fprintf( stderr, "NeoMobileExportFileAppEvent::ExportFile unable to perform HTML export\n" );
#endif	// DEBUG
					// remove temporary directory used to create zip file
				
					[fileMgr performSelectorOnMainThread:@selector(removeItem:) withObject:filePath waitUntilDone:YES modes:NeoMobileGetPerformSelectorOnMainThreadModes()];
					mnErrorCode=1;
					throw this;
				}
				
				if ( mbCanceled )
				{
					// remove temporary directory used to create zip file
				
					[fileMgr performSelectorOnMainThread:@selector(removeItem:) withObject:filePath waitUntilDone:YES modes:NeoMobileGetPerformSelectorOnMainThreadModes()];
					throw this;
				}
				
				OUString htmlExportZipDir(oufilePath);
				OUString htmlExportZipFile(htmlExportZipDir);
				htmlExportZipFile+=OUString::createFromAscii(".zip");
				
				htmlExportZipFileutf8 = OUStringToOString(htmlExportZipFile,RTL_TEXTENCODING_UTF8);

				if(!neoOfficeMobile->zipDirectory(htmlExportZipDir, htmlExportZipFile))
				{
#ifdef DEBUG
					fprintf( stderr, "NeoMobileExportFileAppEvent::ExportFile unable to create HTML zip file\n" );
#endif	// DEBUG
					[fileMgr performSelectorOnMainThread:@selector(removeItem:) withObject:filePath waitUntilDone:YES modes:NeoMobileGetPerformSelectorOnMainThreadModes()];
					mnErrorCode=1;
					throw this;
				}
						
				// remove temporary directory used to create zip file
				
				[fileMgr performSelectorOnMainThread:@selector(removeItem:) withObject:filePath waitUntilDone:YES modes:NeoMobileGetPerformSelectorOnMainThreadModes()];
			}

			if ( mbCanceled )
				throw this;
						
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
			
			if(pdfMimeType.getLength())
			{
				[mpPostBody appendData:[[NSString stringWithFormat:@"--%@\r\n",stringBoundary] dataUsingEncoding:NSUTF8StringEncoding]];
							
				[mpPostBody appendData:[[NSString stringWithString:@"Content-Disposition: form-data; name=\"pdf\"; filename=\"unused.pdf\"\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
				[mpPostBody appendData:[[NSString stringWithFormat:@"Content-Type: %@\r\n\r\n", [NSString stringWithUTF8String: pdfMimeType.getStr()]] dataUsingEncoding:NSUTF8StringEncoding]];
							
				[mpPostBody appendData:[NSData dataWithContentsOfURL:[NSURL URLWithString:[NSString stringWithUTF8String: pdfExportURLutf8.getStr()]]]];
			}
						
			// add HTML zip file data
			
			if(htmlMimeType.getLength())
			{
				[mpPostBody appendData:[[NSString stringWithFormat:@"\r\n--%@\r\n",stringBoundary] dataUsingEncoding:NSUTF8StringEncoding]];
							
				[mpPostBody appendData:[[NSString stringWithString:@"Content-Disposition: form-data; name=\"html\"; filename=\"unused.zip\"\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
				[mpPostBody appendData:[[NSString stringWithFormat:@"Content-Type: %@\r\n\r\n", [NSString stringWithUTF8String: htmlMimeType.getStr()]] dataUsingEncoding:NSUTF8StringEncoding]];
							
				[mpPostBody appendData:[NSData dataWithContentsOfFile:[NSString stringWithUTF8String: htmlExportZipFileutf8.getStr()]]];
			}

			// add ODF data tagged with the appropriate mime type
			
			[mpPostBody appendData:[[NSString stringWithFormat:@"\r\n--%@\r\n",stringBoundary] dataUsingEncoding:NSUTF8StringEncoding]];
			
			OString odfPartName = OUStringToOString(docExtension.copy(1),RTL_TEXTENCODING_UTF8); // extension with first period stripped off
									
			[mpPostBody appendData:[[NSString stringWithFormat:@"Content-Disposition: form-data; name=\"%@\"; filename=\"unused.%@\"\r\n\r\n", [NSString stringWithUTF8String: odfPartName.getStr()], [NSString stringWithUTF8String: odfPartName.getStr()]] dataUsingEncoding:NSUTF8StringEncoding]];
			[mpPostBody appendData:[[NSString stringWithFormat:@"Content-Type: %@\r\n\r\n", [NSString stringWithUTF8String: mimeType.getStr()]] dataUsingEncoding:NSUTF8StringEncoding]];
						
			[mpPostBody appendData:[NSData dataWithContentsOfURL:[NSURL URLWithString:[NSString stringWithUTF8String: openDocExportURLutf8.getStr()]]]];

			// add Office document data tagged with the appropriate mime type
			
			if(officeMimeType.getLength())
			{
				[mpPostBody appendData:[[NSString stringWithFormat:@"\r\n--%@\r\n",stringBoundary] dataUsingEncoding:NSUTF8StringEncoding]];
				
				OString officePartName = OUStringToOString(officeDocExtension.copy(1),RTL_TEXTENCODING_UTF8); // extension with first period stripped off
										
				[mpPostBody appendData:[[NSString stringWithFormat:@"Content-Disposition: form-data; name=\"%@\"; filename=\"unused.%@\"\r\n\r\n", [NSString stringWithUTF8String: officePartName.getStr()], [NSString stringWithUTF8String: officePartName.getStr()]] dataUsingEncoding:NSUTF8StringEncoding]];
				[mpPostBody appendData:[[NSString stringWithFormat:@"Content-Type: %@\r\n\r\n", [NSString stringWithUTF8String: officeMimeType.getStr()]] dataUsingEncoding:NSUTF8StringEncoding]];
						
				[mpPostBody appendData:[NSData dataWithContentsOfURL:[NSURL URLWithString:[NSString stringWithUTF8String: officeDocExportURLutf8.getStr()]]]];
			}

			// add UUID
			
			[mpPostBody appendData:[[NSString stringWithFormat:@"\r\n--%@\r\n",stringBoundary] dataUsingEncoding:NSUTF8StringEncoding]];
			
			OString uuidUtf8 = OUStringToOString(maSaveUUID,RTL_TEXTENCODING_UTF8);
			
			[mpPostBody appendData:[[NSString stringWithString:@"Content-Disposition: form-data; name=\"UUID\"\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
						
			[mpPostBody appendData:[[NSString stringWithUTF8String: uuidUtf8.getStr()] dataUsingEncoding:NSUTF8StringEncoding]];		

			// close out form
			
			[mpPostBody appendData:[[NSString stringWithFormat:@"\r\n--%@--\r\n",stringBoundary] dataUsingEncoding:NSUTF8StringEncoding]];
			
#ifdef DEBUG
			// print post data to stderr
			
			fprintf( stderr, "NeoMobileExportFileAppEvent::ExportFile start of post request\n");
			fwrite( [mpPostBody bytes], [mpPostBody length], 1, stderr );
			fprintf( stderr, "NeoMobileExportFileAppEvent::ExportFile end of post request\n");
#endif	// DEBUG
			
			}
			catch (...)
			{
			}
					
			// remove exported files on disk now that we've finished assembling
			// our post in memory
			
			if(pdfExportURLutf8.getLength())
				[fileMgr performSelectorOnMainThread:@selector(removeItem:) withObject:[[NSURL URLWithString:[NSString stringWithUTF8String: pdfExportURLutf8.getStr()]] path] waitUntilDone:YES modes:NeoMobileGetPerformSelectorOnMainThreadModes()];
			if(htmlExportZipFileutf8.getLength())
				[fileMgr performSelectorOnMainThread:@selector(removeItem:) withObject:[NSString stringWithUTF8String: htmlExportZipFileutf8.getStr()] waitUntilDone:YES modes:NeoMobileGetPerformSelectorOnMainThreadModes()];
			if(openDocExportURLutf8.getLength())
				[fileMgr performSelectorOnMainThread:@selector(removeItem:) withObject:[[NSURL URLWithString:[NSString stringWithUTF8String: openDocExportURLutf8.getStr()]] path] waitUntilDone:YES modes:NeoMobileGetPerformSelectorOnMainThreadModes()];
			if(officeDocExportURLutf8.getLength())
				[fileMgr performSelectorOnMainThread:@selector(removeItem:) withObject:[[NSURL URLWithString:[NSString stringWithUTF8String: officeDocExportURLutf8.getStr()]] path] waitUntilDone:YES modes:NeoMobileGetPerformSelectorOnMainThreadModes()];
			
			[fileMgr release];
				
			// free our autorelease pool
			[pool release];
		}
		else
		{
#ifdef DEBUG
			fprintf( stderr, "NeoMobileExportFileAppEvent::ExportFile : I'm notn Main!\n");
#endif	// DEBUG
		}
		
		mbFinished = true;
	}

	return 0;
}

void NeoMobileExportFileAppEvent::Execute()
{
#if SUPD == 680
	// Do not allow pre-OpenOffice.org 3.0 versions to run this extension
	if ( !IsRunningInNeoOffice() )
	{
		mbCanceled = true;
		return;
	}
#endif	// SUPD == 680

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( !pApp || !Application::IsInMain() )
	{
		mbCanceled = true;
		return;
	}

	vos::IMutex& rSolarMutex = Application::GetSolarMutex();
	rSolarMutex.acquire();

	Application::PostUserEvent( LINK( this, NeoMobileExportFileAppEvent, ExportFile ) );
	if ( IsRunningInNeoOffice() )
		rSolarMutex.release();

	// Dispatch any pending native events until event is
	// dispatched or cancelled
	while ( Application::IsInMain() && !IsFinished() && !IsCanceled() )
	{
		if ( IsRunningInNeoOffice() )
		{
			NSEvent *pEvent;
			while ( ( pEvent = [pApp nextEventMatchingMask:NSAnyEventMask untilDate:nil inMode:( [pApp modalWindow] ? NSModalPanelRunLoopMode : NSDefaultRunLoopMode ) dequeue:YES] ) != nil )
				[pApp sendEvent:pEvent];
		}
		else
		{
			Application::Reschedule();
		}
	}

	if ( !IsFinished() && !IsCanceled() )
		mbCanceled = true;

	if ( !IsRunningInNeoOffice() )
		rSolarMutex.release();
}
