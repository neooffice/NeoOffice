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
import java.awt.font.FontRenderContext;
import java.awt.font.GlyphVector;
import java.awt.geom.AffineTransform;
import java.awt.geom.Area;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.lang.reflect.Method;

/**
 * The Java class that implements the SalGraphics C++ class methods.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public final class VCLGraphics {

	/**
	 * The AUTO_FLUSH_INTERVAL constant.
	 */
	public final static long AUTO_FLUSH_INTERVAL = 100;

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
	 * The GF_ROTMASK constant.
	 */
	public final static int GF_ROTMASK = 0x03000000;

	/**
	 * The GF_ROTL constant.
	 */
	public final static int GF_ROTL = 0x01000000;

	/**
	 * The GF_ROTR constant.
	 */
	public final static int GF_ROTR = 0x03000000;

	/**
	 * The drawGlyphs method.
	 */
	private static Method drawGlyphsMethod = null;

	/**
	 * The drawImage method.
	 */
	private static Method drawImageMethod = null;

	/**
	 * The drawLine method.
	 */
	private static Method drawLineMethod = null;

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
	 * The drawRect method.
	 */
	private static Method drawRectMethod = null;

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
	 * The use default font flag.
	 */
	private static boolean useDefaultFont = true;

	/**
	 * Emits an audio beep.
	 */
	public static void beep() {

		Toolkit.getDefaultToolkit().beep();

	}

	/**
	 * Set the use default font flag.
	 *
	 * @param b the default font flag
	 */
	public static void setUseDefaultFont(boolean b) {

		useDefaultFont = b;

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
		if (screenResolution < VCLScreen.MIN_SCREEN_RESOLUTION)
			screenResolution = VCLScreen.MIN_SCREEN_RESOLUTION;
		if (screenFontResolution < VCLScreen.MIN_SCREEN_RESOLUTION)
			screenFontResolution = VCLScreen.MIN_SCREEN_RESOLUTION;

		// Set the method references
		try {
			drawGlyphsMethod = VCLGraphics.class.getMethod("drawGlyphs", new Class[]{ int.class, int.class, int[].class, int[].class, VCLFont.class, int.class, int.class, int.class, int.class, int.class });
		}
		catch (Throwable t) {
			t.printStackTrace();
		}
		try {
			drawImageMethod = VCLGraphics.class.getMethod("drawImage", new Class[]{ Image.class, int.class, int.class, int.class, int.class, int.class, int.class, int.class, int.class });
		}
		catch (Throwable t) {
			t.printStackTrace();
		}
		try {
			drawLineMethod = VCLGraphics.class.getMethod("drawLine", new Class[]{ int.class, int.class, int.class, int.class, int.class });
		}
		catch (Throwable t) {
			t.printStackTrace();
		}
		try {
			drawPolygonMethod = VCLGraphics.class.getMethod("drawPolygon", new Class[]{ int.class, int[].class, int[].class, int.class, boolean.class });
		}
		catch (Throwable t) {
			t.printStackTrace();
		}
		try {
			drawPolylineMethod = VCLGraphics.class.getMethod("drawPolyline", new Class[]{ int.class, int[].class, int[].class, int.class });
		}
		catch (Throwable t) {
			t.printStackTrace();
		}
		try {
			drawPolyPolygonMethod = VCLGraphics.class.getMethod("drawPolyPolygon", new Class[]{ int.class, int[].class, int[][].class, int[][].class, int.class, boolean.class });
		}
		catch (Throwable t) {
			t.printStackTrace();
		}
		try {
			drawRectMethod = VCLGraphics.class.getMethod("drawRect", new Class[]{ int.class, int.class, int.class, int.class, int.class, boolean.class });
		}
		catch (Throwable t) {
			t.printStackTrace();
		}
	}

	/**
	 * The auto flush flag.
	 */
	private boolean autoFlush = false;

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
	 * The next auto flush.
	 */
	private long nextAutoFlush = 0;

	/**
	 * The printer page format.
	 */
	private VCLPageFormat pageFormat = null;

	/**
	 * The printer drawing queue.
	 */
	private VCLGraphics.PageQueue pageQueue = null;

	/**
	 * The rotated page flag.
	 */
	private boolean rotatedPage = false;

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
			Rectangle bounds = p.getBounds();
			graphicsBounds = new Rectangle(0, 0, bounds.width, bounds.height);
		}
		else {
			graphicsBounds = new Rectangle(0, 0, 1, 1);
		}
		image = new VCLImage(graphicsBounds.width, graphicsBounds.height, frame.getBitCount());
		graphics = image.getImage().createGraphics();
		bitCount = image.getBitCount();
		resetClipRegion();

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
		bitCount = image.getBitCount();
		resetClipRegion();

	}

	/**
	 * Constructs a new <code>VCLGraphics</code> instance from an existing
	 * <code>Graphics2D</code> instance.
	 *
	 * @param g the <code>Graphics2D</code> instance
	 * @param p the <code>VCLPageFormat</code> instance
	 * @param r <code>true</code> if the page is rotated otherwise
	 *  <code>false</code>
	 */
	VCLGraphics(Graphics2D g, VCLPageFormat p, boolean r) {

		pageFormat = p;
		rotatedPage = r;
		graphicsBounds = new Rectangle(pageFormat.getImageableBounds());
		graphicsBounds.x = 0;
		graphicsBounds.y = 0;
		graphics = (Graphics2D)g.create(graphicsBounds.x, graphicsBounds.y, graphicsBounds.width, graphicsBounds.height);
		int b = graphics.getDeviceConfiguration().getColorModel().getPixelSize();

		if (b <= 1)
			bitCount = 1;
		else if (b <= 4)
			bitCount = 4;
		else if (b <= 8)
			bitCount = 8;
		else
			bitCount = 24;

		resetClipRegion();

		// Mac OS X sometimes mangles images when multiple images are rendered
		// to a printer so we need to combine all images into one image and
		// defer other drawing operations until after the combined image is
		// created
		if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX)
			pageQueue = new VCLGraphics.PageQueue(this);

	}

	/**
	 * Marks the entire graphics bounds as requiring flushing.
	 */
	void addToFlush() {

		if (frame != null)
			update = new Rectangle(graphicsBounds);

		if (autoFlush)
			autoFlush();

	}

	/**
	 * Unions the specified rectangle to the rectangle that requires flushing.
	 *
	 * @param b the rectangle to flush
	 */
	void addToFlush(Rectangle b) {

		if (frame != null && !b.isEmpty()) {
			if (update != null)
				update.add(b);
			else
				update = b;

			if (autoFlush)
				autoFlush();
		}

	}

	/**
	 * Flushes if the auto flush timer has expired.
	 */
	void autoFlush() {

		long currentTime = System.currentTimeMillis();
		if (currentTime >= nextAutoFlush) {
			flush();
			nextAutoFlush = System.currentTimeMillis() + VCLGraphics.AUTO_FLUSH_INTERVAL;
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
		bitCount = 0;
		if (graphics != null)
			graphics.dispose();
		graphics = null;
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
			if (!clip.intersects(destBounds)) {
				return;
			}
			else if (clip.contains((double)destBounds.x, (double)destBounds.y, (double)destBounds.width, (double)destBounds.height)) {
				clip = null;
			}
			else if (clip instanceof Rectangle) {
				destBounds = destBounds.intersection((Rectangle)clip);
				clip = null;
			}
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
	 * Draws the specified glyph codes using the specified font and color. Note
	 * if rotating glyphs is set, only a single glyph can be handled.
	 *
	 * @param x the x coordinate
	 * @param y the y coordinate
	 * @param glyphs the array of glyph codes to be drawn
	 * @param advances the advances for each character
	 * @param font the font of the text
	 * @param color the color of the text
	 * @param orientation the tenth of degrees to rotate the text
	 * @param glyphOrientation the glyph rotation constant
	 * @param translateX the x coordinate to translate after rotation
	 * @param translateY the y coordinate to translate after rotation
	 */
	public void drawGlyphs(int x, int y, int[] glyphs, int[] advances, VCLFont font, int color, int orientation, int glyphOrientation, int translateX, int translateY) {

		if (pageQueue != null) {
			VCLGraphics.PageQueueItem pqi = new VCLGraphics.PageQueueItem(VCLGraphics.drawGlyphsMethod, new Object[]{ new Integer(x), new Integer(y), glyphs, advances, font, new Integer(color), new Integer(orientation), new Integer(glyphOrientation), new Integer(translateX), new Integer(translateY) });
			pageQueue.postDrawingOperation(pqi);
			return;
		}

		Graphics2D g = (Graphics2D)graphics.create();
		g.translate(x, y);

		// The graphics may adjust the font
		Font f = font.getFont();
		FontMetrics fm = null;

		// Exceptions can be thrown if a font is disabled or removed
		try {
			fm = g.getFontMetrics(f);
		}
		catch (Throwable t) {
			font = font.getDefaultFont();
			f = font.getFont();
			fm = g.getFontMetrics(f);
		}

		g.setFont(f);

		RenderingHints hints = g.getRenderingHints();
		if (font.isAntialiased())
			hints.put(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
		else
			hints.put(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_OFF);
		g.setRenderingHints(hints);
		g.setColor(new Color(color));

		// Set rotation
		if (orientation != 0)
			g.rotate(Math.toRadians((double)orientation / 10) * -1);

		double fScaleX = font.getScaleX();
		g.scale(fScaleX, 1.0);

		GlyphVector gv = f.createGlyphVector(g.getFontRenderContext(), glyphs);

		double fAdvance = 0;
		for (int i = 0; i < glyphs.length; i++) {
			Point2D p = gv.getGlyphPosition(i);
			p.setLocation(fAdvance, p.getY());
			gv.setGlyphPosition(i, p);
			fAdvance += advances[i] / fScaleX;
		}

		glyphOrientation &= VCLGraphics.GF_ROTMASK;
		if ((glyphOrientation & VCLGraphics.GF_ROTMASK) != 0) {
			if (glyphOrientation == VCLGraphics.GF_ROTL)
				g.rotate(Math.toRadians(-90));
			else
				g.rotate(Math.toRadians(90));
		}

		// Draw the text to a scaled graphics
		g.drawGlyphVector(gv, translateX, translateY);

		Rectangle bounds = gv.getLogicalBounds().getBounds();

		// Estimate bounds
		bounds = g.getTransform().createTransformedShape(bounds).getBounds();
		bounds.x += x - 1;
		bounds.y += y - 1;
		if (fScaleX != 1.0)
			bounds.width *= fScaleX;
		bounds.width += 2;
		bounds.height += 2;
		addToFlush(bounds);

		g.dispose();

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
	public void drawImage(Image img, int srcX, int srcY, int srcWidth, int srcHeight, int destX, int destY, int destWidth, int destHeight) {

		if (pageQueue != null) {
			VCLGraphics.PageQueueItem pqi = new VCLGraphics.PageQueueItem(VCLGraphics.drawImageMethod, new Object[]{ img, new Integer(srcX), new Integer(srcY), new Integer(srcWidth), new Integer(srcHeight), new Integer(destX), new Integer(destY), new Integer(destWidth), new Integer(destHeight) });
			pageQueue.postImageOperation(pqi, new Rectangle(destX, destY, destWidth, destHeight), (double)srcWidth / destWidth, (double)srcHeight / destHeight);
			return;
		}

		Rectangle srcBounds = new Rectangle(srcX, srcY, srcWidth, srcHeight).intersection(new Rectangle(0, 0, img.getWidth(null), img.getHeight(null)));
		if (srcBounds.isEmpty())
			return;
		Rectangle destBounds = new Rectangle(destX, destY, destWidth, destHeight).intersection(graphicsBounds);
		if (destBounds.isEmpty())
			return;
		Shape clip = graphics.getClip();
		if (clip != null && !clip.intersects(destBounds))
			return;
		Graphics2D g = (Graphics2D)graphics.create(destX, destY, destWidth, destHeight);
		if (destWidth != srcWidth || destHeight != srcHeight)
			g.scale((double)destWidth / srcWidth, (double)destHeight / srcHeight);
		// Fix bug 625 by not reading outside of the source image's bounds
		g.translate(srcBounds.x - srcX, srcBounds.y - srcY);
		g.drawImage(img, 0, 0, srcBounds.width, srcBounds.height, srcBounds.x, srcBounds.y, srcBounds.x + srcBounds.width, srcBounds.y + srcBounds.height, null);
		g.dispose();
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
			if (!clip.intersects(destBounds)) {
				return;
			}
			else if (clip.contains((double)destBounds.x, (double)destBounds.y, (double)destBounds.width, (double)destBounds.height)) {
				clip = null;
			}
			else if (clip instanceof Rectangle) {
				destBounds = destBounds.intersection((Rectangle)clip);
				clip = null;
			}
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

		Rectangle bounds = new Rectangle(x1, y1, x2 - x1, y2 - y1);
		if (bounds.width < 0) {
			bounds.x += bounds.width;
			bounds.width *= -1;
		}
		if (bounds.height < 0) {
			bounds.y += bounds.height;
			bounds.height *= -1;
		}
		bounds.x -= 1;
		bounds.y -= 1;
		bounds.width += 2;
		bounds.height += 2;
		bounds = bounds.intersection(graphicsBounds);
		if (bounds.isEmpty())
			return;

		if (xor) {
			VCLImage srcImage = new VCLImage(bounds.width, bounds.height, bitCount);
			Graphics2D srcGraphics = srcImage.getImage().createGraphics();
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
			if (!clip.intersects(destBounds)) {
				return;
			}
			else if (clip.contains((double)destBounds.x, (double)destBounds.y, (double)destBounds.width, (double)destBounds.height)) {
				clip = null;
			}
			else if (clip instanceof Rectangle) {
				destBounds = destBounds.intersection((Rectangle)clip);
				clip = null;
			}
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

		if (npoints == 0)
			return;

		Polygon polygon = new Polygon(xpoints, ypoints, npoints);
		Rectangle bounds = polygon.getBounds();
		bounds.x -= 1;
		bounds.y -= 1;
		bounds.width += 2;
		bounds.height += 2;
		bounds = bounds.intersection(graphicsBounds);
		if (bounds.isEmpty())
			return;

		if (xor) {
			VCLImage srcImage = new VCLImage(bounds.width, bounds.height, bitCount);
			Graphics2D srcGraphics = srcImage.getImage().createGraphics();
			srcGraphics.setColor(new Color(color));
			srcGraphics.translate(bounds.x * -1, bounds.y * -1);
			if (fill) {
				srcGraphics.fillPolygon(polygon);
			}
			else {
				for (int i = 1; i < npoints; i++)
					srcGraphics.drawLine(xpoints[i - 1], ypoints[i - 1], xpoints[i], ypoints[i]);
			}
			srcGraphics.dispose();
			drawImageXOR(srcImage, 0, 0, bounds.width, bounds.height, bounds.x, bounds.y, bounds.width, bounds.height);
			srcImage.dispose();
		}
		else {
			graphics.setColor(new Color(color));
			if (fill) {
				graphics.fillPolygon(polygon);
			}
			else {
				for (int i = 1; i < npoints; i++)
					graphics.drawLine(xpoints[i - 1], ypoints[i - 1], xpoints[i], ypoints[i]);
			}
			addToFlush(bounds);
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

		if (npoints == 0)
			return;

		for (int i = 1; i < npoints; i++)
			drawLine(xpoints[i - 1], ypoints[i - 1], xpoints[i], ypoints[i], color);

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

		if (npoly == 0)
			return;

		Area area = null;
		for (int i = 0; i < npoly; i++) {
			Area a = new Area(new Polygon(xpoints[i], ypoints[i], npoints[i]));
			if (area == null) {
				area = a;
				continue;
			}
			if (fill)
				area.exclusiveOr(a);
			else
				area.add(a);
		}
		if (area == null || area.isEmpty())
			return;

		Rectangle bounds = area.getBounds();
		bounds.x -= 1;
		bounds.y -= 1;
		bounds.width += 2;
		bounds.height += 2;
		bounds = bounds.intersection(graphicsBounds);
		if (bounds.isEmpty())
			return;

		if (xor) {
			VCLImage srcImage = new VCLImage(bounds.width, bounds.height, bitCount);
			Graphics2D srcGraphics = srcImage.getImage().createGraphics();
			srcGraphics.setColor(new Color(color));
			srcGraphics.translate(bounds.x * -1, bounds.y * -1);
			if (fill) {
				srcGraphics.fill(area);
			}
			else {
				for (int i = 0; i < npoly; i++) {
					for (int j = 1; j < npoints[i]; j++)
						srcGraphics.drawLine(xpoints[i][j - 1], ypoints[i][j - 1], xpoints[i][j], ypoints[i][j]);
				}
			}
			srcGraphics.dispose();
			drawImageXOR(srcImage, 0, 0, bounds.width, bounds.height, bounds.x, bounds.y, bounds.width, bounds.height);
			srcImage.dispose();
		}
		else {
			graphics.setColor(new Color(color));
			if (fill) {
				graphics.fill(area);
			}
			else {
				for (int i = 0; i < npoly; i++) {
					for (int j = 1; j < npoints[i]; j++)
						graphics.drawLine(xpoints[i][j - 1], ypoints[i][j - 1], xpoints[i][j], ypoints[i][j]);
				}
			}
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

		if (pageQueue != null) {
			VCLGraphics.PageQueueItem pqi = new VCLGraphics.PageQueueItem(VCLGraphics.drawRectMethod, new Object[]{ new Integer(x), new Integer(y), new Integer(width), new Integer(height), new Integer(color), new Boolean(fill) });
			pageQueue.postDrawingOperation(pqi);
			return;
		}

		Rectangle bounds = new Rectangle(x, y, width, height);
		if (bounds.width < 0) {
			bounds.x += bounds.width;
			bounds.width *= -1;
		}
		if (bounds.height < 0) {
			bounds.y += bounds.height;
			bounds.height *= -1;
		}
		bounds.x -= 1;
		bounds.y -= 1;
		bounds.width += 2;
		bounds.height += 2;
		bounds = bounds.intersection(graphicsBounds);
		if (bounds.isEmpty())
			return;

		if (xor) {
			VCLImage srcImage = new VCLImage(bounds.width, bounds.height, bitCount);
			Graphics2D srcGraphics = srcImage.getImage().createGraphics();
			srcGraphics.setColor(new Color(color));
			srcGraphics.translate(bounds.x * -1, bounds.y * -1);
			if (fill)
				srcGraphics.fillRect(x, y, width, height);
			else
				srcGraphics.drawRect(x, y, width - 1, height - 1);
			srcGraphics.dispose();
			drawImageXOR(srcImage, 0, 0, bounds.width, bounds.height, bounds.x, bounds.y, bounds.width, bounds.height);
			srcImage.dispose();
		}
		else {
			graphics.setColor(new Color(color));
			if (fill)
				graphics.fillRect(x, y, width, height);
			else
				graphics.drawRect(x, y, width - 1, height - 1);
			addToFlush(bounds);
		}

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

		if (update != null && !update.isEmpty()) {
			if (image != null && frame != null) {
				BufferedImage i = image.getImage();
				Panel p = frame.getPanel();
				if (i != null && p != null) {
					synchronized(p) {
						Graphics2D g = (Graphics2D)p.getGraphics();
						if (g != null) {
							// Fix bug 553 by limiting clip to the graphics
							// bounds since the window may have been resized
							// since the graphics bounds were last calculated
							update = update.intersection(g.getDeviceConfiguration().getBounds());
							if (update != null && !update.isEmpty()) {
								g.setClip(update);
								g.drawRenderedImage(i, null);
								g.dispose();
								update = null;
							}
						}
					}
				}
			}
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
	 * Returns the font render context of the underlying graphics device.
	 *
	 * @return the font render context of the underlying graphics device
	 */
	FontRenderContext getFontRenderContext() {

		return graphics.getFontRenderContext();

	}

	/**
	 * Get the bounding rectangle of the specified glyph and font.
	 *
	 * @param glyph the glyph index
	 * @param font the font
	 * @param glyphOrientation the glyph rotation constant
	 */
	public Rectangle getGlyphBounds(int glyph, VCLFont font, int glyphOrientation) {

		GlyphVector glyphs = font.getFont().createGlyphVector(graphics.getFontRenderContext(), new int[]{ glyph });
		Rectangle2D bounds = glyphs.getVisualBounds();

		double fScaleX = font.getScaleX();
		if (fScaleX != 1.0) {
			if ((glyphOrientation & VCLGraphics.GF_ROTMASK) != 0)
				bounds = new Rectangle2D.Double(bounds.getX(), bounds.getY() * fScaleX, bounds.getWidth(), bounds.getHeight() * fScaleX);
			else
				bounds = new Rectangle2D.Double(bounds.getX() * fScaleX, bounds.getY(), bounds.getWidth() * fScaleX, bounds.getHeight());
		}

		return bounds.getBounds();

	}

	/**
	 * Returns the <code>VCLImage</code>.
	 *
	 * @return the <code>VCLImage</code>
	 */
	public VCLImage getImage() {

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

		if (pageFormat != null) {
			Dimension pageResolution = pageFormat.getPageResolution();
			if (rotatedPage)
				return new Dimension(pageResolution.height, pageResolution.width);
			else
				return new Dimension(pageResolution.width, pageResolution.height);
		}
		else {
			return new Dimension(VCLGraphics.screenResolution, VCLGraphics.screenResolution);
		}

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
				if (!clip.intersects(bounds)) {
					return;
				}
				else if (clip.contains((double)bounds.x, (double)bounds.y, (double)bounds.width, (double)bounds.height)) {
					clip = null;
				}
				else if (clip instanceof Rectangle) {
					bounds = bounds.intersection((Rectangle)clip);
					clip = null;
				}
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
		Rectangle bounds = polygon.getBounds();
		bounds.x -= 1;
		bounds.y -= 1;
		bounds.width += 2;
		bounds.height += 2;
		bounds = bounds.intersection(graphicsBounds);
		if (bounds.isEmpty())
			return;

		// Invert the image 
		if ((options & VCLGraphics.SAL_INVERT_TRACKFRAME) == VCLGraphics.SAL_INVERT_TRACKFRAME) {
			VCLImage srcImage = new VCLImage(bounds.width, bounds.height, bitCount);
			Graphics2D srcGraphics = srcImage.getImage().createGraphics();
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
			srcGraphics.setPaint(new TexturePaint(VCLGraphics.image50.getImage(), new Rectangle(0, 0, VCLGraphics.image50.getWidth(), VCLGraphics.image50.getHeight()).getBounds2D()));
			srcGraphics.translate(bounds.x * -1, bounds.y * -1);
			srcGraphics.fillPolygon(polygon);
			srcGraphics.dispose();
			drawImageXOR(srcImage, 0, 0, bounds.width, bounds.height, bounds.x, bounds.y, bounds.width, bounds.height);
			srcImage.dispose();
		}
		else {
			VCLImage srcImage = new VCLImage(bounds.width, bounds.height, bitCount);
			Graphics2D srcGraphics = srcImage.getImage().createGraphics();
			srcGraphics.translate(bounds.x * -1, bounds.y * -1);
			Shape clip = graphics.getClip();
			Area polygonClip = new Area(polygon);
			if (clip != null) {
				Area area = new Area(clip);
				area.intersect(polygonClip);
				srcGraphics.setClip(area);
			}
			else {
				srcGraphics.setClip(polygonClip);
			}
			srcGraphics.drawImage(image.getImage(), bounds.x, bounds.y, bounds.x + bounds.width, bounds.y + bounds.height, bounds.x, bounds.y, bounds.x + bounds.width, bounds.y + bounds.height, null);
			srcGraphics.dispose();

			int[] destData = srcImage.getData();
			int totalPixels = bounds.width * bounds.height;

			// Invert pixel
			for (int i = 0; i < totalPixels; i++) {
				if ((destData[i] & 0xff000000) == 0xff000000)
					destData[i] = ~destData[i] | 0xff000000;
			}

			drawImage(srcImage.getImage(), 0, 0, bounds.width, bounds.height, bounds.x, bounds.y, bounds.width, bounds.height);
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
	public void resetGraphics() {

		if (frame != null) {
			graphics.dispose();
			image.dispose();
			if (frame.getWindow().isShowing()) {
				Panel p = frame.getPanel();
				Rectangle bounds = p.getBounds();
				graphicsBounds = new Rectangle(0, 0, bounds.width, bounds.height);
			}
			else {
				graphicsBounds = new Rectangle(0, 0, 1, 1);
			}
			image = new VCLImage(graphicsBounds.width, graphicsBounds.height, frame.getBitCount());
			graphics = image.getImage().createGraphics();
			bitCount = image.getBitCount();
			update = null;
			resetClipRegion();
		}

	}

	/**
	 * Set the line drawing antialiasing attributes.
	 *
	 * @param b the line drawing antialiasing flag
	 */
	public void setLineAntialiasing(boolean b) {

		RenderingHints hints = graphics.getRenderingHints();
		if (b)
			hints.put(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
		else
			hints.put(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_OFF);
		graphics.setRenderingHints(hints);

	}

	/**
	 * Set the auto flush flag.
	 *
	 * @param b the auto flush flag 
	 */
	void setAutoFlush(boolean b) {

		if (b == autoFlush)
			return;

		autoFlush = b;
		nextAutoFlush = 0;

		if (autoFlush)
			autoFlush();
		else
			flush();

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

		VCLGraphics.PageQueueItem drawingHead = null;

		VCLGraphics.PageQueueItem drawingTail = null;

		VCLGraphics.PageQueueItem imageHead = null;

		VCLGraphics.PageQueueItem imageTail = null;

		Area pageImageClip = null;

		double pageImageScaleX = 1.0;

		double pageImageScaleY = 1.0;

		PageQueue(VCLGraphics g) {

			graphics = g;

		}

		void dispose() {

			graphics.pageQueue = null;
			Graphics2D g = graphics.graphics;

			// Draw the combined image
			if (imageHead != null && pageImageClip != null) {
				if (imageHead != imageTail) {
					Rectangle pageBounds = pageImageClip.getBounds();
					Rectangle destBounds = pageBounds.intersection(graphicsBounds);
					if (destBounds.isEmpty())
						return;
					Dimension maxResolution = graphics.getResolution();
					if (pageImageScaleX > maxResolution.width)
						pageImageScaleX = maxResolution.width;
					if (pageImageScaleY > maxResolution.height)
						pageImageScaleY = maxResolution.height;
					VCLImage pageImage = new VCLImage((int)(destBounds.width * pageImageScaleX), (int)(destBounds.height * pageImageScaleY), graphics.getBitCount());
					VCLGraphics pageGraphics = pageImage.getGraphics();
					pageGraphics.graphics.scale(pageImageScaleX, pageImageScaleY);
					pageGraphics.graphics.translate(destBounds.x * -1, destBounds.y * -1);
					pageGraphics.graphicsBounds = destBounds;
					while (imageHead != null) {
						pageGraphics.graphics.setClip(imageHead.clip);
						try {
							imageHead.method.invoke(pageGraphics, imageHead.params);
						}
						catch (Throwable t) {
							t.printStackTrace();
						}
						imageHead = imageHead.next;
					}
					pageGraphics.dispose();
					g.setClip(pageImageClip);
					g.drawImage(pageImage.getImage(), destBounds.x, destBounds.y, destBounds.x + destBounds.width, destBounds.y + destBounds.height, 0, 0, pageImage.getWidth(), pageImage.getHeight(), null);
					pageImage.dispose();
				}
				else {
					g.setClip(imageHead.clip);
					try {
						imageHead.method.invoke(graphics, imageHead.params);
					}
					catch (Throwable t) {
						t.printStackTrace();
					}
					imageHead = null;
				}
				imageTail = null;
			}

			// Invoke all of the queued drawing operations
			while (drawingHead != null) {
				if (pageImageClip != null) {
					Area area = new Area(pageImageClip);
					if (drawingHead.imageClip != null)
						area.subtract(drawingHead.imageClip);
					drawingHead.clip.subtract(area);
				}
				g.setClip(drawingHead.clip);
				try {
					drawingHead.method.invoke(graphics, drawingHead.params);
				}
				catch (Throwable t) {
					t.printStackTrace();
				}
				drawingHead = drawingHead.next;
			}
			drawingTail = null;
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
			if (drawingHead != null) {
				drawingTail.next = i;
				drawingTail = i;
			}
			else {
				drawingHead = drawingTail = i;
			}

		}

		void postImageOperation(VCLGraphics.PageQueueItem i, Rectangle b, double scaleX, double scaleY) {

			if (b == null || b.isEmpty())
				return;

			Graphics2D g = graphics.graphics;
			Area area = new Area(b);
			Shape clip = g.getClip();
			if (clip != null)
				area.intersect(new Area(clip));
			if (area.isEmpty())
				return;
			if (pageImageClip != null)
				pageImageClip.add(area);
			else
				pageImageClip = area;

			if (scaleX > pageImageScaleX)
				pageImageScaleX = scaleX;
			if (scaleY > pageImageScaleY)
				pageImageScaleY = scaleY;

			// Add the image operation to the queue
			if (clip != null)
				i.clip = new Area(clip);
			else
				i.clip = new Area(graphics.graphicsBounds);
			if (imageHead != null) {
				imageTail.next = i;
				imageTail = i;
			}
			else {
				imageHead = imageTail = i;
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
