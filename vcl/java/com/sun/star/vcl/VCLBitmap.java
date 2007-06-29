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
 *  Copyright 2003 Planamesa Inc.
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
		bitCount = b;
		width = w;
		height = h;

		VCLEventQueue.runGCIfNeeded(0);

		// Create the image. Note that all rasters are mapped to 32 bit rasters
		// since this is what the JVM will convert all rasters to every time
		// a non-32 bit raster is drawn.
		try {
			image = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB_PRE);
		}
		catch (OutOfMemoryError ome) {
			// Force the garbage collector to run
			VCLEventQueue.runGCIfNeeded(VCLEventQueue.GC_DISPOSED_PIXELS);
			image = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB_PRE);
		}

	}

	/**
	 * Disposes the image and releases any system resources that it is
	 * using.
	 */
	public void dispose() {

		image = null;
		VCLEventQueue.runGCIfNeeded(0);

	}

	/**
	 * Returns the bit count of the bitmap.
	 *
	 * @return the bit count of the bitmap
	 */
	int getBitCount() {

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
