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

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.Insets;
import java.awt.Panel;
import java.awt.Point;
import java.awt.Polygon;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.Shape;
import java.awt.TexturePaint;
import java.awt.Toolkit;
import java.awt.event.PaintEvent;
import java.awt.font.GlyphVector;
import java.awt.geom.AffineTransform;
import java.awt.geom.Area;
import java.awt.geom.Point2D;
import java.awt.image.BufferedImage;
import java.awt.print.PageFormat;
import java.util.LinkedList;

/**
 * The Java class that implements the SalGraphics C++ class methods.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public final class VCLGraphics {

	/**
	 * The SAL_INVERT_HIGHLIGHT constant.
	 */
	public final static int SAL_INVERT_HIGHLIGHT = 0x0001;

	/**
	 * The SAL_INVERT_50 constant.
	 */
	public final static int SAL_INVERT_50 = 0x0002;

	/**
	 * The SAL_INVERT_TRACKFRAME constant.
	 */
	public final static int SAL_INVERT_TRACKFRAME = 0x0004;

	/**
	 * The auto flush flag.
	 */
	private static boolean autoFlush = true;

	/**
	 * The graphics list.
	 */
	private static LinkedList graphicsList = new LinkedList();

	/**
	 * The image50 image.
	 */
	private static VCLImage image50 = VCLImage.loadImageFromResource("com/sun/star/vcl/50.jpg");

	/**
	 * Emits an audio beep.
	 */
	public static void beep() {

		Toolkit.getDefaultToolkit().beep();

	}

	/**
	 * Flushes all native drawing buffers to their native windows.
	 */
	public static void flushAll() {

		int elements = graphicsList.size();
		for (int i = 0; i < elements; i++) {
			VCLGraphics g = (VCLGraphics)graphicsList.get(i);
			if (g.frame != null && g.update != null)
				g.flush();
		}

	}

	/**
	 * Get the screen resolution.
	 *
	 * @return the screen resolution in dots per inch
	 */
	public static long getScreenResolution() {

		return (long)Toolkit.getDefaultToolkit().getScreenResolution();

	}

	/**
	 * Set the auto flush flag.
	 *
	 * @param b the auto flush flag
	 */
	static void setAutoFlush(boolean b) {

		autoFlush = b;

	}

	/**
	 * Set default rendering attributes.
	 *
	 * @param g the <code>Graphics2D</code> instance
	 */
	static void setDefaultRenderingAttributes(Graphics2D g) {

		// Set rendering hints
		RenderingHints hints = g.getRenderingHints();
		hints.put(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
		hints.put(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		g.setRenderingHints(hints);

		// Set stroke
		g.setStroke(new BasicStroke(1.0f));

	}

	/**
	 * The cached bit count.
	 */
	private int bitCount = 0;

	/**
	 * The frame that the graphics draws to.
	 */
	private VCLFrame frame = null;

	/**
	 * The graphics context.
	 */
	private Graphics2D graphics = null;

	/**
	 * The image that the graphics draws to.
	 */
	private VCLImage image = null;

	/**
	 * The page format.
	 */
	private PageFormat pageFormat = null;

	/**
	 * The panel's graphics context.
	 */
	private Graphics2D panelGraphics = null;

	/**
	 * The cached update area.
	 */
	private Rectangle update = null;

	/**
	 * The cached clipping area.
	 */
	private Area userClip = null;

	/**
	 * The XOR mode.
	 */
	private boolean xor = false;

	/**
	 * Constructs a new <code>VCLGraphics</code> instance.
	 *
	 * @param i the <code>VCLFrame</code> instance
	 */
	VCLGraphics(VCLFrame f) {

		frame = f;
		Rectangle bounds = null;
		if (frame.getWindow().isVisible()) {
			Panel p = frame.getPanel();
			bounds = p.getBounds();
			panelGraphics = (Graphics2D)p.getGraphics();
		}
		else {
			bounds = new Rectangle(0, 0, 1, 1);
			panelGraphics = null;
		}
		image = new VCLImage(bounds.width, bounds.height, frame.getBitCount());
		graphics = image.getImage().createGraphics();
		VCLGraphics.setDefaultRenderingAttributes(graphics);
		bitCount = image.getBitCount();

		synchronized (graphicsList) {
			graphicsList.add(this);
		}

	}

	/**
	 * Constructs a new <code>VCLGraphics</code> instance from an existing
	 * <code>VCLImage</code> instance.
	 *
	 * @param i the <code>VCLImage</code> instance
	 */
	VCLGraphics(VCLImage i) {

		image = i;
		graphics = image.getImage().createGraphics();
		VCLGraphics.setDefaultRenderingAttributes(graphics);
		bitCount = image.getBitCount();

	}

	/**
	 * Constructs a new <code>VCLGraphics</code> instance from an existing
	 * <code>Graphics2D</code> instance.
	 *
	 * @param g the <code>Graphics2D</code> instance
	 * @param p the <code>PageFormat</code> instance
	 */
	VCLGraphics(Graphics2D g, PageFormat p) {

		graphics = g;
		pageFormat = p;

	}

	/**
	 * Unions the specified rectangle to the rectangle that requires flushing.
	 *
	 * @param b the rectangle to flush
	 */
	void addToFlush(Rectangle b) {

		if (frame != null) {
			Toolkit.getDefaultToolkit().sync();
			if (update != null)
				update.add(b);
			else
				update = b;

			if (autoFlush)
				flush();
		}

	}

	/**
	 * Sets the cached clipping area to an empty area. The cached clipping
	 * area is not actually applied until the {@link #endSetClipRegion()}
	 * method is called.
	 */
	public void beginSetClipRegion() {

		userClip = null;

	}

	/**
	 * Disposes the underlying graphics and releases any system resources that
	 * it is using.
	 */
	void dispose() {

		synchronized (graphicsList) {
			graphicsList.remove(this);
		}
		bitCount = 0;
		frame = null;
		if (graphics != null)
			graphics.dispose();
		graphics = null;
		if (panelGraphics != null)
			panelGraphics.dispose();
		panelGraphics = null;
		if (frame != null && image != null)
			image.dispose();
		image = null;
		frame = null;
		update = null;
		userClip = null;

	}

	/*
	 * Draws specified <code>VCLGraphics</code> to the underlying graphics
	 * scaled to the specified specified width and height.
	 *
	 * @param g the graphics to be copied 
	 * @param destX the x coordinate of the graphics to copy to
	 * @param destY the y coordinate of the graphics to copy to
	 * @param destWidth the width of the graphics to copy to
	 * @param destHeight the height of the graphics to copy to
	 * @param srcX the x coordinate of the graphics to be copied
	 * @param srcY the y coordinate of the graphics to be copied 
	 * @param srcWidth the width of the graphics to be copied 
	 * @param srcHeight the height of the graphics to be copied 
	 */
	public void copyBits(VCLGraphics g, int destX, int destY, int destWidth, int destHeight, int srcX, int srcY, int srcWidth, int srcHeight) {

		VCLImage i = g.getImage();
		if (i != null)
			drawImage(i, destX, destY, destWidth, destHeight, srcX, srcY, srcWidth, srcHeight);

	}

	/**
	 * Draws specified <code>VCLBitmap</code> to the underlying graphics scaled
	 * to the specified width and height.
	 *
	 * @param bmp the bitmap to be drawn
	 * @param destX the x coordinate of the graphics to draw to
	 * @param destY the y coordinate of the graphics to draw to
	 * @param destWidth the width of the graphics to draw to
	 * @param destHeight the height of the graphics to draw to
	 * @param srcX the x coordinate of the bitmap to be drawn
	 * @param srcY the y coordinate of the bitmap to be drawn
	 * @param srcWidth the width of the bitmap to be drawn
	 * @param srcHeight the height of the bitmap to be drawn
	 */
	public void drawBitmap(VCLBitmap bmp, int destX, int destY, int destWidth, int destHeight, int srcX, int srcY, int srcWidth, int srcHeight) {

		Shape clip = graphics.getClip();
		if (clip != null && clip.contains((double)destX, (double)destY, (double)destWidth, (double)destHeight))
			clip = null;
		if (image == null) {
			// Convert to an image and scale it
			if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX) {
				// Mac OS X will render only PDF 1.3 (which is not supported by
				// the Mac OS X Preview application) if we draw images that are
				// smaller than the page
				VCLImage bmpImage = new VCLImage((int)pageFormat.getWidth() + 1, (int)pageFormat.getHeight() + 1, bitCount);
				VCLGraphics bmpGraphics = bmpImage.getGraphics();
				bmpGraphics.drawBitmap(bmp, destX, destY, destWidth, destHeight, srcX, srcY, srcWidth, srcHeight);
				bmpGraphics.dispose();
				graphics.drawImage(bmpImage.getImage(), 0, 0, null);
				bmpImage.dispose();
			}
			else {
				VCLImage bmpImage = new VCLImage(bmp, srcX, srcY, srcWidth, srcHeight);
				graphics.drawImage(bmpImage.getImage(), destX, destY, destX + destWidth, destY + destHeight, 0, 0, srcWidth, srcHeight, null);
				bmpImage.dispose();
			}
		}
		else if (clip != null || destWidth != srcWidth || destHeight != srcHeight) {
			// Convert to an image and scale it
			VCLImage bmpImage = new VCLImage(bmp, srcX, srcY, srcWidth, srcHeight);
			graphics.drawImage(bmpImage.getImage(), destX, destY, destX + destWidth, destY + destHeight, 0, 0, srcWidth, srcHeight, null);
			bmpImage.dispose();
		}
		else {
			if (destX < 0) {
				destWidth += destX;
				srcX += destX;
				destX = 0;
			}
			if (destY < 0) {
				destHeight += destY;
				srcY += destY;
				destY = 0;
			}
			int[] destData = image.getData();
			int destDataWidth = image.getWidth();
			Point srcPoint = new Point(srcX, srcY);
			Point destPoint = new Point(destX, destY);
			int totalPixels = destWidth * destHeight;

			// Copy all pixels
			for (int i = 0; i < totalPixels; i++) {
				// Copy pixel
				destData[(destPoint.y * destDataWidth) + destPoint.x] = bmp.getPixel(srcPoint);

				// Update current points
				srcPoint.x++;
				if (srcPoint.x >= srcX + srcWidth) {
					srcPoint.x = srcX;
					srcPoint.y++;
				}
				destPoint.x++;
				if (destPoint.x >= destX + destWidth) {
					destPoint.x = destX;
					destPoint.y++;
				}
			}
		}
		addToFlush(new Rectangle(destX, destY, destWidth, destHeight));

	}

	/**
	 * Draws all of the pixels in the first specified <code>VCLBitmap</code>
	 * where the pixels in the second specified <code>VCLBitmap</code> are
	 * zero to the underlying graphics scaled to the specified width and height.
	 *
	 * @param bmp the bitmap to be drawn
	 * @param transBmp the transparent bitmap to be drawn
	 * @param destX the x coordinate of the graphics to draw to
	 * @param destY the y coordinate of the graphics to draw to
	 * @param destWidth the width of the graphics to draw to
	 * @param destHeight the height of the graphics to draw to
	 * @param srcX the x coordinate of the bitmap to be drawn
	 * @param srcY the y coordinate of the bitmap to be drawn
	 * @param srcWidth the width of the bitmap to be drawn
	 * @param srcHeight the height of the bitmap to be drawn
	 */
	public void drawBitmap(VCLBitmap bmp, VCLBitmap transBmp, int destX, int destY, int destWidth, int destHeight, int srcX, int srcY, int srcWidth, int srcHeight) {

		Shape clip = graphics.getClip();
		if (clip != null && clip.contains((double)destX, (double)destY, (double)destWidth, (double)destHeight))
			clip = null;
		if (image == null) {
			if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX) {
				// Mac OS X will render only PDF 1.3 (which is not supported by
				// the Mac OS X Preview application) if we draw images that are
				// smaller than the page
				VCLImage bmpImage = new VCLImage((int)pageFormat.getWidth() + 1, (int)pageFormat.getHeight() + 1, bitCount);
				VCLGraphics bmpGraphics = bmpImage.getGraphics();
				bmpGraphics.drawBitmap(bmp, destX, destY, destWidth, destHeight, srcX, srcY, srcWidth, srcHeight);
				bmpGraphics.dispose();
				graphics.drawImage(bmpImage.getImage(), 0, 0, null);
				bmpImage.dispose();
			}
		}
		else if (clip != null || destWidth != srcWidth || destHeight != srcHeight) {
			// Draw to a temporary image and scale it
			VCLImage mergedImage = new VCLImage(srcWidth, srcHeight, bmp.getBitCount());
			mergedImage.getGraphics().drawBitmap(bmp, transBmp, 0, 0, srcWidth, srcHeight, srcX, srcY, srcWidth, srcHeight);
			graphics.drawImage(mergedImage.getImage(), destX, destY, destX + destWidth, destY + destHeight, 0, 0, srcWidth, srcHeight, null);
			mergedImage.dispose();
		}
		else {
			if (destX < 0) {
				destWidth += destX;
				srcX += destX;
				destX = 0;
			}
			if (destY < 0) {
				destHeight += destY;
				srcY += destY;
				destY = 0;
			}
			int[] destData = image.getData();
			int destDataWidth = image.getWidth();
			Point srcPoint = new Point(srcX, srcY);
			Point destPoint = new Point(destX, destY);
			int totalPixels = destWidth * destHeight;

			// If the pixel in the second image is black, copy the pixel
			// from the first image. Otherwise mark it as transparent.
			for (int i = 0; i < totalPixels; i++) {
				// Copy pixel
				if (transBmp.getPixel(srcPoint) == 0xff000000)
					destData[(destPoint.y * destDataWidth) + destPoint.x] = bmp.getPixel(srcPoint);

				// Update current points
				srcPoint.x++;
				if (srcPoint.x >= srcX + srcWidth) {
					srcPoint.x = srcX;
					srcPoint.y++;
				}
				destPoint.x++;
				if (destPoint.x >= destX + destWidth) {
					destPoint.x = destX;
					destPoint.y++;
				}
			}
		}
		addToFlush(new Rectangle(destX, destY, destWidth, destHeight));

	}

	/*
	 * Draws specified <code>VCLImage</code> to the underlying graphics
	 * scaled to the specified specified width and height.
	 *
	 * @param image the image to be drawn 
	 * @param destX the x coordinate of the image to draw to
	 * @param destY the y coordinate of the image to draw to
	 * @param destWidth the width of the image to draw to
	 * @param destHeight the height of the image to draw to
	 * @param srcX the x coordinate of the image to be drawn
	 * @param srcY the y coordinate of the image to be drawn 
	 * @param srcWidth the width of the image to be drawn 
	 * @param srcHeight the height of the image to be drawn 
	 */
	void drawImage(VCLImage img, int destX, int destY, int destWidth, int destHeight, int srcX, int srcY, int srcWidth, int srcHeight) {

		graphics.drawImage(img.getImage(), destX, destY, destX + destWidth, destY + destHeight, srcX, srcY, srcX + srcWidth, srcY + srcHeight, null);
		addToFlush(new Rectangle(destX, destY, destWidth, destHeight));

	}

	/*
	 * Draws specified <code>VCLImage</code> in XOR mode to the underlying
	 * graphics scaled to the specified specified width and height.
	 *
	 * @param image the image to be drawn 
	 * @param destX the x coordinate of the image to draw to
	 * @param destY the y coordinate of the image to draw to
	 * @param destWidth the width of the image to draw to
	 * @param destHeight the height of the image to draw to
	 * @param srcX the x coordinate of the image to be drawn
	 * @param srcY the y coordinate of the image to be drawn 
	 * @param srcWidth the width of the image to be drawn 
	 * @param srcHeight the height of the image to be drawn 
	 */
	void drawImageXOR(VCLImage img, int destX, int destY, int destWidth, int destHeight, int srcX, int srcY, int srcWidth, int srcHeight) {

		if (image == null) {
			drawImageXOR(img, destX, destY, destWidth, destHeight, srcX, srcY, srcWidth, srcHeight);
			return;
		}
		else if (destWidth != srcWidth || destHeight != srcHeight) {
			// Scale source image
			VCLImage destImage = new VCLImage(destWidth, destHeight, bitCount);
			destImage.getGraphics().drawImage(img, 0, 0, destWidth, destHeight, srcX, srcY, srcWidth, srcHeight);
			drawImageXOR(destImage, destX, destY, destWidth, destHeight, 0, 0, destWidth, destHeight);
			destImage.dispose();
			return;
		}

		Shape clip = graphics.getClip();
		if (clip != null && clip.contains((double)destX, (double)destY, (double)destWidth, (double)destHeight))
			clip = null;
		if (clip != null) {
			// Draw to a temporary image to handle clipping
			VCLImage destImage = new VCLImage(destWidth, destHeight, bitCount);
			destImage.getGraphics().drawImage(image, 0, 0, destWidth, destHeight, destX, destY, destWidth, destHeight);
			destImage.getGraphics().drawImageXOR(img, destX, destY, destWidth, destHeight, srcX, srcY, srcWidth, srcHeight);
			graphics.drawImage(destImage.getImage(), destX, destY, destX + destWidth, destY + destHeight, 0, 0, destWidth, destHeight, null);
			destImage.dispose();
		}
		else {
			if (destX < 0) {
				destWidth += destX;
				srcX += destX;
				destX = 0;
			}
			if (destY < 0) {
				destHeight += destY;
				srcY += destY;
				destY = 0;
			}
			int[] srcData = img.getData();
			int srcDataWidth = img.getWidth();
			int[] destData = image.getData();
			int destDataWidth = image.getWidth();
			Point srcPoint = new Point(srcX, srcY);
			Point destPoint = new Point(destX, destY);
			int totalPixels = destWidth * destHeight;

			for (int i = 0; i < totalPixels; i++) {
				// XOR pixel
				int j = (srcPoint.y * srcDataWidth) + srcPoint.x;
				int k = (destPoint.y * destDataWidth) + destPoint.x;
				destData[k] = destData[k] ^ 0xff000000 ^ srcData[j] | 0xff000000;

				// Update current points
				srcPoint.x++;
				if (srcPoint.x >= srcX + srcWidth) {
					srcPoint.x = srcX;
					srcPoint.y++;
				}
				destPoint.x++;
				if (destPoint.x >= destX + destWidth) {
					destPoint.x = destX;
					destPoint.y++;
				}
			}
		}
		addToFlush(new Rectangle(destX, destY, destWidth, destHeight));
	}

	/**
	 * Draws a line, using the current color, between the points
	 * <code>(x1, y1)</code> and <code>(x2, y2)</code> in this graphics
	 * context's coordinate system.
	 *
	 * @param x1 the first point's x coordinate
	 * @param y1 the first point's y coordinate
	 * @param x2 the second point's x coordinate
	 * @param y2 the second point's y coordinate
	 * @param color the color of the line
	 */
	public void drawLine(int x1, int y1, int x2, int y2, int color) {

		Rectangle bounds = new Rectangle(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
		if (xor) {
			VCLImage srcImage = new VCLImage(bounds.width, bounds.height, bitCount);
			Graphics2D srcGraphics = srcImage.getImage().createGraphics();
			VCLGraphics.setDefaultRenderingAttributes(srcGraphics);
			srcGraphics.setColor(new Color(color));
			srcGraphics.translate(x1 * -1, y1 * -1);
			srcGraphics.drawLine(x1, y1, x2, y2);
			srcGraphics.dispose();
			drawImageXOR(srcImage, bounds.x, bounds.y, bounds.width, bounds.height, 0, 0, bounds.width, bounds.height);
			srcImage.dispose();
		}
		else {
			graphics.setColor(new Color(color));
			graphics.drawLine(x1, y1, x2, y2);
			addToFlush(bounds);
		}

	}

	/**
	 * Draws the specified color for all of the pixels in the specified
	 * <code>VCLBitmap</code> where the pixels are non-white scaled to the
	 * specified width and height.
	 *
	 * @param bmp the bitmap to be drawn
	 * @param color the color that is used to match which pixels to draw
	 * @param destX the x coordinate of the graphics to draw to
	 * @param destY the y coordinate of the graphics to draw to
	 * @param destWidth the width of the graphics to draw to
	 * @param destHeight the height of the graphics to draw to
	 * @param srcX the x coordinate of the bitmap to be drawn
	 * @param srcY the y coordinate of the bitmap to be drawn
	 * @param srcWidth the width of the bitmap to be drawn
	 * @param srcHeight the height of the bitmap to be drawn
	 */
	public void drawMask(VCLBitmap bmp, int color, int destX, int destY, int destWidth, int destHeight, int srcX, int srcY, int srcWidth, int srcHeight) {

		Shape clip = graphics.getClip();
		if (clip != null && clip.contains((double)destX, (double)destY, (double)destWidth, (double)destHeight))
			clip = null;
		if (image == null || clip != null || destWidth != srcWidth || destHeight != srcHeight) {
			// Draw to a temporary image and scale it
			VCLImage maskImage = new VCLImage(srcWidth, srcHeight, bmp.getBitCount());
			maskImage.getGraphics().drawMask(bmp, color, 0, 0, srcWidth, srcHeight, srcX, srcY, srcWidth, srcHeight);
			graphics.drawImage(maskImage.getImage(), destX, destY, destX + destWidth, destY + destHeight, 0, 0, srcWidth, srcHeight, null);
			maskImage.dispose();
		}
		else {
			if (destX < 0) {
				destWidth += destX;
				srcX += destX;
				destX = 0;
			}
			if (destY < 0) {
				destHeight += destY;
				srcY += destY;
				destY = 0;
			}
			int[] destData = image.getData();
			int destDataWidth = image.getWidth();
			Point srcPoint = new Point(srcX, srcY);
			Point destPoint = new Point(destX, destY);
			int totalPixels = destWidth * destHeight;

			// If the pixel is black, set the pixel to the mask color
			color |= 0xff000000;
			for (int i = 0; i < totalPixels; i++) {
				// Copy pixel
				if (bmp.getPixel(srcPoint) == 0xff000000)
					destData[(destPoint.y * destDataWidth) + destPoint.x] = color;

				// Update current points
				srcPoint.x++;
				if (srcPoint.x >= srcX + srcWidth) {
					srcPoint.x = srcX;
					srcPoint.y++;
				}
				destPoint.x++;
				if (destPoint.x >= destX + destWidth) {
					destPoint.x = destX;
					destPoint.y++;
				}
			}
		}
		addToFlush(new Rectangle(destX, destY, destWidth, destHeight));

	}

	/**
	 * Draws or fills the specified polygon with the specified color.
	 *
	 * @param xpoints the array of x coordinates
	 * @param ypoints the array of y coordinates
	 * @param npoints the total number of points in the polygon
	 * @param color the color of the polygon
	 * @param fill <code>true</code> to fill the polygon and <code>false</code>
	 *  to draw just the outline
	 */
	public void drawPolygon(int npoints, int[] xpoints, int[] ypoints, int color, boolean fill) {

		Polygon polygon = new Polygon(xpoints, ypoints, npoints);
		Rectangle bounds = polygon.getBounds();
		if (fill) {
			if (xor)
				graphics.setXORMode(Color.black);
			graphics.setColor(new Color(color));
			graphics.fillPolygon(polygon);
			if (xor)
				graphics.setPaintMode();
			addToFlush(bounds);
		}
		else {
			if (xor) {
				VCLImage srcImage = new VCLImage(bounds.width, bounds.height, bitCount);
				Graphics2D srcGraphics = srcImage.getImage().createGraphics();
				VCLGraphics.setDefaultRenderingAttributes(srcGraphics);
				srcGraphics.setColor(new Color(color));
				srcGraphics.translate(bounds.x * -1, bounds.y * -1);
				srcGraphics.drawPolygon(polygon);
				srcGraphics.dispose();
				drawImageXOR(srcImage, bounds.x, bounds.y, bounds.width, bounds.height, 0, 0, bounds.width, bounds.height);
				srcImage.dispose();
			}
			else {
				graphics.setColor(new Color(color));
				graphics.drawPolygon(polygon);
				addToFlush(bounds);
			}
		}

	}

	/**
	 * Draws the specified polyline with the specified color.
	 *
	 * @param xpoints the array of x coordinates
	 * @param ypoints the array of y coordinates
	 * @param npoints the total number of points in the polyline
	 * @param color the color of the polyline
	 */
	public void drawPolyline(int npoints, int[] xpoints, int[] ypoints, int color) {

		Rectangle bounds = new Polygon(xpoints, ypoints, npoints).getBounds();
		if (xor) {
			VCLImage srcImage = new VCLImage(bounds.width, bounds.height, bitCount);
			Graphics2D srcGraphics = srcImage.getImage().createGraphics();
			VCLGraphics.setDefaultRenderingAttributes(srcGraphics);
			srcGraphics.setColor(new Color(color));
			srcGraphics.translate(bounds.x * -1, bounds.y * -1);
			srcGraphics.drawPolyline(xpoints, ypoints, npoints);
			srcGraphics.dispose();
			drawImageXOR(srcImage, bounds.x, bounds.y, bounds.width, bounds.height, 0, 0, bounds.width, bounds.height);
			srcImage.dispose();
		}
		else {
			graphics.setColor(new Color(color));
			graphics.drawPolyline(xpoints, ypoints, npoints);
			addToFlush(bounds);
		}

	}

	/**
	 * Draws or fills the specified rectangle with the specified color.
	 *
	 * @param x the x coordinate of the rectangle
	 * @param y the y coordinate of the rectangle
	 * @param width the width of the rectangle
	 * @param height the height of the rectangle
	 * @param color the color of the rectangle
	 * @param fill <code>true</code> to fill the polygon and <code>false</code>
	 *  to draw just the outline
	 */
	public void drawRect(int x, int y, int width, int height, int color, boolean fill) {

		Rectangle bounds = new Rectangle(x, y, width, height);
		if (fill) {
			if (xor)
				graphics.setXORMode(Color.black);
			graphics.setColor(new Color(color));
			graphics.fillRect(x, y, width, height);
			if (xor)
				graphics.setPaintMode();
			addToFlush(bounds);
		}
		else {
			if (xor) {
				VCLImage srcImage = new VCLImage(width, height, bitCount);
				Graphics2D srcGraphics = srcImage.getImage().createGraphics();
				VCLGraphics.setDefaultRenderingAttributes(srcGraphics);
				srcGraphics.setColor(new Color(color));
				srcGraphics.translate(x * -1, y * -1);
				srcGraphics.drawRect(x, y, width - 1, height - 1);
				srcGraphics.dispose();
				drawImageXOR(srcImage, x, y, width, height, 0, 0, width, height);
				srcImage.dispose();
			}
			else {
				graphics.setColor(new Color(color));
				graphics.drawRect(x, y, width - 1, height - 1);
				addToFlush(bounds);
			}
		}
	}

	/**
	 * Draws the text given by the specified characters, using this graphics
	 * context's current font and color. The baseline of the leftmost character
	 * is at position <code>(x, y)</code> in this graphics context's coordinate
	 * system.
	 *
	 * @param x the x coordinate
	 * @param y the y coordinate
	 * @param chars the array of characters to be drawn
	 * @param font the font of the text
	 * @param color the color of the text
	 */
	public void drawText(int x, int y, char[] chars, VCLFont font, int color) {

		drawTextArray(x, y, chars, font, color, null);

	}

	/**
	 * Draws the text given by the specified characters, using this graphics
	 * context's current font and color. The baseline of each character's
	 * leftmost position is at <code>(x + offsets[i], y)</code> in this graphics
	 * context's coordinate system where <code>i</code> is the position of
	 * each character in the specified <code>str</code>.
	 *
	 * @param x the x coordinate
	 * @param y the y coordinate
	 * @param chars the array of characters to be drawn
	 * @param font the font of the text
	 * @param color the color of the text
	 * @param offsets the x coordinate offsets for each character
	 */
	public void drawTextArray(int x, int y, char[] chars, VCLFont font, int color, int[] offsets) {

		Font f = font.getFont();
		graphics.setFont(f);
		graphics.setColor(new Color(color));
		FontMetrics fontMetrics = graphics.getFontMetrics();

		// Set rotation
		Point2D origin = new Point2D.Float((float)x, (float)y);
		short orientation = font.getOrientation();
		AffineTransform transform = null;
		AffineTransform rotateTransform = null;
		if (orientation != 0) {
			transform = graphics.getTransform();
			double radians = Math.toRadians((double)orientation / 10);
			graphics.rotate(radians * -1);
			rotateTransform = AffineTransform.getRotateInstance(radians);
			// Adjust drawing origin so that it is not transformed
			origin = AffineTransform.getRotateInstance(radians).transform(origin, null);
		}

		// Divide the character array by whitespace
		int startChar = 0;
		int currentChar = 0;
		GlyphVector glyphs = f.createGlyphVector(graphics.getFontRenderContext(), chars);
		Point2D p = glyphs.getGlyphPosition(0);
		if (offsets != null) {
			double start = p.getX();
			for (int i = 1; i < chars.length; i++) {
				if (Character.getType(chars[i]) == Character.NON_SPACING_MARK && f.canDisplay(chars[i]))
					p.setLocation(start + offsets[i - 1] - fontMetrics.charWidth(chars[i]), p.getY());
				else
					p.setLocation(start + offsets[i - 1], p.getY());
				glyphs.setGlyphPosition(i, p);
			}
		}
		else {
			double adjust = 0;
			for (int i = 1; i < chars.length; i++) {
				if (Character.getType(chars[i]) == Character.NON_SPACING_MARK && f.canDisplay(chars[i]))
					adjust += fontMetrics.charWidth(chars[i]);
				if (adjust != 0) {
					p = glyphs.getGlyphPosition(i);
					p.setLocation(p.getX() - adjust, p.getY());
					glyphs.setGlyphPosition(i, p);
				}
			}
		}
		graphics.drawGlyphVector(glyphs, (float)origin.getX(), (float)origin.getY());

		// Reverse rotation
		if (transform != null)
			graphics.setTransform(transform);

		// Estimate bounds
		Rectangle bounds = null;
		if (orientation != 0)
			bounds = rotateTransform.createTransformedShape(glyphs.getLogicalBounds()).getBounds();
		else
			bounds = glyphs.getLogicalBounds().getBounds();
		bounds.x += x;
		bounds.y += y;
		addToFlush(bounds);

	}

	/**
	 * Applies the cached clipping area. The cached clipping area is set using
	 * the {@link #beginSetClipRegion()} and the
	 * {@link #unionClipRegion(long, long, long, long)} methods.
	 */
	public void endSetClipRegion() {

		graphics.setClip(userClip);

	}

	/**
	 * Flushes any pixels in the underlying <code>VCLImage</code> to the
	 * underlying <code>VCLFrame</code>.
	 */
	void flush() {

		if (panelGraphics != null && image != null && update != null) {
			panelGraphics.setClip(update);
			panelGraphics.drawImage(image.getImage(), 0, 0, null);
			Toolkit.getDefaultToolkit().sync();
			update = null;
			panelGraphics.setClip(null);
		}

	}

	/**
	 * Returns the bit count of the underlying graphics device.
	 *
	 * @return the bit count of the underlying graphics device
	 */
	public int getBitCount() {

		return bitCount;

	}

	/**
	 * Returns the <code>VCLImage</code>.
	 *
	 * @return the <code>VCLImage</code>
	 */
	VCLImage getImage() {

		return image;

	}

	/**
	 * Returns the pixel color in ARGB format for the specified coordinate.
	 *
	 * @param x the x coordinate of the source rectangle
	 * @param y the y coordinate of the source rectangle
	 * @return the pixel color in ARGB format
	 */
	public int getPixel(int x, int y) {

		if (image == null)
			return 0xff000000;

		int[] pixels = image.getData();
		return pixels[(image.getWidth() * y) + x];

	}

	/**
	 * Inverts the specified rectangle depending on the specified options.
	 *
	 * @param x the x coordinate of the rectangle
	 * @param y the y coordinate of the rectangle
	 * @param width the width of the rectangle
	 * @param height the height of the rectangle
	 * @param options the invert options
	 */
	public void invert(int x, int y, int width, int height, int options) {

		if (image == null)
			return;

		// Don't do anything if x or y is outside of the image's width or
		// height
		int destDataWidth = image.getWidth();
		int destDataHeight = image.getHeight();
		Rectangle bounds = new Rectangle(0, 0, image.getWidth(), image.getHeight()).intersection(new Rectangle(x, y, width, height));
		if (bounds.isEmpty())
			return;

		// Invert the image 
		if ((options & VCLGraphics.SAL_INVERT_TRACKFRAME) == VCLGraphics.SAL_INVERT_TRACKFRAME) {
			int[] destData = image.getData();
			Shape clip = graphics.getClip();
			if (clip == null)
				clip = new Rectangle(0, 0, destDataWidth, destDataHeight);

			// Draw horizontal lines
			Point topPoint = new Point(x, y);
			Point bottomPoint = new Point(topPoint.x, y + height - 1);
			while (topPoint.x < x + width && bottomPoint.x < x + width) {
				if (clip == null || clip.contains((double)topPoint.x, (double)topPoint.y)) {
					int i = (topPoint.y * destDataWidth) + topPoint.x;
					destData[i] = destData[i] ^ 0xff000000 ^ 0xffffffff;
				}
				if (clip == null || clip.contains((double)bottomPoint.x, (double)bottomPoint.y)) {
					int i = (bottomPoint.y * destDataWidth) + bottomPoint.x;
					destData[i] = destData[i] ^ 0xff000000 ^ 0xffffffff;
				}
				// Update current points
				topPoint.x += 2;
				bottomPoint.x += 2;
			}

			// Draw vertical lines
			Point leftPoint = new Point(x, y + 2);
			Point rightPoint = new Point(x + width - 1, leftPoint.y);
			while (leftPoint.y < y + height - 1 && rightPoint.y < y + height - 1) {
				if (clip == null || clip.contains((double)leftPoint.x, (double)leftPoint.y)) {
					int i = (leftPoint.y * destDataWidth) + leftPoint.x;
					destData[i] = destData[i] ^ 0xff000000 ^ 0xffffffff;
				}
				if (clip == null || clip.contains((double)rightPoint.x, (double)rightPoint.y)) {
					int i = (rightPoint.y * destDataWidth) + rightPoint.x;
					destData[i] = destData[i] ^ 0xff000000 ^ 0xffffffff;
				}
				// Update current points
				leftPoint.y += 2;
				rightPoint.y += 2;
			}
		}
		else if ((options & VCLGraphics.SAL_INVERT_50) == VCLGraphics.SAL_INVERT_50) {
			Shape clip = graphics.getClip();
			if (clip != null && clip.contains((double)x, (double)y, (double)width, (double)height))
				clip = null;
			if (clip != null) {
				// Draw to a temporary image to handle clipping
				VCLImage destImage = new VCLImage(width, height, bitCount);
				destImage.getGraphics().drawImage(image, 0, 0, width, height, x, y, width, height);
				destImage.getGraphics().invert(0, 0, width, height, options);
				graphics.drawImage(destImage.getImage(), x, y, x + width, y + height, 0, 0, width, height, null);
				destImage.dispose();
			}
			else {
				VCLImage srcImage = new VCLImage(width, height, VCLGraphics.image50.getBitCount());
				Graphics2D srcGraphics = srcImage.getImage().createGraphics();
				VCLGraphics.setDefaultRenderingAttributes(srcGraphics);
				BufferedImage textureImage = VCLGraphics.image50.getImage();
				srcGraphics.setPaint(new TexturePaint(textureImage, new Rectangle(0, 0, textureImage.getWidth(), textureImage.getHeight()).getBounds2D()));
				srcGraphics.setPaint(new TexturePaint(textureImage, new Rectangle(0, 0, textureImage.getWidth(), textureImage.getHeight()).getBounds2D()));
				srcGraphics.fillRect(0, 0, width, height);
				srcGraphics.dispose();
				drawImageXOR(srcImage, x, y, width, height, 0, 0, width, height);
				srcImage.dispose();
			}
		}
		else {
			if (x < 0) {
				width += x;
				x = 0;
			}
			if (y < 0) {
				height += y;
				y = 0;
			}
			Shape clip = graphics.getClip();
			if (clip != null && clip.contains((double)x, (double)y, (double)width, (double)height))
				clip = null;
			if (clip != null) {
				// Draw to a temporary image to handle clipping
				VCLImage destImage = new VCLImage(width, height, bitCount);
				destImage.getGraphics().drawImage(image, 0, 0, width, height, x, y, width, height);
				destImage.getGraphics().invert(0, 0, width, height, options);
				graphics.drawImage(destImage.getImage(), x, y, x + width, y + height, 0, 0, width, height, null);
				destImage.dispose();
			}
			else {
				int[] destData = image.getData();
				Point destPoint = new Point(x, y);
				int totalPixels = width * height;

				for (int i = 0; i < totalPixels; i++) {
					// Invert pixel
					int j = (destPoint.y * destDataWidth) + destPoint.x;
					destData[j] = ~destData[j] | 0xff000000;
					// Update current points
					destPoint.x++;
					if (destPoint.x >= x + width) {
						destPoint.x = x;
						destPoint.y++;
					}
				}
			}
		}
		addToFlush(new Rectangle(x, y, width, height));

	}

	/**
	 * Resets the applied clipping area.
	 */
	public void resetClipRegion() {

		userClip = null;
		graphics.setClip(userClip);

	}

	/**
	 * Resets the underlying graphics context.
	 */
	synchronized void resetGraphics() {

		if (image == null)
			return;

		if (frame != null) {
			graphics.dispose();
			if (panelGraphics != null)
				panelGraphics.dispose();
			image.dispose();
			Rectangle bounds = null;
			if (frame.getWindow().isVisible()) {
				Panel p = frame.getPanel();
				bounds = p.getBounds();
				panelGraphics = (Graphics2D)p.getGraphics();
			}
			else {
				bounds = new Rectangle(0, 0, 1, 1);
				panelGraphics = null;
			}
			image = new VCLImage(bounds.width, bounds.height, frame.getBitCount());
			graphics = image.getImage().createGraphics();
			VCLGraphics.setDefaultRenderingAttributes(graphics);
			bitCount = image.getBitCount();
			userClip = null;
			update = null;
		}

	}

	/**
	 * Set the antialiasing drawing attributes.
	 *
	 * @param b the antialiasing flag
	 */
	public void setAntialias(boolean b) {

		RenderingHints hints = graphics.getRenderingHints();
		if (b)
			hints.put(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		else
			hints.put(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_OFF);
		graphics.setRenderingHints(hints);

	}

	/**
	 * Set the pixel color for the specified coordinate.
	 *
	 * @param x the x coordinate of the source rectangle
	 * @param y the y coordinate of the source rectangle
	 * @param color the color of the pixel 
	 */
	public void setPixel(int x, int y, int color) {

		if (image != null) {
			int[] pixels = image.getData();
			pixels[(image.getWidth() * y) + x] = 0xff000000 | color;
		}
		else {
			drawRect(x, y, 1, 1, color, true);
		}
		addToFlush(new Rectangle(x, y, 1, 1));

	}

	/**
	 * Enables or disables painting in XOR mode.
	 *
	 * @param b <code>true</code> to enable XOR mode and <code>false</code>
	 *  to disable it
	 */
	public void setXORMode(boolean b) {

		if (image != null)
			xor = b;

	}

	/**
	 * Unions the cached clipping area with the specified rectangle. The
	 * cached clipping area is not actually applied until the
	 * {@link #endSetClipRegion()} method is called.
	 *
	 * @param x the x coordinate of the rectangle to add to clip region
	 * @param y the y coordinate of the rectangle to add to clip region
	 * @param width the width of the rectangle to add to clip region
	 * @param height the height of the rectangle to add to clip region
	 */
	public void unionClipRegion(int x, int y, int width, int height) {

		Area area = new Area(new Rectangle(x, y, width, height));
		if (userClip != null)
			userClip.add(area);
		else
			userClip = area;

	}

}
