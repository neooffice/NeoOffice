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
 * 
 *   Modified May 2022 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 3.3
 *   of the Mozilla Public License, v. 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "rtl/ustrbuf.hxx"

#include "vcl/button.hxx"

#ifdef USE_JAVA
#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

#include "java/salsys.h"

#include "../java/source/app/salinst_cocoa.h"
#include "../java/source/java/VCLEventQueue_cocoa.h"
#else	// USE_JAVA
#include "osx/salsys.h"
#include "osx/saldata.hxx"
#include "osx/salinst.h"
#endif	// USE_JAVA
#include "quartz/utils.h"

#include "svids.hrc"

#ifndef USE_JAVA

AquaSalSystem::~AquaSalSystem()
{
}

unsigned int AquaSalSystem::GetDisplayScreenCount()
{
    NSArray* pScreens = [NSScreen screens];
    return pScreens ? [pScreens count] : 1;
}

Rectangle AquaSalSystem::GetDisplayScreenPosSizePixel( unsigned int nScreen )
{
    NSArray* pScreens = [NSScreen screens];
    Rectangle aRet;
    NSScreen* pScreen = nil;
    if( pScreens && nScreen < [pScreens count] )
        pScreen = [pScreens objectAtIndex: nScreen];
    else
        pScreen = [NSScreen mainScreen];

    if( pScreen )
    {
        NSRect aFrame = [pScreen frame];
        aRet = Rectangle( Point( static_cast<long int>(aFrame.origin.x), static_cast<long int>(aFrame.origin.y) ),
                          Size( static_cast<long int>(aFrame.size.width), static_cast<long int>(aFrame.size.height) ) );
    }
    return aRet;
}

OUString AquaSalSystem::GetDisplayScreenName( unsigned int nScreen )
{
   NSArray* pScreens = [NSScreen screens];
   OUString aRet;
   if( nScreen < [pScreens count] )
   {
        ResMgr* pMgr = ImplGetResMgr();
        if( pMgr )
        {
            OUString aScreenName(ResId(SV_MAC_SCREENNNAME, *pMgr).toString());
            aRet = aScreenName.replaceAll("%d", OUString::number(nScreen));
        }
   }
   return aRet;
}

#endif	// !USE_JAVA

static NSString* getStandardString( int nButtonId, bool bUseResources )
{
    OUString aText;
    if( bUseResources )
    {
        aText = Button::GetStandardText( nButtonId );
    }
    if( aText.isEmpty() ) // this is for bad cases, we might be missing the vcl resource
    {
        switch( nButtonId )
        {
        case BUTTON_OK:         aText = "OK";break;
        case BUTTON_ABORT:      aText = "Abort";break;
        case BUTTON_CANCEL:     aText = "Cancel";break;
        case BUTTON_RETRY:      aText = "Retry";break;
        case BUTTON_YES:        aText = "Yes";break;
        case BUTTON_NO :        aText = "No";break;
        }
    }
    return aText.isEmpty() ? nil : CreateNSString( aText);
}

#ifdef USE_JAVA

@interface VCLShowNativeMessageBox : NSObject
{
    NSAlert*                mpAlert;
    BOOL                    mbCancelled;
    BOOL                    mbFinished;
    NSModalResponse         mnModalResponse;
    NSString*               mpTitle;
    NSString*               mpMessage;
    NSString*               mpDefText;
    NSString*               mpAltText;
    NSString*               mpOthText;
}
+ (id)createShowNativeMessageBox:(NSString *)pTitle message:(NSString *)pMessage defText:(NSString *)pDefText altText:(NSString *)pAltText othText:(NSString *)pOthText;
- (void)checkForErrors:(id)pObject;
- (void)dealloc;
- (void)destroy:(id)pObject;
- (id)initShowNativeMessageBox:(NSString *)pTitle message:(NSString *)pMessage defText:(NSString *)pDefText altText:(NSString *)pAltText othText:(NSString *)pOthText;
- (BOOL)finished;
- (NSModalResponse)modalResponse;
- (void)showNativeMessageBox:(id)pObject;
@end

@implementation VCLShowNativeMessageBox

+ (id)createShowNativeMessageBox:(NSString *)pTitle message:(NSString *)pMessage defText:(NSString *)pDefText altText:(NSString *)pAltText othText:(NSString *)pOthText
{
    VCLShowNativeMessageBox *pRet = [[VCLShowNativeMessageBox alloc] initShowNativeMessageBox:pTitle message:pMessage defText:pDefText altText:pAltText othText:pOthText];
    [pRet autorelease];
    return pRet;
}

- (void)checkForErrors:(id)pObject
{
    // Detect if the alert window has been closed without any call to the
    // completion handler
    if ( !mbFinished && !mbCancelled && ( !mpAlert || ![mpAlert window] || ![[mpAlert window] isVisible] ) )
        mbCancelled = YES;
}

- (void)dealloc
{
    [self destroy:self];

    [super dealloc];
}

- (void)destroy:(id)pObject
{
    if ( mpAlert )
    {
        [mpAlert release];
        mpAlert = nil;
    }

    if ( mpTitle )
    {
        [mpTitle release];
        mpTitle = nil;
    }

    if ( mpMessage )
    {
        [mpMessage release];
        mpMessage = nil;
    }

    if ( mpDefText )
    {
        [mpDefText release];
        mpDefText = nil;
    }

    if ( mpAltText )
    {
        [mpAltText release];
        mpAltText = nil;
    }

    if ( mpOthText )
    {
        [mpOthText release];
        mpOthText = nil;
    }
}

- (id)initShowNativeMessageBox:(NSString *)pTitle message:(NSString *)pMessage defText:(NSString *)pDefText altText:(NSString *)pAltText othText:(NSString *)pOthText
{
    [super init];

    mpAlert = nil;
    mbCancelled = NO;
    mbFinished = NO;
    mnModalResponse = NSModalResponseCancel;
    mpTitle = pTitle;
    if ( mpTitle )
        [mpTitle retain];
    mpMessage = pMessage;
    if ( mpMessage )
        [mpMessage retain];
    mpDefText = pDefText;
    if ( mpDefText )
        [mpDefText retain];
    mpAltText = pAltText;
    if ( mpAltText )
        [mpAltText retain];
    mpOthText = pOthText;
    if ( mpOthText )
        [mpOthText retain];

    return self;
}

- (BOOL)finished
{
    return ( mbCancelled || mbFinished );
}

- (NSModalResponse)modalResponse
{
    return mnModalResponse;
}

- (void)showNativeMessageBox:(id)pObject
{
    // Do not allow recursion or reuse
    if ( mpAlert || mbCancelled || mbFinished )
    {
        return;
    }
    else if ( !pObject || ![pObject isKindOfClass:[NSWindow class]] )
    {
        mbCancelled = YES;
        return;
    }

    mnModalResponse = NSModalResponseCancel;

    NSApplication *pApp = [NSApplication sharedApplication];
    if ( !pApp )
    {
        mbCancelled = YES;
        return;
    }

    NSWindow *pParentWindow = [pApp modalWindow];
    if ( !pParentWindow || ![pParentWindow isVisible] || [pParentWindow isMiniaturized] )
    {
        pParentWindow = [pApp keyWindow];
        if ( !pParentWindow || ![pParentWindow isVisible] || [pParentWindow isMiniaturized] )
        {
            pParentWindow = [pApp mainWindow];
            if ( !pParentWindow || ![pParentWindow isVisible] || [pParentWindow isMiniaturized] )
            {
                mbCancelled = YES;
                return;
            }
        }
    }

    mpAlert = [[NSAlert alloc] init];
    if ( mpAlert )
    {
        if ( mpTitle )
            mpAlert.messageText = mpTitle;
        if ( mpMessage )
            mpAlert.informativeText = mpMessage;
        if ( mpDefText )
            [mpAlert addButtonWithTitle:mpDefText];
        if ( mpAltText )
            [mpAlert addButtonWithTitle:mpAltText];
        if ( mpOthText )
            [mpAlert addButtonWithTitle:mpOthText];

        @try
        {
            // When running in the sandbox, native alert dialog calls may
            // throw exceptions if the PowerBox daemon process is killed
            [mpAlert beginSheetModalForWindow:pParentWindow completionHandler:^(NSModalResponse nReturnCode) {
                mnModalResponse = nReturnCode;
                mbFinished = YES;
            }];
        }
        @catch ( NSException *pExc )
        {
            mbCancelled = YES;
            if ( pExc )
                NSLog( @"%@", [pExc callStackSymbols] );
        }
    }
    else
    {
        mbCancelled = YES;
    }
}

@end

int JavaSalSystem::ShowNativeMessageBox( const OUString& rTitle,
#else	// USE_JAVA
int AquaSalSystem::ShowNativeMessageBox( const OUString& rTitle,
#endif	// USE_JAVA
                                        const OUString& rMessage,
                                        int nButtonCombination,
                                        int nDefaultButton, bool bUseResources)
{
#ifdef USE_JAVA
    NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];
#endif	// USE_JAVA

    NSString* pTitle = CreateNSString( rTitle );
    NSString* pMessage = CreateNSString( rMessage );

#ifdef USE_JAVA
    NSLog( @"%@: %@\n", pTitle ? pTitle : @"Error", pMessage ? pMessage : @"Unknown error" );
#endif	// USE_JAVA

    struct id_entry
    {
        int nCombination;
        int nDefaultButton;
        int nTextIds[3];
    } aButtonIds[] =
    {
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK, { BUTTON_OK, -1, -1 } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK, { BUTTON_OK, BUTTON_CANCEL, -1 } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL, { BUTTON_CANCEL, BUTTON_OK, -1 } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_ABORT_RETRY_IGNORE, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_ABORT, { BUTTON_ABORT, BUTTON_IGNORE, BUTTON_RETRY } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_ABORT_RETRY_IGNORE, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_RETRY, { BUTTON_RETRY, BUTTON_IGNORE, BUTTON_ABORT } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_ABORT_RETRY_IGNORE, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_IGNORE, { BUTTON_IGNORE, BUTTON_IGNORE, BUTTON_ABORT } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO_CANCEL, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_YES, { BUTTON_YES, BUTTON_NO, BUTTON_CANCEL } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO_CANCEL, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO, { BUTTON_NO, BUTTON_YES, BUTTON_CANCEL } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO_CANCEL, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL, { BUTTON_CANCEL, BUTTON_YES, BUTTON_NO } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_YES, { BUTTON_YES, BUTTON_NO, -1 } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO, { BUTTON_NO, BUTTON_YES, -1 } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_RETRY_CANCEL, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_RETRY, { BUTTON_RETRY, BUTTON_CANCEL, -1 } },
        { SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_RETRY_CANCEL, SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL, { BUTTON_CANCEL, BUTTON_RETRY, -1 } }
    };

    NSString* pDefText = nil;
    NSString* pAltText = nil;
    NSString* pOthText = nil;

    unsigned int nC;
    for( nC = 0; nC < sizeof(aButtonIds)/sizeof(aButtonIds[0]); nC++ )
    {
        if( aButtonIds[nC].nCombination == nButtonCombination )
        {
            if( aButtonIds[nC].nDefaultButton == nDefaultButton )
            {
                if( aButtonIds[nC].nTextIds[0] != -1 )
                    pDefText = getStandardString(
                        aButtonIds[nC].nTextIds[0], bUseResources );
                if( aButtonIds[nC].nTextIds[1] != -1 )
                    pAltText = getStandardString(
                        aButtonIds[nC].nTextIds[1], bUseResources );
                if( aButtonIds[nC].nTextIds[2] != -1 )
                    pOthText = getStandardString(
                        aButtonIds[nC].nTextIds[2], bUseResources );
                break;
            }
        }
    }

#ifdef USE_JAVA
    int nResult = 0;

    VCLShowNativeMessageBox *pVCLShowNativeMessageBox = [VCLShowNativeMessageBox createShowNativeMessageBox:pTitle message:pMessage defText:pDefText altText:pAltText othText:pOthText];
    if ( ![pVCLShowNativeMessageBox finished] )
    {
        NSWindow *pNSWindow = nil;
        if ( Application_beginModalSheet( &pNSWindow ) )
        {
            NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
            [pVCLShowNativeMessageBox performSelectorOnMainThread:@selector(showNativeMessageBox:) withObject:pNSWindow waitUntilDone:YES modes:pModes];
             while ( ![pVCLShowNativeMessageBox finished] && !Application::IsShutDown() )
                 [pVCLShowNativeMessageBox performSelectorOnMainThread:@selector(checkForErrors:) withObject:pVCLShowNativeMessageBox waitUntilDone:YES modes:pModes];

            NSModalResponse nModalResponse = [pVCLShowNativeMessageBox modalResponse];
            if ( nModalResponse == NSAlertFirstButtonReturn )
                nResult = 1;
            else if ( nModalResponse == NSAlertSecondButtonReturn )
                nResult = 2;
            else if ( nModalResponse == NSAlertThirdButtonReturn )
                nResult = 3;

            [pVCLShowNativeMessageBox performSelectorOnMainThread:@selector(destroy:) withObject:pVCLShowNativeMessageBox waitUntilDone:YES modes:pModes];

            Application_endModalSheet();
        }
    }
#else	// USE_JAVA
    int nResult = NSRunAlertPanel( pTitle, @"%@", pDefText, pAltText, pOthText, pMessage );
#endif	// USE_JAVA

    if( pTitle )
        [pTitle release];
    if( pMessage )
        [pMessage release];
    if( pDefText )
        [pDefText release];
    if( pAltText )
        [pAltText release];
    if( pOthText )
        [pOthText release];

#ifdef USE_JAVA
    [pPool release];
#endif	// USE_JAVA

    int nRet = 0;
    if( nC < sizeof(aButtonIds)/sizeof(aButtonIds[0]) && nResult >= 1 && nResult <= 3 )
    {
        int nPressed = aButtonIds[nC].nTextIds[nResult-1];
        switch( nPressed )
        {
        case BUTTON_NO:     nRet = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO; break;
        case BUTTON_YES:    nRet = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_YES; break;
        case BUTTON_OK:     nRet = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK; break;
        case BUTTON_CANCEL: nRet = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL; break;
        case BUTTON_ABORT:  nRet = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_ABORT; break;
        case BUTTON_RETRY:  nRet = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_RETRY; break;
        case BUTTON_IGNORE: nRet = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_IGNORE; break;
        }
    }

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
