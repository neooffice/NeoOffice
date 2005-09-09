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
	private PrinterJob job = null;

	/**
	 * The cached <code>VCLPageFormat</code>.
	 */
	private VCLPageFormat pageFormat = null;

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

		if (graphicsInfo != null)
		{
			synchronized (graphicsInfo) {
				if (graphicsInfo.currentGraphics != null) {
					// Dispose a print graphics throws exceptions when memory
					// is low
					try {
						graphicsInfo.currentGraphics.dispose();
					}
					catch (Throwable t) {
						t.printStackTrace();
					}
				}

				graphicsInfo.graphics = null;
				graphicsInfo.pageFormat = null;
				graphicsInfo.currentGraphics = null;
			}
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
		pageFormat.setEditable(true);

	}

	/**
	 * Release the graphics for the current page.
	 */
	public void endPage() {

		synchronized (graphicsInfo) {
 			if (!printThreadStarted || printThreadFinished)
				return;

			// Allow the printer thread to move to the next page
			graphicsInfo.endPage = true;
			while (graphicsInfo.endPage) {
				graphicsInfo.notifyAll();
				try {
					graphicsInfo.wait();
				}
				catch (Throwable t) {}
			}
		}

	}

	/**
	 * Prints the specified page into the specified <code>Graphics</code>
	 * context. This method will block until the <code>endPage</code> is
	 * invoked.
	 */
	public int print(Graphics g, PageFormat f, int i) throws PrinterException {

		if (job.isCancelled())
			throw new PrinterAbortException();
		else if (endJob)
			return Printable.NO_SUCH_PAGE;

		Graphics2D graphics = (Graphics2D)g;

		// Normalize to device resolution
		graphics.transform(graphics.getDeviceConfiguration().getNormalizingTransform());

		synchronized (graphicsInfo) {
			graphicsInfo.graphics = graphics;
			graphicsInfo.pageFormat = f;

			// Notify other threads and wait until painting is finished
			while (!graphicsInfo.endPage) {
				runNativeTimers();
				graphicsInfo.notifyAll();
				try {
					graphicsInfo.wait(500);
				}
				catch (Throwable t) {}
			}

			// Allow endPage() method to continue
			graphicsInfo.endPage = false;

			if (graphicsInfo.currentGraphics != null) {
				runNativeTimers();
				try {
					graphicsInfo.currentGraphics.dispose();
				}
				catch (Throwable t) {
					t.printStackTrace();
				}
				runNativeTimers();
			}

			graphicsInfo.graphics = null;
			graphicsInfo.pageFormat = f;
			graphicsInfo.currentGraphics = null;
		}

		if (job.isCancelled())
			throw new PrinterAbortException();
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
		catch (PrinterAbortException pae) {
			// Don't print anything since the user pressed the cancel button
		}
		catch (Throwable t) {
			t.printStackTrace();
		}
		// Notify other threads that printing is finished
		synchronized (graphicsInfo) {
			printThreadFinished = true;
			graphicsInfo.notifyAll();
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
		pageFormat = p;
		job = pageFormat.getPrinterJob();
		job.setPrintable(this, pageFormat.getPageFormat());
		if (job.printDialog()) {
			pageFormat.setEditable(false);
			return true;
		}
		else {
			return false;
		}

	}

	/**
	 * Get the <code>VCLGraphics</code> instance for the current page.
	 *
	 * @param o the page orientation
	 * @return the <code>VCLGraphics</code> instance for the current page
	 */
	public VCLGraphics startPage(int o) {

		synchronized (graphicsInfo) {
			// Start the printing thread if it has not yet been started
			if (!printThreadStarted) {
				printThread = new Thread(this);
				printThread.setPriority(Thread.MIN_PRIORITY);
				printThreadStarted = true;
				printThread.start();
			}

			// Wait for the printing thread to gain the lock on the
			// graphics queue
			if (graphicsInfo.graphics == null && printThread != null && printThread.isAlive()) {
				try {
					graphicsInfo.wait();
				}
				catch (Throwable t) {}
			}

			// Mac OS X wants each page printed twice so skip the first as it
			// seems to have no effect on the printed output
			endPage();

			// Get the current page's graphics context
			if (graphicsInfo.graphics != null && printThread != null && printThread.isAlive()) {
				// Set the origin to the origin of the printable area
				graphicsInfo.graphics.translate((int)graphicsInfo.pageFormat.getImageableX(), (int)graphicsInfo.pageFormat.getImageableY());

				// Rotate the page if necessary
				int orientation = graphicsInfo.pageFormat.getOrientation();
				boolean rotatedPage = false;
				if (o == VCLPageFormat.ORIENTATION_PORTRAIT && orientation != PageFormat.PORTRAIT) {
					if (orientation == PageFormat.REVERSE_LANDSCAPE) {
						graphicsInfo.graphics.translate(0, (int)graphicsInfo.pageFormat.getImageableHeight());
						graphicsInfo.graphics.rotate(Math.toRadians(-90));
					}
					else {
						graphicsInfo.graphics.translate((int)graphicsInfo.pageFormat.getImageableWidth(), 0);
						graphicsInfo.graphics.rotate(Math.toRadians(90));
					}
					rotatedPage = true;
				}
				else if (o != VCLPageFormat.ORIENTATION_PORTRAIT && orientation == PageFormat.PORTRAIT ) {
					graphicsInfo.graphics.translate(0, (int)graphicsInfo.pageFormat.getImageableHeight());
					graphicsInfo.graphics.rotate(Math.toRadians(-90));
					rotatedPage = true;
				}

				// Scale to printer resolution
				Dimension pageResolution = pageFormat.getPageResolution();
				if (rotatedPage)
					graphicsInfo.graphics.scale((double)72 / pageResolution.height, (double)72 / pageResolution.width);
				else
					graphicsInfo.graphics.scale((double)72 / pageResolution.width, (double)72 / pageResolution.height);

				graphicsInfo.currentGraphics = new VCLGraphics(graphicsInfo.graphics, pageFormat, rotatedPage);
			}
			else {
				graphicsInfo.currentGraphics = null;
			}

			return graphicsInfo.currentGraphics;
		}

	}

	class GraphicsInfo {

		boolean endPage = false;

		Graphics2D graphics = null;

		PageFormat pageFormat = null;

		VCLGraphics currentGraphics = null;

	}

}
