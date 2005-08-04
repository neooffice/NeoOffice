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
import java.awt.EventQueue;
import java.awt.Frame;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Insets;
import java.awt.KeyboardFocusManager;
import java.awt.MenuBar;
import java.awt.Panel;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.Shape;
import java.awt.Toolkit;
import java.awt.Window;
import java.awt.event.ComponentEvent;
import java.awt.event.ComponentListener;
import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.awt.event.InputEvent;
import java.awt.event.InputMethodEvent;
import java.awt.event.InputMethodListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.event.MouseWheelEvent;
import java.awt.event.MouseWheelListener;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.awt.font.TextHitInfo;
import java.awt.im.InputContext;
import java.awt.im.InputMethodRequests;
import java.awt.image.BufferedImage;
import java.awt.peer.ComponentPeer;
import java.text.CharacterIterator;
import java.text.AttributedCharacterIterator;
import java.text.AttributedString;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;

/**
 * The Java class that implements the SalFrame C++ class methods.
 * <p>
 * @version		$Revision$ $Date$
 * @author		$Author$
 */
public final class VCLFrame implements ComponentListener, FocusListener, KeyListener, InputMethodListener, InputMethodRequests, MouseListener, MouseMotionListener, MouseWheelListener, WindowListener {

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
	 * SAL_FRAME_STYLE_NOSHADOW constant.
	 */
	public final static long SAL_FRAME_STYLE_NOSHADOW = 0x00000010;

	/**
	 * SAL_FRAME_STYLE_TOOLTIP constant.
	 */
	public final static long SAL_FRAME_STYLE_TOOLTIP = 0x00000020;

	/**
	 * SAL_FRAME_STYLE_CHILD constant.
	 */
	public final static long SAL_FRAME_STYLE_CHILD = 0x10000000;

	/**
	 * SAL_FRAME_STYLE_FLOAT constant.
	 */
	public final static long SAL_FRAME_STYLE_FLOAT = 0x20000000;

	/**
	 * SAL_FRAMESTATE_NORMAL constant.
	 */
	public final static long SAL_FRAMESTATE_NORMAL = 0x00000001;

	/**
	 * SAL_FRAMESTATE_MINIMIZED constant.
	 */
	public final static long SAL_FRAMESTATE_MINIMIZED = 0x00000002;

	/**
	 * SAL_FRAMESTATE_MAXIMIZED constant.
	 */
	public final static long SAL_FRAMESTATE_MAXIMIZED = 0x00000004;

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
	 * The component to <code>VCLFrame</code> mapping.
	 */
	private static HashMap componentMap = new HashMap();

	/** 
	 * The custom cursors.
	 */
	private static HashMap customCursors = null;

	/** 
	 * The default attributed character iterator.
	 */
	private final static AttributedCharacterIterator defaultAttributedCharacterIterator = new AttributedString("").getIterator();

	/** 
	 * The default text location.
	 */
	private final static Rectangle defaultTextLocation = new Rectangle();

	/** 
	 * The key modifiers pressed.
	 */
	private static int keyModifiersPressed = 0;

	/** 
	 * The key modifiers unpressed.
	 */
	private static int keyModifiersUnpressed = 0;

	/** 
	 * The last key pressed for which a key typed event has not been received.
	 */
	private static KeyEvent lastKeyPressed = null;

	/**
	 * The last mouse drag event.
	 */
	private static MouseEvent lastMouseDragEvent = null;

	/** 
	 * The mouse modifiers pressed.
	 */
	private static int mouseModifiersPressed = 0;

	/**
	 * Find the matching <code>VCLFrame</code> for the specified component.
	 *
	 * @param c the component
	 * @return the matching <code>VCLFrame</code>
	 */
	static VCLFrame findFrame(Component c) {

		return (VCLFrame)componentMap.get(c);

	}

	/**
	 * Initialize input context.
	 */
	static {

		// Set the keyboard focus manager so that Java's default focus
		// switching key events are passed are not consumed
		KeyboardFocusManager kfm = KeyboardFocusManager.getCurrentKeyboardFocusManager();
		kfm.setDefaultFocusTraversalKeys(KeyboardFocusManager.FORWARD_TRAVERSAL_KEYS, Collections.EMPTY_SET);
		kfm.setDefaultFocusTraversalKeys(KeyboardFocusManager.BACKWARD_TRAVERSAL_KEYS, Collections.EMPTY_SET);
		kfm.setDefaultFocusTraversalKeys(KeyboardFocusManager.UP_CYCLE_TRAVERSAL_KEYS, Collections.EMPTY_SET);
		kfm.setDefaultFocusTraversalKeys(KeyboardFocusManager.DOWN_CYCLE_TRAVERSAL_KEYS, Collections.EMPTY_SET);

		// Load pointer images
		Toolkit t = Toolkit.getDefaultToolkit();
		customCursors = new HashMap();
		BufferedImage img = new BufferedImage(1, 1, BufferedImage.TYPE_INT_ARGB_PRE);
		Cursor c = t.createCustomCursor(img, new Point(0, 0), "POINTER_NULL");
		if (c != null)
			customCursors.put(new Integer(POINTER_NULL), c);

	}
	
	/**
	 * The bit count.
	 */
	private int bitCount = 0;

	/** 
	 * The child frames.
	 */
	private LinkedList children = new LinkedList();

	/**
	 * The frame pointer.
	 */
	private int frame = 0;

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
	 * The style flags.
	 */
	private long style = 0;

	/**
	 * The native window.
	 */
	private VCLFrame.NoPaintFrame window = null;

	/**
	 * Constructs a new <code>VCLFrame</code> instance.
	 *
	 * @param s the SAL_FRAME_STYLE flags
	 * @param q the event queue to post events to
	 * @param f the frame pointer
	 */
	public VCLFrame(long s, VCLEventQueue q, int f, VCLFrame p) {

		queue = q;
		frame = f;
		style = s;

		// Create the native window
		window = new VCLFrame.NoPaintFrame(this);
		if ((style & (SAL_FRAME_STYLE_DEFAULT | SAL_FRAME_STYLE_MOVEABLE | SAL_FRAME_STYLE_SIZEABLE)) == 0) {
			window.setUndecorated(true);
			window.setFocusableWindowState(false);
		}

		// Process remaining style flags
		if ((style & SAL_FRAME_STYLE_SIZEABLE) != 0)
			resizable = true;

		// Add a panel as the only component
		panel = new VCLFrame.NoPaintPanel(this);
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

		window.setSize(1, 1);
		window.addNotify();
		insets = window.getInsets();
		if (p != null)
			p.addChild(this);
		graphics = new VCLGraphics(this);

	}

	/**
	 * Gets the latest committed text from the text editing component and
	 * removes it from the component's text body.
	 *
	 * @param attributes a list of attributes that the input method is
	 *  interested in
	 * @return the latest committed text, or null when the "Undo Commit"
	 *  feature is not supported
	 */
	public AttributedCharacterIterator cancelLatestCommittedText(AttributedCharacterIterator.Attribute[] attributes) {

		return defaultAttributedCharacterIterator;

	}

	/**
	 * Invoked when the caret within composed text has changed.
	 *
	 * @param event the input method event
	 */
	public void caretPositionChanged(InputMethodEvent e) {

		e.consume();

	}

	/**
	 * Invoked when the the native window's size changes.
	 *
	 * @param e the <code>ComponentEvent</code>
	 */
	public void componentResized(ComponentEvent e) {

		if (queue == null || window == null || !window.isShowing())
			return;

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MOVERESIZE, this, 0));

	}

	/**
	 * Invoked when the the native window's position changes.
	 *
	 * @param e the <code>ComponentEvent</code>
	 */
	public void componentMoved(ComponentEvent e) {

		if (queue == null || window == null || !window.isShowing())
			return;

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MOVERESIZE, this, 0));

	}

	/**
	 * Invoked when the native window has been made visible.
	 *
	 * @param e the <code>ComponentEvent</code>
	 */
	public void componentShown(ComponentEvent e) {

		// Add panel to mapping
		if (panel != null)
			VCLFrame.componentMap.put(panel, this);

		if (queue == null || window == null || !window.isShowing())
			return;

	}

	/**
	 * Invoked when the native window has been made invisible.
	 *
	 * @param e the <code>ComponentEvent</code>
	 */
	public void componentHidden(ComponentEvent e) {

		// Remove panel from mapping
		VCLFrame.componentMap.remove(panel);

		// Unregister listeners
		if (window != null) {
			window.removeComponentListener(this);
			window.removeWindowListener(this);
		}
		if (panel != null) {
			panel.removeFocusListener(this);
			panel.removeKeyListener(this);
			panel.removeInputMethodListener(this);
			panel.removeMouseListener(this);
			panel.removeMouseMotionListener(this);
			panel.removeMouseWheelListener(this);
		}

		if (queue != null)
			queue.removeCachedEvents(frame);

	}

	/**
	 * Adds a child frame.
	 *
	 * @param f the child frame.
	 */
	public void addChild(VCLFrame f) {

		if (f != null) {
			synchronized (f) {
				synchronized (children) {
					f.parent = this;
					children.add(f);
				}
			}
		}

	}

	/**
	 * Disposes the native window and releases any system resources that it is
	 * using.
	 */
	public void dispose() {

		if (window != null) {
			setVisible(false);
			setMenuBar(null);
		}
		if (queue != null)
			queue.removeCachedEvents(frame);
		queue = null;
		bitCount = 0;
		frame = 0;
		fullScreenMode = false;
		if (graphics != null)
			graphics.dispose();
		insets = null;
		if (panel != null) {
			panel.removeNotify();
			InputContext ic = panel.getInputContext();
			if (ic != null)
				ic.removeNotify(panel);
		}
		panel = null;
		parent = null;
		graphics = null;
		if (window != null) {
			window.removeNotify();
			InputContext ic = window.getInputContext();
			if (ic != null)
				ic.removeNotify(window);
			window.dispose();
		}
		window = null;

	}

	/**
	 * Create and post event to end any uncommitted key input.
	 */
	public void endComposition() {

		InputContext ic = window.getInputContext();
		if (ic != null)
			ic.endComposition();

	}

	/**
	 * Flushes the native window.
	 */
	public void flush() {

		graphics.flush();
		synchronized (window.getTreeLock()) {
			Toolkit.getDefaultToolkit().sync();
		}

	}

	/**
	 * Invoked when the native window has gained focus.
	 *
	 * @param e the <code>FocusEvent</code>
	 */
	public void focusGained(FocusEvent e) {

		if (queue == null || window == null || !window.isShowing())
			return;

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_GETFOCUS, this, 0));

	}

	/**
	 * Invoked when the native window has lost focus.
	 *
	 * @param e the <code>FocusEvent</code>
	 */
	public void focusLost(FocusEvent e) {

		if (queue == null || window == null || !window.isShowing())
			return;

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_LOSEFOCUS, this, 0));

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

		return window.getBounds();

	}

	/**
	 * Gets an iterator providing access to the entire text and attributes
	 * contained in the text editing component except for uncommitted
	 * text.
	 *
	 * @param beginIndex the index of the first character
	 * @param endIndex the index of the character following the last character
	 * @param attributes a list of attributes that the input method is
	 *  interested in
	 * @return an iterator providing access to the text and its attributes
	 */
	public AttributedCharacterIterator getCommittedText(int beginIndex, int endIndex, AttributedCharacterIterator.Attribute[] attributes) {

		return defaultAttributedCharacterIterator;

	}

	/**
	 * Gets the length of the entire text contained in the text editing
	 * component except for uncommitted (composed) text.
	 *
	 * @return the length of the text except for uncommitted text
	 */
	public int getCommittedTextLength() {

		return 0;

	}

	/**
	 * Returns the modifier keys that are currently pressed.
	 *
	 * @return the modifier keys that are currently pressed
	 */
	int getCurrentModButtons() {

		return VCLFrame.keyModifiersPressed;

	}

	/**
	 * Returns the frame pointer for this component.
	 *
	 * @return the frame pointer for this component
	 */
	int getFrame() {

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
	 * Gets the offset of the insert position in the committed text contained
	 * in the text editing component.
	 * 
	 * @return the offset of the insert position
	 */
	public int getInsertPositionOffset() {

		return 0;

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
		if ((keyCode & VCLEvent.KEY_SHIFT) == VCLEvent.KEY_SHIFT)
			buf.append("\u21e7");
		if ((keyCode & VCLEvent.KEY_CONTROLMOD) == VCLEvent.KEY_CONTROLMOD)
			buf.append("\u2303");
		if ((keyCode & VCLEvent.KEY_MOD1) == VCLEvent.KEY_MOD1)
			buf.append("\u2318");
		if ((keyCode & VCLEvent.KEY_MOD2) == VCLEvent.KEY_MOD2)
			buf.append("\u2325");

		int outCode = VCLEvent.convertVCLKeyCode(keyCode);
		if (outCode != 0)
			buf.append(KeyEvent.getKeyText(outCode));
		else
			buf.append("???");
		return buf.toString();

	}

	/**
	 * Gets the offset within the composed text for the specified absolute x
	 * and y coordinates on the screen.
	 *
	 * @param x the absolute x coordinate on screen
	 * @param y the absolute y coordinate on screen
	 * @return a text hit info describing the offset in the composed text
	 */
	public TextHitInfo getLocationOffset(int x, int y) {

		return null;

	}

	/**
	 * Returns the menubar for this native window.
	 *
	 * @return the <code>MenuBar</code> instance
	 */
	MenuBar getMenuBar() {

		return window.getMenuBar();

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
	 * Returns the <code>ComponentPeer</code>.
	 *
	 * @return the <code>ComponentPeer</code>
	 */
	public ComponentPeer getPeer() {

		return window.getPeer();

	}

	/**
	 * Gets the currently selected text from the text editing component.
	 *
	 * @param attributes a list of attributes that the input method is
	 *  interested in
	 * @return the currently selected text
	 */
	public AttributedCharacterIterator getSelectedText(AttributedCharacterIterator.Attribute[] attributes) {

		return defaultAttributedCharacterIterator;

	}

	/**
	 * Returns the state of the native window.
	 *
	 * @return the state of the native window
	 */
	public long getState() {

		int s = window.getState();
		if (s == Frame.ICONIFIED)
			return SAL_FRAMESTATE_MINIMIZED;
		else
			return SAL_FRAMESTATE_NORMAL;

	}

	/**
	 * Gets the location of a specified offset in the current composed text,
	 * or of the selection in committed text.
	 *
	 * @return a rectangle representing the screen location of the offset
	 */
	public Rectangle getTextLocation(TextHitInfo offset) {

		return defaultTextLocation;

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
	 * Invoked when the text entered through an input method has changed.
	 *
	 * @param event the input method event
	 */
	public void inputMethodTextChanged(InputMethodEvent e) {

		e.consume();

		if (queue == null || window == null || !window.isShowing())
			return;

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_EXTTEXTINPUT, this, 0));

	}

	/**
	 * Returns whether or not the native window is a floating window.
	 *
	 * @return <code>true</code> if the native window is a floating window
	 *  otherwise <code>false</code>
	 */
	public boolean isFloatingWindow() {

		return (!fullScreenMode && window.isUndecorated());

	}

	/**
	 * Returns whether or not the native window is in full screen mode.
	 *
	 * @return <code>true</code> if the component is in full screen mode and
	 *  <code>false</code> if it is in normal mode
	 */
	boolean isFullScreenMode() {

		return fullScreenMode;

	}

	/**
	 * Invoked when a key has been pressed.
	 *
	 * @param e the <code>KeyEvent</code>
	 */
	public void keyPressed(KeyEvent e) {

		e.consume();

		if (VCLFrame.lastKeyPressed != null && VCLFrame.lastKeyPressed.getID() != KeyEvent.KEY_PRESSED)
			VCLFrame.lastKeyPressed = null;

		if (queue == null || window == null || !window.isShowing())
			return;

		int keyCode = e.getKeyCode();
		if (keyCode == KeyEvent.VK_SHIFT || keyCode == KeyEvent.VK_CONTROL || keyCode == KeyEvent.VK_ALT || keyCode == KeyEvent.VK_META )
			queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_KEYMODCHANGE, this, 0));
		else if (e.isActionKey() || keyCode == KeyEvent.VK_ESCAPE || (keyCode ==KeyEvent.VK_ENTER && e.getKeyLocation() == KeyEvent.KEY_LOCATION_NUMPAD))
			queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_KEYINPUT, this, 0));
		else
			VCLFrame.lastKeyPressed = e;

		VCLFrame.keyModifiersPressed = e.getModifiers() & ~VCLFrame.keyModifiersUnpressed;

	}

	/**
	 * Invoked when a key has been released.
	 *
	 * @param e the <code>KeyEvent</code>
	 */
	public void keyReleased(KeyEvent e) {

		e.consume();

		if (queue == null || window == null || !window.isShowing())
			return;

		int keyCode = e.getKeyCode();
		if (keyCode == KeyEvent.VK_SHIFT || keyCode == KeyEvent.VK_CONTROL || keyCode == KeyEvent.VK_ALT || keyCode == KeyEvent.VK_META ) {
			queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_KEYMODCHANGE, this, 0));
		}
		else if (e.isActionKey() || keyCode == KeyEvent.VK_ESCAPE || (keyCode ==KeyEvent.VK_ENTER && e.getKeyLocation() == KeyEvent.KEY_LOCATION_NUMPAD)) {
			queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_KEYUP, this, 0));
		}

		VCLFrame.keyModifiersPressed = e.getModifiers();
		VCLFrame.keyModifiersUnpressed &= VCLFrame.keyModifiersPressed;

	}

	/**
	 * Invoked when a key has been typed.
	 *
	 * @param e the <code>KeyEvent</code>
	 */
	public void keyTyped(KeyEvent e) {

		e.consume();

		if (queue == null || window == null || !window.isShowing())
			return;

		VCLFrame.lastKeyPressed = e;

		// Fix bug 710 by stripping out the Alt modifier. Note that we do it
		// here because we need to let the Alt modifier through for action
		// keys.
		int modifiers = e.getModifiers();
		if ((modifiers & InputEvent.ALT_MASK) == InputEvent.ALT_MASK)
			e = new KeyEvent(e.getComponent(), e.getID(), e.getWhen(), modifiers & ~InputEvent.ALT_MASK, e.getKeyCode(), e.getKeyChar());

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_KEYINPUT, this, 0));
		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_KEYUP, this, 0));

	}

	/**
	 * Invoked when the mouse has been clicked on a component.
	 *
	 * @param e the <code>MouseEvent</code>
	 */
	public void mouseClicked(MouseEvent e) {

		e.consume();

	}


	/**
	 * Invoked when a mouse button has been pressed on a component.
	 *
	 * @param e the <code>MouseEvent</code>
	 */
	public void mousePressed(MouseEvent e) {

		e.consume();

		if (queue == null || window == null || !window.isShowing())
			return;

		int keyModifiers = VCLFrame.keyModifiersPressed | VCLFrame.keyModifiersUnpressed;
		if (VCLFrame.keyModifiersPressed != 0) {
			// Remove button modifiers that are really key modifiers
			int modifiers = e.getModifiers();
			if ((keyModifiers & InputEvent.CTRL_MASK) == InputEvent.CTRL_MASK && (modifiers & (InputEvent.BUTTON1_MASK | InputEvent.BUTTON2_MASK | InputEvent.CTRL_MASK)) == (InputEvent.BUTTON1_MASK | InputEvent.BUTTON2_MASK | InputEvent.CTRL_MASK)) {
				VCLFrame.keyModifiersPressed &= ~InputEvent.CTRL_MASK;
				VCLFrame.keyModifiersUnpressed |= InputEvent.CTRL_MASK;
				modifiers &= ~(InputEvent.BUTTON1_MASK | InputEvent.CTRL_MASK);
			}
			if ((keyModifiers & InputEvent.ALT_MASK) == InputEvent.ALT_MASK && (modifiers & (InputEvent.BUTTON1_MASK | InputEvent.BUTTON2_MASK)) == (InputEvent.BUTTON1_MASK | InputEvent.BUTTON2_MASK)) {
				VCLFrame.keyModifiersPressed &= ~InputEvent.ALT_MASK;
				VCLFrame.keyModifiersUnpressed |= InputEvent.ALT_MASK;
				modifiers = (modifiers & ~(InputEvent.BUTTON1_MASK | InputEvent.BUTTON2_MASK)) | InputEvent.BUTTON3_MASK;
			}
			if ((keyModifiers & InputEvent.META_MASK) == InputEvent.META_MASK && (modifiers & (InputEvent.BUTTON1_MASK | InputEvent.BUTTON3_MASK)) == (InputEvent.BUTTON1_MASK | InputEvent.BUTTON3_MASK))
				modifiers &= ~InputEvent.BUTTON3_MASK;
			if ((keyModifiers & InputEvent.SHIFT_MASK) == InputEvent.SHIFT_MASK)
				modifiers &= ~InputEvent.SHIFT_MASK;
			e = new MouseEvent(e.getComponent(), e.getID(), e.getWhen(), modifiers, e.getX(), e.getY(), e.getClickCount(), e.isPopupTrigger());
		}

		VCLFrame.mouseModifiersPressed |= e.getModifiers();

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MOUSEBUTTONDOWN, this, 0, VCLFrame.keyModifiersPressed));

	}

	/**
	 * Invoked when a mouse button has been released on a component.
	 *
	 * @param e the <code>MouseEvent</code>
	 */
	public void mouseReleased(MouseEvent e) {

		e.consume();

		if (queue == null || window == null || !window.isShowing())
			return;

		// Mac OS X only returns a single mouse released event
		e = new MouseEvent(e.getComponent(), e.getID(), e.getWhen(), VCLFrame.mouseModifiersPressed, e.getX(), e.getY(), e.getClickCount(), e.isPopupTrigger());

		VCLFrame.mouseModifiersPressed &= ~(e.getModifiers());

		synchronized (window.getTreeLock()) {
			e = preprocessMouseEvent(e);

			if (VCLFrame.lastMouseDragEvent != null) {
				// If we are releasing after dragging, send the event to the
				// last dragged frame
				Component currentComponent = e.getComponent();
				Component lastComponent = VCLFrame.lastMouseDragEvent.getComponent();
				if (lastComponent != currentComponent && lastComponent != null && currentComponent != null && lastComponent.isShowing() && currentComponent.isShowing()) {
					Point srcPoint = currentComponent.getLocationOnScreen();
					srcPoint.x += e.getX();
					srcPoint.y += e.getY();
					Point destPoint = lastComponent.getLocationOnScreen();
					e = new MouseEvent(lastComponent, e.getID(), e.getWhen(), e.getModifiers(), srcPoint.x - destPoint.x, srcPoint.y - destPoint.y, e.getClickCount(), e.isPopupTrigger());
				}

				VCLFrame.lastMouseDragEvent = null;
			}
		}

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MOUSEBUTTONUP, VCLFrame.findFrame(e.getComponent()), 0, VCLFrame.keyModifiersPressed));

	}

	/**
	 * Invoked when a mouse button is pressed on a component and then dragged.
	 *
	 * @param e the <code>MouseEvent</code>
	 */
	public void mouseDragged(MouseEvent e) {

		e.consume();

		if (queue == null || window == null || !window.isShowing())
			return;

		// Use adjusted mouse modifiers
		e = new MouseEvent(e.getComponent(), e.getID(), e.getWhen(), VCLFrame.mouseModifiersPressed, e.getX(), e.getY(), e.getClickCount(), e.isPopupTrigger());

		if (VCLFrame.lastMouseDragEvent == null)
			VCLFrame.lastMouseDragEvent = e;

		synchronized (window.getTreeLock()) {
			e = preprocessMouseEvent(e);

			// Generate mouse exited events that the OOo code expects
			Component lastComponent = VCLFrame.lastMouseDragEvent.getComponent();
			Component currentComponent = e.getComponent();
			if (lastComponent != currentComponent && lastComponent != null && currentComponent != null && lastComponent.isShowing() && currentComponent.isShowing()) {
				// If dragging and there are floating windows visible, don't let
				// the mouse fall through to a non-floating window
				if (currentComponent == panel && currentComponent != null && panel != null && !isFloatingWindow() && lastComponent.isShowing() && currentComponent.isShowing()) {
					Point srcPoint = currentComponent.getLocationOnScreen();
					srcPoint.x += e.getX();
					srcPoint.y += e.getY();
					Point destPoint = lastComponent.getLocationOnScreen();
					e = new MouseEvent(lastComponent, e.getID(), e.getWhen(), e.getModifiers(), srcPoint.x - destPoint.x, srcPoint.y - destPoint.y, e.getClickCount(), e.isPopupTrigger());
				}

				VCLFrame.lastMouseDragEvent = e;
			}
		}

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MOUSEMOVE, VCLFrame.findFrame(e.getComponent()), 0, VCLFrame.keyModifiersPressed));

	}

	/**
	 * Invoked when the mouse enters a component.
	 *
	 * @param e the <code>MouseEvent</code>
	 */
	public void mouseEntered(MouseEvent e) {

		e.consume();

		if (VCLFrame.mouseModifiersPressed != 0 || queue == null || window == null || !window.isShowing())
			return;

		// Use adjusted mouse modifiers
		e = new MouseEvent(e.getComponent(), e.getID(), e.getWhen(), VCLFrame.mouseModifiersPressed, e.getX(), e.getY(), e.getClickCount(), e.isPopupTrigger());

		synchronized (window.getTreeLock()) {
			e = preprocessMouseEvent(e);
		}

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MOUSEMOVE, VCLFrame.findFrame(e.getComponent()), 0, VCLFrame.keyModifiersPressed));

	}

	/**
	 * Invoked when the mouse exits a component.
	 *
	 * @param e the <code>MouseEvent</code>
	 */
	public void mouseExited(MouseEvent e) {

		e.consume();

		if (VCLFrame.mouseModifiersPressed != 0 || queue == null || window == null || !window.isShowing())
			return;

		// Use adjusted mouse modifiers
		e = new MouseEvent(e.getComponent(), e.getID(), e.getWhen(), VCLFrame.mouseModifiersPressed, e.getX(), e.getY(), e.getClickCount(), e.isPopupTrigger());

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MOUSELEAVE, VCLFrame.findFrame(e.getComponent()), 0, VCLFrame.keyModifiersPressed));

	}

	/**
	 * Invoked when the mouse button has been moved on a component (with no
	 * buttons pressed).
	 *
	 * @param e the <code>MouseEvent</code>
	 */
	public void mouseMoved(MouseEvent e) {

		e.consume();

		if (VCLFrame.mouseModifiersPressed != 0 || queue == null || window == null || !window.isShowing())
			return;

		// Use adjusted mouse modifiers
		e = new MouseEvent(e.getComponent(), e.getID(), e.getWhen(), VCLFrame.mouseModifiersPressed, e.getX(), e.getY(), e.getClickCount(), e.isPopupTrigger());

		synchronized (window.getTreeLock()) {
			e = preprocessMouseEvent(e);
		}

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MOUSEMOVE, VCLFrame.findFrame(e.getComponent()), 0, VCLFrame.keyModifiersPressed));

	}

	/**
	 * Invoked when the mouse wheel has been moved on a component.
	 *
	 * @param e the <code>MouseWheelEvent</code>
	 */
    public void mouseWheelMoved(MouseWheelEvent e) {

		e.consume();

		if (queue == null || window == null || !window.isShowing())
			return;

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_WHEELMOUSE, VCLFrame.findFrame(e.getComponent()), 0, VCLFrame.keyModifiersPressed));

    }

	/**
	 * Returns a <code>MouseEvent</code> for the topmost floating window above
	 * the specified <code>MouseEvent</code>.
	 *
	 * @param e the <code>MouseEvent</code>
	 * @return the <code>MouseEvent</code> for the topmost floating window
	 *  above the specified <code>MouseEvent</code>
	 */
	private MouseEvent preprocessMouseEvent(MouseEvent e) {

		if (queue == null || window == null || !window.isShowing())
			return e;

		Component c = e.getComponent();
		if (c != null) {
			// Iterate into children
			if (children.size() > 0) {
				Iterator frames = children.iterator();
				while (frames.hasNext()) {
					VCLFrame f = (VCLFrame)frames.next();
					e = f.preprocessMouseEvent(e);
					c = e.getComponent();
				}
			}

			// Evaluate event
			if (c != panel && c != null && panel != null && c.isShowing() && panel.isShowing() && isFloatingWindow() && (style & SAL_FRAME_STYLE_TOOLTIP) == 0) {
				Point srcPoint = c.getLocationOnScreen();
				srcPoint.x += e.getX();
				srcPoint.y += e.getY();
				Point destPoint = panel.getLocationOnScreen();
				Rectangle destRect = new Rectangle(destPoint.x, destPoint.y, panel.getWidth(), panel.getHeight());
				if (destRect.contains(srcPoint))
					return new MouseEvent(panel, e.getID(), e.getWhen(), e.getModifiers(), srcPoint.x - destPoint.x, srcPoint.y - destPoint.y, e.getClickCount(), e.isPopupTrigger());
			}
		}

		return e;

	}

	/**
	 * Removes a child frame.
	 *
	 * @param f the child frame.
	 */
	public void removeChild(VCLFrame f) {

		if (f != null) {
			synchronized (f) {
				synchronized (children) {
					f.parent = null;
					children.remove(f);
				}
			}
		}

	}

	/**
	 * Set the auto flush flag.
	 *
	 * @param b the auto flush flag 
	 */
	public void setAutoFlush(boolean b) {

		graphics.setAutoFlush(b);

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

		Dimension size = window.getMinimumSize();
		if (width < size.width)
			width = size.width;
		if (height < size.height)
			height = size.height;

		synchronized (window.getTreeLock()) {
			window.setBounds(x, y, width, height);
		}

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

		if (window.isUndecorated()) {
			fullScreenMode = b;
			window.setFocusableWindowState(fullScreenMode);
		}

	}

	/**
	 * Sets the menubar for this native window.
	 *
	 * @param menubar the <code>MenuBar</code> instance of <code>null</code>
	 */
	void setMenuBar(MenuBar menubar) {

		if (!window.isUndecorated())
			window.setMenuBar(menubar);

	}

	/**
	 * Sets the minimum client size.
	 *
	 * @param width the minimum width
	 * @param height the minimum height
	 */
	public void setMinClientSize(int width, int height) {

		window.setMinimumSize(width, height);

	}

	/**
	 * Sets the cursor.
	 *
	 * @param style the cursor style constant
	 */
	public void setPointer(int style) {

		Cursor c = (Cursor)VCLFrame.customCursors.get(new Integer(style));

		if (c == null)
		{
			int cursor = Cursor.DEFAULT_CURSOR;
			switch (style) {
				case POINTER_WAIT:
					cursor = Cursor.WAIT_CURSOR;
					break;
				case POINTER_TEXT:
					cursor = Cursor.TEXT_CURSOR;
					break;
				case POINTER_CROSS:
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
				case POINTER_VSPLIT:
				case POINTER_VSIZEBAR:
				case POINTER_WINDOW_SSIZE:
					cursor = Cursor.S_RESIZE_CURSOR;
					break;
				case POINTER_WSIZE:
				case POINTER_WINDOW_WSIZE:
					cursor = Cursor.W_RESIZE_CURSOR;
					break;
				case POINTER_ESIZE:
				case POINTER_HSPLIT:
				case POINTER_HSIZEBAR:
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
				case POINTER_REFHAND:
					cursor = Cursor.HAND_CURSOR;
					break;
				case POINTER_TEXT_VERTICAL:
				case POINTER_NULL:
					cursor = Cursor.TEXT_CURSOR;
					break;
				case POINTER_ARROW:
				case POINTER_HELP:
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
					cursor = Cursor.DEFAULT_CURSOR;
					break;
			}

			c = Cursor.getPredefinedCursor(cursor);
		}

		if (c != null && window != null) {
			window.setCursor(c);
			synchronized (window.getTreeLock()) {
				Toolkit.getDefaultToolkit().sync();
			}
		}

	}

	/**
	 * Sets the state of the native window.
	 *
	 * @param state the state of the native window
	 */
	public void setState(long state) {

		if (window.isShowing()) {
			// Only invoke Frame.setState() if the state needs to be changed
			// as this method can cause a deadlock with the native menu handler
			// on Mac OS X
			int s = Frame.NORMAL;
			if (state == SAL_FRAMESTATE_MINIMIZED)
				s = Frame.ICONIFIED;
 			if (((Frame)window).getState() != s)
				((Frame)window).setState(s);
		}

	}

	/**
	 * Sets the title for this native window to the specified string.
	 *
	 * @param title the title to be displayed in the native window's border
	 */
	public void setTitle(String title) {

		window.setTitle(title);

	}

	/**
	 * Shows or hides the native window.
	 *
	 * @param b <code>true</code> shows this component and <code>false</code>
	 *  hides this component
	 */
	public void setVisible(boolean b) {

		if (b == window.isShowing())
			return;

		// Set the resizable flag if needed
		window.setResizable(resizable);

		if (b) {
			// Register listeners
			window.addComponentListener(this);
			panel.addFocusListener(this);
			panel.addKeyListener(this);
			panel.addInputMethodListener(this);
			panel.addMouseListener(this);
			panel.addMouseMotionListener(this);
			panel.addMouseWheelListener(this);
			window.addWindowListener(this);

			// Show the window
			window.show();

			// Focus doesn't seem to always get set to the panel for focusable
			// borderless windows
			if (fullScreenMode)
				panel.requestFocus();
		}
		else {
			// Hide the window
			window.hide();
		}

	}

	/**
	 * Brings the native window to the front.
	 */
	public void toFront() {

		if (window.isShowing())
			window.toFront();

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

		if (queue == null || window == null || !window.isShowing())
			return;

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
	public void windowIconified(WindowEvent e) {

		// Force the C++ code to hide all child frames by posting a resize event
		if (queue == null || window == null || !window.isShowing())
			return;

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MOVERESIZE, this, 0));

	}

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
	public void windowActivated(WindowEvent e) {

		// Fix bug 375 by resetting cached events and modifiers
		VCLFrame.keyModifiersPressed = 0;
		VCLFrame.lastMouseDragEvent = null;
		VCLFrame.mouseModifiersPressed = 0;

	}

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
	final class NoPaintFrame extends Frame {

		/**
		 * The <code>VCLFrame</code>.
		 */
		private VCLFrame frame = null;

		/**
		 * The minimum size.
		 */
		private Dimension minSize = null;

		/**
		 * Constructs a new <code>VCLFrame.NoPaintFrame</code> instance.
		 *
		 * @param f the <code>VCLFrame</code>
		 */
		NoPaintFrame(VCLFrame f) {

			frame = f;
			setMinimumSize(1, 1);

		}

		/**
		 * Returns the minimum size for the frame.
		 *
		 * @return the minimum size for the frame
		 */
		public Dimension getMinimumSize() {

			return minSize;
			
		}

		/**
		 * Set the minimum size for the frame.
		 *
		 * @param width the minimum width
		 * @param height the minimum height
		 */
		public void setMinimumSize(int width, int height) {

			if (width < 1)
				width = 1;
			if (height < 1)
				height = 1;

			insets = getInsets();
			minSize = new Dimension(width + insets.left + insets.right, height + insets.top + insets.bottom);
			
		}

		/**
		 * This method performs no painting of the frame. This method is used
		 * to prevent Java from painting over what VCL has painted.
		 *
		 * @param g the <code>Graphics</code>
		 */
		public void update(Graphics g) {

			paint(g);

		}

	}

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
			setBackground(Color.white);
			enableInputMethods(true);

		}

		/**
		 * Returns the input method request handler which supports requests
		 * from input methods for this component.
		 *
		 * @return the input method request handler for this component
		 */
		public InputMethodRequests getInputMethodRequests() {

			return frame;

		}

		/**
		 * This method performs no painting of the panel. This method is used
		 * to prevent Java from painting over what VCL has painted.
		 *
		 * @param g the <code>Graphics</code>
		 */
		public void paint(Graphics g) {

            Rectangle bounds;
            Shape clip = g.getClip();
            if (clip != null)
                bounds = clip.getBounds();
            else
                bounds = ((Graphics2D)g).getDeviceConfiguration().getBounds();

			VCLGraphics graphics = frame.getGraphics();
			if (graphics != null) {
				synchronized (graphics) {
					graphics.addToFlush(bounds);
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
