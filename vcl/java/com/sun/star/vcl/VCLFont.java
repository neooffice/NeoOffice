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

import java.awt.Font;
import java.awt.FontFormatException;
import java.awt.Graphics2D;
import java.awt.GraphicsEnvironment;
import java.awt.image.BufferedImage;
import java.util.LinkedList;

/** 
 * The Java class that implements the convenience methods for accessing Java
 * font information.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public final class VCLFont {

	/**
	 * FAMILY_DONTKNOW constant.
	 */
	public final static int FAMILY_DONTKNOW = 0;

	/**
	 * FAMILY_DECORATIVE constant.
	 */
	public final static int FAMILY_DECORATIVE = 1;

	/**
	 * FAMILY_MODERN constant.
	 */
	public final static int FAMILY_MODERN = 2;

	/**
	 * FAMILY_ROMAN constant.
	 */
	public final static int FAMILY_ROMAN = 3;

    /**
     * FAMILY_SCRIPT constant.
     */
    public final static int FAMILY_SCRIPT = 4;

	/**
	 * FAMILY_SWISS constant.
	 */
	public final static int FAMILY_SWISS = 5;

	/**
	 * FAMILY_SYSTEM constant.
	 */
	public final static int FAMILY_SYSTEM = 6;

	/**
	 * Cached buffered image.
	 */
	private static BufferedImage image = new BufferedImage(1, 1, BufferedImage.TYPE_INT_ARGB_PRE);

	/**
	 * The antialiased flag.
	 */
	private boolean antialiased = false;

	/**
	 * The cached font.
	 */
	private Font font = null;

	/**
	 * The cached X axis scale factor.
	 */
	private double scaleX = 0.0;

	/**
	 * Constructs a new <code>VCLFont</code> instance.
	 *
	 * @param f the font
	 * @param s the size of the font
	 * @param a <code>true</code> to enable antialiasing and <code>false</code>
	 *  to disable antialiasing
	 * @param x the X axis scale factor
	 */
	VCLFont(Font f, int s, boolean a, double x) throws FontFormatException {

		antialiased = a;
		scaleX = x;

		// Cache font and font metrics
		font = f.deriveFont((float)s);

	}

	/**
	 * Constructs a new <code>VCLFont</code> instance.
	 *
	 * @param n the name of the font
	 * @param s the size of the font
	 * @param a <code>true</code> to enable antialiasing and <code>false</code>
	 *  to disable antialiasing
	 * @param x the X axis scale factor
	 */
	public VCLFont(String n, int s, boolean a, double x) throws FontFormatException {

		antialiased = a;
		font = new Font(n, Font.PLAIN, s);
		scaleX = x;

	}

	/**
	 * Returns the <code>Font</code>.
	 *
	 * @return the <code>Font</code>
	 */
	Font getFont() {

		return font;

	}

	/**
	 * Returns the font's PostScript name.
	 *
	 * @return the font's PostScript name
	 */
	public String getPSName() {

		return font.getPSName();

	}

	/**
	 * Returns the X axis scale factor.
	 *
	 * @return the X axis scale factor
	 */
	double getScaleX() {

		return scaleX;

	}

	/**
	 * Indicates whether or not the <code>Font</code> is antialiased.
	 *
	 * @return <code>true</code> if the <code>Font</code> is antialiased 
	 */
	boolean isAntialiased() {

		return antialiased;

	}

}
