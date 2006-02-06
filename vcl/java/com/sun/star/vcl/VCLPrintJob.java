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
import java.awt.print.PrinterAbortException;
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
	 * The cached <code>VCLGraphics</code>.
	 */
	private VCLGraphics graphics = null;

	/**
	 * The disposed flag.
	 */
	private boolean disposed = false;

	/**
	 * The end job flag.
	 */
	private boolean endJob = false;

	/**
	 * The cached printer job.
	 */
	private PrinterJob job = null;

	/**
	 * The job started flag.
	 */
	private boolean jobStarted = false;

	/**
	 * The last page queue.
	 */
	VCLGraphics.PageQueue lastPageQueue = null;

	/**
	 * The cached <code>VCLPageFormat</code>.
	 */
	private VCLPageFormat pageFormat = null;

	/**
	 * The cached native graphics context.
	 */
	private Graphics2D printGraphics = null;

	/**
	 * The cached native page format.
	 */
	private PageFormat printPageFormat = null;

	/**
	 * The event queue.
	 */
	private VCLEventQueue queue = null;

	/**
	 * The print started flag.
	 */
	private boolean printStarted = false;

	/**
	 * The print thread.
	 */
	private Thread printThread = null;

	/**
	 * The cached scale factor.
	 */
	private float scale = 1.0f;

	/**
	 * Constructs a new <code>VCLPrintJob</code> instance.
	 */
	public VCLPrintJob(VCLEventQueue q) {

		queue = q;

	}

	/**
	 * Abort the printer job.
	 */
	public void abortJob() {

		// Due to a bug in some JVM versions, cancelling a native print job
		// once the PrinterJob.print() method is invoked either deadlocks the
		// JVM or does not cancel the job so we do not allow cancellation
		endJob();


	}

	/**
 	 * Disposes the printer job and releases any system resources that it is
	 * using.
	 */
	public synchronized void dispose() {

		if (disposed)
			return;

		abortJob();

		// Don't dispose the graphics as we only want to draw in the
		// printing thread
		graphics = null;
		if (lastPageQueue != null) {
			lastPageQueue.dispose();
			lastPageQueue = null;
		}
		job = null;
		printGraphics = null;
		printPageFormat = null;
		printThread = null;

		disposed = true;

	}

	/**
	 * End the printer job.
	 */
	public synchronized void endJob() {

		if (endJob)
			return;

		// End the job after disposing of the printGraphics
		endJob = true;

		// Wait for print thread to finish
		while (printStarted && printThread != null) {
			notifyAll();
			try {
				wait(10);
			}
			catch (Throwable t) {}
		}

		if (pageFormat != null)
			pageFormat.setEditable(true);

		queue.setPrinting(false);

	}

	/**
	 * Release the printGraphics for the current page.
	 */
	public synchronized void endPage() {

		if (printStarted && printThread != null) {
			notifyAll();
			try {
				wait();
			}
			catch (Throwable t) {}
		}

	}

	/**
	 * Prints the specified page into the specified <code>Graphics</code>
	 * context. This method will block until the <code>endPage</code> is
	 * invoked.
	 */
	public synchronized int print(Graphics g, PageFormat f, int i) throws PrinterException {

		if (!printStarted) {
			printStarted = true;
			notifyAll();
		}

		if (lastPageQueue != null) {
			lastPageQueue.dispose();
			lastPageQueue = null;
		}

		printGraphics = (Graphics2D)g;
		printPageFormat = f;
		printGraphics.scale(scale, scale);

		// Notify other threads and wait until painting is finished
		if (!endJob) {
			notifyAll();
			try {
				wait();
			}
			catch (Throwable t) {}
		}

		if (graphics != null) {
			// Cache the queued drawing operations. This is necessary
			// because the JVM will crash if we are printing bitmaps
			// and those bitmaps are garbage collected before the JVM
			// finishes the print job.
			lastPageQueue = graphics.getPageQueue();
			graphics.dispose();
			graphics = null;
		}

		printGraphics = null;
		printPageFormat = null;

		if (endJob)
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
		catch (Throwable t) {
			t.printStackTrace();
		}

		// Notify other threads that printing is finished
		synchronized (this) {
			// Don't dispose the graphics as we only want to draw in the
			// printing thread
			graphics = null;
			if (lastPageQueue != null) {
				lastPageQueue.dispose();
				lastPageQueue = null;
			}
			printGraphics = null;
			printPageFormat = null;
			printThread = null;
			notifyAll();
		}

	}

	/**
	 * Return the status of the print job.
	 *
	 * @return <code>true</code> if a print job has ended or aborted or
	 *  <code>false</code> if the print job is still running
	 */
	public synchronized boolean isFinished() {

		return (printStarted && printThread == null);

	}

	/**
	 * Initialize the print job.
	 *
	 * @param p the <code>VCLPageFormat</code>
	 * @param n the job name
	 * @param s the scale factor
	 * @return <code>true</code> if the print job is properly setup otherwise
	 *  <code>false</code>
	 */
	public boolean startJob(VCLPageFormat p, String n, float s) {

		if (!jobStarted) {
			pageFormat = p;
			scale = s;
			job = pageFormat.getPrinterJob();
			job.setPrintable(this, pageFormat.getPageFormat());
			job.setJobName(n);
			pageFormat.setEditable(false);
			jobStarted = true;
		}

		queue.setPrinting(jobStarted);

		return jobStarted;

	}

	/**
	 * Get the <code>VCLGraphics</code> instance for the current page.
	 *
	 * @param o the page orientation
	 * @return the <code>VCLGraphics</code> instance for the current page
	 */
	public synchronized VCLGraphics startPage(int o) {

		// Start the printing thread if it has not yet been started
		if (!printStarted && printThread == null) {
			printStarted = false;
			printThread = new Thread(this);
			printThread.start();
			try {
				wait();
			}
			catch (Throwable t) {}
		}

		// Mac OS X wants each page printed twice so skip the first as it
		// seems to have no effect on the printed output
		endPage();

		// Get the current page's printGraphics context
		if (printStarted && printThread != null) {
			// Set the origin to the origin of the printable area
			printGraphics.translate((int)printPageFormat.getImageableX(), (int)printPageFormat.getImageableY());

			// Rotate the page if necessary
			int orientation = printPageFormat.getOrientation();
			boolean rotatedPage = false;
			if (o == VCLPageFormat.ORIENTATION_PORTRAIT && orientation != PageFormat.PORTRAIT) {
				if (orientation == PageFormat.REVERSE_LANDSCAPE) {
					printGraphics.translate(0, (int)printPageFormat.getImageableHeight());
					printGraphics.rotate(Math.toRadians(-90));
				}
				else {
					printGraphics.translate((int)printPageFormat.getImageableWidth(), 0);
					printGraphics.rotate(Math.toRadians(90));
				}
				rotatedPage = true;
			}
			else if (o != VCLPageFormat.ORIENTATION_PORTRAIT && orientation == PageFormat.PORTRAIT ) {
				printGraphics.translate(0, (int)printPageFormat.getImageableHeight());
				printGraphics.rotate(Math.toRadians(-90));
				rotatedPage = true;
			}

			// Scale to printer resolution
			Dimension pageResolution = pageFormat.getPageResolution();
			if (rotatedPage)
				printGraphics.scale((double)72 / pageResolution.height, (double)72 / pageResolution.width);
			else
				printGraphics.scale((double)72 / pageResolution.width, (double)72 / pageResolution.height);

			graphics = new VCLGraphics(printGraphics, pageFormat, rotatedPage);
		}
		else {
			graphics = null;
		}

		return graphics;
	}

}
