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


#ifdef USE_JAVA
#include "java/salinst.h"
#else	// USE_JAVA
#include "osx/salinst.h"
#endif	// USE_JAVA
#include "osx/saldata.hxx"

#include "osx/a11ywrapper.h"
#include "osx/a11ylistener.hxx"
#include "osx/a11yfactory.h"
#include "osx/a11yfocustracker.hxx"

#include "quartz/utils.h"

#include "a11yfocuslistener.hxx"
#include "a11yactionwrapper.h"
#include "a11ycomponentwrapper.h"
#include "a11yselectionwrapper.h"
#include "a11ytablewrapper.h"
#include "a11ytextwrapper.h"
#include "a11yvaluewrapper.h"
#include "a11yrolehelper.h"

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#ifdef USE_JAVA
#include "../java/source/app/salinst_cocoa.h"
#include "../java/source/java/VCLEventQueue_cocoa.h"
#endif	// USE_JAVA

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

#ifdef USE_JAVA
@interface NSObject (AquaA11yWrapper)
- (id)accessibilityAttributeValue:(NSAccessibilityAttributeName)aAttribute;
@end
#else	// USE_JAVA
@interface SalFrameWindow : NSWindow
{
}
-(Reference<XAccessibleContext>)accessibleContext;
@end
#endif	// USE_JAVA

static BOOL isPopupMenuOpen = NO;

static std::ostream &operator<<(std::ostream &s, NSObject *obj) {
    return s << [[obj description] UTF8String];
}

#ifndef _LP64

// In 64-bit code NSPoint == CGPoint, and CGPoint already has
// an operator<< in vcl/inc/quartz/util.h

static std::ostream &operator<<(std::ostream &s, NSPoint point) {
    return s << NSStringFromPoint(point);
}

#endif

#ifdef USE_JAVA
@implementation AquaA11yWrapper
#else	// USE_JAVA
@implementation AquaA11yWrapper : NSView
#endif	// USE_JAVA

#pragma mark -
#pragma mark Init and dealloc

-(id)initWithAccessibleContext: (Reference < XAccessibleContext >) rxAccessibleContext {
    self = [ super init ];
    if ( self != nil ) {
        [ self setDefaults: rxAccessibleContext ];
    }
    return self;
}

-(void) setDefaults: (Reference < XAccessibleContext >) rxAccessibleContext {
    mpReferenceWrapper = new ReferenceWrapper;
    mActsAsRadioGroup = NO;
    mpReferenceWrapper -> rAccessibleContext = rxAccessibleContext;
    mIsTableCell = NO;
#ifdef USE_JAVA
    mbDisposed = NO;
#endif	// USE_JAVA
    // Querying all supported interfaces
    try {
        // XAccessibleComponent
        mpReferenceWrapper -> rAccessibleComponent = Reference < XAccessibleComponent > ( rxAccessibleContext, UNO_QUERY );
        // XAccessibleExtendedComponent
        mpReferenceWrapper -> rAccessibleExtendedComponent = Reference < XAccessibleExtendedComponent > ( rxAccessibleContext, UNO_QUERY );
        // XAccessibleSelection
        mpReferenceWrapper -> rAccessibleSelection = Reference< XAccessibleSelection > ( rxAccessibleContext, UNO_QUERY );
        // XAccessibleTable
        mpReferenceWrapper -> rAccessibleTable = Reference < XAccessibleTable > ( rxAccessibleContext, UNO_QUERY );
        // XAccessibleText
        mpReferenceWrapper -> rAccessibleText = Reference < XAccessibleText > ( rxAccessibleContext, UNO_QUERY );
        // XAccessibleEditableText
        mpReferenceWrapper -> rAccessibleEditableText = Reference < XAccessibleEditableText > ( rxAccessibleContext, UNO_QUERY );
        // XAccessibleValue
        mpReferenceWrapper -> rAccessibleValue = Reference < XAccessibleValue > ( rxAccessibleContext, UNO_QUERY );
        // XAccessibleAction
        mpReferenceWrapper -> rAccessibleAction = Reference < XAccessibleAction > ( rxAccessibleContext, UNO_QUERY );
        // XAccessibleTextAttributes
        mpReferenceWrapper -> rAccessibleTextAttributes = Reference < XAccessibleTextAttributes > ( rxAccessibleContext, UNO_QUERY );
        // XAccessibleMultiLineText
        mpReferenceWrapper -> rAccessibleMultiLineText = Reference < XAccessibleMultiLineText > ( rxAccessibleContext, UNO_QUERY );
        // XAccessibleTextMarkup
        mpReferenceWrapper -> rAccessibleTextMarkup = Reference < XAccessibleTextMarkup > ( rxAccessibleContext, UNO_QUERY );
        // XAccessibleEventBroadcaster
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
            Reference< XAccessibleEventBroadcaster > xBroadcaster(rxAccessibleContext, UNO_QUERY);
            if( xBroadcaster.is() ) {
                /*
                 * We intentionally do not hold a reference to the event listener in the wrapper object,
                 * but let the listener control the life cycle of the wrapper instead ..
                 */
                xBroadcaster->addAccessibleEventListener( new AquaA11yEventListener( self, rxAccessibleContext -> getAccessibleRole() ) );
            }
        }
        // TABLE_CELL
        if ( rxAccessibleContext -> getAccessibleRole() == AccessibleRole::TABLE_CELL ) {
            mIsTableCell = YES;
        }
    } catch ( const Exception ) {
    }
}

-(void)dealloc {
    if ( mpReferenceWrapper != nil ) {
#ifdef USE_JAVA
		JavaSalEvent *pUserEvent = new JavaSalEvent( SALEVENT_DELETEREFWRAPPER, NULL, mpReferenceWrapper );
		JavaSalEventQueue::postCachedEvent( pUserEvent );
		pUserEvent->release();
#else	// USE_JAVA
        delete mpReferenceWrapper;
#endif	// USE_JAVA
    }
    [ super dealloc ];
}

#ifdef USE_JAVA

-(void)disposing {
    mbDisposed = YES;
}

-(BOOL)isDisposed {
    return mbDisposed;
}

#endif	// USE_JAVA

#pragma mark -
#pragma mark Utility Section

// generates selectors for attribute name AXAttributeNameHere
// (getter without parameter) attributeNameHereAttribute
// (getter with parameter)    attributeNameHereAttributeForParameter:
// (setter)                   setAttributeNameHereAttributeForElement:to:
-(SEL)selectorForAttribute:(NSString *)attribute asGetter:(BOOL)asGetter withGetterParameter:(BOOL)withGetterParameter {
    SEL selector = nil;
    NSAutoreleasePool * pool = [ [ NSAutoreleasePool alloc ] init ];
    @try {
        // step 1: create method name from attribute name
#ifdef USE_JAVA
        NSMutableString * methodName = [ NSMutableString stringWithCapacity: 100 ];
#else	// USE_JAVA
        NSMutableString * methodName = [ NSMutableString string ];
#endif	// USE_JAVA
        if ( ! asGetter ) {
            [ methodName appendString: @"set" ];
        }
        NSRange aRange = { 2, 1 };
        NSString * firstChar = [ attribute substringWithRange: aRange ]; // drop leading "AX" and get first char
        if ( asGetter ) {
            [ methodName appendString: [ firstChar lowercaseString ] ];
        } else {
            [ methodName appendString: firstChar ];
        }
        [ methodName appendString: [ attribute substringFromIndex: 3 ] ]; // append rest of attribute name
        // append rest of method name
        [ methodName appendString: @"Attribute" ];
        if ( ! asGetter ) {
            [ methodName appendString: @"ForElement:to:" ];
        } else if ( asGetter && withGetterParameter ) {
            [ methodName appendString: @"ForParameter:" ];
        }
        // step 2: create selector
        selector = NSSelectorFromString ( methodName );
    } @catch ( id exception ) {
        selector = nil;
    }
    [ pool release ];
    return selector;
}

-(Reference < XAccessible >)getFirstRadioButtonInGroup {
    Reference < XAccessibleRelationSet > rxAccessibleRelationSet = [ self accessibleContext ] -> getAccessibleRelationSet();
    if( rxAccessibleRelationSet.is() )
    {
        AccessibleRelation relationMemberOf = rxAccessibleRelationSet -> getRelationByType ( AccessibleRelationType::MEMBER_OF );
        if ( relationMemberOf.RelationType == AccessibleRelationType::MEMBER_OF && relationMemberOf.TargetSet.hasElements() )
            return Reference < XAccessible > ( relationMemberOf.TargetSet[0], UNO_QUERY );
    }
    return Reference < XAccessible > ();
}

-(BOOL)isFirstRadioButtonInGroup {
    Reference < XAccessible > rFirstMateAccessible = [ self getFirstRadioButtonInGroup ];
    if ( rFirstMateAccessible.is() && rFirstMateAccessible -> getAccessibleContext().get() == [ self accessibleContext ] ) {
        return YES;
    }
    return NO;
}

#pragma mark -
#pragma mark Attribute Value Getters
// ( called via Reflection by accessibilityAttributeValue )

/*
    Radiobutton grouping is done differently in NSAccessibility and the UNO-API. In UNO related radio buttons share an entry in their
    RelationSet. In NSAccessibility the relationship is axpressed through the hierarchy. A AXRadioGroup contains two or more AXRadioButton
    objects. Since this group is not available in the UNO hierarchy, an extra wrapper is used for it. This wrapper shares almost all
    attributes with the first radio button of the group, except for the role, subrole, role description, parent and children attributes.
    So in this five methods there is a special treatment for radio buttons and groups.
*/

-(id)roleAttribute {
    if ( mActsAsRadioGroup ) {
        return NSAccessibilityRadioGroupRole;
    }
    else {
        return [ AquaA11yRoleHelper getNativeRoleFrom: [ self accessibleContext ] ];
    }
}

-(id)subroleAttribute {
    if ( mActsAsRadioGroup ) {
        return @"";
    } else {
        NSString * subRole = [ AquaA11yRoleHelper getNativeSubroleFrom: [ self accessibleContext ] -> getAccessibleRole() ];
        if ( ! [ subRole isEqualToString: @"" ] ) {
            return subRole;
        } else {
#ifdef USE_JAVA
            if ( [ super respondsToSelector:@selector(accessibilityAttributeValue:) ] )
#else	// USE_JAVA
            [ subRole release ];
#endif	// USE_JAVA
            return [ super accessibilityAttributeValue: NSAccessibilitySubroleAttribute ];
#ifdef USE_JAVA
            else
                return @"";
#endif	// USE_JAVA
        }
    }
}

-(id)titleAttribute {
#ifdef USE_JAVA
    XAccessibleContext *pAccessibleContext = [ self accessibleContext ];
    if ( pAccessibleContext )
        return [ CreateNSString ( pAccessibleContext -> getAccessibleName() ) autorelease ];
    else
        return [ NSString string ];
#else	// USE_JAVA
    return CreateNSString ( [ self accessibleContext ] -> getAccessibleName() );
#endif	// USE_JAVA
}

-(id)descriptionAttribute {
    if ( [ self accessibleContext ] -> getAccessibleRole() == AccessibleRole::COMBO_BOX ) {
        return [ self titleAttribute ];
    } else if ( [ self accessibleExtendedComponent ] != nil ) {
        return [ AquaA11yComponentWrapper descriptionAttributeForElement: self ];
    } else {
#ifdef USE_JAVA
        return [ CreateNSString ( [ self accessibleContext ] -> getAccessibleDescription() ) autorelease ];
#else	// USE_JAVA
        return CreateNSString ( [ self accessibleContext ] -> getAccessibleDescription() );
#endif	// USE_JAVA
    }
}

-(id)enabledAttribute {
#ifdef USE_JAVA
    XAccessibleContext *pAccessibleContext = [ self accessibleContext ];
    if ( pAccessibleContext && pAccessibleContext -> getAccessibleStateSet().is() ) {
        return [ NSNumber numberWithBool: pAccessibleContext -> getAccessibleStateSet() -> contains ( AccessibleStateType::ENABLED ) ];
#else	// USE_JAVA
    if ( [ self accessibleContext ] -> getAccessibleStateSet().is() ) {
        return [ NSNumber numberWithBool: [ self accessibleContext ] -> getAccessibleStateSet() -> contains ( AccessibleStateType::ENABLED ) ];
#endif	// USE_JAVA
    } else {
        return nil;
    }
}

-(id)focusedAttribute {
    if ( [ self accessibleContext ] -> getAccessibleRole() == AccessibleRole::COMBO_BOX ) {
        id isFocused = nil;
        Reference < XAccessible > rxParent = [ self accessibleContext ] -> getAccessibleParent();
        if ( rxParent.is() ) {
            Reference < XAccessibleContext > rxContext = rxParent -> getAccessibleContext();
            if ( rxContext.is() && rxContext -> getAccessibleStateSet().is() ) {
                isFocused = [ NSNumber numberWithBool: rxContext -> getAccessibleStateSet() -> contains ( AccessibleStateType::FOCUSED ) ];
            }
        }
        return isFocused;
    } else if ( [ self accessibleContext ] -> getAccessibleStateSet().is() ) {
        return [ NSNumber numberWithBool: [ self accessibleContext ] -> getAccessibleStateSet() -> contains ( AccessibleStateType::FOCUSED ) ];
    } else {
        return nil;
    }
}

-(id)parentAttribute {
    if ( [ self accessibleContext ] -> getAccessibleRole() == AccessibleRole::RADIO_BUTTON && ! mActsAsRadioGroup ) {
        Reference < XAccessible > rxAccessible = [ self getFirstRadioButtonInGroup ];
        if ( rxAccessible.is() && rxAccessible -> getAccessibleContext().is() ) {
            Reference < XAccessibleContext > rxAccessibleContext = rxAccessible -> getAccessibleContext();
            id parent_wrapper = [ AquaA11yFactory wrapperForAccessibleContext: rxAccessibleContext createIfNotExists: YES asRadioGroup: YES ];
#ifndef USE_JAVA
            [ parent_wrapper autorelease ];
#endif	// !USE_JAVA
            return NSAccessibilityUnignoredAncestor( parent_wrapper );
        }
        return nil;
    }
    try {
        Reference< XAccessible > xParent( [ self accessibleContext ] -> getAccessibleParent() );
        if ( xParent.is() ) {
            Reference< XAccessibleContext > xContext( xParent -> getAccessibleContext() );
            if ( xContext.is() ) {
                id parent_wrapper = [ AquaA11yFactory wrapperForAccessibleContext: xContext ];
#ifndef USE_JAVA
                [ parent_wrapper autorelease ];
#endif	// !USE_JAVA
                return NSAccessibilityUnignoredAncestor( parent_wrapper );
            }
        }
    } catch (const Exception&) {
    }

    OSL_ASSERT( false );
    return nil;
}

-(id)childrenAttribute {
    if ( mActsAsRadioGroup ) {
#ifdef USE_JAVA
        NSMutableArray * children = nil;
#else	// USE_JAVA
        NSMutableArray * children = [ [ NSMutableArray alloc ] init ];
#endif	// USE_JAVA
        Reference < XAccessibleRelationSet > rxAccessibleRelationSet = [ self accessibleContext ] -> getAccessibleRelationSet();
        AccessibleRelation relationMemberOf = rxAccessibleRelationSet -> getRelationByType ( AccessibleRelationType::MEMBER_OF );
        if ( relationMemberOf.RelationType == AccessibleRelationType::MEMBER_OF && relationMemberOf.TargetSet.hasElements() ) {
#ifdef USE_JAVA
            children = [ NSMutableArray arrayWithCapacity: relationMemberOf.TargetSet.getLength() ];
#endif	// USE_JAVA
            for ( int index = 0; index < relationMemberOf.TargetSet.getLength(); index++ ) {
                Reference < XAccessible > rMateAccessible = Reference < XAccessible > ( relationMemberOf.TargetSet[index], UNO_QUERY );
                if ( rMateAccessible.is() ) {
                    Reference< XAccessibleContext > rMateAccessibleContext( rMateAccessible -> getAccessibleContext() );
                    if ( rMateAccessibleContext.is() ) {
                        id wrapper = [ AquaA11yFactory wrapperForAccessibleContext: rMateAccessibleContext ];
                        [ children addObject: wrapper ];
#ifndef USE_JAVA
                        [ wrapper release ];
#endif	// !USE_JAVA
                    }
                }
            }
        }
        return children;
    } else if ( [ self accessibleTable ] != nil )
    {
        AquaA11yTableWrapper* pTable = [self isKindOfClass: [AquaA11yTableWrapper class]] ? (AquaA11yTableWrapper*)self : nil;
        return [ AquaA11yTableWrapper childrenAttributeForElement: pTable ];
    } else {
        try {
#ifndef USE_JAVA
            NSMutableArray * children = [ [ NSMutableArray alloc ] init ];
#endif	// !USE_JAVA
            Reference< XAccessibleContext > xContext( [ self accessibleContext ] );

            sal_Int32 cnt = xContext -> getAccessibleChildCount();
#ifdef USE_JAVA
            // Fix issue #11 by limiting the maximum number of child views that
            // can be attached to the window
            if ( cnt > AQUA11Y_MAX_CHILD_COUNT )
                cnt = AQUA11Y_MAX_CHILD_COUNT;

            NSMutableArray * children = [ NSMutableArray arrayWithCapacity: cnt ];
#endif	// USE_JAVA
            for ( sal_Int32 i = 0; i < cnt; i++ ) {
                Reference< XAccessible > xChild( xContext -> getAccessibleChild( i ) );
                if( xChild.is() ) {
                    Reference< XAccessibleContext > xChildContext( xChild -> getAccessibleContext() );
                    // the menubar is already accessible (including Apple- and Application-Menu) through NSApplication => omit it here
                    if ( xChildContext.is() && AccessibleRole::MENU_BAR != xChildContext -> getAccessibleRole() ) {
                        id wrapper = [ AquaA11yFactory wrapperForAccessibleContext: xChildContext ];
                        [ children addObject: wrapper ];
#ifndef USE_JAVA
                        [ wrapper release ];
#endif	// !USE_JAVA
                    }
                }
            }

            // if not already acting as RadioGroup now is the time to replace RadioButtons with RadioGroups and remove RadioButtons
            if ( ! mActsAsRadioGroup ) {
                NSEnumerator * enumerator = [ children objectEnumerator ];
                AquaA11yWrapper * element;
                while ( ( element = ( (AquaA11yWrapper *) [ enumerator nextObject ] ) ) ) {
                    if ( [ element accessibleContext ] -> getAccessibleRole() == AccessibleRole::RADIO_BUTTON ) {
                        if ( [ element isFirstRadioButtonInGroup ] ) {
                            id wrapper = [ AquaA11yFactory wrapperForAccessibleContext: [ element accessibleContext ] createIfNotExists: YES asRadioGroup: YES ];
                            [ children replaceObjectAtIndex: [ children indexOfObjectIdenticalTo: element ] withObject: wrapper ];
                        }
                        [ children removeObject: element ];
                    }
                }
            }

#ifndef USE_JAVA
            [ children autorelease ];
#endif	// !USE_JAVA
            return NSAccessibilityUnignoredChildren( children );
        } catch (const Exception &e) {
            // TODO: Log
            return nil;
        }
    }
}

-(id)windowAttribute {
    // go upstairs until reaching the broken connection
    AquaA11yWrapper * aWrapper = self;
    int loops = 0;
    while ( [ aWrapper accessibleContext ] -> getAccessibleParent().is() ) {
        AquaA11yWrapper *aTentativeParentWrapper = [ AquaA11yFactory wrapperForAccessibleContext: [ aWrapper accessibleContext ] -> getAccessibleParent() -> getAccessibleContext() ];
        // Quick-and-dirty fix for infinite loop after fixing crash in
        // fdo#47275
        if ( aTentativeParentWrapper == aWrapper )
            break;
        // Even dirtier fix for infinite loop in fdo#55156
        if ( loops++ == 100 )
            break;
        aWrapper = aTentativeParentWrapper;
#ifndef USE_JAVA
        [ aWrapper autorelease ];
#endif	// !USE_JAVA
    }
    // get associated NSWindow
    NSWindow* theWindow = [ aWrapper windowForParent ];
    return theWindow;
}

-(id)topLevelUIElementAttribute {
    return [ self windowAttribute ];
}

-(id)sizeAttribute {
    if ( [ self accessibleComponent ] != nil ) {
        return [ AquaA11yComponentWrapper sizeAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)positionAttribute {
    if ( [ self accessibleComponent ] != nil ) {
        return [ AquaA11yComponentWrapper positionAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)helpAttribute {
#ifdef USE_JAVA
    return [ CreateNSString ( [ self accessibleContext ] -> getAccessibleDescription() ) autorelease ];
#else	// USE_JAVA
    return CreateNSString ( [ self accessibleContext ] -> getAccessibleDescription() );
#endif	// USE_JAVA
}

-(id)roleDescriptionAttribute {
    if ( mActsAsRadioGroup ) {
        return [ AquaA11yRoleHelper getRoleDescriptionFrom: NSAccessibilityRadioGroupRole with: @"" ];
	} else if( [ self accessibleContext ] -> getAccessibleRole() == AccessibleRole::RADIO_BUTTON ) {
		// FIXME: VO should read this because of hierarchy, this is just a workaround
		// get parent and its children
		AquaA11yWrapper * parent = [ self parentAttribute ];
		NSArray * children = [ parent childrenAttribute ];
		// find index of self
		int index = 1;
		NSEnumerator * enumerator = [ children objectEnumerator ];
		AquaA11yWrapper * child = nil;
		while ( ( child = [ enumerator nextObject ] ) ) {
			if ( self == child ) {
				break;
			}
			index++;
		}
		// build string
		NSNumber * nIndex = [ NSNumber numberWithInt: index ];
		NSNumber * nGroupsize = [ NSNumber numberWithInt: [ children count ] ];
#ifdef USE_JAVA
		NSMutableString * value = [ NSMutableString stringWithCapacity: 100 ];
#else	// USE_JAVA
		NSMutableString * value = [ [ NSMutableString alloc ] init ];
#endif	// USE_JAVA
		[ value appendString: @"radio button " ];
		[ value appendString: [ nIndex stringValue ] ];
		[ value appendString: @" of " ];
		[ value appendString: [ nGroupsize stringValue ] ];
		// clean up and return string
#ifndef USE_JAVA
		[ nIndex release ];
		[ nGroupsize release ];
		[ children release ];
#endif	// !USE_JAVA
		return value;
    } else {
        return [ AquaA11yRoleHelper getRoleDescriptionFrom:
                [ AquaA11yRoleHelper getNativeRoleFrom: [ self accessibleContext ] ]
                with: [ AquaA11yRoleHelper getNativeSubroleFrom: [ self accessibleContext ] -> getAccessibleRole() ] ];
    }
}

-(id)valueAttribute {
    if ( [ [ self roleAttribute ] isEqualToString: NSAccessibilityMenuItemRole ] ) {
        return nil;
    } else if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper valueAttributeForElement: self ];
    } else if ( [ self accessibleValue ] != nil ) {
        return [ AquaA11yValueWrapper valueAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)minValueAttribute {
    if ( [ self accessibleValue ] != nil ) {
        return [ AquaA11yValueWrapper minValueAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)maxValueAttribute {
    if ( [ self accessibleValue ] != nil ) {
        return [ AquaA11yValueWrapper maxValueAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)contentsAttribute {
    return [ self childrenAttribute ];
}

-(id)selectedChildrenAttribute {
    return [ AquaA11ySelectionWrapper selectedChildrenAttributeForElement: self ];
}

-(id)numberOfCharactersAttribute {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper numberOfCharactersAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)selectedTextAttribute {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper selectedTextAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)selectedTextRangeAttribute {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper selectedTextRangeAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)visibleCharacterRangeAttribute {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper visibleCharacterRangeAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)tabsAttribute {
    return self; // TODO ???
}

-(id)sharedTextUIElementsAttribute {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper sharedTextUIElementsAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)sharedCharacterRangeAttribute {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper sharedCharacterRangeAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)expandedAttribute {
    return [ NSNumber numberWithBool: [ self accessibleContext ] -> getAccessibleStateSet() -> contains ( AccessibleStateType::EXPANDED ) ];
}

-(id)selectedAttribute {
    return [ NSNumber numberWithBool: [ self accessibleContext ] -> getAccessibleStateSet() -> contains ( AccessibleStateType::SELECTED ) ];
}

-(id)stringForRangeAttributeForParameter:(id)range {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper stringForRangeAttributeForElement: self forParameter: range ];
    } else {
        return nil;
    }
}

-(id)attributedStringForRangeAttributeForParameter:(id)range {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper attributedStringForRangeAttributeForElement: self forParameter: range ];
    } else {
        return nil;
    }
}

-(id)rangeForIndexAttributeForParameter:(id)index {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper rangeForIndexAttributeForElement: self forParameter: index ];
    } else {
        return nil;
    }
}

-(id)rangeForPositionAttributeForParameter:(id)point {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper rangeForPositionAttributeForElement: self forParameter: point ];
    } else {
        return nil;
    }
}

-(id)boundsForRangeAttributeForParameter:(id)range {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper boundsForRangeAttributeForElement: self forParameter: range ];
    } else {
        return nil;
    }
}

-(id)styleRangeForIndexAttributeForParameter:(id)index {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper styleRangeForIndexAttributeForElement: self forParameter: index ];
    } else {
        return nil;
    }
}

-(id)rTFForRangeAttributeForParameter:(id)range {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper rTFForRangeAttributeForElement: self forParameter: range ];
    } else {
        return nil;
    }
}

-(id)orientationAttribute {
#ifdef USE_JAVA
    NSNumber * orientation = nil;
#else	// USE_JAVA
    NSString * orientation = nil;
#endif	// USE_JAVA
    Reference < XAccessibleStateSet > stateSet = [ self accessibleContext ] -> getAccessibleStateSet();
    if ( stateSet -> contains ( AccessibleStateType::HORIZONTAL ) ) {
#ifdef USE_JAVA
        orientation = [ NSNumber numberWithInteger: NSAccessibilityOrientationHorizontal ];
#else	// USE_JAVA
        orientation = NSAccessibilityHorizontalOrientationValue;
#endif	// USE_JAVA
    } else if ( stateSet -> contains ( AccessibleStateType::VERTICAL ) ) {
#ifdef USE_JAVA
        orientation = [ NSNumber numberWithInteger: NSAccessibilityOrientationVertical ];
    } else {
        orientation = [ NSNumber numberWithInteger: NSAccessibilityOrientationUnknown ];
#else	// USE_JAVA
        orientation = NSAccessibilityVerticalOrientationValue;
#endif	// USE_JAVA
    }
    return orientation;
}

-(id)titleUIElementAttribute {
    if ( [ self accessibleContext ] -> getAccessibleRelationSet().is() ) {
        NSString * title = [ self titleAttribute ];
        id titleElement = nil;
        if ( [ title length ] == 0 ) {
            AccessibleRelation relationLabeledBy = [ self accessibleContext ] -> getAccessibleRelationSet() -> getRelationByType ( AccessibleRelationType::LABELED_BY );
            if ( relationLabeledBy.RelationType == AccessibleRelationType::LABELED_BY && relationLabeledBy.TargetSet.hasElements()  ) {
                Reference < XAccessible > rxAccessible ( relationLabeledBy.TargetSet[0], UNO_QUERY );
                titleElement = [ AquaA11yFactory wrapperForAccessibleContext: rxAccessible -> getAccessibleContext() ];
            }
        }
#ifndef USE_JAVA
        if ( title != nil ) {
            [ title release ];
        }
#endif	// !USE_JAVA
        return titleElement;
    } else {
        return nil;
    }
}

-(id)servesAsTitleForUIElementsAttribute {
    if ( [ self accessibleContext ] -> getAccessibleRelationSet().is() ) {
        id titleForElement = nil;
        AccessibleRelation relationLabelFor = [ self accessibleContext ] -> getAccessibleRelationSet() -> getRelationByType ( AccessibleRelationType::LABEL_FOR );
        if ( relationLabelFor.RelationType == AccessibleRelationType::LABEL_FOR && relationLabelFor.TargetSet.hasElements() ) {
            Reference < XAccessible > rxAccessible ( relationLabelFor.TargetSet[0], UNO_QUERY );
            titleForElement = [ AquaA11yFactory wrapperForAccessibleContext: rxAccessible -> getAccessibleContext() ];
        }
        return titleForElement;
    } else {
        return nil;
    }
}

-(id)lineForIndexAttributeForParameter:(id)index {
    if ( [ self accessibleMultiLineText ] != nil ) {
        return [ AquaA11yTextWrapper lineForIndexAttributeForElement: self forParameter: index ];
    } else {
        return nil;
    }
}

-(id)rangeForLineAttributeForParameter:(id)line {
    if ( [ self accessibleMultiLineText ] != nil ) {
        return [ AquaA11yTextWrapper rangeForLineAttributeForElement: self forParameter: line ];
    } else {
        return nil;
    }
}

#pragma mark -
#pragma mark Accessibility Protocol

-(id)accessibilityAttributeValue:(NSString *)attribute {
    SAL_INFO("vcl.a11y", "[" << self << " accessibilityAttributeValue:" << attribute << "]");
    // #i90575# guard NSAccessibility protocol against unwanted access
    if ( isPopupMenuOpen ) {
        return nil;
    }

    id value = nil;
#ifdef USE_JAVA
    if ( !ImplApplicationIsRunning() )
        return nil;
    // Set drag lock if it has not already been set since dispatching native
    // events to windows during an accessibility call can cause crashing
    ACQUIRE_DRAGPRINTLOCK
    if ( [ self isDisposed ] ) {
        RELEASE_DRAGPRINTLOCKIFNEEDED
        return nil;
    }
#endif	// USE_JAVA
    // if we are no longer in the wrapper repository, we have been disposed
    AquaA11yWrapper * theWrapper = [ AquaA11yFactory wrapperForAccessibleContext: [ self accessibleContext ] createIfNotExists: NO ];
    if ( theWrapper != nil || mIsTableCell ) {
        try {
            SEL methodSelector = [ self selectorForAttribute: attribute asGetter: YES withGetterParameter: NO ];
            if ( [ self respondsToSelector: methodSelector ] ) {
                value = [ self performSelector: methodSelector ];
            }
        } catch ( const DisposedException & e ) {
            mIsTableCell = NO; // just to be sure
            [ AquaA11yFactory removeFromWrapperRepositoryFor: [ self accessibleContext ] ];
#ifdef USE_JAVA
            RELEASE_DRAGPRINTLOCKIFNEEDED
#endif	// USE_JAVA
            return nil;
        } catch ( const Exception & e ) {
            // empty
        }
    }
#ifdef USE_JAVA
    RELEASE_DRAGPRINTLOCK
#else	// USE_JAVA
    if ( theWrapper != nil ) {
        [ theWrapper release ]; // the above called method calls retain on the returned Wrapper
    }
#endif	// USE_JAVA
    return value;
}

-(BOOL)accessibilityIsIgnored {
    SAL_INFO("vcl.a11y", "[" << self << " accessibilityIsIgnored]");
    // #i90575# guard NSAccessibility protocol against unwanted access
    if ( isPopupMenuOpen ) {
        return NO;
    }
    BOOL ignored = NO;
#ifdef USE_JAVA
    if ( !ImplApplicationIsRunning() )
        return ignored;
    // Set drag lock if it has not already been set since dispatching native
    // events to windows during an accessibility call can cause crashing
    ACQUIRE_DRAGPRINTLOCK
    if ( [ self isDisposed ] ) {
        RELEASE_DRAGPRINTLOCKIFNEEDED
        return ignored;
    }
    XAccessibleContext *pAccessibleContext = [ self accessibleContext ];
    if ( pAccessibleContext ) {
        sal_Int16 nRole = pAccessibleContext -> getAccessibleRole();
#else	// USE_JAVA
    sal_Int16 nRole = [ self accessibleContext ] -> getAccessibleRole();
#endif	// USE_JAVA
    switch ( nRole ) {
        //case AccessibleRole::PANEL:
        case AccessibleRole::FRAME:
        case AccessibleRole::ROOT_PANE:
        case AccessibleRole::SEPARATOR:
        case AccessibleRole::FILLER:
        case AccessibleRole::DIALOG:
            ignored = YES;
            break;
        default:
#ifdef USE_JAVA
            ignored = ! ( pAccessibleContext -> getAccessibleStateSet() -> contains ( AccessibleStateType::VISIBLE ) );
#else	// USE_JAVA
            ignored = ! ( [ self accessibleContext ] -> getAccessibleStateSet() -> contains ( AccessibleStateType::VISIBLE ) );
#endif	// USE_JAVA
            break;
    }
#ifdef USE_JAVA
    }
    RELEASE_DRAGPRINTLOCK
#endif	// USE_JAVA
    return ignored; // TODO: to be completed
}

-(NSArray *)accessibilityAttributeNames {
    SAL_INFO("vcl.a11y", "[" << self << " accessibilityAttributeNames]");
    // #i90575# guard NSAccessibility protocol against unwanted access
    if ( isPopupMenuOpen ) {
        return nil;
    }
#ifdef USE_JAVA
    if ( !ImplApplicationIsRunning() )
        return nil;
    // Set drag lock if it has not already been set since dispatching native
    // events to windows during an accessibility call can cause crashing
    ACQUIRE_DRAGPRINTLOCK
    if ( [ self isDisposed ] ) {
        RELEASE_DRAGPRINTLOCKIFNEEDED
        return nil;
    }
#endif	// USE_JAVA
    NSString * nativeSubrole = nil;
    NSString * title = nil;
    NSMutableArray * attributeNames = nil;
    sal_Int32 nAccessibleChildren = 0;
    try {
        // Default Attributes
        attributeNames = [ NSMutableArray arrayWithObjects:
            NSAccessibilityRoleAttribute,
            NSAccessibilityDescriptionAttribute,
            NSAccessibilityParentAttribute,
            NSAccessibilityWindowAttribute,
            NSAccessibilityHelpAttribute,
            NSAccessibilityTopLevelUIElementAttribute,
            NSAccessibilityRoleDescriptionAttribute,
            nil ];
#ifdef USE_JAVA
        XAccessibleContext *pAccessibleContext = [ self accessibleContext ];
        if ( pAccessibleContext )
            nativeSubrole = (NSString *) [ AquaA11yRoleHelper getNativeSubroleFrom: pAccessibleContext -> getAccessibleRole() ];
#else	// USE_JAVA
        nativeSubrole = (NSString *) [ AquaA11yRoleHelper getNativeSubroleFrom: [ self accessibleContext ] -> getAccessibleRole() ];
#endif	// USE_JAVA
        title = (NSString *) [ self titleAttribute ];
#ifdef USE_JAVA
        Reference < XAccessibleRelationSet > rxRelationSet;
        if ( pAccessibleContext )
            rxRelationSet = pAccessibleContext -> getAccessibleRelationSet();
#else	// USE_JAVA
        Reference < XAccessibleRelationSet > rxRelationSet = [ self accessibleContext ] -> getAccessibleRelationSet();
#endif	// USE_JAVA
        // Special Attributes depending on attribute values
        if ( nativeSubrole != nil && ! [ nativeSubrole isEqualToString: @"" ] ) {
            [ attributeNames addObject: NSAccessibilitySubroleAttribute ];
        }
        try
        {
#ifdef USE_JAVA
            if ( pAccessibleContext )
                nAccessibleChildren = pAccessibleContext -> getAccessibleChildCount();
#else	// USE_JAVA
            nAccessibleChildren = [ self accessibleContext ] -> getAccessibleChildCount();
#endif	// USE_JAVA
            if (  nAccessibleChildren > 0 ) {
                [ attributeNames addObject: NSAccessibilityChildrenAttribute ];
        }
        }
        catch( DisposedException& ) {}
        catch( RuntimeException& ) {}

        if ( title != nil && ! [ title isEqualToString: @"" ] ) {
            [ attributeNames addObject: NSAccessibilityTitleAttribute ];
        }
        if ( [ title length ] == 0 && rxRelationSet.is() && rxRelationSet -> containsRelation ( AccessibleRelationType::LABELED_BY ) ) {
            [ attributeNames addObject: NSAccessibilityTitleUIElementAttribute ];
        }
        if ( rxRelationSet.is() && rxRelationSet -> containsRelation ( AccessibleRelationType::LABEL_FOR ) ) {
            [ attributeNames addObject: NSAccessibilityServesAsTitleForUIElementsAttribute ];
        }
        // Special Attributes depending on interface
#ifdef USE_JAVA
        if ( pAccessibleContext && pAccessibleContext -> getAccessibleRole() == AccessibleRole::TABLE )
#else	// USE_JAVA
        if( [self accessibleContext ] -> getAccessibleRole() == AccessibleRole::TABLE )
#endif	// USE_JAVA
            [AquaA11yTableWrapper addAttributeNamesTo: attributeNames object: self];

        if ( [ self accessibleText ] != nil ) {
            [ AquaA11yTextWrapper addAttributeNamesTo: attributeNames ];
        }
        if ( [ self accessibleComponent ] != nil ) {
            [ AquaA11yComponentWrapper addAttributeNamesTo: attributeNames ];
        }
        if ( [ self accessibleSelection ] != nil ) {
            [ AquaA11ySelectionWrapper addAttributeNamesTo: attributeNames ];
        }
        if ( [ self accessibleValue ] != nil ) {
            [ AquaA11yValueWrapper addAttributeNamesTo: attributeNames ];
        }
#ifdef USE_JAVA
        RELEASE_DRAGPRINTLOCKIFNEEDED
#else	// USE_JAVA
        [ nativeSubrole release ];
        [ title release ];
#endif	// USE_JAVA
        return attributeNames;
    } catch ( DisposedException & e ) { // Object is no longer available
#ifndef USE_JAVA
        if ( nativeSubrole != nil ) {
            [ nativeSubrole release ];
        }
        if ( title != nil ) {
            [ title release ];
        }
        if ( attributeNames != nil ) {
            [ attributeNames release ];
        }
#endif	// !USE_JAVA
        [ AquaA11yFactory removeFromWrapperRepositoryFor: [ self accessibleContext ] ];
#ifdef USE_JAVA
        RELEASE_DRAGPRINTLOCKIFNEEDED
        return [ NSArray array ];
#else	// USE_JAVA
        return [ [ NSArray alloc ] init ];
#endif	// USE_JAVA
    }
#ifdef USE_JAVA
    RELEASE_DRAGPRINTLOCK
    return [ NSArray array ];
#endif	// USE_JAVA
}

-(BOOL)accessibilityIsAttributeSettable:(NSString *)attribute {
    SAL_INFO("vcl.a11y", "[" << self << " accessibilityAttributeIsSettable:" << attribute << "]");
    BOOL isSettable = NO;
#ifdef USE_JAVA
    if ( !ImplApplicationIsRunning() )
        return isSettable;
    // Set drag lock if it has not already been set since dispatching native
    // events to windows during an accessibility call can cause crashing
    ACQUIRE_DRAGPRINTLOCK
    if ( [ self isDisposed ] ) {
        RELEASE_DRAGPRINTLOCKIFNEEDED
        return isSettable;
    }
#endif	// USE_JAVA
    if ( [ self accessibleText ] != nil ) {
        isSettable = [ AquaA11yTextWrapper isAttributeSettable: attribute forElement: self ];
    }
    if ( ! isSettable && [ self accessibleComponent ] != nil ) {
        isSettable = [ AquaA11yComponentWrapper isAttributeSettable: attribute forElement: self ];
    }
    if ( ! isSettable && [ self accessibleSelection ] != nil ) {
        isSettable = [ AquaA11ySelectionWrapper isAttributeSettable: attribute forElement: self ];
    }
    if ( ! isSettable && [ self accessibleValue ] != nil ) {
        isSettable = [ AquaA11yValueWrapper isAttributeSettable: attribute forElement: self ];
    }
#ifdef USE_JAVA
    RELEASE_DRAGPRINTLOCK
#endif	// USE_JAVA
    return isSettable; // TODO: to be completed
}

-(NSArray *)accessibilityParameterizedAttributeNames {
    SAL_INFO("vcl.a11y", "[" << self << " accessibilityParameterizedAttributeNames]");
#ifdef USE_JAVA
    NSMutableArray * attributeNames = [ NSMutableArray arrayWithCapacity: 10 ];
    if ( !ImplApplicationIsRunning() )
        return attributeNames;
    // Set drag lock if it has not already been set since dispatching native
    // events to windows during an accessibility call can cause crashing
    ACQUIRE_DRAGPRINTLOCK
    if ( [ self isDisposed ] ) {
        RELEASE_DRAGPRINTLOCKIFNEEDED
        return attributeNames;
    }
#else	// USE_JAVA
    NSMutableArray * attributeNames = [ [ NSMutableArray alloc ] init ];
#endif	// USE_JAVA
    // Special Attributes depending on interface
    if ( [ self accessibleText ] != nil ) {
        [ AquaA11yTextWrapper addParameterizedAttributeNamesTo: attributeNames ];
    }
#ifdef USE_JAVA
    RELEASE_DRAGPRINTLOCK
#endif	// USE_JAVA
    return attributeNames; // TODO: to be completed
}

-(id)accessibilityAttributeValue:(NSString *)attribute forParameter:(id)parameter {
    SAL_INFO("vcl.a11y", "[" << self << " accessibilityAttributeValue:" << attribute << " forParameter:" << ((NSObject*)parameter) << "]");
#ifdef USE_JAVA
    if ( !ImplApplicationIsRunning() )
        return nil;
    // Set drag lock if it has not already been set since dispatching native
    // events to windows during an accessibility call can cause crashing
    ACQUIRE_DRAGPRINTLOCK
    if ( [ self isDisposed ] ) {
        RELEASE_DRAGPRINTLOCKIFNEEDED
        return nil;
    }
#endif	// USE_JAVA
    SEL methodSelector = [ self selectorForAttribute: attribute asGetter: YES withGetterParameter: YES ];
    if ( [ self respondsToSelector: methodSelector ] ) {
#ifdef USE_JAVA
        id value = [ self performSelector: methodSelector withObject: parameter ];
        RELEASE_DRAGPRINTLOCKIFNEEDED
        return value;
#else	// USE_JAVA
        return [ self performSelector: methodSelector withObject: parameter ];
#endif	// USE_JAVA
    }
#ifdef USE_JAVA
    RELEASE_DRAGPRINTLOCK
#endif	// USE_JAVA
    return nil; // TODO: to be completed
}

-(BOOL)accessibilitySetOverrideValue:(id)value forAttribute:(NSString *)attribute
{
    SAL_INFO("vcl.a11y", "[" << self << " accessibilitySetOverrideValue:" << ((NSObject*)value) << " forAttribute:" << attribute << "]");
    (void)value;
    (void)attribute;
    return NO; // TODO
}

-(void)accessibilitySetValue:(id)value forAttribute:(NSString *)attribute {
    SAL_INFO("vcl.a11y", "[" << self << " accessibilitySetValue:" << ((NSObject*)value) << " forAttribute:" << attribute << "]");
#ifdef USE_JAVA
    if ( !ImplApplicationIsRunning() )
        return;
    // Set drag lock if it has not already been set since dispatching native
    // events to windows during an accessibility call can cause crashing
    ACQUIRE_DRAGPRINTLOCK
    if ( [ self isDisposed ] ) {
        RELEASE_DRAGPRINTLOCKIFNEEDED
        return;
    }
#endif	// USE_JAVA
    SEL methodSelector = [ self selectorForAttribute: attribute asGetter: NO withGetterParameter: NO ];
    if ( [ AquaA11yComponentWrapper respondsToSelector: methodSelector ] ) {
        [ AquaA11yComponentWrapper performSelector: methodSelector withObject: self withObject: value ];
    }
    if ( [ AquaA11yTextWrapper respondsToSelector: methodSelector ] ) {
        [ AquaA11yTextWrapper performSelector: methodSelector withObject: self withObject: value ];
    }
    if ( [ AquaA11ySelectionWrapper respondsToSelector: methodSelector ] ) {
        [ AquaA11ySelectionWrapper performSelector: methodSelector withObject: self withObject: value ];
    }
    if ( [ AquaA11yValueWrapper respondsToSelector: methodSelector ] ) {
        [ AquaA11yValueWrapper performSelector: methodSelector withObject: self withObject: value ];
    }
#ifdef USE_JAVA
    RELEASE_DRAGPRINTLOCK
#endif	// USE_JAVA
}

-(id)accessibilityFocusedUIElement {
    SAL_INFO("vcl.a11y", "[" << self << " accessibilityFocusedUIElement]");
    // #i90575# guard NSAccessibility protocol against unwanted access
    if ( isPopupMenuOpen ) {
        return nil;
    }

#ifdef USE_JAVA
    if ( !ImplApplicationIsRunning() )
        return nil;
    // Set drag lock if it has not already been set since dispatching native
    // events to windows during an accessibility call can cause crashing
    ACQUIRE_DRAGPRINTLOCK
    if ( [ self isDisposed ] ) {
        RELEASE_DRAGPRINTLOCKIFNEEDED
        return nil;
    }
#endif	// USE_JAVA
    // as this seems to be the first API call on a newly created SalFrameView object,
    // make sure self gets registered in the repository ..
    [ self accessibleContext ];

    AquaA11yWrapper * focusedUIElement = AquaA11yFocusListener::get()->getFocusedUIElement();
//    AquaA11yWrapper * ancestor = focusedUIElement;

      // Make sure the focused object is a descendant of self
//    do  {
//       if( self == ancestor )
#ifdef USE_JAVA
             RELEASE_DRAGPRINTLOCKIFNEEDED
#endif	// USE_JAVA
             return focusedUIElement;

//       ancestor = [ ancestor accessibilityAttributeValue: NSAccessibilityParentAttribute ];
//    }  while( nil != ancestor );
#ifdef USE_JAVA
    RELEASE_DRAGPRINTLOCK
#endif	// USE_JAVA

    return self;
}

-(NSString *)accessibilityActionDescription:(NSString *)action {
    SAL_INFO("vcl.a11y", "[" << self << " accessibilityActionDescription:" << action << "]");
    return NSAccessibilityActionDescription(action);
}

-(AquaA11yWrapper *)actionResponder {
    AquaA11yWrapper * wrapper = nil;
    // get some information
    NSString * role = (NSString *) [ self accessibilityAttributeValue: NSAccessibilityRoleAttribute ];
    id enabledAttr = [ self enabledAttribute ];
    BOOL enabled = [ enabledAttr boolValue ];
    NSView * parent = (NSView *) [ self accessibilityAttributeValue: NSAccessibilityParentAttribute ];
    AquaA11yWrapper * parentAsWrapper = nil;
    if ( [ parent isKindOfClass: [ AquaA11yWrapper class ] ] ) {
        parentAsWrapper = (AquaA11yWrapper *) parent;
    }
#ifdef USE_JAVA
    NSString * parentRole = @"";
    if ( [ parent respondsToSelector:@selector(accessibilityAttributeValue:) ] )
        parentRole = (NSString *) [ parent accessibilityAttributeValue: NSAccessibilityRoleAttribute ];
#else	// USE_JAVA
    NSString * parentRole = (NSString *) [ parent accessibilityAttributeValue: NSAccessibilityRoleAttribute ];
#endif	// USE_JAVA
    // if we are a textarea inside a combobox, then the combobox is the action responder
    if ( enabled
      && [ role isEqualToString: NSAccessibilityTextAreaRole ]
      && [ parentRole isEqualToString: NSAccessibilityComboBoxRole ]
      && parentAsWrapper != nil ) {
        wrapper = parentAsWrapper;
    } else if ( enabled && [ self accessibleAction ] != nil ) {
        wrapper = self ;
    }
#ifndef USE_JAVA
    [ parentRole release ];
    [ enabledAttr release ];
    [ role release ];
#endif	// !USE_JAVA
    return wrapper;
}

#ifdef USE_JAVA
-(void)accessibilityPerformAction:(NSString *)action {
    [ self performAction: action ];
}

-(BOOL)performAction:(NSString *)action {
    BOOL bRet = NO;
#else	// USE_JAVA
-(void)accessibilityPerformAction:(NSString *)action {
#endif	// USE_JAVA
    SAL_INFO("vcl.a11y", "[" << self << " accessibilityPerformAction:" << action << "]");
#ifdef USE_JAVA
    if ( !ImplApplicationIsRunning() )
        return bRet;
    // Set drag lock if it has not already been set since dispatching native
    // events to windows during an accessibility call can cause crashing
    ACQUIRE_DRAGPRINTLOCK
    if ( [ self isDisposed ] ) {
        RELEASE_DRAGPRINTLOCKIFNEEDED
        return bRet;
    }
#endif	// USE_JAVA
    AquaA11yWrapper * actionResponder = [ self actionResponder ];
    if ( actionResponder != nil ) {
#ifdef USE_JAVA
        bRet = YES;
#endif	// USE_JAVA
        [ AquaA11yActionWrapper doAction: action ofElement: actionResponder ];
#ifdef USE_JAVA
    } else {
        bRet = YES;
#endif	// USE_JAVA
    }
#ifdef USE_JAVA
    RELEASE_DRAGPRINTLOCK
    return bRet;
#endif	// USE_JAVA
}

-(NSArray *)accessibilityActionNames {
    SAL_INFO("vcl.a11y", "[" << self << " accessibilityActionNames]");
    NSArray * actionNames = nil;
#ifdef USE_JAVA
    if ( !ImplApplicationIsRunning() )
        return nil;
    // Set drag lock if it has not already been set since dispatching native
    // events to windows during an accessibility call can cause crashing
    ACQUIRE_DRAGPRINTLOCK
    if ( [ self isDisposed ] ) {
        RELEASE_DRAGPRINTLOCKIFNEEDED
        return nil;
    }
#endif	// USE_JAVA
    AquaA11yWrapper * actionResponder = [ self actionResponder ];
    if ( actionResponder != nil ) {
        actionNames = [ AquaA11yActionWrapper actionNamesForElement: actionResponder ];
    } else {
#ifdef USE_JAVA
        actionNames = [ NSArray array ];
#else	// USE_JAVA
        actionNames = [ [ NSArray alloc ] init ];
#endif	// USE_JAVA
    }
#ifdef USE_JAVA
    RELEASE_DRAGPRINTLOCK
#endif	// USE_JAVA
    return actionNames;
}

#pragma mark -
#pragma mark Hit Test

-(BOOL)isViewElement:(NSObject *)viewElement hitByPoint:(NSPoint)point {
    BOOL hit = NO;
    NSAutoreleasePool * pool = [ [ NSAutoreleasePool alloc ] init ];
#ifdef USE_JAVA
    NSValue * position = nil;
    NSValue * size = nil;
    if ( [ viewElement respondsToSelector:@selector(accessibilityAttributeValue:) ] ) {
        position = [ viewElement accessibilityAttributeValue: NSAccessibilityPositionAttribute ];
        size = [ viewElement accessibilityAttributeValue: NSAccessibilitySizeAttribute ];
    }
#else	// USE_JAVA
    NSValue * position = [ viewElement accessibilityAttributeValue: NSAccessibilityPositionAttribute ];
    NSValue * size = [ viewElement accessibilityAttributeValue: NSAccessibilitySizeAttribute ];
#endif	// USE_JAVA
    if ( position != nil && size != nil ) {
        float minX = [ position pointValue ].x;
        float minY = [ position pointValue ].y;
        float maxX = minX + [ size sizeValue ].width;
        float maxY = minY + [ size sizeValue ].height;
        if ( minX < point.x && maxX > point.x && minY < point.y && maxY > point.y ) {
            hit = YES;
        }
    }
    [ pool release ];
    return hit;
}

Reference < XAccessibleContext > hitTestRunner ( com::sun::star::awt::Point point,
                                                 Reference < XAccessibleContext > rxAccessibleContext ) {
    Reference < XAccessibleContext > hitChild;
    Reference < XAccessibleContext > emptyReference;
    try {
        Reference < XAccessibleComponent > rxAccessibleComponent ( rxAccessibleContext, UNO_QUERY );
        if ( rxAccessibleComponent.is() ) {
            com::sun::star::awt::Point location = rxAccessibleComponent -> getLocationOnScreen();
            com::sun::star::awt::Point hitPoint ( point.X - location.X , point.Y - location.Y);
            Reference < XAccessible > rxAccessible = rxAccessibleComponent -> getAccessibleAtPoint ( hitPoint );
            if ( rxAccessible.is() && rxAccessible -> getAccessibleContext().is() &&
                 rxAccessible -> getAccessibleContext() -> getAccessibleChildCount() == 0 ) {
                hitChild = rxAccessible -> getAccessibleContext();
            }
        }

        // iterate the hirerachy looking doing recursive hit testing.
        // apparently necessary as a special treatment for e.g. comboboxes
        if ( !hitChild.is() ) {
            bool bSafeToIterate = true;
            sal_Int32 nCount = rxAccessibleContext -> getAccessibleChildCount();

            if ( nCount < 0 || nCount > SAL_MAX_UINT16 /* slow enough for anyone */ )
                bSafeToIterate = false;
            else { // manages descendants is an horror from the a11y standards guys.
                Reference< XAccessibleStateSet > xStateSet;
                xStateSet = rxAccessibleContext -> getAccessibleStateSet();
                if (xStateSet.is() && xStateSet -> contains(AccessibleStateType::MANAGES_DESCENDANTS ) )
                    bSafeToIterate = false;
            }

            if( bSafeToIterate ) {
                for ( int i = 0; i < rxAccessibleContext -> getAccessibleChildCount(); i++ ) {
                    Reference < XAccessible > rxAccessibleChild = rxAccessibleContext -> getAccessibleChild ( i );
                    if ( rxAccessibleChild.is() && rxAccessibleChild -> getAccessibleContext().is() && rxAccessibleChild -> getAccessibleContext() -> getAccessibleRole() != AccessibleRole::LIST ) {
                        Reference < XAccessibleContext > myHitChild = hitTestRunner ( point, rxAccessibleChild -> getAccessibleContext() );
                        if ( myHitChild.is() ) {
                            hitChild = myHitChild;
                            break;
                        }
                    }
                }
            }
        }
    } catch ( RuntimeException ) {
        return emptyReference;
    }
    return hitChild;
}

-(id)accessibilityHitTest:(NSPoint)point {
    SAL_INFO("vcl.a11y", "[" << self << " accessibilityHitTest:" << point << "]");
    static id wrapper = nil;
#ifdef USE_JAVA
    if ( !ImplApplicationIsRunning() )
        return nil;
    // Set drag lock if it has not already been set since dispatching native
    // events to windows during an accessibility call can cause crashing
    ACQUIRE_DRAGPRINTLOCK
    if ( [ self isDisposed ] ) {
        RELEASE_DRAGPRINTLOCKIFNEEDED
        return nil;
    }
#endif	// USE_JAVA
    if ( nil != wrapper ) {
        [ wrapper release ];
        wrapper = nil;
    }
    Reference < XAccessibleContext > hitChild;
#ifdef USE_JAVA
    NSRect screenRect = JavaSalFrame::GetTotalScreenBounds();
#else	// USE_JAVA
    NSRect screenRect = [ [ NSScreen mainScreen ] frame ];
#endif	// USE_JAVA
#ifdef NO_LIBO_POINT_CASTING_FIX
    com::sun::star::awt::Point hitPoint ( static_cast<long>(point.x) , static_cast<long>(screenRect.size.height - point.y) );
#else	// NO_LIBO_POINT_CASTING_FIX
    com::sun::star::awt::Point hitPoint ( static_cast<sal_Int32>(point.x) , static_cast<sal_Int32>(screenRect.size.height - point.y) );
#endif	// NO_LIBO_POINT_CASTING_FIX
    // check child windows first
    NSWindow * window = (NSWindow *) [ self accessibilityAttributeValue: NSAccessibilityWindowAttribute ];
#ifdef USE_JAVA
    NSArray * childWindows = ( window ? [ window childWindows ] : nil );
    if ( childWindows && [ childWindows count ] > 0 ) {
#else	// USE_JAVA
    NSArray * childWindows = [ window childWindows ];
    if ( [ childWindows count ] > 0 ) {
#endif	// USE_JAVA
        NSWindow * element = nil;
        NSEnumerator * enumerator = [ childWindows objectEnumerator ];
        while ( ( element = [ enumerator nextObject ] ) && hitChild == nil ) {
#ifdef USE_JAVA
            if ( ( [ element isKindOfClass: [ VCLPanel class ] ] || [ element isKindOfClass: [ VCLWindow class ] ] ) && [ self isViewElement: element hitByPoint: point ] ) {
#else	// USE_JAVA
            if ( [ element isKindOfClass: [ SalFrameWindow class ] ] && [ self isViewElement: element hitByPoint: point ] ) {
#endif	// USE_JAVA
                // we have a child window that is hit
#ifdef USE_JAVA
                Reference < XAccessibleRelationSet > relationSet;
                if ( [ element isKindOfClass: [ VCLPanel class ] ] )
                	relationSet = [ ( ( VCLPanel * ) element ) accessibleContext ] -> getAccessibleRelationSet();
                else
                	relationSet = [ ( ( VCLWindow * ) element ) accessibleContext ] -> getAccessibleRelationSet();
#else	// USE_JAVA
                Reference < XAccessibleRelationSet > relationSet = [ ( ( SalFrameWindow * ) element ) accessibleContext ] -> getAccessibleRelationSet();
#endif	// USE_JAVA
                if ( relationSet.is() && relationSet -> containsRelation ( AccessibleRelationType::SUB_WINDOW_OF )) {
                    // we have a valid relation to the parent element
                    AccessibleRelation relation = relationSet -> getRelationByType ( AccessibleRelationType::SUB_WINDOW_OF );
                    for ( int i = 0; i < relation.TargetSet.getLength() && !hitChild.is(); i++ ) {
                        Reference < XAccessible > rxAccessible ( relation.TargetSet [ i ], UNO_QUERY );
                        if ( rxAccessible.is() && rxAccessible -> getAccessibleContext().is() ) {
                            // hit test for children of parent
                            hitChild = hitTestRunner ( hitPoint, rxAccessible -> getAccessibleContext() );
                        }
                    }
                }
            }
        }
    }
    // nothing hit yet, so check ourself
    if ( ! hitChild.is() ) {
        if ( mpReferenceWrapper == nil ) {
            [ self setDefaults: [ self accessibleContext ] ];
        }
        hitChild = hitTestRunner ( hitPoint, mpReferenceWrapper -> rAccessibleContext );
    }
    if ( hitChild.is() ) {
        wrapper = [ AquaA11yFactory wrapperForAccessibleContext: hitChild ];
    }
    if ( wrapper != nil ) {
        [ wrapper retain ]; // TODO: retain only when transient ?
    }
#ifdef USE_JAVA
    RELEASE_DRAGPRINTLOCK
#endif	// USE_JAVA
    return wrapper;
}

#pragma mark -
#pragma mark Access Methods

-(XAccessibleAction *)accessibleAction {
    return mpReferenceWrapper -> rAccessibleAction.get();
}

-(XAccessibleContext *)accessibleContext {
    return mpReferenceWrapper -> rAccessibleContext.get();
}

-(XAccessibleComponent *)accessibleComponent {
    return mpReferenceWrapper -> rAccessibleComponent.get();
}

-(XAccessibleExtendedComponent *)accessibleExtendedComponent {
    return mpReferenceWrapper -> rAccessibleExtendedComponent.get();
}

-(XAccessibleSelection *)accessibleSelection {
    return mpReferenceWrapper -> rAccessibleSelection.get();
}

-(XAccessibleTable *)accessibleTable {
    return mpReferenceWrapper -> rAccessibleTable.get();
}

-(XAccessibleText *)accessibleText {
    return mpReferenceWrapper -> rAccessibleText.get();
}

-(XAccessibleEditableText *)accessibleEditableText {
    return mpReferenceWrapper -> rAccessibleEditableText.get();
}

-(XAccessibleValue *)accessibleValue {
    return mpReferenceWrapper -> rAccessibleValue.get();
}

-(XAccessibleTextAttributes *)accessibleTextAttributes {
    return mpReferenceWrapper -> rAccessibleTextAttributes.get();
}

-(XAccessibleMultiLineText *)accessibleMultiLineText {
    return mpReferenceWrapper -> rAccessibleMultiLineText.get();
}

-(XAccessibleTextMarkup *)accessibleTextMarkup {
    return mpReferenceWrapper -> rAccessibleTextMarkup.get();
}

-(NSWindow*)windowForParent {
#ifdef USE_JAVA
    return nil;
#else	// USE_JAVA
    return [self window];
#endif	// USE_JAVA
}

// These four are for AXTextAreas only. They are needed, because bold and italic
// attributes have to be bound to a font on the Mac. Our UNO-API instead handles
// and reports them independently. When they occur we bundle them to a font with
// this information here to create a according NSFont.
-(void)setActsAsRadioGroup:(BOOL)actsAsRadioGroup {
    mActsAsRadioGroup = actsAsRadioGroup;
}

-(BOOL)actsAsRadioGroup {
    return mActsAsRadioGroup;
}

+(void)setPopupMenuOpen:(BOOL)popupMenuOpen {
    isPopupMenuOpen = popupMenuOpen;
}

#ifdef USE_JAVA

// NSAccessibility selectors

- (BOOL)isAccessibilityElement
{
    return ! [ self accessibilityIsIgnored ];
}

- (BOOL)isAccessibilityFocused
{
    NSNumber *pNumber = [ self accessibilityAttributeValue: NSAccessibilityFocusedAttribute ];
    if ( pNumber )
        return [ pNumber boolValue ];
    else
        return NO;
}

- (id)accessibilityTopLevelUIElement
{
    return [ self accessibilityAttributeValue: NSAccessibilityTopLevelUIElementAttribute ];
}

- (id)accessibilityValue
{
    return [ self accessibilityAttributeValue: NSAccessibilityValueAttribute ];
}

- (NSArray *)accessibilityVisibleChildren
{
    return [ self accessibilityChildren ];
}

- (NSAccessibilitySubrole)accessibilitySubrole
{
    return [ self accessibilityAttributeValue: NSAccessibilitySubroleAttribute ];
}

- (NSString *)accessibilityTitle
{
    return [ self accessibilityAttributeValue: NSAccessibilityTitleAttribute ];
}

- (id)accessibilityTitleUIElement
{
    return [ self accessibilityAttributeValue: NSAccessibilityTitleUIElementAttribute ];
}

- (NSAccessibilityOrientation)accessibilityOrientation
{
    NSNumber *pNumber = [ self accessibilityAttributeValue: NSAccessibilityOrientationAttribute ];
    if ( pNumber )
        return (NSAccessibilityOrientation)[ pNumber integerValue ];
    else
        return NSAccessibilityOrientationUnknown;
}

- (id)accessibilityParent
{
    return [ self accessibilityAttributeValue: NSAccessibilityParentAttribute ];
}

- (NSAccessibilityRole)accessibilityRole
{
    return [ self accessibilityAttributeValue: NSAccessibilityRoleAttribute ];
}

- (NSString *)accessibilityRoleDescription
{
    return [ self accessibilityAttributeValue: NSAccessibilityRoleDescriptionAttribute ];
}

- (BOOL)isAccessibilitySelected
{
    NSNumber *pNumber = [ self accessibilityAttributeValue: NSAccessibilitySelectedAttribute ];
    if ( pNumber )
        return [ pNumber boolValue ];
    else
        return NO;
}

- (NSArray *)accessibilitySelectedChildren
{
    return [ self accessibilityAttributeValue: NSAccessibilitySelectedChildrenAttribute ];
}

- (NSArray *)accessibilityServesAsTitleForUIElements
{
    return [ self accessibilityAttributeValue: NSAccessibilityServesAsTitleForUIElementsAttribute ];
}

- (id)accessibilityMinValue
{
    return [ self accessibilityAttributeValue: NSAccessibilityMinValueAttribute ];
}

- (id)accessibilityMaxValue
{
    return [ self accessibilityAttributeValue: NSAccessibilityMaxValueAttribute ];
}

- (id)accessibilityWindow
{
    return [ self accessibilityAttributeValue: NSAccessibilityWindowAttribute ];
}

- (NSString *)accessibilityHelp
{
    return [ self accessibilityAttributeValue: NSAccessibilityHelpAttribute ];
}

- (BOOL)isAccessibilityExpanded
{
    NSNumber *pNumber = [ self accessibilityAttributeValue: NSAccessibilityExpandedAttribute ];
    if ( pNumber )
        return [ pNumber boolValue ];
    else
        return NO;
}

- (BOOL)isAccessibilityEnabled
{
    NSNumber *pNumber = [ self accessibilityAttributeValue: NSAccessibilityEnabledAttribute ];
    if ( pNumber )
        return [ pNumber boolValue ];
    else
        return NO;
}

- (NSArray *)accessibilityChildren
{
    return [ self accessibilityAttributeValue: NSAccessibilityChildrenAttribute ];
}

- (NSArray <id<NSAccessibilityElement>> *)accessibilityChildrenInNavigationOrder
{
    return [ self accessibilityChildren ];
}

- (NSArray *)accessibilityContents
{
    return [ self accessibilityAttributeValue: NSAccessibilityContentsAttribute ];
}

- (NSString *)accessibilityLabel
{
    return [ self accessibilityAttributeValue: NSAccessibilityDescriptionAttribute ];
}

- (id)accessibilityApplicationFocusedUIElement
{
    return [ self accessibilityFocusedUIElement ];
}

- (BOOL)isAccessibilityDisclosed
{
    NSNumber *pNumber = [ self accessibilityAttributeValue: NSAccessibilityDisclosingAttribute ];
    if ( pNumber )
        return [ pNumber boolValue ];
    else
        return NO;
}

- (id)accessibilityHorizontalScrollBar
{
    return [ self accessibilityAttributeValue: NSAccessibilityHorizontalScrollBarAttribute ];
}

- (id)accessibilityVerticalScrollBar
{
    return [ self accessibilityAttributeValue: NSAccessibilityVerticalScrollBarAttribute ];
}

- (NSArray *)accessibilityTabs
{
    return [ self accessibilityAttributeValue: NSAccessibilityTabsAttribute ];
}

- (NSArray *)accessibilityColumns
{
    return [ self accessibilityAttributeValue: NSAccessibilityColumnsAttribute ];
}

- (NSArray *)accessibilityRows
{
    return [ self accessibilityAttributeValue: NSAccessibilityRowsAttribute ];
}

- (NSRange)accessibilitySharedCharacterRange
{
    NSValue *pValue = [ self accessibilityAttributeValue: NSAccessibilitySharedCharacterRangeAttribute ];
    if ( pValue )
        return [ pValue rangeValue ];
    else
        return NSMakeRange( NSNotFound, 0 );
}

- (NSArray *)accessibilitySharedTextUIElements
{
    return [ self accessibilityAttributeValue: NSAccessibilitySharedTextUIElementsAttribute ];
}

- (NSRange)accessibilityVisibleCharacterRange
{
    NSValue *pValue = [ self accessibilityAttributeValue: NSAccessibilityVisibleCharacterRangeAttribute ];
    if ( pValue )
        return [ pValue rangeValue ];
    else
        return NSMakeRange( NSNotFound, 0 );
}

- (NSInteger)accessibilityNumberOfCharacters
{
    NSNumber *pNumber = [ self accessibilityAttributeValue: NSAccessibilityNumberOfCharactersAttribute ];
    if ( pNumber )
        return [ pNumber integerValue ];
    else
        return 0;
}

- (NSString *)accessibilitySelectedText
{
    return [ self accessibilityAttributeValue: NSAccessibilitySelectedTextAttribute ];
}

- (NSRange)accessibilitySelectedTextRange
{
    NSValue *pValue = [ self accessibilityAttributeValue: NSAccessibilitySelectedTextRangeAttribute ];
    if ( pValue )
        return [ pValue rangeValue ];
    else
        return NSMakeRange( NSNotFound, 0 );
}

- (NSAttributedString *)accessibilityAttributedStringForRange:(NSRange)aRange
{
    return [ self accessibilityAttributeValue: NSAccessibilityAttributedStringForRangeParameterizedAttribute forParameter: [ NSValue valueWithRange: aRange ] ];
}

- (NSRange)accessibilityRangeForLine:(NSInteger)nLine
{
    NSValue *pValue = [ self accessibilityAttributeValue: NSAccessibilityRangeForLineParameterizedAttribute forParameter: [NSNumber numberWithInteger: nLine ] ];
    if ( pValue )
        return [ pValue rangeValue ];
    else
        return NSMakeRange( NSNotFound, 0 );
}

- (NSString *)accessibilityStringForRange:(NSRange)aRange
{
    return [ self accessibilityAttributeValue: NSAccessibilityStringForRangeParameterizedAttribute forParameter: [ NSValue valueWithRange: aRange ] ];
}

- (NSRange)accessibilityRangeForPosition:(NSPoint)aPoint
{
    NSValue *pValue = [ self accessibilityAttributeValue: NSAccessibilityRangeForPositionParameterizedAttribute forParameter: [ NSValue valueWithPoint: aPoint ] ];
    if ( pValue )
        return [ pValue rangeValue ];
    else
        return NSMakeRange( NSNotFound, 0 );
}

- (NSRange)accessibilityRangeForIndex:(NSInteger)nIndex
{
    NSValue *pValue = [ self accessibilityAttributeValue: NSAccessibilityRangeForIndexParameterizedAttribute forParameter: [ NSNumber numberWithInteger: nIndex ] ];
    if ( pValue )
        return [ pValue rangeValue ];
    else
        return NSMakeRange( NSNotFound, 0 );
}

- (NSRect)accessibilityFrameForRange:(NSRange)aRange
{
    NSValue *pValue = [ self accessibilityAttributeValue: NSAccessibilityBoundsForRangeParameterizedAttribute forParameter: [ NSValue valueWithRange: aRange ] ];
    if ( pValue )
        return [ pValue rectValue ];
    else
        return NSZeroRect;
}

- (NSData *)accessibilityRTFForRange:(NSRange)aRange
{
    return [ self accessibilityAttributeValue: NSAccessibilityRTFForRangeParameterizedAttribute forParameter: [ NSValue valueWithRange: aRange ] ];
}

- (NSRange)accessibilityStyleRangeForIndex:(NSInteger)nIndex
{
    NSValue *pValue = [ self accessibilityAttributeValue: NSAccessibilityStyleRangeForIndexParameterizedAttribute forParameter: [ NSNumber numberWithInteger: nIndex ] ];
    if ( pValue )
        return [ pValue rangeValue ];
    else
        return NSMakeRange( NSNotFound, 0 );
}

- (NSInteger)accessibilityLineForIndex:(NSInteger)nIndex
{
    NSNumber *pNumber = [ self accessibilityAttributeValue: NSAccessibilityLineForIndexParameterizedAttribute forParameter: [ NSNumber numberWithInteger: nIndex ] ];
    if ( pNumber )
        return [ pNumber integerValue ];
    else
        return 0;
}

- (BOOL)accessibilityPerformIncrement
{
    return [ self performAction: NSAccessibilityIncrementAction ];
}

- (BOOL)accessibilityPerformDecrement
{
    return [ self performAction: NSAccessibilityDecrementAction ];
}

// NSAccessibilityElement selectors

- (NSRect)accessibilityFrame
{
    if ( !ImplApplicationIsRunning() )
        return NSZeroRect;
    // Set drag lock if it has not already been set since dispatching native
    // events to windows during an accessibility call can cause crashing
    ACQUIRE_DRAGPRINTLOCK
    if ( [ self isDisposed ] ) {
        RELEASE_DRAGPRINTLOCKIFNEEDED
        return NSZeroRect;
    }
    XAccessibleComponent *pAccessibleComponent = [ self accessibleComponent ];
    if ( pAccessibleComponent ) {
        com::sun::star::awt::Point location = pAccessibleComponent->getLocationOnScreen();
        com::sun::star::awt::Size size = pAccessibleComponent->getSize();
        NSRect screenRect = JavaSalFrame::GetTotalScreenBounds();
        NSRect frame = NSMakeRect( (float)location.X, (float)( screenRect.size.height - size.Height - location.Y ), (float)size.Width, (float)size.Height );
        RELEASE_DRAGPRINTLOCKIFNEEDED
        return frame;
    }
    RELEASE_DRAGPRINTLOCK
    return NSZeroRect;
}

- (BOOL)accessibilityNotifiesWhenDestroyed
{
    return YES;
}

// NSAccessibilityButton selectors

- (BOOL)accessibilityPerformPress
{
    return [ self performAction: NSAccessibilityPressAction ];
}

#endif	// USE_JAVA

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
