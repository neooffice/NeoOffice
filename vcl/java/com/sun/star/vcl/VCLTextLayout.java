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
import java.awt.font.TextAttribute;
import java.awt.font.TextHitInfo;
import java.awt.font.TextLayout;
import java.awt.font.TextMeasurer;
import java.text.AttributedString;

/**
 * The Java class that implements the JavaLayout C++ class methods.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public final class VCLTextLayout {

	/**
	 * The font.
	 */
	private VCLFont font = null;

	/**
	 * The graphics.
	 */
	private VCLGraphics graphics = null;

	/**
	 * The text layout.
	 */
	private TextLayout textLayout = null;

	/**
	 * The text length.
	 */
	private int textLength = 0;

	/**
	 * The text measurer.
	 */
	private TextMeasurer textMeasurer = null;

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

		graphics.drawTextArray(textLayout, x, y, orientation, color, font.isAntialiased());

	}

	/**
	 * Returns an array of the offsets for each character.
	 *
	 * @return an array of the offsets for each character
	 */
	public int[] fillDXArray() {

		int[] offsets = new int[textLength];

		int currentOffset = 0;
		int previousOffset = 0;
		for (int i = 0; i < offsets.length; i++) {
			currentOffset = (int)textMeasurer.getAdvanceBetween(0, i + 1);
			offsets[i] = currentOffset - previousOffset;
			previousOffset = currentOffset;
		}

		return offsets;

	}

	/**
	 * Returns an array of the cursor offsets for each character.
	 *
	 * @param n number of characters to return
	 * @return an array of the cursor offsets for each character
	 */
	public int[] getCaretPositions(int n) {

		int[] offsets = new int[n];

		int currentChar = 0;
		for (int i = 0; i < offsets.length; i += 2) {
			offsets[i] = textLayout.getCaretShape(TextHitInfo.leading(currentChar)).getBounds().x;
			offsets[i + 1] = textLayout.getCaretShape(TextHitInfo.trailing(currentChar)).getBounds().x;
			currentChar++;
		}

		return offsets;

	}

	/**
	 * Layout text.
	 *
	 * @param s the string to layout
	 */
	public void layoutText(String s) {

		textLength = s.length();

		// Create the attributed string
		AttributedString as = new AttributedString(s);
		as.addAttribute(TextAttribute.FONT, font.getFont());

		textMeasurer = new TextMeasurer(as.getIterator(), graphics.getFontRenderContext());
		textLayout = textMeasurer.getLayout(0, textLength);

	}

}
