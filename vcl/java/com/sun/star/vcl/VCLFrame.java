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
 *         - GNU General Public License Version 2.1
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

import java.awt.ActiveEvent;
import java.awt.Color;
import java.awt.Component;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.Frame;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Insets;
import java.awt.Panel;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.Toolkit;
import java.awt.Window;
import java.awt.event.ComponentEvent;
import java.awt.event.ComponentListener;
import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.awt.event.InputEvent;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;

/**
 * The Java class that implements the SalFrame C++ class methods.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public final class VCLFrame implements ComponentListener, FocusListener, KeyListener, MouseListener, MouseMotionListener, WindowListener {

	/**
	 * SAL_FRAME_STYLE_DEFAULT constant.
	 */
	public final static long SAL_FRAME_STYLE_DEFAULT = 0x00000001;

	/**
	 * SAL_FRAME_STYLE_MOVEABLE constant.
	 */
	public final static long SAL_FRAME_STYLE_MOVEABLE = 0x00000002;

	/**
	 * SAL_FRAME_STYLE_SIZEABLE constant.
	 */
	public final static long SAL_FRAME_STYLE_SIZEABLE = 0x00000004;

	/**
	 * SAL_FRAME_STYLE_CLOSEABLE constant.
	 */
	public final static long SAL_FRAME_STYLE_CLOSEABLE = 0x00000008;

	/**
	 * SAL_FRAME_STYLE_CHILD constant.
	 */
	public final static long SAL_FRAME_STYLE_CHILD = 0x10000000;

	/**
	 * SAL_FRAME_STYLE_FLOAT constant.
	 */
	public final static long SAL_FRAME_STYLE_FLOAT = 0x20000000;

	/**
	 * POINTER_ARROW constant.
	 */
	public final static int POINTER_ARROW = 0;

	/**
	 * POINTER_NULL constant.
	 */
	public final static int POINTER_NULL = 1;

	/**
	 * POINTER_WAIT constant.
	 */
	public final static int POINTER_WAIT = 2;

	/**
	 * POINTER_TEXT constant.
	 */
	public final static int POINTER_TEXT = 3;

	/**
	 * POINTER_HELP constant.
	 */
	public final static int POINTER_HELP = 4;

	/**
	 * POINTER_CROSS constant.
	 */
	public final static int POINTER_CROSS = 5;

	/**
	 * POINTER_MVOE constant.
	 */
	public final static int POINTER_MOVE = 6;

	/**
	 * POINTER_NSIZE constant.
	 */
	public final static int POINTER_NSIZE = 7;

	/**
	 * POINTER_SSIZE constant.
	 */
	public final static int POINTER_SSIZE = 8;

	/**
	 * POINTER_WSIZE constant.
	 */
	public final static int POINTER_WSIZE = 9;

	/**
	 * POINTER_ESIZE constant.
	 */
	public final static int POINTER_ESIZE = 10;

	/**
	 * POINTER_NWSIZE constant.
	 */
	public final static int POINTER_NWSIZE = 11;

	/**
	 * POINTER_NESIZE constant.
	 */
	public final static int POINTER_NESIZE = 12;

	/**
	 * POINTER_SWSIZE constant.
	 */
	public final static int POINTER_SWSIZE = 13;

	/**
	 * POINTER_SESIZE constant.
	 */
	public final static int POINTER_SESIZE = 14;

	/**
	 * POINTER_WINDOW_NSIZE constant.
	 */
	public final static int POINTER_WINDOW_NSIZE = 15;

	/**
	 * POINTER_WINDOW_SSIZE constant.
	 */
	public final static int POINTER_WINDOW_SSIZE = 16;

	/**
	 * POINTER_WINDOW_WSIZE constant.
	 */
	public final static int POINTER_WINDOW_WSIZE = 17;

	/**
	 * POINTER_WINDOW_ESIZE constant.
	 */
	public final static int POINTER_WINDOW_ESIZE = 18;

	/**
	 * POINTER_WINDOW_NWSIZE constant.
	 */
	public final static int POINTER_WINDOW_NWSIZE = 19;

	/**
	 * POINTER_NESIZE constant.
	 */
	public final static int POINTER_WINDOW_NESIZE = 20;

	/**
	 * POINTER_WINDOW_SWSIZE constant.
	 */
	public final static int POINTER_WINDOW_SWSIZE = 21;

	/**
	 * POINTER_WINDOW_SESIZE constant.
	 */
	public final static int POINTER_WINDOW_SESIZE = 22;

	/**
	 * POINTER_HSPLIT constant.
	 */
	public final static int POINTER_HSPLIT = 23;

	/**
	 * POINTER_VSPLIT constant.
	 */
	public final static int POINTER_VSPLIT = 24;

	/**
	 * POINTER_HSIZEBAR constant.
	 */
	public final static int POINTER_HSIZEBAR = 25;

	/**
	 * POINTER_VSIZEBAR constant.
	 */
	public final static int POINTER_VSIZEBAR = 26;

	/**
	 * POINTER_POINTER_HAND constant.
	 */
	public final static int POINTER_HAND = 27;

	/**
	 * POINTER_REFHNAND constant.
	 */
	public final static int POINTER_REFHAND = 28;

	/**
	 * POINTER_PEN constant.
	 */
	public final static int POINTER_PEN = 29;

	/**
	 * POINTER_MAGNIFY constant.
	 */
	public final static int POINTER_MAGNIFY = 30;

	/**
	 * POINTER_FILL constant.
	 */
	public final static int POINTER_FILL = 31;

	/**
	 * POINTER_ROTATE constant.
	 */
	public final static int POINTER_ROTATE = 32;

	/**
	 * POINTER_HSHEAR constant.
	 */
	public final static int POINTER_HSHEAR = 33;

	/**
	 * POINTER_VSHEAR constant.
	 */
	public final static int POINTER_VSHEAR = 34;

	/**
	 * POINTER_MIRROR constant.
	 */
	public final static int POINTER_MIRROR = 35;

	/**
	 * POINTER_CROOK constant.
	 */
	public final static int POINTER_CROOK = 36;

	/**
	 * POINTER_CROP constant.
	 */
	public final static int POINTER_CROP = 37;

	/**
	 * POINTER_MOVEPOINT constant.
	 */
	public final static int POINTER_MOVEPOINT = 38;

	/**
	 * POINTER_MOVEBEZIERWEIGHT constant.
	 */
	public final static int POINTER_MOVEBEZIERWEIGHT = 39;

	/**
	 * POINTER_MOVEDATA constant.
	 */
	public final static int POINTER_MOVEDATA = 40;

	/**
	 * POINTER_COPYDATA constant.
	 */
	public final static int POINTER_COPYDATA = 41;

	/**
	 * POINTER_LINKDATA constant.
	 */
	public final static int POINTER_LINKDATA = 42;

	/**
	 * POINTER_MOVEDATA_LINK constant.
	 */
	public final static int POINTER_MOVEDATALINK = 43;

	/**
	 * POINTER_COPYDATA_LINK constant.
	 */
	public final static int POINTER_COPYDATALINK = 44;

	/**
	 * POINTER_MOVEFILE constant.
	 */
	public final static int POINTER_MOVEFILE = 45;

	/**
	 * POINTER_COPYFILE constant.
	 */
	public final static int POINTER_COPYFILE = 46;

	/**
	 * POINTER_LINKFILE constant.
	 */
	public final static int POINTER_LINKFILE = 47;

	/**
	 * POINTER_MOVEFILELINK constant.
	 */
	public final static int POINTER_MOVEFILELINK = 48;

	/**
	 * POINTER_COPYFILELINK constant.
	 */
	public final static int POINTER_COPYFILELINK = 49;

	/**
	 * POINTER_MOVEFILES constant.
	 */
	public final static int POINTER_MOVEFILES = 50;

	/**
	 * POINTER_COPYFILES constant.
	 */
	public final static int POINTER_COPYFILES = 51;

	/**
	 * POINTER_NOTALLOWED constant.
	 */
	public final static int POINTER_NOTALLOWED = 52;

	/**
	 * POINTER_DRAW_LINE constant.
	 */
	public final static int POINTER_DRAW_LINE = 53;

	/**
	 * POINTER_DRAW_RECT constant.
	 */
	public final static int POINTER_DRAW_RECT = 54;

	/**
	 * POINTER_DRAW_POLYGON constant.
	 */
	public final static int POINTER_DRAW_POLYGON = 55;

	/**
	 * POINTER_DRAW_BEZIER constant.
	 */
	public final static int POINTER_DRAW_BEZIER = 56;

	/**
	 * POINTER_DRAW_ARC constant.
	 */
	public final static int POINTER_DRAW_ARC = 57;

	/**
	 * POINTER_DRAW_PIE constant.
	 */
	public final static int POINTER_DRAW_PIE = 58;

	/**
	 * POINTER_DRAW_CIRCLECUT constant.
	 */
	public final static int POINTER_DRAW_CIRCLECUT = 59;

	/**
	 * POINTER_DRAW_ELLISPE constant.
	 */
	public final static int POINTER_DRAW_ELLIPSE = 60;

	/**
	 * POINTER_DRAW_FREEHAND constant.
	 */
	public final static int POINTER_DRAW_FREEHAND = 61;

	/**
	 * POINTER_DRAW_CONNECT constant.
	 */
	public final static int POINTER_DRAW_CONNECT = 62;

	/**
	 * POINTER_DRAW_TEXT constant.
	 */
	public final static int POINTER_DRAW_TEXT = 63;

	/**
	 * POINTER_DRAW_CAPTION constant.
	 */
	public final static int POINTER_DRAW_CAPTION = 64;

	/**
	 * POINTER_CHART constant.
	 */
	public final static int POINTER_CHART = 65;

	/**
	 * POINTER_DETECTIVE constant.
	 */
	public final static int POINTER_DETECTIVE = 66;

	/**
	 * POINTER_PIVOT_COL constant.
	 */
	public final static int POINTER_PIVOT_COL = 67;

	/**
	 * POINTER_PIVOT_ROW constant.
	 */
	public final static int POINTER_PIVOT_ROW = 68;

	/**
	 * POINTER_PIVOT_FIELD constant.
	 */
	public final static int POINTER_PIVOT_FIELD = 69;

	/**
	 * POINTER_CHAIN constant.
	 */
	public final static int POINTER_CHAIN = 70;

	/**
	 * POINTER_CHAIN_NOTALLOWED constant.
	 */
	public final static int POINTER_CHAIN_NOTALLOWED = 71;

	/**
	 * POINTER_TIMEEVENT_MOVE constant.
	 */
	public final static int POINTER_TIMEEVENT_MOVE = 72;

	/**
	 * POINTER_TIMEEVENT_SIZE constant.
	 */
	public final static int POINTER_TIMEEVENT_SIZE = 73;

	/**
	 * POINTER_AUTOSCROLL_N constant.
	 */
	public final static int POINTER_AUTOSCROLL_N = 74;

	/**
	 * POINTER_AUTOSCROLL_S constant.
	 */
	public final static int POINTER_AUTOSCROLL_S = 75;

	/**
	 * POINTER_AUTOSCROLL_W constant.
	 */
	public final static int POINTER_AUTOSCROLL_W = 76;

	/**
	 * POINTER_AUTOSCROLL_E constant.
	 */
	public final static int POINTER_AUTOSCROLL_E = 77;

	/**
	 * POINTER_AUTOSCROLL_NW constant.
	 */
	public final static int POINTER_AUTOSCROLL_NW = 78;

	/**
	 * POINTER_AUTOSCROLL_NE constant.
	 */
	public final static int POINTER_AUTOSCROLL_NE = 79;

	/**
	 * POINTER_AUTOSCROLL_SW constant.
	 */
	public final static int POINTER_AUTOSCROLL_SW = 80;

	/**
	 * POINTER_AUTOSCROLL_SE constant.
	 */
	public final static int POINTER_AUTOSCROLL_SE = 81;

	/**
	 * POINTER_AUTOSCROLL_NS constant.
	 */
	public final static int POINTER_AUTOSCROLL_NS = 82;

	/**
	 * POINTER_AUTOSCROLL_WE constant.
	 */
	public final static int POINTER_AUTOSCROLL_WE = 83;

	/**
	 * POINTER_AUTOSCROLL_NSWE constant.
	 */
	public final static int POINTER_AUTOSCROLL_NSWE = 84;

	/**
	 * POINTER_AIRBRUSH constant.
	 */
	public final static int POINTER_AIRBRUSH = 85;

	/**
	 * POINTER_TEXT_VERTICAL constant.
	 */
	public final static int POINTER_TEXT_VERTICAL = 86;

	/**
	 * The capture frame.
	 */
	private static VCLFrame captureFrame = null;

	/**
	 * The capture flag.
	 */
	private static boolean capture = false;

	/**
	 * The first capture event.
	 */
	private static MouseEvent firstCaptureEvent = null;

	/**
	 * The first capture frame.
	 */
	private static VCLFrame firstCaptureFrame = null;

	/**
	 * The last capture frame.
	 */
	private static VCLFrame lastCaptureFrame = null;

	/**
	 * The bit count.
	 */
	private int bitCount = 0;

	/**
	 * The frame pointer.
	 */
	private long frame = 0;

	/**
	 * The full screen mode.
	 */
	private boolean fullScreenMode = false;

	/**
	 * The graphics.
	 */
	private VCLGraphics graphics = null;

	/**
	 * The native window's insets.
	 */
	private Insets insets = null;

	/** 
	 * The last key pressed for which a key typed event has not been received.
	 */
	private KeyEvent lastKeyPressed = null;

	/**
	 * The native window's original bounds.
	 */
	private Rectangle originalBounds = null;

	/**
	 * The native window's panel.
	 */
	private VCLFrame.NoPaintPanel panel = null;

	/**
	 * The parent frame.
	 */
	private VCLFrame parent = null;

	/**
	 * The event queue.
	 */
	private VCLEventQueue queue = null;

	/**
	 * The resizable flag.
	 */
	private boolean resizable = false;

	/**
	 * The native window.
	 */
	private Window window = null;

	/**
	 * Constructs a new <code>VCLFrame</code> instance.
	 *
	 * @param styleFlags the SAL_FRAME_STYLE flags
	 * @param q the event queue to post events to
	 * @param f the frame pointer
	 */
	public VCLFrame(long styleFlags, VCLEventQueue q, long f, VCLFrame p) {

		queue = q;
		frame = f;
		parent = p;

		// Create the native window
		if ((styleFlags & (SAL_FRAME_STYLE_DEFAULT | SAL_FRAME_STYLE_MOVEABLE | SAL_FRAME_STYLE_SIZEABLE)) != 0)
			window = new Frame();
		else
			window = new Window(new Frame());

		// Process remaining style flags
		if ((styleFlags & SAL_FRAME_STYLE_SIZEABLE) != 0)
			resizable = true;

		// Add a panel as the only component
		panel = new VCLFrame.NoPaintPanel(this);
		panel.setBackground(Color.white);
		panel.enableInputMethods(false);
		window.add(panel);
		bitCount = panel.getColorModel().getPixelSize();
		if (bitCount <= 1)
			bitCount = 1;
		else if (bitCount <= 4)
			bitCount = 4;
		else if (bitCount <= 8)
			bitCount = 8;
		else
			bitCount = 24;
		if (window instanceof Frame)
			insets = VCLScreen.getFrameInsets();
		else
			insets = window.getInsets();
		graphics = new VCLGraphics(this);

	}

	/**
	 * Invoked when the the native window's size changes.
	 *
	 * @param e the <code>ComponentEvent</code>
	 */
	public void componentResized(ComponentEvent e) {

		graphics.resetGraphics();
		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MOVERESIZE, this, 0));

	}

	/**
	 * Invoked when the the native window's position changes.
	 *
	 * @param e the <code>ComponentEvent</code>
	 */
	public void componentMoved(ComponentEvent e) {

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MOVERESIZE, this, 0));

	}

	/**
	 * Invoked when the native window has been made visible.
	 *
	 * @param e the <code>ComponentEvent</code>
	 */
	public void componentShown(ComponentEvent e) {

		// Set capture frame
		VCLFrame.captureFrame = this;

	}

	/**
	 * Invoked when the native window has been made invisible.
	 *
	 * @param e the <code>ComponentEvent</code>
	 */
	public void componentHidden(ComponentEvent e) {

		// Set mouse capture to parent frame
		if (VCLFrame.captureFrame == this)
			VCLFrame.captureFrame = parent;
		if (VCLFrame.lastCaptureFrame == this)
			VCLFrame.lastCaptureFrame = null;
		if (VCLFrame.firstCaptureFrame == this && VCLFrame.firstCaptureEvent != null)
			mouseReleased(new MouseEvent(panel, MouseEvent.MOUSE_RELEASED, System.currentTimeMillis(), VCLFrame.firstCaptureEvent.getModifiers(), VCLFrame.firstCaptureEvent.getX(), VCLFrame.firstCaptureEvent.getY(), VCLFrame.firstCaptureEvent.getClickCount(), VCLFrame.firstCaptureEvent.isPopupTrigger()));

	}


	/**
	 * Invoked when the native window has gained focus.
	 *
	 * @param e the <code>FocusEvent</code>
	 */
	public void focusGained(FocusEvent e) {

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_GETFOCUS, this, 0));

	}

	/**
	 * Invoked when the native window has lost focus.
	 *
	 * @param e the <code>FocusEvent</code>
	 */
	public void focusLost(FocusEvent e) {

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_LOSEFOCUS, this, 0));

	}

	/**
	 * Disposes the native window and releases any system resources that it is
	 * using.
	 */
	public void dispose() {

		if (window != null)
			setVisible(false);
		if (queue != null && frame != 0)
			queue.removeCachedEvents(frame);
		bitCount = 0;
		frame = 0;
		fullScreenMode = true;
		insets = null;
		queue = null;
		panel = null;
		parent = null;
		if (graphics != null)
			graphics.dispose();
		graphics = null;
		if (window != null) {
			window.removeNotify();
			window.dispose();
		}
		window = null;
		lastKeyPressed = null;
		originalBounds = null;

	}

	/**
	 * Flushes the native window.
	 */
	public void flush() {

		Toolkit.getDefaultToolkit().sync();

	}

	/**
	 * Returns the bit count of the native window.
	 *
	 * @return the bit count of the native window
	 */
	int getBitCount() {

		return bitCount;

	}

	/**
	 * Returns the bounds of the native window.
	 *
	 * @return the bounds of the native window
	 */
	public Rectangle getBounds() {

		Rectangle bounds = window.getBounds();
		if (window.isShowing()) {
			Point location = window.getLocationOnScreen();
			bounds.x = location.x;
			bounds.y = location.y;
		}
		return bounds;

	}

	/**
	 * Returns the frame pointer for this component.
	 *
	 * @return the frame pointer for this component
	 */
	long getFrame() {

		return frame;

	}

	/**
	 * Returns the graphics context for this component.
	 *
	 * @return the graphics context for this component
	 */
	public VCLGraphics getGraphics() {

		return graphics;

	}

	/**
	 * Returns the insets for the native window.
	 *
	 * @return the insets for the native window
	 */
	public Insets getInsets() {

		return insets;

	}

	/**
	 * Returns the localized name of the specified key code.
	 *
	 * @param keyCode the key code
	 * @return the localized name of the specified key code
	 */
	public String getKeyName(int keyCode) {

		StringBuffer buf = new StringBuffer();
		if ((keyCode & VCLEvent.KEY_MOD1) == VCLEvent.KEY_MOD1) {
			if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX) {
				buf.append("\u2318");
            }
			else {
				buf.append(KeyEvent.getKeyText(KeyEvent.VK_CONTROL));
			    buf.append("+");
            }
		}
		if ((keyCode & VCLEvent.KEY_MOD2) == VCLEvent.KEY_MOD2) {
			if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX) {
				buf.append("\u2325");
            }
			else {
				buf.append(KeyEvent.getKeyText(KeyEvent.VK_ALT));
			    buf.append("+");
            }
		}
		if ((keyCode & VCLEvent.KEY_SHIFT) == VCLEvent.KEY_SHIFT) {
			if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX) {
				buf.append("\u21e7");
            }
			else {
				buf.append(KeyEvent.getKeyText(KeyEvent.VK_SHIFT));
			    buf.append("+");
            }
		}

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

		if (outCode != 0)
			buf.append(KeyEvent.getKeyText(outCode));
		else
			buf.append("???");
		return buf.toString();

	}

	/**
	 * Returns the panel for the native window.
	 *
	 * @return the panel for the native window
	 */
	Panel getPanel() {

		return panel;

	}

	/**
	 * Returns the parent frame.
	 *
	 * @return the parent frame
	 */
	VCLFrame getParent() {

		return parent;

	}

	/**
	 * Returns the native window.
	 *
	 * @return the native window
	 */
	Window getWindow() {

		return window;

	}

	/**
	 * Invoked when a key has been pressed.
	 *
	 * @param e the <code>KeyEvent</code>
	 */
	public void keyPressed(KeyEvent e) {

		int keyCode = e.getKeyCode();
		if (keyCode == KeyEvent.VK_SHIFT || keyCode == KeyEvent.VK_CONTROL || keyCode == KeyEvent.VK_ALT)
			queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_KEYMODCHANGE, this, 0));
		else if (e.isActionKey())
			queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_KEYINPUT, this, 0));
		else
			lastKeyPressed = e;

	}

	/**
	 * Invoked when a key has been released.
	 *
	 * @param e the <code>KeyEvent</code>
	 */
	public void keyReleased(KeyEvent e) {

		int keyCode = e.getKeyCode();
		if (keyCode == KeyEvent.VK_SHIFT || keyCode == KeyEvent.VK_CONTROL || keyCode == KeyEvent.VK_ALT) {
			queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_KEYMODCHANGE, this, 0));
		}
		else if (e.isActionKey()) {
			queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_KEYUP, this, 0));
		}
		else if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX) {
			// Trap the Mac OS X key combinations that are reserved for the
			// items in the Apple Services menu. Even though we don't respond
			// to these services, Java does not generate a key typed event so
			// we need to generate it ourselves.
			if (lastKeyPressed != null && keyCode >= KeyEvent.VK_A && keyCode <= KeyEvent.VK_Z && keyCode != KeyEvent.VK_H && keyCode != KeyEvent.VK_Q) {
				int modifiers = lastKeyPressed.getModifiers();
				if ((modifiers & (InputEvent.SHIFT_MASK | InputEvent.CTRL_MASK)) == (InputEvent.SHIFT_MASK | InputEvent.CTRL_MASK))
					e = new KeyEvent(e.getComponent(), KeyEvent.KEY_TYPED, e.getWhen(), modifiers, KeyEvent.VK_UNDEFINED, Character.toLowerCase((char)keyCode));
					queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_KEYINPUT, this, 0));
					queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_KEYUP, this, 0));
					lastKeyPressed = null;
			}
		}

	}

	/**
	 * Invoked when a key has been typed.
	 *
	 * @param e the <code>KeyEvent</code>
	 */
	public void keyTyped(KeyEvent e) {

		// If a modifier is used to set the character (e.g. the "Alt-c"
		// generates a "c-cedilla" in the Mac OS X U.S. keyboard, we must strip
		// off the modifiers so that the C++ code does not get confused.
		int modifiers = e.getModifiers();
		char keyChar = e.getKeyChar();
		if (lastKeyPressed != null && (modifiers & InputEvent.ALT_MASK) != 0)
			e = new KeyEvent(e.getComponent(), e.getID(), e.getWhen(), modifiers & ~InputEvent.ALT_MASK, e.getKeyCode(), e.getKeyChar());
		lastKeyPressed = null;

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_KEYINPUT, this, 0));
		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_KEYUP, this, 0));

	}

	/**
	 * Invoked when the mouse has been clicked on a component.
	 *
	 * @param e the <code>MouseEvent</code>
	 */
	public void mouseClicked(MouseEvent e) {}

	/**
	 * Invoked when a mouse button has been pressed on a component.
	 *
	 * @param e the <code>MouseEvent</code>
	 */
	public void mousePressed(MouseEvent e) {

		VCLFrame.firstCaptureEvent = e;
		VCLFrame.firstCaptureFrame = this;

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MOUSEBUTTONDOWN, this, 0));

	}

	/**
	 * Invoked when a mouse button has been released on a component.
	 *
	 * @param e the <code>MouseEvent</code>
	 */
	public void mouseReleased(MouseEvent e) {

		VCLFrame f = this;

		if (VCLFrame.capture && VCLFrame.captureFrame != null && e.getComponent().isShowing()) {
			// Send the mouse event to the capture frame
			f = VCLFrame.captureFrame;
			Point srcPoint = e.getComponent().getLocationOnScreen();
			srcPoint.x += e.getX();
			srcPoint.y += e.getY();
			while (f != null) {
				if (f.getWindow().isShowing()) {
					Panel p = f.getPanel();
					Point destPoint = p.getLocationOnScreen();
					Rectangle destRect = new Rectangle(destPoint.x, destPoint.y, p.getWidth(), p.getHeight());
					e = new MouseEvent(f.getPanel(), e.getID(), e.getWhen(), e.getModifiers(), srcPoint.x - destPoint.x, srcPoint.y - destPoint.y, e.getClickCount(), e.isPopupTrigger());
					break;
				}
				f = f.getParent();
			}

			if (f == null)
				f = this;
		}

		// Send a focus lost event if we release on a borderless window
		if (VCLFrame.lastCaptureFrame != null && VCLFrame.lastCaptureFrame != f && VCLFrame.lastCaptureFrame.getWindow() instanceof Frame && VCLFrame.lastCaptureFrame.getWindow().isShowing()) {
			Panel p = VCLFrame.lastCaptureFrame.getPanel();
			VCLFrame.lastCaptureFrame.focusLost(new FocusEvent(p, FocusEvent.FOCUS_LOST));
		}

		// Disable mouse capture
		VCLFrame.capture = false;
		VCLFrame.firstCaptureEvent = null;
		VCLFrame.firstCaptureFrame = null;
		VCLFrame.lastCaptureFrame = null;

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MOUSEBUTTONUP, f, 0));

	}

	/**
	 * Invoked when a mouse button is pressed on a component and then dragged.
	 *
	 * @param e the <code>MouseEvent</code>
	 */
	public void mouseDragged(MouseEvent e) {

		// Enable mouse capture
		VCLFrame.capture = true;

		VCLFrame f = this;

		if (VCLFrame.capture && VCLFrame.captureFrame != null && e.getComponent().isShowing()) {
			// Find the capture window
			f = VCLFrame.captureFrame;
			Point srcPoint = e.getComponent().getLocationOnScreen();
			srcPoint.x += e.getX();
			srcPoint.y += e.getY();
			while (f != null) {
				if (f.getWindow().isShowing()) {
					Panel p = f.getPanel();
					Point destPoint = p.getLocationOnScreen();
					Rectangle destRect = new Rectangle(destPoint.x, destPoint.y, p.getWidth(), p.getHeight());
					if (destRect.contains(srcPoint)) {
						e = new MouseEvent(p, e.getID(), e.getWhen(), e.getModifiers(), srcPoint.x - destPoint.x, srcPoint.y - destPoint.y, e.getClickCount(), e.isPopupTrigger());
						break;
					}
				}
				f = f.getParent();
			}

			if (f == null)
				f = this;

			// Send a mouse exited event if the capture window has changed
			if (VCLFrame.lastCaptureFrame != null && VCLFrame.lastCaptureFrame != f && VCLFrame.captureFrame.getWindow().isShowing()) {
				Panel p = VCLFrame.lastCaptureFrame.getPanel();
				Point destPoint = p.getLocationOnScreen();
				MouseEvent mouseExited = new MouseEvent(p, MouseEvent.MOUSE_EXITED, e.getWhen(), e.getModifiers(), srcPoint.x - destPoint.x, srcPoint.y - destPoint.y, e.getClickCount(), e.isPopupTrigger());
				queue.postCachedEvent(new VCLEvent(mouseExited, VCLEvent.SALEVENT_MOUSELEAVE, VCLFrame.lastCaptureFrame, 0));
			}
			VCLFrame.lastCaptureFrame = f;
		}

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MOUSEMOVE, f, 0));

	}

	/**
	 * Invoked when the mouse enters a component.
	 *
	 * @param e the <code>MouseEvent</code>
	 */
	public void mouseEntered(MouseEvent e) {

		if (!VCLFrame.capture)
			queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MOUSEMOVE, this, 0));

	}

	/**
	 * Invoked when the mouse exits a component.
	 *
	 * @param e the <code>MouseEvent</code>
	 */
	public void mouseExited(MouseEvent e) {

		if (!VCLFrame.capture)
			queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MOUSELEAVE, this, 0));

	}

	/**
	 * Invoked when the mouse button has been moved on a component (with no
	 * buttons pressed).
	 *
	 * @param e the <code>MouseEvent</code>
	 */
	public void mouseMoved(MouseEvent e) {

		if (!VCLFrame.capture)
			queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MOUSEMOVE, this, 0));

	}

	/**
	 * Moves and resizes this native window.
	 *
	 * @param x the new x-coordinate
	 * @param y the new y-coordinate
	 * @param width the new width
	 * @param height the new height
	 */
	public void setBounds(int x, int y, int width, int height) {

		Rectangle bounds = window.getBounds();
		window.setBounds(x, y, width, height);
		if (graphics != null && width != bounds.width || height != bounds.height)
			graphics.resetGraphics();

	}

	/**
	 * Set the native window to show or hide in full screen mode.
	 *
	 * @param b <code>true</code> sets this component full screen mode and
	 *  <code>false</code> sets it to normal mode
	 */
	public void setFullScreenMode(boolean b) {

		if (b == fullScreenMode)
			return;

		if (b) {
			originalBounds = getBounds();
			Dimension d = VCLScreen.getScreenSize();
			setBounds(0, 0, d.width, d.height);
		}
		else {
			setBounds(originalBounds.x, originalBounds.y, originalBounds.width, originalBounds.height);
			originalBounds = null;
		}

		fullScreenMode = b;

	}

	/**
	 * Sets the cursor.
	 *
	 * @param style the cursor style constant
	 */
	public void setPointer(int style) {

		int cursor = Cursor.DEFAULT_CURSOR;
		switch (style) {
			case POINTER_WAIT:
				cursor = Cursor.WAIT_CURSOR;
				break;
			case POINTER_TEXT:
				cursor = Cursor.TEXT_CURSOR;
				break;
			case POINTER_CROSS:
				cursor = Cursor.CROSSHAIR_CURSOR;
				break;
			case POINTER_MOVE:
				cursor = Cursor.MOVE_CURSOR;
				break;
			case POINTER_NSIZE:
			case POINTER_WINDOW_NSIZE:
				cursor = Cursor.N_RESIZE_CURSOR;
				break;
			case POINTER_SSIZE:
			case POINTER_WINDOW_SSIZE:
				cursor = Cursor.S_RESIZE_CURSOR;
				break;
			case POINTER_WSIZE:
			case POINTER_WINDOW_WSIZE:
				cursor = Cursor.W_RESIZE_CURSOR;
				break;
			case POINTER_ESIZE:
			case POINTER_WINDOW_ESIZE:
				cursor = Cursor.E_RESIZE_CURSOR;
				break;
			case POINTER_NWSIZE:
			case POINTER_WINDOW_NWSIZE:
				cursor = Cursor.NW_RESIZE_CURSOR;
				break;
			case POINTER_NESIZE:
			case POINTER_WINDOW_NESIZE:
				cursor = Cursor.NE_RESIZE_CURSOR;
				break;
			case POINTER_SWSIZE:
			case POINTER_WINDOW_SWSIZE:
				cursor = Cursor.SW_RESIZE_CURSOR;
				break;
			case POINTER_SESIZE:
			case POINTER_WINDOW_SESIZE:
				cursor = Cursor.SE_RESIZE_CURSOR;
				break;
			case POINTER_HAND:
				cursor = Cursor.HAND_CURSOR;
				break;
			case POINTER_TEXT_VERTICAL:
				cursor = Cursor.TEXT_CURSOR;
				break;
			case POINTER_ARROW:
			case POINTER_NULL:
			case POINTER_HELP:
			case POINTER_HSPLIT:
			case POINTER_VSPLIT:
			case POINTER_HSIZEBAR:
			case POINTER_VSIZEBAR:
			case POINTER_REFHAND:
			case POINTER_PEN:
			case POINTER_MAGNIFY:
			case POINTER_FILL:
			case POINTER_ROTATE:
			case POINTER_HSHEAR:
			case POINTER_VSHEAR:
			case POINTER_MIRROR:
			case POINTER_CROOK:
			case POINTER_CROP:
			case POINTER_MOVEPOINT:
			case POINTER_MOVEBEZIERWEIGHT:
			case POINTER_MOVEDATA:
			case POINTER_COPYDATA:
			case POINTER_LINKDATA:
			case POINTER_MOVEDATALINK:
			case POINTER_COPYDATALINK:
			case POINTER_MOVEFILE:
			case POINTER_COPYFILE:
			case POINTER_LINKFILE:
			case POINTER_MOVEFILELINK:
			case POINTER_COPYFILELINK:
			case POINTER_MOVEFILES:
			case POINTER_COPYFILES:
			case POINTER_NOTALLOWED:
			case POINTER_DRAW_LINE:
			case POINTER_DRAW_RECT:
			case POINTER_DRAW_POLYGON:
			case POINTER_DRAW_BEZIER:
			case POINTER_DRAW_ARC:
			case POINTER_DRAW_PIE:
			case POINTER_DRAW_CIRCLECUT:
			case POINTER_DRAW_ELLIPSE:
			case POINTER_DRAW_FREEHAND:
			case POINTER_DRAW_CONNECT:
			case POINTER_DRAW_TEXT:
			case POINTER_DRAW_CAPTION:
			case POINTER_CHART:
			case POINTER_DETECTIVE:
			case POINTER_PIVOT_COL:
			case POINTER_PIVOT_ROW:
			case POINTER_PIVOT_FIELD:
			case POINTER_CHAIN:
			case POINTER_CHAIN_NOTALLOWED:
			case POINTER_TIMEEVENT_MOVE:
			case POINTER_TIMEEVENT_SIZE:
			case POINTER_AUTOSCROLL_N:
			case POINTER_AUTOSCROLL_S:
			case POINTER_AUTOSCROLL_W:
			case POINTER_AUTOSCROLL_E:
			case POINTER_AUTOSCROLL_NW:
			case POINTER_AUTOSCROLL_NE:
			case POINTER_AUTOSCROLL_SW:
			case POINTER_AUTOSCROLL_SE:
			case POINTER_AUTOSCROLL_NS:
			case POINTER_AUTOSCROLL_WE:
			case POINTER_AUTOSCROLL_NSWE:
			case POINTER_AIRBRUSH:
				break;
		}
		if (cursor != window.getCursor().getType()) {
			window.setCursor(Cursor.getPredefinedCursor(cursor));
			Toolkit.getDefaultToolkit().sync();
		}

	}

	/**
	 * Sets the title for this native window to the specified string.
	 *
	 * @param title the title to be displayed in the native window's border
	 */
	public void setTitle(String title) {

		if (window instanceof Frame)
			((Frame)window).setTitle(title);
		else
			((Frame)window.getOwner()).setTitle(title);

	}

	/**
	 * Shows or hides the native window.
	 *
	 * @param b <code>true</code> shows this component otherwise and
	 *  <code>false</code> hides this component
	 */
	public void setVisible(boolean b) {

		if (b == window.isShowing())
			return;

		// Set the resizable flag if needed
		if (window instanceof Frame)
			((Frame)window).setResizable(resizable);

		if (b) {
			// Reset key flags
			lastKeyPressed = null;

			// Register listeners
			window.addComponentListener(this);
			window.addFocusListener(this);
			panel.addFocusListener(this);
			panel.addKeyListener(this);
			panel.addMouseListener(this);
			panel.addMouseMotionListener(this);
			window.addWindowListener(this);
		}

		// Show or hide the window
		window.setVisible(b);
		graphics.resetGraphics();

		if (!b) {
			// Unregister listeners
			window.removeComponentListener(this);
			window.removeFocusListener(this);
			panel.removeFocusListener(this);
			panel.removeKeyListener(this);
			panel.removeMouseListener(this);
			panel.removeMouseMotionListener(this);
			window.removeWindowListener(this);

			// Reset key flags
			lastKeyPressed = null;
		}
		else {
			// Some JVMs delay the initial background painting so force
			// it to be painted before VCL does any painting
			window.getPeer().repaint(0, 0, 0, window.getWidth(), window.getHeight());
			toFront();
		}

	}

	/**
	 * Brings this window to the front.
	 */
	public void toFront() {

		window.toFront();

		// Request keyboard focus
		if (fullScreenMode || window instanceof Frame)
			panel.requestFocus();

	}

	/**
	 * Invoked the first time a window is made visible.
	 *
	 * @param e the <code>WindowEvent</code>
	 */
	public void windowOpened(WindowEvent e) {}

	/**
	 * Invoked when the user attempts to close the window from the window's
	 * system menu.
	 *
	 * @param e the <code>WindowEvent</code>
	 */
	public void windowClosing(WindowEvent e) {

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_CLOSE, this, 0));

	}

	/**
	 * Invoked when a window has been closed as the result of calling dispose
	 * on the window.
	 *
	 * @param e the <code>WindowEvent</code>
	 */
	public void windowClosed(WindowEvent e) {}

	/**
	 * Invoked when a window is changed from a normal to a minimized state.
	 *
	 * @param e the <code>WindowEvent</code>
	 */
	public void windowIconified(WindowEvent e) {}

	/**
	 * Invoked when a window is changed from a minimized to a normal state.
	 *
	 * @param e the <code>WindowEvent</code>
	 */
	public void windowDeiconified(WindowEvent e) {}

	/**
	 * Invoked when the window is set to be the user's active window, which
	 * means the window (or one of its subcomponents) will receive keyboard
	 * events.
	 *
	 * @param e the <code>WindowEvent</code>
	 */
	public void windowActivated(WindowEvent e) {}

	/**
	 * Invoked when a window is no longer the user's active window, which
	 * means that keyboard events will no longer be delivered to the window
	 * or its subcomponents.
	 *
	 * @param e the <code>WindowEvent</code>
	 */
	public void windowDeactivated(WindowEvent e) {}

	/**
	 * A class that has painting methods that perform no painting.
	 */
	final class NoPaintPanel extends Panel {

		/**
		 * The <code>VCLFrame</code>.
		 */
		private VCLFrame frame = null;

		/**
		 * Constructs a new <code>VCLFrame.NoPaintPanel</code> instance.
		 *
		 * @param f the <code>VCLFrame</code>
		 */
		NoPaintPanel(VCLFrame f) {

			frame = f;

		}

		/**
		 * This method performs no painting of the panel. This method is used
		 * to prevent Java from painting over what VCL has painted.
		 *
		 * @param g the <code>Graphics</code>
		 */
		public void paint(Graphics g) {

			VCLGraphics graphics = frame.getGraphics();
			if (graphics != null) {
				Rectangle clip = g.getClipBounds();
				synchronized (graphics) {
					if (clip != null)
						graphics.addToFlush(clip);
					else
						graphics.addToFlush(((Graphics2D)g).getDeviceConfiguration().getBounds());
				}
			}

		}

		/**
		 * This method performs no painting of the panel. This method is used
		 * to prevent Java from painting over what VCL has painted.
		 *
		 * @param g the <code>Graphics</code>
		 */
		public void update(Graphics g) {

			paint(g);

		}

	}

}
