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
import java.awt.print.PageFormat;
import java.awt.print.Printable;
import java.awt.print.PrinterException;
import java.awt.print.PrinterJob;

/**
 * The Java class that implements the SalPrinter C++ class methods.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public final class VCLPrintJob implements Printable, Runnable {

	/**
	 * The cached <code>VCLPageFormat</code>.
	 */
	private VCLPageFormat pageFormat = null;

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
	 * The print thread.
	 */
	private Thread printThread = null;

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
			graphicsInfo.pageIndex = -1;
		}
		endJob = true;
		graphicsInfo = null;
		job = null;
		printThread = null;
		printThreadStarted = true;
		printThreadFinished = true;

	}

	/**
	 * End the printer job.
	 */
	public void endJob() {

		// End the job after disposing of the graphics
		endJob = true;
		endPage();
		if (printThread != null) {
			try {
				printThread.join();
			}
			catch (Throwable t) {}
		}

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

 			if (printThreadFinished)
				return;

			// Allow the printer thread to move to the next page
			graphicsInfo.notifyAll();
			try {
				graphicsInfo.wait();
			}
			catch (Throwable t) {}
		}

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

		graphicsInfo.pageIndex = i;

		Graphics2D graphics = (Graphics2D)g;

		// Normalize to device resolution
		graphics.transform(graphics.getDeviceConfiguration().getNormalizingTransform());

		// Set the origin to the origin of the printable area
		graphics.translate((int)f.getImageableX(), (int)f.getImageableY());

		// Scale to printer resolution
		Dimension pageResolution = pageFormat.getPageResolution();
		graphics.scale((double)72 / pageResolution.width, (double)72 / pageResolution.height);

		synchronized (graphicsInfo) {
			graphicsInfo.graphics = graphics;

			// Notify other threads and wait until painting is finished
			graphicsInfo.notifyAll();
			try {
				graphicsInfo.wait();
			}
			catch (Throwable t) {}

			graphicsInfo.graphics = null;
		}

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

		try {
			job.print();
		}
		catch (Throwable t) {}
		printThreadFinished = true;

		// Notify other threads that printing is finished
		synchronized (graphicsInfo) {
			graphicsInfo.notifyAll();
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
	 * @param p the <code>VCLPageFormat</code>
	 * @return <code>true</code> if a print job was successfully created or
	 *  <code>false</code> if the user cancelled the print dialog
	 */
	public boolean startJob(VCLPageFormat p) {

		// Detect if the user cancelled the print dialog
		job.setPrintable(this, p.getPageFormat());
		if (job.printDialog()) {
			pageFormat = p;
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

		synchronized (graphicsInfo) {
			// Start the printing thread if it has not yet been started
			if (!printThreadStarted) {
				printThread = new Thread(this);
				printThread.setPriority(Thread.MIN_PRIORITY);
				printThread.start();
				// Wait for the printing thread to gain the lock on the
				// graphics queue
				try {
					graphicsInfo.wait();
				}
				catch (Throwable t) {}
				printThreadStarted = true;
			}

			// Get the current page's graphics context
			int page = currentPage++;
			// Mac OS X creates two graphics for each page so we need to
			// process each page twice
			if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX)
				page /= 2;
			if (page != graphicsInfo.pageIndex || printThread == null || !printThread.isAlive()) {
				// Return a dummy graphics if this page is not in the selected
				// page range
				currentGraphics = null;
			}
			else {
				currentGraphics = new VCLGraphics(graphicsInfo.graphics, pageFormat);
			}
			// On Mac OS X, skip the first graphics for each page
			if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX && currentPage % 2 != 0)
				return null;
		}

		return currentGraphics;

	}

	class GraphicsInfo {

		Graphics2D graphics = null;

		int pageIndex = -1;

	}

}
