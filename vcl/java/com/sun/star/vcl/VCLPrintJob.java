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
import java.awt.Frame;
import java.awt.Graphics2D;
import java.awt.JobAttributes;
import java.awt.PageAttributes;
import java.awt.PrintJob;
import java.awt.Rectangle;
import java.awt.Toolkit;
import java.awt.geom.AffineTransform;
import java.awt.print.PageFormat;
import java.awt.print.PrinterJob;

/**
 * The Java class that implements the SalPrinter C++ class methods.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public class VCLPrintJob {

	/** 
	 * ORIENTATION_PORTRAIT constant.
	 */
	public final static int ORIENTATION_PORTRAIT = 0x0; 

	/** 
	 * ORIENTATION_LANDSCAPE constant.
	 */
	public final static int ORIENTATION_LANDSCAPE = 0x1; 

	/**
	 * The cached frame.
	 */
	private static Frame frame = new Frame();

	/**
	 * The page format.
	 */
	private static PageFormat pageFormat = null;

	/**
	 * The printer job.
	 */
	private static PrinterJob printerJob = null;

	/**
	 * Get the imageable bounds of the page using a resolution of 72 dpi.
	 * Conversion to the real resolution is done in the <code>VCLGraphics</code>
	 * class.
	 *
	 * @return the imageable bounds of the page using a resolution of 72 dpi
	 */
	public static Rectangle getImageableBounds() {

		synchronized (pageFormat) {
			return new Rectangle((int)pageFormat.getImageableX(), (int)pageFormat.getImageableY(), (int)pageFormat.getImageableWidth(), (int)pageFormat.getImageableHeight());
		}

	}

	/**
	 * Get the page orientation.
	 *
	 * @return the page orientation
	 */
	public static int getOrientation() {

		synchronized (pageFormat) {
			if (pageFormat.getOrientation() == PageFormat.PORTRAIT)
				return ORIENTATION_PORTRAIT;
			else
				return ORIENTATION_LANDSCAPE;
		}

	}

	/**
	 * Get the page size using a resolution of 72 dpi. Conversion
	 * to the real resolution is done in the <code>VCLGraphics</code> class.
	 *
	 * @return the page size using a resolution of 72 dpi
	 */
	public static Dimension getPageSize() {

		synchronized (pageFormat) {
			return new Dimension((int)pageFormat.getWidth(), (int)pageFormat.getHeight());
		}

	}

	/**
	 * Setup the page configuration. This method displays a native page setup
	 * dialog and saves any changes made by the user.
	 */
	public static void setup() {

		synchronized (pageFormat) {
			pageFormat = printerJob.validatePage(printerJob.pageDialog(pageFormat));
		}

	}

	/**
	 * Initialize default printer settings.
	 */
	static {

		printerJob = PrinterJob.getPrinterJob();
		pageFormat = printerJob.validatePage(printerJob.defaultPage());

	}

	/**
	 * The current page.
	 */
	private int currentPage = 0;

	/**
	 * The cached job attributes.
	 */
	private JobAttributes jobAttributes = new JobAttributes();

	/**
	 * The cached page attributes.
	 */
	private PageAttributes pageAttributes = new PageAttributes();

	/**
	 * The current page's graphics.
	 */
	private VCLGraphics pageGraphics = null;

	/**
	 * The current page's image.
	 */
	private VCLImage pageImage = null;

	/**
	 * The page ranges to print.
	 */
	private int[][] pageRanges = null;

	/**
	 * The print job.
	 */
	private PrintJob printJob = null;

	/**
	 * Constructs a new <code>VCLPrintJob</code> instance.
	 */
	public VCLPrintJob() {

		if (pageFormat.getOrientation() == PageFormat.PORTRAIT)
			pageAttributes.setOrientationRequested(PageAttributes.OrientationRequestedType.PORTRAIT);
		else
			pageAttributes.setOrientationRequested(PageAttributes.OrientationRequestedType.LANDSCAPE);
		pageAttributes.setOrigin(PageAttributes.OriginType.PRINTABLE);

		// Create the print job
		printJob = Toolkit.getDefaultToolkit().getPrintJob(frame, "", jobAttributes, pageAttributes);

		// Note: this does not work on Mac OS X and is implemented in the
		// C++ class that calls this constructor
		if (jobAttributes.getDefaultSelection() == JobAttributes.DefaultSelectionType.RANGE)
			pageRanges = jobAttributes.getPageRanges();

	}

	/**
	 * Abort the printer job.
	 */
	public void abortJob() {

		// End the job before disposing of the graphics
		if (printJob != null)
			printJob.end();
		endPage();
		dispose();

	}

	/**
 	 * Disposes the printer job and releases any system resources that it is
	 * using.
	 */
	public void dispose() {

		currentPage = 0;
		jobAttributes = null;
		pageAttributes = null;
		// Make sure that the print job is ended
		if (printJob != null)
			printJob.end();
		printJob = null;
		if (pageGraphics != null)
			pageGraphics.dispose();
		pageGraphics = null;
		if (pageImage != null)
			pageImage.dispose();
		pageImage = null;
		pageRanges = null;

	}

	/**
	 * End the printer job.
	 */
	public void endJob() {

		// End the job after disposing of the graphics
		endPage();
		if (printJob != null)
			printJob.end();
		dispose();

	}

	/**
	 * Release the graphics for the current page.
	 */
	public void endPage() {

		if (pageGraphics != null)	
			pageGraphics.dispose();
		pageGraphics = null;
		if (pageImage != null)
			pageImage.dispose();
		pageImage = null;

	}

	/**
	 * Initialize the print job.
	 *
	 * @return <code>true</code> if a print job was successfully created or
	 *  <code>false</code> if the user cancelled the print dialog
	 */
	public boolean startJob() {

		// Detect if the user cancelled the print dialog
		if (printJob == null)
			return false;
		else
			return true;

	}

	/**
	 * Get the <code>VCLGraphics</code> instance for the current page.
	 *
	 * @return the <code>VCLGraphics</code> instance for the current page
	 */
	public VCLGraphics startPage() {

		// Increment the current page
		currentPage++;

		// Determine if this page is in the printable range
		boolean printPage = true;
		if (pageRanges != null) {
			printPage = false;
			for (int i = 0; i < pageRanges.length; i++) {
				if (currentPage >= pageRanges[i][0] && currentPage <= pageRanges[i][1]) {
					printPage = true;
					break;
				}
			}
		}

		// Get print graphics
		if (printPage && printJob != null) {
			Graphics2D g = (Graphics2D)printJob.getGraphics();
			if (g != null) {
				// Set the transform in case we are printing landscape
				g.transform(new AffineTransform(pageFormat.getMatrix()));
				pageGraphics = new VCLGraphics(g);
			}
		}

		// Fall back to a dummy graphics
		if (pageGraphics == null) {
			pageImage = new VCLImage(1, 1, 0);
			pageGraphics = pageImage.getGraphics();
		}

		return pageGraphics;

	}

}
