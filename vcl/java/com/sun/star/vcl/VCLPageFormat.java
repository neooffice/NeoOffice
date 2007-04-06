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
	 * Set the paper dimensions to the specified values.
	 *
	 * @param p the paper
	 * @param w the width to set the paper to
	 * @param h the height to set the paper to
	 * @param ix the x coordinate to set the paper's imageable area to
	 * @param iy the y coordinate to set the paper's imageable area to
	 * @param iw the height to set the paper's imageable area to
	 * @param ih the width to set the paper's imageable area to
	 */
	public static void validatePaper(Paper p, float w, float h, float ix, float iy, float iw, float ih) {

		p.setSize(w, h);
		p.setImageableArea(ix, iy, iw, ih);

	}

	/**
	 * The editable flag.
	 */
	private boolean editable = true;
 
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
		try {
			pageFormat = job.validatePage(pageFormat);
		}
		catch (Throwable t) {}
		// We always set the page format to portrait as all of the fixes for
		// for bugs 2202 depend on this
		pageFormat.setOrientation(PageFormat.PORTRAIT);
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

		int width = (int)(pageFormat.getImageableWidth() * printerTextResolution / 72);
		int height = (int)(pageFormat.getImageableHeight() * printerTextResolution / 72);
		int x = (int)((pageFormat.getWidth() - pageFormat.getImageableX() - pageFormat.getImageableWidth()) * printerTextResolution / 72);
		int y = (int)((pageFormat.getHeight() - pageFormat.getImageableY() - pageFormat.getImageableHeight()) * printerTextResolution / 72);

		// Part of fix for bug 2202: Always return the portrait paper settings
		return new Rectangle(x, y, width, height);

	}

	/**
	 * Get the page orientation.
	 *
	 * @return the page orientation
	 */
	public int getOrientation() {

		if (paperOrientation == PageFormat.PORTRAIT)
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

		// Part of fix for bug 2202: Always return the portrait paper settings
		return new Dimension((int)(pageFormat.getWidth() * printerTextResolution / 72), (int)(pageFormat.getHeight() * printerTextResolution / 72));

	}

	/**
	 * Get the paper type.
	 *
	 * @return the paper type
	 */
	public int getPaperType() {

		// Part of fix for bug 2202: Always return the portrait paper settings
		Paper paper = pageFormat.getPaper();
		long width = Math.round(paper.getWidth());
		long height = Math.round(paper.getHeight());

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
	 * Returns the editable state
	 *
	 * @return the editable state
	 */
	public boolean isEditable() {

		return editable;

	}

	/**
	 * Set the number of copies.
	 *
	 * @param n the number of copies
	 */
	public void setCopies(int n) {

		if (!editable)
			return;

		job.setCopies(n);

	}

	/**
	 * Set the editability of this component.
	 *
	 * @param b <code>true</code> to make this component editable or else
	 *  <code>false</code>
	 */
	void setEditable(boolean b) {

		editable = b;

	}

	/**
	 * Set the page orientation.
	 *
	 * @param o the page orientation
	 */
	public void setOrientation(int o) {

		if (o == ORIENTATION_PORTRAIT)
			paperOrientation = PageFormat.PORTRAIT;
		else
			paperOrientation = PageFormat.LANDSCAPE;

	}

	/**
	 * Update the page format.
	 *
	 * @param o the page orientation
	 */
	public void updatePageFormat(int o) {

		if (!editable)
			return;

		pageFormat = job.defaultPage();
		try {
			pageFormat = job.validatePage(pageFormat);
		}
		catch (Throwable t) {}

		setOrientation(o);

	}

}
