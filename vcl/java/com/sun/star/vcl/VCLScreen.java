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

import java.awt.Frame;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.Insets;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.SystemColor;
import java.awt.Toolkit;
import java.awt.Window;

/** 
 * The Java class that implements the convenience methods for accessing
 * screen information.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public final class VCLScreen {

	/**
	 * The minimum screen resolution
	 */
	public final static int MAX_PRINTER_RESOLUTION = 300;

	/**
	 * The minimum screen resolution
	 */
	public final static int MIN_SCREEN_RESOLUTION = 96;

	/**
	 * The cached frame insets.
	 */
	private static Insets frameInsets = null;

	/**
	 * Initialize screen size and frame insets.
	 */
	static {

		Frame f = new Frame();
		f.setSize(100, 100);
		f.addNotify();
		frameInsets = f.getInsets();
		f.removeNotify();
		f.dispose();

	}

	/**
	 * Returns the <code>System.control</code>.
	 *
	 * @return the <code>System.control</code>
	 */
	public static int getControlColor() {

		return SystemColor.control.getRGB();

	}

	/**
	 * Gets the <code>Frame</code> insets.
	 *
	 * @return the <code>Frame</code> insets
	 */
	public static Insets getFrameInsets() {

		return frameInsets;

	}

	/**
	 * Gets the bounds of the screen that a <code>VCLFrame</code> is located in.
	 *
	 * @param f the <code>VCLFrame</code>
	 * @return the bounds of the screen
	 */
	public static Rectangle getScreenBounds(VCLFrame f) {

		Rectangle bounds = null;

		// Iterate through the screen devices and find the screen that the
		// top left corner of the frame is in
		GraphicsDevice[] screens = GraphicsEnvironment.getLocalGraphicsEnvironment().getScreenDevices();
		Window w = f.getWindow();
		if (w != null) {
			Point p = w.getLocation();
			for (int i = 0; i < screens.length; i++) {
				Rectangle r = screens[i].getDefaultConfiguration().getBounds();
				if (r.contains(p)) {
					bounds = new Rectangle(r);
					break;
				}
			}
		}

		if (bounds == null)
			bounds = new Rectangle(Toolkit.getDefaultToolkit().getScreenSize());

		return bounds;

	}

	/**
	 * Returns the <code>System.textHighlight</code>.
	 *
	 * @return the <code>System.textHighlight</code>
	 */
	public static int getTextHighlightColor() {

		return SystemColor.textHighlight.getRGB();

	}

	/**
	 * Returns the <code>System.textHighlightText</code>.
	 *
	 * @return the <code>System.textHighlightText</code>
	 */
	public static int getTextHighlightTextColor() {

		return SystemColor.textHighlightText.getRGB();

	}

	/**
	 * Returns the <code>System.textText</code>.
	 *
	 * @return the <code>System.textText</code>
	 */
	public static int getTextTextColor() {

		return SystemColor.textText.getRGB();

	}

}
