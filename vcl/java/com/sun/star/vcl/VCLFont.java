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
		for (int j = 0; j < fontFamilies.length; j++) {
			String name = fontFamilies[j].toLowerCase();
			boolean b = false;
			boolean i = false;
			if (macosx) {
				// Get rid of hidden Mac OS X fonts
				if (name.startsWith("."))
					continue;
				// Determine bold and italic settings
				if (name.endsWith(" bold italic")) {
					b = true;
					i = true;
				}
				else if (name.endsWith(" bold")) {
					b = true;
				}
				else if (name.endsWith(" italic")) {
					i = true;
				}
			}
			Font font = new Font(fontFamilies[j], Font.PLAIN, 1);
			if (font.isPlain())
				array.add(new VCLFont(font, (short)0, b, i, true));
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
	 * The bold flag.
	 */
	private boolean bold = false;

	/**
	 * The font.
	 */
	private Font font = null;

	/**
	 * The font metrics.
	 */
	private FontMetrics fontMetrics = null;

	/**
	 * The italic flag.
	 */
	private boolean italic = false;

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
	 * @param b <code>true</code> if the font is bold
	 * @param i <code>true</code> if the font is italic
	 * @param a <code>true</code> to enable antialiasing and <code>false</code>
	 *  to disable antialiasing
	 */
	VCLFont(Font f, short o, boolean b, boolean i, boolean a) {

		antialiased = a;
		bold = b;
		font = f;
		italic = i;
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
	 * @param b whether the new <code>VCLFont</code> should be bold
	 * @param i whether the new <code>VCLFont</code> should be italic
	 * @param o the orientation of the new <code>VCLFont</code> in degrees
	 * @param a <code>true</code> to enable antialiasing and <code>false</code>
	 *  to disable antialiasing
	 * @return a new <code>VCLFont</code> object
	 */
	public VCLFont deriveFont(int size, boolean b, boolean i, short o, boolean a) {

		boolean macosx = false;
		if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX)
			macosx = true;

		Font f = null;

		if (macosx) {
			String fontName = font.getFamily().toLowerCase();
			int index = 0;
			if ((index = fontName.lastIndexOf(" bold italic")) != -1 || (index = fontName.lastIndexOf(" bold")) != -1 || (index = fontName.lastIndexOf(" italic")) != -1 || (index = fontName.lastIndexOf(" regular")) != -1)
				fontName = fontName.substring(0, index);
			for (int j = 0; j < fontFamilies.length; j++) {
				String name = fontFamilies[j].toLowerCase();
				if (!name.startsWith(fontName))
					continue;
				if (b && i && name.endsWith(" bold italic")) {
					f = new Font(fontFamilies[j], Font.PLAIN, size);
					break;
				}
				else if (b && !i && name.endsWith(" bold")) {
					f = new Font(fontFamilies[j], Font.PLAIN, size);
					break;
				}
				else if (!b && i && name.endsWith(" italic") && !name.endsWith(" bold italic")) {
					f = new Font(fontFamilies[j], Font.PLAIN, size);
					break;
				}
				else if (!b && !i && name.endsWith(" regular")) {
					f = new Font(fontFamilies[j], Font.PLAIN, size);
					break;
				}
			}
		}
			
		if (f == null) {
			int style = Font.PLAIN;
			if (b)
				style |= Font.BOLD;
			if (i)
				style |= Font.ITALIC;
			f = font.deriveFont(style, (float)size);
		}

		return new VCLFont(f, o, b, i, a);

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
