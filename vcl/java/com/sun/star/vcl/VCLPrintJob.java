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
import java.awt.print.PageFormat;
import java.awt.print.Printable;
import java.awt.print.PrinterException;
import java.awt.print.PrinterJob;
import java.util.LinkedList;

/**
 * The Java class that implements the SalPrinter C++ class methods.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public class VCLPrintJob extends Thread implements Printable {

	/** 
	 * ORIENTATION_PORTRAIT constant.
	 */
	public final static int ORIENTATION_PORTRAIT = 0x0; 

	/** 
	 * ORIENTATION_LANDSCAPE constant.
	 */
	public final static int ORIENTATION_LANDSCAPE = 0x1; 

	/**
	 * The page format.
	 */
	private static PageFormat pageFormat = null;

	/**
	 * Get the imageable bounds of the page using a resolution of 72 dpi.
	 * Conversion to the real resolution is done in the <code>VCLGraphics</code>
	 * class.
	 *
	 * @return the imageable bounds of the page using a resolution of 72 dpi
	 */
	public synchronized static Rectangle getImageableBounds() {

		return new Rectangle((int)pageFormat.getImageableX(), (int)pageFormat.getImageableY(), (int)pageFormat.getImageableWidth(), (int)pageFormat.getImageableHeight());

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
	 * Get the page size using a resolution of 72 dpi. Conversion
	 * to the real resolution is done in the <code>VCLGraphics</code> class.
	 *
	 * @return the page size using a resolution of 72 dpi
	 */
	public synchronized static Dimension getPageSize() {

		return new Dimension((int)pageFormat.getWidth(), (int)pageFormat.getHeight());

	}

	/**
	 * Setup the page configuration. This method displays a native page setup
	 * dialog and saves any changes made by the user.
	 */
	public synchronized static void setup() {

		pageFormat = PrinterJob.getPrinterJob().pageDialog(pageFormat);

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
	 * The current printer job page.
	 */
	private int currentJobPage = -1;

	/**
	 * The current page.
	 */
	private int currentPage = 0;

	/**
	 * The end job flag.
	 */
	private boolean endJob = false;

	/**
	 * The cached queue of graphics.
	 */
	private LinkedList graphicsQueue = new LinkedList();

	/**
	 * The cached printer job.
	 */
	private PrinterJob job = PrinterJob.getPrinterJob();

	/**
	 * The current page's image.
	 */
	private VCLImage pageImage = null;

	/**
	 * Constructs a new <code>VCLPrintJob</code> instance.
	 */
	public VCLPrintJob() {

		// Copy the cached page format to this printer job
		job.setPrintable(this, VCLPrintJob.pageFormat);

	}

	/**
	 * Abort the printer job.
	 */
	public void abortJob() {

		// End the job before disposing of the graphics
		job.cancel();
		endPage();
		try {
			join();
		}
		catch (Throwable t) {}
		dispose();

	}

	/**
 	 * Disposes the printer job and releases any system resources that it is
	 * using.
	 */
	public void dispose() {

		if (currentGraphics != null)
			currentGraphics.dispose();
		currentGraphics = null;
		currentJobPage = -1;
		currentPage = 0;
		endJob = false;
		while (graphicsQueue.size() > 0)
			graphicsQueue.removeFirst();
		job = null;
		if (pageImage != null)
			pageImage.dispose();
		pageImage = null;

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
		dispose();

	}

	/**
	 * Release the graphics for the current page.
	 */
	public void endPage() {

		synchronized (graphicsQueue) {
			if (currentGraphics != null)
				currentGraphics.dispose();
			currentGraphics = null;
			if (pageImage != null) {
				pageImage.dispose();
				pageImage = null;
				return;
			}
			// Allow the printer thread to move to the next page
			graphicsQueue.notifyAll();
		}
		Thread.currentThread().yield();

	}

	/**
	 * Prints the specified page into the specified <code>Graphics</code>
	 * context. This method will block until the <code>endPage</code> is
	 * invoked.
	 */
	public int print(Graphics g, PageFormat f, int i) throws PrinterException {

		if (endJob)
			return Printable.NO_SUCH_PAGE;
		else if (job.isCancelled())
			throw new PrinterException();

		// Mac OS X creates two graphics for each page so we need to create
		// separate page numbers for each page.
		if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX) {
			if (currentJobPage == i * 2) 
				currentJobPage++;
			else
				currentJobPage = i * 2;
		}
		else {
			currentJobPage = i;
		}

		graphicsQueue.add(g);

		// Wait until painting is finished
		try {
			graphicsQueue.wait();
		}
		catch (Throwable t) {}
	
		if (endJob)
			return Printable.NO_SUCH_PAGE;
		else if (job.isCancelled())
			throw new PrinterException();
		else
			return Printable.PAGE_EXISTS;

	}

	/**
	 * This method invokes the <code>PrinterJob.print()</code> method. This
	 * method will block in each page until the <code>endPage</code> is
	 * invoked.
	 */
	public void run() {

		synchronized (graphicsQueue) {
			// Notify the thread that started this thread that it can proceed
			graphicsQueue.notifyAll();
			try {
				job.print();
			}
			catch (Throwable t) {}
		}
		
	}

	/**
	 * Initialize the print job.
	 *
	 * @return <code>true</code> if a print job was successfully created or
	 *  <code>false</code> if the user cancelled the print dialog
	 */
	public boolean startJob() {

		// Detect if the user cancelled the print dialog
		if (job.printDialog()) {
			synchronized (graphicsQueue) {
				// Start the printing thread
				setPriority(Thread.MAX_PRIORITY);
				start();
				// Wait for the printing thread to gain the lock on the
				// graphics queue
				try {
					graphicsQueue.wait();
				}
				catch (Throwable t) {}
			}
			return true;
		}
		else {
			return false;
		}

	}

	/**
	 * Get the <code>VCLGraphics</code> instance for the current page.
	 *
	 * @return the <code>VCLGraphics</code> instance for the current page
	 */
	public VCLGraphics startPage() {

		synchronized (graphicsQueue) {
			if (currentPage++ != currentJobPage || !isAlive()) {
				// Return a dummy graphics if this page is not in the selected
				// page range
				pageImage = new VCLImage(1, 1, 0);
				currentGraphics = pageImage.getGraphics();
			}
			else {
				currentGraphics = new VCLGraphics((Graphics2D)graphicsQueue.getFirst());
				graphicsQueue.removeFirst();
			}
		}

		return currentGraphics;

	}

}
