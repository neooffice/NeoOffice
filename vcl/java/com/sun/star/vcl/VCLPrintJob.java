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
import java.util.LinkedList;

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
	 * The end page flag.
	 */
	private boolean endPage = false;

	/**
	 * The cached printer job.
	 */
	private PrinterJob job = null;

	/**
	 * The job started flag.
	 */
	private boolean jobStarted = false;

	/**
	 * The cached <code>VCLPageFormat</code>.
	 */
	private VCLPageFormat pageFormat = null;

	/**
	 * The cached printed page queues.
	 */
	LinkedList printedPageQueues = null;

	/**
	 * The cached native graphics context.
	 */
	private Graphics2D printGraphics = null;

	/**
	 * The cached native page format.
	 */
	private PageFormat printPageFormat = null;

	/**
	 * The print thread.
	 */
	private Thread printThread = null;

	/**
	 * Constructs a new <code>VCLPrintJob</code> instance.
	 */
	public VCLPrintJob() {}

	/**
	 * Abort the printer job.
	 */
	public void abortJob() {

		// End the job before disposing of the printGraphics
		job.cancel();
		endJob();

	}

	/**
 	 * Disposes the printer job and releases any system resources that it is
	 * using.
	 */
	public void dispose() {

		if (disposed)
			return;

		abortJob();

		synchronized (this) {
			if (graphics != null) {
				// Dispose a print printGraphics throws exceptions when memory
				// is low
				try {
					graphics.dispose();
				}
				catch (Throwable t) {
					t.printStackTrace();
				}
				graphics = null;
			}

			printGraphics = null;
			printPageFormat = null;

			if (printedPageQueues != null) {
				while (printedPageQueues.size() > 0) {
					VCLGraphics.PageQueue pq = (VCLGraphics.PageQueue)printedPageQueues.removeFirst();
					pq.dispose();
				}
				printedPageQueues = null;
			}

			job = null;
			printThread = null;

			disposed = true;
		}

	}

	/**
	 * End the printer job.
	 */
	public void endJob() {

		// End the job after disposing of the printGraphics
		endJob = true;

		endPage();

		// Release any pending locks
		if (printThread != null && printThread.isAlive()) {
			try {
				printThread.join();
			}
			catch (Throwable t) {}
		}

		pageFormat.setEditable(true);
		jobStarted = false;

	}

	/**
	 * Release the printGraphics for the current page.
	 */
	public void endPage() {

		synchronized (this) {
			// Allow the printer thread to move to the next page
			endPage = true;

			while (endPage && printThread != null && printThread.isAlive()) {
				notifyAll();
				try {
					wait(100);
				}
				catch (Throwable t) {}
			}

			endPage = false;
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
	 * Prints the specified page into the specified <code>Graphics</code>
	 * context. This method will block until the <code>endPage</code> is
	 * invoked.
	 */
	public int print(Graphics g, PageFormat f, int i) throws PrinterException {

		synchronized (this) {
			if (job.isCancelled())
				throw new PrinterAbortException();
			else if (endJob)
				return Printable.NO_SUCH_PAGE;

			printGraphics = (Graphics2D)g;
			printPageFormat = f;

			// Notify other threads and wait until painting is finished
			while (!endPage) {
				runNativeTimers();
				notifyAll();
				try {
					wait(100);
				}
				catch (Throwable t) {}
			}

			// Allow endPage() method to continue
			endPage = false;

			runNativeTimers();

			if (graphics != null) {
				// Cache the queued drawing operations. This is necessary
				// because the JVM will crash if we are printing bitmaps
				// and those bitmaps are garbage collected before the JVM
				// finishes the print job.
				printedPageQueues.add(graphics.getPageQueue());

				try {
					graphics.dispose();
				}
				catch (Throwable t) {
					t.printStackTrace();
				}
	
				graphics = null;
			}

			runNativeTimers();

			printGraphics = null;
			printPageFormat = null;

			if (job.isCancelled())
				throw new PrinterAbortException();
			else if (endJob)
				return Printable.NO_SUCH_PAGE;
			else
				return Printable.PAGE_EXISTS;
		}

	}

	/**
	 * This method invokes the <code>PrinterJob.print()</code> method. This
	 * method will block in each page until the <code>endPage</code> is
	 * invoked.
	 */
	public void run() {

		try {
			synchronized (this) {
				runNativeTimers();
				if (printedPageQueues == null)
					printedPageQueues = new LinkedList();
			}

			job.print();
		}
		catch (PrinterAbortException pae) {
			// Don't print anything since the user pressed the cancel button
		}
		catch (Throwable t) {
			t.printStackTrace();
		}

		// Notify other threads that printing is finished
		synchronized (this) {
			runNativeTimers();

			if (printedPageQueues != null) {
				while (printedPageQueues.size() > 0) {
					VCLGraphics.PageQueue pq = (VCLGraphics.PageQueue)printedPageQueues.removeFirst();
					pq.dispose();
				}
				printedPageQueues = null;
			}

			notifyAll();
		}

	}

	/**
	 * Run any native timers that are pending. This method needs to be called
	 * within the {@link #print(Graphics, PageFormat, int)} method as that
	 * method is run by the JVM in a native timer.
	 */
	native void runNativeTimers();

	/**
	 * Return the status of the print job.
	 *
	 * @return <code>true</code> if a print job has ended or aborted or
	 *  <code>false</code> if the print job is still running
	 */
	public boolean isFinished() {

		return ((printThread != null && !printThread.isAlive()) || job.isCancelled());

	}

	/**
	 * Initialize the print job.
	 *
	 * @param p the <code>VCLPageFormat</code>
	 * @param n the job name
	 * @return <code>true</code> if a print job was successfully created or
	 *  <code>false</code> if the user cancelled the print dialog
	 */
	public boolean startJob(VCLPageFormat p, String n) {

		// Detect if the user cancelled the print dialog
		if (!jobStarted ) {
			pageFormat = p;
			job = pageFormat.getPrinterJob();
			job.setPrintable(this, pageFormat.getPageFormat());
			if (job.printDialog()) {
				pageFormat.setEditable(false);
				jobStarted = true;
			}
			else {
				jobStarted = false;
			}
		}
		else {
			job.setJobName(n);
		}

		return jobStarted;

	}

	/**
	 * Get the <code>VCLGraphics</code> instance for the current page.
	 *
	 * @param o the page orientation
	 * @return the <code>VCLGraphics</code> instance for the current page
	 */
	public VCLGraphics startPage(int o) {

		synchronized (this) {
			// Start the printing thread if it has not yet been started
			if (printThread == null) {
				printThread = new Thread(this);
				printThread.setPriority(Thread.MIN_PRIORITY);
				printThread.start();
			}

			// Mac OS X wants each page printed twice so skip the first as it
			// seems to have no effect on the printed output
			endPage();

			// Get the current page's printGraphics context
			if (printGraphics != null && printThread != null && printThread.isAlive()) {
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

}
