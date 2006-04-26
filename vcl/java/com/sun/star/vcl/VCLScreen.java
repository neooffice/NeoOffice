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

import java.awt.Dimension;
import java.awt.Frame;
import java.awt.Graphics2D;
import java.awt.Insets;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.SystemColor;
import java.awt.Toolkit;
import java.awt.Window;
import java.awt.image.BufferedImage;

/** 
 * The Java class that implements the convenience methods for accessing
 * screen information.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public final class VCLScreen {

	/**
	 * The minimum screen resolution.
	 */
	public final static int MIN_SCREEN_RESOLUTION = 72;

	/**
	 * The cached frame insets.
	 */
	private static Insets frameInsets = null;

	/**
	 * The cached minimum frame size.
	 */
	private static Dimension minimumFrameSize = null;

	/**
	 * Cached buffered image.
	 */
	private static BufferedImage image = new BufferedImage(1, 1, BufferedImage.TYPE_INT_ARGB_PRE);

	/**
	 * Initialize screen size and frame insets.
	 */
	static {

		Frame f = new Frame();
		f.setSize(1, 1);
		f.addNotify();
		frameInsets = f.getInsets();
		minimumFrameSize = f.getSize();
		f.removeNotify();
		f.dispose();

	}

	/**
	 * Returns the <code>System.control</code>.
	 *
	 * @return the <code>System.control</code>
	 */
	public static int getControlColor() {

		int color = 0x00000000;

		Graphics2D g = image.createGraphics();
		if (g != null) {
			try {
				g.setColor(SystemColor.control);
				g.fillRect(0, 0, 1, 1);
				color = image.getRGB(0, 0);
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			g.dispose();
		}

		return color;

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
	 * Gets the minimum <code>Frame</code> size.
	 *
	 * @return the minimum <code>Frame</code> size
	 */
	public static Dimension getMinimumFrameSize() {

		return minimumFrameSize;

	}

	/**
	 * Returns the <code>System.textHighlight</code>.
	 *
	 * @return the <code>System.textHighlight</code>
	 */
	public static int getTextHighlightColor() {

		int color = 0x00000000;

		Graphics2D g = image.createGraphics();
		if (g != null) {
			try {
				g.setColor(SystemColor.textHighlight);
				g.fillRect(0, 0, 1, 1);
				color = image.getRGB(0, 0);
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			g.dispose();
		}

		return color;

	}

	/**
	 * Returns the <code>System.textHighlightText</code>.
	 *
	 * @return the <code>System.textHighlightText</code>
	 */
	public static int getTextHighlightTextColor() {

		int color = 0x00000000;

		Graphics2D g = image.createGraphics();
		if (g != null) {
			try {
				g.setColor(SystemColor.textHighlightText);
				g.fillRect(0, 0, 1, 1);
				color = image.getRGB(0, 0);
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			g.dispose();
		}

		return color;

	}

	/**
	 * Returns the <code>System.textText</code>.
	 *
	 * @return the <code>System.textText</code>
	 */
	public static int getTextTextColor() {

		int color = 0x00000000;

		Graphics2D g = image.createGraphics();
		if (g != null) {
			try {
				g.setColor(SystemColor.textText);
				g.fillRect(0, 0, 1, 1);
				color = image.getRGB(0, 0);
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			g.dispose();
		}

		return color;

	}

}
