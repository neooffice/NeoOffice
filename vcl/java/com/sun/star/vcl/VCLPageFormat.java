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

import java.awt.Dimension;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.image.BufferedImage;
import java.awt.print.PageFormat;
import java.awt.print.PrinterJob;

/**
 * The Java class that implements the SalPrinter C++ class methods.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public final class VCLPageFormat {

	/** 
	 * ORIENTATION_PORTRAIT constant.
	 */
	public final static int ORIENTATION_PORTRAIT = 0x0; 

	/** 
	 * ORIENTATION_LANDSCAPE constant.
	 */
	public final static int ORIENTATION_LANDSCAPE = 0x1; 

	/**
	 * Cached native graphics.
	 */
	private Graphics2D graphics = null;

	/**
	 * The page format.
	 */
	private PageFormat pageFormat = null;

	/**
	 * The page resolution.
	 */
	private Dimension pageResolution = null;

	/**
	 * Constructs a new <code>VCLPageFormat</code> instance.
	 */
	public VCLPageFormat() {

		graphics = new BufferedImage(1, 1, BufferedImage.TYPE_INT_ARGB_PRE).createGraphics();
		pageFormat = PrinterJob.getPrinterJob().defaultPage();
		pageResolution = new Dimension(72, 72);

	}

	/**
 	 * Disposes the page format and releases any system resources that it is
	 * using.
	 */
	public void dispose() {

		if (graphics != null)
			graphics.dispose();
		graphics = null;
		pageFormat = null;
		pageResolution = null;

	}

	/**
	 * Returns the graphics context for this component.
	 *
	 * @return the graphics context for this component
	 */
	public VCLGraphics getGraphics() {

		return new VCLGraphics(graphics, graphics.getDeviceConfiguration().getBounds(), this);

	}

	/**
	 * Get the imageable bounds of the page in pixels.
	 *
	 * @return the imageable bounds of the page in pixels
	 */
	public Rectangle getImageableBounds() {

		return new Rectangle((int)(pageFormat.getImageableX() * pageResolution.width / 72), (int)(pageFormat.getImageableY() * pageResolution.height / 72), (int)(pageFormat.getImageableWidth() * pageResolution.width / 72), (int)(pageFormat.getImageableHeight() * pageResolution.height / 72));

	}

	/**
	 * Get the page orientation.
	 *
	 * @return the page orientation
	 */
	public int getOrientation() {

		if (pageFormat.getOrientation() == PageFormat.PORTRAIT)
			return ORIENTATION_PORTRAIT;
		else
			return ORIENTATION_LANDSCAPE;

	}

	/**
	 * Returns the page format for this component.
	 *
	 * @return the page format for this component
	 */
	PageFormat getPageFormat() {

		return pageFormat;

	}

	/**
	 * Get the page size in pixels.
	 *
	 * @return the page size in pixels
	 */
	public Dimension getPageSize() {

		return new Dimension((int)(pageFormat.getWidth() * pageResolution.width / 72), (int)(pageFormat.getHeight() * pageResolution.height / 72));

	}

	/**
	 * Get the page resolution.
	 *
	 * @return the page resolution.
	 */
	Dimension getPageResolution() {

		if (pageFormat.getOrientation() == PageFormat.PORTRAIT)
			return new Dimension(pageResolution.width, pageResolution.height);
		else
			return new Dimension(pageResolution.height, pageResolution.width);

	}

	/**
	 * Set the page orientation.
	 *
	 * @param o the page orientation
	 */
	public void setOrientation(int o) {

		if (o == ORIENTATION_PORTRAIT)
			pageFormat.setOrientation(PageFormat.PORTRAIT);
		else if (pageFormat.getOrientation() != PageFormat.REVERSE_LANDSCAPE)
			pageFormat.setOrientation(PageFormat.LANDSCAPE);

	}

	/**
	 * Set the page resolution.
	 *
	 * @param h the horizontal page resolution
	 * @param v the vertical page resolution
	 */
	public void setPageResolution(int h, int v) {

		pageResolution = new Dimension(h, v);

	}

	/**
	 * Setup the page configuration. This method displays a native page setup
	 * dialog and saves any changes made by the user.
	 *
	 * @return <code>false</code> if the user pressed the page dialog's
	 *  cancel button or else <code>true</code>
	 */
	public boolean setup() {

		PageFormat p = PrinterJob.getPrinterJob().pageDialog(pageFormat);
		if (p != pageFormat) {
			pageFormat = p;
			return true;
		}
		else {
			return false;
		}

	}

}
