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
 *   Modified July 2022 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 3.3
 *   of the Mozilla Public License, v. 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "osx/salinst.h"
#include "osx/a11yfactory.h"
#include "osx/a11yfocustracker.hxx"

#include "a11yfocuslistener.hxx"
#include "a11yrolehelper.h"
#include "a11ywrapperbutton.h"
#include "a11ywrapperstatictext.h"
#include "a11ywrappertextarea.h"
#include "a11ywrappercheckbox.h"
#include "a11ywrappercombobox.h"
#include "a11ywrappergroup.h"
#include "a11ywrapperlist.h"
#include "a11ywrapperradiobutton.h"
#include "a11ywrapperradiogroup.h"
#include "a11ywrapperrow.h"
#include "a11ywrapperscrollarea.h"
#include "a11ywrapperscrollbar.h"
#include "a11ywrappersplitter.h"
#include "a11ywrappertabgroup.h"
#include "a11ywrappertoolbar.h"
#include "a11ytablewrapper.h"

#include <com/sun/star/accessibility/AccessibleStateType.hpp>

#ifdef USE_JAVA

#include <osl/objcutils.h>

#include "a11yactionwrapper.h"

#include "../java/source/app/salinst_cocoa.h"
#include "../java/source/java/VCLEventQueue_cocoa.h"

#define AQUA11Y_MAX_REMOVE_BATCH_SIZE 10000

#endif	// USE_JAVA

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;

static bool enabled = false; 

@implementation AquaA11yFactory : NSObject

#pragma mark -
#pragma mark Wrapper Repository

+(NSMutableDictionary *)allWrapper {
    static NSMutableDictionary * mdAllWrapper = nil;
    if ( mdAllWrapper == nil ) {
#ifdef USE_JAVA
        mdAllWrapper = [ NSMutableDictionary dictionaryWithCapacity: AQUA11Y_MAX_REMOVE_BATCH_SIZE * 10 ];
        if ( mdAllWrapper )
            [ mdAllWrapper retain ];
#else	// USE_JAVA
        mdAllWrapper = [ [ [ NSMutableDictionary alloc ] init ] retain ];
#endif	// USE_JAVA
        // initialize keyboard focus tracker
        rtl::Reference< AquaA11yFocusListener > listener( AquaA11yFocusListener::get() );
        AquaA11yFocusTracker::get().setFocusListener(listener.get());
        enabled = true;      
    }
    return mdAllWrapper;
}

#ifdef USE_JAVA

+(NSMutableDictionary *)radioGroupWrapper {
    if ( ! enabled )
        [ AquaA11yFactory allWrapper ];

    static NSMutableDictionary * mdRadioGroupWrapper = nil;
    if ( mdRadioGroupWrapper == nil ) {
        mdRadioGroupWrapper = [ NSMutableDictionary dictionaryWithCapacity: 100 ];
        if ( mdRadioGroupWrapper )
            [ mdRadioGroupWrapper retain ];
    }
    return mdRadioGroupWrapper;
}

+(NSMutableDictionary *)reverseWrapper {
    if ( ! enabled ) 
        [ AquaA11yFactory allWrapper ];

    static NSMutableDictionary * mdReverseWrapper = nil;
    if ( mdReverseWrapper == nil ) {
        mdReverseWrapper = [ NSMutableDictionary dictionaryWithCapacity: AQUA11Y_MAX_REMOVE_BATCH_SIZE * 10 ];
        if ( mdReverseWrapper )
            [ mdReverseWrapper retain ];
    }
    return mdReverseWrapper;
}

#endif	// USE_JAVA

+(NSValue *)keyForAccessibleContext: (Reference < XAccessibleContext >) rxAccessibleContext {
    return [ NSValue valueWithPointer: rxAccessibleContext.get() ];
}

#ifdef USE_JAVA

+(NSValue *)keyForElement: (id<NSAccessibility>) pElement {
    return [ NSValue valueWithPointer: pElement ];
}

#else	// USE_JAVA

+(NSValue *)keyForAccessibleContextAsRadioGroup: (Reference < XAccessibleContext >) rxAccessibleContext {
    return [ NSValue valueWithPointer: ( rxAccessibleContext.get() + 2 ) ];
}

#endif	// USE_JAVA

+(AquaA11yWrapper *)wrapperForAccessible: (Reference < XAccessible >) rxAccessible {
    if ( rxAccessible.is() ) {
        Reference< XAccessibleContext > xAccessibleContext = rxAccessible->getAccessibleContext();
        if( xAccessibleContext.is() ) {
            return [ AquaA11yFactory wrapperForAccessibleContext: xAccessibleContext ];
        }
    }
    return nil;
}

+(AquaA11yWrapper *)wrapperForAccessibleContext: (Reference < XAccessibleContext >) rxAccessibleContext {
    return [ AquaA11yFactory wrapperForAccessibleContext: rxAccessibleContext createIfNotExists: YES asRadioGroup: NO ];
}

+(AquaA11yWrapper *)wrapperForAccessibleContext: (Reference < XAccessibleContext >) rxAccessibleContext createIfNotExists:(BOOL) bCreate {
    return [ AquaA11yFactory wrapperForAccessibleContext: rxAccessibleContext createIfNotExists: bCreate asRadioGroup: NO ];
}

+(AquaA11yWrapper *)wrapperForAccessibleContext: (Reference < XAccessibleContext >) rxAccessibleContext createIfNotExists:(BOOL) bCreate asRadioGroup:(BOOL) asRadioGroup{
#ifdef USE_JAVA
    NSValue * nKey = [ AquaA11yFactory keyForAccessibleContext: rxAccessibleContext ];
    NSMutableDictionary * dWrapper = ( asRadioGroup ? [ AquaA11yFactory radioGroupWrapper ] : [ AquaA11yFactory allWrapper ] );
    AquaA11yWrapper * aWrapper = (AquaA11yWrapper *) [ dWrapper objectForKey: nKey ];
#else	// USE_JAVA
    NSMutableDictionary * dAllWrapper = [ AquaA11yFactory allWrapper ];
    NSValue * nKey = nil;
    if ( asRadioGroup ) {
        nKey = [ AquaA11yFactory keyForAccessibleContextAsRadioGroup: rxAccessibleContext ];
    } else {
        nKey = [ AquaA11yFactory keyForAccessibleContext: rxAccessibleContext ];
    }
    AquaA11yWrapper * aWrapper = (AquaA11yWrapper *) [ dAllWrapper objectForKey: nKey ];
#endif	// USE_JAVA
#ifdef USE_JAVA
    if ( !aWrapper && bCreate ) {
#else	// USE_JAVA
    if ( aWrapper != nil ) {
        [ aWrapper retain ];
    } else if ( bCreate ) {
#endif	// USE_JAVA
        NSString * nativeRole = [ AquaA11yRoleHelper getNativeRoleFrom: rxAccessibleContext.get() ];
        // TODO: reflection
        if ( [ nativeRole isEqualToString: NSAccessibilityButtonRole ] ) {
            aWrapper = [ [ AquaA11yWrapperButton alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityTextAreaRole ] ) {
            aWrapper = [ [ AquaA11yWrapperTextArea alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityStaticTextRole ] ) {
            aWrapper = [ [ AquaA11yWrapperStaticText alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityComboBoxRole ] ) {
            aWrapper = [ [ AquaA11yWrapperComboBox alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityGroupRole ] ) {
            aWrapper = [ [ AquaA11yWrapperGroup alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityToolbarRole ] ) {
            aWrapper = [ [ AquaA11yWrapperToolbar alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityScrollAreaRole ] ) {
            aWrapper = [ [ AquaA11yWrapperScrollArea alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityTabGroupRole ] ) {
            aWrapper = [ [ AquaA11yWrapperTabGroup alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityScrollBarRole ] ) {
            aWrapper = [ [ AquaA11yWrapperScrollBar alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityCheckBoxRole ] ) {
            aWrapper = [ [ AquaA11yWrapperCheckBox alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityRadioGroupRole ] ) {
            aWrapper = [ [ AquaA11yWrapperRadioGroup alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityRadioButtonRole ] ) {
            aWrapper = [ [ AquaA11yWrapperRadioButton alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityRowRole ] ) {
            aWrapper = [ [ AquaA11yWrapperRow alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityListRole ] ) {
            aWrapper = [ [ AquaA11yWrapperList alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilitySplitterRole ] ) {
            aWrapper = [ [ AquaA11yWrapperSplitter alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityTableRole ] ) {
            aWrapper = [ [ AquaA11yTableWrapper alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else {
            aWrapper = [ [ AquaA11yWrapper alloc ] initWithAccessibleContext: rxAccessibleContext ];
        }
#ifdef USE_JAVA
        [ aWrapper autorelease ];
#else	// USE_JAVA
        [ nativeRole release ];
#endif	// USE_JAVA
        [ aWrapper setActsAsRadioGroup: asRadioGroup ];
        #if 0
        /* #i102033# NSAccessibility does not seemt to know an equivalent for transient children.
           That means we need to cache this, else e.g. tree list boxes are not accessible (moreover
           it crashes by notifying dead objects - which would seemt o be another bug)
           
           FIXME:
           Unfortunately this can increase memory consumption drastically until the non transient parent
           is destroyed an finally all the transients are released.
        */
        if ( ! rxAccessibleContext -> getAccessibleStateSet() -> contains ( AccessibleStateType::TRANSIENT ) )
        #endif
        {
#ifdef USE_JAVA
            [ dWrapper setObject: aWrapper forKey: nKey ];
            [ [ AquaA11yFactory reverseWrapper ] setObject: nKey forKey: [ AquaA11yFactory keyForElement: aWrapper ] ];
#else	// USE_JAVA
            [ dAllWrapper setObject: aWrapper forKey: nKey ];
#endif	// USE_JAVA
            /* fdo#67410: Accessibility notifications are not delivered on NSView subclasses that do not
               "reasonably" participate in NSView hierarchy (perhaps the only important point is
               that the view is a transitive subview of the NSWindow's content view, but I
               did not try to verify that).

               So let the superview-subviews relationship mirror the AXParent-AXChildren relationship.
            */
            id parent = [aWrapper accessibilityAttributeValue:NSAccessibilityParentAttribute];
#ifdef USE_JAVA
            if (parent && [aWrapper isKindOfClass:[NSView class]]) {
#else	// USE_JAVA
            if (parent) {
#endif	// USE_JAVA
                if ([parent isKindOfClass:[NSView class]]) {
                    // SAL_DEBUG("Wrapper INIT: " << [[aWrapper description] UTF8String] << " ==> " << [[parent description] UTF8String]);
                    NSView *parentView = (NSView *)parent;
#ifdef USE_JAVA
                    [parentView addSubview:(NSView *)aWrapper positioned:NSWindowBelow relativeTo:nil];
                } else if ([parent isKindOfClass:[VCLPanel class]] || [parent isKindOfClass:[VCLWindow class]]) {
#else	// USE_JAVA
                    [parentView addSubview:aWrapper positioned:NSWindowBelow relativeTo:nil];
                } else if ([parent isKindOfClass:NSClassFromString(@"SalFrameWindow")]) {
#endif	// USE_JAVA
                    NSWindow *window = (NSWindow *)parent;
                    NSView *salView = [window contentView];
                    // SAL_DEBUG("Wrapper INIT SAL: " << [[aWrapper description] UTF8String] << " ==> " << [[salView description] UTF8String]);
#ifdef USE_JAVA
                    [salView addSubview:(NSView *)aWrapper positioned:NSWindowBelow relativeTo:nil];
#else	// USE_JAVA
                    [salView addSubview:aWrapper positioned:NSWindowBelow relativeTo:nil];
#endif	// USE_JAVA
                } else {
                    // SAL_DEBUG("Wrapper INIT: !! " << [[aWrapper description] UTF8String] << " !==>! " << [[parent description] UTF8String] << "!!");
                }
            } else {
                // SAL_DEBUG("Wrapper INIT: " << [[aWrapper description] UTF8String] << " ==> NO PARENT");
            }
        }
    }
    return aWrapper;
}

#ifdef USE_JAVA
+(void)insertIntoWrapperRepository: (id<NSAccessibility>) viewElement forAccessibleContext: (Reference < XAccessibleContext >) rxAccessibleContext {
#else	// USE_JAVA
+(void)insertIntoWrapperRepository: (NSView *) viewElement forAccessibleContext: (Reference < XAccessibleContext >) rxAccessibleContext {
#endif	// USE_JAVA
    NSMutableDictionary * dAllWrapper = [ AquaA11yFactory allWrapper ];
#ifdef USE_JAVA
    NSValue * nKey = [ AquaA11yFactory keyForAccessibleContext: rxAccessibleContext ];
    [ dAllWrapper setObject: viewElement forKey: nKey ];
    [ [ AquaA11yFactory reverseWrapper ] setObject: nKey forKey: [ AquaA11yFactory keyForElement: viewElement ] ];
#else	// USE_JAVA
    [ dAllWrapper setObject: viewElement forKey: [ AquaA11yFactory keyForAccessibleContext: rxAccessibleContext ] ];
#endif	// USE_JAVA
}

+(void)removeFromWrapperRepositoryFor: (::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleContext >) rxAccessibleContext {
    // TODO: when RADIO_BUTTON search for associated RadioGroup-wrapper and delete that as well
    AquaA11yWrapper * theWrapper = [ AquaA11yFactory wrapperForAccessibleContext: rxAccessibleContext createIfNotExists: NO ];
    if ( theWrapper != nil ) {
#ifdef USE_JAVA
        NSAccessibilityPostNotification( theWrapper, NSAccessibilityUIElementDestroyedNotification );

        if ([theWrapper isKindOfClass:[NSView class]] && ![theWrapper isKindOfClass:[VCLView class]]) {
            [(NSView *)theWrapper removeFromSuperviewWithoutNeedingDisplay];
#else	// USE_JAVA
        if (![theWrapper isKindOfClass:NSClassFromString(@"SalFrameView")]) {
            [theWrapper removeFromSuperview];
#endif	// USE_JAVA
        }
        [ [ AquaA11yFactory allWrapper ] removeObjectForKey: [ AquaA11yFactory keyForAccessibleContext: rxAccessibleContext ] ];
#ifndef USE_JAVA
        [ theWrapper release ];
#endif	// !USE_JAVA
    }
}

#ifdef USE_JAVA

+(void)removeFromWrapperRepositoryForWrapper: (AquaA11yWrapper *) theWrapper {
    if ( ! theWrapper )
        return;

    NSAccessibilityPostNotification( theWrapper, NSAccessibilityUIElementDestroyedNotification );

    if ([theWrapper isKindOfClass:[NSView class]] && ![theWrapper isKindOfClass:[VCLView class]]) {
        [(NSView *)theWrapper removeFromSuperviewWithoutNeedingDisplay];
    }

    // The accessible context pointer may be NULL because this selector is
    // called asynchronously so remove any orphaned wrappers
    NSMutableDictionary * dReverseWrapper = [ AquaA11yFactory reverseWrapper ];
    NSValue *nKey = [ AquaA11yFactory keyForElement: theWrapper ];
    NSValue *pValue = [ dReverseWrapper objectForKey: nKey ];
    if ( pValue ) {
        NSMutableDictionary * dCurrentWrapper = ( [ theWrapper actsAsRadioGroup ] ? [ AquaA11yFactory radioGroupWrapper ] : [ AquaA11yFactory allWrapper ] );
        id pCurrentValue = [ dCurrentWrapper objectForKey: pValue ];
        if ( pCurrentValue && pCurrentValue == theWrapper )
            [ dCurrentWrapper removeObjectForKey: pValue ];

        [ dReverseWrapper removeObjectForKey: nKey ];
    }
}

+(void)registerView: (id<NSAccessibility>) theView {
#else	// USE_JAVA
+(void)registerView: (NSView *) theView {
#endif	// USE_JAVA
    if ( enabled && [ theView isKindOfClass: [ AquaA11yWrapper class ] ] ) {
        // insertIntoWrapperRepository gets called from SalFrameView itself to bootstrap the bridge initially
        [ (AquaA11yWrapper *) theView accessibleContext ];
    }
}

#ifdef USE_JAVA
+(void)revokeView: (id<NSAccessibility>) theView {
#else	// USE_JAVA
+(void)revokeView: (NSView *) theView {
#endif	// USE_JAVA
    if ( enabled && [ theView isKindOfClass: [ AquaA11yWrapper class ] ] ) {
#ifdef USE_JAVA
        [ AquaA11yRemoveFromWrapperRepository addElementToPendingRemovalQueue: (AquaA11yWrapper *)theView ];
#else	// USE_JAVA
        [ AquaA11yFactory removeFromWrapperRepositoryFor: [ (AquaA11yWrapper *) theView accessibleContext ] ];
#endif	// USE_JAVA
    }
}

@end

#ifdef USE_JAVA

@implementation AquaA11yWrapperForAccessibleContext

+ (id)createWithAccessibleContext:(::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >)xAccessibleContext
{
    AquaA11yWrapperForAccessibleContext *pRet = [ [ AquaA11yWrapperForAccessibleContext alloc ] initWithAccessibleContext: xAccessibleContext ];
    [ pRet autorelease ];
    return pRet;
}

- (id)initWithAccessibleContext:(::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >)xAccessibleContext
{
    [super init];

    mxAccessibleContext = xAccessibleContext;

    return self;
}

- (void)dealloc
{
    if ( mpWrapper )
        [ mpWrapper release ];

    [super dealloc];
}

- (void)wrapperForAccessibleContext:(id)pObject
{
    (void)pObject;

    ACQUIRE_DRAGPRINTLOCK
    if ( ! mpWrapper && mxAccessibleContext.is() ) {
        mpWrapper = [ AquaA11yFactory wrapperForAccessibleContext: mxAccessibleContext ];
        if ( mpWrapper )
            [ mpWrapper retain ];
    }
    RELEASE_DRAGPRINTLOCK
}

- (AquaA11yWrapper *)wrapper
{
    return mpWrapper;
}

@end

static NSMutableArray<AquaA11yRemoveFromWrapperRepository*> *pPendingRemoveFromWrapperRepositoryQueue = nil;
static ::osl::Mutex aPendingRemoveFromWrapperRepositoryQueueMutex;
static BOOL bInRemovePendingFromWrapperRepository = NO;

@implementation AquaA11yRemoveFromWrapperRepository

+ (id)addElementToPendingRemovalQueue:(AquaA11yWrapper *)pElement
{
    AquaA11yRemoveFromWrapperRepository *pRet = [ [ AquaA11yRemoveFromWrapperRepository alloc ] initWithElement: pElement ];
    [ pRet autorelease ];
    return pRet;
}

- (id)initWithElement:(AquaA11yWrapper *)pElement
{
    [ super init ];

    mpElement = pElement;
    if ( mpElement ) {
        [mpElement retain];

        // Mark the element as disposed immediately
        [mpElement disposing];
    }

    ::osl::ClearableMutexGuard aGuard( aPendingRemoveFromWrapperRepositoryQueueMutex );

    if ( !pPendingRemoveFromWrapperRepositoryQueue ) {
        pPendingRemoveFromWrapperRepositoryQueue = [ NSMutableArray arrayWithCapacity: 10 ];
        if ( pPendingRemoveFromWrapperRepositoryQueue )
            [ pPendingRemoveFromWrapperRepositoryQueue retain ];
    }

    // Post the removal asynchronously
    if ( pPendingRemoveFromWrapperRepositoryQueue ) {
        [ pPendingRemoveFromWrapperRepositoryQueue addObject: self ];
        osl_performSelectorOnMainThread( self, @selector(removeFromWrapperRepository:), self, NO );
    }

    aGuard.clear();

    return self;
}

- (void)dealloc
{
    if ( mpElement )
        [ mpElement release ];

    [ super dealloc ];
}

- (void)removeFromWrapperRepository:(id)pObject
{
    if ( bInRemovePendingFromWrapperRepository )
        return;

    // Prevent posting of notification if we are already within an
    // NSAccessibility call
    if ( VCLInstance_isInOrAcquiringDragPrintLock() ) {
        ::osl::MutexGuard aGuard( aPendingRemoveFromWrapperRepositoryQueueMutex );
        if ( pPendingRemoveFromWrapperRepositoryQueue && [ pPendingRemoveFromWrapperRepositoryQueue count ] )
            [self performSelector:@selector(removeFromWrapperRepository:) withObject:pObject afterDelay:0.1f];
        return;
    }

    bInRemovePendingFromWrapperRepository = YES;

    ACQUIRE_DRAGPRINTLOCK

    // Prioritize pending macOS accessiblity calls
    CFRunLoopRef aRunLoop = CFRunLoopGetCurrent();
    if ( aRunLoop )
    {
        CFRunLoopMode aMode = CFRunLoopCopyCurrentMode( aRunLoop );
        if ( aMode )
        {
            while ( CFRunLoopRunInMode( aMode, 0, false ) == kCFRunLoopRunHandledSource )
                ;
            CFRelease( aMode );
        }
    }

    ::osl::ClearableMutexGuard aGuard( aPendingRemoveFromWrapperRepositoryQueueMutex );

    if ( pPendingRemoveFromWrapperRepositoryQueue ) {
        // Eliminate random spinnning beach ball by removing only a small batch
        // of pending wrappers in each pass
        for ( NSUInteger i = 0; i < AQUA11Y_MAX_REMOVE_BATCH_SIZE && [ pPendingRemoveFromWrapperRepositoryQueue count ]; i++ ) {
            AquaA11yRemoveFromWrapperRepository *pRemoveFromWrapperRepository = [ pPendingRemoveFromWrapperRepositoryQueue objectAtIndex: 0 ];
            if ( pRemoveFromWrapperRepository && pRemoveFromWrapperRepository->mpElement ) {
                try {
                    [ AquaA11yFactory removeFromWrapperRepositoryForWrapper: pRemoveFromWrapperRepository->mpElement ];
                } catch ( const ::com::sun::star::lang::DisposedException& ) {
                } catch ( ... ) {
                    NSLog( @"Exception caught while in -[AquaA11yFactory removeFromWrapperRepositoryFor:]: %s", __PRETTY_FUNCTION__ );
                }
            }

            [ pPendingRemoveFromWrapperRepositoryQueue removeObjectAtIndex: 0 ];
            if ( [ pPendingRemoveFromWrapperRepositoryQueue count ] )
                [self performSelector:@selector(removeFromWrapperRepository:) withObject:pObject afterDelay:0.1f];
        }
    }

    aGuard.clear();
    RELEASE_DRAGPRINTLOCK

    bInRemovePendingFromWrapperRepository = NO;
}

@end

static NSMutableArray<AquaA11yPostNotification*> *pPendingPostNotificationQueue = nil;
static ::osl::Mutex aPendingPostNotificationQueueMutex;
static BOOL bInPostPendingNotifications = NO;
static NSDictionary *pPriorityDict = nil;

@implementation AquaA11yPostNotification

+ (id)addElementToPendingNotificationQueue:(id)pElement name:(NSAccessibilityNotificationName)pName
{
    AquaA11yPostNotification *pRet = [ [ AquaA11yPostNotification alloc ] initWithElement: pElement name: pName ];
    [ pRet autorelease ];
    return pRet;
}

- (id)initWithElement:(id)pElement name:(NSAccessibilityNotificationName)pName
{
    [ super init ];

    mpElement = pElement;
    if ( mpElement )
        [mpElement retain];
    mpName = pName;
    if ( mpName )
        [mpName retain];

    ::osl::ClearableMutexGuard aGuard( aPendingPostNotificationQueueMutex );

    if ( !pPendingPostNotificationQueue ) {
        pPendingPostNotificationQueue = [ NSMutableArray arrayWithCapacity: 10 ];
        if ( pPendingPostNotificationQueue )
            [ pPendingPostNotificationQueue retain ];
    }

    // Post the notification asynchronously
    if ( pPendingPostNotificationQueue ) {
        [ pPendingPostNotificationQueue addObject: self ];
        osl_performSelectorOnMainThread( self, @selector(postPendingNotifications:), self, NO );
    }

    aGuard.clear();

    return self;
}

- (void)dealloc
{
    if ( mpElement )
        [ mpElement release ];

    if ( mpName )
        [ mpName release ];

    [ super dealloc ];
}

- (void)postNotification
{
    if ( mpElement && mpName && ( ! [ mpElement isKindOfClass: [ AquaA11yWrapper class ] ] || ! [ (AquaA11yWrapper *)mpElement isDisposed ] ) ) {
        // Set priority low to increase the odds that transient objects'
        // notifications will get announced
        if ( pPriorityDict ) {
            pPriorityDict = [ NSDictionary dictionaryWithObjects: [ NSArray arrayWithObject: [ NSNumber numberWithInteger: NSAccessibilityPriorityLow ] ] forKeys: [ NSArray arrayWithObject: NSAccessibilityPriorityKey ] ];
            if ( pPriorityDict )
                [ pPriorityDict retain ];
        }

        if ( pPriorityDict )
            NSAccessibilityPostNotificationWithUserInfo( mpElement, mpName, pPriorityDict );
        else
            NSAccessibilityPostNotification( mpElement, mpName );
    }
}

- (void)postPendingNotifications:(id)pObject
{
    if ( bInPostPendingNotifications )
        return;

    // Prevent posting of notification if we are already within an
    // NSAccessibility call
    if ( VCLInstance_isInOrAcquiringDragPrintLock() ) {
        ::osl::MutexGuard aGuard( aPendingPostNotificationQueueMutex );
        if ( pPendingPostNotificationQueue && [ pPendingPostNotificationQueue count ] )
            [self performSelector:@selector(postPendingNotifications:) withObject:pObject afterDelay:0.01f];
        return;
    }

    if ( !ImplApplicationIsRunning() )
        return;

    bInPostPendingNotifications = YES;

    // Fix hanging when notifications cause a modal dialog to appear by
    // using application mutex instead of the drag print lock when
    // posting notifications
    comphelper::SolarMutex& rSolarMutex = Application::GetSolarMutex();
    rSolarMutex.acquire();

    // Prioritize pending macOS accessiblity calls
    CFRunLoopRef aRunLoop = CFRunLoopGetCurrent();
    if ( aRunLoop )
    {
        CFRunLoopMode aMode = CFRunLoopCopyCurrentMode( aRunLoop );
        if ( aMode )
        {
            while ( CFRunLoopRunInMode( aMode, 0, false ) == kCFRunLoopRunHandledSource )
                ;
            CFRelease( aMode );
        }
    }

    ::osl::ClearableMutexGuard aGuard( aPendingPostNotificationQueueMutex );

    if ( pPendingPostNotificationQueue && [ pPendingPostNotificationQueue count ] ) {
        AquaA11yPostNotification *pPostNotification = [ pPendingPostNotificationQueue objectAtIndex: 0 ];
        // Do not coalesce notifications as it appears to suppress selected
        // item notifications
        if ( pPostNotification ) {
            @try {
                [ pPostNotification postNotification ];
            }
            @catch ( NSException *pExc ) {
            }
        }
        [ pPendingPostNotificationQueue removeObjectAtIndex: 0 ];
        if ( [ pPendingPostNotificationQueue count ] )
            [self performSelector:@selector(postPendingNotifications:) withObject:pObject afterDelay:0.01f];
    }

    aGuard.clear();
    rSolarMutex.release();

    bInPostPendingNotifications = NO;
}

@end

@implementation AquaA11yDoAction

+ (id)addElementToPendingNotificationQueue:(id)pElement action:(NSAccessibilityActionName)pAction
{
    AquaA11yDoAction *pRet = [ [ AquaA11yDoAction alloc ] initWithElement: pElement action: pAction ];
    [ pRet autorelease ];
    return pRet;
}

- (id)initWithElement:(id)pElement action:(NSAccessibilityActionName)pAction
{
    [super initWithElement: pElement name: nil ];

    mpAction = pAction;
    if ( mpAction )
        [ mpAction retain ];

    return self;
}

- (void)dealloc
{
    if ( mpAction )
        [ mpAction release ];

    [ super dealloc ];
}

- (void)postNotification
{
    if ( mpElement && mpAction && ( ! [ mpElement isKindOfClass: [ AquaA11yWrapper class ] ] || ! [ (AquaA11yWrapper *)mpElement isDisposed ] ) )
        [ AquaA11yActionWrapper doAction: mpAction ofElement: mpElement ];
}

@end

@implementation AquaA11ySetValue

+ (id)addElementToPendingNotificationQueue:(id)pElement value:(id)pValue attribute:(NSAccessibilityAttributeName)pAttribute
{
    AquaA11ySetValue *pRet = [ [ AquaA11ySetValue alloc ] initWithElement: pElement value: pValue attribute: pAttribute ];
    [ pRet autorelease ];
    return pRet;
}

- (id)initWithElement:(id)pElement value:(id)pValue attribute:(NSAccessibilityAttributeName)pAttribute
{
    [super initWithElement: pElement name: nil ];

    mpValue = pValue;
    if ( mpValue )
        [ mpValue retain ];
    mpAttribute = pAttribute;
    if ( mpAttribute )
        [ mpAttribute retain ];

    return self;
}

- (void)dealloc
{
    if ( mpValue )
        [ mpValue release ];

    if ( mpAttribute )
        [ mpAttribute release ];

    [ super dealloc ];
}

- (void)postNotification
{
    if ( mpElement && mpAttribute && ( ! [ mpElement isKindOfClass: [ AquaA11yWrapper class ] ] || ! [ (AquaA11yWrapper *)mpElement isDisposed ] ) )
        [ (AquaA11yWrapper *)mpElement setValue: mpValue forAttribute: mpAttribute ];
}

@end

#endif	// USE_JAVA

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
