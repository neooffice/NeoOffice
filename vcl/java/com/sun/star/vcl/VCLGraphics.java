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
import java.awt.Dimension;
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
import java.lang.reflect.Method;
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
	 * The drawLine method.
	 */
	private static Method drawLineMethod = null;

	/**
	 * The drawRect method.
	 */
	private static Method drawRectMethod = null;

	/**
	 * The drawPolygon method.
	 */
	private static Method drawPolygonMethod = null;

	/**
	 * The drawPolyline method.
	 */
	private static Method drawPolylineMethod = null;

	/**
	 * The drawPolyPolygon method.
	 */
	private static Method drawPolyPolygonMethod = null;

	/**
	 * The drawTextArray method.
	 */
	private static Method drawTextArrayMethod = null;

	/**
	 * The graphics list.
	 */
	private static LinkedList graphicsList = new LinkedList();

	/**
	 * The image50 image.
	 */
	private static VCLImage image50 = null;

	/**
	 * The cached screen resolution.
	 */
	private static int screenResolution = 0;

	/**
	 * The cached screen font resolution.
	 */
	private static int screenFontResolution = 0;

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
		hints.put(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		g.setRenderingHints(hints);

		// Set stroke
		g.setStroke(new BasicStroke(1.0f));

	}

	/**
	 * Initialize static data.
	 */
	static {

		// Create the image50 image
		VCLImage textureImage = new VCLImage(2, 2, 1);
		int[] textureData = textureImage.getData();
		textureData[0] = 0xff000000;
		textureData[1] = 0xffffffff;
		textureData[2] = 0xffffffff;
		textureData[3] = 0xff000000;
		image50 = textureImage;

		// Set the screen and font resolutions
		screenResolution = screenFontResolution = Toolkit.getDefaultToolkit().getScreenResolution();
		if (screenResolution < 96)
			screenResolution = 96;

		// Set the method references
		try {
			drawLineMethod = VCLGraphics.class.getMethod("drawLine", new Class[]{ int.class, int.class, int.class, int.class, int.class });
		}
		catch (Throwable t) {}
		try {
			drawRectMethod = VCLGraphics.class.getMethod("drawRect", new Class[]{ int.class, int.class, int.class, int.class, int.class, boolean.class });
		}
		catch (Throwable t) {}
		try {
			drawPolygonMethod = VCLGraphics.class.getMethod("drawPolygon", new Class[]{ int.class, int[].class, int[].class, int.class, boolean.class });
		}
		catch (Throwable t) {}
		try {
			drawPolylineMethod = VCLGraphics.class.getMethod("drawPolyline", new Class[]{ int.class, int[].class, int[].class, int.class });
		}
		catch (Throwable t) {}
		try {
			drawPolyPolygonMethod = VCLGraphics.class.getMethod("drawPolyPolygon", new Class[]{ int.class, int[].class, int[][].class, int[][].class, int.class, boolean.class });
		}
		catch (Throwable t) {}
		try {
			drawTextArrayMethod = VCLGraphics.class.getMethod("drawTextArray", new Class[]{ int.class, int.class, char[].class, VCLFont.class, int.class, int[].class });
		}
		catch (Throwable t) {}

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
	 * The cached bounds of the graphics context.
	 */
	private Rectangle graphicsBounds = null;

	/**
	 * The image that the graphics draws to.
	 */
	private VCLImage image = null;

	/**
	 * The printer page format.
	 */
	private VCLPageFormat pageFormat = null;

	/**
	 * The printer drawing queue.
	 */
	private VCLGraphics.PageQueue pageQueue = null;

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
		if (frame.getWindow().isShowing()) {
			Panel p = frame.getPanel();
			panelGraphics = (Graphics2D)p.getGraphics();
			Rectangle bounds = p.getBounds();
			graphicsBounds = new Rectangle(0, 0, bounds.width, bounds.height);
		}
		else {
			panelGraphics = null;
			graphicsBounds = new Rectangle(0, 0, 1, 1);
		}
		image = new VCLImage(graphicsBounds.width, graphicsBounds.height, frame.getBitCount());
		graphics = image.getImage().createGraphics();
		VCLGraphics.setDefaultRenderingAttributes(graphics);
		bitCount = image.getBitCount();
		resetClipRegion();

		synchronized (graphicsList) {
			graphicsList.add(this);
		}

	}

	/**
	 * Constructs a new <code>VCLGraphics</code> instance from existing
	 * <code>VCLImage</code> and <code>VCLPageFormat,/code> instances.
	 *
	 * @param i the <code>VCLImage</code> instance
	 * @param p the <code>VCLPageFormat</code> instance
	 */
	VCLGraphics(VCLImage i, VCLPageFormat p) {

		image = i;
		graphics = image.getImage().createGraphics();
		graphicsBounds = new Rectangle(0, 0, image.getWidth(), image.getHeight());
		pageFormat = p;
		VCLGraphics.setDefaultRenderingAttributes(graphics);
		bitCount = image.getBitCount();
		resetClipRegion();

	}

	/**
	 * Constructs a new <code>VCLGraphics</code> instance from an existing
	 * <code>Graphics2D</code> instance.
	 *
	 * @param g the <code>Graphics2D</code> instance
	 * @param p the <code>VCLPageFormat</code> instance
	 */
	VCLGraphics(Graphics2D g, VCLPageFormat p) {

		pageFormat = p;
		graphicsBounds = new Rectangle(pageFormat.getImageableBounds());
		graphicsBounds.x = 0;
		graphicsBounds.y = 0;
		graphics = (Graphics2D)g.create(graphicsBounds.x, graphicsBounds.y, graphicsBounds.width, graphicsBounds.height);;
		bitCount = graphics.getDeviceConfiguration().getColorModel().getPixelSize();
		resetClipRegion();

		// Mac OS X sometimes mangles images when multiple images are rendered
		// to a printer so we need to combine all images into one image and
		// defer other drawing operations until after the combined image is
		// created
		if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX)
			pageQueue = new VCLGraphics.PageQueue(this);

	}

	/**
	 * Unions the specified rectangle to the rectangle that requires flushing.
	 *
	 * @param b the rectangle to flush
	 */
	void addToFlush(Rectangle b) {

		if (frame != null) {
			if (update != null)
				update.add(b);
			else
				update = b;
			if (update.isEmpty())
				update = null;

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

		if (pageQueue != null)
			pageQueue.dispose();
		pageQueue = null;
		synchronized (graphicsList) {
			graphicsList.remove(this);
		}
		bitCount = 0;
		if (graphics != null)
			graphics.dispose();
		graphics = null;
		if (panelGraphics != null)
			panelGraphics.dispose();
		panelGraphics = null;
		if (image != null && frame != null)
			image.dispose();
		image = null;
		frame = null;
		pageFormat = null;
		update = null;
		userClip = null;

	}

	/**
	 * Draws specified <code>VCLGraphics</code> to the underlying graphics.
	 *
	 * @param g the graphics to be copied 
	 * @param srcX the x coordinate of the graphics to be copied
	 * @param srcY the y coordinate of the graphics to be copied 
	 * @param srcWidth the width of the graphics to be copied
	 * @param srcHeight the height of the graphics to be copied
	 * @param destY the x coordinate of the graphics to copy to
	 * @param destY the y coordinate of the graphics to copy to
	 * @param destWidth the width of the graphics to copy to
	 * @param destHeight the height of the graphics to copy to
	 */
	public void copyBits(VCLGraphics g, int srcX, int srcY, int srcWidth, int srcHeight, int destX, int destY, int destWidth, int destHeight) {

		VCLImage i = g.getImage();
		if (i != null) {
			if (xor)
				drawImageXOR(i, srcX, srcY, srcWidth, srcHeight, destX, destY, destWidth, destHeight);
			else
				drawImage(i.getImage(), srcX, srcY, srcWidth, srcHeight, destX, destY, destWidth, destHeight);
		}

	}

	/**
	 * Draws specified <code>VCLBitmap</code> to the underlying graphics.
	 *
	 * @param bmp the bitmap to be drawn
	 * @param srcX the x coordinate of the bitmap to be drawn
	 * @param srcY the y coordinate of the bitmap to be drawn
	 * @param srcWidth the width of the bitmap to be drawn
	 * @param srcHeight the height of the bitmap to be drawn
	 * @param destX the x coordinate of the graphics to draw to
	 * @param destY the y coordinate of the graphics to draw to
	 * @param destWidth the width of the graphics to copy to
	 * @param destHeight the height of the graphics to copy to
	 */
	public void drawBitmap(VCLBitmap bmp, int srcX, int srcY, int srcWidth, int srcHeight, int destX, int destY, int destWidth, int destHeight) {

		Rectangle destBounds = new Rectangle(destX, destY, destWidth, destHeight).intersection(graphicsBounds);
		if (destBounds.isEmpty())
			return;
		Shape clip = graphics.getClip();
		if (clip != null && !clip.intersects(destBounds))
			return;
		drawImage(bmp.getImage(), srcX, srcY, srcWidth, srcHeight, destX, destY, destWidth, destHeight);
		addToFlush(destBounds);

	}

	/**
	 * Draws all of the pixels in the first specified <code>VCLBitmap</code>
	 * where the pixels in the second specified <code>VCLBitmap</code> are
	 * zero to the underlying graphics.
	 *
	 * @param bmp the bitmap to be drawn
	 * @param transBmp the transparent bitmap to be drawn
	 * @param srcX the x coordinate of the bitmap to be drawn
	 * @param srcY the y coordinate of the bitmap to be drawn
	 * @param srcWidth the width of the bitmap to be drawn
	 * @param srcHeight the height of the bitmap to be drawn
	 * @param destX the x coordinate of the graphics to draw to
	 * @param destY the y coordinate of the graphics to draw to
	 * @param destWidth the width of the graphics to copy to
	 * @param destHeight the height of the graphics to copy to
	 */
	public void drawBitmap(VCLBitmap bmp, VCLBitmap transBmp, int srcX, int srcY, int srcWidth, int srcHeight, int destX, int destY, int destWidth, int destHeight) {

		Rectangle destBounds = new Rectangle(destX, destY, destWidth, destHeight).intersection(graphicsBounds);
		if (destBounds.isEmpty())
			return;
		Shape clip = graphics.getClip();
		if (clip != null) {
			if (!clip.intersects(destBounds))
				return;
			else if (clip.contains((double)destBounds.x, (double)destBounds.y, (double)destBounds.width, (double)destBounds.height))
				clip = null;
		}
		if (image == null || clip != null || srcWidth != destWidth || srcHeight != destHeight) {
			// Draw to a temporary image
			VCLImage mergedImage = new VCLImage(srcWidth, srcHeight, bmp.getBitCount());
			mergedImage.getGraphics().drawBitmap(bmp, transBmp, srcX, srcY, srcWidth, srcHeight, 0, 0, srcWidth, srcHeight);
			drawImage(mergedImage.getImage(), 0, 0, srcWidth, srcHeight, destX, destY, destWidth, destHeight);
			mergedImage.dispose();
		}
		else {
			Rectangle srcBounds = new Rectangle(srcX + destBounds.x - destX, srcY + destBounds.y - destY, destBounds.width, destBounds.height);
			int[] destData = image.getData();
			int destDataWidth = image.getWidth();
			Point srcPoint = new Point(srcBounds.x, srcBounds.y);
			Point destPoint = new Point(destBounds.x, destBounds.y);
			int totalPixels = destBounds.width * destBounds.height;

			// If the pixel in the second image is black, copy the pixel
			// from the first image. Otherwise mark it as transparent.
			for (int i = 0; i < totalPixels; i++) {
				// Copy pixel
				if (transBmp.getPixel(srcPoint) == 0xff000000)
					destData[(destPoint.y * destDataWidth) + destPoint.x] = bmp.getPixel(srcPoint) | 0xff000000;

				// Update current points
				srcPoint.x++;
				if (srcPoint.x >= srcBounds.x + destBounds.width) {
					srcPoint.x = srcBounds.x;
					srcPoint.y++;
				}
				destPoint.x++;
				if (destPoint.x >= destBounds.x + destBounds.width) {
					destPoint.x = destBounds.x;
					destPoint.y++;
				}
			}
			addToFlush(destBounds);
		}

	}

	/**
	 * Draws specified <code>VCLImage</code> to the underlying graphics.
	 *
	 * @param image the image to be drawn 
	 * @param srcX the x coordinate of the image to be drawn
	 * @param srcY the y coordinate of the image to be drawn 
	 * @param srcWidth the width of the image to be drawn
	 * @param srcHeight the height of the image to be drawn
	 * @param destX the x coordinate of the image to draw to
	 * @param destY the y coordinate of the image to draw to
	 * @param destWidth the width of the graphics to copy to
	 * @param destHeight the height of the graphics to copy to
	 */
	void drawImage(Image img, int srcX, int srcY, int srcWidth, int srcHeight, int destX, int destY, int destWidth, int destHeight) {

		Rectangle destBounds = new Rectangle(destX, destY, destWidth, destHeight).intersection(graphicsBounds);
		if (destBounds.isEmpty())
			return;
		Shape clip = graphics.getClip();
		if (clip != null && !clip.intersects(destBounds))
			return;
		if (pageQueue != null) {
			if (pageQueue.pageImage == null)
				pageQueue.pageImage = new VCLImage(graphicsBounds.width, graphicsBounds.height, bitCount);
			Graphics2D pageGraphics = pageQueue.pageImage.getImage().createGraphics();
			VCLGraphics.setDefaultRenderingAttributes(pageGraphics);
			pageGraphics.setClip(clip);
			pageGraphics.drawImage(img, destX, destY, destX + destWidth, destY + destHeight, srcX, srcY, srcX + srcWidth, srcY + srcHeight, null);
			pageGraphics.dispose();
			pageQueue.updateImageClip(destBounds);
		}
		else {
			graphics.drawImage(img, destX, destY, destX + destWidth, destY + destHeight, srcX, srcY, srcX + srcWidth, srcY + srcHeight, null);
		}
		addToFlush(destBounds);

	}

	/**
	 * Draws specified <code>VCLImage</code> in XOR mode to the underlying
	 * graphics.
	 *
	 * @param image the image to be drawn 
	 * @param srcX the x coordinate of the image to be drawn
	 * @param srcY the y coordinate of the image to be drawn 
	 * @param srcWidth the width of the image to be drawn
	 * @param srcHeight the height of the image to be drawn
	 * @param destX the x coordinate of the image to draw to
	 * @param destY the y coordinate of the image to draw to
	 * @param destWidth the width of the graphics to copy to
	 * @param destHeight the height of the graphics to copy to
	 */
	void drawImageXOR(VCLImage img, int srcX, int srcY, int srcWidth, int srcHeight, int destX, int destY, int destWidth, int destHeight) {

		if (image == null) {
			drawImage(img.getImage(), srcX, srcY, srcWidth, srcHeight, destX, destY, destWidth, destHeight);
			return;
		}

		Rectangle destBounds = new Rectangle(destX, destY, destWidth, destHeight).intersection(graphicsBounds);
		if (destBounds.isEmpty())
			return;
		Shape clip = graphics.getClip();
		if (clip != null) {
			if (!clip.intersects(destBounds))
				return;
			else if (clip.contains((double)destBounds.x, (double)destBounds.y, (double)destBounds.width, (double)destBounds.height))
				clip = null;
		}
		if (clip != null || srcWidth != destWidth || srcHeight != destHeight) {
			// Draw to a temporary image
			VCLImage destImage = new VCLImage(srcWidth, srcHeight, img.getBitCount());
			destImage.getGraphics().drawImage(image.getImage(), destX, destY, destWidth, destHeight, 0, 0, srcWidth, srcHeight);
			destImage.getGraphics().drawImageXOR(img, srcX, srcY, srcWidth, srcHeight, 0, 0, srcWidth, srcHeight);
			drawImage(destImage.getImage(), 0, 0, srcWidth, srcHeight, destX, destY, destWidth, destHeight);
			destImage.dispose();
		}
		else {
			Rectangle srcBounds = new Rectangle(srcX + destBounds.x - destX, srcY + destBounds.y - destY, destBounds.width, destBounds.height);
			int[] srcData = img.getData();
			int srcDataWidth = img.getWidth();
			int[] destData = image.getData();
			int destDataWidth = image.getWidth();
			Point srcPoint = new Point(srcBounds.x, srcBounds.y);
			Point destPoint = new Point(destBounds.x, destBounds.y);
			int totalPixels = destBounds.width * destBounds.height;

			for (int i = 0; i < totalPixels; i++) {
				// XOR pixel
				int j = (srcPoint.y * srcDataWidth) + srcPoint.x;
				int k = (destPoint.y * destDataWidth) + destPoint.x;
				destData[k] = destData[k] ^ 0xff000000 ^ srcData[j] | 0xff000000;

				// Update current points
				srcPoint.x++;
				if (srcPoint.x >= srcBounds.x + destBounds.width) {
					srcPoint.x = srcBounds.x;
					srcPoint.y++;
				}
				destPoint.x++;
				if (destPoint.x >= destBounds.x + destBounds.width) {
					destPoint.x = destBounds.x;
					destPoint.y++;
				}
				addToFlush(destBounds);
			}
		}
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

		if (pageQueue != null) {
			VCLGraphics.PageQueueItem pqi = new VCLGraphics.PageQueueItem(VCLGraphics.drawLineMethod, new Object[]{ new Integer(x1), new Integer(y1), new Integer(x2), new Integer(y2), new Integer(color) });
			pageQueue.postDrawingOperation(pqi);
			return;
		}

		Rectangle bounds = new Rectangle(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
		if (bounds.width < 0) {
			bounds.x += bounds.width;
			bounds.width *= -1;
		}
		if (bounds.height < 0) {
			bounds.y += bounds.height;
			bounds.height *= -1;
		}
		if (xor) {
			VCLImage srcImage = new VCLImage(bounds.width, bounds.height, bitCount);
			Graphics2D srcGraphics = srcImage.getImage().createGraphics();
			VCLGraphics.setDefaultRenderingAttributes(srcGraphics);
			srcGraphics.setColor(new Color(color));
			srcGraphics.translate(bounds.x * -1, bounds.y * -1);
			srcGraphics.drawLine(x1, y1, x2, y2);
			srcGraphics.dispose();
			drawImageXOR(srcImage, 0, 0, bounds.width, bounds.height, bounds.x, bounds.y, bounds.width, bounds.height);
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
	 * <code>VCLBitmap</code> where the pixels are non-white.
	 *
	 * @param bmp the bitmap to be drawn
	 * @param color the color that is used to match which pixels to draw
	 * @param srcX the x coordinate of the bitmap to be drawn
	 * @param srcY the y coordinate of the bitmap to be drawn
	 * @param srcWidth the width of the graphics to be drawn
	 * @param srcHeight the height of the graphics to be drawn
	 * @param destX the x coordinate of the graphics to draw to
	 * @param destY the y coordinate of the graphics to draw to
	 * @param destWidth the width of the graphics to copy to
	 * @param destHeight the height of the graphics to copy to
	 */
	public void drawMask(VCLBitmap bmp, int color, int srcX, int srcY, int srcWidth, int srcHeight, int destX, int destY, int destWidth, int destHeight) {

		Rectangle destBounds = new Rectangle(destX, destY, destWidth, destHeight).intersection(graphicsBounds);
		if (destBounds.isEmpty())
			return;
		Shape clip = graphics.getClip();
		if (clip != null) {
			if (!clip.intersects(destBounds))
				return;
			else if (clip.contains((double)destBounds.x, (double)destBounds.y, (double)destBounds.width, (double)destBounds.height))
				clip = null;
		}
		if (image == null || clip != null || srcWidth != destWidth || srcHeight != destHeight) {
			// Draw to a temporary image
			VCLImage maskImage = new VCLImage(srcWidth, srcHeight, bmp.getBitCount());
			maskImage.getGraphics().drawMask(bmp, color, srcX, srcY, srcWidth, srcHeight, 0, 0, srcWidth, srcHeight);
			drawImage(maskImage.getImage(), 0, 0, srcWidth, srcHeight, destX, destY, destWidth, destHeight);
			maskImage.dispose();
		}
		else {
			Rectangle srcBounds = new Rectangle(srcX + destBounds.x - destX, srcY + destBounds.y - destY, destBounds.width, destBounds.height);
			int[] destData = image.getData();
			int destDataWidth = image.getWidth();
			Point srcPoint = new Point(srcBounds.x, srcBounds.y);
			Point destPoint = new Point(destBounds.x, destBounds.y);
			int totalPixels = destBounds.width * destBounds.height;

			// If the pixel is black, set the pixel to the mask color
			color |= 0xff000000;
			for (int i = 0; i < totalPixels; i++) {
				// Copy pixel
				if (bmp.getPixel(srcPoint) == 0xff000000)
					destData[(destPoint.y * destDataWidth) + destPoint.x] = color;

				// Update current points
				srcPoint.x++;
				if (srcPoint.x >= srcBounds.x + destBounds.width) {
					srcPoint.x = srcBounds.x;
					srcPoint.y++;
				}
				destPoint.x++;
				if (destPoint.x >= destBounds.x + destBounds.width) {
					destPoint.x = destBounds.x;
					destPoint.y++;
				}
				addToFlush(destBounds);
			}
		}

	}

	/**
	 * Draws or fills the specified polygon with the specified color.
	 *
	 * @param npoints the total number of points in the polygon
	 * @param xpoints the array of x coordinates
	 * @param ypoints the array of y coordinates
	 * @param color the color of the polygon
	 * @param fill <code>true</code> to fill the polygon and <code>false</code>
	 *  to draw just the outline
	 */
	public void drawPolygon(int npoints, int[] xpoints, int[] ypoints, int color, boolean fill) {

		if (pageQueue != null) {
			VCLGraphics.PageQueueItem pqi = new VCLGraphics.PageQueueItem(VCLGraphics.drawPolygonMethod, new Object[]{ new Integer(npoints), xpoints, ypoints, new Integer(color), new Boolean(fill) });
			pageQueue.postDrawingOperation(pqi);
			return;
		}

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
				drawImageXOR(srcImage, 0, 0, bounds.width, bounds.height, bounds.x, bounds.y, bounds.width, bounds.height);
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
	 * @param npoints the total number of points in the polyline
	 * @param xpoints the array of x coordinates
	 * @param ypoints the array of y coordinates
	 * @param color the color of the polyline
	 */
	public void drawPolyline(int npoints, int[] xpoints, int[] ypoints, int color) {

		if (pageQueue != null) {
			VCLGraphics.PageQueueItem pqi = new VCLGraphics.PageQueueItem(VCLGraphics.drawPolylineMethod, new Object[]{ new Integer(npoints), xpoints, ypoints, new Integer(color) });
			pageQueue.postDrawingOperation(pqi);
			return;
		}

		Rectangle bounds = new Polygon(xpoints, ypoints, npoints).getBounds();
		if (xor) {
			VCLImage srcImage = new VCLImage(bounds.width, bounds.height, bitCount);
			Graphics2D srcGraphics = srcImage.getImage().createGraphics();
			VCLGraphics.setDefaultRenderingAttributes(srcGraphics);
			srcGraphics.setColor(new Color(color));
			srcGraphics.translate(bounds.x * -1, bounds.y * -1);
			srcGraphics.drawPolyline(xpoints, ypoints, npoints);
			srcGraphics.dispose();
			drawImageXOR(srcImage, 0, 0, bounds.width, bounds.height, bounds.x, bounds.y, bounds.width, bounds.height);
			srcImage.dispose();
		}
		else {
			graphics.setColor(new Color(color));
			graphics.drawPolyline(xpoints, ypoints, npoints);
			addToFlush(bounds);
		}

	}

	/**
	 * Draws or fills the specified set of polygons with the specified color.
	 *
	 * @param npoly the number of polygons
	 * @param npoints the array of the total number of points in each polygon
	 * @param xpoints the array of arrays of x coordinates
	 * @param ypoints the array of arrays of y coordinates
	 * @param color the color of the polygons
	 * @param fill <code>true</code> to fill the polygons and <code>false</code>
	 *  to draw just the outline
	 */
	public void drawPolyPolygon(int npoly, int[] npoints, int[][] xpoints, int[][] ypoints, int color, boolean fill) {

		if (pageQueue != null) {
			VCLGraphics.PageQueueItem pqi = new VCLGraphics.PageQueueItem(VCLGraphics.drawPolyPolygonMethod, new Object[]{ new Integer(npoly), npoints, xpoints, ypoints, new Integer(color), new Boolean(fill) });
			pageQueue.postDrawingOperation(pqi);
			return;
		}

		Area area = new Area(new Polygon(xpoints[0], ypoints[0], npoints[0]));
		for (int i = 1; i < npoly; i++) {
			Area a = new Area(new Polygon(xpoints[i], ypoints[i], npoints[i]));
			if (fill)
				area.exclusiveOr(a);
			else
				area.add(a);
		}
		Rectangle bounds = area.getBounds();
		if (bounds.isEmpty())
			return;

		if (fill) {
			if (xor)
				graphics.setXORMode(Color.black);
			graphics.setColor(new Color(color));
			graphics.fill(area);
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
				for (int i = 0; i < npoly; i++)
					srcGraphics.drawPolygon(xpoints[i], ypoints[i], npoints[i]);
				srcGraphics.dispose();
				drawImageXOR(srcImage, 0, 0, bounds.width, bounds.height, bounds.x, bounds.y, bounds.width, bounds.height);
				srcImage.dispose();
			}
			else {
				graphics.setColor(new Color(color));
				for (int i = 0; i < npoly; i++)
					graphics.drawPolygon(xpoints[i], ypoints[i], npoints[i]);
				addToFlush(bounds);
			}
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

		if (pageQueue != null) {
			VCLGraphics.PageQueueItem pqi = new VCLGraphics.PageQueueItem(VCLGraphics.drawRectMethod, new Object[]{ new Integer(x), new Integer(y), new Integer(width), new Integer(height), new Integer(color), new Boolean(fill) });
			pageQueue.postDrawingOperation(pqi);
			return;
		}

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
				drawImageXOR(srcImage, 0, 0, width - 1, height - 1, x, y, width - 1, height - 1);
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

		if (pageQueue != null) {
			VCLGraphics.PageQueueItem pqi = new VCLGraphics.PageQueueItem(VCLGraphics.drawTextArrayMethod, new Object[]{ new Integer(x), new Integer(y), chars, font, new Integer(color), offsets });
			pageQueue.postDrawingOperation(pqi);
			return;
		}

		Font f = font.getFont();
		RenderingHints hints = graphics.getRenderingHints();
		graphics.setFont(f);
		if (font.isAntialiased())
			hints.put(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
		else
			hints.put(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_OFF);
		graphics.setRenderingHints(hints);
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

		if (userClip != null)
			graphics.setClip(userClip);
		else
			graphics.setClip(graphicsBounds);

	}

	/**
	 * Flushes any pixels in the underlying <code>VCLImage</code> to the
	 * underlying <code>VCLFrame</code>.
	 */
	void flush() {

		if (panelGraphics != null && update != null) {
			// Add a little extra area so that we don't miss any antialiased
			// pixels
			update.x -= 1;
			update.y -= 1;
			update.width += 2;
			update.height += 2;
			panelGraphics.setClip(update);
			panelGraphics.drawRenderedImage(image.getImage(), null);
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
	 * Returns the resolution of the underlying graphics device.
	 *
	 * @return the resolution of the underlying graphics device.
	 */
	public Dimension getResolution() {

		if (pageFormat != null)
			return pageFormat.getPageResolution();
		else
			return new Dimension(VCLGraphics.screenResolution, VCLGraphics.screenResolution);

	}

	/**
	 * Returns the font resolution of the underlying graphics device.
	 *
	 * @return the font resolution of the underlying graphics device.
	 */
	public Dimension getScreenFontResolution() {

		return new Dimension(VCLGraphics.screenFontResolution, VCLGraphics.screenFontResolution);

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

		// Clip any area outside of the image
		Rectangle bounds = new Rectangle(x, y, width, height).intersection(graphicsBounds);
		if (bounds.isEmpty())
			return;

		// Invert the image 
		if ((options & VCLGraphics.SAL_INVERT_TRACKFRAME) == VCLGraphics.SAL_INVERT_TRACKFRAME) {
			VCLImage srcImage = new VCLImage(bounds.width, bounds.height, bitCount);
			Graphics2D srcGraphics = srcImage.getImage().createGraphics();
			VCLGraphics.setDefaultRenderingAttributes(srcGraphics);
			BasicStroke stroke = (BasicStroke)srcGraphics.getStroke();
			srcGraphics.setStroke(new BasicStroke(1.0f, BasicStroke.CAP_BUTT, BasicStroke.JOIN_BEVEL, 1.0f, new float[]{ 2.0f, 2.0f }, 0.0f));
			srcGraphics.setColor(Color.white);
			srcGraphics.translate(bounds.x * -1, bounds.y * -1);
			srcGraphics.drawRect(x, y, width, height);
			srcGraphics.dispose();
			drawImageXOR(srcImage, 0, 0, bounds.width, bounds.height, bounds.x, bounds.y, bounds.width, bounds.height);
			srcImage.dispose();
		}
		else if ((options & VCLGraphics.SAL_INVERT_50) == VCLGraphics.SAL_INVERT_50) {
			VCLImage srcImage = new VCLImage(bounds.width, bounds.height, VCLGraphics.image50.getBitCount());
			Graphics2D srcGraphics = srcImage.getImage().createGraphics();
			VCLGraphics.setDefaultRenderingAttributes(srcGraphics);
			srcGraphics.setPaint(new TexturePaint(VCLGraphics.image50.getImage(), new Rectangle(0, 0, VCLGraphics.image50.getWidth(), VCLGraphics.image50.getHeight()).getBounds2D()));
			srcGraphics.translate(bounds.x * -1, bounds.y * -1);
			srcGraphics.fillRect(x, y, width, height);
			srcGraphics.dispose();
			drawImageXOR(srcImage, 0, 0, bounds.width, bounds.height, bounds.x, bounds.y, bounds.width, bounds.height);
			srcImage.dispose();
		}
		else {
			Shape clip = graphics.getClip();
			if (clip != null) {
				if (!clip.intersects(bounds))
					return;
				else if (clip.contains((double)bounds.x, (double)bounds.y, (double)bounds.width, (double)bounds.height))
					clip = null;
			}
			if (clip != null) {
				// Draw to a temporary image
				VCLImage srcImage = new VCLImage(bounds.width, bounds.height, bitCount);
				srcImage.getGraphics().drawImage(image.getImage(), bounds.x, bounds.y, bounds.width, bounds.height, 0, 0, bounds.width, bounds.height);
				srcImage.getGraphics().invert(0, 0, bounds.width, bounds.height, options);
				drawImage(srcImage.getImage(), 0, 0, bounds.width, bounds.height, bounds.x, bounds.y, bounds.width, bounds.height);
				srcImage.dispose();
			}
			else {
				int[] destData = image.getData();
				int destDataWidth = image.getWidth();
				Point destPoint = new Point(bounds.x, bounds.y);
				int totalPixels = bounds.width * bounds.height;

				for (int i = 0; i < totalPixels; i++) {
					// Invert pixel
					int j = (destPoint.y * destDataWidth) + destPoint.x;
					destData[j] = ~destData[j] | 0xff000000;

					// Update current point
					destPoint.x++;
					if (destPoint.x >= bounds.x + bounds.width) {
						destPoint.x = bounds.x;
						destPoint.y++;
					}
				}
				addToFlush(bounds);
			}
		}

	}

	/**
	 * Inverts the specified polyline depending on the specified options.
	 *
	 * @param npoints the total number of points in the polyline
	 * @param xpoints the array of x coordinates
	 * @param ypoints the array of y coordinates
	 * @param options the invert options
	 */
	public void invert(int npoints, int[] xpoints, int[] ypoints, int options) {

		if (image == null)
			return;

		Polygon polygon = new Polygon(xpoints, ypoints, npoints);

		// Clip any area outside of the image
		Rectangle bounds = polygon.getBounds().intersection(graphicsBounds);

		// Some polylines can be simply a vertical or horizontal line
		if (bounds.width == 0)
			bounds.width = 1;
		if (bounds.height == 0)
			bounds.height = 1;

		if (bounds.isEmpty())
			return;

		// Invert the image 
		if ((options & VCLGraphics.SAL_INVERT_TRACKFRAME) == VCLGraphics.SAL_INVERT_TRACKFRAME) {
			VCLImage srcImage = new VCLImage(bounds.width, bounds.height, bitCount);
			Graphics2D srcGraphics = srcImage.getImage().createGraphics();
			VCLGraphics.setDefaultRenderingAttributes(srcGraphics);
			BasicStroke stroke = (BasicStroke)srcGraphics.getStroke();
			srcGraphics.setStroke(new BasicStroke(1.0f, BasicStroke.CAP_BUTT, BasicStroke.JOIN_BEVEL, 1.0f, new float[]{ 2.0f, 2.0f }, 0.0f));
			srcGraphics.setColor(Color.white);
			srcGraphics.translate(bounds.x * -1, bounds.y * -1);
			srcGraphics.drawPolyline(xpoints, ypoints, npoints);
			srcGraphics.dispose();
			drawImageXOR(srcImage, 0, 0, bounds.width, bounds.height, bounds.x, bounds.y, bounds.width, bounds.height);
			srcImage.dispose();
		}
		else if ((options & VCLGraphics.SAL_INVERT_50) == VCLGraphics.SAL_INVERT_50) {
			VCLImage srcImage = new VCLImage(bounds.width, bounds.height, VCLGraphics.image50.getBitCount());
			Graphics2D srcGraphics = srcImage.getImage().createGraphics();
			VCLGraphics.setDefaultRenderingAttributes(srcGraphics);
			srcGraphics.setPaint(new TexturePaint(VCLGraphics.image50.getImage(), new Rectangle(0, 0, VCLGraphics.image50.getWidth(), VCLGraphics.image50.getHeight()).getBounds2D()));
			srcGraphics.translate(bounds.x * -1, bounds.y * -1);
			srcGraphics.fillPolygon(polygon);
			srcGraphics.dispose();
			drawImageXOR(srcImage, 0, 0, bounds.width, bounds.height, bounds.x, bounds.y, bounds.width, bounds.height);
			srcImage.dispose();
		}
		else {
			VCLImage srcImage = new VCLImage(bounds.width, bounds.height, bitCount);
			srcImage.getGraphics().drawImage(image.getImage(), bounds.x, bounds.y, bounds.width, bounds.height, 0, 0, bounds.width, bounds.height);

			int[] destData = srcImage.getData();
			int totalPixels = bounds.width * bounds.height;

			// Invert pixel
			for (int i = 0; i < totalPixels; i++)
				destData[i] = ~destData[i] | 0xff000000;

			Shape clip = graphics.getClip();
			Area polygonClip = new Area(polygon);
			if (clip != null) {
				Area area = new Area(clip);
				area.intersect(polygonClip);
				graphics.setClip(area);
			}
			else {
				graphics.setClip(polygonClip);
			}
			drawImage(srcImage.getImage(), 0, 0, bounds.width, bounds.height, bounds.x, bounds.y, bounds.width, bounds.height);
			graphics.setClip(clip);
			srcImage.dispose();
		}

	}

	/**
	 * Resets the applied clipping area.
	 */
	public void resetClipRegion() {

		userClip = null;
		graphics.setClip(new Area(graphicsBounds));

	}

	/**
	 * Resets the underlying graphics context.
	 */
	synchronized void resetGraphics() {

		if (frame != null) {
			graphics.dispose();
			if (panelGraphics != null)
				panelGraphics.dispose();
			image.dispose();
			if (frame.getWindow().isShowing()) {
				Panel p = frame.getPanel();
				panelGraphics = (Graphics2D)p.getGraphics();
				Rectangle bounds = p.getBounds();
				graphicsBounds = new Rectangle(0, 0, bounds.width, bounds.height);
			}
			else {
				panelGraphics = null;
				graphicsBounds = new Rectangle(0, 0, 1, 1);
			}
			image = new VCLImage(graphicsBounds.width, graphicsBounds.height, frame.getBitCount());
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
			Shape clip = graphics.getClip();
			if (clip != null && !clip.contains((double)x, (double)y))
				return;
			int[] pixels = image.getData();
			int i = (image.getWidth() * y) + x;
			color |= 0xff000000;
			if (xor)
				color = pixels[i] ^ 0xff000000 ^ color;
			pixels[i] = color;
			addToFlush(new Rectangle(x, y, 1, 1));
		}
		else {
			drawLine(x, y, x, y, color);
		}

	}

	/**
	 * Enables or disables drawing in XOR mode.
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
		if (userClip.isEmpty())
			userClip = null;

	}

	/**
	 * The <code>PageQueue</code> object holds pointers to the beginning and
	 * end of the drawing queue.
	 */
	final class PageQueue {

		VCLGraphics graphics = null;

		VCLGraphics.PageQueueItem head = null;

		VCLImage pageImage = null;

		Area pageImageClip = null;

		VCLGraphics.PageQueueItem tail = null;

		PageQueue(VCLGraphics g) {

			graphics = g;

		}

		void dispose() {

			graphics.pageQueue = null;
			Graphics2D g = graphics.graphics;

			// Draw the combined image
			if (pageImage != null) {
				if (pageImageClip != null) {
					g.setClip(pageImageClip);
					g.drawRenderedImage(pageImage.getImage(), null);
				}
				pageImage.dispose();
			}
			pageImage = null;

			// Invoke all of the queued drawing operations
			while (head != null) {
				if (pageImageClip != null) {
					Area area = new Area(pageImageClip);
					if (head.imageClip != null)
						area.subtract(head.imageClip);
					head.clip.subtract(area);
				}
				g.setClip(head.clip);
				try {
					head.method.invoke(graphics, head.params);
				}
				catch (Throwable t) {}
				head = head.next;
			}
			tail = null;
			graphics = null;
			pageImageClip = null;

		}

		void postDrawingOperation(VCLGraphics.PageQueueItem i) {

			// Add the drawing operation to the queue
			Graphics2D g = graphics.graphics;
			Shape clip = g.getClip();
			if (clip != null)
				i.clip = new Area(clip);
			else
				i.clip = new Area(graphics.graphicsBounds);
			if (pageImageClip != null)
				i.imageClip = new Area(pageImageClip);
			if (head != null) {
				tail.next = i;
				tail = i;
			}
			else {
				head = tail = i;
			}

		}

		void updateImageClip(Rectangle b) {

			if (b == null || b.isEmpty())
				return;

			Area area = new Area(b);
			Graphics2D g = graphics.graphics;
			Shape clip = g.getClip();
			if (clip != null)
				area.intersect(new Area(clip));
			if (pageImageClip != null)
				pageImageClip.add(area);
			else
				pageImageClip = area;

			// Update the image clip for all queue items
			VCLGraphics.PageQueueItem pqi = head;
			while (pqi != null) {
				if (pageImageClip != null && pqi.imageClip != null)
					pqi.imageClip.subtract(pageImageClip);
				pqi = pqi.next;
			}

		}

	}

	/**
	 * The <code>QueueItem</code> object is a wrapper for queued
	 * <code>VCLGraphics</cade> drawing calls.
	 */
	final class PageQueueItem {

		Area clip = null;

		Area imageClip = null;

		Method method = null;

		PageQueueItem next = null;

		Object[] params = null;

		PageQueueItem(Method m, Object[] p) {

			method = m;
			params = p;

		}

	}

}
