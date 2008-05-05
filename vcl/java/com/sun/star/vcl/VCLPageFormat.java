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
	 * The paper rotated flag.
	 */
	private boolean paperRotated = false;

	/**
	 * The printer resolution.
	 */
	private Dimension printerResolution = null;

	/**
	 * Constructs a new <code>VCLPageFormat</code> instance.
	 */
	public VCLPageFormat() {

		job = PrinterJob.getPrinterJob();

		// Avoid a costly call if this is a network printer by setting a
		// generic page size and then setting the desired size using the
		// updatePageFormat() method
		pageFormat = new PageFormat();

		// We always set the page format to portrait as all of the fixes for
		// for bugs 2202 depend on this
		pageFormat.setOrientation(PageFormat.PORTRAIT);
		image = new VCLImage(1, 1, 32, this);
		printerResolution = image.getGraphics().getResolution();

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

		int width = (int)(pageFormat.getImageableWidth() * printerResolution.width / 72);
		int height = (int)(pageFormat.getImageableHeight() * printerResolution.height / 72);
		int x = (int)((pageFormat.getWidth() - pageFormat.getImageableX() - pageFormat.getImageableWidth()) * printerResolution.width / 72);
		int y = (int)((pageFormat.getHeight() - pageFormat.getImageableY() - pageFormat.getImageableHeight()) * printerResolution.height / 72);

		// Part of fix for bug 2202: Always return the portrait paper settings
		if (paperRotated)
			return new Rectangle(y, x, height, width);
		else
			return new Rectangle(x, y, width, height);

	}

	/**
	 * Get the page orientation.
	 *
	 * @return the page orientation
	 */
	public int getOrientation() {

		if ((!paperRotated && paperOrientation == PageFormat.PORTRAIT) || (paperRotated && paperOrientation != PageFormat.PORTRAIT))
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

		// Part of fix for bug 2202: Always return the portrait paper settings
		if (paperRotated)
			return new Dimension((int)(pageFormat.getHeight() * printerResolution.height / 72), (int)(pageFormat.getWidth() * printerResolution.width / 72));
		else
			return new Dimension((int)(pageFormat.getWidth() * printerResolution.width / 72), (int)(pageFormat.getHeight() * printerResolution.height/ 72));

	}

	/**
	 * Get the paper type.
	 *
	 * @return the paper type
	 */
	public int getPaperType() {

		// Part of fix for bug 2202: Always return the portrait paper settings
		Paper paper = pageFormat.getPaper();
		long width;
		long height;
		if ((!paperRotated && paperOrientation == PageFormat.PORTRAIT) || (paperRotated && paperOrientation != PageFormat.PORTRAIT)) {
			width = Math.round(paper.getWidth());
			height = Math.round(paper.getHeight());
		}
		else {
			width = Math.round(paper.getHeight());
			height = Math.round(paper.getWidth());
		}

		if (Math.abs(width - 842) < 2 && Math.abs(height - 1191) < 2)
			return VCLPageFormat.PAPER_A3;
		else if (Math.abs(width - 595) < 2 && Math.abs(height - 842) < 2)
			return VCLPageFormat.PAPER_A4;
		else if (Math.abs(width - 420) < 2 && Math.abs(height - 595) < 2)
			return VCLPageFormat.PAPER_A5;
		else if (Math.abs(width - 709) < 2 && Math.abs(height - 1001) < 2)
			return VCLPageFormat.PAPER_B4;
		else if (Math.abs(width - 499) < 2 && Math.abs(height - 709) < 2)
			return VCLPageFormat.PAPER_B5;
		else if (Math.abs(width - 612) < 2 && Math.abs(height - 792) < 2)
			return VCLPageFormat.PAPER_LETTER;
		else if (Math.abs(width - 612) < 2 && Math.abs(height - 1008) < 2)
			return VCLPageFormat.PAPER_LEGAL;
		else if (Math.abs(width - 792) < 2 && Math.abs(height - 1224) < 2)
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
	 * Get the resolution in pixels per inch.
	 *
	 * @return the resolution in pixels per inch
	 */
	public Dimension getResolution() {

		return printerResolution;

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
	 * Returns the paper rotated state
	 *
	 * @return the paper rotated state
	 */
	public boolean isPaperRotated() {

		return paperRotated;

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
	 * @param w the width to set the paper to
	 * @param h the height to set the paper to
	 * @param ix the x coordinate to set the paper's imageable area to
	 * @param iy the y coordinate to set the paper's imageable area to
	 * @param iw the height to set the paper's imageable area to
	 * @param ih the width to set the paper's imageable area to
	 */
	public void updatePageFormat(int o, float w, float h, float ix, float iy, float iw, float ih) {

		if (!editable)
			return;

		Paper p = new Paper();
		p.setSize(w, h);
		p.setImageableArea(ix, iy, iw, ih);
		pageFormat.setPaper(p);

		if (o == ORIENTATION_PORTRAIT)
			paperRotated = false;
		else
			paperRotated = true;

	}

}
