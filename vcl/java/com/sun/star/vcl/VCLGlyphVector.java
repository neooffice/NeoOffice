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

/**
 * The Java class that implements the JavaLayout C++ class methods.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public final class VCLGlyphVector {

	/**
	 * The font.
	 */
	private VCLFont font = null;

	/**
	 * The glyph vector.
	 */
	private GlyphVector glyphs = null;

	/**
	 * The graphics.
	 */
	private VCLGraphics graphics = null;

	/**
	 * Constructs a new <code>VCLGlyphVector</code> instance.
	 *
	 * @param g the <code>VCLGraphics</code> instance
	 * @param f the <code>VCLFont</code> instance
	 */
	public VCLGlyphVector(VCLGraphics g, VCLFont f) {

		graphics = g;
		font = f;
	}

	/**
	 * Layout text.
	 *
	 * @param chars the array of characters to layout
	 */
	public void layoutText(char[] chars) {

		glyphs = font.getFont().createGlyphVector(graphics.getFontRenderContext(), chars);

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
	 * Returns an array of the offsets for each glyph in the glyph vector.
	 *
	 * @return an array of the offsets for each glyph in the glyph vector
	 */
	public int[] fillDXArray() {

		int[] offsets = new int[glyphs.getNumGlyphs()];

		for (int i = 0; i < offsets.length; i++)
			offsets[i] = (int)glyphs.getGlyphPosition(i).getX();

		return offsets;

	}

}
