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
 *  Patrick Luby, June 2004
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2004 by Patrick Luby (patrick.luby@planamesa.com)
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
import java.awt.font.GlyphVector;
import java.text.AttributedString;

/**
 * The Java class that implements the JavaLayout C++ class methods.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public final class VCLTextLayout {

	/*
	 * The number of characters.
	 */
	private int count = 0;

	/**
	 * The font.
	 */
	private VCLFont font = null;

	/**
	 * The caret positions.
	 */
	private int[] caretPositions = null;

	/**
	 * The character advances.
	 */
	private int[] charAdvances = null;

	/**
	 * The glyph vector.
	 */
	private GlyphVector glyphs = null;

	/**
	 * The graphics.
	 */
	private VCLGraphics graphics = null;

	/**
	 * The width.
	 */
	private int width = -1;

	/**
	 * Constructs a new <code>VCLTextLayout</code> instance.
	 *
	 * @param g the <code>VCLGraphics</code> instance
	 * @param f the <code>VCLFont</code> instance
	 */
	public VCLTextLayout(VCLGraphics g, VCLFont f) {

		graphics = g;
		font = f;
	}

	/**
	 * Draws all of the text.
	 *
	 * @param x the x coordinate
	 * @param y the y coordinate
	 * @param orientation the orientation to draw the text runs
	 * @param color the color to draw the text runs
	 */
	public void drawText(int x, int y, int orientation, int color) {

		graphics.drawTextArray(glyphs, x, y, orientation, color, font.isAntialiased());

	}

	/**
	 * Returns an array of the offsets for each character.
	 *
	 * @return an array of the offsets for each character
	 */
	public int[] fillDXArray() {

		// Cache the glyph vector width
		if (width < 0)
			width = glyphs.getLogicalBounds().getBounds().width;

		// Cache the character advances
		if (charAdvances == null) {
			charAdvances = new int[count];
			int currentAdvance = 0;
			int previousAdvance = 0;
			int i;
			for (i = 0; i < charAdvances.length - 1; i++) {
				currentAdvance = (int)glyphs.getGlyphPosition(i + 1).getX();
				charAdvances[i] = currentAdvance - previousAdvance;
				previousAdvance = currentAdvance;
			}
			// Compute the advance of the last glyph
			charAdvances[i] = width - currentAdvance;
		}

		return charAdvances;

	}

	/**
	 * Returns an array of the cursor offsets for each character.
	 *
	 * @return an array of the cursor offsets for each character
	 */
	public int[] getCaretPositions() {

		// Cache the caret positions
		if (caretPositions == null) {
			int[] advances = fillDXArray();
			caretPositions = new int[count * 2];
			int currentPosition = 0;
			for (int i = 0; i < caretPositions.length; i += 2) {
				caretPositions[i] = currentPosition;
				currentPosition += advances[i / 2];
				caretPositions[i + 1] = currentPosition;
			}
		}

		return caretPositions;

	}

	/**
	 * Returns the text break character.
	 *
	 * @param the maximum width
	 * @param the number of extra characters
	 * @param the factor to apply to the maximum width
	 * @return the text break character
	 */
	public int getTextBreak(int maxWidth, int charExtra, int factor) {

		int[] advances = fillDXArray();

		if (((width * factor) + (count * charExtra)) <= maxWidth)
			return Integer.MAX_VALUE;

		// Iterate through char widths until the maximum width is reached
		int currentWidth = 0;
		for ( int i = 0; i < count; i++ )
		{
			currentWidth += (advances[i] * factor);
			if (currentWidth >= maxWidth)
				return i;
		}

		return Integer.MAX_VALUE;

	}

	/**
	 * Layout text.
	 *
	 * @param s the string to layout
	 */
	public void layoutText(String s) {

		count = s.length();
		charAdvances = null;
		caretPositions = null;
		width = -1;

		// Create the attributed string and the glyph vector
		AttributedString as = new AttributedString(s);
		glyphs = font.getFont().createGlyphVector(graphics.getFontRenderContext(), as.getIterator());

	}

}
