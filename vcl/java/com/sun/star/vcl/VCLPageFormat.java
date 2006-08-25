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
import java.awt.print.Paper;
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
	 * PAPER_A3 constant.
	 */
	public final static int PAPER_A3 = 0;

	/** 
	 * PAPER_A4 constant.
	 */
	public final static int PAPER_A4 = 1;

	/** 
	 * PAPER_A5 constant.
	 */
	public final static int PAPER_A5 = 2;

	/** 
	 * PAPER_B4 constant.
	 */
	public final static int PAPER_B4 = 3;

	/** 
	 * PAPER_B5 constant.
	 */
	public final static int PAPER_B5 = 4;

	/** 
	 * PAPER_LETTER constant.
	 */
	public final static int PAPER_LETTER = 5;

	/** 
	 * PAPER_LEGAL constant.
	 */
	public final static int PAPER_LEGAL = 6;

	/** 
	 * PAPER_TABLOID constant.
	 */
	public final static int PAPER_TABLOID = 7;

	/** 
	 * PAPER_USER constant.
	 */
	public final static int PAPER_USER = 8;

	/**
	 * The printer text resolution.
	 */
	private static int printerTextResolution = 300;

	/**
	 * Cached <code>VCLImage</code>.
	 */
	private VCLImage image = null;

	/**
	 * Cached printer job.
	 */
	private PrinterJob job = null;

	/**
	 * The page format.
	 */
	private PageFormat pageFormat = null;

	/**
	 * The paper orientation.
	 */
	private int paperOrientation = PageFormat.PORTRAIT;

	/**
	 * Constructs a new <code>VCLPageFormat</code> instance.
	 */
	public VCLPageFormat() {

		job = PrinterJob.getPrinterJob();
		pageFormat = job.defaultPage();
		pageFormat.setOrientation(paperOrientation);
		image = new VCLImage(1, 1, 32, this);

	}

	/**
	 * Disposes the page format and releases any system resources that it is
	 * using.
	 */
	protected void finalize() throws Throwable {

		if (image != null)
			image.dispose();
		image = null;
		job = null;
		pageFormat = null;

	}

	/**
	 * Returns the graphics context for this component.
	 *
	 * @return the graphics context for this component
	 */
	public VCLGraphics getGraphics() {

		return image.getGraphics();

	}

	/**
	 * Get the imageable bounds of the page in pixels.
	 *
	 * @return the imageable bounds of the page in pixels
	 */
	public Rectangle getImageableBounds() {

		if (paperOrientation == PageFormat.PORTRAIT)
			return new Rectangle((int)(pageFormat.getImageableX() * printerTextResolution / 72), (int)(pageFormat.getImageableY() * printerTextResolution / 72), (int)(pageFormat.getImageableWidth() * printerTextResolution / 72), (int)(pageFormat.getImageableHeight() * printerTextResolution / 72));
		else
			return new Rectangle((int)(pageFormat.getImageableY() * printerTextResolution / 72), (int)(pageFormat.getImageableX() * printerTextResolution / 72), (int)(pageFormat.getImageableHeight() * printerTextResolution / 72), (int)(pageFormat.getImageableWidth() * printerTextResolution / 72));

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
	 * Get the page resolution.
	 *
	 * @return the <code>VCLPageFormat</code> instance
	 */
	Dimension getPageResolution() {

		return new Dimension(printerTextResolution, printerTextResolution);

	}

	/**
	 * Get the page size in pixels.
	 *
	 * @return the page size in pixels
	 */
	public Dimension getPageSize() {

		if (paperOrientation == PageFormat.PORTRAIT)
			return new Dimension((int)(pageFormat.getWidth() * printerTextResolution / 72), (int)(pageFormat.getHeight() * printerTextResolution / 72));
		else
			return new Dimension((int)(pageFormat.getHeight() * printerTextResolution / 72), (int)(pageFormat.getWidth() * printerTextResolution / 72));

	}

	/**
	 * Get the paper orientation
	 *
	 * @return the paper orientation
	 */
	int getPaperOrientation() {

		return paperOrientation;

	}

	/**
	 * Get the paper type.
	 *
	 * @return the paper type
	 */
	public int getPaperType() {

		Paper paper = pageFormat.getPaper();
		long width;
		long height;
		if (paperOrientation == PageFormat.PORTRAIT) {
			width = Math.round(paper.getWidth());
			height = Math.round(paper.getHeight());
		}
		else {
			width = Math.round(paper.getHeight());
			height = Math.round(paper.getWidth());
		}

		if (width == 842 && height == 1191)
			return VCLPageFormat.PAPER_A3;
		else if (width == 595 && height == 842)
			return VCLPageFormat.PAPER_A4;
		else if (width == 420 && height == 595)
			return VCLPageFormat.PAPER_A5;
		else if (width == 709 && height == 1001)
			return VCLPageFormat.PAPER_B4;
		else if (width == 499 && height == 709)
			return VCLPageFormat.PAPER_B5;
		else if (width == 612 && height == 792)
			return VCLPageFormat.PAPER_LETTER;
		else if (width == 612 && height == 1008)
			return VCLPageFormat.PAPER_LEGAL;
		else if (width == 792 && height == 1224)
			return VCLPageFormat.PAPER_TABLOID;
		else
			return VCLPageFormat.PAPER_USER;

	}

	/**
	 * Returns the <code>PrinterJob</code> instance.
	 *
	 * @return the <code>PrinterJob</code> instance
	 */
	public PrinterJob getPrinterJob() {

		return job;

	}  

	/**
	 * Get the text resolution.
	 *
	 * @return the text resolution
	 */
	public Dimension getTextResolution() {

		return new Dimension(printerTextResolution, printerTextResolution);

	}

	/**
	 * Set the page orientation.
	 *
	 * @param o the page orientation
	 */
	public void setOrientation(int o) {

		if (o == ORIENTATION_PORTRAIT)
			pageFormat.setOrientation(PageFormat.PORTRAIT);
		else
			pageFormat.setOrientation(PageFormat.LANDSCAPE);

	}

	/**
	 * Update the page format.
	 *
	 * @param o the page orientation
	 */
	public void updatePageFormat(int o) {

		int oldOrientation = pageFormat.getOrientation();

		if (o == ORIENTATION_PORTRAIT)
			paperOrientation = PageFormat.PORTRAIT;
		else {
			paperOrientation = PageFormat.LANDSCAPE;
		}

		pageFormat = job.defaultPage();

		pageFormat.setOrientation(oldOrientation);

	}

}
