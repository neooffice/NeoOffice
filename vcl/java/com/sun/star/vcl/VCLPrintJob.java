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
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.geom.AffineTransform;
import java.awt.image.BufferedImage;
import java.awt.print.PageFormat;
import java.awt.print.Paper;
import java.awt.print.Printable;
import java.awt.print.PrinterException;
import java.awt.print.PrinterJob;

/**
 * The Java class that implements the SalPrinter C++ class methods.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public final class VCLPrintJob extends Thread implements Printable {

	/** 
	 * ORIENTATION_PORTRAIT constant.
	 */
	public final static int ORIENTATION_PORTRAIT = 0x0; 

	/** 
	 * ORIENTATION_LANDSCAPE constant.
	 */
	public final static int ORIENTATION_LANDSCAPE = 0x1; 

	/**
	 * SCALE_FACTOR constant.
	 */
	public final static int SCALE_FACTOR = 4;

	/**
	 * Cached native graphics.
	 */
	private static Graphics2D graphics = new BufferedImage(1, 1, BufferedImage.TYPE_INT_ARGB_PRE).createGraphics();

	/**
	 * The page format.
	 */
	private static PageFormat pageFormat = null;

	/**
	 * Creates a graphics context for this component.
	 *
	 * @return a graphics context for this component
	 */
	public synchronized static VCLGraphics getGraphics() {

		return new VCLGraphics(graphics, SCALE_FACTOR * 72, graphics.getDeviceConfiguration().getBounds());

	}

	/**
	/**
	 * Get the imageable bounds of the page in pixels.
	 *
	 * @return the imageable bounds of the page in pixels
	 */
	public synchronized static Rectangle getImageableBounds() {

		return new Rectangle((int)pageFormat.getImageableX() * SCALE_FACTOR, (int)pageFormat.getImageableY() * SCALE_FACTOR, (int)pageFormat.getImageableWidth() * SCALE_FACTOR, (int)pageFormat.getImageableHeight() * SCALE_FACTOR);

	}

	/**
	 * Get the page orientation.
	 *
	 * @return the page orientation
	 */
	public synchronized static int getOrientation() {

		if (pageFormat.getOrientation() == PageFormat.PORTRAIT)
			return ORIENTATION_PORTRAIT;
		else
			return ORIENTATION_LANDSCAPE;

	}

	/**
	 * Get the page size in pixels.
	 *
	 * @return the page size in pixels
	 */
	public synchronized static Dimension getPageSize() {

		return new Dimension((int)pageFormat.getWidth() * SCALE_FACTOR, (int)pageFormat.getHeight() * SCALE_FACTOR);

	}

	/**
	 * Set the page orientation.
	 *
	 * @param o the page orientation
	 */
	public synchronized static void setOrientation(int o) {

		if (o == ORIENTATION_PORTRAIT)
			pageFormat.setOrientation(PageFormat.PORTRAIT);
		else if (pageFormat.getOrientation() != PageFormat.REVERSE_LANDSCAPE)
			pageFormat.setOrientation(PageFormat.LANDSCAPE);

	}

	/**
	 * Setup the page configuration. This method displays a native page setup
	 * dialog and saves any changes made by the user.
	 *
	 * @return <code>false</code> if the user pressed the page dialog's
	 *  cancel button or else <code>true</code>
	 */
	public synchronized static boolean setup() {

		PageFormat p = PrinterJob.getPrinterJob().pageDialog(pageFormat);
		if (p != pageFormat) {
			pageFormat = p;
			return true;
		}
		else {
			return false;
		}

	}

	/**
	 * Initialize default printer settings.
	 */
	static {

		pageFormat = PrinterJob.getPrinterJob().defaultPage();

	}

	/**
	 * The current <code>VCLGraphics</code>
	 */
	private VCLGraphics currentGraphics = null;

	/**
	 * The current page.
	 */
	private int currentPage = 0;

	/**
	 * The end job flag.
	 */
	private boolean endJob = false;

	/**
	 * The cached graphics info.
	 */
	private VCLPrintJob.GraphicsInfo graphicsInfo = new VCLPrintJob.GraphicsInfo();

	/**
	 * The cached printer job.
	 */
	private PrinterJob job = PrinterJob.getPrinterJob();

	/**
	 * The print thread finished flag.
	 */
	private boolean printThreadFinished = false;

	/**
	 * The print thread started flag.
	 */
	private boolean printThreadStarted = false;

	/**
	 * Constructs a new <code>VCLPrintJob</code> instance.
	 */
	public VCLPrintJob() {}

	/**
	 * Abort the printer job.
	 */
	public void abortJob() {

		// End the job before disposing of the graphics
		job.cancel();
		endJob();

	}

	/**
 	 * Disposes the printer job and releases any system resources that it is
	 * using.
	 */
	public void dispose() {

		if (currentGraphics != null)
			currentGraphics.dispose();
		currentGraphics = null;
		currentPage = 0;
		if (graphicsInfo != null)
		{
			graphicsInfo.graphics = null;
			graphicsInfo.pageFormat = null;
			graphicsInfo.pageIndex = -1;
		}
		endJob = true;
		graphicsInfo = null;
		job = null;
		printThreadStarted = true;
		printThreadFinished = true;
		System.gc();

	}

	/**
	 * End the printer job.
	 */
	public void endJob() {

		// End the job after disposing of the graphics
		endJob = true;
		endPage();
		try {
			join();
		}
		catch (Throwable t) {}

	}

	/**
	 * Release the graphics for the current page.
	 */
	public void endPage() {

		synchronized (graphicsInfo) {
			if (currentGraphics == null) {
 				if (!endJob && !job.isCancelled())
					return;
			}
			else {
				currentGraphics.dispose();
				currentGraphics = null;
			}

			// Allow the printer thread to move to the next page
			graphicsInfo.notifyAll();
		}
		Thread.yield();

	}

	/**
	 * Prints the specified page into the specified <code>Graphics</code>
	 * context. This method will block until the <code>endPage</code> is
	 * invoked.
	 */
	public int print(Graphics g, PageFormat f, int i) throws PrinterException {

		if (job.isCancelled())
			throw new PrinterException();
		else if (endJob)
			return Printable.NO_SUCH_PAGE;

		// Mac OS X creates two graphics for each page so we need to create
		// separate page numbers for each page.
		if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX) {
			if (graphicsInfo.pageIndex == i * 2)
				graphicsInfo.pageIndex++;
			else
				graphicsInfo.pageIndex = i * 2;
		}
		else {
			graphicsInfo.pageIndex = i;
		}

		Graphics2D graphics = (Graphics2D)g;

		// Normalize to device resolution
		graphics.transform(graphics.getDeviceConfiguration().getNormalizingTransform());

		// Set the origin to the origin of the printable area
		graphics.translate((int)f.getImageableX(), (int)f.getImageableY());

		graphics.scale((double)1.0 / VCLPrintJob.SCALE_FACTOR, (double)1.0 / VCLPrintJob.SCALE_FACTOR);

		graphicsInfo.graphics = graphics;
		graphicsInfo.pageFormat = f;

		// Wait until painting is finished
		Thread.yield();
		try {
			graphicsInfo.wait();
		}
		catch (Throwable t) {}
		Thread.yield();

		if (job.isCancelled())
			throw new PrinterException();
		else if (endJob)
			return Printable.NO_SUCH_PAGE;
		else
			return Printable.PAGE_EXISTS;

	}

	/**
	 * This method invokes the <code>PrinterJob.print()</code> method. This
	 * method will block in each page until the <code>endPage</code> is
	 * invoked.
	 */
	public void run() {

		synchronized (graphicsInfo) {
			// Notify the thread that started this thread that it can proceed
			graphicsInfo.notifyAll();
			try {
				job.print();
			}
			catch (Throwable t) {}
			printThreadFinished = true;
		}

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
	 * Return the status of the print job.
	 *
	 * @return <code>true</code> if a print job has ended or aborted or
	 *  <code>false</code> if the print job is still running
	 */
	public boolean isFinished() {

		return (printThreadFinished || job.isCancelled());

	}

	/**
	 * Initialize the print job.
	 *
	 * @return <code>true</code> if a print job was successfully created or
	 *  <code>false</code> if the user cancelled the print dialog
	 */
	public boolean startJob() {

		// Copy the cached page format to this printer job
		job.setPrintable(this, (PageFormat)VCLPrintJob.pageFormat.clone());

		// Detect if the user cancelled the print dialog
		if (job.printDialog())
			return true;
		else
			return false;

	}

	/**
	 * Get the <code>VCLGraphics</code> instance for the current page.
	 *
	 * @return the <code>VCLGraphics</code> instance for the current page
	 */
	public VCLGraphics startPage() {

		synchronized (graphicsInfo) {
			// Start the printing thread if it has not yet been started
			if (!printThreadStarted) {
				start();
				// Wait for the printing thread to gain the lock on the
				// graphics queue
				try {
					graphicsInfo.wait();
				}
				catch (Throwable t) {}
				printThreadStarted = true;
			}
		}

		synchronized (graphicsInfo) {
			if (currentPage++ != graphicsInfo.pageIndex || !isAlive()) {
				// Return a dummy graphics if this page is not in the selected
				// page range
				currentGraphics = null;
			}
			else {
				currentGraphics = new VCLGraphics(graphicsInfo.graphics, VCLPrintJob.SCALE_FACTOR * 72, new Rectangle(0, 0, (int)graphicsInfo.pageFormat.getImageableWidth() * VCLPrintJob.SCALE_FACTOR, (int)graphicsInfo.pageFormat.getImageableHeight() * VCLPrintJob.SCALE_FACTOR));
				graphicsInfo.graphics = null;
				graphicsInfo.pageFormat = null;
			}
		}

		return currentGraphics;

	}

	class GraphicsInfo {

		Graphics2D graphics = null;

		PageFormat pageFormat = null;

		int pageIndex = -1;

	}

}
