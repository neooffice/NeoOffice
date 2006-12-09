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

import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.DataBufferInt;
import java.awt.image.WritableRaster;

/**
 * The Java class that implements the SalFrame C++ class methods.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public final class VCLImage {

	/**
	 * The bit count.
	 */
	private int bitCount = 0;

	/**
	 * The graphics.
	 */
	private VCLGraphics graphics = null;

	/**
	 * The height.
	 */
	private int height = 0;

	/**
	 * The image.
	 */
	private BufferedImage image = null;

	/**
	 * The <code>VCLPageFormat</code>.
	 */
	private VCLPageFormat pageFormat = null;

	/**
	 * The width.
	 */
	private int width = 0;

	/**
	 * Constructs a new <code>VCLImage</code> instance.
	 *
	 * @param w the width of the image
	 * @param h the height of the image
	 * @param b the desired bit count of the image
	 */
	public VCLImage(int w, int h, int b) {

		this(w, h, b, null);

	}

	/**
	 * Constructs a new <code>VCLImage</code> instance.
	 *
	 * @param w the width of the image
	 * @param h the height of the image
	 * @param b the desired bit count of the image
	 * @param p the <code>VCLPageFormat</code> instance.
	 */
	VCLImage(int w, int h, int b, VCLPageFormat p) {

		width = w;
		height = h;

		// Always set bit count to 32
		bitCount = 32;

		// Create the image
		try {
			image = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB_PRE);
		}
		catch (OutOfMemoryError ome) {
			System.gc();
			image = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB_PRE);
		}

		// Cache the graphics
		pageFormat = p;
		graphics = new VCLGraphics(this, pageFormat);

	}

	/**
	 * Constructs a new <code>VCLImage</code> instance.
	 *
	 * @param r the writable raster
	 */
	VCLImage(WritableRaster r) {

		width = r.getWidth();
		height = r.getHeight();

		// Always set bit count to 32
		bitCount = 32;

		// Create the image
		image = new BufferedImage(ColorModel.getRGBdefault(), r, true, null);

		// Cache the graphics
		pageFormat = null;
		graphics = new VCLGraphics(this, pageFormat);

	}

	/**
 	 * Disposes the image and releases any system resources that it is
	 * using.
	 */
	public void dispose() {

		if (graphics != null)
			graphics.dispose();
		graphics = null;
		image = null;
		pageFormat = null;

	}

	/**
	 * Returns the bit count of the underlying image.
	 *
	 * @return the bit count of the underlying image
	 */
	int getBitCount() {

		return bitCount;

	}

	/**
	 * Returns the image's data. Warning: calling this method will cause all
	 * succeeding drawing operations to this image to be an order of magnitude
	 * slower on Mac OS X 10.4 due to the way the JVM does graphics
	 * acceleration.
	 *
	 * @return the image's data
	 */
	public int[] getData() {

		return ((DataBufferInt)image.getRaster().getDataBuffer()).getData();

	}

	/**
	 * Creates a graphics context for this component.
	 *
	 * @return a graphics context for this component
	 */
	public VCLGraphics getGraphics() {

		return graphics;

	}

	/**
	 * Returns the height of the underlying image.
	 * 
	 * @return the height of the underlying image.
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
	 * Returns the width of the underlying image.
	 * 
	 * @return the width of the underlying image
	 */
	int getWidth() {

		return width;

	}

}
