//
//  AppDelegate.h
//  Flipr
//
//  Created by Rainer Brockerhoff on 12/20/06.
//  Copyright 2006,2007 Rainer Brockerhoff. Some rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface AppDelegate : NSObject {
	IBOutlet NSTextView* text1;
	IBOutlet NSWindow* window1;
	IBOutlet NSTextView* text2;
	IBOutlet NSWindow* window2;
}
- (IBAction)flipAction:(id)sender;
@end
