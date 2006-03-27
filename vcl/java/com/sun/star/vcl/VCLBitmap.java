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

import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferInt;

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
	 * The height.
	 */
	private int height = 0;

	/**
	 * The image.
	 */
	private BufferedImage image = null;

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

		if (b <= 1)
			bitCount = 1;
		else if (b <= 4)
			bitCount = 4;
		else if (b <= 8)
			bitCount = 8;
		else if (b <= 16)
			bitCount = 16;
		else if (b <= 24)
			bitCount = 24;
		else
			bitCount = 32;

		// Create the image. Note that all rasters are mapped to 32 bit rasters
		// since this is what the JVM will convert all rasters to every time
		// a non-32 bit raster is drawn.
		image = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB_PRE);

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
	public void copyBits(VCLGraphics graphics, int srcX, int srcY, int srcWidth, int srcHeight, int destX, int destY) {

		Rectangle srcBounds = new Rectangle(srcX, srcY, srcWidth, srcHeight).intersection(graphics.getGraphicsBounds());
		if (srcBounds.isEmpty())
			return;

		if (!srcBounds.contains(srcX, srcY, srcWidth, srcHeight)) {
			destX = srcBounds.x - srcX;
			destY = srcBounds.y - srcY;
			srcX = srcBounds.x;
			srcY = srcBounds.y;
			srcWidth = srcBounds.width;
			srcY = srcBounds.height;
		}

		BufferedImage img = null;
		if ( graphics.getImage() != null)
			img = graphics.getImage().getImage();

		if (img == null) {
            VCLImage srcImage = graphics.createImage(srcX, srcY, srcWidth, srcHeight);
            if (srcImage == null)
                return;

			srcX = 0;
			srcY = 0;

			img = srcImage.getImage();
			srcImage.dispose();
		}

		Graphics2D g = image.createGraphics();
		try {
			g.drawImage(img, destX, destY, destX + srcWidth, destY + srcHeight, srcX, srcY, srcX + srcWidth, srcY + srcHeight, null);
		}
		catch (Throwable t) {
			t.printStackTrace();
		}
		g.dispose();

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
	 * Returns the bitmap's data. Warning: calling this method will cause all
	 * succeeding drawing operations to this image to be an order of magnitude
	 * slower on Mac OS X 10.4 due to the way the JVM does graphics
	 * acceleration.
	 *
	 * @return the bitmap's data
	 */
	public int[] getData() {

		return ((DataBufferInt)image.getRaster().getDataBuffer()).getData();

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
	 * Returns the underlying image.
	 *
	 * @return the underlying image
	 */
	BufferedImage getImage() {

		return image;

	}

	/**
	 * Returns the width of the bitmap.
	 *
	 * @return the width of the bitmap
	 */
	int getWidth() {

		return width;

	}

}
