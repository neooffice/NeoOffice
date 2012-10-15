//
//  NSWindow_Flipr.h
//  Flipr
//
//  Created by Rainer Brockerhoff on 12/20/06.
//  Copyright 2006,2007 Rainer Brockerhoff. Some rights reserved.
//

#import <Cocoa/Cocoa.h>

// Read the "ReadMe.rtf" file for general discussion.

#ifdef USE_JAVA
// Redefine class name to avoid namespace conflict with the iMedia browser
@interface NSWindow (NeoMobileFlipperWindow)
#else	// USE_JAVA
@interface NSWindow (NSWindow_Flipr)
#endif	// USE_JAVA

// Call during initialization this to prepare the flipping window.
// If you don't call this, the first flip will take a little longer.

#ifdef USE_JAVA
+ (NSWindow*)neoMobileFlippingWindow;
#else	// USE_JAVA
+ (NSWindow*)flippingWindow;
#endif	// USE_JAVA

// Call this if you want to release the flipping window. If you flip
// again after calling this, it will take a little longer.

#ifdef USE_JAVA
+ (void)neoMobileReleaseFlippingWindow;
#else	// USE_JAVA
+ (void)releaseFlippingWindow;
#endif	// USE_JAVA

// Call this on a visible window to flip it and show the parameter window,
// which is supposed to not be on-screen.

#ifdef USE_JAVA
- (void)neoMobileFlipToShowWindow:(NSWindow*)window forward:(BOOL)forward;
#else	// USE_JAVA
- (void)flipToShowWindow:(NSWindow*)window forward:(BOOL)forward;
#endif	// USE_JAVA

@end
