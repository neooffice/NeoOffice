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
	 * Cached fonts.
	 */
	private static VCLFont[] fonts = null;

	/**
	 * Cached native graphics.
	 */
	private static Graphics2D graphics = new BufferedImage(1, 1, BufferedImage.TYPE_INT_ARGB_PRE).createGraphics();

	/**
	 * Initialize the cached fonts.
	 */
	static {

		boolean macosx = false;
		if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX)
			macosx = true;

		// Get all of the fonts and screen out duplicates
		String[] fontFamilies = GraphicsEnvironment.getLocalGraphicsEnvironment().getAvailableFontFamilyNames();
		ArrayList array = new ArrayList();
		for (int i = 0; i < fontFamilies.length; i++) {
			String name = fontFamilies[i].toLowerCase();
			if (name.endsWith("bold"))
				continue;
			else if (name.endsWith("italic"))
				continue;
			else if (name.endsWith("bold italic"))
				continue;
			// Get rid of hidden Mac OS X fonts
			if (macosx && name.startsWith("."))
				continue;
			// Chop off redundant portions of the family name
			int j = name.indexOf(" regular");
			if (j >= 0)
				fontFamilies[i] = fontFamilies[i].substring(0, j);
			j = name.indexOf(" medium");
			if (j >= 0)
				fontFamilies[i] = fontFamilies[i].substring(0, j);
			Font font = new Font(fontFamilies[i], Font.PLAIN, 1);
			if (font.isPlain())
				array.add(new VCLFont(font, (short)0, true));
		}
		VCLFont.fonts = (VCLFont[])array.toArray(new VCLFont[array.size()]);

	}

	/**
	 * Returns an array containing one-point instances of all fonts
	 * available in the local <code>GraphicsEnvironment</code>.
	 *
	 * @return an array of <code>VCLFont</code> objects
	 */
	public static VCLFont[] getAllFonts() {

		return fonts;

	}

	/**
	 * The antialiased flag.
	 */
	private boolean antialiased = false;

	/**
	 * The font.
	 */
	private Font font = null;

	/**
	 * The font metrics.
	 */
	private FontMetrics fontMetrics = null;

	/**
	 * The cached orientation.
	 */
	private short orientation = 0;

	/**
	 * The family type.
	 */
	private int type = VCLFont.FAMILY_DONTKNOW;

	/**
	 * Constructs a new <code>VCLFont</code> instance.
	 *
	 * @param f a <code>Font</code> instance
	 * @param o the orientation of the new <code>VCLFont</code> in degrees
	 * @param b <code>true</code> to enable antialiasing and <code>false</code>
	 *  to disable antialiasing
	 */
	VCLFont(Font f, short o, boolean b) {

		antialiased = b;
		font = f;
		orientation = o;

		// Get family type
		String name = f.getFamily().toLowerCase();
		if (name.startsWith("monospaced"))
			type = VCLFont.FAMILY_MODERN;
		else if (name.startsWith("sansserif"))
			type = VCLFont.FAMILY_SWISS;
		else if (name.startsWith("serif"))
			type = VCLFont.FAMILY_ROMAN;

		// Get the font metrics
		fontMetrics = VCLFont.graphics.getFontMetrics(f);

	}

	/**
	 * Creates a new <code>VCLFont</code> object by replicating this
	 * <code>VCLFont</code> object and applying a new size.
	 *
	 * @param size the size for the new <code>VCLFont</code>
	 * @param bold whether the new <code>VCLFont</code> should be bold
	 * @param italic whether the new <code>VCLFont</code> should be italic
	 * @param o the orientation of the new <code>VCLFont</code> in degrees
	 * @param b <code>true</code> to enable antialiasing and <code>false</code>
	 *  to disable antialiasing
	 * @return a new <code>VCLFont</code> object
	 */
	public VCLFont deriveFont(int size, boolean bold, boolean italic, short o, boolean b) {

		int style = Font.PLAIN;
		if (bold)
			style |= Font.BOLD;
		if (italic)
			style |= Font.ITALIC;
		Font f = font.deriveFont(style, (float)size);
		return new VCLFont(f, o, b);

	}

	/**
	 * Determines the font ascent of the <code>Font</code>.
	 *
	 * @return the font ascent of the <code>Font</code>
	 */
	public int getAscent() {

		return fontMetrics.getAscent();

	}

	/**
	 * Returns the advance width of the specified character.
	 *
	 * @param start the starting character
	 * @param end the ending character
	 * @return the array of advance widths of the specified characters
	 */
	public int[] getCharWidth(char start, char end) {

		int[] widths = new int[end - start + 1];
		for (char i = start; i <= end; i++) {
			if (Character.getType(i) == Character.NON_SPACING_MARK && font.canDisplay(i))
				widths[i - start] = 0;
			else
				widths[i - start] = fontMetrics.charWidth(i);
		}
		return widths;

	}

	/**
	 * Determines the font descent of the <code>Font</code>.
	 *
	 * @return the font descent of the <code>Font</code>
	 */
	public int getDescent() {

		return fontMetrics.getDescent();

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

		return fontMetrics.getLeading();

	}

	/**
	 * Returns the logical name of the <code>Font</code>.
	 *
	 * @return the logical name of the <code>Font</code>
	 */
	public String getName() {

		return font.getName();

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
	 * Determines the point size of the <code>Font</code>.
	 *
	 * @return the point size of the <code>Font</code>
	 */
	public int getSize() {

		return font.getSize();

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

		return font.isBold();

	}

	/**
	 * Indicates whether or not the <code>Font</code> is italic.
	 *
	 * @return <code>true</code> if the <code>Font</code> is italic
	 */
	public boolean isItalic() {

		return font.isItalic();

	}

}
