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

import java.awt.Container;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.MediaTracker;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.Toolkit;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferInt;

/**
 * The Java class that implements the SalFrame C++ class methods.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public class VCLImage {

	/**
	 * Load an image from a resource.
	 *
	 * @param resource the resource name
	 * @return a <code>VCLImage</code> instance
	 */
	static VCLImage loadImageFromResource(String resource) {

		Image i = Toolkit.getDefaultToolkit().createImage(VCLImage.class.getClassLoader().getResource(resource));
		MediaTracker m = new MediaTracker(new Container());
		m.addImage(i, 0);
		try {
			m.waitForID(0);
		}
		catch (InterruptedException t) {
			return null;
		}
		int width = i.getWidth(null);
		int height = i.getWidth(null);
		VCLImage image = new VCLImage(width, height, 0);
		Graphics2D g = image.getImage().createGraphics();
		g.drawImage(i, 0, 0, null);
		g.dispose();
		Toolkit.getDefaultToolkit().sync();
		return image;

	}

	/**
	 * The bit count.
	 */
	private int bitCount = 0;

	/**
	 * The data buffer.
	 */
	private int[] data = null;

	/**
	 * The graphics.
	 */
	private VCLGraphics graphics = null;

	/**
	 * The height.
	 */
	private int height = 0;

	/**
	 * The native image.
	 */
	private BufferedImage image = null;

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

		// Adjust bit count
		if (b <= 8)
			bitCount = 8;
		else
			bitCount = 24;

		// Create the native image
		image = new BufferedImage(w, h, BufferedImage.TYPE_INT_ARGB_PRE);
		width = w;
		height = h;

		// Cache the graphics
		graphics = new VCLGraphics(this);

		// Cache the data buffer
		data = ((DataBufferInt)image.getRaster().getDataBuffer()).getData();

	}

	/**
	 * Constructs a new <code>VCLImage</code> instance from a
	 * <code>VCLBitmap</code>.
	 *
	 * @param bmp the bitmap
	 * @param x the x coordinate of the bitmap
	 * @param y the y coordinate of the bitmap
	 * @param w the width to copy from
	 * @param h the height to copy from
	 */
	VCLImage(VCLBitmap bmp, int x, int y, int w, int h) {

		this(w, h, bmp.getBitCount());

		Point srcPoint = new Point(x, y);
		int totalPixels = w * h;
		for (int i = 0; i < totalPixels; i++) {
			// Copy the pixels from the bitmap
			data[i] = bmp.getPixel(srcPoint);

			// Update curent points
			srcPoint.x++;
			if (srcPoint.x >= x + w) {
				srcPoint.x = x;
				srcPoint.y++;
			}
		}
	}

	/**
 	 * Disposes the image and releases any system resources that it is
	 * using.
	 */
	public void dispose() {

		bitCount = 0;
		if (graphics != null)
			graphics.dispose();
		graphics = null;
		image = null;

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
	 * Returns the underlying image's data.
	 *
	 * @return the underlying image's data
	 */
	int[] getData() {

		return data;

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
