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
import java.awt.Rectangle;
import java.awt.font.GlyphVector;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.text.AttributedString;

/**
 * The Java class that implements the JavaLayout C++ class methods.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public final class VCLTextLayout {

	/**
	 * The bounds.
	 */
	private Rectangle bounds = null;

	/*
	 * The beginning index of the substring that this glyph vector applies to.
	 */
	private int beginIndex = 0;

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

	/*
	 * The ending index of the substring that this glyph vector applies to.
	 */
	private int endIndex = 0;

	/**
	 * The glyph vector.
	 */
	private GlyphVector glyphs = null;

	/**
	 * The graphics.
	 */
	private VCLGraphics graphics = null;

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

		// Force glyph layout to match current width
		justify(getWidth());

		graphics.drawTextArray(glyphs, x, y, orientation, color, font.isAntialiased());

	}

	/**
	 * Returns the bounds of the glyph vector.
	 *
	 * @return the bounds of the glyph vector
	 */
	public Rectangle getBounds() {

		// Cache the bounds
		if (bounds == null)
			bounds = glyphs.getLogicalBounds().getBounds();

		return bounds;
	}

	/**
	 * Returns an array of the cursor advances for each character.
	 *
	 * @return an array of the cursor advances for each character
	 */
	public int[] getCaretPositions() {

		// Cache the caret positions
		if (caretPositions == null) {
			int[] advances = getDXArray();
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
	 * Returns an array of the advances for each character.
	 *
	 * @return an array of the advances for each character
	 */
	public int[] getDXArray() {

		// Force glyph layout to match current width
		justify(getWidth());

		return charAdvances;

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

		int[] advances = getDXArray();

		if (((getBounds().width * factor) + (count * charExtra)) <= maxWidth)
			return -1;

		// Iterate through char widths until the maximum width is reached
		int currentWidth = 0;
		for ( int i = 0; i < count; i++ )
		{
			currentWidth += (advances[i] * factor);
			if (currentWidth >= maxWidth)
				return i + beginIndex;
			currentWidth += charExtra;
		}

		return -1;

	}

	/**
	 * Returns the width of the glyph vector.
	 *
	 * @return the width of the glyph vector
	 */
	public int getWidth() {

		// Cache the bounds
		if (bounds == null) {
			Rectangle2D r = glyphs.getLogicalBounds();
			bounds = r.getBounds();
			// If Java rounds the width up by more than half a pixel, round
			// down instead
			if (bounds.width - r.getWidth() > 0.5)
				bounds.width--;
		}

		return bounds.width;
	}

	/**
	 * Justifies the glyph vector to fit within the specified width.
	 *
	 * @param the desired width of the glyph vector
	 */
	public void justify(int width) {

		double totalAdjust = width - glyphs.getGlyphPosition(count).getX();
		if (totalAdjust == 0 && charAdvances != null)
			return;

		bounds = null;

		// Adjust and cache the character advances
		double charAdjust = totalAdjust / count;
		charAdvances = new int[count];
		int previousAdvance = 0;
		Point2D currentAdvance;
		int i;
		int n = charAdvances.length - 1;
		for (i = 0; i < n; i++) {
			currentAdvance = glyphs.getGlyphPosition(i + 1);
			currentAdvance.setLocation(currentAdvance.getX() + charAdjust, currentAdvance.getY());
			glyphs.setGlyphPosition(i + 1, currentAdvance);
			charAdvances[i] = (int)(currentAdvance.getX() - previousAdvance);
			previousAdvance += charAdvances[i];
		}
		currentAdvance = glyphs.getGlyphPosition(i + 1);
		currentAdvance.setLocation((double)width, currentAdvance.getY());
		glyphs.setGlyphPosition(i + 1, currentAdvance);
		charAdvances[i] = width - previousAdvance;

	}

	/**
	 * Layout text.
	 *
	 * @param s the string to layout
	 * @param b the beginning index
	 * @param e the ending index
	 * @param f the layout flags
	 */
	public void layoutText(String s, int b, int e, int f) {

		beginIndex = b;
		endIndex = e;
		count = endIndex - beginIndex;

		bounds = null;
		charAdvances = null;
		caretPositions = null;

		// Create the attributed string and the glyph vector
		AttributedString as = new AttributedString(s);
		glyphs = font.getFont().createGlyphVector(graphics.getFontRenderContext(), as.getIterator());

	}

	/**
	 * Sets the advances for each character.
	 *
	 * @param advances an array of the advances for each character
	 */
	public void setDXArray(int[] advances) {

		bounds = null;

		// Adjust and cache the character advances
		charAdvances = advances;
		Point2D currentAdvance;
		for (int i = 0; i < charAdvances.length; i++) {
			currentAdvance = glyphs.getGlyphPosition(i + 1);
			currentAdvance.setLocation((double)charAdvances[i], currentAdvance.getY());
			glyphs.setGlyphPosition(i + 1, currentAdvance);
		}

	}

}
