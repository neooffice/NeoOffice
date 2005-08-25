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
import java.awt.Composite;
import java.awt.CompositeContext;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics2D;
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
import java.awt.image.ColorModel;
import java.awt.image.Raster;
import java.awt.image.WritableRaster;
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
	 * The cached copy composite.
	 */
	private static CopyComposite copyComposite = new CopyComposite();

	/**
	 * The drawBitmapMethod method.
	 */
	private static Method drawBitmapMethod = null;

	/**
	 * The drawBitmap2Method method.
	 */
	private static Method drawBitmap2Method = null;

	/**
	 * The drawGlyphs method.
	 */
	private static Method drawGlyphsMethod = null;

	/**
	 * The drawLine method.
	 */
	private static Method drawLineMethod = null;

	/**
	 * The drawMaskMethod method.
	 */
	private static Method drawMaskMethod = null;

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
	 * The hidden window image.
	 */
	private static VCLImage hiddenWindowImage = new VCLImage(1, 1, 32);

	/**
	 * The image50 image.
	 */
	private static VCLImage image50 = null;

	/**
	 * The cached invert composite.
	 */
	private static InvertComposite invertComposite = new InvertComposite();

	/**
	 * The cached mask composite.
	 */
	private static MaskComposite maskComposite = new MaskComposite();

	/**
	 * The cached screen resolution.
	 */
	private static int screenResolution = 0;

	/**
	 * The cached screen font resolution.
	 */
	private static int screenFontResolution = 0;

	/**
	 * The cached transparent composite.
	 */
	private static TransparentComposite transparentComposite = new TransparentComposite();

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
		int w = 2;
		int h = 2;
		VCLImage srcImage = new VCLImage(w, h, 1);
		BufferedImage img = srcImage.getImage();
		img.setRGB(0, 0, 0xff000000);
		img.setRGB(1, 0, 0xffffffff);
		img.setRGB(1, 1, 0xffffffff);
		img.setRGB(1, 1, 0xff000000);
		image50 = srcImage;

		// Set the screen and font resolutions
		screenResolution = screenFontResolution = Toolkit.getDefaultToolkit().getScreenResolution();
		if (screenResolution < VCLScreen.MIN_SCREEN_RESOLUTION)
			screenResolution = VCLScreen.MIN_SCREEN_RESOLUTION;
		if (screenFontResolution < VCLScreen.MIN_SCREEN_RESOLUTION)
			screenFontResolution = VCLScreen.MIN_SCREEN_RESOLUTION;

		// Set the method references
		try {
			drawBitmapMethod = VCLGraphics.class.getMethod("drawBitmap", new Class[]{ VCLBitmap.class, int.class, int.class, int.class, int.class, int.class, int.class, int.class, int.class });
		}
		catch (Throwable t) {
			t.printStackTrace();
		}
		try {
			drawBitmap2Method = VCLGraphics.class.getMethod("drawBitmap", new Class[]{ VCLBitmap.class, VCLBitmap.class, int.class, int.class, int.class, int.class, int.class, int.class, int.class, int.class });
		}
		catch (Throwable t) {
			t.printStackTrace();
		}
		try {
			drawGlyphsMethod = VCLGraphics.class.getMethod("drawGlyphs", new Class[]{ int.class, int.class, int[].class, int[].class, VCLFont.class, int.class, int.class, int.class, int.class, int.class });
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
			drawMaskMethod = VCLGraphics.class.getMethod("drawMask", new Class[]{ VCLBitmap.class, int.class, int.class, int.class, int.class, int.class, int.class, int.class, int.class, int.class });
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
	 * The rotated page flag.
	 */
	private boolean rotatedPage = false;

	/**
	 * The single pixel buffer.
	 */
	private BufferedImage singlePixelImage = null;

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
		resetGraphics();

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
		graphicsBounds = new Rectangle(0, 0, image.getWidth(), image.getHeight());
		pageFormat = p;
		bitCount = image.getBitCount();

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
		Rectangle bounds = pageFormat.getImageableBounds();
		if (rotatedPage)
			graphicsBounds = new Rectangle(0, 0, bounds.height, bounds.width);
		else
			graphicsBounds = new Rectangle(0, 0, bounds.width, bounds.height);
		graphics = (Graphics2D)g;
		bitCount = 32;

		// Mac OS X sometimes mangles images when multiple images are rendered
		// to a printer so we need to combine all images into one image and
		// defer other drawing operations until after the combined image is
		// created
		pageQueue = new VCLGraphics.PageQueue(this);

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
		graphics = null;
		image = null;
		frame = null;
		pageFormat = null;
		singlePixelImage = null;
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
	public void copyBits(VCLGraphics vg, int srcX, int srcY, int srcWidth, int srcHeight, int destX, int destY, int destWidth, int destHeight) {

		// No copy bits allowed for printing
		if (graphics != null)
			return;

		if (vg != this || srcWidth != destWidth || srcHeight != destHeight) {
			BufferedImage img = null;
			if (vg.getImage() != null)
				img = vg.getImage().getImage();

			if (img == null) {
				Rectangle srcBounds = new Rectangle(srcX, srcY, srcWidth, srcHeight).intersection(vg.getGraphicsBounds());
				if (srcBounds.isEmpty())
					return;

				img = vg.getImageFromFrame(srcBounds.x, srcBounds.y, srcBounds.width, srcBounds.height);
				if (img == null)
					return;

				srcX -= srcBounds.x;
				srcY -= srcBounds.y;

				Graphics2D g = getGraphics();
				if (g != null) {
					try {
						g.drawImage(img, destX, destY, destX + destWidth, destY + destHeight, srcX, srcY, srcX + srcWidth, srcY + srcHeight, null);
					}
					catch (Throwable t) {
						t.printStackTrace();
					}
				}
				g.dispose();
			}
			else {
				Graphics2D g = getGraphics();
				if (g != null) {
					try {
						g.drawImage(img, destX, destY, destX + destWidth, destY + destHeight, srcX, srcY, srcX + srcWidth, srcY + srcHeight, null);
					}
					catch (Throwable t) {
						t.printStackTrace();
					}
					g.dispose();
				}
			}
		}
		else {
			Graphics2D g = getGraphics();
			if (g != null) {
				try {
					g.copyArea(srcX, srcY, srcWidth, srcHeight, destX - srcX, destY - srcY);
				}
				catch (Throwable t) {
					t.printStackTrace();
				}
				g.dispose();
			}
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

		if (pageQueue != null) {
			VCLGraphics.PageQueueItem pqi = new VCLGraphics.PageQueueItem(VCLGraphics.drawBitmapMethod, new Object[]{ bmp, new Integer(srcX), new Integer(srcY), new Integer(srcWidth), new Integer(srcHeight), new Integer(destX), new Integer(destY), new Integer(destWidth), new Integer(destHeight) });
			pageQueue.postDrawingOperation(pqi);
			return;
		}

		Rectangle destBounds = new Rectangle(destX, destY, destWidth, destHeight).intersection(graphicsBounds);
		if (userClip != null)
			destBounds = destBounds.intersection(userClip.getBounds());
		if (destBounds.isEmpty())
			return;

		Graphics2D g = getGraphics();
		if (g != null) {
			try {
				g.drawImage(bmp.getImage(), destX, destY, destX + destWidth, destY + destHeight, srcX, srcY, srcX + srcWidth, srcY + srcHeight, null);
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			g.dispose();
		}

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
		if (pageQueue != null) {
			VCLGraphics.PageQueueItem pqi = new VCLGraphics.PageQueueItem(VCLGraphics.drawBitmap2Method, new Object[]{ bmp, transBmp, new Integer(srcX), new Integer(srcY), new Integer(srcWidth), new Integer(srcHeight), new Integer(destX), new Integer(destY), new Integer(destWidth), new Integer(destHeight) });
			pageQueue.postDrawingOperation(pqi);
			return;
		}

		Rectangle destBounds = new Rectangle(destX, destY, destWidth, destHeight).intersection(graphicsBounds);
		if (userClip != null)
			destBounds = destBounds.intersection(userClip.getBounds());
		if (destBounds.isEmpty())
			return;

		VCLImage mergedImage = new VCLImage(destBounds.width, destBounds.height, bitCount);
		Graphics2D mergedGraphics = mergedImage.getGraphics().getGraphics();
		if (mergedGraphics != null) {
			try {
				mergedGraphics.setComposite(VCLGraphics.transparentComposite);
				VCLGraphics.transparentComposite.setFirstPass(true);
				mergedGraphics.translate(destBounds.x * -1, destBounds.y * -1 );
				mergedGraphics.drawImage(transBmp.getImage(), destX, destY, destX + destWidth, destY + destHeight, srcX, srcY, srcX + srcWidth, srcY + srcHeight, null);
				VCLGraphics.transparentComposite.setFirstPass(false);
				mergedGraphics.drawImage(bmp.getImage(), destX, destY, destX + destWidth, destY + destHeight, srcX, srcY, srcX + srcWidth, srcY + srcHeight, null);
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			mergedGraphics.dispose();
		}

		Graphics2D g = getGraphics();
		if (g != null) {
			try {
				g.drawImage(mergedImage.getImage(), destBounds.x, destBounds.y, destBounds.x + destBounds.width, destBounds.y + destBounds.height, 0, 0, destBounds.width, destBounds.height, null);
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			g.dispose();
		}

		mergedImage.dispose();

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

		Graphics2D g = getGraphics();
		if (g != null) {
			try {
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
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			g.dispose();
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

		Graphics2D g = getGraphics();
		if (g != null) {
			try {
				if (xor)
					g.setXORMode(color == 0xffffffff ? Color.black : Color.white);
				g.setColor(new Color(color));
				g.drawLine(x1, y1, x2, y2);
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			g.dispose();
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

		if (pageQueue != null) {
			VCLGraphics.PageQueueItem pqi = new VCLGraphics.PageQueueItem(VCLGraphics.drawMaskMethod, new Object[]{ bmp, new Integer(color), new Integer(srcX), new Integer(srcY), new Integer(srcWidth), new Integer(srcHeight), new Integer(destX), new Integer(destY), new Integer(destWidth), new Integer(destHeight) });
			pageQueue.postDrawingOperation(pqi);
			return;
		}

		Rectangle destBounds = new Rectangle(destX, destY, destWidth, destHeight).intersection(graphicsBounds);
		if (userClip != null)
			destBounds = destBounds.intersection(userClip.getBounds());
		if (destBounds.isEmpty())
			return;

		Graphics2D g = getGraphics();
		if (g != null) {
			try {
				g.setComposite(VCLGraphics.maskComposite);
				VCLGraphics.maskComposite.setMaskColor(color);
				g.drawImage(bmp.getImage(), destX, destY, destX + destWidth, destY + destHeight, srcX, srcY, srcX + srcWidth, srcY + srcHeight, null);
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			g.dispose();
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

		Graphics2D g = getGraphics();
		if (g != null) {
			try {
				if (xor)
					g.setXORMode(color == 0xffffffff ? Color.black : Color.white);
				g.setColor(new Color(color));
				if (fill) {
					g.fillPolygon(polygon);
				}
				else {
					for (int i = 1; i < npoints; i++)
						g.drawLine(xpoints[i - 1], ypoints[i - 1], xpoints[i], ypoints[i]);
				}
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			g.dispose();
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

		Graphics2D g = getGraphics();
		if (g != null) {
			try {
				if (xor)
					g.setXORMode(color == 0xffffffff ? Color.black : Color.white);
				g.setColor(new Color(color));
				for (int i = 1; i < npoints; i++)
					g.drawLine(xpoints[i - 1], ypoints[i - 1], xpoints[i], ypoints[i]);
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			g.dispose();
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

		if (npoly == 0)
			return;

		// Fix bug 786 by drawing overlapping polygons in XOR mode
		if (xor) {
			for (int i = 0; i < npoly; i++)
				drawPolygon(npoints[i], xpoints[i], ypoints[i], color, fill);
			return;
		}

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

		Graphics2D g = getGraphics();
		if (g != null) {
			try {
				g.setColor(new Color(color));
				if (fill) {
					g.fill(area);
				}
				else {
					for (int i = 0; i < npoly; i++) {
						for (int j = 1; j < npoints[i]; j++)
							g.drawLine(xpoints[i][j - 1], ypoints[i][j - 1], xpoints[i][j], ypoints[i][j]);
					}
				}
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			g.dispose();
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

		Graphics2D g = getGraphics();
		if (g != null) {
			try {
				if (xor)
					g.setXORMode(color == 0xffffffff ? Color.black : Color.white);
				g.setColor(new Color(color));
				if (fill)
					g.fillRect(x, y, width, height);
				else
					g.drawRect(x, y, width - 1, height - 1);
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			g.dispose();
		}

	}

	/**
	 * Applies the cached clipping area. The cached clipping area is set using
	 * the {@link #beginSetClipRegion()} and the
	 * {@link #unionClipRegion(long, long, long, long)} methods.
	 */
	public void endSetClipRegion() {}

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

		Rectangle2D bounds = null;

		Graphics2D g = getGraphics();
		if (g != null) {
			try {
				GlyphVector glyphs = font.getFont().createGlyphVector(g.getFontRenderContext(), new int[]{ glyph });
				bounds = glyphs.getVisualBounds();
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			g.dispose();
		}

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
	 * Returns the graphics context.
	 *
	 * @return the graphics context
	 */
	Graphics2D getGraphics() {

		// Don't bother painting if we haven't attached to the panel yet
		if (frame != null && image != null)
			return null;

		Graphics2D g;
		if (image != null)
			g = image.getImage().createGraphics();
		else if (frame != null)
			g = (Graphics2D)frame.getPanel().getGraphics();
		else if (graphics != null)
			g = (Graphics2D)graphics.create();
		else
			return null;

		g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_OFF);
		g.setClip(userClip);

		return g;

	}

	/**
	 * Returns the graphics bounds.
	 *
	 * @return the graphics bounds
	 */
	Rectangle getGraphicsBounds() {

		return graphicsBounds;

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
	 * Returns a <code>VCLImage</code> of the frame's pixels.
	 *
	 * @param x the x coordinate of the rectangle
	 * @param y the y coordinate of the rectangle
	 * @param width the width of the rectangle
	 * @param height the height of the rectangle
	 * @return a <code>BufferedImage</code> of the frame's pixels
	 */
	public BufferedImage getImageFromFrame(int x, int y, int w, int h) {

		if (frame == null)
			return null;

		BufferedImage img = null;

		Graphics2D g = getGraphics();
		if (g != null) {
			try {
				img = g.getDeviceConfiguration().createCompatibleImage(w, h);
				g.setComposite(VCLGraphics.copyComposite);
				g.setClip(null);
				VCLGraphics.copyComposite.setRaster(img.getRaster());
				g.drawImage(img, x, y, x + w, y + h, 0, 0, w, h, null);
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			g.dispose();
		}

		return img;

	}

	/**
	 * Returns the pixel color in ARGB format for the specified coordinate.
	 *
	 * @param x the x coordinate of the source rectangle
	 * @param y the y coordinate of the source rectangle
	 * @return the pixel color in ARGB format
	 */
	public int getPixel(int x, int y) {

		if (graphics != null || !graphicsBounds.contains(x, y) || (userClip != null && !userClip.contains(x, y)))
			return 0x00000000;

		if (image != null) {
			return image.getImage().getRGB(x, y);
		}
		else {
			int pixel = 0x00000000;

			Graphics2D g = getGraphics();
			if (g != null) {
				try {
					g.setComposite(VCLGraphics.copyComposite);
					g.setClip(null);
					if (singlePixelImage == null)
						singlePixelImage = g.getDeviceConfiguration().createCompatibleImage(1, 1);
					VCLGraphics.copyComposite.setRaster(singlePixelImage.getRaster());
					g.drawImage(singlePixelImage, x, y, x + 1, y + 1, 0, 0, 1, 1, null);
					pixel = singlePixelImage.getRGB(0, 0);
				}
				catch (Throwable t) {
					t.printStackTrace();
				}
				g.dispose();
			}

			return pixel;
		}

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

		// No inverting allowed for printing
		if (graphics != null)
			return;

		// Invert the image 
		if ((options & VCLGraphics.SAL_INVERT_TRACKFRAME) == VCLGraphics.SAL_INVERT_TRACKFRAME) {
			Graphics2D g = getGraphics();
			if (g != null) {
				try {
					BasicStroke stroke = (BasicStroke)g.getStroke();
					g.setStroke(new BasicStroke(stroke.getLineWidth(), BasicStroke.CAP_BUTT, BasicStroke.JOIN_MITER, stroke.getMiterLimit(), new float[]{ 1.0f, 1.0f }, 0.0f));
					g.setXORMode(Color.white);
					g.setColor(Color.black);
					// Note: the JVM seems to have a bug and drawRect() draws
					// dashed strokes one pixel above the specified y coordinate
					g.drawRect(x, y + 1, width - 1, height - 1);
				}
				catch (Throwable t) {
					t.printStackTrace();
				}
				g.dispose();
			}
		}
		else if ((options & VCLGraphics.SAL_INVERT_50) == VCLGraphics.SAL_INVERT_50) {
			Graphics2D g = getGraphics();
			if (g != null) {
				try {
					g.setXORMode(Color.white);
					g.setPaint(new TexturePaint(VCLGraphics.image50.getImage(), new Rectangle(0, 0, VCLGraphics.image50.getWidth(), VCLGraphics.image50.getHeight()).getBounds2D()));
					g.fillRect(x, y, width, height);
				}
				catch (Throwable t) {
					t.printStackTrace();
				}
				g.dispose();
			}
			
		}
		else {
			Graphics2D g = getGraphics();
			if (g != null) {
				try {
					g.setComposite(VCLGraphics.invertComposite);
					g.clipRect(x, y, width, height);
					g.fillRect(x, y, width, height);
				}
				catch (Throwable t) {
					t.printStackTrace();
				}
				g.dispose();
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

		// No inverting allowed for printing
		if (graphics != null)
			return;

		Polygon polygon = new Polygon(xpoints, ypoints, npoints);

		// Invert the image 
		if ((options & VCLGraphics.SAL_INVERT_TRACKFRAME) == VCLGraphics.SAL_INVERT_TRACKFRAME) {
			Graphics2D g = getGraphics();
			if (g != null) {
				try {
					BasicStroke stroke = (BasicStroke)g.getStroke();
					g.setStroke(new BasicStroke(stroke.getLineWidth(), BasicStroke.CAP_BUTT, BasicStroke.JOIN_MITER, stroke.getMiterLimit(), new float[]{ 1.0f, 1.0f }, 0.0f));
					g.setXORMode(Color.white);
					g.setColor(Color.black);
					for (int i = 1; i < npoints; i++)
						g.drawLine(xpoints[i - 1], ypoints[i - 1], xpoints[i], ypoints[i]);
				}
				catch (Throwable t) {
					t.printStackTrace();
				}
				g.dispose();
			}
		}
		else if ((options & VCLGraphics.SAL_INVERT_50) == VCLGraphics.SAL_INVERT_50) {
			Graphics2D g = getGraphics();
			if (g != null) {
				try {
					g.setXORMode(Color.white);
					g.setPaint(new TexturePaint(VCLGraphics.image50.getImage(), new Rectangle(0, 0, VCLGraphics.image50.getWidth(), VCLGraphics.image50.getHeight()).getBounds2D()));
					g.fillPolygon(polygon);
				}
				catch (Throwable t) {
					t.printStackTrace();
				}
				g.dispose();
			}
		}
		else {
			Graphics2D g = getGraphics();
			if (g != null) {
				try {
					g.setComposite(VCLGraphics.invertComposite);
					g.clip(polygon);
					g.fillPolygon(polygon);
				}
				catch (Throwable t) {
					t.printStackTrace();
				}
				g.dispose();
			}
		}

	}

	/**
	 * Resets the applied clipping area.
	 */
	public void resetClipRegion() {

		userClip = null;

	}

	/**
	 * Resets the underlying graphics context.
	 */
	public void resetGraphics() {

		if (frame != null) {
			if (image != null)
				image = null;
			if (frame.getWindow().isShowing()) {
				Panel p = frame.getPanel();
				Rectangle bounds = p.getBounds();
				graphicsBounds = new Rectangle(0, 0, bounds.width, bounds.height);
				bitCount = frame.getBitCount();
			}
			else {
				image = hiddenWindowImage;
				graphicsBounds = new Rectangle(0, 0, image.getWidth(), image.getHeight());
				bitCount = image.getBitCount();
			}
			resetClipRegion();
		}

	}

	/**
	 * Set the pixel color for the specified coordinate.
	 *
	 * @param x the x coordinate of the source rectangle
	 * @param y the y coordinate of the source rectangle
	 * @param color the color of the pixel 
	 */
	public void setPixel(int x, int y, int color) {

		if (!graphicsBounds.contains(x, y) || (userClip != null && !userClip.contains(x, y)))
			return;

		if (!xor && image != null) {
			image.getImage().setRGB(x, y, color);
		}
		else {
			Graphics2D g = getGraphics();
			if (g != null) {
				try {
					if (xor)
						g.setXORMode(color == 0xffffffff ? Color.black : Color.white);
					if (singlePixelImage == null)
						singlePixelImage = g.getDeviceConfiguration().createCompatibleImage(1, 1);
					singlePixelImage.setRGB(0, 0, color);
					g.drawImage(singlePixelImage, x, y, x + 1, y + 1, 0, 0, 1, 1, null);
				}
				catch (Throwable t) {
					t.printStackTrace();
				}
				g.dispose();
			}
		}

	}

	/**
	 * Enables or disables drawing in XOR mode.
	 *
	 * @param b <code>true</code> to enable XOR mode and <code>false</code>
	 *  to disable it
	 */
	public void setXORMode(boolean b) {

		// XORing is not allowed for printing
		if (graphics == null)
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

		VCLGraphics.PageQueueItem tail = null;

		PageQueue(VCLGraphics g) {

			graphics = g;

		}

		void dispose() {

			graphics.pageQueue = null;

			// Invoke all of the queued drawing operations
			while (head != null) {
				graphics.userClip = head.clip;
				try {
					head.method.invoke(graphics, head.params);
				}
				catch (Throwable t) {
					t.printStackTrace();
				}
				head = head.next;
			}
			tail = null;
			graphics = null;

		}

		void postDrawingOperation(VCLGraphics.PageQueueItem i) {

			// Add the drawing operation to the queue
			if (userClip != null)
				i.clip = new Area(userClip);
			else
				i.clip = null;
			if (head != null) {
				tail.next = i;
				tail = i;
			}
			else {
				head = tail = i;
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

	final static class CopyComposite implements Composite, CompositeContext {

		private WritableRaster raster = null;

		public void compose(Raster src, Raster destIn, WritableRaster destOut) {

			destOut.setRect(destIn);
			raster.setRect(destIn);

			raster = null;

		}

		public CompositeContext createContext(ColorModel srcColorModel, ColorModel destColorModel, RenderingHints hints) {

			return this;

		}

		public void dispose() {}

		void setRaster(WritableRaster r) {

			raster = r;

		}

	}

	final static class InvertComposite implements Composite, CompositeContext {

		public void compose(Raster src, Raster destIn, WritableRaster destOut) {

			int w = destOut.getWidth();
			int h = destOut.getHeight();
			int[] data = new int[w];
			for (int line = 0; line < h; line++) {
				data = (int[])destIn.getDataElements(0, line, data.length, 1, data);
				for (int i = 0; i < data.length; i++)
					data[i] = ~data[i] | 0xff000000;
				destOut.setDataElements(0, line, data.length, 1, data);
			}

		}

		public CompositeContext createContext(ColorModel srcColorModel, ColorModel destColorModel, RenderingHints hints) {

			return this;

		}

		public void dispose() {}

	}

	final static class MaskComposite implements Composite, CompositeContext {

		private int maskColor;

		public void compose(Raster src, Raster destIn, WritableRaster destOut) {

			int w = destOut.getWidth();
			int h = destOut.getHeight();
			int[] srcData = new int[w];
			int[] destData = new int[w];
			for (int line = 0; line < h; line++) {
				srcData = (int[])src.getDataElements(0, line, srcData.length, 1, srcData);
				destData = (int[])destIn.getDataElements(0, line, destData.length, 1, destData);
				for (int i = 0; i < srcData.length && i < destData.length; i++) {
					// If the pixel is black, set the pixel to the mask color
					if (srcData[i] == 0xff000000)
						destData[i] = maskColor;
				}
				destOut.setDataElements(0, line, destData.length, 1, destData);
			}

		}

		public CompositeContext createContext(ColorModel srcColorModel, ColorModel destColorModel, RenderingHints hints) {

			return this;

		}

		public void dispose() {}

		void setMaskColor(int c) {

			maskColor = c;

		}

	}

	final static class TransparentComposite implements Composite, CompositeContext {

		private boolean firstPass = true;

		public void compose(Raster src, Raster destIn, WritableRaster destOut) {

			int w = destOut.getWidth();
			int h = destOut.getHeight();

			if (firstPass)
			{
				int[] data = new int[w];
				for (int line = 0; line < h; line++) {
					data = (int[])src.getDataElements(0, line, data.length, 1, data);
					// Only copy black pixels from the source.
					for (int i = 0; i < data.length; i++) {
						if (data[i] != 0xff000000)
							data[i] = 0x00000000;
					}
					destOut.setDataElements(0, line, data.length, 1, data);
				}
			}
			else {
				int[] srcData = new int[w];
				int[] destData = new int[w];
				for (int line = 0; line < h; line++) {
					srcData = (int[])src.getDataElements(0, line, srcData.length, 1, srcData);
					destData = (int[])destIn.getDataElements(0, line, destData.length, 1, destData);
					// Only copy pixel if the pixel in the destination is not
					// transparent,
					for (int i = 0; i < srcData.length && i < destData.length; i++) {
						if (destData[i] != 0x00000000)
							destData[i] = srcData[i];
					}
					destOut.setDataElements(0, line, destData.length, 1, destData);
				}
			}

		}

		public CompositeContext createContext(ColorModel srcColorModel, ColorModel destColorModel, RenderingHints hints) {

			return this;

		}

		public void dispose() {}

		void setFirstPass(boolean b) {

			firstPass = b;

		}

	}

}
