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
import java.awt.Transparency;
import java.awt.color.ColorSpace;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.ComponentColorModel;
import java.awt.image.DataBuffer;
import java.awt.image.DataBufferByte;
import java.awt.image.FilteredImageSource;
import java.awt.image.ImageFilter;
import java.awt.image.IndexColorModel;
import java.awt.image.MultiPixelPackedSampleModel;
import java.awt.image.PixelInterleavedSampleModel;
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
	 * The default 1 bit palette.
	 */
	private static int[] default1BitPalette = null;

	/**
	 * The default 1 bit color model.
	 */
	private static IndexColorModel default1BitColorModel = null;

	/**
	 * The default 4 bit palette.
	 */
	private static int[] default4BitPalette = null;

	/**
	 * The default 4 bit color model.
	 */
	private static IndexColorModel default4BitColorModel = null;

	/**
	 * The default 8 bit palette.
	 */
	private static int[] default8BitPalette = null;

	/**
	 * The default 8 bit color model.
	 */
	private static IndexColorModel default8BitColorModel = null;

	/**
	 * The default 24 bit color model.
	 */
	private static ComponentColorModel default24BitColorModel = null;

	/**
	 * Initialize the default palettes and color models.
	 */
	static {

		default1BitPalette = new int[2];
		default1BitPalette[0] = 0xff000000;
		default1BitPalette[1] = 0xffffffff;
		default1BitColorModel = new IndexColorModel(1, default1BitPalette.length, default1BitPalette, 0, false, -1, DataBuffer.TYPE_BYTE);

		default4BitPalette = new int[16];
		for (int i = 0; i < default4BitPalette.length; i++) {
			int j = i * 17;
			default4BitPalette[i] = 0xff000000 | ((j << 16) & 0x00ff0000) | ((j << 8) & 0x0000ff00) | (j & 0x000000ff);
		}
		default4BitColorModel = new IndexColorModel(4, default4BitPalette.length, default4BitPalette, 0, false, -1, DataBuffer.TYPE_BYTE);

		BufferedImage img = new BufferedImage(1, 1, BufferedImage.TYPE_BYTE_INDEXED);
		default8BitColorModel = (IndexColorModel)img.getColorModel();
		default8BitPalette = new int[default8BitColorModel.getMapSize()];
		default8BitColorModel.getRGBs(default8BitPalette);

		default24BitColorModel = new ComponentColorModel(ColorSpace.getInstance(ColorSpace.CS_sRGB), new int[]{ 8, 8, 8 }, false, true, Transparency.OPAQUE, DataBuffer.TYPE_BYTE);
	}

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
	 * The image.
	 */
	private BufferedImage image = null;

	/**
	 * The filtered image.
	 */
	private Image filteredImage = null;

	/**
	 * The color model.
	 */
	private ColorModel model = null;

	/**
	 * The color palette.
	 */
	private int[] palette = null;

	/**
	 * The writable raster.
	 */
	private WritableRaster raster = null;

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

		// Adjust the bit count
		if (b <= 1)
			bitCount = 1;
		else if (b <= 4)
			bitCount = 4;
		else if (b <= 8)
			bitCount = 8;
		else
			bitCount = 24;

		// Create the color model
		if (bitCount == 1) {
			palette = VCLBitmap.default1BitPalette;
			model = VCLBitmap.default1BitColorModel;
		}
		else if (bitCount == 4) {
			palette = VCLBitmap.default4BitPalette;
			model = VCLBitmap.default4BitColorModel;
		}
		else if (bitCount == 8) {
			palette = VCLBitmap.default8BitPalette;
			model = VCLBitmap.default8BitColorModel;
		}
		else {
			model = default24BitColorModel;
		}

		// Align buffer size to 4 bytes and create the sample model
		scanline = (((bitCount * width) + 31) >> 5) << 2;
		SampleModel sampleModel = null;
		if (bitCount < 8)
			sampleModel = new MultiPixelPackedSampleModel(DataBuffer.TYPE_BYTE, width, height, bitCount, scanline, 0);
		else if (bitCount == 8)
			sampleModel = new SinglePixelPackedSampleModel(DataBuffer.TYPE_BYTE, width, height, scanline, new int[]{ 0xff });
		else
			sampleModel = new PixelInterleavedSampleModel(DataBuffer.TYPE_BYTE, width, height, bitCount / 8, scanline, new int[]{ 0, 1, 2 });

		// Create the raster
		data = new byte[scanline * height];
		raster = Raster.createWritableRaster(sampleModel, new DataBufferByte(data, data.length), null);
		image = new BufferedImage(model, raster, true, null);

		// Fix bug 639 by wrapping the buffered image in a filter
		filteredImage = Toolkit.getDefaultToolkit().createImage(new FilteredImageSource(image.getSource(), new ImageFilter()));

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

		VCLImage srcImage = graphics.getImage();
		if (srcImage == null)
			return;

		Graphics2D g = image.createGraphics();
		g.drawImage(srcImage.getImage(), destX, destY, destX + srcWidth, destY + srcHeight, srcX, srcY, srcX + srcWidth, srcY + srcHeight, null);
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
	 * Returns the underlying image.
	 *
	 * @return the underlying image
	 */
	Image getImage() {

		return filteredImage;

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
	 * @param x the x coordinate
	 * @param y the y coordinate
	 * @return the pixel
	 */
	int getPixel(int x, int y) {

		return image.getRGB(x, y);

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
			model = new IndexColorModel(bitCount, palette.length, palette, 0, false, -1, DataBuffer.TYPE_BYTE);
			image = new BufferedImage(model, raster, true, null);
			filteredImage = Toolkit.getDefaultToolkit().createImage(new FilteredImageSource(image.getSource(), new ImageFilter()));
		}

	}

	/**
	 * Sets the pixel for the specified point.
	 *
	 * @param x the x coordinate
	 * @param y the y coordinate
	 * @param c the pixel
	 */
	void setPixel(int x, int y, int c) {

		image.setRGB(x, y, c);

	}

}
