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

/**
 * The Java class that implements the JavaLayout C++ class methods.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public final class VCLTextLayout {

	/**
	 * SAL_LAYOUT_BIDI_RTL constant.
	 */
	public final static int SAL_LAYOUT_BIDI_RTL = 0x0001;

	/**
	 * SAL_LAYOUT_BIDI_STRONG constant.
	 */
	public final static int SAL_LAYOUT_BIDI_STRONG = 0x0002;

	/**
	 * SAL_LAYOUT_RIGHT_ALIGN constant.
	 */
	public final static int SAL_LAYOUT_RIGHT_ALIGN = 0x0004;

	/**
	 * SAL_LAYOUT_KERNING_PAIRS constant.
	 */
	public final static int SAL_LAYOUT_KERNING_PAIRS = 0x0010;

	/**
	 * SAL_LAYOUT_KERNING_ASIAN constant.
	 */
	public final static int SAL_LAYOUT_KERNING_ASIAN = 0x0020;

	/**
	 * SAL_LAYOUT_VERTICAL constant.
	 */
	public final static int SAL_LAYOUT_VERTICAL = 0x0040;

	/**
	 * SAL_LAYOUT_COMPLEX_DISABLED constant.
	 */
	public final static int SAL_LAYOUT_COMPLEX_DISABLED = 0x0100;

	/**
	 * SAL_LAYOUT_ENABLE_LIGATURES constant.
	 */
	public final static int SAL_LAYOUT_ENABLE_LIGATURES = 0x0200;

	/**
	 * SAL_LAYOUT_SUBSTITUTE_DIGITS constant.
	 */
	public final static int SAL_LAYOUT_SUBSTITUTE_DIGITS = 0x0400;

	/**
	 * SAL_LAYOUT_KASHIDA_JUSTIFICATON constant.
	 */
	public final static int SAL_LAYOUT_KASHIDA_JUSTIFICATON = 0x0800;

	/**
	 * SAL_LAYOUT_DISABLE_GLYPH_PROCESSING constant.
	 */
	public final static int SAL_LAYOUT_DISABLE_GLYPH_PROCESSING = 0x1000;

	/**
	 * SAL_LAYOUT_FOR_FALLBACK constant.
	 */
	public final static int SAL_LAYOUT_FOR_FALLBACK = 0x2000;

	/**
	 * The bounds.
	 */
	private Rectangle bounds = null;

	/*
	 * The beginning index of the substring that this glyph vector applies to.
	 */
	private int beginIndex = 0;

	/**
	 * The caret positions.
	 */
	private int[] caretPositions = null;

	/**
	 * The character advances.
	 */
	private int[] charAdvances = null;

	/*
	 * The number of characters.
	 */
	private int charCount = 0;

	/**
	 * The character run lengths.
	 */
	private int[] charRunLengths = null;

	/*
	 * The ending index of the substring that this glyph vector applies to.
	 */
	private int endIndex = 0;

	/**
	 * The font.
	 */
	private VCLFont font = null;

	/*
	 * The number of glyphs.
	 */
	private int glyphCount = 0;

	/**
	 * The glyph run lengths.
	 */
	private int[] glyphRunLengths = null;

	/**
	 * The glyph vector.
	 */
	private GlyphVector glyphs = null;

	/**
	 * The graphics.
	 */
	private VCLGraphics graphics = null;

	/**
	 * The run flags.
	 */
	private int[] runFlags = null;

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
	 * @param color the color to draw the text runs
	 */
	public void drawText(int x, int y, int color) {

		// Force glyph layout to match current width
		justify(getWidth());

		graphics.drawTextArray(glyphs, x, y, font.getOrientation(), color, font.isAntialiased());

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
			caretPositions = new int[charCount * 2];
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

		if (((getWidth() * factor) + (charCount * charExtra)) <= maxWidth)
			return -1;

		// Iterate through char widths until the maximum width is reached
		int currentWidth = 0;
		for ( int i = 0; i < charCount; i++ )
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
			// Always round down so that ligature glyphs are not separated when
			// justifying the glyph vector
			bounds.width = (int)r.getWidth();
		}

		return bounds.width;
	}

	/**
	 * Justifies the glyph vector to fit within the specified width.
	 *
	 * @param the desired width of the glyph vector
	 */
	public void justify(int width) {

		double totalAdjust = width - glyphs.getLogicalBounds().getWidth();
		if (totalAdjust == 0 && charAdvances != null)
			return;

		bounds = null;
		caretPositions = null;

		// Adjust and cache the character advances
		double charAdjust = totalAdjust / glyphCount;
		charAdvances = new int[charCount];
		int previousAdvance = 0;
		Point2D currentAdvance;
		int previousChar = 0;
		int currentChar = 0;
		int currentGlyph = 0;
		for (int i = 0; i < charRunLengths.length; i++) {
			for (int j = 0; j < charRunLengths[i] || j < glyphRunLengths[i]; j++) {
				if (j < glyphRunLengths[i]) {
					currentAdvance = glyphs.getGlyphPosition(currentGlyph + 1);
					currentAdvance.setLocation((double)currentAdvance.getX() + charAdjust, currentAdvance.getY());
					glyphs.setGlyphPosition(currentGlyph + 1, currentAdvance);
					charAdvances[currentChar] = (int)(currentAdvance.getX() - previousAdvance);
					previousAdvance += charAdvances[currentChar];
					currentGlyph++;
				}

				if (j < charRunLengths[i])
					currentChar++;
			}
		}

		// Adjust for any rounding errors
		currentAdvance = glyphs.getGlyphPosition(glyphCount);
		currentAdvance.setLocation((double)width, currentAdvance.getY());
		glyphs.setGlyphPosition(glyphCount, currentAdvance);
		charAdvances[charCount - 1] += width - previousAdvance;

	}

	/**
	 * Layout text.
	 *
	 * @param chars the characters to be laid out
	 * @param b the beginning index
	 * @param e the ending index
	 * @param crl the character run lengths
	 * @param grl the glyph run lengths
	 * @param f the flags for each run
	 */
	public void layoutText(char[] chars, int b, int e, int[] crl, int[] grl, int[] f) {

		beginIndex = b;
		endIndex = e;
		charRunLengths = crl;
		glyphRunLengths = grl;
		runFlags = f;
		charCount = e - b;

		bounds = null;
		charAdvances = null;
		caretPositions = null;

		// Reverse characters in RTL runs
		int previousChar = 0;
		int currentChar = 0;
		for (int i = 0; i < glyphRunLengths.length; i++) {
			currentChar += glyphRunLengths[i];
			if ((runFlags[i] & VCLTextLayout.SAL_LAYOUT_BIDI_RTL) != 0) {
				for (int j = 0; j < glyphRunLengths[i]; j++) {
					int k = previousChar + j;
					int l = currentChar - j - 1;
					if (k < l) {
						chars[k] ^= chars[l];
						chars[l] ^= chars[k];
						chars[k] ^= chars[l];
					}
				}
			}
			previousChar = currentChar;
		}

		// Create the attributed string and the glyph vector
		glyphs = font.getFont().createGlyphVector(graphics.getFontRenderContext(), chars);
		glyphCount = glyphs.getNumGlyphs();

	}

	/**
	 * Sets the advances for each character.
	 *
	 * @param advances an array of the advances for each character
	 */
	public void setDXArray(int[] advances) {

		bounds = null;
		caretPositions = null;
		charAdvances = null;

		// Set the character advances but only at the start of the run so
		// that we don't mess up any ligatures
		double charAdjust = 0;
		Point2D currentAdvance;
		int currentChar = 0;
		int currentGlyph = 0;
		for (int i = 0; i < charRunLengths.length; i++) {
			for (int j = 0; j < charRunLengths[i] || j < glyphRunLengths[i]; j++) {
				if (j < glyphRunLengths[i]) {
					currentAdvance = glyphs.getGlyphPosition(currentGlyph + 1);
					if (j < charRunLengths[i])
						charAdjust = (double)advances[currentChar] - currentAdvance.getX();
					currentAdvance.setLocation((double)currentAdvance.getX() + charAdjust, currentAdvance.getY());
					glyphs.setGlyphPosition(currentGlyph + 1, currentAdvance);
					currentGlyph++;
				}

				if (j < charRunLengths[i])
					currentChar++;
			}
		}

	}

}
