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
import java.awt.image.ImageObserver;
import java.util.LinkedList;

/**
 * The Java class that implements the SalGraphics C++ class methods.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public final class VCLGraphics implements ImageObserver {

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
	private static VCLImage image50 = null;

	/**
	 * The cached screen resolution.
	 */
	private static int screenResolution = 0;

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
		hints.put(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
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

		// Set the screen resolution
		screenResolution = Toolkit.getDefaultToolkit().getScreenResolution();

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
	 * The bounds of the graphics context.
	 */
	private Rectangle pageBounds = null;

	/**
	 * The panel's graphics context.
	 */
	private Graphics2D panelGraphics = null;

	/**
	 * The cached graphics device resolution.
	 */
	private int resolution = 0;

	/**
	 * The cached font resolution.
	 */
	private int screenFontResolution = 0;

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
		resolution = VCLGraphics.screenResolution;
		screenFontResolution = VCLGraphics.screenResolution;

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
		resolution = VCLGraphics.screenResolution;
		screenFontResolution = VCLGraphics.screenResolution;

	}

	/**
	 * Constructs a new <code>VCLGraphics</code> instance from an existing
	 * <code>Graphics2D</code> instance.
	 *
	 * @param g the <code>Graphics2D</code> instance
	 * @param r the resolution in pixels per inch
	 * @param b the page bounds in pixels
	 */
	VCLGraphics(Graphics2D g, int r, Rectangle b) {

		graphics = g;
		resolution = r;
		screenFontResolution = VCLGraphics.screenResolution;
		pageBounds = b;

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
		if (image != null && graphics != null)
			graphics.dispose();
		graphics = null;
		if (panelGraphics != null)
			panelGraphics.dispose();
		panelGraphics = null;
		if (frame != null && image != null)
			image.dispose();
		image = null;
		frame = null;
		pageBounds = null;
		resolution = 0;
		screenFontResolution = 0;
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
		if (image == null || clip != null || destWidth != srcWidth || destHeight != srcHeight) {
			// Draw to a temporary image and scale it
			VCLImage bmpImage = new VCLImage(srcWidth, srcHeight, bmp.getBitCount());
			bmpImage.getGraphics().drawBitmap(bmp, 0, 0, srcWidth, srcHeight, srcX, srcY, srcWidth, srcHeight);
			if (image == null && VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX) {
				// Mac OS X will render only PDF 1.3 (which is not supported by
				// the Mac OS X Preview application) if we draw images that are
				// smaller than the page
				VCLImage pageImage = new VCLImage(pageBounds.width + 1, pageBounds.height + 1, bmp.getBitCount());
				pageImage.getGraphics().drawImage(bmpImage, destX, destY, destWidth, destHeight, 0, 0, srcWidth, srcHeight);
				drawImage(pageImage, 0, 0, pageImage.getWidth(), pageImage.getHeight(), 0, 0, pageImage.getWidth(), pageImage.getHeight());
				pageImage.dispose();
			}
			else {
				drawImage(bmpImage, destX, destY, destWidth, destHeight, 0, 0, srcWidth, srcHeight);
			}
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
		if (image == null || clip != null || destWidth != srcWidth || destHeight != srcHeight) {
			VCLImage mergedImage = new VCLImage(srcWidth, srcHeight, bmp.getBitCount());
			mergedImage.getGraphics().drawBitmap(bmp, transBmp, 0, 0, srcWidth, srcHeight, srcX, srcY, srcWidth, srcHeight);
			// Draw to a temporary image and scale it
			if (image == null && VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX) {
				// Mac OS X will render only PDF 1.3 (which is not supported by
				// the Mac OS X Preview application) if we draw images that are
				// smaller than the page
				Rectangle bounds = graphics.getDeviceConfiguration().getBounds();
				VCLImage pageImage = new VCLImage(pageBounds.width + 1, pageBounds.height + 1, bmp.getBitCount());
				pageImage.getGraphics().drawImage(mergedImage, destX, destY, destWidth, destHeight, 0, 0, srcWidth, srcHeight);
				drawImage(pageImage, 0, 0, pageImage.getWidth(), pageImage.getHeight(), 0, 0, pageImage.getWidth(), pageImage.getHeight());
				pageImage.dispose();
			}
			else {
				drawImage(mergedImage, destX, destY, destWidth, destHeight, 0, 0, srcWidth, srcHeight);
			}
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
					destData[(destPoint.y * destDataWidth) + destPoint.x] = bmp.getPixel(srcPoint) | 0xff000000;

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

		BufferedImage i = img.getImage();
		synchronized (i) {
			if (!graphics.drawImage(img.getImage(), destX, destY, destX + destWidth, destY + destHeight, srcX, srcY, srcX + srcWidth, srcY + srcHeight, this)) {
				try {
					i.wait();
				}
				catch (Throwable t) {}
			}
		}

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
			drawImage(img, destX, destY, destWidth, destHeight, srcX, srcY, srcWidth, srcHeight);
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
			destImage.getGraphics().drawImageXOR(img, 0, 0, destWidth, destHeight, srcX, srcY, srcWidth, srcHeight);
			drawImage(destImage, destX, destY, destWidth, destHeight, 0, 0, destWidth, destHeight);
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
			drawImage(maskImage, destX, destY, destWidth, destHeight, 0, 0, srcWidth, srcHeight);
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
	 * @param npoints the total number of points in the polygon
	 * @param xpoints the array of x coordinates
	 * @param ypoints the array of y coordinates
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
	 * @param npoints the total number of points in the polyline
	 * @param xpoints the array of x coordinates
	 * @param ypoints the array of y coordinates
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
				drawImageXOR(srcImage, x, y, width - 1, height - 1, 0, 0, width - 1, height - 1);
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
			synchronized (image) {
			 	if (!panelGraphics.drawImage(image.getImage(), 0, 0, this)) {
					try {
						image.wait();
					}
					catch (Throwable t) {}
				}
			}	
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
	 * Returns the resolution of the underlying graphics device.
	 *
	 * @return the resolution of the underlying graphics device.
	 */
	public int getResolution() {

		System.out.println("Resolution: " + resolution);
		return resolution;

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
	 * This method is called when information about an image which was
	 * previously requested using an asynchronous interface becomes available.
	 *
	 * @param img the image being observed
	 * @param infoflags the <code>ImageObserver</code> flags
	 * @param x the x coordinate  
	 * @param y the y coordinate
	 * @param w the width
	 * @param h the height
	 * @return <code>false</code> if the infoflags indicate that the image is
	 *  completely loaded or else <code>true</code>
	 */
	public boolean imageUpdate(Image img, int infoflags, int x, int y, int width, int height) {

		if ((infoflags & (ImageObserver.ALLBITS | ImageObserver.ABORT)) != 0) {
			// Notify the methods that are drawing that rendering is complete
			synchronized (img) {
				img.notifyAll();
			}
			return false;
		}

		return true;

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

		Polygon polygon = new Polygon();
		polygon.addPoint(x, y);
		polygon.addPoint(x + width, y);
		polygon.addPoint(x + width, y + height);
		polygon.addPoint(x, y + height);
		if ((options & VCLGraphics.SAL_INVERT_TRACKFRAME) == VCLGraphics.SAL_INVERT_TRACKFRAME)
			polygon.addPoint(x, y);
		invert(polygon.npoints, polygon.xpoints, polygon.ypoints, options);

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
		Rectangle bounds = polygon.getBounds();
		bounds = new Rectangle(0, 0, image.getWidth(), image.getHeight()).intersection(bounds);
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
			drawImageXOR(srcImage, bounds.x, bounds.y, bounds.width, bounds.height, 0, 0, bounds.width, bounds.height);
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
			drawImageXOR(srcImage, bounds.x, bounds.y, bounds.width, bounds.height, 0, 0, bounds.width, bounds.height);
			srcImage.dispose();
		}
		else {
			VCLImage srcImage = new VCLImage(bounds.width, bounds.height, bitCount);
			srcImage.getGraphics().drawImage(image, 0, 0, bounds.width, bounds.height, bounds.x, bounds.y, bounds.width, bounds.height);

			int[] destData = srcImage.getData();
			int totalPixels = bounds.width * bounds.height;

			// Invert pixel
			for (int i = 0; i < totalPixels; i++)
				destData[i] = ~destData[i] | 0xff000000;

			Shape clip = graphics.getClip();
			Area polygonClip = new Area(polygon);
			if (clip != null) {
				Area area = new Area(clip);
				area.add(polygonClip);
				graphics.setClip(area);
			}
			else {
				graphics.setClip(polygonClip);
			}
			drawImage(srcImage, bounds.x, bounds.y, bounds.width, bounds.height, 0, 0, bounds.width, bounds.height);
			graphics.setClip(clip);
			srcImage.dispose();
		}
		addToFlush(bounds);

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
			Shape clip = graphics.getClip();
			if (clip != null && !clip.contains((double)x, (double)y))
				return;
			int[] pixels = image.getData();
			int i = (image.getWidth() * y) + x;
			color |= 0xff000000;
			if (xor)
				color = pixels[i] ^ 0xff000000 ^ color;
			pixels[i] = color;
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
