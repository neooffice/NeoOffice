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
import java.awt.Image;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.Toolkit;
import java.awt.color.ColorSpace;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.DataBuffer;
import java.awt.image.DataBufferByte;
import java.awt.image.DataBufferInt;
import java.awt.image.DataBufferUShort;
import java.awt.image.DirectColorModel;
import java.awt.image.FilteredImageSource;
import java.awt.image.ImageFilter;
import java.awt.image.IndexColorModel;
import java.awt.image.MultiPixelPackedSampleModel;
import java.awt.image.Raster;
import java.awt.image.SampleModel;
import java.awt.image.SinglePixelPackedSampleModel;
import java.awt.image.WritableRaster;

/**
 * The Java class that implements the SalFrame C++ class methods.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public final class VCLBitmap {

	/**
	 * The default 32 bit color model.
	 */
	private static DirectColorModel default32BitColorModel = null;

	/**
	 * Initialize the default palettes and color models.
	 */
	static {

		default32BitColorModel = new DirectColorModel(ColorSpace.getInstance(ColorSpace.CS_sRGB), 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000, true, DataBuffer.TYPE_INT);

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
		else
			bitCount = 24;

		// Create the image. Note that all rasters are mapped to 32 bit rasters
		// since this is what the JVM will convert all rasters to every time
		// a non-32 bit raster is drawn.
		data = new int[width * height];
		DirectColorModel model = new DirectColorModel(ColorSpace.getInstance(ColorSpace.CS_sRGB), 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000, true, DataBuffer.TYPE_INT);
		WritableRaster raster = Raster.createWritableRaster(model.createCompatibleSampleModel(width, height), new DataBufferInt(data, data.length), null);
        image = new BufferedImage(model, raster, true, null);

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

		BufferedImage img = null;
		if ( graphics.getImage() != null)
			img = graphics.getImage().getImage();

		if (img == null) {
			Rectangle srcBounds = new Rectangle(srcX, srcY, srcWidth, srcHeight).intersection(graphics.getGraphicsBounds());
			if (srcBounds.isEmpty())
				return;

            VCLImage srcImage = graphics.createImage(srcBounds.x, srcBounds.y, srcBounds.width, srcBounds.height);
            if (srcImage == null)
                return;

			srcX -= srcBounds.x;
			srcY -= srcBounds.y;

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
	 * Returns the bitmap's data.
	 *
	 * @return the bitmap's data
	 */
	public int[] getData() {

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
