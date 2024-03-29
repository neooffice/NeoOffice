//
//  NSWindow_Flipr.m
//  Flipr
//
//  Created by Rainer Brockerhoff on 12/20/06.
//  Copyright 2006,2007 Rainer Brockerhoff. Some rights reserved.
//

#import "NSWindow_Flipr.h"
#import <QuartzCore/QuartzCore.h>
#include <sys/sysctl.h>

// Read the "ReadMe.rtf" file for general discussion.

// This is the minimum duration of the animation in seconds. 0.75 seems best.
#define DURATION (0.75)

// We subclass NSAnimation to maximize frame rate, instead of using progress marks.

#ifdef USE_JAVA
// Redefine class name to avoid namespace conflict with the iMedia browser
@interface NeoMobileFliprAnimation : NSAnimation {
#else	// USE_JAVA
@interface FliprAnimation : NSAnimation {
#endif	// USE_JAVA
}
@end

#ifdef USE_JAVA
@implementation NeoMobileFliprAnimation
#else	// USE_JAVA
@implementation FliprAnimation
#endif	// USE_JAVA

// We initialize the animation with some huge default value.

- (id)initWithAnimationCurve:(NSAnimationCurve)animationCurve {
	self = [super initWithDuration:1.0E8 animationCurve:animationCurve];
	return self;
}

// We call this to start the animation just beyond the first frame.

- (void)startAtProgress:(NSAnimationProgress)value withDuration:(NSTimeInterval)duration {
	[super setCurrentProgress:value];
	[self setDuration:duration];
	[self startAnimation];
}

// Called periodically by the NSAnimation timer.

- (void)setCurrentProgress:(NSAnimationProgress)progress {
// Call super to update the progress value.
	[super setCurrentProgress:progress];
	if ([self isAnimating]&&(progress<0.99)) {
/// Update the window unless we're nearly at the end. No sense duplicating the final window.
// We can be sure the delegate responds to display.
		[[self delegate] display];
	}
}

@end

// This is the flipping window's content view.

#ifdef USE_JAVA
// Redefine class name to avoid namespace conflict with the iMedia browser
@interface NeoMobileFliprView : NSView {
#else	// USE_JAVA
@interface FliprView : NSView {
#endif	// USE_JAVA
	NSRect originalRect;			// this rect covers the initial and final windows.
	NSWindow* initialWindow;
	NSWindow* finalWindow;
    CIImage* finalImage;			// this is the rendered image of the final window.
	CIFilter* transitionFilter;
	NSShadow* shadow;
#ifdef USE_JAVA
	NeoMobileFliprAnimation* animation;
#else	// USE_JAVA
	FliprAnimation* animation;
#endif	// USE_JAVA
	float direction;				// this will be 1 (forward) or -1 (backward).
	float frameTime;				// time for last drawRect:
}
@end

#ifdef USE_JAVA
@implementation NeoMobileFliprView
#else	// USE_JAVA
@implementation FliprView
#endif	// USE_JAVA

// The designated initializer; will be called when the flipping window is set up.

- (id)initWithFrame:(NSRect)frame andOriginalRect:(NSRect)rect {
	self = [super initWithFrame:frame];
	if (self) {
		originalRect = rect;
		initialWindow = nil;
		finalWindow = nil;
		finalImage = nil;
		animation = nil;
		frameTime = 0.0;
// Initialize the CoreImage filter.
		transitionFilter = [[CIFilter filterWithName:@"CIPerspectiveTransform"] retain];
		[transitionFilter setDefaults];
// These parameters come from http://boredzo.org/imageshadowadder/ by Peter Hosey,
// and reproduce reasonably well the standard Tiger NSWindow shadow.
// You should change these when flipping NSPanels and/or on Leopard.
		shadow = [[NSShadow alloc] init];
		[shadow setShadowColor:[[NSColor shadowColor] colorWithAlphaComponent:0.8]];
		[shadow setShadowBlurRadius:23];
		[shadow setShadowOffset:NSMakeSize(0,-8)];
	}
	return self;
}

// Notice that we don't retain the initial and final windows, so we don't release them here either.

- (void)dealloc {
	[finalImage release];
	[transitionFilter release];
	[shadow release];
	[animation release];
	[super dealloc];
}

// This view, and the flipping window itself, are mostly transparent.

- (BOOL)isOpaque {
	return NO;
}


// This is called to calculate the transition images and to start the animation.
// The initial and final windows aren't retained, so weird things might happen if
// they go away during the animation. We assume both windows have the exact same frame.

- (void)setInitialWindow:(NSWindow*)initial andFinalWindow:(NSWindow*)final forward:(BOOL)forward {
#ifdef USE_JAVA
	NSWindow* flipr = [NSWindow neoMobileFlippingWindow];
#else	// USE_JAVA
	NSWindow* flipr = [NSWindow flippingWindow];
#endif	// USE_JAVA
	if (flipr) {
		[NSCursor hide];
		initialWindow = initial;
		finalWindow = final;
		direction = forward?1:-1;
// Here we reposition and resize the flipping window so that originalRect will cover the original windows.
		NSRect frame = [initialWindow frame];
		NSRect flp = [flipr frame];
		flp.origin.x = frame.origin.x-originalRect.origin.x;
		flp.origin.y = frame.origin.y-originalRect.origin.y;
		flp.size.width += frame.size.width-originalRect.size.width;
		flp.size.height += frame.size.height-originalRect.size.height;
		[flipr setFrame:flp display:NO];
		originalRect.size = frame.size;
// Here we get an image of the initial window and make a CIImage from it.
		NSView* view = [[initialWindow contentView] superview];
		flp = [view bounds];
		NSBitmapImageRep* bitmap = [view bitmapImageRepForCachingDisplayInRect:flp];
		[view cacheDisplayInRect:flp toBitmapImageRep:bitmap];
		CIImage* initialImage = [[CIImage alloc] initWithBitmapImageRep:bitmap];
// We immediately pass the initial image to the filter and release it.
		[transitionFilter setValue:initialImage forKey:@"inputImage"];
		[initialImage release];
// To prevent flicker...
		NSDisableScreenUpdates();
// We bring the final window to the front in order to build the final image.
		[finalWindow makeKeyAndOrderFront:self];
// Here we get an image of the final window and make a CIImage from it.
		view = [[finalWindow contentView] superview];
		flp = [view bounds];
		bitmap = [view bitmapImageRepForCachingDisplayInRect:flp];
		[view cacheDisplayInRect:flp toBitmapImageRep:bitmap];
		finalImage = [[CIImage alloc] initWithBitmapImageRep:bitmap];
// To save time, we don't order the final window out, just make it completely transparent.
		[finalWindow setAlphaValue:0];
		[initialWindow orderOut:self];
// This will draw the first frame at value 0, duplicating the initial window. This is not really optimal,
// but we need to compensate for the time spent here, which seems to be about 3 to 5x what's needed
// for subsequent frames.
#ifdef USE_JAVA
		animation = [[NeoMobileFliprAnimation alloc] initWithAnimationCurve:NSAnimationEaseInOut];
#else	// USE_JAVA
		animation = [[FliprAnimation alloc] initWithAnimationCurve:NSAnimationEaseInOut];
#endif	// USE_JAVA
		[animation setDelegate:self];
// This is probably redundant...
		[animation setCurrentProgress:0.0];
		[flipr orderWindow:NSWindowBelow relativeTo:[finalWindow windowNumber]];
		float duration = DURATION;
// Slow down by a factor of 5 if the shift key is down.
		if ([[NSApp currentEvent] modifierFlags]&NSShiftKeyMask) {
			duration *= 5.0;
		}
// We accumulate drawing time and draw a second frame at the point where the rotation starts to show.
		float totalTime = frameTime;
		[animation setCurrentProgress:DURATION/15];
// Now we update the screen and the second frame appears, boom! :-)
		NSEnableScreenUpdates();
		totalTime += frameTime;
// We set up the animation. At this point, totalTime will be the time needed to draw the first two frames,
// and frameTime the time for the second (normal) frame.
// We stretch the duration, if necessary, to make sure at least 5 more frames will be drawn. 
		if ((duration-totalTime)<(frameTime*5)) {
			duration = totalTime+frameTime*5;
		}
// ...and everything else happens in the animation delegates. We start the animation just
// after the second frame.
		[animation startAtProgress:totalTime/duration withDuration:duration];
	}
}

// This is called when the animation has finished.

- (void)animationDidEnd:(NSAnimation*)theAnimation {
// We order the flipping window out and make the final window visible again.
	NSDisableScreenUpdates();
#ifdef USE_JAVA
	[[NSWindow neoMobileFlippingWindow] orderOut:self];
#else	// USE_JAVA
	[[NSWindow flippingWindow] orderOut:self];
#endif	// USE_JAVA
	[finalWindow setAlphaValue:1.0];
	[finalWindow display];
	NSEnableScreenUpdates();
// Clear stuff out...
	[animation autorelease];
	animation = nil;
	initialWindow = nil;
	finalWindow = nil;
	[finalImage release];
	finalImage = nil;
	[NSCursor unhide];
}

// All the magic happens here... drawing the flipping animation.

- (void)drawRect:(NSRect)rect {
	if (!initialWindow) {
// If there's no window yet, we don't need to draw anything.
		return;
	}
// For calculating the draw time...
	AbsoluteTime startTime = UpTime();
// time will vary from 0.0 to 1.0. 0.5 means halfway.
	float time = [animation currentValue];
// This code was adapted from http://www.macs.hw.ac.uk/~rpointon/osx/coreimage.html by Robert Pointon.
// First we calculate the perspective.
	float radius = originalRect.size.width/2;
	float width = radius;
	float height = originalRect.size.height/2;
	float dist = 1600; // visual distance to flipping window, 1600 looks about right. You could try radius*5, too.
	float angle = direction*M_PI*time;
	float px1 = radius*cos(angle);
	float pz = radius*sin(angle);
	float pz1 = dist+pz;
	float px2 = -px1;
	float pz2 = dist-pz;
	if (time>0.5) {
// At this point,  we need to swap in the final image, for the second half of the animation.
		if (finalImage) {
			[transitionFilter setValue:finalImage forKey:@"inputImage"];
			[finalImage release];
			finalImage = nil;
		}
		float ss;
		ss = px1; px1 = px2; px2 = ss;
		ss = pz1; pz1 = pz2; pz2 = ss;
	}
	float sx1 = dist*px1/pz1;
	float sy1 = dist*height/pz1;
	float sx2 = dist*px2/pz2;
	float sy2 = dist*height/pz2;
// Everything is set up, we pass the perspective to the CoreImage filter
	[transitionFilter setValue:[CIVector vectorWithX:width+sx1 Y:height+sy1] forKey:@"inputTopRight"];
	[transitionFilter setValue:[CIVector vectorWithX:width+sx2 Y:height+sy2] forKey:@"inputTopLeft" ];
	[transitionFilter setValue:[CIVector vectorWithX:width+sx1 Y:height-sy1] forKey:@"inputBottomRight"];
	[transitionFilter setValue:[CIVector vectorWithX:width+sx2 Y:height-sy2] forKey:@"inputBottomLeft"];
	CIImage* outputCIImage = [transitionFilter valueForKey:@"outputImage"];
// This will make the standard window shadow appear beneath the flipping window
	[shadow set];
// And we draw the result image.
	NSRect bounds = [self bounds];
	[outputCIImage drawInRect:bounds fromRect:NSMakeRect(-originalRect.origin.x,-originalRect.origin.y,bounds.size.width,bounds.size.height) operation:NSCompositeSourceOver fraction:1.0];
// Calculate the time spent drawing
	frameTime = UnsignedWideToUInt64(AbsoluteDeltaToNanoseconds(UpTime(),startTime))/1E9;
}

@end

#ifdef USE_JAVA
@implementation NSWindow (NeoMobileFlipperWindow)
#else	// USE_JAVA
@implementation NSWindow (NSWindow_Flipr)
#endif	// USE_JAVA

// This function checks if the CPU can perform flipping. We assume all Intel Macs can do it,
// but PowerPC Macs need AltiVec.

static BOOL CPUIsSuitable() {
#ifdef __LITTLE_ENDIAN__
	return YES;
#else
	int altivec = 0;
	size_t length = sizeof(altivec);
	int error = sysctlbyname("hw.optional.altivec",&altivec,&length,NULL,0);
	return error?NO:altivec!=0;
#endif
}

// There's only one flipping window!

static NSWindow* flippingWindow = nil;

// Get (and initialize, if necessary) the flipping window.

#ifdef USE_JAVA
+ (NSWindow*)neoMobileFlippingWindow {
#else	// USE_JAVA
+ (NSWindow*)flippingWindow {
#endif	// USE_JAVA
	if (!flippingWindow) {
// We initialize the flipping window if the CPU can do it...
		if (CPUIsSuitable()) {
// This is a little arbitary... the window will be resized every time it's used.
			NSRect frame = NSMakeRect(128,128,512,768);
			flippingWindow = [[NSWindow alloc] initWithContentRect:frame styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:NO];
			[flippingWindow setBackgroundColor:[NSColor clearColor]];
			[flippingWindow setOpaque:NO];	
			[flippingWindow setHasShadow:NO];
			[flippingWindow setOneShot:YES];
			frame.origin = NSZeroPoint;
// The inset values seem large enough so the animation doesn't slop over the frame.
// They could be calculated more exactly, though.
#ifdef USE_JAVA
			NeoMobileFliprView* view = [[[NeoMobileFliprView alloc] initWithFrame:frame andOriginalRect:NSInsetRect(frame,64,256)] autorelease];
#else	// USE_JAVA
			FliprView* view = [[[FliprView alloc] initWithFrame:frame andOriginalRect:NSInsetRect(frame,64,256)] autorelease];
#endif	// USE_JAVA
			[view setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
			[flippingWindow setContentView:view];
		}
	}
	return flippingWindow;
}

// Release the flipping window.

#ifdef USE_JAVA
+ (void)neoMobileReleaseFlippingWindow {
#else	// USE_JAVA
+ (void)releaseFlippingWindow {
#endif	// USE_JAVA
	[flippingWindow autorelease];
	flippingWindow = nil;
}

// This is called from outside to start the animation process.

#ifdef USE_JAVA
- (void)neoMobileFlipToShowWindow:(NSWindow*)window forward:(BOOL)forward {
#else	// USE_JAVA
- (void)flipToShowWindow:(NSWindow*)window forward:(BOOL)forward {
#endif	// USE_JAVA
// We resize the final window to exactly the same frame.
	[window setFrame:[self frame] display:NO];
#ifdef USE_JAVA
	// Disable flipping as it is too slow on some versions of Mac OS X
	NSWindow* flipr = nil;
#else	// USE_JAVA
	NSWindow* flipr = [NSWindow flippingWindow];
#endif	// USE_JAVA
	if (!flipr) {
// If we fall in here, the CPU isn't able to animate and we just change windows.
		[window makeKeyAndOrderFront:self];
		[self orderOut:self];
		return;
	}
#ifdef USE_JAVA
	[(NeoMobileFliprView*)[flipr contentView] setInitialWindow:self andFinalWindow:window forward:forward];
#else	// USE_JAVA
	[(FliprView*)[flipr contentView] setInitialWindow:self andFinalWindow:window forward:forward];
#endif	// USE_JAVA
}

@end
