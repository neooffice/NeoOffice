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
 *	 - GNU General Public License Version 2.1
 *
 *  Patrick Luby, June 2003
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 by Patrick Luby (patrick.luby@planamesa.com)
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
 ************************************************************************/

package com.sun.star.vcl;

import java.awt.AWTEvent;
import java.awt.Rectangle;
import java.awt.Toolkit;
import java.awt.event.ComponentEvent;
import java.awt.event.InputEvent;
import java.awt.event.InputMethodEvent;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseWheelEvent;
import java.awt.event.PaintEvent;
import java.awt.font.TextAttribute;
import java.awt.font.TextHitInfo;
import java.awt.im.InputMethodHighlight;
import java.text.CharacterIterator;
import java.text.AttributedCharacterIterator;

/**
 * The Java class that extends the <code>AWTEvent</code> class. This class is
 * used by the SalFrame C++ class methods to post events to the Java event
 * queue.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public final class VCLEvent extends AWTEvent {

	/**
	 * KEYGROUP_NUM constant.
	 */
	public final static int KEYGROUP_NUM = 0x0100;

	/**
	 * KEYGROUP_ALPHA constant.
	 */
	public final static int KEYGROUP_ALPHA = 0x0200;

	/**
	 * KEYGROUP_FKEYS constant.
	 */
	public final static int KEYGROUP_FKEYS = 0x0300;

	/**
	 * KEYGROUP_CURSOR constant.
	 */
	public final static int KEYGROUP_CURSOR = 0x0400;

	/**
	 * KEYGROUP_MISC constant.
	 */
	public final static int KEYGROUP_MISC = 0x0500;

	/**
	 * KEYGROUP_TYPE constant.
	 */
	public final static int KEYGROUP_TYPE = 0x0f00;

	/**
	 * KEY_0 constant.
	 */
	public final static int KEY_0 = VCLEvent.KEYGROUP_NUM + 0;

	/**
	 * KEY_1 constant.
	 */
	public final static int KEY_1 = VCLEvent.KEYGROUP_NUM + 1;

	/**
	 * KEY_2 constant.
	 */
	public final static int KEY_2 = VCLEvent.KEYGROUP_NUM + 2;

	/**
	 * KEY_3 constant.
	 */
	public final static int KEY_3 = VCLEvent.KEYGROUP_NUM + 3;

	/**
	 * KEY_4 constant.
	 */
	public final static int KEY_4 = VCLEvent.KEYGROUP_NUM + 4;

	/**
	 * KEY_5 constant.
	 */
	public final static int KEY_5 = VCLEvent.KEYGROUP_NUM + 5;

	/**
	 * KEY_6 constant.
	 */
	public final static int KEY_6 = VCLEvent.KEYGROUP_NUM + 6;

	/**
	 * KEY_7 constant.
	 */
	public final static int KEY_7 = VCLEvent.KEYGROUP_NUM + 7;

	/**
	 * KEY_8 constant.
	 */
	public final static int KEY_8 = VCLEvent.KEYGROUP_NUM + 8;

	/**
	 * KEY_9 constant.
	 */
	public final static int KEY_9 = VCLEvent.KEYGROUP_NUM + 9;

	/**
	 * KEY_A constant.
	 */
	public final static int KEY_A = VCLEvent.KEYGROUP_ALPHA + 0;

	/**
	 * KEY_B constant.
	 */
	public final static int KEY_B = VCLEvent.KEYGROUP_ALPHA + 1;

	/**
	 * KEY_C constant.
	 */
	public final static int KEY_C = VCLEvent.KEYGROUP_ALPHA + 2;

	/**
	 * KEY_D constant.
	 */
	public final static int KEY_D = VCLEvent.KEYGROUP_ALPHA + 3;

	/**
	 * KEY_E constant.
	 */
	public final static int KEY_E = VCLEvent.KEYGROUP_ALPHA + 4;

	/**
	 * KEY_F constant.
	 */
	public final static int KEY_F = VCLEvent.KEYGROUP_ALPHA + 5;

	/**
	 * KEY_G constant.
	 */
	public final static int KEY_G = VCLEvent.KEYGROUP_ALPHA + 6;

	/**
	 * KEY_H constant.
	 */
	public final static int KEY_H = VCLEvent.KEYGROUP_ALPHA + 7;

	/**
	 * KEY_I constant.
	 */
	public final static int KEY_I = VCLEvent.KEYGROUP_ALPHA + 8;

	/**
	 * KEY_J constant.
	 */
	public final static int KEY_J = VCLEvent.KEYGROUP_ALPHA + 9;

	/**
	 * KEY_K constant.
	 */
	public final static int KEY_K = VCLEvent.KEYGROUP_ALPHA + 10;

	/**
	 * KEY_L constant.
	 */
	public final static int KEY_L = VCLEvent.KEYGROUP_ALPHA + 11;

	/**
	 * KEY_M constant.
	 */
	public final static int KEY_M = VCLEvent.KEYGROUP_ALPHA + 12;

	/**
	 * KEY_N constant.
	 */
	public final static int KEY_N = VCLEvent.KEYGROUP_ALPHA + 13;

	/**
	 * KEY_O constant.
	 */
	public final static int KEY_O = VCLEvent.KEYGROUP_ALPHA + 14;

	/**
	 * KEY_P constant.
	 */
	public final static int KEY_P = VCLEvent.KEYGROUP_ALPHA + 15;

	/**
	 * KEY_Q constant.
	 */
	public final static int KEY_Q = VCLEvent.KEYGROUP_ALPHA + 16;

	/**
	 * KEY_R constant.
	 */
	public final static int KEY_R = VCLEvent.KEYGROUP_ALPHA + 17;

	/**
	 * KEY_S constant.
	 */
	public final static int KEY_S = VCLEvent.KEYGROUP_ALPHA + 18;

	/**
	 * KEY_T constant.
	 */
	public final static int KEY_T = VCLEvent.KEYGROUP_ALPHA + 19;

	/**
	 * KEY_U constant.
	 */
	public final static int KEY_U = VCLEvent.KEYGROUP_ALPHA + 20;

	/**
	 * KEY_V constant.
	 */
	public final static int KEY_V = VCLEvent.KEYGROUP_ALPHA + 21;

	/**
	 * KEY_W constant.
	 */
	public final static int KEY_W = VCLEvent.KEYGROUP_ALPHA + 22;

	/**
	 * KEY_X constant.
	 */
	public final static int KEY_X = VCLEvent.KEYGROUP_ALPHA + 23;

	/**
	 * KEY_Y constant.
	 */
	public final static int KEY_Y = VCLEvent.KEYGROUP_ALPHA + 24;

	/**
	 * KEY_Z constant.
	 */
	public final static int KEY_Z = VCLEvent.KEYGROUP_ALPHA + 25;

	/**
	 * KEY_F1 constant.
	 */
	public final static int KEY_F1 = VCLEvent.KEYGROUP_FKEYS + 0;

	/**
	 * KEY_F2 constant.
	 */
	public final static int KEY_F2 = VCLEvent.KEYGROUP_FKEYS + 1;

	/**
	 * KEY_F3 constant.
	 */
	public final static int KEY_F3 = VCLEvent.KEYGROUP_FKEYS + 2;

	/**
	 * KEY_F4 constant.
	 */
	public final static int KEY_F4 = VCLEvent.KEYGROUP_FKEYS + 3;

	/**
	 * KEY_F5 constant.
	 */
	public final static int KEY_F5 = VCLEvent.KEYGROUP_FKEYS + 4;

	/**
	 * KEY_F6 constant.
	 */
	public final static int KEY_F6 = VCLEvent.KEYGROUP_FKEYS + 5;

	/**
	 * KEY_F7 constant.
	 */
	public final static int KEY_F7 = VCLEvent.KEYGROUP_FKEYS + 6;

	/**
	 * KEY_F8 constant.
	 */
	public final static int KEY_F8 = VCLEvent.KEYGROUP_FKEYS + 7;

	/**
	 * KEY_F9 constant.
	 */
	public final static int KEY_F9 = VCLEvent.KEYGROUP_FKEYS + 8;

	/**
	 * KEY_F10 constant.
	 */
	public final static int KEY_F10 = VCLEvent.KEYGROUP_FKEYS + 9;

	/**
	 * KEY_F11 constant.
	 */
	public final static int KEY_F11 = VCLEvent.KEYGROUP_FKEYS + 10;

	/**
	 * KEY_F12 constant.
	 */
	public final static int KEY_F12 = VCLEvent.KEYGROUP_FKEYS + 11;

	/**
	 * KEY_F13 constant.
	 */
	public final static int KEY_F13 = VCLEvent.KEYGROUP_FKEYS + 12;

	/**
	 * KEY_F14 constant.
	 */
	public final static int KEY_F14 = VCLEvent.KEYGROUP_FKEYS + 13;

	/**
	 * KEY_F15 constant.
	 */
	public final static int KEY_F15 = VCLEvent.KEYGROUP_FKEYS + 14;

	/**
	 * KEY_F16 constant.
	 */
	public final static int KEY_F16 = VCLEvent.KEYGROUP_FKEYS + 15;

	/**
	 * KEY_F17 constant.
	 */
	public final static int KEY_F17 = VCLEvent.KEYGROUP_FKEYS + 16;

	/**
	 * KEY_F18 constant.
	 */
	public final static int KEY_F18 = VCLEvent.KEYGROUP_FKEYS + 17;

	/**
	 * KEY_F19 constant.
	 */
	public final static int KEY_F19 = VCLEvent.KEYGROUP_FKEYS + 18;

	/**
	 * KEY_F20 constant.
	 */
	public final static int KEY_F20 = VCLEvent.KEYGROUP_FKEYS + 19;

	/**
	 * KEY_F21 constant.
	 */
	public final static int KEY_F21 = VCLEvent.KEYGROUP_FKEYS + 20;

	/**
	 * KEY_F22 constant.
	 */
	public final static int KEY_F22 = VCLEvent.KEYGROUP_FKEYS + 21;

	/**
	 * KEY_F23 constant.
	 */
	public final static int KEY_F23 = VCLEvent.KEYGROUP_FKEYS + 22;

	/**
	 * KEY_F24 constant.
	 */
	public final static int KEY_F24 = VCLEvent.KEYGROUP_FKEYS + 23;

	/**
	 * KEY_F25 constant.
	 */
	public final static int KEY_F25 = VCLEvent.KEYGROUP_FKEYS + 24;

	/**
	 * KEY_F26 constant.
	 */
	public final static int KEY_F26 = VCLEvent.KEYGROUP_FKEYS + 25;

	/**
	 * KEY_DOWN constant.
	 */
	public final static int KEY_DOWN = VCLEvent.KEYGROUP_CURSOR + 0;

	/**
	 * KEY_UP constant.
	 */
	public final static int KEY_UP = VCLEvent.KEYGROUP_CURSOR + 1;

	/**
	 * KEY_LEFT constant.
	 */
	public final static int KEY_LEFT = VCLEvent.KEYGROUP_CURSOR + 2;

	/**
	 * KEY_RIGHT constant.
	 */
	public final static int KEY_RIGHT = VCLEvent.KEYGROUP_CURSOR + 3;

	/**
	 * KEY_HOME constant.
	 */
	public final static int KEY_HOME = VCLEvent.KEYGROUP_CURSOR + 4;

	/**
	 * KEY_END constant.
	 */
	public final static int KEY_END = VCLEvent.KEYGROUP_CURSOR + 5;

	/**
	 * KEY_PAGEUP constant.
	 */
	public final static int KEY_PAGEUP = VCLEvent.KEYGROUP_CURSOR + 6;

	/**
	 * KEY_PAGEDOWN constant.
	 */
	public final static int KEY_PAGEDOWN = VCLEvent.KEYGROUP_CURSOR + 7;

	/**
	 * KEY_RETURN constant.
	 */
	public final static int KEY_RETURN = VCLEvent.KEYGROUP_MISC + 0;

	/**
	 * KEY_ESCAPE constant.
	 */
	public final static int KEY_ESCAPE = VCLEvent.KEYGROUP_MISC + 1;

	/**
	 * KEY_TAB constant.
	 */
	public final static int KEY_TAB = VCLEvent.KEYGROUP_MISC + 2;

	/**
	 * KEY_BACKSPACE constant.
	 */
	public final static int KEY_BACKSPACE = VCLEvent.KEYGROUP_MISC + 3;

	/**
	 * KEY_SPACE constant.
	 */
	public final static int KEY_SPACE = VCLEvent.KEYGROUP_MISC + 4;

	/**
	 * KEY_INSERT constant.
	 */
	public final static int KEY_INSERT = VCLEvent.KEYGROUP_MISC + 5;

	/**
	 * KEY_DELETE constant.
	 */
	public final static int KEY_DELETE = VCLEvent.KEYGROUP_MISC + 6;

	/**
	 * KEY_ADD constant.
	 */
	public final static int KEY_ADD = VCLEvent.KEYGROUP_MISC + 7;

	/**
	 * KEY_SUBTRACT constant.
	 */
	public final static int KEY_SUBTRACT = VCLEvent.KEYGROUP_MISC + 8;

	/**
	 * KEY_MULTIPLY constant.
	 */
	public final static int KEY_MULTIPLY = VCLEvent.KEYGROUP_MISC + 9;

	/**
	 * KEY_DIVIDE constant.
	 */
	public final static int KEY_DIVIDE = VCLEvent.KEYGROUP_MISC + 10;

	/**
	 * KEY_POINT constant.
	 */
	public final static int KEY_POINT = VCLEvent.KEYGROUP_MISC + 11;

	/**
	 * KEY_COMMA constant.
	 */
	public final static int KEY_COMMA = VCLEvent.KEYGROUP_MISC + 12;

	/**
	 * KEY_LESS constant.
	 */
	public final static int KEY_LESS = VCLEvent.KEYGROUP_MISC + 13;

	/**
	 * KEY_GREATER constant.
	 */
	public final static int KEY_GREATER = VCLEvent.KEYGROUP_MISC + 14;

	/**
	 * KEY_EQUAL constant.
	 */
	public final static int KEY_EQUAL = VCLEvent.KEYGROUP_MISC + 15;

	/**
	 * KEY_OPEN constant.
	 */
	public final static int KEY_OPEN = VCLEvent.KEYGROUP_MISC + 16;

	/**
	 * KEY_CUT constant.
	 */
	public final static int KEY_CUT = VCLEvent.KEYGROUP_MISC + 17;

	/**
	 * KEY_COPY constant.
	 */
	public final static int KEY_COPY = VCLEvent.KEYGROUP_MISC + 18;

	/**
	 * KEY_PASTE constant.
	 */
	public final static int KEY_PASTE = VCLEvent.KEYGROUP_MISC + 19;

	/**
	 * KEY_UNDO constant.
	 */
	public final static int KEY_UNDO = VCLEvent.KEYGROUP_MISC + 20;

	/**
	 * KEY_REPEAT constant.
	 */
	public final static int KEY_REPEAT = VCLEvent.KEYGROUP_MISC + 21;

	/**
	 * KEY_FIND constant.
	 */
	public final static int KEY_FIND = VCLEvent.KEYGROUP_MISC + 22;

	/**
	 * KEY_PROPERTIES constant.
	 */
	public final static int KEY_PROPERTIES = VCLEvent.KEYGROUP_MISC + 23;

	/**
	 * KEY_FRONT constant.
	 */
	public final static int KEY_FRONT = VCLEvent.KEYGROUP_MISC + 24;

	/**
	 * KEY_CONTEXTMENU constant.
	 */
	public final static int KEY_CONTEXTMENU = VCLEvent.KEYGROUP_MISC + 25;

	/**
	 * KEY_MENU constant.
	 */
	public final static int KEY_MENU = VCLEvent.KEYGROUP_MISC + 26;

	/**
	 * KEY_HELP constant.
	 */
	public final static int KEY_HELP = VCLEvent.KEYGROUP_MISC + 27;

	/**
	 * KEY_CODE constant.
	 */
	public final static int KEY_CODE = 0x0FFF;

	/**
	 * KEY_SHIFT constant.
	 */
	public final static int KEY_SHIFT = 0x1000;

	/**
	 * KEY_MOD1 constant.
	 */
	public final static int KEY_MOD1 = 0x2000;

	/**
	 * KEY_MOD2 constant.
	 */
	public final static int KEY_MOD2 = 0x4000;

	/**
	 * KEY_MODTYPE constant.
	 */
	public final static int KEY_MODTYPE = 0x7000;

	/**
	 * KEY_CONTROLMOD constant.
	 */
	public final static int KEY_CONTROLMOD = 0x8000;

	/**
	 * KEY_ALLMODTYPE constant.
	 */
	public final static int KEY_ALLMODTYPE = 0xf000;

	/**
	 * MOUSE_LEFT constant.
	 */
	public final static int MOUSE_LEFT = 0x1;

	/**
	 * MOUSE_MIDDLE constant.
	 */
	public final static int MOUSE_MIDDLE = 0x2;

	/**
	 * MOUSE_RIGHT constant.
	 */
	public final static int MOUSE_RIGHT = 0x4;

	/**
	 * SALEVENT_MOUSEMOVE constant.
	 */
	public final static int SALEVENT_MOUSEMOVE = 1;

	/**
	 * SALEVENT_MOUSELEAVE constant.
	 */
	public final static int SALEVENT_MOUSELEAVE = 2;

	/**
	 * SALEVENT_MOUSEBUTTONDOWN constant.
	 */
	public final static int SALEVENT_MOUSEBUTTONDOWN = 3;

	/**
	 * SALEVENT_MOUSEBUTTONUP constant.
	 */
	public final static int SALEVENT_MOUSEBUTTONUP = 4;

	/**
	 * SALEVENT_KEYINPUT constant.
	 */
	public final static int SALEVENT_KEYINPUT = 5;

	/**
	 * SALEVENT_KEYUP constant.
	 */
	public final static int SALEVENT_KEYUP = 6;

	/**
	 * SALEVENT_KEYMODCHANGE constant.
	 */
	public final static int SALEVENT_KEYMODCHANGE = 7;

	/**
	 * SALEVENT_PAINT constant.
	 */
	public final static int SALEVENT_PAINT = 8;

	/**
	 * SALEVENT_RESIZE constant.
	 */
	public final static int SALEVENT_RESIZE = 9;

	/**
	 * SALEVENT_GETFOCUS constant.
	 */
	public final static int SALEVENT_GETFOCUS = 10;

	/**
	 * SALEVENT_LOSEFOCUS constant.
	 */
	public final static int SALEVENT_LOSEFOCUS = 11;

	/**
	 * SALEVENT_CLOSE constant.
	 */
	public final static int SALEVENT_CLOSE = 12;

	/**
	 * SALEVENT_SHUTDOWN constant.
	 */
	public final static int SALEVENT_SHUTDOWN = 13;

	/**
	 * SALEVENT_SETTINGSCHANGED constant.
	 */
	public final static int SALEVENT_SETTINGSCHANGED = 14;

	/**
	 * SALEVENT_VOLUMECHANGED constant.
	 */
	public final static int SALEVENT_VOLUMECHANGED = 15;

	/**
	 * SALEVENT_PRINTERCHANGED constant.
	 */
	public final static int SALEVENT_PRINTERCHANGED = 16;

	/**
	 * SALEVENT_DISPLAYCHANGED constant.
	 */
	public final static int SALEVENT_DISPLAYCHANGED = 17;

	/**
	 * SALEVENT_FONTCHANGED constant.
	 */
	public final static int SALEVENT_FONTCHANGED = 18;

	/**
	 * SALEVENT_DATETIMECHANGED constant.
	 */
	public final static int SALEVENT_DATETIMECHANGED = 19;

	/**
	 * SALEVENT_KEYBOARDCHANGED constant.
	 */
	public final static int SALEVENT_KEYBOARDCHANGED = 20;

	/**
	 * SALEVENT_WHEELMOUSE constant.
	 */
	public final static int SALEVENT_WHEELMOUSE = 21;

	/**
	 * SALEVENT_USEREVENT constant.
	 */
	public final static int SALEVENT_USEREVENT = 22;

	/**
	 * SALEVENT_MOUSEACTIVATE constant.
	 */
	public final static int SALEVENT_MOUSEACTIVATE = 23;

	/**
	 * SALEVENT_EXTTEXTINPUT constant.
	 */
	public final static int SALEVENT_EXTTEXTINPUT = 24;

	/**
	 * SALEVENT_ENDEXTTEXTINPUT constant.
	 */
	public final static int SALEVENT_ENDEXTTEXTINPUT = 25;

	/**
	 * SALEVENT_EXTTEXTINPUTPOS constant.
	 */
	public final static int SALEVENT_EXTTEXTINPUTPOS = 26;

	/**
	 * SALEVENT_INPUTCONTEXTCHANGE constant.
	 */
	public final static int SALEVENT_INPUTCONTEXTCHANGE = 27;

	/**
	 * SALEVENT_MOVE constant.
	 */
	public final static int SALEVENT_MOVE = 28;

	/**
	 * SALEVENT_MOVERESIZE constant.
	 */
	public final static int SALEVENT_MOVERESIZE = 29;

	/**
	 * SALEVENT_CLOSEPOPUPS constant.
	 */
	public final static int SALEVENT_CLOSEPOPUPS = 30;

	/**
	 * SALEVENT_EXTERNALKEYINPUT constant.
	 */
	public final static int SALEVENT_EXTERNALKEYINPUT = 31;

	/**
	 * SALEVENT_EXTERNALKEYUP constant.
	 */
	public final static int SALEVENT_EXTERNALKEYUP = 32;

	/**
	 * SALEVENT_MENUCOMMAND constant.
	 */
	public final static int SALEVENT_MENUCOMMAND = 33;

	/**
	 * SALEVENT_MENUHIGHLIGHT constant.
	 */
	public final static int SALEVENT_MENUHIGHLIGHT = 34;

	/**
	 * SALEVENT_MENUACTIVATE constant.
	 */
	public final static int SALEVENT_MENUACTIVATE = 35;

	/**
	 * SALEVENT_MENUDEACTIVATE constant.
	 */
	public final static int SALEVENT_MENUDEACTIVATE = 36;

	/**
	 * SALEVENT_COUNT constant.
	 */
	public final static int SALEVENT_COUNT = 36;

	/**
	 * SALEVENT_OPENDOCUMENT constant.
	 */
	public final static int SALEVENT_OPENDOCUMENT = 100;

	/**
	 * SALEVENT_PRINTDOCUMENT constant.
	 */
	public final static int SALEVENT_PRINTDOCUMENT = 101;

	/**
	 * SALEVENT_ABOUT constant.
	 */
	public final static int SALEVENT_ABOUT = 130;
	
	/**
	 * SALEEVENT_PREFS constant
	 */
	public final static int SALEVENT_PREFS = 140;

	/**
	 * SAL_EXTTEXTINPUT_ATTR_GRAYWAVELINE constant.
	 */
	public final static int SAL_EXTTEXTINPUT_ATTR_GRAYWAVELINE = 0x100;

	/**
	 * SAL_EXTTEXTINPUT_ATTR_UNDERLINE constant.
	 */
	public final static int SAL_EXTTEXTINPUT_ATTR_UNDERLINE = 0x200;

	/**
	 * SAL_EXTTEXTINPUT_ATTR_BOLDUNDERLINE constant.
	 */
	public final static int SAL_EXTTEXTINPUT_ATTR_BOLDUNDERLINE = 0x400;

	/**
	 * SAL_EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE constant.
	 */
	public final static int SAL_EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE = 0x800;

	/**
	 * SAL_EXTTEXTINPUT_ATTR_DASHDOTUNDERLINE constant.
	 */
	public final static int SAL_EXTTEXTINPUT_ATTR_DASHDOTUNDERLINE = 0x1000;

	/**
	 * SAL_EXTTEXTINPUT_ATTR_HIGHLIGHT constant.
	 */
	public final static int SAL_EXTTEXTINPUT_ATTR_HIGHLIGHT = 0x2000;

	/**
	 * SAL_EXTTEXTINPUT_ATTR_REDTEXT constant.
	 */
	public final static int SAL_EXTTEXTINPUT_ATTR_REDTEXT = 0x4000;

	/**
	 * SAL_EXTTEXTINPUT_ATTR_HALFTONETEXT constant.
	 */
	public final static int SAL_EXTTEXTINPUT_ATTR_HALFTONETEXT = 0x8000;

	/**
	 * The AWT event flag.
	 */
	private boolean awtEvent = false;

	/**
	 * The data pointer.
	 */
	private int data = 0;

	/**
	 * The frame pointer.
	 */
	private int frame = 0;

	/**
	 * The cached key character.
	 */
	private char keyChar = KeyEvent.CHAR_UNDEFINED;

	/**
	 * The cached key code.
	 */
	private int keyCode = -1;

	/**
	 * The cached modifiers pressed.
	 */
	private int modifiers = 0;

	/**
	 * The document path.
	 */
	private String path = null;

	/**
	 * The repeat count.
	 */
	private short repeatCount = 0;

	/**
	 * The shutdown cancelled flag.
	 */
	private boolean shutdownCancelled = false;

	/**
	 * The text.
	 */
	private String text = null;

	/**
	 * The text attributes.
	 */
	private int[] textAttributes = null;

	/**
	 * The menu item ID.
	 */
	private short menuID=0;

	/**
	 * The C++ object Menu pointer
	 */
	private int menuCookie=0;

	/**
	 * Constructs a new <code>VCLEvent</code> instance.
	 *
	 * @param id the event type
	 * @param f the <code>VCLFrame</code> instance
	 * @param d the data pointer
	 */
	public VCLEvent(int id, VCLFrame f, int d) {

		super(new Object(), id);
		if (f != null)
			frame = f.getFrame();
		data = d;

	}

	/**
	 * Constructs a new <code>VCLEvent</code> instance.  This constructor
	 * should be used only for injecting SALEVENT_MENU* events into the queue.
	 *
	 * @param id the event type
	 * @param mID the menu id
	 * @param mCookie the menu pointer
	 */
	public VCLEvent(int id, VCLFrame f, short mID, int mCookie) {
	    
		super(new Object(), id);
		if (f != null)
			frame = f.getFrame();
		menuID = mID;
		menuCookie = mCookie;

	}

	/**
	 * Constructs a new <code>VCLEvent</code> instance.
	 *
	 * @param id the event type
	 * @param f the <code>VCLFrame</code> instance
	 * @param d the data pointer
	 * @param p the document path
	 */
	public VCLEvent(int id, VCLFrame f, int d, String p) {

		this(id, f, d);
		path = p;

	}

	/**
	 * Constructs a new <code>VCLEvent</code> instance.
	 *
	 * @param source the <code>AWTEvent</code> that originated the event
	 * @param id the event type
	 * @param f the <code>VCLFrame</code> instance
	 * @param d the data pointer
	 */
	VCLEvent(AWTEvent event, int id, VCLFrame f, int d) {

		this(id, f, d);

		awtEvent = true;

		if (event instanceof InputEvent)
		{
			InputEvent e = (InputEvent)event;
			int awtModifiers = e.getModifiers() | e.getModifiersEx();
			int eid = e.getID();

			// Treat the Mac OS X command key as a control key and the control
			// key as the meta key
			if (awtModifiers != 0 && event instanceof KeyEvent)
			{
				if ((awtModifiers & (InputEvent.CTRL_DOWN_MASK | InputEvent.META_DOWN_MASK)) == (InputEvent.CTRL_DOWN_MASK | InputEvent.META_DOWN_MASK)) {
					; // No switching is needed
				}
				else if ((awtModifiers & InputEvent.CTRL_DOWN_MASK) != 0) {
					KeyEvent ke = (KeyEvent)e;
					int keyCode = ke.getKeyCode();
					if (keyCode == KeyEvent.VK_CONTROL) {
						keyCode = KeyEvent.VK_META;
						event = e = new KeyEvent(e.getComponent(), eid, e.getWhen(), awtModifiers, keyCode, ke.getKeyChar());
					}
				}
				else if ((awtModifiers & InputEvent.META_DOWN_MASK) != 0) {
					KeyEvent ke = (KeyEvent)e;
					int keyCode = ke.getKeyCode();
					if (keyCode == KeyEvent.VK_META) {
						keyCode = KeyEvent.VK_CONTROL;
						event = e = new KeyEvent(e.getComponent(), eid, e.getWhen(), awtModifiers, keyCode, ke.getKeyChar());
					}
				}
			}

			if (awtModifiers != 0)
			{
				if ((awtModifiers & InputEvent.BUTTON1_DOWN_MASK) != 0)
					modifiers |= VCLEvent.MOUSE_LEFT;
				if ((awtModifiers & InputEvent.BUTTON2_DOWN_MASK) != 0)
					modifiers |= VCLEvent.MOUSE_MIDDLE;
				if ((awtModifiers & InputEvent.BUTTON3_DOWN_MASK) != 0)
					modifiers |= VCLEvent.MOUSE_RIGHT;
				if ((awtModifiers & InputEvent.META_DOWN_MASK) != 0)
					modifiers |= VCLEvent.KEY_MOD1;
				if ((awtModifiers & InputEvent.ALT_DOWN_MASK) != 0)
					modifiers |= VCLEvent.KEY_MOD2;
				if ((awtModifiers & InputEvent.SHIFT_DOWN_MASK) != 0)
					modifiers |= VCLEvent.KEY_SHIFT;
				if ((awtModifiers & InputEvent.CTRL_DOWN_MASK) != 0)
					modifiers |= VCLEvent.KEY_CONTROLMOD;

				// If command plus left or middle button is pressed, Java
				// will add the right button so we need to strip it out
				if ((modifiers & VCLEvent.MOUSE_RIGHT) != 0 && (modifiers & VCLEvent.KEY_MOD1) != 0 && (modifiers & (VCLEvent.MOUSE_LEFT | VCLEvent.MOUSE_MIDDLE)) != 0)
					modifiers &= ~VCLEvent.MOUSE_RIGHT;

				if ((modifiers & VCLEvent.MOUSE_LEFT) != 0) {
					// Convert control plus left button events to right button
					// events since one button mice have no right button
					if ((modifiers & VCLEvent.KEY_CONTROLMOD) != 0)
						modifiers = (modifiers & ~(VCLEvent.KEY_CONTROLMOD | VCLEvent.MOUSE_LEFT)) | VCLEvent.MOUSE_RIGHT;

					// Convert alt plus left button events to middle button
					// events since one button mice have no middle button
					if ((modifiers & VCLEvent.KEY_MOD2) != 0)
						modifiers = (modifiers & ~(VCLEvent.KEY_MOD2 | VCLEvent.MOUSE_LEFT)) | VCLEvent.MOUSE_MIDDLE;
				}
			}
		}
		else if (event instanceof InputMethodEvent) {
			AttributedCharacterIterator i = ((InputMethodEvent)event).getText();
			if (i != null) {
				StringBuffer buf = new StringBuffer();
				int count = 0;
				for (char c = i.first(); c != CharacterIterator.DONE; c = i.next()) {
					buf.append(c);
					count++;
				}
				text = buf.toString();

				int committedCount = getCommittedCharacterCount();
				if (count > committedCount)
				{
					textAttributes = new int[count];
					count = 0;
					boolean selected = false;
					for (char c = i.first(); c != CharacterIterator.DONE; c = i.next()) {
						int attribute = 0;
						InputMethodHighlight hl = (InputMethodHighlight)i.getAttribute(TextAttribute.INPUT_METHOD_HIGHLIGHT);
						if (hl != null) {
							if (hl.isSelected()) {
								if (!selected) {
									// When we encounter the first selected
									// character, reset the preceding characters
									selected = true;
									for (int j = 0; j < count; j++)
										textAttributes[j] = SAL_EXTTEXTINPUT_ATTR_UNDERLINE;
								}
								attribute = SAL_EXTTEXTINPUT_ATTR_HIGHLIGHT;
							}
							else if (!selected) {
								// If no characters are selected, highlight
								// all of them
								attribute = SAL_EXTTEXTINPUT_ATTR_HIGHLIGHT;
							}
							else {
								attribute = SAL_EXTTEXTINPUT_ATTR_UNDERLINE;
							}
						}
						else if (count >= committedCount) {
							// Fix bug 1190 by highlighting uncommitted
							// characters
							attribute = SAL_EXTTEXTINPUT_ATTR_HIGHLIGHT;
						}
						textAttributes[count++] = attribute;
					}
				}
			}
		}

		source = event;

	}

	/**
	 * Adds the specified repeat count.
	 *
	 * @param r the repeat count to add
	 */
	void addRepeatCount(short r) {

		repeatCount += r;

	}

	/**
	 * Adds the specified amount to the mouse wheel rotation
	 *
	 * @param r the mouse wheel rotation to add
	 */
	void addWheelRotation(int r) {

		if (source instanceof MouseWheelEvent) {
			MouseWheelEvent e = (MouseWheelEvent)source;
			source = new MouseWheelEvent(e.getComponent(), e.getID(), e.getWhen(), e.getModifiers() | e.getModifiersEx(), e.getX(), e.getY(), e.getClickCount(), e.isPopupTrigger(), e.getScrollType(), e.getScrollAmount(), e.getWheelRotation() + r);
		}

	}

	/**
	 * Sets the shutdown cancelled status.
	 */
	public synchronized void cancelShutdown() {

		if (getID() == SALEVENT_SHUTDOWN)
			shutdownCancelled = true;

	}

    /**
	/**
	 * Returns the number of committed characters in the text associated with
	 * this event.
	 *
	 * @return the number of committed characters
	 */
	public int getCommittedCharacterCount() {

		if (source instanceof InputMethodEvent)
			return ((InputMethodEvent)source).getCommittedCharacterCount();
		else
			return 0;

	}

	/**
	 * Gets the cursor position for input method events.
	 *
	 * @return the cursor position for input method events
	 */
	public int getCursorPosition() {

		if (source instanceof InputMethodEvent) {
			TextHitInfo hi = ((InputMethodEvent)source).getCaret();
			return (hi != null ? hi.getInsertionIndex() : 0);
		}
		else {
			return 0;
		}

	}

	/**
	 * Gets the data pointer.
	 *
	 * @return the data pointer.
	 */
	public int getData() {

		return data;

	}

	/**
	 * Gets the frame pointer.
	 *
	 * @return the frame pointer.
	 */
	public int getFrame() {

		return frame;

	}

	/**
	 * Returns the character associated with the key in this event.
	 *
	 * @return the Unicode character
	 */
	public char getKeyChar() {

		if (keyChar == KeyEvent.CHAR_UNDEFINED)
		{
			keyChar = 0;
			if (source instanceof KeyEvent) {
				KeyEvent e = (KeyEvent)source;
				keyChar = e.getKeyChar();
				if (keyChar == KeyEvent.CHAR_UNDEFINED)
					keyChar = 0;
				// The C++ code expects that Ctrl-key events will have the key
				// char resolved to their respective ASCII equivalents. Since
				// we convert Mac OS X Meta-key events into Ctrl-key events,
				// we need to do the resolving manually.
				if ((e.getModifiersEx() & InputEvent.CTRL_DOWN_MASK) != 0 && keyChar >= 'a' && keyChar <= 0x7d )
					keyChar -= 0x60;
			}
		}
		return keyChar;
	
	}
	
	/**
	 * Returns the code associated with the key in this event.
	 *
	 * @return the key code
	 */
	public int getKeyCode() {

		if (keyCode < 0) {
			keyCode = 0;
			if (source instanceof KeyEvent) {
				switch (((KeyEvent)source).getKeyCode()) {
					case KeyEvent.VK_UNDEFINED:
						char keyChar = ((KeyEvent)source).getKeyChar();
						if (keyChar >= '0' && keyChar <= '9')
							keyCode = VCLEvent.KEYGROUP_NUM + keyChar - '0';
						else if (keyChar >= 'A' && keyChar <= 'Z')
							keyCode = VCLEvent.KEYGROUP_ALPHA + keyChar - 'A';
						else if (keyChar >= 'a' && keyChar <= 'z')
							keyCode = VCLEvent.KEYGROUP_ALPHA + keyChar - 'a';
						else if (keyChar == 0x08)
							keyCode = VCLEvent.KEY_BACKSPACE;
						else if (keyChar == 0x09)
							keyCode = VCLEvent.KEY_TAB;
						else if (keyChar == 0x03 || keyChar == 0x0A || keyChar == 0x0D)
							keyCode = VCLEvent.KEY_RETURN;
						else if (keyChar == 0x1B)
							keyCode = VCLEvent.KEY_ESCAPE;
						else if (keyChar == 0x20)
							keyCode = VCLEvent.KEY_SPACE;
						else if (keyChar == 0x7f)
							keyCode = VCLEvent.KEY_DELETE;
						else
							keyCode = 0;
						break;
					case KeyEvent.VK_ENTER:
						keyCode = VCLEvent.KEY_RETURN;
					break;
					case KeyEvent.VK_BACK_SPACE:
						keyCode = VCLEvent.KEY_BACKSPACE;
						break;
					case KeyEvent.VK_TAB:
						keyCode = VCLEvent.KEY_TAB;
						break;
					case KeyEvent.VK_SHIFT:
						keyCode = VCLEvent.KEY_SHIFT;
						break;
					case KeyEvent.VK_CONTROL:
						keyCode = VCLEvent.KEY_MOD1;
						break;
					case KeyEvent.VK_ALT:
						keyCode = VCLEvent.KEY_MOD2;
						break;
					case KeyEvent.VK_META:
						keyCode = VCLEvent.KEY_CONTROLMOD;
						break;
					case KeyEvent.VK_ESCAPE:
						keyCode = VCLEvent.KEY_ESCAPE;
						break;
					case KeyEvent.VK_SPACE:
						keyCode = VCLEvent.KEY_SPACE;
						break;
					case KeyEvent.VK_PAGE_UP:
						keyCode = VCLEvent.KEY_PAGEUP;
						break;
					case KeyEvent.VK_PAGE_DOWN:
						keyCode = VCLEvent.KEY_PAGEDOWN;
						break;
					case KeyEvent.VK_END:
						keyCode = VCLEvent.KEY_END;
						break;
					case KeyEvent.VK_HOME:
						keyCode = VCLEvent.KEY_HOME;
						break;
					case KeyEvent.VK_LEFT:
						keyCode = VCLEvent.KEY_LEFT;
						break;
					case KeyEvent.VK_UP:
						keyCode = VCLEvent.KEY_UP;
						break;
					case KeyEvent.VK_RIGHT:
						keyCode = VCLEvent.KEY_RIGHT;
						break;
					case KeyEvent.VK_DOWN:
						keyCode = VCLEvent.KEY_DOWN;
						break;
					case KeyEvent.VK_COMMA:
						keyCode = VCLEvent.KEY_COMMA;
						break;
					case KeyEvent.VK_PERIOD:
						keyCode = VCLEvent.KEY_POINT;
						break;
					case KeyEvent.VK_0:
						keyCode = VCLEvent.KEY_0;
						break;
					case KeyEvent.VK_1:
						keyCode = VCLEvent.KEY_1;
						break;
					case KeyEvent.VK_2:
						keyCode = VCLEvent.KEY_2;
						break;
					case KeyEvent.VK_3:
						keyCode = VCLEvent.KEY_3;
						break;
					case KeyEvent.VK_4:
						keyCode = VCLEvent.KEY_4;
						break;
					case KeyEvent.VK_5:
						keyCode = VCLEvent.KEY_5;
						break;
					case KeyEvent.VK_6:
						keyCode = VCLEvent.KEY_6;
						break;
					case KeyEvent.VK_7:
						keyCode = VCLEvent.KEY_7;
						break;
					case KeyEvent.VK_8:
						keyCode = VCLEvent.KEY_8;
						break;
					case KeyEvent.VK_9:
						keyCode = VCLEvent.KEY_9;
						break;
					case KeyEvent.VK_EQUALS:
						keyCode = VCLEvent.KEY_EQUAL;
						break;
					case KeyEvent.VK_A:
						keyCode = VCLEvent.KEY_A;
						break;
					case KeyEvent.VK_B:
						keyCode = VCLEvent.KEY_B;
						break;
					case KeyEvent.VK_C:
						keyCode = VCLEvent.KEY_C;
						break;
					case KeyEvent.VK_D:
						keyCode = VCLEvent.KEY_D;
						break;
					case KeyEvent.VK_E:
						keyCode = VCLEvent.KEY_E;
						break;
					case KeyEvent.VK_F:
						keyCode = VCLEvent.KEY_F;
						break;
					case KeyEvent.VK_G:
						keyCode = VCLEvent.KEY_G;
						break;
					case KeyEvent.VK_H:
						keyCode = VCLEvent.KEY_H;
						break;
					case KeyEvent.VK_I:
						keyCode = VCLEvent.KEY_I;
						break;
					case KeyEvent.VK_J:
						keyCode = VCLEvent.KEY_J;
						break;
					case KeyEvent.VK_K:
						keyCode = VCLEvent.KEY_K;
						break;
					case KeyEvent.VK_L:
						keyCode = VCLEvent.KEY_L;
						break;
					case KeyEvent.VK_M:
						keyCode = VCLEvent.KEY_M;
						break;
					case KeyEvent.VK_N:
						keyCode = VCLEvent.KEY_N;
						break;
					case KeyEvent.VK_O:
						keyCode = VCLEvent.KEY_O;
						break;
					case KeyEvent.VK_P:
						keyCode = VCLEvent.KEY_P;
						break;
					case KeyEvent.VK_Q:
						keyCode = VCLEvent.KEY_Q;
						break;
					case KeyEvent.VK_R:
						keyCode = VCLEvent.KEY_R;
						break;
					case KeyEvent.VK_S:
						keyCode = VCLEvent.KEY_S;
						break;
					case KeyEvent.VK_T:
						keyCode = VCLEvent.KEY_T;
						break;
					case KeyEvent.VK_U:
						keyCode = VCLEvent.KEY_U;
						break;
					case KeyEvent.VK_V:
						keyCode = VCLEvent.KEY_V;
						break;
					case KeyEvent.VK_W:
						keyCode = VCLEvent.KEY_W;
						break;
					case KeyEvent.VK_X:
						keyCode = VCLEvent.KEY_X;
						break;
					case KeyEvent.VK_Y:
						keyCode = VCLEvent.KEY_Y;
						break;
					case KeyEvent.VK_Z:
						keyCode = VCLEvent.KEY_Z;
						break;
					case KeyEvent.VK_MULTIPLY:
						keyCode = VCLEvent.KEY_MULTIPLY;
						break;
					case KeyEvent.VK_ADD:
						keyCode = VCLEvent.KEY_ADD;
						break;
					case KeyEvent.VK_SUBTRACT:
						keyCode = VCLEvent.KEY_SUBTRACT;
						break;
					case KeyEvent.VK_DIVIDE:
						keyCode = VCLEvent.KEY_DIVIDE;
						break;
					case KeyEvent.VK_DELETE:
						keyCode = VCLEvent.KEY_DELETE;
						break;
					case KeyEvent.VK_F1:
						keyCode = VCLEvent.KEY_F1;
						break;
					case KeyEvent.VK_F2:
						keyCode = VCLEvent.KEY_F2;
						break;
					case KeyEvent.VK_F3:
						keyCode = VCLEvent.KEY_F3;
						break;
					case KeyEvent.VK_F4:
						keyCode = VCLEvent.KEY_F4;
						break;
					case KeyEvent.VK_F5:
						keyCode = VCLEvent.KEY_F5;
						break;
					case KeyEvent.VK_F6:
						keyCode = VCLEvent.KEY_F6;
						break;
					case KeyEvent.VK_F7:
						keyCode = VCLEvent.KEY_F7;
						break;
					case KeyEvent.VK_F8:
						keyCode = VCLEvent.KEY_F8;
						break;
					case KeyEvent.VK_F9:
						keyCode = VCLEvent.KEY_F9;
						break;
					case KeyEvent.VK_F10:
						keyCode = VCLEvent.KEY_F10;
						break;
					case KeyEvent.VK_F11:
						keyCode = VCLEvent.KEY_F11;
						break;
					case KeyEvent.VK_F12:
						keyCode = VCLEvent.KEY_F12;
						break;
					case KeyEvent.VK_F13:
						keyCode = VCLEvent.KEY_F13;
						break;
					case KeyEvent.VK_F14:
						keyCode = VCLEvent.KEY_F14;
						break;
					case KeyEvent.VK_F15:
						keyCode = VCLEvent.KEY_F15;
						break;
					case KeyEvent.VK_F16:
						keyCode = VCLEvent.KEY_F16;
						break;
					case KeyEvent.VK_F17:
						keyCode = VCLEvent.KEY_F17;
						break;
					case KeyEvent.VK_F18:
						keyCode = VCLEvent.KEY_F18;
						break;
					case KeyEvent.VK_F19:
						keyCode = VCLEvent.KEY_F19;
						break;
					case KeyEvent.VK_F20:
						keyCode = VCLEvent.KEY_F20;
						break;
					case KeyEvent.VK_F21:
						keyCode = VCLEvent.KEY_F21;
						break;
					case KeyEvent.VK_F22:
						keyCode = VCLEvent.KEY_F22;
						break;
					case KeyEvent.VK_F23:
						keyCode = VCLEvent.KEY_F23;
						break;
					case KeyEvent.VK_F24:
						keyCode = VCLEvent.KEY_F24;
						break;
					case KeyEvent.VK_INSERT:
						keyCode = VCLEvent.KEY_INSERT;
						break;
					case KeyEvent.VK_HELP:
						keyCode = VCLEvent.KEY_HELP;
						break;
					case KeyEvent.VK_LESS:
						keyCode = VCLEvent.KEY_LESS;
						break;
					case KeyEvent.VK_GREATER:
						keyCode = VCLEvent.KEY_GREATER;
						break;
					case KeyEvent.VK_CUT:
						keyCode = VCLEvent.KEY_CUT;
						break;
					case KeyEvent.VK_COPY:
						keyCode = VCLEvent.KEY_COPY;
						break;
					case KeyEvent.VK_PASTE:
						keyCode = VCLEvent.KEY_PASTE;
						break;
					case KeyEvent.VK_UNDO:
						keyCode = VCLEvent.KEY_UNDO;
						break;
					case KeyEvent.VK_AGAIN:
						keyCode = VCLEvent.KEY_REPEAT;
						break;
					case KeyEvent.VK_FIND:
						keyCode = VCLEvent.KEY_FIND;
						break;
					case KeyEvent.VK_PROPS:
						keyCode = VCLEvent.KEY_PROPERTIES;
						break;
					default:
						keyCode = 0;
						break;
				}
			}
		}
		return keyCode;

	}

	/**
	 * Returns the key and mouse modifiers.
	 *
	 * @return the key and mouse modifiers
	 */
	public int getModifiers() {

		return modifiers;

	}

	/**
	 * Returns the path of the document associated with this event.
	 *
	 * @return the rectangle to be repainted
	 */
	public String getPath() {

		return path;

	}

	/**
	 * Returns the repeat count.
	 *
	 * @return the repeat count
	 */
	public short getRepeatCount() {

		return repeatCount;

	}

	/**
	 * Returns the text associated with this event.
	 *
	 * @return the text to be drawn
	 */
	public String getText() {

		return text;

	}

	/**
	 * Returns the text attributes associated with this event.
	 *
	 * @return the text attributes to be used
	 */
	public int[] getTextAttributes() {

		return textAttributes;

	}

	/**
	 * Returns the menu ID associated with this event
	 *
	 * @return menu ID used on AWT menu item creation
	 */
	public short getMenuID() {
	    
	    return menuID;

	}

	/**
	 * Returns the menu cookie associated with this event. This should acutally
	 * be a pointer to an underlying abstract Menu C++ instance.
	 *
	 * @return cookie provided by VCL on AWT menu item creation
	 */
	public int getMenuCookie() {
	    
	    return menuCookie;

	}

	/**
	 * Gets the mouse wheel scroll amount.
	 *
	 * @return the mouse wheel scroll amount
	 */
	public int getScrollAmount() {

		if (source instanceof MouseWheelEvent)
			return ((MouseWheelEvent)source).getScrollAmount();
		else
			return 0;

	}

	/**
	 * Returns the rectangle representing the area which needs to be repainted
	 * in response to this event.
	 *
	 * @return the rectangle to be repainted
	 */
	public Rectangle getUpdateRect() {

		if (source instanceof PaintEvent)
			return ((PaintEvent)source).getUpdateRect();
		else
			return new Rectangle();

	}

	/**
	 * Gets the visible position for input method events.
	 *
	 * @return the visible position for input method events
	 */
	public int getVisiblePosition() {

		if (source instanceof InputMethodEvent) {
			TextHitInfo hi = ((InputMethodEvent)source).getVisiblePosition();
			return (hi != null ? hi.getInsertionIndex() : 0);
		}
		else {
			return 0;
		}

	}

	/**
	 * Gets the mouse wheel rotation.
	 *
	 * @return the mouse wheel rotation
	 */
	public int getWheelRotation() {

		if (source instanceof MouseWheelEvent)
			return ((MouseWheelEvent)source).getWheelRotation() * -1;
		else
			return 0;

	}

	/**
	 * Returns the timestamp of when this event occurred.
	 *
	 * @return the timestamp of when this event occurred
	 */
	public long getWhen() {

		if (source instanceof InputEvent)
			return ((InputEvent)source).getWhen();
		else
			return 0;

	}

	/**
	 * Returns the horizontal <code>x</code> position of the event relative to
	 * the source component.
	 *
	 * @return the horizontal position relative to the component
	 */
	public int getX() {

		if (source instanceof MouseEvent)
			return ((MouseEvent)source).getX();
		else
			return 0;

	}

	/**
	 * Returns the vertical <code>y</code> position of the event relative to
	 * the source component.
	 *
	 * @return the vertical position relative to the component
	 */
	public int getY() {

		if (source instanceof MouseEvent)
			return ((MouseEvent)source).getY();
		else
			return 0;

	}

	/**
	 * Return whether or not this event was generated by an
	 * <code>AWTEvent</code>.
	 *
	 * @return <code>true</code> if the the event was generated by an
	 *  <code>AWTEvent</code> or else <code>false</code>
	 */
	boolean isAWTEvent() {

		return awtEvent;

	}

	/**
	 * Returns the shutdown cancelled status.
	 *
	 * @return <code>true</code> if this is a SALEVENT_SHUTDOWN event and
	 *  the {@link #cancelShutdown()} method has been called or else
	 *  <code>false</code>
	 */
	public synchronized boolean isShutdownCancelled() {

		return shutdownCancelled;

	}

    /**
     * Returns a parameter string identifying this event.
     * This method is useful for event-logging and for debugging.
     *
     * @return a string identifying the event and its attributes
     */
    public String paramString() {

		String typeStr;
		switch (id) {
			case SALEVENT_MOUSEMOVE:
				typeStr = "SALEVENT_MOUSEMOVE";
				break;
			case SALEVENT_MOUSELEAVE:
				typeStr = "SALEVENT_MOUSELEAVE";
				break;
			case SALEVENT_MOUSEBUTTONDOWN:
				typeStr = "SALEVENT_MOUSEBUTTONDOWN";
				break;
			case SALEVENT_MOUSEBUTTONUP:
				typeStr = "SALEVENT_MOUSEBUTTONUP";
				break;
			case SALEVENT_KEYINPUT:
				typeStr = "SALEVENT_KEYINPUT";
				break;
			case SALEVENT_KEYUP:
				typeStr = "SALEVENT_KEYUP";
				break;
			case SALEVENT_KEYMODCHANGE:
				typeStr = "SALEVENT_KEYMODCHANGE";
				break;
			case SALEVENT_PAINT:
				typeStr = "SALEVENT_PAINT";
				break;
			case SALEVENT_RESIZE:
				typeStr = "SALEVENT_RESIZE";
				break;
			case SALEVENT_GETFOCUS:
				typeStr = "SALEVENT_GETFOCUS";
				break;
			case SALEVENT_LOSEFOCUS:
				typeStr = "SALEVENT_LOSEFOCUS";
				break;
			case SALEVENT_CLOSE:
				typeStr = "SALEVENT_CLOSE";
				break;
			case SALEVENT_SHUTDOWN:
				typeStr = "SALEVENT_SHUTDOWN";
				break;
			case SALEVENT_SETTINGSCHANGED:
				typeStr = "SALEVENT_SETTINGSCHANGED";
				break;
			case SALEVENT_VOLUMECHANGED:
				typeStr = "SALEVENT_VOLUMECHANGED";
				break;
			case SALEVENT_PRINTERCHANGED:
				typeStr = "SALEVENT_PRINTERCHANGED";
				break;
			case SALEVENT_DISPLAYCHANGED:
				typeStr = "SALEVENT_DISPLAYCHANGED";
				break;
			case SALEVENT_FONTCHANGED:
				typeStr = "SALEVENT_FONTCHANGED";
				break;
			case SALEVENT_DATETIMECHANGED:
				typeStr = "SALEVENT_DATETIMECHANGED";
				break;
			case SALEVENT_KEYBOARDCHANGED:
				typeStr = "SALEVENT_KEYBOARDCHANGED";
				break;
			case SALEVENT_WHEELMOUSE:
				typeStr = "SALEVENT_WHEELMOUSE";
				break;
			case SALEVENT_USEREVENT:
				typeStr = "SALEVENT_USEREVENT";
				break;
			case SALEVENT_MOUSEACTIVATE:
				typeStr = "SALEVENT_MOUSEACTIVATE";
				break;
			case SALEVENT_EXTTEXTINPUT:
				typeStr = "SALEVENT_EXTTEXTINPUT";
				break;
			case SALEVENT_ENDEXTTEXTINPUT:
				typeStr = "SALEVENT_ENDEXTTEXTINPUT";
				break;
			case SALEVENT_EXTTEXTINPUTPOS:
				typeStr = "SALEVENT_EXTTEXTINPUTPOS";
				break;
			case SALEVENT_INPUTCONTEXTCHANGE:
				typeStr = "SALEVENT_INPUTCONTEXTCHANGE";
				break;
			case SALEVENT_MOVE:
				typeStr = "SALEVENT_MOVE";
				break;
			case SALEVENT_MOVERESIZE:
				typeStr = "SALEVENT_MOVERESIZE";
				break;
			case SALEVENT_CLOSEPOPUPS:
				typeStr = "SALEVENT_CLOSEPOPUPS";
				break;
			case SALEVENT_EXTERNALKEYINPUT:
				typeStr = "SALEVENT_EXTERNALKEYINPUT";
				break;
			case SALEVENT_EXTERNALKEYUP:
				typeStr = "SALEVENT_EXTERNALKEYUP";
				break;
			case SALEVENT_MENUCOMMAND:
				typeStr = "SALEVENT_MENUCOMMAND";
				break;
			case SALEVENT_MENUHIGHLIGHT:
				typeStr = "SALEVENT_MENUHIGHLIGHT";
				break;
			case SALEVENT_MENUACTIVATE:
				typeStr = "SALEVENT_MENUACTIVATE";
				break;
			case SALEVENT_MENUDEACTIVATE:
				typeStr = "SALEVENT_MENUDEACTIVATE";
				break;
			case SALEVENT_OPENDOCUMENT:
				typeStr = "SALEVENT_OPENDOCUMENT";
				break;
			case SALEVENT_PRINTDOCUMENT:
				typeStr = "SALEVENT_PRINTDOCUMENT";
				break;
			case SALEVENT_ABOUT:
				typeStr = "SALEVENT_ABOUT";
				break;
			case SALEVENT_PREFS:
				typeStr = "SALEVENT_PREFS";
				break;
			default:
				typeStr = "unknown type";
		}
		return typeStr;

	}

	/**
	 * Sets the rectangle representing the area which needs to be repainted
	 * in response to this event.
	 *
	 * @param r the rectangle to be repainted
	 */
	void setUpdateRect(Rectangle r) {

		if (source instanceof PaintEvent)
			((PaintEvent)source).setUpdateRect(r);

	}

	/**
	 * Returns a string representation of the event.
	 *
	 * @return a string representation of the event
	 */
	public String toString() {

		if (source instanceof AWTEvent)
			return source.toString();
		else
			return super.toString();

	}

	/**
	 * Convert a VCL keycode into a Java keycode.
	 *
	 * @param keyCode the VCL keycode
	 * @return a Java keycode or 0 if there is no corresponding Java keycode
	 */
	public static int convertVCLKeyCode(int keyCode) {

		int outCode = 0;
		switch (keyCode & VCLEvent.KEY_CODE) {
			case VCLEvent.KEY_0:
				outCode = KeyEvent.VK_0;
				break;
			case VCLEvent.KEY_1:
				outCode = KeyEvent.VK_1;
				break;
			case VCLEvent.KEY_2:
				outCode = KeyEvent.VK_2;
				break;
			case VCLEvent.KEY_3:
				outCode = KeyEvent.VK_3;
				break;
			case VCLEvent.KEY_4:
				outCode = KeyEvent.VK_4;
				break;
			case VCLEvent.KEY_5:
				outCode = KeyEvent.VK_5;
				break;
			case VCLEvent.KEY_6:
				outCode = KeyEvent.VK_6;
				break;
			case VCLEvent.KEY_7:
				outCode = KeyEvent.VK_7;
				break;
			case VCLEvent.KEY_8:
				outCode = KeyEvent.VK_8;
				break;
			case VCLEvent.KEY_9:
				outCode = KeyEvent.VK_9;
				break;
			case VCLEvent.KEY_A:
				outCode = KeyEvent.VK_A;
				break;
			case VCLEvent.KEY_B:
				outCode = KeyEvent.VK_B;
				break;
			case VCLEvent.KEY_C:
				outCode = KeyEvent.VK_C;
				break;
			case VCLEvent.KEY_D:
				outCode = KeyEvent.VK_D;
				break;
			case VCLEvent.KEY_E:
				outCode = KeyEvent.VK_E;
				break;
			case VCLEvent.KEY_F:
				outCode = KeyEvent.VK_F;
				break;
			case VCLEvent.KEY_G:
				outCode = KeyEvent.VK_G;
				break;
			case VCLEvent.KEY_H:
				outCode = KeyEvent.VK_H;
				break;
			case VCLEvent.KEY_I:
				outCode = KeyEvent.VK_I;
				break;
			case VCLEvent.KEY_J:
				outCode = KeyEvent.VK_J;
				break;
			case VCLEvent.KEY_K:
				outCode = KeyEvent.VK_K;
				break;
			case VCLEvent.KEY_L:
				outCode = KeyEvent.VK_L;
				break;
			case VCLEvent.KEY_M:
				outCode = KeyEvent.VK_M;
				break;
			case VCLEvent.KEY_N:
				outCode = KeyEvent.VK_N;
				break;
			case VCLEvent.KEY_O:
				outCode = KeyEvent.VK_O;
				break;
			case VCLEvent.KEY_P:
				outCode = KeyEvent.VK_P;
				break;
			case VCLEvent.KEY_Q:
				outCode = KeyEvent.VK_Q;
				break;
			case VCLEvent.KEY_R:
				outCode = KeyEvent.VK_R;
				break;
			case VCLEvent.KEY_S:
				outCode = KeyEvent.VK_S;
				break;
			case VCLEvent.KEY_T:
				outCode = KeyEvent.VK_T;
				break;
			case VCLEvent.KEY_U:
				outCode = KeyEvent.VK_U;
				break;
			case VCLEvent.KEY_V:
				outCode = KeyEvent.VK_V;
				break;
			case VCLEvent.KEY_W:
				outCode = KeyEvent.VK_W;
				break;
			case VCLEvent.KEY_X:
				outCode = KeyEvent.VK_X;
				break;
			case VCLEvent.KEY_Y:
				outCode = KeyEvent.VK_Y;
				break;
			case VCLEvent.KEY_Z:
				outCode = KeyEvent.VK_Z;
				break;
			case VCLEvent.KEY_F1:
				outCode = KeyEvent.VK_F1;
				break;
			case VCLEvent.KEY_F2:
				outCode = KeyEvent.VK_F2;
				break;
			case VCLEvent.KEY_F3:
				outCode = KeyEvent.VK_F3;
				break;
			case VCLEvent.KEY_F4:
				outCode = KeyEvent.VK_F4;
				break;
			case VCLEvent.KEY_F5:
				outCode = KeyEvent.VK_F5;
				break;
			case VCLEvent.KEY_F6:
				outCode = KeyEvent.VK_F6;
				break;
			case VCLEvent.KEY_F7:
				outCode = KeyEvent.VK_F7;
				break;
			case VCLEvent.KEY_F8:
				outCode = KeyEvent.VK_F8;
				break;
			case VCLEvent.KEY_F9:
				outCode = KeyEvent.VK_F9;
				break;
			case VCLEvent.KEY_F10:
				outCode = KeyEvent.VK_F10;
				break;
			case VCLEvent.KEY_F11:
				outCode = KeyEvent.VK_F11;
				break;
			case VCLEvent.KEY_F12:
				outCode = KeyEvent.VK_F12;
				break;
			case VCLEvent.KEY_F13:
				outCode = KeyEvent.VK_F13;
				break;
			case VCLEvent.KEY_F14:
				outCode = KeyEvent.VK_F14;
				break;
			case VCLEvent.KEY_F15:
				outCode = KeyEvent.VK_F15;
				break;
			case VCLEvent.KEY_F16:
				outCode = KeyEvent.VK_F16;
				break;
			case VCLEvent.KEY_F17:
				outCode = KeyEvent.VK_F17;
				break;
			case VCLEvent.KEY_F18:
				outCode = KeyEvent.VK_F18;
				break;
			case VCLEvent.KEY_F19:
				outCode = KeyEvent.VK_F19;
				break;
			case VCLEvent.KEY_F20:
				outCode = KeyEvent.VK_F20;
				break;
			case VCLEvent.KEY_F21:
				outCode = KeyEvent.VK_F21;
				break;
			case VCLEvent.KEY_F22:
				outCode = KeyEvent.VK_F22;
				break;
			case VCLEvent.KEY_F23:
				outCode = KeyEvent.VK_F23;
				break;
			case VCLEvent.KEY_F24:
				outCode = KeyEvent.VK_F24;
				break;
			case VCLEvent.KEY_DOWN:
				outCode = KeyEvent.VK_DOWN;
				break;
			case VCLEvent.KEY_UP:
				outCode = KeyEvent.VK_UP;
				break;
			case VCLEvent.KEY_LEFT:
				outCode = KeyEvent.VK_LEFT;
				break;
			case VCLEvent.KEY_RIGHT:
				outCode = KeyEvent.VK_RIGHT;
				break;
			case VCLEvent.KEY_HOME:
				outCode = KeyEvent.VK_HOME;
				break;
			case VCLEvent.KEY_END:
				outCode = KeyEvent.VK_END;
				break;
			case VCLEvent.KEY_PAGEUP:
				outCode = KeyEvent.VK_PAGE_UP;
				break;
			case VCLEvent.KEY_PAGEDOWN:
				outCode = KeyEvent.VK_PAGE_DOWN;
				break;
			case VCLEvent.KEY_RETURN:
				outCode = KeyEvent.VK_ENTER;
				break;
			case VCLEvent.KEY_ESCAPE:
				outCode = KeyEvent.VK_ESCAPE;
				break;
			case VCLEvent.KEY_TAB:
				outCode = KeyEvent.VK_TAB;
				break;
			case VCLEvent.KEY_BACKSPACE:
				outCode = KeyEvent.VK_BACK_SPACE;
				break;
			case VCLEvent.KEY_SPACE:
				outCode = KeyEvent.VK_SPACE;
				break;
			case VCLEvent.KEY_INSERT:
				outCode = KeyEvent.VK_INSERT;
				break;
			case VCLEvent.KEY_DELETE:
				outCode = KeyEvent.VK_DELETE;
				break;
			case VCLEvent.KEY_ADD:
				outCode = KeyEvent.VK_ADD;
				break;
			case VCLEvent.KEY_SUBTRACT:
				outCode = KeyEvent.VK_SUBTRACT;
				break;
			case VCLEvent.KEY_MULTIPLY:
				outCode = KeyEvent.VK_MULTIPLY;
				break;
			case VCLEvent.KEY_DIVIDE:
				outCode = KeyEvent.VK_DIVIDE;
				break;
			case VCLEvent.KEY_POINT:
				outCode = KeyEvent.VK_PERIOD;
				break;
			case VCLEvent.KEY_COMMA:
				outCode = KeyEvent.VK_COMMA;
				break;
			case VCLEvent.KEY_LESS:
				outCode = KeyEvent.VK_LESS;
				break;
			case VCLEvent.KEY_GREATER:
				outCode = KeyEvent.VK_GREATER;
				break;
			case VCLEvent.KEY_EQUAL:
				outCode = KeyEvent.VK_EQUALS;
				break;
			case VCLEvent.KEY_CUT:
				outCode = KeyEvent.VK_CUT;
				break;
			case VCLEvent.KEY_COPY:
				outCode = KeyEvent.VK_COPY;
				break;
			case VCLEvent.KEY_PASTE:
				outCode = KeyEvent.VK_PASTE;
				break;
			case VCLEvent.KEY_UNDO:
				outCode = KeyEvent.VK_UNDO;
				break;
			case VCLEvent.KEY_REPEAT:
				outCode = KeyEvent.VK_AGAIN;
				break;
			case VCLEvent.KEY_FIND:
				outCode = KeyEvent.VK_FIND;
				break;
			case VCLEvent.KEY_PROPERTIES:
				outCode = KeyEvent.VK_PROPS;
				break;
			case VCLEvent.KEY_HELP:
				outCode = KeyEvent.VK_HELP;
				break;
			case VCLEvent.KEY_F25:
			case VCLEvent.KEY_F26:
			case VCLEvent.KEY_OPEN:
			case VCLEvent.KEY_FRONT:
			case VCLEvent.KEY_CONTEXTMENU:
			case VCLEvent.KEY_MENU:
			case VCLEvent.KEY_CODE:
				break;
			default:
				break;
		}

		return outCode;

	}

}
