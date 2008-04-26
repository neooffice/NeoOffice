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
 *  Copyright 2003 Planamesa Inc.
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
import java.awt.Menu;
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
import java.util.Locale;

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
	 * SAL_FRAME_STYLE_OWNERDRAWDECORATION constant.
	 */
	public final static long SAL_FRAME_STYLE_OWNERDRAWDECORATION = 0x00000040;

	/**
	 * SAL_FRAME_STYLE_DIALOG constant.
	 */
	public final static long SAL_FRAME_STYLE_DIALOG = 0x00000080;

	/**
	 * SAL_FRAME_STYLE_PARTIAL_FULLSCREEN constant.
	 */
	public final static long SAL_FRAME_STYLE_PARTIAL_FULLSCREEN = 0x08000000;

	/**
	 * SAL_FRAME_STYLE_CHILD constant.
	 */
	public final static long SAL_FRAME_STYLE_CHILD = 0x10000000;

	/**
	 * SAL_FRAME_STYLE_FLOAT constant.
	 */
	public final static long SAL_FRAME_STYLE_FLOAT = 0x20000000;

	/**
	 * SAL_FRAME_STYLE_TOOLWINDOW constant.
	 */
	public final static long SAL_FRAME_STYLE_TOOLWINDOW = 0x40000000;

	/**
	 * SAL_FRAME_STYLE_INTRO constant.
	 */
	public final static long SAL_FRAME_STYLE_INTRO = 0x80000000;

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
	 * POINTER_PAINTBRUSH constant.
	 */
	public final static int POINTER_PAINTBRUSH = 93;
	
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
	 * The native utility window insets.
	 */
	private static Insets utilityWindowInsets = null;

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
	 * Flushes all of the native windows.
	 */
	public static void flushAllFrames() {

		if (!EventQueue.isDispatchThread()) {
			VCLEventQueue.runGCIfNeeded(0);

			FlushAllFramesHandler handler = new FlushAllFramesHandler();
			Toolkit.getDefaultToolkit().getSystemEventQueue().invokeLater(handler);
			Thread.yield();
			return;
		}

		LinkedList windowList = new LinkedList();

		Frame[] frames = Frame.getFrames();
		for (int i = 0; i < frames.length; i++) {
			Window[] windows = frames[i].getOwnedWindows();
			for (int j = 0; j < windows.length; j++)
				windowList.add(windows[j]);
			windowList.add(frames[i]);
		}

		Iterator iterator = windowList.iterator();
		while (iterator.hasNext()) {
			VCLFrame f = findFrame((Component)iterator.next());
			if (f != null) {
				synchronized (f) {
					f.enableFlushing(true);
					f.enableFlushing(false);
				}
			}
		}

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
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_NULL");
			if (c != null)
				customCursors.put(new Integer(POINTER_NULL), c);
		}
				
		url = cl.getSystemResource("com/sun/star/vcl/images/airbrush.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(2, 31), "POINTER_AIRBRUSH");
			if (c != null)
				customCursors.put(new Integer(POINTER_AIRBRUSH), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/ase.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_AUTOSCROLL_E");
			if (c != null)
				customCursors.put(new Integer(POINTER_AUTOSCROLL_E), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/asn.gif2");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_AUTOSCROLL_N");
			if (c != null)
				customCursors.put(new Integer(POINTER_AUTOSCROLL_N), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/asne.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_AUTOSCROLL_NE");
			if (c != null)
				customCursors.put(new Integer(POINTER_AUTOSCROLL_NE), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/asns.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_AUTOSCROLL_NS");
			if (c != null)
				customCursors.put(new Integer(POINTER_AUTOSCROLL_NS), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/asnswe");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_AUTOSCROLL_NSWE");
			if (c != null)
				customCursors.put(new Integer(POINTER_AUTOSCROLL_NSWE), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/asnw.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_AUTOSCROLL_NW");
			if (c != null)
				customCursors.put(new Integer(POINTER_AUTOSCROLL_NW), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/ass.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_AUTOSCROLL_S");
			if (c != null)
				customCursors.put(new Integer(POINTER_AUTOSCROLL_S), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/asse.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_AUTOSCROLL_SE");
			if (c != null)
				customCursors.put(new Integer(POINTER_AUTOSCROLL_SE), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/assw.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_AUTOSCROLL_SW");
			if (c != null)
				customCursors.put(new Integer(POINTER_AUTOSCROLL_SW), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/asw.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_AUTOSCROLL_W");
			if (c != null)
				customCursors.put(new Integer(POINTER_AUTOSCROLL_W), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/aswe.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_AUTOSCROLL_WE");
			if (c != null)
				customCursors.put(new Integer(POINTER_AUTOSCROLL_WE), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/chain.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(2, 3), "POINTER_CHAIN");
			if (c != null)
				customCursors.put(new Integer(POINTER_CHAIN), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/chainnot.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(10, 10), "POINTER_CHAIN_NOTALLOWED");
			if (c != null)
				customCursors.put(new Integer(POINTER_CHAIN_NOTALLOWED), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/chart.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(11, 11), "POINTER_CHART");
			if (c != null)
				customCursors.put(new Integer(POINTER_CHART), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/copydata.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(2, 3), "POINTER_COPYDATA");
			if (c != null)
				customCursors.put(new Integer(POINTER_COPYDATA), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/copydlnk.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(10, 2), "POINTER_COPYDATALINK");
			if (c != null)
				customCursors.put(new Integer(POINTER_COPYDATALINK), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/copyf.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(5, 10), "POINTER_COPYFILE");
			if (c != null)
				customCursors.put(new Integer(POINTER_COPYFILE), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/copyf2.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(9, 10), "POINTER_COPYFILES");
			if (c != null)
				customCursors.put(new Integer(POINTER_COPYFILES), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/copyflnk.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(9, 7), "POINTER_COPYFILELINK");
			if (c != null)
				customCursors.put(new Integer(POINTER_COPYFILELINK), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/crook.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 17), "POINTER_CROOK");
			if (c != null)
				customCursors.put(new Integer(POINTER_CROOK), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/crop.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_CROP");
			if (c != null)
				customCursors.put(new Integer(POINTER_CROP), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/cross.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_CROSS");
			if (c != null)
				customCursors.put(new Integer(POINTER_CROSS), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/darc.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(8, 8), "POINTER_DRAW_ARC");
			if (c != null)
				customCursors.put(new Integer(POINTER_DRAW_ARC), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/dbezier.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(8, 8), "POINTER_DRAW_BEZIER");
			if (c != null)
				customCursors.put(new Integer(POINTER_DRAW_BEZIER), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/dcapt.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(8, 8), "POINTER_DRAW_CAPTION");
			if (c != null)
				customCursors.put(new Integer(POINTER_DRAW_CAPTION), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/dcirccut.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(8, 8), "POINTER_DRAW_CIRCLECUT");
			if (c != null)
				customCursors.put(new Integer(POINTER_DRAW_CIRCLECUT), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/dconnect.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(8, 8), "POINTER_DRAW_CONNECT");
			if (c != null)
				customCursors.put(new Integer(POINTER_DRAW_CONNECT), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/dellipse.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(8, 8), "POINTER_DRAW_ELLIPSE");
			if (c != null)
				customCursors.put(new Integer(POINTER_DRAW_ELLIPSE), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/detectiv.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_DETECTIVE");
			if (c != null)
				customCursors.put(new Integer(POINTER_DETECTIVE), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/dfree.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(8, 8), "POINTER_DRAW_FREEHAND");
			if (c != null)
				customCursors.put(new Integer(POINTER_DRAW_FREEHAND), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/dline.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(8, 8), "POINTER_DRAW_LINE");
			if (c != null)
				customCursors.put(new Integer(POINTER_DRAW_LINE), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/dpie.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(8, 8), "POINTER_DRAW_PIE");
			if (c != null)
				customCursors.put(new Integer(POINTER_DRAW_PIE), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/dpolygon.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(8, 8), "POINTER_DRAW_POLYGON");
			if (c != null)
				customCursors.put(new Integer(POINTER_DRAW_POLYGON), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/drect.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(8, 8), "POINTER_DRAW_RECT");
			if (c != null)
				customCursors.put(new Integer(POINTER_DRAW_RECT), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/dtext.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(8, 8), "POINTER_DRAW_TEXT");
			if (c != null)
				customCursors.put(new Integer(POINTER_DRAW_TEXT), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/fill.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_FILL");
			if (c != null)
				customCursors.put(new Integer(POINTER_FILL), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/hand.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_HAND");
			if (c != null)
				customCursors.put(new Integer(POINTER_HAND), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/help.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(2, 3), "POINTER_HELP");
			if (c != null)
				customCursors.put(new Integer(POINTER_HELP), c);
		}
		
		// NOTE:  Missing constant from Win32 :  HSIZE.CUR
		
		url = cl.getSystemResource("com/sun/star/vcl/images/hsizebar.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_HSIZEBAR");
			if (c != null)
				customCursors.put(new Integer(POINTER_HSIZEBAR), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/hsplit.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_HSPLIT");
			if (c != null)
				customCursors.put(new Integer(POINTER_HSPLIT), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/linkdata.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(10, 2), "POINTER_LINKDATA");
			if (c != null)
				customCursors.put(new Integer(POINTER_LINKDATA), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/linkf.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(9, 7), "POINTER_LINKFILE");
			if (c != null)
				customCursors.put(new Integer(POINTER_LINKFILE), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/magnify.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_MAGNIFY");
			if (c != null)
				customCursors.put(new Integer(POINTER_MAGNIFY), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/mirror.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_MIRROR");
			if (c != null)
				customCursors.put(new Integer(POINTER_MIRROR), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/move.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_MOVE");
			if (c != null)
				customCursors.put(new Integer(POINTER_MOVE), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/movebw.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(2, 3), "POINTER_MOVEBEZIERWEIGHT");
			if (c != null)
				customCursors.put(new Integer(POINTER_MOVEBEZIERWEIGHT), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/movedata.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(2, 3), "POINTER_MOVEDATA");
			if (c != null)
				customCursors.put(new Integer(POINTER_MOVEDATA), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/movedlnk.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(3, 3), "POINTER_MOVEDATALINK");
			if (c != null)
				customCursors.put(new Integer(POINTER_MOVEDATALINK), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/movef.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(5, 10), "POINTER_MOVEFILE");
			if (c != null)
				customCursors.put(new Integer(POINTER_MOVEFILE), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/movef2.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(9, 10), "POINTER_MOVEFILES");
			if (c != null)
				customCursors.put(new Integer(POINTER_MOVEFILES), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/moveflnk.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(2, 3), "POINTER_MOVEFILELINK");
			if (c != null)
				customCursors.put(new Integer(POINTER_MOVEFILELINK), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/movept.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(2, 3), "POINTER_MOVEPOINT");
			if (c != null)
				customCursors.put(new Integer(POINTER_MOVEPOINT), c);
		}
		
		// NOTE:  Unused POINTER_NESWSIZE
		
		url = cl.getSystemResource("com/sun/star/vcl/images/notallow.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_NOTALLOWED");
			if (c != null)
				customCursors.put(new Integer(POINTER_NOTALLOWED), c);
		}
		
		// NOTE:  Unused POINTER_NWSESIZE
		
		url = cl.getSystemResource("com/sun/star/vcl/images/pen.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(2, 31), "POINTER_PEN");
			if (c != null)
				customCursors.put(new Integer(POINTER_PEN), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/pivotcol.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(2, 3), "POINTER_PIVOT_COL");
			if (c != null)
				customCursors.put(new Integer(POINTER_PIVOT_COL), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/pivotdel.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_PIVOT_DELETE");
			if (c != null)
				customCursors.put(new Integer(POINTER_PIVOT_DELETE), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/pivotfld.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(2, 3), "POINTER_PIVOT_FIELD");
			if (c != null)
				customCursors.put(new Integer(POINTER_PIVOT_FIELD), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/pivotrow.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(2, 3), "POINTER_PIVOT_ROW");
			if (c != null)
				customCursors.put(new Integer(POINTER_PIVOT_ROW), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/pntbrsh.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(4, 30), "POINTER_PAINTBRUSH");
			if (c != null)
				customCursors.put(new Integer(POINTER_PAINTBRUSH), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/refhand.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_REFHAND");
			if (c != null)
				customCursors.put(new Integer(POINTER_REFHAND), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/rotate.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_ROTATE");
			if (c != null)
				customCursors.put(new Integer(POINTER_ROTATE), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/tblsele.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(31, 16), "POINTER_TAB_SELECT_E");
			if (c != null)
				customCursors.put(new Integer(POINTER_TAB_SELECT_E), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/tblsels.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 31), "POINTER_TAB_SELECT_S");
			if (c != null)
				customCursors.put(new Integer(POINTER_TAB_SELECT_S), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/tblselse.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(31, 31), "POINTER_TAB_SELECT_SE");
			if (c != null)
				customCursors.put(new Integer(POINTER_TAB_SELECT_SE), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/tblselsw.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(2, 31), "POINTER_TAB_SELECT_SW");
			if (c != null)
				customCursors.put(new Integer(POINTER_TAB_SELECT_SW), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/tblselw.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(2, 16), "POINTER_TAB_SELECT_W");
			if (c != null)
				customCursors.put(new Integer(POINTER_TAB_SELECT_W), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/timemove.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_TIMEEVENT_MOVE");
			if (c != null)
				customCursors.put(new Integer(POINTER_TIMEEVENT_MOVE), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/timesize.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_TIMEEVENT_SIZE");
			if (c != null)
				customCursors.put(new Integer(POINTER_TIMEEVENT_SIZE), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/vshear.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_VSHEAR");
			if (c != null)
				customCursors.put(new Integer(POINTER_VSHEAR), c);
		}
		
		// NOTE:  Unused POINTER_VSIZE
		
		url = cl.getSystemResource("com/sun/star/vcl/images/vsizebar.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_VSIZEBAR");
			if (c != null)
				customCursors.put(new Integer(POINTER_VSIZEBAR), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/vsplit.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(16, 16), "POINTER_VSPLIT");
			if (c != null)
				customCursors.put(new Integer(POINTER_VSPLIT), c);
		}
		
		url = cl.getSystemResource("com/sun/star/vcl/images/vtext.gif");
		if (url != null) {
			Image img = toolkit.createImage(url);
			Cursor c = toolkit.createCustomCursor(img, new Point(15, 15), "POINTER_TEXT_VERTICAL");
			if (c != null)
				customCursors.put(new Integer(POINTER_TEXT_VERTICAL), c);
		}
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
	 * The flushing enabled flag.
	 */
	private boolean flushingEnabled = true;

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
	 * The hidden menubar.
	 */
	private MenuBar hiddenMenuBar = null;

	/**
	 * The ignore mouse released modifiers.
	 */
	private int ignoreMouseReleasedModifiers = 0;

	/**
	 * The native window's insets.
	 */
	private Insets insets = null;

	/**
	 * The last committed input method event.
	 */
	private InputMethodEvent lastCommittedInputMethodEvent = null;

	/**
	 * The last uncommitted input method event.
	 */
	private InputMethodEvent lastUncommittedInputMethodEvent = null;

	/**
	 * The last window dragged event.
	 */
	private MouseEvent lastWindowDraggedEvent = null;

	/**
	 * The native window's panel.
	 */
	private VCLFrame.NoPaintPanel panel = null;

	/**
	 * The event queue.
	 */
	protected VCLEventQueue queue = null;

	/**
	 * The resizable flag.
	 */
	private boolean resizable = false;

	/**
	 * The show only menus flag.
	 */
	private boolean showOnlyMenus = false;

	/**
	 * The show only menus bounds.
	 */
	private Rectangle showOnlyMenusBounds = null;

	/**
	 * The style flags.
	 */
	private long style = 0;

	/**
	 * The use input method fix flag.
	 */
	private boolean useInputMethodFix = false;

	/**
	 * The native window.
	 */
	private Window window = null;

	/** 
	 * The window undecorated mode.
	 */
	private boolean undecorated = false;

	/** 
	 * The window utility mode.
	 */
	private boolean utility = false;

	/**
	 * Constructs a new <code>VCLFrame</code> instance.
	 *
	 * @param s the SAL_FRAME_STYLE flags
	 * @param q the event queue to post events to
	 * @param f the frame pointer
	 * @param p the parent frame
	 * @param b <code>true</code> if the input method fix is needed otherwise
	 *  <code>false</code>
	 * @param m <code>true</code> if only menus are to shown and the frame is
	 *  to always be hidden behind the native menubar otherwise
	 *  <code>false</code> for normal frame behavior
	 * @param u <code>true</code> if the frame should use a native utility
	 *  window
	 */
	public VCLFrame(long s, VCLEventQueue q, long f, VCLFrame p, boolean b, boolean m, boolean u) {

		queue = q;
		frame = f;
		showOnlyMenus = m;
		style = s;
		useInputMethodFix = b;
		utility = u;

		// Create the native window
		if (!utility && (showOnlyMenus || (style & (SAL_FRAME_STYLE_DEFAULT | SAL_FRAME_STYLE_MOVEABLE | SAL_FRAME_STYLE_SIZEABLE)) == 0))
			undecorated = true;

		// Utility windows should never be attached to a parent window
		Window w = null;
		if (p != null && !utility)
			w = p.getWindow();
		if (w instanceof Dialog)
			window = new VCLFrame.NoPaintDialog(this, (Dialog)w);
		else if (w instanceof Frame)
			window = new VCLFrame.NoPaintDialog(this, (Frame)w);
		else
			window = new VCLFrame.NoPaintFrame(this, queue);

		// Process remaining style flags
		if (showOnlyMenus)
			setBounds(0, 0, 1, 1);
		else if ((style & SAL_FRAME_STYLE_SIZEABLE) != 0)
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

		if (undecorated) {
			insets = window.getInsets();
		}
		else if (utility) {
			if (VCLFrame.utilityWindowInsets == null) {
				Window uw = new VCLFrame.NoPaintFrame(this, queue);
				uw.addNotify();
				VCLFrame.utilityWindowInsets = uw.getInsets();
				uw.removeNotify();
			}
			insets = VCLFrame.utilityWindowInsets;
		}
		else {
			insets = VCLScreen.getFrameInsets();
		}

		graphics = new VCLGraphics(this);

		// Register listeners
		panel.addFocusListener(this);
		panel.addKeyListener(this);
		window.addComponentListener(this);
		window.addFocusListener(this);
		window.addKeyListener(this);
		window.addWindowStateListener(this);
		if (!showOnlyMenus) {
			panel.addInputMethodListener(this);
			panel.addMouseListener(this);
			panel.addMouseMotionListener(this);
			panel.addMouseWheelListener(this);
			window.addInputMethodListener(this);
			window.addMouseListener(this);
			window.addMouseWheelListener(this);
		}

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

		// Fix bug 2370 by listening for mouse events in the window frame.
		// Prevent dispatch of a mouseDragged() event when the window is
		// first shown by listening to such events until after this event is
		// dispatched.
		if (!undecorated) {
			window.addMouseMotionListener(this);
			window.addWindowListener(this);
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

		lastCommittedInputMethodEvent = null;
		lastUncommittedInputMethodEvent = null;
		lastWindowDraggedEvent = null;

		if (!undecorated) {
			window.removeMouseMotionListener(this);
			window.removeWindowListener(this);
		}

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
		hiddenMenuBar = null;
		insets = null;
		lastCommittedInputMethodEvent = null;
		lastUncommittedInputMethodEvent = null;
		lastWindowDraggedEvent = null;

		// Unregister listeners
		panel.removeFocusListener(this);
		panel.removeKeyListener(this);
		window.removeComponentListener(this);
		window.removeFocusListener(this);
		window.removeKeyListener(this);
		window.removeWindowStateListener(this);
		if (!showOnlyMenus) {
			panel.removeInputMethodListener(this);
			panel.removeMouseListener(this);
			panel.removeMouseMotionListener(this);
			panel.removeMouseWheelListener(this);
			window.removeInputMethodListener(this);
			window.removeMouseListener(this);
			window.removeMouseWheelListener(this);
		}
		queue.removeCachedEvents(frame);

		panel = null;
		queue = null;
		window.removeNotify();
		window = null;

		disposed = true;

	}

	/**
	 * Enable or disable flushing of the native window.
	 *
	 * @param b <code>true</code> to enable flushing and <code>false</code> to
	 *  disable flushing
	 */
	public void enableFlushing(boolean b) {

		// Fix occasional crashing by invoking this in the Java event dispatch
		// thread
		if (!EventQueue.isDispatchThread())	{
			VCLFrame.FlushingHandler handler = new VCLFrame.FlushingHandler(this, b);
			Toolkit.getDefaultToolkit().getSystemEventQueue().invokeLater(handler);
			Thread.yield();
			return;
		}

		synchronized (this) {
			if (!disposed && b != flushingEnabled && panel.isShowing()) {
				Graphics2D g = (Graphics2D)panel.getGraphics();
				if (g != null) {
					try {
						if (g instanceof sun.java2d.SunGraphics2D) {
							sun.java2d.SurfaceData sd = ((sun.java2d.SunGraphics2D)g).getSurfaceData();
							if (sd instanceof apple.awt.CPeerSurfaceData) {
								if (b)
									((apple.awt.CPeerSurfaceData)sd).enableFlushing();
								else if (!fullScreenMode)
									((apple.awt.CPeerSurfaceData)sd).disableFlushing();
								flushingEnabled = b;
							}
						}
					}
					catch (Throwable t) {
						t.printStackTrace();
					}
					g.dispose();
				}
			}
		}

	}

	/**
	 * Invoked when the native window has gained focus.
	 *
	 * @param e the <code>FocusEvent</code>
	 */
	public synchronized void focusGained(FocusEvent e) {

		if (disposed || isFloatingWindow() || !window.isShowing())
			return;

		// Update menubar state
		Frame[] frames = Frame.getFrames();
		for (int i = 0; i < frames.length; i++) {
			VCLFrame f = findFrame(frames[i]);
			if (f != null) {
				synchronized (f) {
					f.setMenuBar(f.getMenuBar());
				}
			}
		}

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
		// window. Fix bug 2478 by not letting us lose focus if the opposite
		// component is this frame.
		VCLFrame f = findFrame(e.getOppositeComponent());
		if (f != null) {
			synchronized (f) {
				if (f == this || f.isFloatingWindow()) {
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

		if (showOnlyMenus && showOnlyMenusBounds != null)
			return showOnlyMenusBounds;

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
		if (outCode != 0 && outCode != KeyEvent.VK_AGAIN)
			buf.append(KeyEvent.getKeyText(outCode));

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

		MenuBar mb = null;

		if (window instanceof Frame) {
			mb = ((Frame)window).getMenuBar();
			if (mb == null) {
				synchronized (this) {
					mb = hiddenMenuBar;
				}
			}
		}

		return mb;

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
		long f = 0;

		synchronized (this) {
			if (disposed || !window.isShowing())
				return bounds;
			f = frame;
		}

		// Get the position from the OOo code
		Rectangle b = getTextLocation0(f);

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

		lastCommittedInputMethodEvent = null;

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
				if (useInputMethodFix) {
					AttributedCharacterIterator lastText = lastUncommittedInputMethodEvent.getText();
					boolean commitLast = false;
					int lastCount = 0;
					for (char c = lastText.first(); c != CharacterIterator.DONE; c = lastText.next()) {
						// Fix bug 2492 by only committing if there are
						// uncommitted characters in or below the Indic range
						if (!commitLast && c < 0x0E00)
							commitLast = true;
						lastCount++;
					}

					if (commitLast)
						e = new InputMethodEvent((Component)lastUncommittedInputMethodEvent.getSource(), lastUncommittedInputMethodEvent.getID(), lastUncommittedInputMethodEvent.getWhen(), lastText, lastCount, lastUncommittedInputMethodEvent.getCaret(), lastUncommittedInputMethodEvent.getVisiblePosition());
				}
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
		else if (useInputMethodFix) {
			// Fix bug 2492 by handling when Java duplicates committed input
			// in a key typed event
			if (count == 1)
				lastCommittedInputMethodEvent = e;
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

		return (undecorated && !fullScreenMode && !showOnlyMenus);

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
		int modifiers = e.getModifiersEx();
		if (keyCode == KeyEvent.VK_SHIFT || keyCode == KeyEvent.VK_CONTROL || keyCode == KeyEvent.VK_ALT || keyCode == KeyEvent.VK_META) {
			VCLEvent keyModChangeEvent = new VCLEvent(e, VCLEvent.SALEVENT_KEYMODCHANGE, this, 0);
			queue.postCachedEvent(keyModChangeEvent);
		}
		else if (e.isActionKey() || keyCode == KeyEvent.VK_BACK_SPACE || keyCode == KeyEvent.VK_ENTER || keyCode == KeyEvent.VK_DELETE || keyCode == KeyEvent.VK_ESCAPE || (e.getKeyLocation() == KeyEvent.KEY_LOCATION_NUMPAD && e.getModifiersEx() == InputEvent.META_DOWN_MASK && keyCode == KeyEvent.VK_MULTIPLY)) {
			// Fix bug 3018 by stripping out the Alt modifier for arrow keys
			if ((modifiers & InputEvent.ALT_DOWN_MASK) != 0 && (keyCode == KeyEvent.VK_DOWN || keyCode == KeyEvent.VK_LEFT || keyCode == KeyEvent.VK_RIGHT || keyCode == KeyEvent.VK_UP))
				e = new KeyEvent(e.getComponent(), e.getID(), e.getWhen(), (e.getModifiers() | modifiers) & ~(InputEvent.ALT_MASK | InputEvent.ALT_DOWN_MASK), keyCode, e.getKeyChar());

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
		int modifiers = e.getModifiersEx();
		if (keyCode == KeyEvent.VK_SHIFT || keyCode == KeyEvent.VK_CONTROL || keyCode == KeyEvent.VK_ALT || keyCode == KeyEvent.VK_META) {
			VCLEvent keyModChangeEvent = new VCLEvent(e, VCLEvent.SALEVENT_KEYMODCHANGE, this, 0);
			queue.postCachedEvent(keyModChangeEvent);
		}
		else if (e.isActionKey() || keyCode == KeyEvent.VK_BACK_SPACE || keyCode == KeyEvent.VK_ENTER || keyCode == KeyEvent.VK_DELETE || keyCode == KeyEvent.VK_ESCAPE || (e.getKeyLocation() == KeyEvent.KEY_LOCATION_NUMPAD && e.getModifiersEx() == InputEvent.META_DOWN_MASK && keyCode == KeyEvent.VK_MULTIPLY)) {
			// Fix bug 3018 by stripping out the Alt modifier for arrow keys
			if ((modifiers & InputEvent.ALT_DOWN_MASK) != 0 && (keyCode == KeyEvent.VK_DOWN || keyCode == KeyEvent.VK_LEFT || keyCode == KeyEvent.VK_RIGHT || keyCode == KeyEvent.VK_UP))
				e = new KeyEvent(e.getComponent(), e.getID(), e.getWhen(), (e.getModifiers() | modifiers) & ~(InputEvent.ALT_MASK | InputEvent.ALT_DOWN_MASK), keyCode, e.getKeyChar());

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

		char keyChar = e.getKeyChar();

		// Fix bug 2492 by handling when Java duplicates committed input
		// in a key typed event
		if (lastCommittedInputMethodEvent != null) {
			AttributedCharacterIterator text = lastCommittedInputMethodEvent.getText();
			lastCommittedInputMethodEvent = null;
			if (text != null && text.first() == keyChar)
				return;
		}

		// These are handled in the key pressed and released events.
		int modifiers = e.getModifiersEx();
		if (keyChar == (char)0x03 || keyChar == (char)0x08 || keyChar == (char)0x0a || keyChar == (char)0x0d || keyChar == (char)0x7f)
			return;

		// Fix bug 710 by stripping out the Alt modifier. Note that we do it
		// here because we need to let the Alt modifier through for action
		// keys.
		if ((modifiers & InputEvent.ALT_DOWN_MASK) != 0)
			e = new KeyEvent(e.getComponent(), e.getID(), e.getWhen(), (e.getModifiers() | modifiers) & ~(InputEvent.ALT_MASK | InputEvent.ALT_DOWN_MASK), e.getKeyCode(), keyChar);

		if ((modifiers & InputEvent.META_DOWN_MASK) != 0) {
			// Fix bug 1143 by converting any capital alpha characters to
			// lowercase when the meta key is pressed. Fix bug 2698 by handling
			// the '_' key.
			keyChar = e.getKeyChar();
			if (keyChar >= 'A' && keyChar <= 'Z')
				e = new KeyEvent(e.getComponent(), e.getID(), e.getWhen(), e.getModifiers() | modifiers, e.getKeyCode(), (char)(keyChar + 32));
			else if (keyChar == '_')
				e = new KeyEvent(e.getComponent(), e.getID(), e.getWhen(), e.getModifiers() | modifiers, e.getKeyCode(), '-');
		}
		else if ((modifiers & InputEvent.CTRL_DOWN_MASK) != 0) {
			// Fix bug 2795 by ignoring any Control key events
			return;
		}

		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_KEYINPUT, this, 0));
		queue.postCachedEvent(new VCLEvent(e, VCLEvent.SALEVENT_KEYUP, this, 0));

	}

	/**
	 * Force the native window to be a utility window.
	 *
	 * @param p the <code>ComponentPeer</code>
	 * @return the new top inset or zero if the method failed to change the
	 *  window style
	 */
	public native int makeUtilityWindow(ComponentPeer p);

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

		lastWindowDraggedEvent = null;

		// The JVM can get confused when we click on a non-focused window. In
		// these cases, we will receive no mouse move events so if the OOo code
		// displays a popup menu, the popup menu will receive no mouse move
		// events.
		int modifiers = e.getModifiersEx();
		if (!isFloatingWindow() && !window.isFocused()) {
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

		lastWindowDraggedEvent = null;

		// Cache only extended modifiers for synthetic mouse move event
		int remainingModifiers = e.getModifiersEx();

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

		// Strange but true, fix bug 2157 by posting a synthetic mouse moved
		// event
		MouseEvent mouseMoved = new MouseEvent(e.getComponent(), MouseEvent.MOUSE_MOVED, e.getWhen(), remainingModifiers, e.getX(), e.getY(), e.getClickCount(), e.isPopupTrigger());
		queue.postCachedEvent(new VCLEvent(mouseMoved, VCLEvent.SALEVENT_MOUSEMOVE, VCLFrame.findFrame(e.getComponent()), 0));

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

		// For events where the component is a window, use the first drag event
		// as the coordinates should not change but Java does sometimes change
		// them
		Component c = e.getComponent();
		if (c instanceof Window) {
			if (lastWindowDraggedEvent == null)
				lastWindowDraggedEvent = e;
			else
				e = lastWindowDraggedEvent;
System.out.println(e);
		}

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

		lastWindowDraggedEvent = null;

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

		lastWindowDraggedEvent = null;

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

		lastWindowDraggedEvent = null;

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

		lastWindowDraggedEvent = null;

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

		// Always put showOnlyMenus windows under the main menubar
		if (showOnlyMenus) {
			showOnlyMenusBounds = new Rectangle(x, y, width, height);
			x = 0;
			y = 0;
			width = 1;
			height = 1;
		}

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

		panel.setFullScreenMode(fullScreenMode);
		if (window instanceof VCLFrame.NoPaintDialog)
			((VCLFrame.NoPaintDialog)window).setFullScreenMode(fullScreenMode);
		else if (window instanceof VCLFrame.NoPaintFrame)
			((VCLFrame.NoPaintFrame)window).setFullScreenMode(fullScreenMode);

		// Run the garbage collector as we are guaranteed to be creating or
		// releasing one or more large bitmaps
		System.gc();

	}

	/**
	 * Sets the menubar for this native window.
	 *
	 * @param menubar the <code>MenuBar</code> instance of <code>null</code>
	 */
	void setMenuBar(MenuBar menubar) {

		if (window instanceof Frame)
			((Frame)window).setMenuBar(menubar);

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
			// Set the cursor in all frames as the fix for bug 2370 exposes
			// the fact that Java only shows the cursor for the current
			// focus window
			Frame[] frames = Frame.getFrames();
			for (int i = 0; i < frames.length; i++) {
				Window[] windows = frames[i].getOwnedWindows();
				for (int j = 0; j < windows.length; j++)
					windows[j].setCursor(c);
				frames[i].setCursor(c);
			}

			Toolkit.getDefaultToolkit().sync();
		}

	}

	/**
	 * Sets the state of the native window.
	 *
	 * @param state the state of the native window
	 */
	public synchronized void setState(long state) {

		if (!showOnlyMenus && window instanceof Frame && window.isShowing()) {
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
			// Fix bug 1012 by deiconifying the parent window. Fix bug 1388 by
			// skipping this step if the current window is a floating window.
			if (!isFloatingWindow()) {
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

			panel.setVisible(true);
			window.show();

			// Fix bug 2702 by ensuring that the input context's
			// locale is never null
			InputContext ic = window.getInputContext();
			if (ic != null && ic.getLocale() == null)
				ic.selectInputMethod(Locale.getDefault());

			if (focusable) {
				window.setFocusable(true);
				window.setFocusableWindowState(true);
			}
			enableFlushing(true);
		}
		else {
			// Hide the window
			enableFlushing(false);
			panel.setVisible(false);
			window.hide();
			window.removeNotify();

			// Force immediate cleanup of cached window data
			componentHidden(new ComponentEvent(window,  ComponentEvent.COMPONENT_HIDDEN));
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

		panel.setVisible(false);

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

		panel.setVisible(true);

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

			// Fix bug 3012 by only returning a minimum size when the dialog
			// is visible
			if (isVisible())
				return minSize;
			else
				return new Dimension(0,0);
			
		}

		/**
		 * Set the native dialog's initial size and style.
		 */
		private void initialize() {

			if (frame.undecorated) {
				setUndecorated(true);
				if (!frame.showOnlyMenus) {
					setFocusable(false);
					setFocusableWindowState(false);
				}
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
			}

			// Set background to black in full screen mode
			setBackground(b ? Color.black : Color.white);

		}

		/**
		 * Set the minimum size for the dialog.
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

			Insets insets = getInsets();
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
		 * The <code>VCLEventQueue</code>.
		 */
		private VCLEventQueue queue = null;

		/**
		 * The utility window top inset.
		 */
		private int utilityWindowTopInset = 0;

		/**
		 * Constructs a new <code>VCLFrame.NoPaintFrame</code> instance.
		 *
		 * @param f the <code>VCLFrame</code>
		 */
		NoPaintFrame(VCLFrame f, VCLEventQueue q) {

			frame = f;
			queue = q;
			initialize();

		}

		/**
		 * Creates the native frame.
		 */
		public void addNotify() {

			super.addNotify();

			// Make the native window a utility window if necessary
			if (frame.utility)
				utilityWindowTopInset = frame.makeUtilityWindow(getPeer());

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
		 * Returns the insets for this frame.
		 *
		 * @return the insets for this frame
		 */
		public Insets getInsets() {

			Insets insets = super.getInsets();

			if (frame.utility && utilityWindowTopInset != 0 && insets.top != 0)
				insets.top = utilityWindowTopInset;

			return insets;

		}

		/**
		 * Returns the minimum size for the frame.
		 *
		 * @return the minimum size for the frame
		 */
		public Dimension getMinimumSize() {

			// Fix bug 3012 by only returning a minimum size when the frame
			// is visible
			if (isVisible())
				return minSize;
			else
				return new Dimension(0,0);
			
		}

		/**
		 * Set the native frame's initial size and style.
		 */
		private void initialize() {

			if (frame.undecorated) {
				setUndecorated(true);
				if (!frame.showOnlyMenus) {
					setFocusable(false);
					setFocusableWindowState(false);
				}
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
		 * Set whether this frame can become the focused component.
		 *
		 * @param b <code>true</code> to make this frame can become the focused
		 *  component
		 */
		public void setFocusable(boolean b) {

			// Never let utility windows not be focusable as on Mac OS X 10.5
			// that will cause the resize button to not be displayed
			if (frame.utility)
				b = true;

			super.setFocusable(b);

		}

		/**
		 * Set whether this frame can become the focused window.
		 *
		 * @param b <code>true</code> to make this frame can become the focused
		 *  window
		 */
		public void setFocusableWindowState(boolean b) {

			// Never let utility windows not be focusable as on Mac OS X 10.5
			// that will cause the resize button to not be displayed
			if (frame.utility)
				b = true;

			super.setFocusableWindowState(b);

		}

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
			}

			// Set background to black in full screen mode
			setBackground(b ? Color.black : Color.white);

		}

		/**
		 * Sets the menubar for this frame to the specified menubar.
		 *
		 * @param mb the menubar or <code>null</code>
		 */
		public void setMenuBar(MenuBar mb) {

			// Fix bug 3003 by only setting the menubar in the Java event
			// dispatch thread
			if (!EventQueue.isDispatchThread()) {
				VCLFrame.SetMenuBarHandler handler = new VCLFrame.SetMenuBarHandler(this, mb);
				Toolkit.getDefaultToolkit().getSystemEventQueue().invokeLater(handler);
				Thread.yield();
				return;
			}

			MenuBar oldMenuBar = getMenuBar();

			boolean active = isActive();
			synchronized (frame) {
				if (active) {
					frame.hiddenMenuBar = null;
				}
				else {
					frame.hiddenMenuBar = mb;
					mb = null;
				}
			}

			synchronized (getTreeLock()) {
				// If we are changing menubars, we need to remove all of the
				// menus from the current menubar in reverse order and readd
				// them after the menubar has been detached from the frame to
				// prevent doubling of menus when a child dialog has focus
				if (oldMenuBar != mb) {
					LinkedList oldMenus = new LinkedList();
					if (oldMenuBar != null) {
						int count = oldMenuBar.getMenuCount();
						for (int i = count - 1; i >= 0; i--) {
							Menu m = oldMenuBar.getMenu(i);
							if (m != null) {
								m.setEnabled(true);
								oldMenuBar.remove(i);
								oldMenus.add(m);
							}
						}
					}

					// On Mac OS X 10.3.x, we need to remove the menubar before
					// replacing it
					if (mb != null)
						super.setMenuBar(null);
					super.setMenuBar(mb);

					if (oldMenuBar != null) {
						while (oldMenus.size() > 0) {
							Menu m = (Menu)oldMenus.removeLast();
							if (m != null)
								oldMenuBar.add(m);
						}
					}
				}
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

			Insets insets = getInsets();
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
			setBackground(Color.white);
			enableInputMethods(frame.showOnlyMenus ? false : true);

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
		 * Set the native panel to show or hide in full screen mode.
		 *
		 * @param b <code>true</code> sets this panel to full screen mode and
		 *  <code>false</code> sets it to normal mode
		 */
		void setFullScreenMode(boolean b) {

			// Set background to black in full screen mode
			setBackground(b ? Color.black : Color.white);

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

	/**
	 * A class that handles flushing updates.
	 */
	final class FlushingHandler implements Runnable {

		/**
		 * The flushing enabled flag.
		 */
		private boolean flushingEnabled = true;

		/**
		 * The <code>VCLFrame</code>.
		 */
		private VCLFrame frame = null;

		/**
		 * Constructs a new <code>VCLFrame.FlushingHandler</code> instance.
		 *
		 * @param f the <code>VCLFrame</code>
		 * @param b <code>true</code> to enable flushing and <code>false</code>
		 *  to disable flushing
		 */
		FlushingHandler(VCLFrame f, boolean b) {

			frame = f;
			flushingEnabled = b;

		}

		public void run() {

			frame.enableFlushing(flushingEnabled);

		}

	}

	/**
	 * A class that flushes all <code>VCLFrame</code> instances.
	 */
	final static class FlushAllFramesHandler implements Runnable {

		public void run() {

			VCLFrame.flushAllFrames();

		}

	}

	/**
	 * A class that handles flushing updates.
	 */
	final class SetMenuBarHandler implements Runnable {

		/**
		 * The menubar.
		 */
		private MenuBar menubar = null;

		/**
		 * The <code>Frame</code>.
		 */
		private Frame frame = null;

		/**
		 * Constructs a new
		 * <code>VCLFrame.NoPaintFrame.SetMenuBarHandler</code> instance.
		 *
		 * @param f the <code>Frame</code>
		 * @param m the <code>MenuBar</code>
		 */
		SetMenuBarHandler(Frame f, MenuBar m) {

			frame = f;
			menubar = m;

		}

		public void run() {

			frame.setMenuBar(menubar);

		}

	}

}
