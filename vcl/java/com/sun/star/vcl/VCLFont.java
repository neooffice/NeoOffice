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

import java.awt.Font;
import java.awt.FontFormatException;
import java.awt.FontMetrics;
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
	 * The cached ascent.
	 */
	private int ascent = 0;

	/**
	 * The cached descent.
	 */
	private int descent = 0;

	/**
	 * The cached font.
	 */
	private Font font = null;

	/**
	 * The cached font metrics.
	 */
	private FontMetrics fontMetrics = null;

	/**
	 * The cached leading.
	 */
	private int leading = 0;

	/**
	 * The cached native font.
	 */
	private int nativeFont = 0;

	/**
	 * The cached orientation.
	 */
	private short orientation = 0;

	/**
	 * The cached X axis scale factor.
	 */
	private double scaleX = 0.0;

	/**
	 * The cached size.
	 */
	private int size = 0;

	/**
	 * The vertical flag.
	 */
	private boolean vertical = false;

	/**
	 * Constructs a new <code>VCLFont</code> instance.
	 *
	 * @param f the font
	 * @param nf the native font
	 * @param s the size of the font
	 * @param o the orientation of the new <code>VCLFont</code> in degrees
	 * @param a <code>true</code> to enable antialiasing and <code>false</code>
	 *  to disable antialiasing
	 * @param v <code>true</code> if the font is vertical 
	 * @param x the X axis scale factor
	 */
	VCLFont(Font f, int nf, int s, short o, boolean a, boolean v, double x) throws FontFormatException {

		antialiased = a;
		nativeFont = nf;
		orientation = o;
		scaleX = x;
		size = s;
		vertical = v;

		// Cache font and font metrics
		font = f.deriveFont(Font.PLAIN, size);
		Graphics2D g = VCLFont.image.createGraphics();
		if (g != null)
		{
			try {
				fontMetrics = g.getFontMetrics(font);
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			g.dispose();
		}

		// Get size metrics
		if (fontMetrics != null) {
			ascent = fontMetrics.getMaxAscent();
			descent = fontMetrics.getMaxDescent();
			leading = fontMetrics.getLeading();
			if (ascent < 0)
				ascent *= -1;
			if (descent < 0)
				descent *= -1;
			if (leading < 0)
				leading *= -1;
		}

		if (ascent == 0 && descent == 0 && leading == 0)
			throw new FontFormatException("Font " + font.getName() + " has no height");

		// Mac OS X seems to understate the actual advance
		ascent++;

	}

	/**
	 * Constructs a new <code>VCLFont</code> instance.
	 *
	 * @param n the name of the font
	 * @param nf the native font
	 * @param s the size of the font
	 * @param o the orientation of the new <code>VCLFont</code> in degrees
	 * @param a <code>true</code> to enable antialiasing and <code>false</code>
	 *  to disable antialiasing
	 * @param v <code>true</code> if the font is vertical 
	 * @param x the X axis scale factor
	 */
	public VCLFont(String n, int nf, int s, short o, boolean a, boolean v, double x) throws FontFormatException {

		this(new Font(n, Font.PLAIN, s), nf, s, o, a, v, x);

	}

	/**
	 * Creates a new <code>VCLFont</code> object by replicating this
	 * <code>VCLFont</code> object and applying a new size.
	 *
	 * @param s the size for the new <code>VCLFont</code>
	 * @param o the orientation of the new <code>VCLFont</code> in degrees
	 * @param a <code>true</code> to enable antialiasing and <code>false</code>
	 *  to disable antialiasing
	 * @param v <code>true</code> if the font is vertical 
	 * @param x the X axis scale factor
	 * @return a new <code>VCLFont</code> object
	 */
	public VCLFont deriveFont(int s, short o, boolean a, boolean v, double x) throws FontFormatException {

		return new VCLFont(font, nativeFont, s, o, a, v, x);

	}

	/**
	 * Determines the font ascent of the <code>Font</code>.
	 *
	 * @return the font ascent of the <code>Font</code>
	 */
	public int getAscent() {

		return ascent;

	}

	/**
	 * Determines the font descent of the <code>Font</code>.
	 *
	 * @return the font descent of the <code>Font</code>
	 */
	public int getDescent() {

		return descent;

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
	 * Determines the kerning adjustment for the specified characters.
	 *
	 * @param a the first character
	 * @param b the second character
	 * @return the kerning adjustment for the specified characters
	 */
	public int getKerning(char a, char b) {

		// Get width without kerning
		int width = 0;
		if (Character.getType(a) != Character.NON_SPACING_MARK || !font.canDisplay(a))
			width += fontMetrics.charWidth(a);
		if (Character.getType(b) != Character.NON_SPACING_MARK || !font.canDisplay(b))
			width += fontMetrics.charWidth(b);

		// Subtract the width with kerning
		width -= fontMetrics.charsWidth(new char[]{ a, b }, 0, 2);

		return width;

	}

	/**
	 * Determines the standard leading of the <code>Font</code>.
	 *
	 * @return the standard leading of the <code>Font</code>
	 */
	public int getLeading() {

		return leading;

	}

	/**
	 * Returns the font's name.
	 *
	 * @return the font's name
	 */
	public String getName() {

		return font.getName();

	}

	/**
	 * Returns the native font.
	 *
	 * @return the native font
	 */
	public int getNativeFont() {

		return nativeFont;

	}

	/**
	 * Returns the orientation of the <code>VCLFont</code> in degrees.
	 *
	 * @return the orientation of the <code>VCLFont</code>
	 */
	public short getOrientation() {

		return orientation;

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
	public double getScaleX() {

		return scaleX;

	}

	/**
	 * Returns the point size of the <code>Font</code>.
	 *
	 * @return the point size of the <code>Font</code>
	 */
	public int getSize() {

		return size;

	}

	/**
	 * Indicates whether or not the <code>Font</code> is antialiased.
	 *
	 * @return <code>true</code> if the <code>Font</code> is antialiased 
	 */
	public boolean isAntialiased() {

		return antialiased;

	}

	/**
	 * Indicates whether or not the <code>Font</code> is vertical.
	 *
	 * @return <code>true</code> if the <code>Font</code> is vertical
	 */
	public boolean isVertical() {

		return vertical;

	}

}
