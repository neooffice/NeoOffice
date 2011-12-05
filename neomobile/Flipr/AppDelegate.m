//
//  AppDelegate.m
//  Flipr
//
//  Created by Rainer Brockerhoff on 12/20/06.
//  Copyright 2006,2007 Rainer Brockerhoff. Some rights reserved.
//

#import "AppDelegate.h"
#import "NSWindow_Flipr.h"

// Read the "ReadMe.rtf" file for general discussion.

@implementation AppDelegate

// We call flippingWindow to preset the window used for flipping.
// This reduces the delay on the first flip.

- (void)applicationWillFinishLaunching:(NSNotification*)aNotification {
	NSString* readme = [[NSBundle mainBundle] pathForResource:@"ReadMe" ofType:@"rtf"];
	[text1 readRTFDFromFile:readme];
	[text2 readRTFDFromFile:readme];
	[NSWindow flippingWindow];
}

// This action method is connected to the two "Flip" buttons.
// In order to capture the buttons in the unhighlighted state, we do a delayed perform on the appropriate method.

- (IBAction)flipAction:(id)sender {
	[self performSelector:[NSApp keyWindow]==window1?@selector(flipForward):@selector(flipBackward) withObject:nil afterDelay:0.0];
}

// These flip forward and backward. In the nib file, window1 is set as visible at load time, window2 not.

- (void)flipForward {
	[window1 flipToShowWindow:window2 forward:YES];
}

- (void)flipBackward {
	[window2 flipToShowWindow:window1 forward:NO];
}

@end
