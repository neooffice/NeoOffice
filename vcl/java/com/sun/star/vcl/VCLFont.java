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
public class VCLFont {

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
				array.add(new VCLFont(font));
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
	 * The font.
	 */
	private Font font = null;

	/**
	 * The font metrics.
	 */
	private FontMetrics fontMetrics = null;

	/**
	 * The font name.
	 */
	private String name = null;

	/**
	 * Constructs a new <code>VCLFont</code> instance.
	 *
	 * @param f a <code>Font</code> instance
	 */
	VCLFont(Font f) {

		font = f;

		// Get the font metrics
		fontMetrics = VCLFont.graphics.getFontMetrics(f);

	}

	/**
	 * Constructs a new <code>VCLFont</code> instance.
	 *
	 * @param f a <code>Font</code> instance
	 * @param n the name to use for this font
	 */
	VCLFont(Font f, String n) {

		this(f);
		name = n;

	}

	/**
	 * Creates a new <code>VCLFont</code> object by replicating this
	 * <code>VCLFont</code> object and applying a new size.
	 *
	 * @param size the size for the new <code>VCLFont</code>
	 * @param bold whether the new <code>VCLFont</code> should be bold
	 * @param italic whether the new <code>VCLFont</code> should be italic
	 * @return a new <code>VCLFont</code> object
	 */
	public VCLFont deriveFont(int size, boolean bold, boolean italic) {

		int style = Font.PLAIN;
		if (bold)
			style |= Font.BOLD;
		if (italic)
			style |= Font.ITALIC;
		Font f = font.deriveFont(style, (float)size);
		return new VCLFont(f);

	}

	/**
	 * Determines the font ascent of the <code>Font</code>.
	 *
	 * @return the font ascent of the <code>Font</code>
	 */
	public long getAscent() {

		return (long)fontMetrics.getAscent();

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
			int type = Character.getType(i);
			if (type == Character.NON_SPACING_MARK)
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
	public long getDescent() {

		return (long)fontMetrics.getDescent();

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
	 * Determines the standard leading of the <code>Font</code>.
	 *
	 * @return the standard leading of the <code>Font</code>
	 */
	public long getLeading() {

		return (long)fontMetrics.getLeading();

	}

	/**
	 * Returns the logical name of the <code>Font</code>.
	 *
	 * @return the logical name of the <code>Font</code>
	 */
	public String getName() {

		if (name != null)
			return name;
		else
			return font.getName();

	}

	/**
	 * Determines the point size of the <code>Font</code>.
	 *
	 * @return the point size of the <code>Font</code>
	 */
	public long getSize() {

		return (long)font.getSize();

	}

	/**
	 * Indicates whether or not the <code>Font</code> is bold.
	 *
	 * @return <code> if the <code>Font</code> is bold
	 */
	public boolean isBold() {

		return font.isBold();

	}

	/**
	 * Indicates whether or not the <code>Font</code> is italic.
	 *
	 * @return <code> if the <code>Font</code> is italic
	 */
	public boolean isItalic() {

		return font.isItalic();

	}

}
