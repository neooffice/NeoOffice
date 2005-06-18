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
import java.awt.FontMetrics;
import java.awt.Graphics2D;
import java.awt.GraphicsEnvironment;
import java.awt.image.BufferedImage;
import java.awt.peer.FontPeer;
import java.util.ArrayList;

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
	 * Cached default font.
	 */
	private static VCLFont defaultFont = null;

	/**
	 * Cached fonts.
	 */
	private static VCLFont[] fonts = null;

	/**
	 * Cached native graphics.
	 */
	private static Graphics2D graphics = new BufferedImage(1, 1, BufferedImage.TYPE_INT_ARGB_PRE).createGraphics();

	/**
	 * Returns the default font adjusted to the specified size and style.
	 *
	 * @param s the size of the font
	 * @param b <code>true</code> if the font is bold
	 * @param i <code>true</code> if the font is italic
	 * @param o the orientation of the font in degrees
	 * @param a <code>true</code> to enable antialiasing and <code>false</code>
	 *  to disable antialiasing
	 * @param v <code>true</code> if the font is vertical
	 * @param x the X axis scale factor
	 * @return the default font adjusted to the specified size and style
	 */
	static VCLFont getDefaultFont(int s, boolean b, boolean i, short o, boolean a, boolean v, double x) {

		// Set default font
		if (defaultFont == null)
			defaultFont = new VCLFont("Dialog", VCLFont.FAMILY_DONTKNOW, 1, o, false, false, true, false, x);

		return new VCLFont(VCLFont.defaultFont.getName(), VCLFont.defaultFont.getFamilyType(), s, o, b, i, a, v, x);

	}

	/**
	 * Returns an array containing one-point instances of all fonts
	 * available in the local <code>GraphicsEnvironment</code>.
	 *
	 * @return an array of <code>VCLFont</code> objects
	 */
	public static VCLFont[] getAllFonts() {

		// Initialize the cached fonts
		if (fonts == null) {
			// Get all of the fonts and screen out duplicates
			String[] fontNames;
			if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX && VCLPlatform.getJavaVersion() < VCLPlatform.JAVA_VERSION_1_4) {
 				fontNames = GraphicsEnvironment.getLocalGraphicsEnvironment().getAvailableFontFamilyNames();
			}
			else {
				Font[] fonts = GraphicsEnvironment.getLocalGraphicsEnvironment().getAllFonts();
				fontNames = new String[fonts.length];
				for (int i = 0; i < fonts.length; i++)
					fontNames[i] = fonts[i].getName();
			}

			// Java sometimes sets Times to Times Roman
			if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX) {
				int timesRomanIndex = -1;
				for (int i = 0; i < fontNames.length; i++) {
					if (fontNames[i].equals("Times")) {
						timesRomanIndex = -1;
						break;
					}
					else if (fontNames[i].equals("Times Roman")) {
						timesRomanIndex = i;
					}
				}

				if (timesRomanIndex > 0)
					fontNames[timesRomanIndex] = "Times";
			}

			ArrayList array = new ArrayList();
			for (int i = 0; i < fontNames.length; i++) {
				// Get rid of hidden Mac OS X fonts
				if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX && (fontNames[i].startsWith(".") || fontNames[i].equals("LastResort")))
					continue;

				// Get family type
				int type;
				if (fontNames[i].indexOf("Mono") >= 0)
					type = VCLFont.FAMILY_MODERN;
				else if (fontNames[i].indexOf("Serif") >= 0 || fontNames[i].indexOf("Times") >= 0 || fontNames[i].indexOf("Roman") >= 0)
					type = VCLFont.FAMILY_ROMAN;
				else
					type = VCLFont.FAMILY_SWISS;

				array.add(new VCLFont(fontNames[i], type, 1, (short)0, false, false, true, false, 1.0));
			}
	
			fonts = (VCLFont[])array.toArray(new VCLFont[array.size()]);
		}

		return fonts;

	}

	/**
	 * The antialiased flag.
	 */
	private boolean antialiased = false;

	/**
	 * The cached ascent.
	 */
	private int ascent = 0;

	/**
	 * The bold flag.
	 */
	private boolean bold = false;

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
	 * The italic flag.
	 */
	private boolean italic = false;

	/**
	 * The cached leading.
	 */
	private int leading = 0;

	/**
	 * The cached name.
	 */
	private String name = null;

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
	 * The family type.
	 */
	private int type = VCLFont.FAMILY_SWISS;

	/**
	 * The vertical flag.
	 */
	private boolean vertical = false;

	/**
	 * Constructs a new <code>VCLFont</code> instance.
	 *
	 * @param n the name of the font
	 * @param ft the family type of the font
	 * @param s the size of the font
	 * @param o the orientation of the new <code>VCLFont</code> in degrees
	 * @param b <code>true</code> if the font is bold
	 * @param i <code>true</code> if the font is italic
	 * @param a <code>true</code> to enable antialiasing and <code>false</code>
	 *  to disable antialiasing
	 * @param v <code>true</code> if the font is vertical 
	 * @param x the X axis scale factor
	 */
	VCLFont(String n, int ft, int s, short o, boolean b, boolean i, boolean a, boolean v, double x) {

		antialiased = a;
		// Mac OS X applications and printing can't handle artificial bold and
		// italics generation very well so we always use the plain version
		if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX) {
			bold = false;
			italic = false;
		}
		else {
			bold = b;
			italic = i;
		}
		name = n;
		orientation = o;
		scaleX = x;
		size = s;
		type = ft;
		vertical = v;

		// Cache font and font metrics
		int style = Font.PLAIN;
		if (bold)
			style |= Font.BOLD;
		if (italic)
			style |= Font.ITALIC;
		font = new Font(name, style, size);

		// Exceptions can be thrown if a font is disabled or removed
		try {
			fontMetrics = VCLFont.graphics.getFontMetrics(font);
		}
		catch (Throwable t) {
			font = getDefaultFont().getFont();
			fontMetrics = VCLFont.graphics.getFontMetrics(font);
		}

		// Get size metrics
		ascent = fontMetrics.getMaxAscent();
		descent = fontMetrics.getMaxDescent();
		leading = fontMetrics.getLeading();
		if (ascent < 0)
			ascent *= -1;
		if (descent < 0)
			descent *= -1;
		if (leading < 0)
			leading *= -1;

		// Mac OS X seems to understate the actual advance
		if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX)
			ascent++;

	}

	/**
	 * Creates a new <code>VCLFont</code> object by replicating this
	 * <code>VCLFont</code> object and applying a new size.
	 *
	 * @param s the size for the new <code>VCLFont</code>
	 * @param b whether the new <code>VCLFont</code> should be bold
	 * @param i whether the new <code>VCLFont</code> should be italic
	 * @param o the orientation of the new <code>VCLFont</code> in degrees
	 * @param a <code>true</code> to enable antialiasing and <code>false</code>
	 *  to disable antialiasing
	 * @param v <code>true</code> if the font is vertical 
	 * @param x the X axis scale factor
	 * @return a new <code>VCLFont</code> object
	 */
	public VCLFont deriveFont(int s, boolean b, boolean i, short o, boolean a, boolean v, double x) {

		return new VCLFont(name, type, s, o, b, i, a, v, x);

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
	 * Returns the default font adjusted to this font's size and style.
	 *
	 * @return the default font adjusted to this font's size and style
	 */
	public VCLFont getDefaultFont() {

		return VCLFont.getDefaultFont(size, bold, italic, orientation, antialiased, vertical, scaleX);

	}

	/**
	 * Returns the family type.
	 *
	 * @return the family type
	 */
	public int getFamilyType() {

		return type;

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
	 * Returns the logical name of the <code>Font</code>.
	 *
	 * @return the logical name of the <code>Font</code>
	 */
	public String getName() {

		return name;

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
	 * Returns the <code>FontPeer</code>.
	 *
	 * @return the <code>FontPeer</code>
	 */
	public FontPeer getPeer() {

		return font.getPeer();

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
	 * Indicates whether or not the <code>Font</code> is bold.
	 *
	 * @return <code>true</code> if the <code>Font</code> is bold
	 */
	public boolean isBold() {

		return bold;

	}

	/**
	 * Indicates whether or not the <code>Font</code> is italic.
	 *
	 * @return <code>true</code> if the <code>Font</code> is italic
	 */
	public boolean isItalic() {

		return italic;

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
