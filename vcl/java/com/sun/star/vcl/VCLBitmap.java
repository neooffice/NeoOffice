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

import java.awt.Point;
import java.awt.Rectangle;
import java.awt.image.BufferedImage;
import java.awt.image.DataBuffer;
import java.awt.image.IndexColorModel;

/**
 * The Java class that implements the SalFrame C++ class methods.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public final class VCLBitmap {

	/**
	 * The bit count.
	 */
	private int bitCount = 0;

	/**
	 * The data buffer.
	 */
	private byte[] data = null;

	/**
	 * The height.
	 */
	private int height = 0;

	/**
	 * The color model.
	 */
	private IndexColorModel model = null;

	/**
	 * The color palette.
	 */
	private int[] palette = null;

	/**
	 * The scanline.
	 */
	private int scanline = 0;

	/**
	 * The width.
	 */
	private int width = 0;

	/**
	 * Constructs a new <code>VCLBitmap</code> instance.
	 *
	 * @param w the width of the bitmap 
	 * @param h the height of the bitmap
	 * @param b the desired bit count of the bitmap 
	 * @param p the color palette for the bitmap 
	 * @param d the bitmap data
	 */
	public VCLBitmap(int w, int h, int b) {

		// Cache the height and width
		width = w;
		height = h;

		// Create the data buffer
		if (b <= 8) {
			bitCount = 8;
			BufferedImage img = new BufferedImage(1, 1, BufferedImage.TYPE_BYTE_INDEXED);
			model = (IndexColorModel)img.getColorModel();
			palette = new int[model.getMapSize()];
			model.getRGBs(palette);
		}
		else {
			bitCount = 24;
		}

		// Align buffer to 4 bytes
		scanline = (((bitCount * width) + 31) >> 5) << 2;
		data = new byte[scanline * height];

	}

	/**
	 * Copies the pixels from the specified <code>VCLGraphics</code> to the
	 * bitmap with scaling.
	 *
	 * @param g the <code>VCLGraphics</code>
	 * @param srcX the x coordinate of the graphics
	 * @param srcY the y coordinate of the graphics
	 * @param srcWidth the width to be copied
	 * @param srcHeight the height to be copied
	 * @param destX the x coordinate of the bitmap
	 * @param destY the y coordinate of the bitmap
	 */
	public void copyBits(VCLGraphics g, int srcX, int srcY, int srcWidth, int srcHeight, int destX, int destY) {

		VCLImage srcImage = g.getImage();
		Rectangle srcBounds = new Rectangle(srcX, srcY, srcWidth, srcHeight).intersection(new Rectangle(0, 0, srcImage.getWidth(), srcImage.getHeight()));
		if (srcX < 0)
			srcBounds.width += srcX;
		if (srcY < 0)
			srcBounds.height += srcY;
		Rectangle destBounds = new Rectangle(destX, destY, srcWidth, srcHeight).intersection(new Rectangle(0, 0, width, height));
		if (destX < 0)
			destBounds.width += destX;
		if (destY < 0)
			destBounds.height += destY;
		srcWidth = (srcBounds.width > destBounds.width) ? destBounds.width : srcBounds.width;
		srcHeight = (srcBounds.height > destBounds.height) ? destBounds.height : srcBounds.height;
		int[] srcData = srcImage.getData();
		int srcDataWidth = srcImage.getWidth();
		Point srcPoint = new Point(srcBounds.x, srcBounds.y);
		Point destPoint = new Point(destBounds.x, destBounds.y);
		int totalPixels = srcWidth * srcHeight;

		for (int i = 0; i < totalPixels; i++) {
			// Copy pixel
			setPixel(destPoint, srcData[(srcPoint.y * srcDataWidth) + srcPoint.x]);

			// Update current points
			srcPoint.x++;
			if (srcPoint.x >= srcX + srcWidth) {
				srcPoint.x = srcX;
				srcPoint.y++;
			}
			destPoint.x++;
			if (destPoint.x >= destX + srcWidth) {
				destPoint.x = destX;
				destPoint.y++;
			}
		}

	}

	/**
	 * Returns the bit count of the bitmap.
	 *
	 * @return the bit count of the bitmap
	 */
	public int getBitCount() {

		return bitCount;

	}

	/**
	 * Returns the bitmap's data.
	 *
	 * @return the bitmap's data
	 */
	public byte[] getData() {

		return data;

	}

	/**
	 * Returns the height of the bitmap.
	 *
	 * @return the height of the bitmap
	 */
	int getHeight() {

		return height;

	}

	/**
	 * Returns the bitmap's palette.
	 *
	 * @return the palette 
	 */
	public int[] getPalette() {

		return palette;

	}

	/**
	 * Returns the pixel for the specified point.
	 *
	 * @param p the point
	 * @return the pixel
	 */
	int getPixel(Point p) {

		int pixel = 0;
		if (bitCount <= 8) { 
			int i = data[(p.y * scanline) + p.x];
			if (i < 0)
				i += 256;
			pixel = palette[i];
		}
		else if (bitCount <= 24) {
			int i = (p.y * scanline) + (p.x * 3);
			pixel = 0xff000000 | ((data[i++] << 16) & 0x00ff0000) | ((data[i++] << 8) & 0x0000ff00) | (data[i++] & 0x000000ff);
		}
		return pixel;

	}

	/**
	 * Returns the width of the bitmap.
	 *
	 * @return the width of the bitmap
	 */
	int getWidth() {

		return width;

	}

	/**
	 * Sets the bitmap's palette.
	 *
	 * @param p the palette 
	 */
	public void setPalette(int[] p) {

		palette = p;
		if (palette != null) {
			for (int i = 0; i < palette.length; i++)
				palette[i] |= 0xff000000;
			model = new IndexColorModel(8, palette.length, p, 0, false, -1, DataBuffer.TYPE_BYTE);
		}
		else {
			model = null;
		}

	}

	/**
	 * Sets the pixel for the specified point.
	 *
	 * @param p the point 
	 * @param c the pixel
	 */
	void setPixel(Point p, int c) {

		if (bitCount <= 8) {
			data[(p.y * scanline) + p.x] = ((byte[])model.getDataElements(c, null))[0];
		}
		else if (bitCount <= 24) {
			int i = (p.y * scanline) + (p.x * 3);
			data[i++] = (byte)((c & 0x00ff0000) >> 16);
			data[i++] = (byte)((c & 0x0000ff00) >> 8);
			data[i++] = (byte)(c & 0x000000ff);
		}

	}

}
