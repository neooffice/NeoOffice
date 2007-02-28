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
import java.awt.Dialog;
import java.awt.Dimension;
import java.awt.EventQueue;
import java.awt.Frame;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.Insets;
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
import java.awt.event.PaintEvent;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.awt.event.WindowStateListener;
import java.awt.font.TextHitInfo;
import java.awt.im.InputContext;
import java.awt.im.InputMethodRequests;
import java.awt.image.BufferedImage;
import java.awt.peer.ComponentPeer;
import java.lang.reflect.Method;
import java.net.URL;
import java.text.CharacterIterator;
import java.text.AttributedCharacterIterator;
import java.text.AttributedString;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;

/**
 * The Java class that implements the SalFrame C++ class methods.
 * <p>
 * @version		$Revision$ $Date$
 * @author		$Author$
 */
public final class VCLFrame implements ComponentListener, FocusListener, KeyListener, InputMethodListener, InputMethodRequests, MouseListener, MouseMotionListener, MouseWheelListener, WindowListener, WindowStateListener {

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
	 * POINTER_PIVOT_DELETE constant.
	 */
	public final static int POINTER_PIVOT_DELETE = 87;

	/**
	 * POINTER_TAB_SELECT_S constant.
	 */
	public final static int POINTER_TAB_SELECT_S = 88;

	/**
	 * POINTER_TAB_SELECT_E constant.
	 */
	public final static int POINTER_TAB_SELECT_E = 89;

	/**
	 * POINTER_TAB_SELECT_SE constant.
	 */
	public final static int POINTER_TAB_SELECT_SE = 90;

	/**
	 * POINTER_TAB_SELECT_W constant.
	 */
	public final static int POINTER_TAB_SELECT_W = 91;

	/**
	 * POINTER_TAB_SELECT_SW constant.
	 */
	public final static int POINTER_TAB_SELECT_SW = 92;

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
	 * Find the matching <code>VCLFrame</code> for the specified component.
	 *
	 * @param c the component
	 * @return the matching <code>VCLFrame</code>
	 */
	static synchronized VCLFrame findFrame(Component c) {

		return (VCLFrame)componentMap.get(c);

	}

	/**
	 * Initialize static data members.
	 */
	static {

		Toolkit toolkit = Toolkit.getDefaultToolkit();
		ClassLoader cl = ClassLoader.getSystemClassLoader();
		customCursors = new HashMap();

		// Create POINTER_NULL
		URL url = cl.getSystemResource("com/sun/star/vcl/images/nullptr.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(0, 0), "POINTER_NULL");
			if (c != null)
				customCursors.put(new Integer(POINTER_NULL), c);
		}

/*
		// TODO: Find the real cursor hot spot point and add the remaining
		// cursor files.

		// Create POINTER_TAB_SELECT_S
		url = cl.getSystemResource("com/sun/star/vcl/images/tblsels.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(img.getWidth(null) / 2, img.getHeight(null)), "POINTER_TAB_SELECT_S");
			if (c != null)
				customCursors.put(new Integer(POINTER_TAB_SELECT_S), c);
		}

		// Create POINTER_TAB_SELECT_E
		url = cl.getSystemResource("com/sun/star/vcl/images/tblsele.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(img.getWidth(null), img.getHeight(null) / 2), "POINTER_TAB_SELECT_E");
			if (c != null)
				customCursors.put(new Integer(POINTER_TAB_SELECT_E), c);
		}

		// Create POINTER_TAB_SELECT_SE
		url = cl.getSystemResource("com/sun/star/vcl/images/tblselse.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(img.getWidth(null), img.getHeight(null)), "POINTER_TAB_SELECT_SE");
			if (c != null)
				customCursors.put(new Integer(POINTER_TAB_SELECT_SE), c);
		}

		// Create POINTER_TAB_SELECT_W
		url = cl.getSystemResource("com/sun/star/vcl/images/tblselw.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(0, img.getHeight(null) / 2), "POINTER_TAB_SELECT_W");
			if (c != null)
				customCursors.put(new Integer(POINTER_TAB_SELECT_W), c);
		}

		// Create POINTER_TAB_SELECT_SW
		url = cl.getSystemResource("com/sun/star/vcl/images/tblselsw.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(0, img.getHeight(null)), "POINTER_TAB_SWLECT_SE");
			if (c != null)
				customCursors.put(new Integer(POINTER_TAB_SELECT_SW), c);
		}
*/

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
	 * The disposed flag.
	 */
	private boolean disposed = false;

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
	 * The ignore mouse released modifiers.
	 */
	private int ignoreMouseReleasedModifiers = 0;

	/**
	 * The native window's insets.
	 */
	private Insets insets = null;

	/**
	 * The last uncommitted input method event.
	 */
	private InputMethodEvent lastUncommittedInputMethodEvent = null;

	/**
	 * The native window's panel.
	 */
	private VCLFrame.NoPaintPanel panel = null;

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
	private Window window = null;

	/** 
	 * The window undecorated mode.
	 */
	private boolean undecorated = false;

	/**
	 * Constructs a new <code>VCLFrame</code> instance.
	 *
	 * @param s the SAL_FRAME_STYLE flags
	 * @param q the event queue to post events to
	 * @param f the frame pointer
	 * @param p the parent frame
	 */
	public VCLFrame(long s, VCLEventQueue q, long f, VCLFrame p) {

		queue = q;
		frame = f;
		style = s;

		// Create the native window
		if ((style & (SAL_FRAME_STYLE_DEFAULT | SAL_FRAME_STYLE_MOVEABLE | SAL_FRAME_STYLE_SIZEABLE)) == 0)
			undecorated = true;

		Window w = null;
		if (p != null)
			w = p.getWindow();
		if (w instanceof Dialog)
			window = new VCLFrame.NoPaintDialog(this, (Dialog)w);
		else if (w instanceof Frame)
			window = new VCLFrame.NoPaintDialog(this, (Frame)w);
		else
			window = new VCLFrame.NoPaintFrame(this);

		// Process remaining style flags
		if ((style & SAL_FRAME_STYLE_SIZEABLE) != 0)
			resizable = true;

		// Add a panel as the only component
		panel = new VCLFrame.NoPaintPanel(this);
		window.add(panel);
		bitCount = panel.getColorModel().getPixelSize();
		if (bitCount <= 4)
			bitCount = 4;
		else if (bitCount <= 8)
			bitCount = 8;
		else if (bitCount <= 16)
			bitCount = 16;
		else if (bitCount <= 24)
			bitCount = 24;
		else
			bitCount = 32;

		if (undecorated)
			insets = window.getInsets();
		else
			insets = VCLScreen.getFrameInsets();
		graphics = new VCLGraphics(this);

		// Register listeners
		panel.addFocusListener(this);
		panel.addInputMethodListener(this);
		panel.addKeyListener(this);
		panel.addMouseListener(this);
		panel.addMouseMotionListener(this);
		panel.addMouseWheelListener(this);
		window.addComponentListener(this);
		window.addFocusListener(this);
		window.addInputMethodListener(this);
		window.addKeyListener(this);
		window.addWindowListener(this);
		window.addWindowStateListener(this);

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

		return null;

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
	public synchronized void componentResized(ComponentEvent e) {

		if (disposed || !window.isShowing())
			return;

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MOVERESIZE, this, 0));

	}

	/**
	 * Invoked when the the native window's position changes.
	 *
	 * @param e the <code>ComponentEvent</code>
	 */
	public synchronized void componentMoved(ComponentEvent e) {

		if (disposed || !window.isShowing())
			return;

		// The OOo code may move floating windows while dragging its
		// lightweight title bar so we need to ignore move events associated
		// with that action as they will be out of sync with what the OOo
		// code expects
		if (isFloatingWindow() && queue.getLastAdjustedMouseModifiers() != 0)
			return;

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MOVERESIZE, this, 0));

	}

	/**
	 * Invoked when the native window has been made visible.
	 *
	 * @param e the <code>ComponentEvent</code>
	 */
	public synchronized void componentShown(ComponentEvent e) {

		if (disposed || !window.isShowing())
			return;

		// Add window and panel to mapping
		synchronized (VCLFrame.class) {
			VCLFrame.componentMap.put(window, this);
			VCLFrame.componentMap.put(panel, this);
		}

	}

	/**
	 * Invoked when the native window has been made invisible.
	 *
	 * @param e the <code>ComponentEvent</code>
	 */
	public synchronized void componentHidden(ComponentEvent e) {

		if (disposed)
			return;

		lastUncommittedInputMethodEvent = null;

		// Remove window and panel from mapping
		synchronized (VCLFrame.class) {
			VCLFrame.componentMap.remove(window);
			VCLFrame.componentMap.remove(panel);
		}

	}

	/**
	 * Adds a child frame.
	 *
	 * @param f the child frame.
	 */
	public synchronized void addChild(VCLFrame f) {

		if (f != null)
			children.add(f);

	}

	/**
	 * Disposes the native window and releases any system resources that it is
	 * using.
	 */
	public synchronized void dispose() {

		if (disposed)
			return;

		setVisible(false, false);
		setMenuBar(null);
		children = null;
		graphics.dispose();
		graphics = null;
		insets = null;
		lastUncommittedInputMethodEvent = null;

		// Unregister listeners
		panel.removeFocusListener(this);
		panel.removeInputMethodListener(this);
		panel.removeKeyListener(this);
		panel.removeMouseListener(this);
		panel.removeMouseMotionListener(this);
		panel.removeMouseWheelListener(this);
		window.removeComponentListener(this);
		window.removeFocusListener(this);
		window.removeInputMethodListener(this);
		window.removeKeyListener(this);
		window.removeWindowListener(this);
		window.removeWindowStateListener(this);
		queue.removeCachedEvents(frame);

		panel = null;
		queue = null;
		window = null;

		disposed = true;

	}

	/**
	 * Invoked when the native window has gained focus.
	 *
	 * @param e the <code>FocusEvent</code>
	 */
	public synchronized void focusGained(FocusEvent e) {

		if (disposed || isFloatingWindow() || !window.isShowing())
			return;

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_GETFOCUS, this, 0));

	}

	/**
	 * Invoked when the native window has lost focus.
	 *
	 * @param e the <code>FocusEvent</code>
	 */
	public synchronized void focusLost(FocusEvent e) {

		if (disposed || isFloatingWindow() || !window.isShowing())
			return;

		// Fix bug 1645 by ensuring that we don't lose focus to a floating
		// window
		VCLFrame f = findFrame(e.getOppositeComponent());
		if (f != null) {
			synchronized (f) {
				if (f.isFloatingWindow()) {
					requestFocus();
					return;
				}
			}
		}

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

		Rectangle bounds = window.getBounds();

		// Fix bugs 1479 and 1444 by using the window's location on the screen
		// as the cached bounds may be incorrect with certain multiple monitor
		// configurations
		if (window.isShowing()) {
			Point p = window.getLocationOnScreen();
			if (p != null && (p.x != bounds.x || p.y != bounds.y))
				bounds = new Rectangle(p.x, p.y, bounds.width, bounds.height);
		}

		return bounds;

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

		return VCLFrame.defaultAttributedCharacterIterator;

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
	 * Returns the frame pointer for this component.
	 *
     * @return the frame pointer for this component
	 */
	synchronized long getFrame() {

		return frame;

	}

	/**
	 * Returns the graphics context for this component.
	 *
	 * @return the graphics context for this component
	 */
	public synchronized VCLGraphics getGraphics() {

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

		if (window instanceof Frame)
			return ((Frame)window).getMenuBar();
		else
			return null;

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

		return VCLFrame.defaultAttributedCharacterIterator;

	}

	/**
	 * Returns the state of the native window.
	 *
	 * @return the state of the native window
	 */
	public long getState() {

		
		if (window instanceof Frame) {
			int s = ((Frame)window).getExtendedState();
			if ((s & Frame.ICONIFIED) != 0)
				return SAL_FRAMESTATE_MINIMIZED;
			else if ((s & (Frame.MAXIMIZED_BOTH | Frame.MAXIMIZED_HORIZ | Frame.MAXIMIZED_VERT)) != 0)
				return SAL_FRAMESTATE_MAXIMIZED;
		}

		return SAL_FRAMESTATE_NORMAL;

	}

	/**
	 * Gets the location of a specified offset in the current composed text,
	 * or of the selection in committed text.
	 *
	 * @return a rectangle representing the screen location of the offset
	 */
	public Rectangle getTextLocation(TextHitInfo offset) {

		Rectangle bounds = new Rectangle(300, 300, 2, 12);

		synchronized (this) {
			if (disposed || !window.isShowing())
				return bounds;
		}

		// Get the position from the OOo code
		Rectangle b = getTextLocation0(frame);

		synchronized (this) {
			if (disposed || !window.isShowing())
				return bounds;

			if (b != null) {
				Point loc = window.getLocationOnScreen();
				b.x += loc.x;
				b.y += loc.y;
				bounds = b;
			}
		}

		return bounds;

	}

	/**
	 * Gets the bounds of the text cursor.
	 *
	 * @param f the frame pointer
	 * @return the bounds of the text cursor
	 */
	native Rectangle getTextLocation0(long f);

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
	public synchronized void inputMethodTextChanged(InputMethodEvent e) {

		e.consume();

		if (disposed || !window.isShowing())
			return;

		AttributedCharacterIterator text = e.getText();
		if (text == null)
			return;

		int count = 0;
		for (char c = text.first(); c != CharacterIterator.DONE; c = text.next())
			count++;

		if (count == 0) {
			// Fix bug 1429 by committing last uncommitted text if there is no
			// text in this event. Since this code assumes that uncommitted text
			// is never cancelled, there are fixes in the C++ code that post
			// input method events with the text set to null to indicate that
			// the uncommitted text should be cancelled.
			if (lastUncommittedInputMethodEvent != null) {
				AttributedCharacterIterator lastText = lastUncommittedInputMethodEvent.getText();
				boolean commitLast = false;
				int lastCount = 0;
				for (char c = lastText.first(); c != CharacterIterator.DONE; c = lastText.next()) {
					// Only commit if there are uncommitted Indic characters
					if (!commitLast && c >= 0x0900 && c < 0x0E00)
						commitLast = true;
					lastCount++;
				}

				if (commitLast)
					e = new InputMethodEvent((Component)lastUncommittedInputMethodEvent.getSource(), lastUncommittedInputMethodEvent.getID(), lastUncommittedInputMethodEvent.getWhen(), lastText, lastCount, lastUncommittedInputMethodEvent.getCaret(), lastUncommittedInputMethodEvent.getVisiblePosition());
				lastUncommittedInputMethodEvent = null;
			}
		}
		else if (count == 1 && text.first() == ' ') {
			e = new InputMethodEvent((Component)e.getSource(), e.getID(), e.getWhen(), defaultAttributedCharacterIterator, 0, TextHitInfo.beforeOffset(0), TextHitInfo.beforeOffset(0));
			lastUncommittedInputMethodEvent = null;
		}
		else if (count > e.getCommittedCharacterCount()) {
			lastUncommittedInputMethodEvent = e;
		}
		else {
			lastUncommittedInputMethodEvent = null;
		}

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_EXTTEXTINPUT, this, 0));

	}

	/**
	 * Returns whether or not the frame has been disposed.
	 *
	 * @return <code>true</code> if the frame has been disposed otherwise
	 *  <code>false</code>
	 */
	public boolean isDisposed() {

		return disposed;

	}

	/**
	 * Returns whether or not the native window is a floating window.
	 *
	 * @return <code>true</code> if the native window is a floating window
	 *  otherwise <code>false</code>
	 */
	public boolean isFloatingWindow() {

		return (undecorated && !fullScreenMode);

	}

	/**
	 * Returns whether or not the native window is in full screen mode.
	 *
	 * @return <code>true</code> if the component is in full screen mode and
	 *  <code>false</code> if it is in normal mode
	 */
	synchronized boolean isFullScreenMode() {

		return fullScreenMode;

	}

	/**
	 * Invoked when a key has been pressed.
	 *
	 * @param e the <code>KeyEvent</code>
	 */
	public synchronized void keyPressed(KeyEvent e) {

		e.consume();

		if (disposed || !window.isShowing())
			return;

		int keyCode = e.getKeyCode();
		if (keyCode == KeyEvent.VK_SHIFT || keyCode == KeyEvent.VK_CONTROL || keyCode == KeyEvent.VK_ALT || keyCode == KeyEvent.VK_META) {
			VCLEvent keyModChangeEvent = new VCLEvent(e, VCLEvent.SALEVENT_KEYMODCHANGE, this, 0);
			queue.postCachedEvent(keyModChangeEvent);
		}
		else if (e.isActionKey() || keyCode == KeyEvent.VK_BACK_SPACE || keyCode == KeyEvent.VK_ENTER || keyCode == KeyEvent.VK_DELETE || keyCode == KeyEvent.VK_ESCAPE || (e.getKeyLocation() == KeyEvent.KEY_LOCATION_NUMPAD && e.getModifiersEx() == InputEvent.META_DOWN_MASK && keyCode == KeyEvent.VK_MULTIPLY)) {
			queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_KEYINPUT, this, 0));
		}

	}

	/**
	 * Invoked when a key has been released.
	 *
	 * @param e the <code>KeyEvent</code>
	 */
	public synchronized void keyReleased(KeyEvent e) {

		e.consume();

		if (disposed || !window.isShowing())
			return;

		int keyCode = e.getKeyCode();
		if (keyCode == KeyEvent.VK_SHIFT || keyCode == KeyEvent.VK_CONTROL || keyCode == KeyEvent.VK_ALT || keyCode == KeyEvent.VK_META) {
			VCLEvent keyModChangeEvent = new VCLEvent(e, VCLEvent.SALEVENT_KEYMODCHANGE, this, 0);
			queue.postCachedEvent(keyModChangeEvent);
		}
		else if (e.isActionKey() || keyCode == KeyEvent.VK_BACK_SPACE || keyCode == KeyEvent.VK_ENTER || keyCode == KeyEvent.VK_DELETE || keyCode == KeyEvent.VK_ESCAPE || (e.getKeyLocation() == KeyEvent.KEY_LOCATION_NUMPAD && e.getModifiersEx() == InputEvent.META_DOWN_MASK && keyCode == KeyEvent.VK_MULTIPLY)) {
			queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_KEYUP, this, 0));
		}

	}

	/**
	 * Invoked when a key has been typed.
	 *
	 * @param e the <code>KeyEvent</code>
	 */
	public synchronized void keyTyped(KeyEvent e) {

		e.consume();

		if (disposed || !window.isShowing())
			return;

		// These are handled in the key pressed and released events.
		char keyChar = e.getKeyChar();
		int modifiers = e.getModifiersEx();
		if (keyChar == (char)0x08 || keyChar == (char)0x0a || keyChar == (char)0x7f)
			return;

		// Fix bug 710 by stripping out the Alt modifier. Note that we do it
		// here because we need to let the Alt modifier through for action
		// keys.
		if ((modifiers & InputEvent.ALT_DOWN_MASK) != 0)
			e = new KeyEvent(e.getComponent(), e.getID(), e.getWhen(), (e.getModifiers() | modifiers) & ~(InputEvent.ALT_MASK | InputEvent.ALT_DOWN_MASK), e.getKeyCode(), keyChar);

		// Fix bug 1143 by converting any capital alpha characters to lowercase
		// when the meta key is pressed
		if ((modifiers & InputEvent.META_DOWN_MASK) != 0) {
			keyChar = e.getKeyChar();
			if (keyChar >= 'A' && keyChar <= 'Z')
				e = new KeyEvent(e.getComponent(), e.getID(), e.getWhen(), e.getModifiers() | modifiers, e.getKeyCode(), (char)(keyChar + 32));
		}

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

		if (disposed || !window.isShowing())
			return;

	}


	/**
	 * Invoked when a mouse button has been pressed on a component.
	 *
	 * @param e the <code>MouseEvent</code>
	 */
	public synchronized void mousePressed(MouseEvent e) {

		e.consume();

		if (disposed || !window.isShowing())
			return;

		// The JVM can get confused when we click on a non-focused window. In
		// these cases, we will receive no mouse move events so if the OOo code
		// displays a popup menu, the popup menu will receive no mouse move
		// events.
		if (!isFloatingWindow() && !window.isFocused()) {
			int modifiers = e.getModifiersEx();
			if (modifiers != InputEvent.BUTTON1_DOWN_MASK) {
				ignoreMouseReleasedModifiers = e.getModifiersEx();
				return;
			}
		}

		ignoreMouseReleasedModifiers = 0;

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MOUSEBUTTONDOWN, this, 0));

	}

	/**
	 * Invoked when a mouse button has been released on a component.
	 *
	 * @param e the <code>MouseEvent</code>
	 */
	public synchronized void mouseReleased(MouseEvent e) {

		e.consume();

		if (disposed || !window.isShowing())
			return;

		// Use adjusted modifiers
		int modifiers = queue.getLastAdjustedMouseModifiers();
		e = new MouseEvent(e.getComponent(), e.getID(), e.getWhen(), e.getModifiers() | modifiers, e.getX(), e.getY(), e.getClickCount(), e.isPopupTrigger());

		// The JVM can get confused when we click on a non-focused window. In
		// these cases, we will receive no mouse move events so if the OOo code
		// displays a popup menu, the popup menu will receive no mouse move
		// events.
		if (ignoreMouseReleasedModifiers != 0 && (ignoreMouseReleasedModifiers & modifiers) == modifiers) {
			ignoreMouseReleasedModifiers &= ~modifiers;
			return;
		}

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MOUSEBUTTONUP, VCLFrame.findFrame(e.getComponent()), 0));

	}

	/**
	 * Invoked when a mouse button is pressed on a component and then dragged.
	 *
	 * @param e the <code>MouseEvent</code>
	 */
	public synchronized void mouseDragged(MouseEvent e) {

		e.consume();

		if (disposed || !window.isShowing())
			return;

		// Use adjusted modifiers
		int modifiers = queue.getLastAdjustedMouseModifiers();
		e = new MouseEvent(e.getComponent(), e.getID(), e.getWhen(), e.getModifiers() | modifiers, e.getX(), e.getY(), e.getClickCount(), e.isPopupTrigger());

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MOUSEMOVE, VCLFrame.findFrame(e.getComponent()), 0));

	}

	/**
	 * Invoked when the mouse enters a component.
	 *
	 * @param e the <code>MouseEvent</code>
	 */
	public synchronized void mouseEntered(MouseEvent e) {

		e.consume();

		if (disposed || !window.isShowing())
			return;

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MOUSEMOVE, VCLFrame.findFrame(e.getComponent()), 0));

	}

	/**
	 * Invoked when the mouse exits a component.
	 *
	 * @param e the <code>MouseEvent</code>
	 */
	public synchronized void mouseExited(MouseEvent e) {

		e.consume();

		if (disposed || !window.isShowing())
			return;

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MOUSELEAVE, VCLFrame.findFrame(e.getComponent()), 0));

	}

	/**
	 * Invoked when the mouse button has been moved on a component (with no
	 * buttons pressed).
	 *
	 * @param e the <code>MouseEvent</code>
	 */
	public synchronized void mouseMoved(MouseEvent e) {

		e.consume();

		if (disposed || !window.isShowing())
			return;

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MOUSEMOVE, VCLFrame.findFrame(e.getComponent()), 0));

	}

	/**
	 * Invoked when the mouse wheel has been moved on a component.
	 *
	 * @param e the <code>MouseWheelEvent</code>
	 */
    public synchronized void mouseWheelMoved(MouseWheelEvent e) {

		e.consume();

		if (disposed || !window.isShowing())
			return;

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_WHEELMOUSE, VCLFrame.findFrame(e.getComponent()), 0));

    }

	/**
	 * Post a paint event.
	 *
	 * @param b the bounds to paint
	 */
	synchronized void paint(Rectangle b) {

		if (disposed || !window.isShowing() || b.isEmpty())
			return;

		queue.postCachedEvent(new VCLEvent(new PaintEvent(panel, PaintEvent.UPDATE, b), VCLEvent.SALEVENT_PAINT, this, 0));

	}

	/**
	 * Removes a child frame.
	 *
	 * @param f the child frame.
	 */
	public synchronized void removeChild(VCLFrame f) {

		if (f != null)
			children.remove(f);

	}

	/**
	 * Set focus to the native window.
	 *
	 * @return <code>true</code> is the window was brought to the front else
	 *  <code>false</code>
	 */
	public boolean requestFocus() {

		if (window.isShowing() && !isFloatingWindow()) {
			panel.requestFocusInWindow();
			return true;
		}
		else {
			return false;
		}

	}

	/**
	 * Moves and resizes this native window.
	 *
	 * @param x the new x-coordinate
	 * @param y the new y-coordinate
	 * @param width the new width
	 * @param height the new height
	 */
	public synchronized void setBounds(int x, int y, int width, int height) {

		Dimension size = window.getMinimumSize();
		if (width < size.width)
			width = size.width;
		if (height < size.height)
			height = size.height;

		window.setBounds(x, y, width, height);

	}

	/**
	 * Set the native window to show or hide in full screen mode.
	 *
	 * @param b <code>true</code> sets this component full screen mode and
	 *  <code>false</code> sets it to normal mode
	 */
	public synchronized void setFullScreenMode(boolean b) {

		if (b == fullScreenMode)
			return;

		fullScreenMode = b;

		if (window instanceof VCLFrame.NoPaintDialog)
			((VCLFrame.NoPaintDialog)window).setFullScreenMode(fullScreenMode);
		else if (window instanceof VCLFrame.NoPaintFrame)
			((VCLFrame.NoPaintFrame)window).setFullScreenMode(fullScreenMode);

	}

	/**
	 * Sets the menubar for this native window.
	 *
	 * @param menubar the <code>MenuBar</code> instance of <code>null</code>
	 */
	void setMenuBar(MenuBar menubar) {

		if (window instanceof Frame) {
			// The menubar doesn't refresh when a child window until the focus
			// is restored so hide any children while changing the menubar
			LinkedList detachedChildren = new LinkedList();
			if (menubar != null) {
				// Detach any visible children
				Iterator frames = children.iterator();
				while (frames.hasNext()) {
					VCLFrame f = (VCLFrame)frames.next();
					synchronized (f) {
						if (!f.isDisposed()) {
							Window w = f.getWindow();
							if (w.isShowing()) {
								f.setVisible(false, false);
								detachedChildren.add(f);
							}
						}
					}
				}
			}

			((Frame)window).setMenuBar(menubar);

			// Reattach any visible children
			Iterator frames = detachedChildren.iterator();
			while (frames.hasNext()) {
				VCLFrame f = (VCLFrame)frames.next();
				synchronized (f) {
					if (!f.isDisposed())
						f.setVisible(true, true);
				}
			}
		}

	}

	/**
	 * Sets the minimum client size.
	 *
	 * @param width the minimum width
	 * @param height the minimum height
	 */
	public void setMinClientSize(int width, int height) {

		if (window instanceof VCLFrame.NoPaintFrame)
			((VCLFrame.NoPaintFrame)window).setMinimumSize(width, height);
		else if (window instanceof VCLFrame.NoPaintDialog)
			((VCLFrame.NoPaintDialog)window).setMinimumSize(width, height);

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
				case POINTER_PIVOT_DELETE:
				case POINTER_TAB_SELECT_S:
				case POINTER_TAB_SELECT_E:
				case POINTER_TAB_SELECT_SE:
				case POINTER_TAB_SELECT_W:
				case POINTER_TAB_SELECT_SW:
					cursor = Cursor.DEFAULT_CURSOR;
					break;
			}

			c = Cursor.getPredefinedCursor(cursor);
		}

		if (c != null) {
			window.setCursor(c);
			Toolkit.getDefaultToolkit().sync();
		}

	}

	/**
	 * Sets the state of the native window.
	 *
	 * @param state the state of the native window
	 */
	public synchronized void setState(long state) {

		if (window instanceof Frame && window.isShowing()) {
			// Only invoke Frame.setState() if the state needs to be changed
			// as this method can cause a deadlock with the native menu handler
			// on Mac OS X
			int s;
			if (state == SAL_FRAMESTATE_MINIMIZED)
				s = Frame.ICONIFIED;
			else if (state == SAL_FRAMESTATE_MAXIMIZED)
				s = Frame.MAXIMIZED_BOTH;
			else
				s = Frame.NORMAL;
			((Frame)window).setExtendedState(s);
		}

	}

	/**
	 * Sets the title for this native window to the specified string.
	 *
	 * @param title the title to be displayed in the native window's border
	 */
	public void setTitle(String title) {

		if (window instanceof Dialog)
			((Dialog)window).setTitle(title);
		else if (window instanceof Frame)
			((Frame)window).setTitle(title);

	}

	/**
	 * Shows or hides the native window.
	 *
	 * @param b <code>true</code> shows this component and <code>false</code>
	 *  hides this component
	 * @param noActivate <code>true</code> displays the window without giving
	 *  it focus
	 */
	public synchronized void setVisible(boolean b, boolean noActivate) {

		if (b == window.isShowing())
			return;

		// Set the resizable flag if needed
		if (window instanceof Dialog)
			((Dialog)window).setResizable(resizable);
		else if (window instanceof Frame)
			((Frame)window).setResizable(resizable);

		graphics.notifyGraphicsChanged();

		if (b) {
			// Fix bug 1012 by deiconifying the parent window
			Window w = window;
			VCLFrame f = this;
			while (f != null) {
				w = w.getOwner();
 				if (w == null || !w.isShowing())
					break;
				f = VCLFrame.findFrame(w);
				if (f != null) {
 					if (f.getState() == SAL_FRAMESTATE_MINIMIZED)
						f.setState(SAL_FRAMESTATE_NORMAL);
				}
			}

			// Show the window
			boolean focusable;
			if (noActivate)
				focusable = window.isFocusable();
			else
				focusable = false;
			if (focusable) {
				window.setFocusable(false);
				window.setFocusableWindowState(false);
			}
			window.show();
			if (focusable) {
				window.setFocusable(true);
				window.setFocusableWindowState(true);
			}
		}
		else {
			// Hide the window
			window.hide();
		}

	}

	/**
	 * Syncs the native window.
	 */
	public void sync() {

		if (!window.isShowing())
			return;

		Toolkit.getDefaultToolkit().sync();

	}

	/**
	 * Brings the native window to the front.
	 *
	 * @return <code>true</code> is the window was brought to the front else
	 *  <code>false</code>
	 */
	public boolean toFront() {

		if (window.isShowing() && !isFloatingWindow()) {
			window.toFront();
			panel.requestFocusInWindow();
			return true;
		}
		else {
			return false;
		}

	}

	/**
	 * Force the native window to update its cached screen location.
	 *
	 * @param p the <code>ComponentPeer</code>
	 */
	public native void updateLocation(ComponentPeer p);

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
	public synchronized void windowClosing(WindowEvent e) {

		if (disposed || !window.isShowing())
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
	public synchronized void windowIconified(WindowEvent e) {

		if (disposed || !window.isShowing())
			return;

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_MINIMIZED, this, 0));

	}

	/**
	 * Invoked when a window is changed from a minimized to a normal state.
	 *
	 * @param e the <code>WindowEvent</code>
	 */
	public synchronized void windowDeiconified(WindowEvent e) {

		if (disposed || !window.isShowing())
			return;

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_DEMINIMIZED, this, 0));

	}

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
	 * Invoked when the the native window's state changes.
	 *
	 * @param e the <code>WindowEvent</code>
	 */
	public synchronized void windowStateChanged(WindowEvent e) {

		if (disposed || !window.isShowing())
			return;

		// Fix bug 1174 by forcing the JVM to update its cached location for
		// the native window. The JVM has a bug in that when changing window
		// state, the size of the window is updated but not the location.
		updateLocation(window.getPeer());

	}

	/**
	 * A class that has painting methods that perform no painting.
	 */
	final class NoPaintDialog extends Dialog {

		/**
		 * The <code>VCLFrame</code>.
		 */
		private VCLFrame frame = null;

		/**
		 * The minimum size.
		 */
		private Dimension minSize = null;

		/**
		 * Constructs a new <code>VCLFrame.NoPaintDialog</code> instance.
		 *
		 * @param f the <code>VCLFrame</code>
		 * @param o the native dialog that owns this dialog
		 */
		NoPaintDialog(VCLFrame f, Dialog o) {

			super(o);
			frame = f;
			initialize();

		}

		/**
		 * Constructs a new <code>VCLFrame.NoPaintDialog</code> instance.
		 *
		 * @param f the <code>VCLFrame</code>
		 * @param o the native frame that owns this dialog
		 */
		NoPaintDialog(VCLFrame f, Frame o) {

			super(o);
			frame = f;
			initialize();

		}

		/**
		 * Called by the garbage collector on an object when garbage collection
		 * determines that there are no more references to the object.
		 */
		protected void finalize() throws Throwable
		{
			// Fix bug 1145 by destroying the native window and fix bugs
			// 1899 and  2151 by destroying it in the finalizer
			removeNotify();

		}

		/**
		 * Returns the focus owner of this dialog.
		 *
		 * @return the focus owner of this dialog
		 */
		public Component getFocusOwner() {

			if (frame.isFullScreenMode())
				return this;
			else
				return super.getFocusOwner();

		}

		/**
		 * Returns the minimum size for the dialog.
		 *
		 * @return the minimum size for the dialog
		 */
		public Dimension getMinimumSize() {

			return minSize;
			
		}

		/**
		 * Set the native dialog's initial size and style.
		 */
		private void initialize() {

			if (frame.undecorated) {
				setUndecorated(true);
				setFocusable(false);
				setFocusableWindowState(false);
			}

			setMinimumSize(0, 0);

			setBackground(Color.white);
			enableInputMethods(false);

		}

		/**
		 * This method performs no painting of the dialog. This method is used
		 * to prevent Java from painting over what VCL has painted.
		 *
		 * @param g the <code>Graphics</code>
		 */
		public void paint(Graphics g) {}

		/**
		 * Set the native dialog to show or hide in full screen mode.
		 *
		 * @param b <code>true</code> sets this dialog to full screen mode and
		 *  <code>false</code> sets it to normal mode
		 */
		void setFullScreenMode(boolean b) {

			if (isUndecorated()) {
				setFocusable(b);
				setFocusableWindowState(b);

				// Set background to black in full screen mode
				setBackground(b ? Color.black : Color.white);
			}

		}

		/**
		 * Set the minimum size for the dialog.
		 *
		 * @param width the minimum width
		 * @param height the minimum height
		 */
		void setMinimumSize(int width, int height) {

			Dimension minimumFrameSize = VCLScreen.getMinimumFrameSize();
			if (isUndecorated())
 				minimumFrameSize = new Dimension(0, 0);
			else
 				minimumFrameSize = VCLScreen.getMinimumFrameSize();

			insets = getInsets();
			width += insets.left + insets.right;
			height += insets.top + insets.bottom;

			if (width < minimumFrameSize.width)
				width = minimumFrameSize.width;
			if (height < minimumFrameSize.height)
				height = minimumFrameSize.height;

			minSize = new Dimension(width, height);
			
		}

		/**
		 * This method performs no painting of the dialog. This method is used
		 * to prevent Java from painting over what VCL has painted.
		 *
		 * @param g the <code>Graphics</code>
		 */
		public void update(Graphics g) {}

	}

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
			initialize();

		}

		/**
		 * Called by the garbage collector on an object when garbage collection
		 * determines that there are no more references to the object.
		 */
		protected void finalize() throws Throwable
		{
			// Fix bug 1145 by destroying the native window and fix bugs
			// 1899 and  2151 by destroying it in the finalizer
			removeNotify();

		}

		/**
		 * Returns the focus owner of this frame.
		 *
		 * @return the focus owner of this frame
		 */
		public Component getFocusOwner() {

			if (frame.isFullScreenMode())
				return this;
			else
				return super.getFocusOwner();

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
		 * Set the native frame's initial size and style.
		 */
		private void initialize() {

			if (frame.undecorated) {
				setUndecorated(true);
				setFocusable(false);
				setFocusableWindowState(false);
			}

			setMinimumSize(0, 0);

			setBackground(Color.white);
			enableInputMethods(false);

		}

		/**
		 * This method performs no painting of the frame. This method is used
		 * to prevent Java from painting over what VCL has painted.
		 *
		 * @param g the <code>Graphics</code>
		 */
		public void paint(Graphics g) {}

		/**
		 * Set the native frame to show or hide in full screen mode.
		 *
		 * @param b <code>true</code> sets this frame to full screen mode and
		 *  <code>false</code> sets it to normal mode
		 */
		void setFullScreenMode(boolean b) {

			if (isUndecorated()) {
				setFocusable(b);
				setFocusableWindowState(b);

				// Set background to black in full screen mode
				setBackground(b ? Color.black : Color.white);
			}

		}

		/**
		 * Set the minimum size for the frame.
		 *
		 * @param width the minimum width
		 * @param height the minimum height
		 */
		void setMinimumSize(int width, int height) {

			Dimension minimumFrameSize;
			if (isUndecorated())
 				minimumFrameSize = new Dimension(0, 0);
			else
 				minimumFrameSize = VCLScreen.getMinimumFrameSize();

			insets = getInsets();
			width += insets.left + insets.right;
			height += insets.top + insets.bottom;

			if (width < minimumFrameSize.width)
				width = minimumFrameSize.width;
			if (height < minimumFrameSize.height)
				height = minimumFrameSize.height;

			minSize = new Dimension(width, height);
			
		}

		/**
		 * This method performs no painting of the frame. This method is used
		 * to prevent Java from painting over what VCL has painted.
		 *
		 * @param g the <code>Graphics</code>
		 */
		public void update(Graphics g) {}

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
			setBackground(new Color(0x00000000, true));
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

			Shape clip = g.getClip();
			if (clip != null)
				frame.paint(clip.getBounds());
			else
				frame.paint(new Rectangle(getSize()));

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
