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

import java.awt.AWTException;
import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Composite;
import java.awt.CompositeContext;
import java.awt.Dimension;
import java.awt.Font;
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
import java.awt.Window;
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
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.StringTokenizer;
import javax.swing.ButtonModel;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JRadioButton;

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
	 * The BUTTONVALUE_DONTKNOW constant.
	 */
	public final static int BUTTONVALUE_DONTKNOW = 0;

	/**
	 * The BUTTONVALUE_ON constant.
	 */
	public final static int BUTTONVALUE_ON = 1;

	/**
	 * The BUTTONVALUE_OFF constant.
	 */
	public final static int BUTTONVALUE_OFF = 2;

	/**
	 * The BUTTONVALUE_MIXED constant.
	 */
	public final static int BUTTONVALUE_MIXED = 3;

	/**
	 * The button component.
	 */
	private static DefaultableJButton button = null;

	/**
	 * The cached copy composite.
	 */
	private static CopyComposite copyComposite = new CopyComposite();

	/**
	 * The cached create copy composite.
	 */
	private static CreateCopyComposite createCopyComposite = new CreateCopyComposite();

	/**
	 * The drawBitmapMethod method.
	 */
	private static Method drawBitmapMethod = null;

	/**
	 * The drawBitmapBufferMethod method.
	 */
	private static Method drawBitmapBufferMethod = null;

	/**
	 * The drawCheckBox method.
	 */
	private static Method drawCheckBoxMethod = null;

	/**
	 * The drawEPSMethod method.
	 */
	private static Method drawEPSMethod = null;

	/**
	 * The drawGlyphBuffer method.
	 */
	private static Method drawGlyphBufferMethod = null;

	/**
	 * The drawLine method.
	 */
	private static Method drawLineMethod = null;

	/**
	 * The draw on main thread flag.
	 */
	private static boolean drawOnMainThread = true;

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
	 * The drawPushButton method.
	 */
	private static Method drawPushButtonMethod = null;

	/**
	 * The drawRadioButton method.
	 */
	private static Method drawRadioButtonMethod = null;

	/**
	 * The drawRect method.
	 */
	private static Method drawRectMethod = null;

	/**
	 * The setPixel method.
	 */
	private static Method setPixelMethod = null;

	/**
	 * The image50 image.
	 */
	private static VCLImage image50 = null;

	/**
	 * The cached invert composite.
	 */
	private static InvertComposite invertComposite = new InvertComposite();

	/**
	 * The needs dispose graphics.
	 */
	private static VCLGraphics needsDisposeGraphics = null;

	/**
	 * The radio button component.
	 */
	private static JRadioButton radioButton = null;

	/**
	 * The radio button x coordinate offset.
	 */
	private static int radioButtonXOffset = 0;

	/**
	 * The radio button y coordinate offset.
	 */
	private static int radioButtonYOffset = 0;

	/**
	 * The radio button preferred size.
	 */
	private final static Dimension radioButtonPreferredSize = new Dimension(16, 16);

	/**
	 * The checkbox component.
	 */
	private static JCheckBox checkBoxButton = null;

	/**
	 * The checkbox x coordinate offset.
	 */
	private static int checkBoxButtonXOffset = 0;

	/**
	 * The checkbox y coordinate offset.
	 */
	private static int checkBoxButtonYOffset = 0;

	/**
	 * The checkbox preferred size.
	 */
	private final static Dimension checkBoxButtonPreferredSize = new Dimension(16, 16);

	/**
	 * The cached printer resolution.
	 */
	private static Dimension printerResolution = null;

	/**
	 * The cached screen resolution.
	 */
	private static Dimension screenResolution = null;

	/**
	 * The cached XOR image composite.
	 */
	private static XORImageComposite xorImageComposite = new XORImageComposite();

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
	 * Dispose the needs dispose graphics.
	 */
	static void disposeNeedsDisposeGraphics() {

		if (needsDisposeGraphics != null) {
			if (needsDisposeGraphics.image != null)
				needsDisposeGraphics.image.dispose();
			else
				needsDisposeGraphics.dispose();
			needsDisposeGraphics = null;
		}

	}

	/**
	 * Set the needs dispose graphics.
	 *
	 * @param disposeGraphics the needs dispose graphics
	 * @return <code>true</code> if the need dispose graphics has been set
	 *  to the specified graphics otherwise <code>false</false>
	 */
	static boolean setNeedsDisposeGraphics(VCLGraphics disposeGraphics) {

		// Fix bug 3189 by always returning false if both graphics are the same
		if (disposeGraphics == needsDisposeGraphics || disposeGraphics == null || disposeGraphics.image == null || disposeGraphics.disposed || disposeGraphics.changeListeners == null)
			return false;

		disposeNeedsDisposeGraphics();
		needsDisposeGraphics = disposeGraphics;
		return true;

	}

	/**
	 * Releases any native bitmaps that were created while printing.
	 *
	 * @param b <code>true</code> if the print job is complete otherwise
	 *  <code>false</code>
	 */
	public static native void releaseNativeBitmaps(boolean b);

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

		// Determine OS version
		StringTokenizer tokenizer = new StringTokenizer(System.getProperty("os.version"), ".");
		int osVersion = 0;
		if (tokenizer.hasMoreTokens()) {
			try {
				osVersion |= ( Integer.valueOf(tokenizer.nextToken()).intValue() & 0x000000ff ) << 16;
			}
			catch (Throwable t) {}
		}
		if (tokenizer.hasMoreTokens()) {
			try {
				osVersion |= ( Integer.valueOf(tokenizer.nextToken()).intValue() & 0x000000ff ) << 8;
			}
			catch (Throwable t) {}
		}
		if (tokenizer.hasMoreTokens()) {
			try {
				osVersion |= Integer.valueOf(tokenizer.nextToken()).intValue() & 0x000000ff;
			}
			catch (Throwable t) {}
		}

		// Set the draw on main thread flag
		try {
			// Test for Java 1.5 or higher
			Class.forName("java.lang.Appendable");
			drawOnMainThread = false;
		}
		catch (Throwable t) {}

		Color c = new Color(0x00000000, true);

		// Create the button with a transparent background
		button = new DefaultableJButton();
		button.setBackground(c);

		radioButton = new JRadioButton();
		radioButton.setBackground(c);

		checkBoxButton = new JCheckBox();
		checkBoxButton.setBackground(c);

		// Adjust offsets for Swing controls
		if (osVersion < 0x000a0505) {
			VCLGraphics.radioButtonXOffset = 0;
			VCLGraphics.radioButtonYOffset = -2;
			VCLGraphics.checkBoxButtonXOffset = 0;
			VCLGraphics.checkBoxButtonYOffset = -1;
		}
		else {
			VCLGraphics.radioButtonXOffset = -3;
			VCLGraphics.radioButtonYOffset = -1;
			VCLGraphics.checkBoxButtonXOffset = -3;
			VCLGraphics.checkBoxButtonYOffset = 0;
		}

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

		// Fix bug 3051 by setting the printer resolution to twips
		printerResolution = new Dimension(1440, 1440);

		// Set screen resolutions
		int resolution = Toolkit.getDefaultToolkit().getScreenResolution();
		if (resolution < VCLScreen.MIN_SCREEN_RESOLUTION)
			resolution = VCLScreen.MIN_SCREEN_RESOLUTION;
		screenResolution = new Dimension(resolution, resolution);

		// Set the method references
		try {
			drawBitmapMethod = VCLGraphics.class.getMethod("drawBitmap", new Class[]{ VCLBitmap.class, int.class, int.class, int.class, int.class, int.class, int.class, int.class, int.class, long.class });
		}
		catch (Throwable t) {
			t.printStackTrace();
		}
		try {
			drawBitmapBufferMethod = VCLGraphics.class.getMethod("drawBitmapBuffer", new Class[]{ long.class, int.class, int.class, int.class, int.class, int.class, int.class, int.class, int.class, long.class });
		}
		catch (Throwable t) {
			t.printStackTrace();
		}
		try {
			drawCheckBoxMethod = VCLGraphics.class.getMethod("drawCheckBox", new Class[]{ int.class, int.class, int.class, int.class, String.class, boolean.class, boolean.class, boolean.class, int.class });
		}
		catch (Throwable t) {
			t.printStackTrace();
		}
		try {
			drawGlyphBufferMethod = VCLGraphics.class.getMethod("drawGlyphBuffer", new Class[]{ int.class, int.class, int.class, long.class, long.class, int.class, int.class, double.class, int.class, int.class, int.class, int.class, int.class, float.class, long.class });
		}
		catch (Throwable t) {
			t.printStackTrace();
		}
		try {
			drawEPSMethod = VCLGraphics.class.getMethod("drawEPS", new Class[]{ long.class, long.class, int.class, int.class, int.class, int.class, long.class });
		}
		catch (Throwable t) {
			t.printStackTrace();
		}
		try {
			drawLineMethod = VCLGraphics.class.getMethod("drawLine", new Class[]{ int.class, int.class, int.class, int.class, int.class, long.class });
		}
		catch (Throwable t) {
			t.printStackTrace();
		}
		try {
			drawPolygonMethod = VCLGraphics.class.getMethod("drawPolygon", new Class[]{ int.class, int[].class, int[].class, int.class, boolean.class, long.class });
		}
		catch (Throwable t) {
			t.printStackTrace();
		}
		try {
			drawPolylineMethod = VCLGraphics.class.getMethod("drawPolyline", new Class[]{ int.class, int[].class, int[].class, int.class, long.class });
		}
		catch (Throwable t) {
			t.printStackTrace();
		}
		try {
			drawPolyPolygonMethod = VCLGraphics.class.getMethod("drawPolyPolygon", new Class[]{ int.class, int[].class, int[][].class, int[][].class, int.class, boolean.class, long.class });
		}
		catch (Throwable t) {
			t.printStackTrace();
		}
		try {
			drawPushButtonMethod = VCLGraphics.class.getMethod("drawPushButton", new Class[]{ int.class, int.class, int.class, int.class, String.class, boolean.class, boolean.class, boolean.class, boolean.class });
		}
		catch (Throwable t) {
			t.printStackTrace();
		}
		try {
			drawRadioButtonMethod = VCLGraphics.class.getMethod("drawRadioButton", new Class[]{ int.class, int.class, int.class, int.class, String.class, boolean.class, boolean.class, boolean.class, int.class });
		}
		catch (Throwable t) {
			t.printStackTrace();
		}
		try {
			drawRectMethod = VCLGraphics.class.getMethod("drawRect", new Class[]{ int.class, int.class, int.class, int.class, int.class, boolean.class, long.class });
		}
		catch (Throwable t) {
			t.printStackTrace();
		}
		try {
			setPixelMethod = VCLGraphics.class.getMethod("setPixel", new Class[]{ int.class, int.class, int.class, long.class });
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
	 * The change listeners.
	 */
	private LinkedList changeListeners = null;

	/**
	 * The disposed flag.
	 */
	private boolean disposed = false;

	/**
	 * The frame that the graphics draws to.
	 */
	private VCLFrame frame = null;

	/**
	 * The frame's cached clipping area.
	 */
	private Area frameClip = null;

	/**
	 * The frame's cached clipping list.
	 */
	private LinkedList frameClipList = null;

	/**
	 * The frame's polygon clipping flag.
	 */
	private boolean framePolygonClip = false;

	/**
	 * The graphics context.
	 */
	private Graphics2D graphics = null;

	/**
	 * The graphics's cached clipping area.
	 */
	private Area graphicsClip = null;

	/**
	 * The graphics's cached clipping list.
	 */
	private LinkedList graphicsClipList = null;

	/**
	 * The graphics's polygon clipping flag.
	 */
	private boolean graphicsPolygonClip = false;

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
	 * The horizontal scale factor.
	 */
	private float pageScaleX = 1.0f;

	/**
	 * The vertical scale factor.
	 */
	private float pageScaleY = 1.0f;

	/**
	 * The rotate page angle.
	 */
	private float rotatedPageAngle = 0.0f;

	/**
	 * The cached clipping area.
	 */
	private Area userClip = null;

	/**
	 * The cached clipping list.
	 */
	private LinkedList userClipList = null;

	/**
	 * The polygon clipping flag.
	 */
	private boolean userPolygonClip = false;

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
	 * @param r the rotation angle
	 * @param x the horizontal scale factor
	 * @param y the vertical scale factor
     */
    VCLGraphics(Graphics2D g, VCLPageFormat p, float r, float x, float y) {

		pageFormat = p;
		rotatedPageAngle = r;
		Rectangle bounds = pageFormat.getImageableBounds();
		// Fix bug 3079 by not rotating the bounds as they are already
		// rotated by the VCLPageFormat class. Fix bug 3090 by only applying
		// the fix when the page is portrait.
		if (p.getOrientation() == VCLPageFormat.ORIENTATION_PORTRAIT)
			graphicsBounds = new Rectangle(0, 0, bounds.width, bounds.height);
		else
			graphicsBounds = new Rectangle(0, 0, bounds.height, bounds.width);
		pageScaleX = x;
		pageScaleY = y;
		graphics = (Graphics2D)g;
		bitCount = 24;

		// Mac OS X sometimes mangles images when multiple images are rendered
		// to a printer so we need to combine all images into one image and
		// defer other drawing operations until after the combined image is
		// created
		pageQueue = new VCLGraphics.PageQueue(this);

	}

	/**
	 * Adds a change listener.
	 *
	 * @param listener a pointer to a C++ JavaSalBitmap instance
	 */
	public void addGraphicsChangeListener(long listener) {

		// No listeners allowed for printing
		if (graphics != null) {
			notifyGraphicsChanged(new long[]{ listener }, false);
			return;
		}

		if (changeListeners == null)
			changeListeners = new LinkedList();
		changeListeners.add(new Long(listener));

	}

	/**
	 * Sets the cached clipping area to an empty area. The cached clipping
	 * area is not actually applied until the {@link #endSetClipRegion(boolean)}
	 * method is called.
	 *
	 * @param b <code>true</code> if this clip is coming from the C++ SalFrame
	 *  methods otherwise <code>false</code>
	 */
	public void beginSetClipRegion(boolean b) {

		if (graphics != null)
			return;

		resetClipRegion(b);

	}

	/**
	 * Returns a <code>VCLImage</code> of the frame's pixels.
	 *
	 * @param x the x coordinate of the rectangle
	 * @param y the y coordinate of the rectangle
	 * @param width the width of the rectangle
	 * @param height the height of the rectangle
	 * @return a <code>VCLImage</code> of the frame's pixels
	 */
	VCLImage createImage(int x, int y, int width, int height) {

		if (graphics != null)
			return null;

		Rectangle destBounds = new Rectangle(x, y, width, height).intersection(graphicsBounds);
		if (destBounds.isEmpty())
			return null;

		VCLImage img = null;

		Graphics2D g = getGraphics(false);
		if (g != null) {
			try {
				g.setComposite(VCLGraphics.createCopyComposite);
				g.setClip(destBounds.x, destBounds.y, destBounds.width, destBounds.height);
				g.fillRect(destBounds.x, destBounds.y, destBounds.width, destBounds.height);
				img = new VCLImage(VCLGraphics.createCopyComposite.getRaster());
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			g.dispose();
		}

		return img;

	}

	/**
	 * Disposes the underlying graphics and releases any system resources that
	 * it is using.
	 */
	boolean dispose() {

		if (disposed || VCLGraphics.setNeedsDisposeGraphics(this))
			return false;

		// Prevent recursion if this is the need dispose graphics
		disposed = true;

		notifyGraphicsChanged();
		changeListeners = null;
		if (pageQueue != null)
			pageQueue.drawOperations();
		pageQueue = null;
		graphics = null;
		image = null;
		frame = null;
		frameClip = null;
		frameClipList = null;
		graphicsClip = null;
		graphicsClipList = null;
		pageFormat = null;
		userClip = null;
		userClipList = null;

		return true;

	}

	/**
	 * Draws specified <code>VCLGraphics</code> to the underlying graphics.
	 *
	 * @param g the graphics to be copied
	 * @param srcX the x coordinate of the graphics to be copied
	 * @param srcY the y coordinate of the graphics to be copied
	 * @param srcWidth the width of the graphics to be copied
	 * @param srcHeight the height of the graphics to be copied
	 * @param destX the x coordinate of the graphics to copy to
	 * @param destY the y coordinate of the graphics to copy to
	 * @param destWidth the width of the graphics to copy to
	 * @param destHeight the height of the graphics to copy to
	 * @param allowXOR if <code>true</code> use the graphic's current XOR
	 *  setting, otherwise do not perform any XORing
	 */
	public void copyBits(VCLGraphics vg, int srcX, int srcY, int srcWidth, int srcHeight, int destX, int destY, int destWidth, int destHeight, boolean allowXOR) {

		// No copy bits allowed for printing
		if (graphics != null)
			return;

		Rectangle srcBounds = new Rectangle(srcX, srcY, srcWidth, srcHeight).intersection(vg.getGraphicsBounds());
		if (srcBounds.isEmpty())
			return;

		Rectangle destBounds = new Rectangle(destX, destY, destWidth, destHeight).intersection(graphicsBounds);
		if (destBounds.isEmpty())
			return;

		LinkedList clipList = new LinkedList();
		if (userClipList != null) {
			Iterator clipRects = userClipList.iterator();
			while (clipRects.hasNext()) {
				Rectangle clip = ((Rectangle)clipRects.next()).intersection(destBounds);
				if (!clip.isEmpty())
					clipList.add(clip);
			}
		}
		else {
			clipList.add(destBounds);
		}

		if ((xor && allowXOR) || vg != this || srcWidth != destWidth || srcHeight != destHeight || userPolygonClip) {
			BufferedImage img = null;
			if (vg.getImage() != null) {
				img = vg.getImage().getImage();

				// Fix bug 3189 and wipe down presentation transition previews
				// without causing bug 3191 by flushing in certain cases
				if (img != null && frame != null && allowXOR)
					VCLFrame.flushAllFrames();
			}

			if (img == null) {
				VCLImage srcImage = vg.createImage(srcBounds.x, srcBounds.y, srcBounds.width, srcBounds.height);
				if (srcImage == null)
					return;

				srcX -= srcBounds.x;
				srcY -= srcBounds.y;
				srcBounds.x = 0;
				srcBounds.y = 0;
				srcBounds.width = srcImage.getWidth();
				srcBounds.height = srcImage.getHeight();
				img = srcImage.getImage();
				srcImage.dispose();
			}

			Graphics2D g = getGraphics();
			if (g != null) {
				try {
					// Make sure source bounds don't fall outside the bitmap
					float scaleX = destWidth / srcWidth;
					float scaleY = destHeight / srcHeight;
					destX += (int)((srcBounds.x - srcX) * scaleX);
					destY += (int)((srcBounds.y - srcY) * scaleY);
					destWidth += (int)((srcBounds.width - srcWidth) * scaleX);
					destHeight += (int)((srcBounds.height - srcHeight) * scaleY);

					if (!userPolygonClip || (xor && allowXOR)) {
						if (xor && allowXOR) {
							g.setComposite(VCLGraphics.xorImageComposite);
							VCLGraphics.xorImageComposite.setXORMode(Color.black);
						}
						Iterator clipRects = clipList.iterator();
						while (clipRects.hasNext()) {
							g.setClip((Rectangle)clipRects.next());
							g.drawImage(img, destX, destY, destX + destWidth, destY + destHeight, srcBounds.x, srcBounds.y, srcBounds.x + srcBounds.width, srcBounds.y + srcBounds.height, null);
						}
						if (userPolygonClip)
							throw new PolygonClipException("Polygonal clip not supported for this drawing operation");
					}
					else {
						g.setClip(userClip);
						g.drawImage(img, destX, destY, destX + destWidth, destY + destHeight, srcBounds.x, srcBounds.y, srcBounds.x + srcBounds.width, srcBounds.y + srcBounds.height, null);
					}
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
					Iterator clipRects = clipList.iterator();
					while (clipRects.hasNext()) {
						// Some versions of the JVM ignore clip in copyArea()
						// so limit copying to the clip area
						Rectangle clipRect = (Rectangle)clipRects.next();
						// Fix bug 2439 by explicitly setting the clip
						g.setClip(clipRect);
						g.copyArea(srcX + clipRect.x - destX, srcY + clipRect.y - destY, clipRect.width, clipRect.height, destX - srcX, destY - srcY);
					}
				}
				catch (Throwable t) {
					t.printStackTrace();
				}
				g.dispose();
			}
		}

	}

	/**
	 * Draws the underlying graphics to the specified buffer.
	 *
	 * @param buffer the by buffer to draw into
	 * @param capacity the size of the buffer in bytes
	 * @param srcX the x coordinate of the graphics to be copied
	 * @param srcY the y coordinate of the graphics to be copied
	 * @param srcWidth the width of the graphics to be copied
	 * @param srcHeight the height of the graphics to be copied
	 * @param destX the x coordinate of the graphics to copy to
	 * @param destY the y coordinate of the graphics to copy to
	 * @param dataWidth the width of the graphics to copy to
	 * @param dataHeight the height of the graphics to copy to
	 */
	public void copyBits(ByteBuffer buffer, int capacity, int srcX, int srcY, int srcWidth, int srcHeight, int destX, int destY, int dataWidth, int dataHeight) {

		// No copy bits allowed for printing
		if (graphics != null || capacity < dataWidth * dataHeight * 4)
			return;

		buffer.order(ByteOrder.nativeOrder());

		Rectangle srcBounds = new Rectangle(srcX, srcY, srcWidth, srcHeight).intersection(graphicsBounds);
		if (srcBounds.isEmpty())
			return;

		destX += srcBounds.x - srcX;
		destY += srcBounds.y - srcY;

		Rectangle destBounds = new Rectangle(destX, destY, srcBounds.width, srcBounds.height).intersection(new Rectangle(0, 0, dataWidth, dataHeight));
		if (destBounds.isEmpty())
			return;

		srcBounds.x += destBounds.x - destX;
		srcBounds.y += destBounds.y - destY;

		boolean inRetry = false;
		int incrementY = (int)VCLEventQueue.GC_DISPOSED_PIXELS / destBounds.width;
		for (int offsetY = 0; offsetY < destBounds.height; offsetY += incrementY) {
			if (incrementY < destBounds.height)
				VCLEventQueue.runGCIfNeeded(VCLEventQueue.GC_DISPOSED_PIXELS);
			else
				VCLEventQueue.runGCIfNeeded(destBounds.width * destBounds.height);

			Graphics2D g = getGraphics(false);
			if (g != null) {
				try {
					Rectangle currentDestBounds = new Rectangle(destBounds.x, destBounds.y + offsetY, destBounds.width, destBounds.height - offsetY);
					if (currentDestBounds.height > incrementY)
						currentDestBounds.height = incrementY;

					g.setComposite(VCLGraphics.copyComposite);
					VCLGraphics.copyComposite.setData(buffer, currentDestBounds, dataWidth, dataHeight);
					g.setClip(srcBounds.x, srcBounds.y + offsetY, currentDestBounds.width, currentDestBounds.height);
					g.fillRect(srcBounds.x, srcBounds.y + offsetY, currentDestBounds.width, currentDestBounds.height);
				}
				catch (Throwable t) {
					// Force rerunning of this section
					VCLEventQueue.runGCIfNeeded(VCLEventQueue.GC_DISPOSED_PIXELS);
					if (!inRetry) {
						inRetry = true;
						offsetY -= incrementY;
					}
					else {
						inRetry = false;
						t.printStackTrace();
					}
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
	 * @param destHeight the height of the graphics to copy to
	 * @param nativeClipPath the native clip path
	 */
	public void drawBitmap(VCLBitmap bmp, int srcX, int srcY, int srcWidth, int srcHeight, int destX, int destY, int destWidth, int destHeight, long nativeClipPath) {

		if (pageQueue != null) {
			VCLGraphics.PageQueueItem pqi = new VCLGraphics.PageQueueItem(VCLGraphics.drawBitmapMethod, new Object[]{ bmp, new Integer(srcX), new Integer(srcY), new Integer(srcWidth), new Integer(srcHeight), new Integer(destX), new Integer(destY), new Integer(destWidth), new Integer(destHeight), new Long(nativeClipPath) });
			pageQueue.postDrawingOperation(pqi);
			return;
		}

		Rectangle srcBounds = new Rectangle(srcX, srcY, srcWidth, srcHeight).intersection(new Rectangle(0, 0, bmp.getWidth(), bmp.getHeight()));
		if (srcBounds.isEmpty())
			return;

		Rectangle destBounds = new Rectangle(destX, destY, destWidth, destHeight).intersection(graphicsBounds);
		if (destBounds.isEmpty())
			return;

		Graphics2D g = getGraphics();
		if (g != null) {
			try {
				if (graphics != null) {
					AffineTransform transform = g.getTransform();
					drawBitmap0(bmp.getData(), bmp.getWidth(), bmp.getHeight(), srcX, srcY, srcWidth, srcHeight, destX, destY, destWidth, destHeight, nativeClipPath, VCLGraphics.drawOnMainThread, (float)transform.getTranslateX(), (float)transform.getTranslateY(), rotatedPageAngle, pageScaleX, pageScaleY);
				}
				else {
					LinkedList clipList = new LinkedList();
					if (userClipList != null) {
						Iterator clipRects = userClipList.iterator();
						while (clipRects.hasNext()) {
							Rectangle clip = ((Rectangle)clipRects.next()).intersection(destBounds);
							if (!clip.isEmpty())
								clipList.add(clip);
						}
					}
					else {
						clipList.add(destBounds);
					}

					// Make sure source bounds don't fall outside the bitmap
					float scaleX = destWidth / srcWidth;
					float scaleY = destHeight / srcHeight;
					destX += (int)((srcBounds.x - srcX) * scaleX);
					destY += (int)((srcBounds.y - srcY) * scaleY);
					destWidth += (int)((srcBounds.width - srcWidth) * scaleX);
					destHeight += (int)((srcBounds.height - srcHeight) * scaleY);

					if (!userPolygonClip || xor) {
						if (xor) {
							g.setComposite(VCLGraphics.xorImageComposite);
							VCLGraphics.xorImageComposite.setXORMode(Color.black);
						}
						Iterator clipRects = clipList.iterator();
						while (clipRects.hasNext()) {
							g.setClip((Rectangle)clipRects.next());
							g.drawImage(bmp.getImage(), destX, destY, destX + destWidth, destY + destHeight, srcBounds.x, srcBounds.y, srcBounds.x + srcBounds.width, srcBounds.y + srcBounds.height, null);
						}
						if (userPolygonClip)
							throw new PolygonClipException("Polygonal clip not supported for this drawing operation");
					}
					else {
						g.setClip(userClip);
						g.drawImage(bmp.getImage(), destX, destY, destX + destWidth, destY + destHeight, srcBounds.x, srcBounds.y, srcBounds.x + srcBounds.width, srcBounds.y + srcBounds.height, null);
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
	 * Draws specified bitmap to the underlying graphics.
	 *
	 * @param bmpData the bitmap data buffer
	 * @param width the width of the bitmap
	 * @param height the height of the bitmap
	 * @param srcX the x coordinate of the bitmap to be drawn
	 * @param srcY the y coordinate of the bitmap to be drawn
	 * @param srcWidth the width of the bitmap to be drawn
	 * @param srcHeight the height of the bitmap to be drawn
	 * @param destX the x coordinate of the graphics to draw to
	 * @param destY the y coordinate of the graphics to draw to
	 * @param destWidth the width of the graphics to copy to
	 * @param destHeight the height of the graphics to copy to
	 * @param nativeClipPath the native clip path
	 * @param drawOnMainThread do drawing on main event dispatch thread
	 * @param translateX the horizontal translation
	 * @param translateY the vertical translation
	 * @param rotateAngle the rotation angle
	 * @param scaleX the horizontal scale factor
	 * @param scaleY the vertical scale factor
	 */
	native void drawBitmap0(int[] bmpData, int width, int height, int srcX, int srcY, int srcWidth, int srcHeight, float destX, float destY, float destWidth, float destHeight, long nativeClipPath, boolean drawOnMainThread, float translateX, float translateY, float rotateAngle, float scaleX, float scaleY);

	/**
	 * Draws the specified BitmapBuffer pointer to the underlying graphics.
	 *
	 * @param buffer the BitmapBuffer pointer to be drawn
	 * @param srcX the x coordinate of the bitmap to be drawn
	 * @param srcY the y coordinate of the bitmap to be drawn
	 * @param srcWidth the width of the bitmap to be drawn
	 * @param srcHeight the height of the bitmap to be drawn
	 * @param destX the x coordinate of the graphics to draw to
	 * @param destY the y coordinate of the graphics to draw to
	 * @param destWidth the width of the graphics to copy to
	 * @param destHeight the height of the graphics to copy to
	 * @param nativeClipPath the native clip path
	 */
	public void drawBitmapBuffer(long buffer, int srcX, int srcY, int srcWidth, int srcHeight, int destX, int destY, int destWidth, int destHeight, long nativeClipPath) {

		// Only allow drawing of JavaSalBitmp to printer
		if (graphics == null)
			return;

		if (pageQueue != null) {
			VCLGraphics.PageQueueItem pqi = new VCLGraphics.PageQueueItem(VCLGraphics.drawBitmapBufferMethod, new Object[]{ new Long(buffer), new Integer(srcX), new Integer(srcY), new Integer(srcWidth), new Integer(srcHeight), new Integer(destX), new Integer(destY), new Integer(destWidth), new Integer(destHeight), new Long(nativeClipPath) });
			pageQueue.postDrawingOperation(pqi);
			return;
		}

		Rectangle destBounds = new Rectangle(destX, destY, destWidth, destHeight).intersection(graphicsBounds);
		if (destBounds.isEmpty())
			return;

		Graphics2D g = getGraphics();
		if (g != null) {
			try {
				AffineTransform transform = g.getTransform();
				drawBitmapBuffer0(buffer, srcX, srcY, srcWidth, srcHeight, destX, destY, destWidth, destHeight, nativeClipPath, VCLGraphics.drawOnMainThread, (float)transform.getTranslateX(), (float)transform.getTranslateY(), rotatedPageAngle, pageScaleX, pageScaleY);
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			g.dispose();
		}

	}

	/**
	 * Draws specified BitmapBuffer pointer to the underlying graphics.
	 *
	 * @param buffer the BitmapBuffer pointer to be drawn
	 * @param srcX the x coordinate of the bitmap to be drawn
	 * @param srcY the y coordinate of the bitmap to be drawn
	 * @param srcWidth the width of the bitmap to be drawn
	 * @param srcHeight the height of the bitmap to be drawn
	 * @param destX the x coordinate of the graphics to draw to
	 * @param destY the y coordinate of the graphics to draw to
	 * @param destWidth the width of the graphics to copy to
	 * @param destHeight the height of the graphics to copy to
	 * @param nativeClipPath the native clip path
	 * @param drawOnMainThread do drawing on main event dispatch thread
	 * @param translateX the horizontal translation
	 * @param translateY the vertical translation
	 * @param rotateAngle the rotation angle
	 * @param scaleX the horizontal scale factor
	 * @param scaleY the vertical scale factor
	 */
	native void drawBitmapBuffer0(long buffer, int srcX, int srcY, int srcWidth, int srcHeight, float destX, float destY, float destWidth, float destHeight, long nativeClipPath, boolean drawOnMainThread, float translateX, float translateY, float rotateAngle, float scaleX, float scaleY);

	/**
	 * Draws specified EPS data to the underlying graphics.
	 *
	 * @param epsData the pointer to the EPS data
	 * @param epsDataSize the size of the EPS data pointer
	 * @param destX the x coordinate of the graphics to draw to
	 * @param destY the y coordinate of the graphics to draw to
	 * @param destWidth the width of the graphics to copy to
	 * @param destHeight the height of the graphics to copy to
	 * @param nativeClipPath the native clip path
	 */
	public void drawEPS(long epsData, long epsDataSize, int destX, int destY, int destWidth, int destHeight, long nativeClipPath) {

		// Only allow drawing of EPS data to printer
		if (graphics == null)
			return;

		if (pageQueue != null) {
			VCLGraphics.PageQueueItem pqi = new VCLGraphics.PageQueueItem(VCLGraphics.drawEPSMethod, new Object[]{ new Long(epsData), new Long(epsDataSize), new Integer(destX), new Integer(destY), new Integer(destWidth), new Integer(destHeight), new Long(nativeClipPath) });
			pageQueue.postDrawingOperation(pqi);
			return;
		}

		Rectangle destBounds = new Rectangle(destX, destY, destWidth, destHeight).intersection(graphicsBounds);
		if (destBounds.isEmpty())
			return;

		Graphics2D g = getGraphics();
		if (g != null) {
			Rectangle bounds = pageFormat.getImageableBounds();
			try {
				AffineTransform transform = g.getTransform();
				drawEPS0(epsData, epsDataSize, destX, destY, destWidth, destHeight, nativeClipPath, VCLGraphics.drawOnMainThread, (float)transform.getTranslateX(), (float)transform.getTranslateY(), rotatedPageAngle, pageScaleX, pageScaleY);
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			g.dispose();
		}

	}

	/**
	 * Draws specified EPS data to the underlying graphics.
	 *
	 * @param epsData the pointer to the EPS data
	 * @param epsDataSize the size of the EPS data pointer
	 * @param destX the x coordinate of the graphics to draw to
	 * @param destY the y coordinate of the graphics to draw to
	 * @param destWidth the width of the graphics to copy to
	 * @param destHeight the height of the graphics to copy to
	 * @param nativeClipPath the native clip path
	 * @param drawOnMainThread do drawing on main event dispatch thread
	 * @param translateX the horizontal translation
	 * @param translateY the vertical translation
	 * @param rotateAngle the rotation angle
	 * @param scaleX the horizontal scale factor
	 * @param scaleY the vertical scale factor
	 */
	native void drawEPS0(long epsData, long epsDataSize, float destX, float destY, float destWidth, float destHeight, long nativeClipPath, boolean drawOnMainThread, float translateX, float translateY, float rotateAngle, float scaleX, float scaleY);

	/**
	 * Draws the specified glyph codes using the specified font and color. Note
	 * if rotating glyphs is set, only a single glyph can be handled.
	 *
	 * @param x the x coordinate
	 * @param y the y coordinate
	 * @param count the number of glyph codes to be drawn
	 * @param glyphs the array of glyph codes to be drawn
	 * @param advances the advances for each character
	 * @param font the font of the text
	 * @param fontSize the size of the font
	 * @param fontScaleX the scale factor of the font
	 * @param color the color of the text
	 * @param orientation the tenth of degrees to rotate the text
	 * @param glyphOrientation the glyph rotation constant
	 * @param glyphTranslateX the x coordinate to translate after rotation
	 * @param glyphTranslateY the y coordinate to translate after rotation
	 * @param glyphScaleX the scale factor to apply in addition to the font's
	 *  scale factor
	 * @param nativeClipPath the native clip path
	 */
	public void drawGlyphBuffer(int x, int y, int count, long glyphs, long advances, int font, int fontSize, double fontScaleX, int color, int orientation, int glyphOrientation, int glyphTranslateX, int glyphTranslateY, float glyphScaleX, long nativeClipPath) {

		// Only allow drawing of glyph buffer to printer
		if (graphics == null)
			return;

		if (pageQueue != null) {
			VCLGraphics.PageQueueItem pqi = new VCLGraphics.PageQueueItem(VCLGraphics.drawGlyphBufferMethod, new Object[]{ new Integer(x), new Integer(y), new Integer(count), new Long(glyphs), new Long(advances), new Integer(font), new Integer(fontSize), new Double(fontScaleX), new Integer(color), new Integer(orientation), new Integer(glyphOrientation), new Integer(glyphTranslateX), new Integer(glyphTranslateY), new Float(glyphScaleX), new Long(nativeClipPath) });
			pageQueue.postDrawingOperation(pqi);
			return;
		}

		Graphics2D g = getGraphics();
		if (g != null) {
			try {
				// Calculate glyph rotation and scale
				float rotateAngle = 0.0f;
				float scaleX = 1.0f;
				float scaleY = 1.0f;

				if (orientation != 0)
					rotateAngle += (float)Math.toRadians((double)orientation / 10) * -1;

				// Fix bug 2673 by applying font scale here instead of in the
				// native method
				glyphOrientation &= VCLGraphics.GF_ROTMASK;
				if ((glyphOrientation & VCLGraphics.GF_ROTMASK) != 0) {
					if (glyphOrientation == VCLGraphics.GF_ROTL)
						rotateAngle += (float)Math.toRadians(-90);
					else
						rotateAngle += (float)Math.toRadians(90);

					scaleX *= (float)fontScaleX;
					scaleY *= glyphScaleX;
				}
				else {
					scaleX *= (float)fontScaleX * glyphScaleX;
				}

				AffineTransform transform = g.getTransform();
				drawGlyphBuffer0(x, y, count, glyphs, advances, font, fontSize, color, glyphTranslateX, glyphTranslateY, rotateAngle, scaleX, scaleY, nativeClipPath, VCLGraphics.drawOnMainThread, (float)transform.getTranslateX(), (float)transform.getTranslateY(), rotatedPageAngle, pageScaleX, pageScaleY);
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			g.dispose();
		}

	}

	/**
	 * Draws the specified glyph codes using the specified font and color. Note
	 * if rotating glyphs is set, only a single glyph can be handled.
	 *
	 * @param x the x coordinate
	 * @param y the y coordinate
	 * @param count the number of glyph codes to be drawn
	 * @param glyphs the array of glyph codes to be drawn
	 * @param advances the advances for each character
	 * @param font the font of the text
	 * @param fontSize the size of the font
	 * @param color the color of the text
	 * @param glyphTranslateX the x coordinate to translate after rotation
	 * @param glyphTranslateY the y coordinate to translate after rotation
	 * @param glyphRotateAngle the rotation angle to rotate the glyphs
	 * @param glyphScaleX the horizontal scale factor to apply after rotation
	 * @param glyphScaleY the vertical scale factor to apply after rotation
	 * @param nativeClipPath the native clip path
	 * @param drawOnMainThread do drawing on main event dispatch thread
	 * @param translateX the horizontal translation
	 * @param translateY the vertical translation
	 * @param rotateAngle the rotation angle
	 * @param scaleX the horizontal scale factor
	 * @param scaleY the vertical scale factor
	 */
	native void drawGlyphBuffer0(int x, int y, int count, long glyphs, long advances, int font, int fontSize, int color, int glyphTranslateX, int glyphTranslateY, float glyphRotateAngle, float glyphScaleX, float glyphScaleY, long nativeGlyphBuffer, boolean drawOnMainThread, float translateX, float translateY, float rotateAngle, float scaleX, float scaleY);

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
	 * @param glyphScaleX the scale factor to apply in addition to the font's
	 *  scale factor
	 */
	public void drawGlyphs(int x, int y, int[] glyphs, int[] advances, VCLFont font, int color, int orientation, int glyphOrientation, int translateX, int translateY, float glyphScaleX) {

		// Don't allow drawing of glyphs to printer
		if (graphics != null)
			return;

		LinkedList clipList = new LinkedList();
		if (userClipList != null) {
			Iterator clipRects = userClipList.iterator();
			while (clipRects.hasNext()) {
				Rectangle clip = ((Rectangle)clipRects.next()).intersection(graphicsBounds);
				if (!clip.isEmpty())
					clipList.add(clip);
			}
		}
		else {
			clipList.add(graphicsBounds);
		}

		Graphics2D g = getGraphics();
		if (g != null) {
			Graphics2D g2 = null;
			try {
				// The graphics may adjust the font
				Font f = font.getFont();
				g.setFont(f);

				if (!font.isAntialiased()) {
					RenderingHints hints = g.getRenderingHints();
					hints.put(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_OFF);
					g.setRenderingHints(hints);
				}
				g.setColor(new Color(color, true));

				GlyphVector gv = f.createGlyphVector(g.getFontRenderContext(), glyphs);

				// Fix bug 2497 by reverse scaling the advance using the font
				// scale
				double fScaleX = font.getScaleX();
				double advance = 0;
				for (int i = 0; i < glyphs.length; i++) {
					Point2D p = gv.getGlyphPosition(i);
					p.setLocation(advance, p.getY());
					gv.setGlyphPosition(i, p);
					advance += advances[i] / fScaleX;
				}

				glyphOrientation &= VCLGraphics.GF_ROTMASK;

				// Fix bug 2618 by scaling the individual glyphs instead of
				// scaling the graphics
				if (fScaleX != 1.0f || glyphScaleX != 1.0f) {
					AffineTransform glyphTransform = new AffineTransform();
					if ((glyphOrientation & VCLGraphics.GF_ROTMASK) != 0)
						glyphTransform.scale(fScaleX, glyphScaleX);
					else
						glyphTransform.scale(fScaleX * glyphScaleX, 1.0);

					for (int i = 0; i < glyphs.length; i++)
						gv.setGlyphTransform(i, glyphTransform);
				}

				// Significantly increase the overall drawing speed by only
				// using rectangular clip regions
				Iterator clipRects = clipList.iterator();
				while (clipRects.hasNext()) {
					g2 = (Graphics2D)g.create();
					g2.setClip((Rectangle)clipRects.next());
					g2.translate(x, y);

					// Set rotation
					if (orientation != 0)
						g2.rotate(Math.toRadians((double)orientation / 10) * -1);

					if ((glyphOrientation & VCLGraphics.GF_ROTMASK) != 0) {
						if (glyphOrientation == VCLGraphics.GF_ROTL)
							g2.rotate(Math.toRadians(-90));
						else
							g2.rotate(Math.toRadians(90));
					}

					// Draw the text to a scaled graphics
					g2.drawGlyphVector(gv, translateX, translateY);
				}
				if (userPolygonClip)
					throw new PolygonClipException("Polygonal clip not supported for this drawing operation");
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			if (g2 != null)
				g2.dispose();
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
	 * @param nativeClipPath the native clip path
	 */
	public void drawLine(int x1, int y1, int x2, int y2, int color, long nativeClipPath) {

		if (pageQueue != null) {
			VCLGraphics.PageQueueItem pqi = new VCLGraphics.PageQueueItem(VCLGraphics.drawLineMethod, new Object[]{ new Integer(x1), new Integer(y1), new Integer(x2), new Integer(y2), new Integer(color), new Long(nativeClipPath) });
			pageQueue.postDrawingOperation(pqi);
			return;
		}

		Rectangle destBounds = new Rectangle(x1 < x2 ? x1 : x2, y1 < y2 ? y1 : y2, x1 < x2 ? x2 - x1 : x1 - x2, y1 < y2 ? y2 - y1 : y1 - y2);
		destBounds.width++;
		destBounds.height++;
		destBounds = destBounds.intersection(graphicsBounds);
		if (destBounds.isEmpty())
			return;

		Graphics2D g = getGraphics();
		if (g != null) {
			try {
				if (graphics != null) {
					AffineTransform transform = g.getTransform();
					drawLine0(x1, y1, x2, y2, color, nativeClipPath, VCLGraphics.drawOnMainThread, (float)transform.getTranslateX(), (float)transform.getTranslateY(), rotatedPageAngle, pageScaleX, pageScaleY);
				}
				else {
					LinkedList clipList = new LinkedList();
					if (userClipList != null) {
						Iterator clipRects = userClipList.iterator();
						while (clipRects.hasNext()) {
							Rectangle clip = ((Rectangle)clipRects.next()).intersection(destBounds);
							if (!clip.isEmpty())
								clipList.add(clip);
						}
					}
					else {
						clipList.add(destBounds);
					}

					if (xor)
						g.setXORMode(color == 0xff000000 ? Color.white : Color.black);
					g.setColor(new Color(color, true));
					if (!userPolygonClip) {
						Iterator clipRects = clipList.iterator();
						while (clipRects.hasNext()) {
							g.setClip((Rectangle)clipRects.next());
							g.drawLine(x1, y1, x2, y2);
						}
					}
					else {
						g.setClip(userClip);
						g.drawLine(x1, y1, x2, y2);
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
	 * Draws a line, using the current color, between the points
	 * <code>(x1, y1)</code> and <code>(x2, y2)</code> to the underlying
	 * graphics.
	 *
	 * @param x1 the first point's x coordinate
	 * @param y1 the first point's y coordinate
	 * @param x2 the second point's x coordinate
	 * @param y2 the second point's y coordinate
	 * @param color the color of the line
	 * @param nativeClipPath the native clip path
	 * @param drawOnMainThread do drawing on main event dispatch thread
	 * @param translateX the horizontal translation
	 * @param translateY the vertical translation
	 * @param rotateAngle the rotation angle
	 * @param scaleX the horizontal scale factor
	 * @param scaleY the vertical scale factor
	 */
	native void drawLine0(float x1, float y1, float x2, float y2, int color, long nativeClipPath, boolean drawOnMainThread, float translateX, float translateY, float rotateAngle, float scaleX, float scaleY);

	/**
	 * Draws or fills the specified polygon with the specified color.
	 *
	 * @param npoints the total number of points in the polygon
	 * @param xpoints the array of x coordinates
	 * @param ypoints the array of y coordinates
	 * @param color the color of the polygon
	 * @param fill <code>true</code> to fill the polygon and <code>false</code>
	 *  to draw just the outline
	 * @param nativeClipPath the native clip path
	 */
	public void drawPolygon(int npoints, int[] xpoints, int[] ypoints, int color, boolean fill, long nativeClipPath) {

		if (pageQueue != null) {
			VCLGraphics.PageQueueItem pqi = new VCLGraphics.PageQueueItem(VCLGraphics.drawPolygonMethod, new Object[]{ new Integer(npoints), xpoints, ypoints, new Integer(color), new Boolean(fill), new Long(nativeClipPath) });
			pageQueue.postDrawingOperation(pqi);
			return;
		}

		if (npoints == 0)
			return;

		Polygon polygon = new Polygon(xpoints, ypoints, npoints);
		Rectangle destBounds = polygon.getBounds();
		if (destBounds.width < 0) {
			destBounds.x += destBounds.width;
			destBounds.width *= -1;
		}
		if (destBounds.height < 0) {
			destBounds.y += destBounds.height;
			destBounds.height *= -1;
		}
		if (destBounds.width == 0 || destBounds.height == 0) {
			drawLine(destBounds.x, destBounds.y, destBounds.x + destBounds.width, destBounds.y + destBounds.height, color, nativeClipPath);
			return;
		}
		if (!fill) {
			destBounds.width++;
			destBounds.height++;
		}
		destBounds = destBounds.intersection(graphicsBounds);
		if (destBounds.isEmpty())
			return;

		Graphics2D g = getGraphics();
		if (g != null) {
			try {
				if (graphics != null) {
					AffineTransform transform = g.getTransform();
					drawPolygon0(npoints, xpoints, ypoints, color, fill, nativeClipPath, VCLGraphics.drawOnMainThread, (float)transform.getTranslateX(), (float)transform.getTranslateY(), rotatedPageAngle, pageScaleX, pageScaleY);
				}
				else {
					LinkedList clipList = new LinkedList();
					if (userClipList != null) {
						Iterator clipRects = userClipList.iterator();
						while (clipRects.hasNext()) {
							Rectangle clip = ((Rectangle)clipRects.next()).intersection(destBounds);
							if (!clip.isEmpty())
								clipList.add(clip);
						}
					}
					else {
						clipList.add(destBounds);
					}

					if (xor) {
						// Smooth out image drawing for bug 2475 image
						if (fill)
							g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
						g.setXORMode(color == 0xff000000 ? Color.white : Color.black);
					}
					g.setColor(new Color(color, true));
					if (!userPolygonClip) {
						Iterator clipRects = clipList.iterator();
						while (clipRects.hasNext()) {
							g.setClip((Rectangle)clipRects.next());
							if (fill)
								g.fillPolygon(polygon);
							else
								g.drawPolygon(polygon);
						}
					}
					else {
						g.setClip(userClip);
						if (fill)
							g.fillPolygon(polygon);
						else
							g.drawPolygon(polygon);
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
	 * Draws or fills the specified polygon with the specified color to the
	 * underlying graphics.
	 *
	 * @param npoints the total number of points in the polygon
	 * @param xpoints the array of x coordinates
	 * @param ypoints the array of y coordinates
	 * @param color the color of the polygon
	 * @param fill <code>true</code> to fill the polygon and <code>false</code>
	 *  to draw just the outline
	 * @param nativeClipPath the native clip path
	 * @param drawOnMainThread do drawing on main event dispatch thread
	 * @param translateX the horizontal translation
	 * @param translateY the vertical translation
	 * @param rotateAngle the rotation angle
	 * @param scaleX the horizontal scale factor
	 * @param scaleY the vertical scale factor
	 */
	native void drawPolygon0(int npoints, int[] xpoints, int[] ypoints, int color, boolean fill, long nativeClipPath, boolean drawOnMainThread, float translateX, float translateY, float rotateAngle, float scaleX, float scaleY);

	/**
	 * Draws the specified polyline with the specified color.
	 *
	 * @param npoints the total number of points in the polyline
	 * @param xpoints the array of x coordinates
	 * @param ypoints the array of y coordinates
	 * @param color the color of the polyline
	 * @param nativeClipPath the native clip path
	 */
	public void drawPolyline(int npoints, int[] xpoints, int[] ypoints, int color, long nativeClipPath) {

		if (pageQueue != null) {
			VCLGraphics.PageQueueItem pqi = new VCLGraphics.PageQueueItem(VCLGraphics.drawPolylineMethod, new Object[]{ new Integer(npoints), xpoints, ypoints, new Integer(color), new Long(nativeClipPath) });
			pageQueue.postDrawingOperation(pqi);
			return;
		}

		if (npoints == 0)
			return;

		Polygon polygon = new Polygon(xpoints, ypoints, npoints);
		Rectangle destBounds = polygon.getBounds();
		if (destBounds.width < 0) {
			destBounds.x += destBounds.width;
			destBounds.width *= -1;
		}
		if (destBounds.height < 0) {
			destBounds.y += destBounds.height;
			destBounds.height *= -1;
		}
		if (destBounds.width == 0 || destBounds.height == 0) {
			drawLine(destBounds.x, destBounds.y, destBounds.x + destBounds.width, destBounds.y + destBounds.height, color, nativeClipPath);
			return;
		}
		destBounds.width++;
		destBounds.height++;
		destBounds = destBounds.intersection(graphicsBounds);
		if (destBounds.isEmpty())
			return;

		Graphics2D g = getGraphics();
		if (g != null) {
			try {
				if (graphics != null) {
					AffineTransform transform = g.getTransform();
					drawPolyline0(npoints, xpoints, ypoints, color, nativeClipPath, VCLGraphics.drawOnMainThread, (float)transform.getTranslateX(), (float)transform.getTranslateY(), rotatedPageAngle, pageScaleX, pageScaleY);
				}
				else {
					LinkedList clipList = new LinkedList();
					if (userClipList != null) {
						Iterator clipRects = userClipList.iterator();
						while (clipRects.hasNext()) {
							Rectangle clip = ((Rectangle)clipRects.next()).intersection(destBounds);
							if (!clip.isEmpty())
								clipList.add(clip);
						}
					}
					else {
						clipList.add(destBounds);
					}

					if (xor)
						g.setXORMode(color == 0xff000000 ? Color.white : Color.black);
					g.setColor(new Color(color, true));
					if (!userPolygonClip) {
						Iterator clipRects = clipList.iterator();
						while (clipRects.hasNext()) {
							g.setClip((Rectangle)clipRects.next());
							g.drawPolyline(xpoints, ypoints, npoints);
						}
					}
					else {
						g.setClip(userClip);
						g.drawPolyline(xpoints, ypoints, npoints);
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
	 * Draws the specified polyline with the specified color to the
	 * underlying graphics.
	 *
	 * @param npoints the total number of points in the polygon
	 * @param xpoints the array of x coordinates
	 * @param ypoints the array of y coordinates
	 * @param color the color of the polygon
	 * @param nativeClipPath the native clip path
	 * @param drawOnMainThread do drawing on main event dispatch thread
	 * @param translateX the horizontal translation
	 * @param translateY the vertical translation
	 * @param rotateAngle the rotation angle
	 * @param scaleX the horizontal scale factor
	 * @param scaleY the vertical scale factor
	 */
	native void drawPolyline0(int npoints, int[] xpoints, int[] ypoints, int color, long nativeClipPath, boolean drawOnMainThread, float translateX, float translateY, float rotateAngle, float scaleX, float scaleY);

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
	 * @param nativeClipPath the native clip path
	 */
	public void drawPolyPolygon(int npoly, int[] npoints, int[][] xpoints, int[][] ypoints, int color, boolean fill, long nativeClipPath) {

		if (pageQueue != null) {
			VCLGraphics.PageQueueItem pqi = new VCLGraphics.PageQueueItem(VCLGraphics.drawPolyPolygonMethod, new Object[]{ new Integer(npoly), npoints, xpoints, ypoints, new Integer(color), new Boolean(fill), new Long(nativeClipPath) });
			pageQueue.postDrawingOperation(pqi);
			return;
		}

		if (npoly == 0) {
			return;
		}
		else if (npoly == 1) {
			drawPolygon(npoints[0], xpoints[0], ypoints[0], color, fill, nativeClipPath);
			return;
		}

		Area area = null;
		for (int i = 0; i < npoly; i++) {
			Area a = new Area(new Polygon(xpoints[i], ypoints[i], npoints[i]));
			if (area == null) {
				area = a;
				continue;
			}
			area.exclusiveOr(a);
		}
		if (area == null)
			return;

		Rectangle destBounds = area.getBounds();
		if (destBounds.width < 0) {
			destBounds.x += destBounds.width;
			destBounds.width *= -1;
		}
		if (destBounds.height < 0) {
			destBounds.y += destBounds.height;
			destBounds.height *= -1;
		}
		if (destBounds.width == 0 || destBounds.height == 0) {
			drawLine(destBounds.x, destBounds.y, destBounds.x + destBounds.width, destBounds.y + destBounds.height, color, nativeClipPath);
			return;
		}
		if (!fill) {
			destBounds.width++;
			destBounds.height++;
		}
		destBounds = destBounds.intersection(graphicsBounds);
		if (destBounds.isEmpty())
			return;

		Graphics2D g = getGraphics();
		if (g != null) {
			try {
				if (graphics != null) {
					AffineTransform transform = g.getTransform();
					drawPolyPolygon0(npoly, npoints, xpoints, ypoints, color, fill, nativeClipPath, VCLGraphics.drawOnMainThread, (float)transform.getTranslateX(), (float)transform.getTranslateY(), rotatedPageAngle, pageScaleX, pageScaleY);
				}
				else {
					LinkedList clipList = new LinkedList();
					if (userClipList != null) {
						Iterator clipRects = userClipList.iterator();
						while (clipRects.hasNext()) {
							Rectangle clip = ((Rectangle)clipRects.next()).intersection(destBounds);
							if (!clip.isEmpty())
								clipList.add(clip);
						}
					}
					else {
						clipList.add(destBounds);
					}

					g.setColor(new Color(color, true));
					if (!userPolygonClip) {
						Iterator clipRects = clipList.iterator();
						while (clipRects.hasNext()) {
							g.setClip((Rectangle)clipRects.next());
							if (fill)
								g.fill(area);
							else
								g.draw(area);
						}
					}
					else {
						g.setClip(userClip);
						if (fill)
							g.fill(area);
						else
							g.draw(area);
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
	 * Draws or fills the specified polygon with the specified color to the
	 * underlying graphics.
	 *
	 * @param npoly the total number of polygons
	 * @param npoints the total number of points in each polygon
	 * @param xpoints the array of x coordinates in each polygon
	 * @param ypoints the array of y coordinates in each polygon
	 * @param color the color of the polygons
	 * @param fill <code>true</code> to fill the polygons and <code>false</code>
	 *  to draw just the outline
	 * @param nativeClipPath the native clip path
	 * @param drawOnMainThread do drawing on main event dispatch thread
	 * @param translateX the horizontal translation
	 * @param translateY the vertical translation
	 * @param rotateAngle the rotation angle
	 * @param scaleX the horizontal scale factor
	 * @param scaleY the vertical scale factor
	 */
	native void drawPolyPolygon0(int npoly, int[] npoints, int[][] xpoints, int[][] ypoints, int color, boolean fill, long nativeClipPath, boolean drawOnMainThread, float translateX, float translateY, float rotateAngle, float scaleX, float scaleY);

	/**
	 * Draws or fills the specified rectangle with the specified color.
	 *
	 * @param x the x coordinate of the rectangle
	 * @param y the y coordinate of the rectangle
	 * @param width the width of the rectangle
	 * @param height the height of the rectangle
	 * @param color the color of the rectangle
	 * @param fill <code>true</code> to fill the rectangle and
	 *  <code>false</code> to draw just the outline
	 * @param nativeClipPath the native clip path
	 */
	public void drawRect(int x, int y, int width, int height, int color, boolean fill, long nativeClipPath) {

		if (pageQueue != null) {
			VCLGraphics.PageQueueItem pqi = new VCLGraphics.PageQueueItem(VCLGraphics.drawRectMethod, new Object[]{ new Integer(x), new Integer(y), new Integer(width), new Integer(height), new Integer(color), new Boolean(fill), new Long(nativeClipPath) });
			pageQueue.postDrawingOperation(pqi);
			return;
		}

		if (width < 0) {
			x += width;
			width *= -1;
		}
		if (height < 0) {
			y += height;
			height *= -1;
		}
		Rectangle destBounds = new Rectangle(x, y, width, height);
		if (destBounds.width == 0 || destBounds.height == 0) {
			drawLine(destBounds.x, destBounds.y, destBounds.x + destBounds.width, destBounds.y + destBounds.height, color, nativeClipPath);
			return;
		}
		destBounds = destBounds.intersection(graphicsBounds);
		if (destBounds.isEmpty())
			return;

		Graphics2D g = getGraphics();
		if (g != null) {
			try {
				if (graphics != null) {
					AffineTransform transform = g.getTransform();
					if (fill)
						drawRect0(x, y, width, height, color, fill, nativeClipPath, VCLGraphics.drawOnMainThread, (float)transform.getTranslateX(), (float)transform.getTranslateY(), rotatedPageAngle, pageScaleX, pageScaleY);
					else
						drawRect0(x, y, width - 1, height - 1, color, fill, nativeClipPath, VCLGraphics.drawOnMainThread, (float)transform.getTranslateX(), (float)transform.getTranslateY(), rotatedPageAngle, pageScaleX, pageScaleY);
				}
				else {
					LinkedList clipList = new LinkedList();
					if (userClipList != null) {
						Iterator clipRects = userClipList.iterator();
						while (clipRects.hasNext()) {
							Rectangle clip = ((Rectangle)clipRects.next()).intersection(destBounds);
							if (!clip.isEmpty())
								clipList.add(clip);
						}
					}
					else {
						clipList.add(destBounds);
					}

					if (xor)
						g.setXORMode(color == 0xff000000 ? Color.white : Color.black);
					g.setColor(new Color(color, true));
					if (!userPolygonClip) {
						Iterator clipRects = clipList.iterator();
						while (clipRects.hasNext()) {
							g.setClip((Rectangle)clipRects.next());
							if (fill)
								g.fillRect(destBounds.x, destBounds.y, destBounds.width, destBounds.height);
							else
								g.drawRect(x, y, width - 1, height - 1);
						}
					}
					else {
						g.setClip(userClip);
						if (fill)
							g.fillRect(destBounds.x, destBounds.y, destBounds.width, destBounds.height);
						else
							g.drawRect(x, y, width - 1, height - 1);
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
	 * Draws or fills the specified rectangle with the specified color to
	 * the underlying graphics.
	 *
	 * @param x the x coordinate of the rectangle
	 * @param y the y coordinate of the rectangle
	 * @param width the width of the rectangle
	 * @param height the height of the rectangle
	 * @param color the color of the rectangle
	 * @param fill <code>true</code> to fill the rectangle and
	 *  <code>false</code> to draw just the outline
	 * @param nativeClipPath the native clip path
	 * @param drawOnMainThread do drawing on main event dispatch thread
	 * @param translateX the horizontal translation
	 * @param translateY the vertical translation
	 * @param rotateAngle the rotation angle
	 * @param scaleX the horizontal scale factor
	 * @param scaleY the vertical scale factor
	 */
	native void drawRect0(float x, float y, float width, float height, int color, boolean fill, long nativeClipPath, boolean drawOnMainThread, float translateX, float translateY, float rotateAngle, float scaleX, float scaleY);

	/**
	 * Draws a pushbutton into the graphics using the default Swing LAF.
	 *
	 * @param x the x coordinate of the top left of the button frame
	 * @param y the y coordinate of the top left of the button frame
	 * @param width the width of the button
	 * @param height the height of the button
	 * @param title the text to be contained within the button.  Will be placed in the button literally without accelerator replacement.
	 * @param enabled true if the button is enabled, false if the button is disabled
	 * @param focused true if the button is keyboard focused, false if the button is not keyboard focused
	 * @param pressed true if the button is currently pressed, false if it is in normal state
	 * @param isDefault true if the button is the default button of the window, false if it is a regular pushbutton
	 */
	public void drawPushButton(int x, int y, int width, int height, String title, boolean enabled, boolean focused, boolean pressed, boolean isDefault) {

		if (pageQueue != null) {
			VCLGraphics.PageQueueItem pqi = new VCLGraphics.PageQueueItem(VCLGraphics.drawPushButtonMethod, new Object[]{ new Integer(x), new Integer(y), new Integer(width), new Integer(height), title, new Boolean(enabled), new Boolean(focused), new Boolean(pressed), new Boolean(isDefault) });
			pageQueue.postDrawingOperation(pqi);
			return;
		}

		Rectangle destBounds = new Rectangle(x, y, width, height).intersection(graphicsBounds);
		if (destBounds.isEmpty())
			return;

		LinkedList clipList = new LinkedList();
		if (userClipList != null) {
			Iterator clipRects = userClipList.iterator();
			while (clipRects.hasNext()) {
				Rectangle clip = ((Rectangle)clipRects.next()).intersection(destBounds);
				if (!clip.isEmpty())
					clipList.add(clip);
			}
		}
		else {
			clipList.add(destBounds);
		}

		Graphics2D g = getGraphics();
		if (g != null) {
			try {
				if (enabled && isDefault)
					VCLGraphics.button.setDefault(true);
				else
					VCLGraphics.button.setDefault(false);
				ButtonModel m = VCLGraphics.button.getModel();
				m.setSelected(pressed);
				m.setEnabled(enabled);

				VCLGraphics.button.setLabel(title);
				Rectangle bounds = new Rectangle(x, y, width, VCLGraphics.button.getPreferredSize().height);
				boolean placard = false;
				if (bounds.height >= width - 1) {
					bounds.width--;
					bounds.height = height - 1;
					placard = true;
				}
				// Fix bug 1633 by vertically centering button
				bounds.y += (height - bounds.height) / 2;

				if (!placard && bounds.height > height) {
					VCLImage srcImage = new VCLImage(bounds.width, bounds.height, bitCount);
					VCLGraphics srcGraphics = srcImage.getGraphics();
					srcGraphics.drawPushButton(0, 0, bounds.width, bounds.height, title, enabled, focused, pressed, isDefault);
					copyBits(srcGraphics, 0, 0, bounds.width, bounds.height, x, y, bounds.width, height, false);
					srcImage.dispose();
				}
				else {
					VCLGraphics.button.setSize(bounds.width, bounds.height);
					if (!userPolygonClip) {
						Iterator clipRects = clipList.iterator();
						while (clipRects.hasNext()) {
							g.setClip((Rectangle)clipRects.next());
							g.translate(bounds.x, bounds.y);
							VCLGraphics.button.getUI().paint(g, VCLGraphics.button);
							g.translate(bounds.x * -1, bounds.y * -1);
						}
					}
					else {
						g.setClip(userClip);
						g.translate(bounds.x, bounds.y);
						VCLGraphics.button.getUI().paint(g, VCLGraphics.button);
						g.translate(bounds.x * -1, bounds.y * -1);
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
	 * Retrieves the desired width for a button implemented via default
	 * Swing LAF.
	 *
	 * @param x the x coordinate of the top left of the button frame
	 * @param y the y coordinate of the top left of the button frame
	 * @param width the width of the button
	 * @param height the height of the button
	 * @param title the text to be contained within the button.  Will be placed in the button literally without accelerator replacement.
	 * @return desired button bounds
	 */
	public Rectangle getPreferredPushButtonBounds(int x, int y, int width, int height, String title) {

		// If the button width is less than the preferred height, assume that
		// it's intended to be a "placard" type button with an icon. In that
		// case, return the requested height as the Aqua LAF will then draw it
		// as a placard button instead of a rounded button. This makes buttons
		// used as parts of subcontrols (combo boxes, small toolbar buttons)
		// draw with the appropriate style.
		Rectangle bounds = new Rectangle(x, y, width, VCLGraphics.button.getPreferredSize().height);
		if (bounds.height >= width) {
			bounds.width++;
			bounds.height = height + 1;
		}
		else if (bounds.height != height && height > 0) {
			bounds.height = height;
		}
		bounds.y += (height - bounds.height) / 2;
		return bounds;

	}

	/**
	 * Draws a radiobutton into the graphics using the default Swing LAF
	 *
	 * @param x the x coordinate of the top left of the button frame
	 * @param y the y coordinate of the top left of the button frame
	 * @param width the width of the button
	 * @param height the height of the button
	 * @param title the text to be drawn aside the button.  Will be placed besode the button literally without accelerator replacement.
	 * @param enabled true if the button is enabled, false if the button is disabled
	 * @param focused true if the button is keyboard focused, false if the button is not keyboard focused
	 * @param pressed true if the button is currently pressed, false if it is in normal state
	 * @param buttonState	0 = off, 1 = on, 2 = mixed.  Note that Aqua does not provide mixed button state by default.
	 */
	public void drawRadioButton(int x, int y, int width, int height, String title, boolean enabled, boolean focused, boolean pressed, int buttonState) {

		if (pageQueue != null) {
			VCLGraphics.PageQueueItem pqi = new VCLGraphics.PageQueueItem(VCLGraphics.drawRadioButtonMethod, new Object[]{ new Integer(x), new Integer(y), new Integer(width), new Integer(height), title, new Boolean(enabled), new Boolean(focused), new Boolean(pressed), new Integer(buttonState) });
			pageQueue.postDrawingOperation(pqi);
			return;
		}

		Rectangle destBounds = new Rectangle(x, y, width, height).intersection(graphicsBounds);
		if (destBounds.isEmpty())
			return;

		LinkedList clipList = new LinkedList();
		if (userClipList != null) {
			Iterator clipRects = userClipList.iterator();
			while (clipRects.hasNext()) {
				Rectangle clip = ((Rectangle)clipRects.next()).intersection(destBounds);
				if (!clip.isEmpty())
					clipList.add(clip);
			}
		}
		else {
			clipList.add(destBounds);
		}

		Graphics2D g = getGraphics();
		if (g != null) {
			try {
				ButtonModel m = VCLGraphics.radioButton.getModel();
				m.setEnabled(enabled);
				m.setPressed(pressed);
				if (pressed)
					m.setArmed(true);
				else
					m.setArmed(false);
				if (buttonState == VCLGraphics.BUTTONVALUE_ON)
					m.setSelected(true);
				else
					m.setSelected(false);

				// Fix bug 3028 by using the adjusted preferred bounds
				Dimension d = getPreferredRadioButtonBounds(0, 0, 1, 1, "").getSize();
				Rectangle bounds = new Rectangle(x + VCLGraphics.radioButtonXOffset, y + VCLGraphics.radioButtonYOffset, d.width, d.height);
				if (width > d.width)
					bounds.x += (width - d.width) / 2;
				if (height > d.height)
					bounds.y += (height - d.height) / 2;
				if (bounds.width > width || bounds.height > height) {
					VCLImage srcImage = new VCLImage(bounds.width, bounds.height, bitCount);
					VCLGraphics srcGraphics = srcImage.getGraphics();
					srcGraphics.drawRadioButton(0, 0, bounds.width, bounds.height, title, enabled, focused, pressed, buttonState);
					copyBits(srcGraphics, 0, 0, bounds.width, bounds.height, x, y, width, height, false);
					srcImage.dispose();
				}
				else {
					VCLGraphics.radioButton.setSize(d.width, d.height);
					if (!userPolygonClip) {
						Iterator clipRects = clipList.iterator();
						while (clipRects.hasNext()) {
							g.setClip((Rectangle)clipRects.next());
							g.translate(bounds.x, bounds.y);
							VCLGraphics.radioButton.getUI().paint(g, VCLGraphics.radioButton);
							g.translate(bounds.x * -1, bounds.y * -1);
						}
					}
					else {
						g.setClip(userClip);
						g.translate(bounds.x, bounds.y);
						VCLGraphics.radioButton.getUI().paint(g, VCLGraphics.radioButton);
						g.translate(bounds.x * -1, bounds.y * -1);
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
	 * Retrieves the desired width for a radio button implemented via default
	 * Swing LAF.
	 *
	 * @param x the x coordinate of the top left of the button frame
	 * @param y the y coordinate of the top left of the button frame
	 * @param width the width of the button
	 * @param height the height of the button
	 * @param title the text to be contained within the button.  Will be placed in the button literally without accelerator replacement.
	 * @return desired button bounds
	 */
	public Rectangle getPreferredRadioButtonBounds(int x, int y, int width, int height, String title) {

		// Fix bug 3028 by applying the same amount of adjustment to both
		// the width and height
		return new Rectangle(x, y, VCLGraphics.radioButtonPreferredSize.width + 4, VCLGraphics.radioButtonPreferredSize.height + 4);

	}
	
	/**
	 * Draws a check box into the graphics using the default Swing LAF
	 *
	 * @param x the x coordinate of the top left of the button frame
	 * @param y the y coordinate of the top left of the button frame
	 * @param width the width of the button
	 * @param height the height of the button
	 * @param title the text to be drawn aside the button.  Will be placed besode the button literally without accelerator replacement.
	 * @param enabled true if the button is enabled, false if the button is disabled
	 * @param focused true if the button is keyboard focused, false if the button is not keyboard focused
	 * @param pressed true if the button is currently pressed, false if it is in normal state
	 * @param buttonState	0 = off, 1 = on, 2 = mixed.  Note that Aqua does not provide mixed button state by default.
	 */
	public void drawCheckBox(int x, int y, int width, int height, String title, boolean enabled, boolean focused, boolean pressed, int buttonState) {

		if (pageQueue != null) {
			VCLGraphics.PageQueueItem pqi = new VCLGraphics.PageQueueItem(VCLGraphics.drawCheckBoxMethod, new Object[]{ new Integer(x), new Integer(y), new Integer(width), new Integer(height), title, new Boolean(enabled), new Boolean(focused), new Boolean(pressed), new Integer(buttonState) });
			pageQueue.postDrawingOperation(pqi);
			return;
		}

		Rectangle destBounds = new Rectangle(x, y, width, height).intersection(graphicsBounds);
		if (destBounds.isEmpty())
			return;

		LinkedList clipList = new LinkedList();
		if (userClipList != null) {
			Iterator clipRects = userClipList.iterator();
			while (clipRects.hasNext()) {
				Rectangle clip = ((Rectangle)clipRects.next()).intersection(destBounds);
				if (!clip.isEmpty())
					clipList.add(clip);
			}
		}
		else {
			clipList.add(destBounds);
		}

		Graphics2D g = getGraphics();
		if (g != null) {
			try {
				ButtonModel m = VCLGraphics.checkBoxButton.getModel();
				m.setEnabled(enabled);
				m.setPressed(pressed);
				if (pressed)
					m.setArmed(true);
				else
					m.setArmed(false);
				if (buttonState == VCLGraphics.BUTTONVALUE_ON)
					m.setSelected(true);
				else
					m.setSelected(false);

				// Fix bug 3028 by using the adjusted preferred bounds
				Dimension d = getPreferredCheckBoxBounds(0, 0, 1, 1, "").getSize();
				Rectangle bounds = new Rectangle(x + VCLGraphics.checkBoxButtonXOffset, y + VCLGraphics.checkBoxButtonYOffset, d.width, d.height);
				if (width > d.width)
					bounds.x += (width - d.width) / 2;
				if (height > d.height)
					bounds.y += (height - d.height) / 2;
				if (bounds.width > width || bounds.height > height) {
					VCLImage srcImage = new VCLImage(bounds.width, bounds.height, bitCount);
					VCLGraphics srcGraphics = srcImage.getGraphics();
					srcGraphics.drawCheckBox(0, 0, bounds.width, bounds.height, title, enabled, focused, pressed, buttonState);
					copyBits(srcGraphics, 0, 0, bounds.width, bounds.height, x, y, width, height, false);
					srcImage.dispose();
				}
				else {
					VCLGraphics.checkBoxButton.setSize(d.width, d.height);
					if (!userPolygonClip) {
						Iterator clipRects = clipList.iterator();
						while (clipRects.hasNext()) {
							g.setClip((Rectangle)clipRects.next());
							g.translate(bounds.x, bounds.y);
							VCLGraphics.checkBoxButton.getUI().paint(g, VCLGraphics.checkBoxButton);
							g.translate(bounds.x * -1, bounds.y * -1);
						}
					}
					else {
						g.setClip(userClip);
						g.translate(bounds.x, bounds.y);
						VCLGraphics.checkBoxButton.getUI().paint(g, VCLGraphics.checkBoxButton);
						g.translate(bounds.x * -1, bounds.y * -1);
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
	 * Retrieves the desired width for a checkbox implemented via default
	 * Swing LAF.
	 *
	 * @param x the x coordinate of the top left of the button frame
	 * @param y the y coordinate of the top left of the button frame
	 * @param width the width of the button
	 * @param height the height of the button
	 * @param title the text to be contained within the button.  Will be placed in the button literally without accelerator replacement.
	 * @return desired button bounds
	 */
	public Rectangle getPreferredCheckBoxBounds(int x, int y, int width, int height, String title) {

		return new Rectangle(x, y, VCLGraphics.checkBoxButtonPreferredSize.width + 4, VCLGraphics.checkBoxButtonPreferredSize.height);

	}

	/**
	 * Applies the cached clipping area. The cached clipping area is set using
	 * the {@link #beginSetClipRegion(boolean)} and the
	 * {@link #unionClipRegion(long, long, long, long, boolean)} methods.
	 *
	 * @param b <code>true</code> if this clip is coming from the C++ SalFrame
	 *  methods otherwise <code>false</code>
	 */
	public void endSetClipRegion(boolean b) {}

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

		Graphics2D g = getGraphics(false);
		if (g != null) {
			try {
				GlyphVector glyphs = font.getFont().createGlyphVector(g.getFontRenderContext(), new int[]{ glyph });
				bounds = glyphs.getGlyphOutline(0).getBounds2D();
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			g.dispose();
		}

		if (bounds != null) {
			double fScaleX = font.getScaleX();
			if (fScaleX != 1.0) {
				if ((glyphOrientation & VCLGraphics.GF_ROTMASK) != 0)
					bounds = new Rectangle2D.Double(bounds.getX(), bounds.getY() * fScaleX, bounds.getWidth(), bounds.getHeight() * fScaleX);
				else
					bounds = new Rectangle2D.Double(bounds.getX() * fScaleX, bounds.getY(), bounds.getWidth() * fScaleX, bounds.getHeight());
			}

			return bounds.getBounds();
		}

		return null;

	}

	/**
	 * Returns the graphics context.
	 *
	 * @return the graphics context
	 */
	Graphics2D getGraphics() {

		return getGraphics(true);

	}

	/**
	 * Returns the graphics context.
	 *
	 * @param notify <code>true</code> to notify any attached bitmaps that
	 *  the content in this graphics is likely to be changed
	 * @return the graphics context
	 */
	Graphics2D getGraphics(boolean notify) {

		Graphics2D g;
		if (image != null) {
			g = image.getImage().createGraphics();
		}
		else if (frame != null) {
			Panel p = frame.getPanel();
			if (p != null && p.isShowing())
				g = (Graphics2D)p.getGraphics();
			else
				g = null;
		}
		else if (graphics != null) {
			g = (Graphics2D)graphics.create();
		}
		else {
			g = null;
		}

		if (g != null) {
			g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_OFF);
			if (notify)
				notifyGraphicsChanged();
		}

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
	VCLImage getImage() {

		return image;

	}

	/**
	 * Returns the <code>VCLGraphics.PageQueue</code>.
	 *
	 * @return the <code>VCLGraphics.PageQueue</code>
	 */
	VCLGraphics.PageQueue getPageQueue() {

		return pageQueue;

	}

	/**
	 * Returns the pixel color in ARGB format for the specified coordinate.
	 *
	 * @param x the x coordinate of the source rectangle
	 * @param y the y coordinate of the source rectangle
	 * @return the pixel color in ARGB format
	 */
	public int getPixel(int x, int y) {

		if (graphics != null || !graphicsBounds.contains(x, y))
			return 0xff000000;

		int pixel = 0xff000000;

		Graphics2D g = getGraphics(false);
		if (g != null) {
			try {
				g.setComposite(VCLGraphics.createCopyComposite);
				g.setClip(x, y, 1, 1);
				g.fillRect(x, y, 1, 1);
				int[] srcData = new int[1];
				srcData = (int[])VCLGraphics.createCopyComposite.getRaster().getDataElements(0, 0, srcData.length, 1, srcData);
				pixel = srcData[0] | 0xff000000;
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			g.dispose();
		}

		return pixel;

	}

	/**
	 * Returns the resolution of the underlying graphics device.
	 *
	 * @return the resolution of the underlying graphics device.
	 */
	public Dimension getResolution() {

		if (pageFormat != null)
			return VCLGraphics.printerResolution;
		else
			return VCLGraphics.screenResolution;

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

		if (width < 0) {
			x += width;
			width *= -1;
		}
		if (height < 0) {
			y += height;
			height *= -1;
		}
		Rectangle destBounds = new Rectangle(x, y, width, height).intersection(graphicsBounds);
		if ((options & VCLGraphics.SAL_INVERT_TRACKFRAME) == VCLGraphics.SAL_INVERT_TRACKFRAME) {
			destBounds.width++;
			destBounds.height++;
		}
		if (destBounds.isEmpty())
			return;

		LinkedList clipList = new LinkedList();
		if (userClipList != null) {
			Iterator clipRects = userClipList.iterator();
			while (clipRects.hasNext()) {
				Rectangle clip = ((Rectangle)clipRects.next()).intersection(destBounds);
				if (!clip.isEmpty())
					clipList.add(clip);
			}
		}
		else {
			clipList.add(destBounds);
		}

		// Invert the image
		if ((options & VCLGraphics.SAL_INVERT_TRACKFRAME) == VCLGraphics.SAL_INVERT_TRACKFRAME) {
			Graphics2D g = getGraphics();
			if (g != null) {
				try {
					BasicStroke stroke = (BasicStroke)g.getStroke();
					g.setStroke(new BasicStroke(stroke.getLineWidth(), BasicStroke.CAP_BUTT, BasicStroke.JOIN_MITER, stroke.getMiterLimit(), new float[]{ 1.0f, 1.0f }, 0.0f));
					g.setXORMode(Color.white);
					g.setColor(Color.black);
					// Note: the JVM seems to have a bug and drawRect()
					// draws dashed strokes one pixel above the
					// specified y coordinate
					if (!userPolygonClip) {
						Iterator clipRects = clipList.iterator();
						while (clipRects.hasNext()) {
							g.setClip((Rectangle)clipRects.next());
							g.drawRect(x, y + 1, width - 1, height - 2);
						}
					}
					else {
						g.setClip(userClip);
						g.drawRect(x, y + 1, width - 1, height - 2);
					}
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
					if (!userPolygonClip) {
						Iterator clipRects = clipList.iterator();
						while (clipRects.hasNext()) {
							g.setClip((Rectangle)clipRects.next());
							g.fillRect(x, y, width, height);
						}
					}
					else {
						g.setClip(userClip);
						g.fillRect(x, y, width, height);
					}
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
					Iterator clipRects = clipList.iterator();
					while (clipRects.hasNext()) {
						Rectangle clipRect = (Rectangle)clipRects.next();
						g.setClip(clipRect);
						g.fillRect(clipRect.x, clipRect.y, clipRect.width, clipRect.height);
					}
					if (userPolygonClip)
						throw new PolygonClipException("Polygonal clip not supported for this drawing operation");
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

		if (npoints == 0)
			return;

		Polygon polygon = new Polygon(xpoints, ypoints, npoints);
		Rectangle destBounds = polygon.getBounds();
		if (destBounds.width < 0) {
			destBounds.x += destBounds.width;
			destBounds.width *= -1;
		}
		if (destBounds.height < 0) {
			destBounds.y += destBounds.height;
			destBounds.height *= -1;
		}
		if ((options & VCLGraphics.SAL_INVERT_TRACKFRAME) == VCLGraphics.SAL_INVERT_TRACKFRAME) {
			destBounds.width++;
			destBounds.height++;
		}
		destBounds = destBounds.intersection(graphicsBounds);
		if (destBounds.isEmpty())
			return;

		LinkedList clipList = new LinkedList();
		if (userClipList != null) {
			Iterator clipRects = userClipList.iterator();
			while (clipRects.hasNext()) {
				Rectangle clip = ((Rectangle)clipRects.next()).intersection(destBounds);
				if (!clip.isEmpty())
					clipList.add(clip);
			}
		}
		else {
			clipList.add(destBounds);
		}

		// Invert the image
		if ((options & VCLGraphics.SAL_INVERT_TRACKFRAME) == VCLGraphics.SAL_INVERT_TRACKFRAME) {
			Graphics2D g = getGraphics();
			if (g != null) {
				try {
					BasicStroke stroke = (BasicStroke)g.getStroke();
					g.setStroke(new BasicStroke(stroke.getLineWidth(), BasicStroke.CAP_BUTT, BasicStroke.JOIN_MITER, stroke.getMiterLimit(), new float[]{ 1.0f, 1.0f }, 0.0f));
					g.setXORMode(Color.white);
					g.setColor(Color.black);
					if (!userPolygonClip) {
						Iterator clipRects = clipList.iterator();
						while (clipRects.hasNext()) {
							g.setClip((Rectangle)clipRects.next());
							g.drawPolyline(xpoints, ypoints, npoints);
						}
					}
					else {
						g.setClip(userClip);
						g.drawPolyline(xpoints, ypoints, npoints);
					}
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
					if (!userPolygonClip) {
						Iterator clipRects = clipList.iterator();
						while (clipRects.hasNext()) {
							g.setClip((Rectangle)clipRects.next());
							g.fillPolygon(polygon);
						}
					}
					else {
						g.setClip(userClip);
						g.fillPolygon(polygon);
					}
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
					Iterator clipRects = clipList.iterator();
					while (clipRects.hasNext()) {
						g.setClip((Rectangle)clipRects.next());
						g.fillPolygon(polygon);
					}
					if (userPolygonClip)
						throw new PolygonClipException("Polygonal clip not supported for this drawing operation");
				}
				catch (Throwable t) {
					t.printStackTrace();
				}
				g.dispose();
			}
		}

	}

	/**
	 * Notifies the specified change listener.
	 *
	 * @param listeners an array of pointers to C++ JavaSalBitmap instances
	 * @param disposed <code>true</code> if this graphics has been disposed,
	 *  otherwise <code>false</code>
	 */
	native void notifyGraphicsChanged(long[] listeners, boolean disposed);

	/**
	 * Notifies all change listeners.
	 */
	void notifyGraphicsChanged() {

		if (changeListeners != null) {
			boolean d = false;
			if (frame != null) {
				Panel p = frame.getPanel();
				d = (p == null || !p.isShowing());
			}

			int currentElement = 0;
			long[] listenerArray = new long[changeListeners.size()];
			while (changeListeners.size() > 0)
				listenerArray[currentElement++] = ((Long)changeListeners.removeFirst()).longValue();
			notifyGraphicsChanged(listenerArray, d);

			if (VCLGraphics.needsDisposeGraphics == this && changeListeners.size() == 0)
				VCLGraphics.disposeNeedsDisposeGraphics();
		}

	}

	/**
	 * Removes a change listener.
	 *
	 * @param listener a pointer to a C++ JavaSalBitmap instance
	 */
	public void removeGraphicsChangeListener(long listener) {

		if (changeListeners != null)
			changeListeners.remove(new Long(listener));

	}

	/**
	 * Resets the applied clipping area.
	 *
	 * @param b <code>true</code> if this clip is coming from the C++ SalFrame
	 *  methods otherwise <code>false</code>
	 */
	public void resetClipRegion(boolean b) {

		if (graphics != null)
			return;

		if (b) {
			frameClip = null;
			frameClipList = null;
			framePolygonClip = false;
			userClip = graphicsClip;
			userClipList = graphicsClipList;
			userPolygonClip = graphicsPolygonClip;
		}
		else {
			graphicsClip = null;
			graphicsClipList = null;
			graphicsPolygonClip = false;
			userClip = frameClip;
			userClipList = frameClipList;
			userPolygonClip = framePolygonClip;
		}

	}

	/**
	 * Resets the underlying graphics context.
	 */
	public void resetGraphics() {

		if (frame != null) {
			notifyGraphicsChanged();

			Panel p = frame.getPanel();
			if (p != null) {
				Rectangle bounds = p.getBounds();
				graphicsBounds = new Rectangle(0, 0, bounds.width, bounds.height);
			}

			if (graphicsBounds.isEmpty())
				graphicsBounds = new Rectangle(0, 0, 1, 1);

			bitCount = frame.getBitCount();
		}

	}

	/**
	 * Set the pixel color for the specified coordinate.
	 *
	 * @param x the x coordinate of the source rectangle
	 * @param y the y coordinate of the source rectangle
	 * @param color the color of the pixel
	 * @param nativeClipPath the native clip path
	 */
	public void setPixel(int x, int y, int color, long nativeClipPath) {

		if (pageQueue != null) {
			VCLGraphics.PageQueueItem pqi = new VCLGraphics.PageQueueItem(VCLGraphics.setPixelMethod, new Object[]{ new Integer(x), new Integer(y), new Integer(color), new Long(nativeClipPath) });
			pageQueue.postDrawingOperation(pqi);
			return;
		}

		if (!graphicsBounds.contains(x, y))
			return;

		// Invoking Area.contains() does not seem to work
		if (userClipList != null) {
			boolean inClip = false;
			Iterator clipRects = userClipList.iterator();
			while (clipRects.hasNext()) {
				Rectangle clip = ((Rectangle)clipRects.next()).intersection(graphicsBounds);
				if (!clip.isEmpty() && clip.contains(x, y)) {
					inClip = true;
					break;
				}
			}

			if (!inClip)
				return;
		}

		// Don't use BitmapBuffer.setRGB() as it is flaky only small portions
		// of the image have been painted prior to this call
		Graphics2D g = getGraphics();
		if (g != null) {
			try {
				if (graphics != null) {
					AffineTransform transform = g.getTransform();
					drawBitmap0(new int[]{ color }, 1, 1, 0, 0, 1, 1, x, y, 1, 1, nativeClipPath, VCLGraphics.drawOnMainThread, (float)transform.getTranslateX(), (float)transform.getTranslateY(), rotatedPageAngle, pageScaleX, pageScaleY);
				}
				else {
					if (xor)
						g.setXORMode(color == 0xff000000 ? Color.white : Color.black);
					g.setColor(new Color(color, true));
					// Fix bug 2438 by drawing a line instead of
					// filling it and not setting any clip
					g.drawLine(x, y, x, y);
				}
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			g.dispose();
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
	 * {@link #endSetClipRegion(boolean)} method is called.
	 *
	 * @param x the x coordinate of the rectangle to add to clip region
	 * @param y the y coordinate of the rectangle to add to clip region
	 * @param width the width of the rectangle to add to clip region
	 * @param height the height of the rectangle to add to clip region
	 * @param b <code>true</code> if this clip is coming from the C++ SalFrame
	 *  methods otherwise <code>false</code>
	 */
	public void unionClipRegion(int x, int y, int width, int height, boolean b) {

		if (graphics != null)
			return;

		Rectangle bounds = new Rectangle(x, y, width, height);
		if (bounds.isEmpty())
			return;

		Area area = new Area(bounds);
		Area currentClip;
		LinkedList currentClipList;
		boolean currentPolygonClip;
		if (b) {
			currentClip = frameClip;
			currentClipList = frameClipList;
			currentPolygonClip = framePolygonClip;
		}
		else {
			currentClip = graphicsClip;
			currentClipList = graphicsClipList;
			currentPolygonClip = graphicsPolygonClip;
		}

		if (currentClip != null)
			currentClip.add(area);
		else
			currentClip = area;

		if (currentClip.isEmpty()) {
			currentClipList = new LinkedList();
			currentPolygonClip = false;
		}
		else if (currentClip.isRectangular()) {
			currentClipList = new LinkedList();
			currentClipList.add(currentClip.getBounds());
			currentPolygonClip = false;
		}
		else {
			// Don't change currentPolygonClip flag
			if (currentClipList == null)
				currentClipList = new LinkedList();
			currentClipList.add(area.getBounds());
		}

		if (b) {
			frameClip = currentClip;
			frameClipList = currentClipList;
			framePolygonClip = currentPolygonClip;
			if (frameClip != null && graphicsClip != null) {
				// Don't change userPolygonClip flag
				userClip = new Area(frameClip);
				userClip.subtract(graphicsClip);
				userClipList = new LinkedList();
				if (graphicsClipList != null) {
					Iterator graphicsClipRects = graphicsClipList.iterator();
					while (graphicsClipRects.hasNext()) {
						Rectangle currentGraphicsClip = (Rectangle)graphicsClipRects.next();
						Iterator clipRects = graphicsClipList.iterator();
						while (clipRects.hasNext()) {
							Rectangle clip = ((Rectangle)clipRects.next()).intersection(currentGraphicsClip);
							if (!clip.isEmpty())
								userClipList.add(clip);
						}
					}
				}
			}
			else {
				userClip = currentClip;
				userClipList = currentClipList;
				userPolygonClip = currentPolygonClip;
			}
		}
		else {
			graphicsClip = currentClip;
			graphicsClipList = currentClipList;
			graphicsPolygonClip = currentPolygonClip;
			if (graphicsClip != null && frameClip != null) {
				// Don't change userPolygonClip flag
				userClip = new Area(graphicsClip);
				userClip.subtract(frameClip);
				userClipList = new LinkedList();
				if (graphicsClipList != null) {
					Iterator frameClipRects = frameClipList.iterator();
					while (frameClipRects.hasNext()) {
						Rectangle currentFrameClip = (Rectangle)frameClipRects.next();
						Iterator clipRects = graphicsClipList.iterator();
						while (clipRects.hasNext()) {
							Rectangle clip = ((Rectangle)clipRects.next()).intersection(currentFrameClip);
							if (!clip.isEmpty())
								userClipList.add(clip);
						}
					}
				}
			}
			else {
				userClip = currentClip;
				userClipList = currentClipList;
				userPolygonClip = currentPolygonClip;
			}
		}

	}

	/**
	 * Unions the cached clipping area with the specified poly polygon. The
	 * cached clipping area is not actually applied until the
	 * {@link #endSetClipRegion(boolean)} method is called. Note that some
	 * drawing methods in this class cannot handle non-rectangular clip regions
	 * so this method will likely leave too big of a clip region for such
	 * methods.
	 *
	 * @param npoly the number of polygons
	 * @param npoints the array of the total number of points in each polygon
	 * @param xpoints the array of arrays of x coordinates
	 * @param ypoints the array of arrays of y coordinates
	 * @param b <code>true</code> if this clip is coming from the C++ SalFrame
	 *  methods otherwise <code>false</code>
	 * @return <code>true</code> if the clip region is not empty, otherwise
	 *  <code>false</code>
	 */
	public boolean unionClipRegion(int npoly, int[] npoints, int[][] xpoints, int[][] ypoints, boolean b) {

		if (graphics != null)
			return false;

		for (int i = 0; i < npoly; i++) {
			Polygon p = new Polygon(xpoints[i], ypoints[i], npoints[i]);
			Area a = new Area(p);
			if (a != null && !a.isEmpty()) {
				Area currentClip;
				LinkedList currentClipList;
				boolean currentPolygonClip;
				if (b) {
					currentClip = frameClip;
					currentClipList = frameClipList;
					currentPolygonClip = framePolygonClip;
				}
				else {
					currentClip = graphicsClip;
					currentClipList = graphicsClipList;
					currentPolygonClip = graphicsPolygonClip;
				}

				if (currentClip != null)
					currentClip.add(a);
				else
					currentClip = a;

				if (currentClip.isEmpty()) {
					currentClipList = new LinkedList();
					currentPolygonClip = false;
				}
				else if (currentClip.isRectangular()) {
					currentClipList = new LinkedList();
					currentClipList.add(currentClip.getBounds());
					currentPolygonClip = false;
				}
				else {
					if (currentClipList == null)
						currentClipList = new LinkedList();
					if (!currentPolygonClip && !a.isRectangular())
						currentPolygonClip = true;
				}

				if (b) {
					frameClip = currentClip;
					frameClipList = currentClipList;
					framePolygonClip = currentPolygonClip;
				}
				else {
					graphicsClip = currentClip;
					graphicsClipList = currentClipList;
					graphicsPolygonClip = currentPolygonClip;
				}
			}
		}

		if (b) {
			if (frameClip != null && graphicsClip != null) {
				// Don't change userPolygonClip flag
				userClip = new Area(frameClip);
				userClip.subtract(graphicsClip);
				userClipList = new LinkedList();
				if (graphicsClipList != null) {
					Iterator graphicsClipRects = graphicsClipList.iterator();
					while (graphicsClipRects.hasNext()) {
						Rectangle currentGraphicsClip = (Rectangle)graphicsClipRects.next();
						Iterator clipRects = graphicsClipList.iterator();
						while (clipRects.hasNext()) {
							Rectangle clip = ((Rectangle)clipRects.next()).intersection(currentGraphicsClip);
							if (!clip.isEmpty())
								userClipList.add(clip);
						}
					}
				}
			}
			else {
				userClip = frameClip;
				userClipList = frameClipList;
				userPolygonClip = framePolygonClip;
			}
		}
		else {
			if (graphicsClip != null && frameClip != null) {
				// Don't change userPolygonClip flag
				userClip = new Area(graphicsClip);
				userClip.subtract(frameClip);
				userClipList = new LinkedList();
				if (graphicsClipList != null) {
					Iterator frameClipRects = frameClipList.iterator();
					while (frameClipRects.hasNext()) {
						Rectangle currentFrameClip = (Rectangle)frameClipRects.next();
						Iterator clipRects = graphicsClipList.iterator();
						while (clipRects.hasNext()) {
							Rectangle clip = ((Rectangle)clipRects.next()).intersection(currentFrameClip);
							if (!clip.isEmpty())
								userClipList.add(clip);
						}
					}
				}
			}
			else {
				userClip = graphicsClip;
				userClipList = graphicsClipList;
				userPolygonClip = graphicsPolygonClip;
			}
		}

		return (userClip != null && !userClip.isEmpty());

	}

	/**
	 * The <code>PageQueue</code> object holds pointers to the beginning and
	 * end of the drawing queue.
	 */
	final class PageQueue {

		VCLGraphics.PageQueueItem drawingHead = null;

		VCLGraphics.PageQueueItem drawingTail = null;

		VCLGraphics graphics = null;

		PageQueue(VCLGraphics g) {

			graphics = g;

		}

		void dispose() {

			// Release any native bitmaps
			VCLGraphics.releaseNativeBitmaps(false);

			drawingHead = null;
			drawingTail = null;
			graphics = null;

		}

		void drawOperations() {

			Area oldUserClip = graphics.userClip;
			LinkedList oldUserClipList = graphics.userClipList;
			boolean oldUserPolygonClip = graphics.userPolygonClip;
			boolean oldXOR = graphics.xor;

			graphics.pageQueue = null;

			// Invoke all of the queued drawing operations
			while (drawingHead != null) {
				graphics.userClip = drawingHead.clip;
				graphics.userClipList = drawingHead.clipList;
				graphics.userPolygonClip = drawingHead.polygonClip;
				try {
					drawingHead.method.invoke(graphics, drawingHead.params);
				}
				catch (Throwable t) {
					t.printStackTrace();
				}

				VCLGraphics.PageQueueItem i = drawingHead;
				drawingHead = drawingHead.next;
				i.next = null;
			}
			drawingTail = null;

			graphics.userClip = oldUserClip;
			graphics.userClipList = oldUserClipList;
			graphics.userPolygonClip = oldUserPolygonClip;
			graphics.xor = oldXOR;

			graphics.pageQueue = this;

		}

		void postDrawingOperation(VCLGraphics.PageQueueItem i) {

			// Add the drawing operation to the queue
			if (graphics.userClip != null) {
				i.clip = new Area(graphics.userClip);
				i.clipList = new LinkedList(graphics.userClipList);
				i.xor = graphics.xor;
			}

			if (drawingHead != null) {
				drawingTail.next = i;
				drawingTail = i;
			}
			else {
				drawingHead = drawingTail = i;
			}

		}

	}

	/**
	 * The <code>QueueItem</code> object is a wrapper for queued
	 * <code>VCLGraphics</cade> drawing calls.
	 */
	final class PageQueueItem {

		Area clip = null;

		LinkedList clipList = null;

		Method method = null;

		PageQueueItem next = null;

		Object[] params = null;

		boolean polygonClip = false;

		boolean xor = false;

		PageQueueItem(Method m, Object[] p) {

			method = m;
			params = p;

		}

	}

	final static class CopyComposite implements Composite, CompositeContext {

		private Rectangle bounds = null;

		private ByteBuffer dataBuffer = null;

		private int dataWidth = 0;

		private int dataHeight = 0;

		public void compose(Raster src, Raster destIn, WritableRaster destOut) {

			if (destIn != destOut)
				destOut.setDataElements(0, 0, destIn);

			int w = destOut.getWidth();
			int h = destOut.getHeight();

			if (w > bounds.width)
				w = bounds.width;
			if (h > bounds.height)
				h = bounds.height;

			if (w > dataWidth - bounds.x)
				w = dataWidth - bounds.x;
			if (h > dataHeight - bounds.y)
				h = dataHeight - bounds.y;

			int[] destData = new int[w];
			dataWidth *= 4;
			int offset = (bounds.y * dataWidth) + bounds.x;
			for (int line = 0; line < h; line++) {
				destData = (int[])destIn.getDataElements(0, line, destData.length, 1, destData);
				dataBuffer.position(offset);
				for (int i = 0; i < destData.length; i++)
					dataBuffer.putInt(destData[i]);
				offset += dataWidth;
			}

			bounds = null;
			dataBuffer = null;
			dataWidth = 0;
			dataHeight = 0;

		}

		public CompositeContext createContext(ColorModel srcColorModel, ColorModel destColorModel, RenderingHints hints) {

			return this;

		}

		public void dispose() {}

		void setData(ByteBuffer d, Rectangle b, int w, int h) {

			bounds = b;
			dataBuffer = d;
			dataWidth = w;
			dataHeight = h;

		}

	}

	final static class CreateCopyComposite implements Composite, CompositeContext {

		private WritableRaster raster = null;

		public void compose(Raster src, Raster destIn, WritableRaster destOut) {

			if (destIn != destOut)
				destOut.setDataElements(0, 0, destIn);

			raster = destOut;

		}

		public CompositeContext createContext(ColorModel srcColorModel, ColorModel destColorModel, RenderingHints hints) {

			return this;

		}

		public void dispose() {}

		WritableRaster getRaster() {

			WritableRaster r = raster;
			raster = null;
			return r;

		}

	}

	final static class InvertComposite implements Composite, CompositeContext {

		public void compose(Raster src, Raster destIn, WritableRaster destOut) {

			if (destIn != destOut)
				destOut.setDataElements(0, 0, destIn);

			int w = destOut.getWidth();
			int h = destOut.getHeight();

			int[] srcData = new int[w];
			int[] destData = new int[w];
			for (int line = 0; line < h; line++) {
				srcData = (int[])src.getDataElements(0, line, srcData.length, 1, srcData);
				destData = (int[])destIn.getDataElements(0, line, destData.length, 1, destData);
				for (int i = 0; i < srcData.length && i < destData.length; i++) {
					if ((srcData[i] & 0xff000000) == 0xff000000)
						destData[i] = ~destData[i] | 0xff000000;
				}
				destOut.setDataElements(0, line, destData.length, 1, destData);
			}

		}

		public CompositeContext createContext(ColorModel srcColorModel, ColorModel destColorModel, RenderingHints hints) {

			return this;

		}

		public void dispose() {}

	}

	final static class XORImageComposite implements Composite, CompositeContext {

		private int color = Color.black.getRGB();

		public void compose(Raster src, Raster destIn, WritableRaster destOut) {

			if (destIn != destOut)
				destOut.setDataElements(0, 0, destIn);

			int w = destOut.getWidth();
			int h = destOut.getHeight();

			int[] srcData = new int[w];
			int[] destData = new int[w];
			for (int line = 0; line < h; line++) {
				srcData = (int[])src.getDataElements(0, line, srcData.length, 1, srcData);
				destData = (int[])destIn.getDataElements(0, line, destData.length, 1, destData);
				for (int i = 0; i < srcData.length && i < destData.length; i++) {
					if ((srcData[i] & 0xff000000) == 0xff000000)
						destData[i] = (destData[i] ^ color ^ srcData[i]) | 0xff000000;
				}
				destOut.setDataElements(0, line, destData.length, 1, destData);
			}

		}

		public CompositeContext createContext(ColorModel srcColorModel, ColorModel destColorModel, RenderingHints hints) {

			return this;

		}

		public void dispose() {}

		public void setXORMode(Color c) {

			color = c.getRGB();

		}

	}

	final static class DefaultableJButton extends JButton {

		private boolean isDefault = false;

		public boolean hasFocus() {

			return isDefault;

		}

		void setDefault(boolean b) {

			isDefault = b;

		}

	}

	final static class PolygonClipException extends AWTException {

		public PolygonClipException(String msg) {

			super(msg);

		}

	}

}
