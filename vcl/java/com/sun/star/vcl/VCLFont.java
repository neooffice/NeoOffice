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
	 * Cached font family names.
	 */
	private static String[] fontFamilies = null;

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
	 * Initialize the cached fonts.
	 */
	static {

		boolean macosx = false;
		if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX)
			macosx = true;

		// Get all of the fonts and screen out duplicates
		fontFamilies = GraphicsEnvironment.getLocalGraphicsEnvironment().getAvailableFontFamilyNames();
		ArrayList array = new ArrayList();
		for (int i = 0; i < fontFamilies.length; i++) {
			String name = fontFamilies[i].toLowerCase();
			// Get rid of hidden, bold, and italic Mac OS X fonts
			if (macosx && name.startsWith("."))
				continue;
			array.add(new VCLFont(fontFamilies[i], 1, (short)0, false, false, true));
		}

		fonts = (VCLFont[])array.toArray(new VCLFont[array.size()]);

		// Set default font
		defaultFont = new VCLFont("Dialog", 1, (short)0, false, false, true);

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
	 * The cached size.
	 */
	private int size = 0;

	/**
	 * The cached style.
	 */
	private int style = 0;

	/**
	 * The family type.
	 */
	private int type = VCLFont.FAMILY_SWISS;

	/**
	 * Constructs a new <code>VCLFont</code> instance.
	 *
	 * @param n the name of the font
	 * @param s the size of the font
	 * @param o the orientation of the new <code>VCLFont</code> in degrees
	 * @param b <code>true</code> if the font is bold
	 * @param i <code>true</code> if the font is italic
	 * @param a <code>true</code> to enable antialiasing and <code>false</code>
	 *  to disable antialiasing
	 */
	VCLFont(String n, int s, short o, boolean b, boolean i, boolean a) {

		antialiased = a;
		bold = b;
		italic = i;
		name = n;
		orientation = o;
		size = s;

		// Cache style
		style = Font.PLAIN;
		if (bold)
			style |= Font.BOLD;
		if (italic)
			style |= Font.ITALIC;

		// Cache font and font metrics
		font = new Font(name, style, size);

		// Exceptions can be thrown if a font is disabled or removed
		try {
			fontMetrics = VCLFont.graphics.getFontMetrics(font);
		}
		catch (Throwable t) {
			font = getDefaultFont().getFont();
			fontMetrics = VCLFont.graphics.getFontMetrics(font);
		}

		// Get family type
		String fontName = name.toLowerCase();
		if (fontName.startsWith("monospaced"))
			type = VCLFont.FAMILY_MODERN;
		else if (fontName.startsWith("sansserif"))
			type = VCLFont.FAMILY_SWISS;
		else if (fontName.startsWith("serif"))
			type = VCLFont.FAMILY_ROMAN;

		// Get size metrics
		leading = fontMetrics.getLeading();
		ascent = fontMetrics.getAscent() + leading;
		descent = fontMetrics.getDescent();
		if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX) {
			// The Asian fonts return strange values so we adjust them here
			if (leading == 0 && descent < 0) {
				leading = ascent / 2;
				descent = leading;
			}
 			if (fontName.startsWith("hiragino") || fontName.startsWith("osaka")) {
				leading /= 2;
				ascent -= leading;
				descent += leading;
			}
		}

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
	 * @return a new <code>VCLFont</code> object
	 */
	public VCLFont deriveFont(int s, boolean b, boolean i, short o, boolean a) {

		if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX) {
			String fontName = name.toLowerCase();
			int index = 0;
			if ((index = fontName.lastIndexOf(" bold italic")) != -1 || (index = fontName.lastIndexOf(" bold")) != -1 || (index = fontName.lastIndexOf(" italic")) != -1 || (index = fontName.lastIndexOf(" regular")) != -1)
				fontName = fontName.substring(0, index);
			for (int j = 0; j < fontFamilies.length; j++) {
				String fontFamilyName = fontFamilies[j].toLowerCase();
				if (!fontFamilyName.startsWith(fontName))
					continue;
				if (b && i && fontFamilyName.endsWith(" bold italic"))
					return new VCLFont(fontFamilies[j], s, o, false, false, a);
				else if (b && !i && fontFamilyName.endsWith(" bold"))
					return new VCLFont(fontFamilies[j], s, o, false, false, a);
				else if (!b && i && fontFamilyName.endsWith(" italic") && !fontFamilyName.endsWith(" bold italic"))
					return new VCLFont(fontFamilies[j], s, o, false, false, a);
				else if (!b && !i && fontFamilyName.endsWith(" regular"))
					return new VCLFont(fontFamilies[j], s, o, false, false, a);
			}
		}

		return new VCLFont(name, s, o, b, i, a);

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

		return descent;

	}

	/**
	 * Returns the default font adjusted to this font's size and style.
	 *
	 * @return the default font adjusted to this font's size and style
	 */
	public VCLFont getDefaultFont() {

		return new VCLFont(VCLFont.defaultFont.getName(), size, orientation, bold, italic, antialiased);

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
	 * Determines the point size of the <code>Font</code>.
	 *
	 * @return the point size of the <code>Font</code>
	 */
	public int getSize() {

		return size;

	}

	/**
	 * Determines the style of the <code>Font</code>.
	 *
	 * @return the style of the <code>Font</code>
	 */
	int getStyle() {

		return style;

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

}
