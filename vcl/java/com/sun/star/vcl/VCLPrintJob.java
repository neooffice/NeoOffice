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
	 * The current <code>VCLGraphics</code>
	 */
	private VCLGraphics currentGraphics = null;

	/**
	 * The current page.
	 */
	private int currentPage = 0;

	/**
	 * The dialogAccepted flag.
	 */
	private boolean dialogAccepted = false;

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

		if (currentGraphics != null) {
			// Dispose a print graphics throws exceptions when memory is low
			try {
				currentGraphics.dispose();
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
		}
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
			if (currentGraphics == null) {
 				if (!endJob && !job.isCancelled())
					return;
			}
			else {
				// Dispose a print graphics throws exceptions when memory is low
				try {
					currentGraphics.dispose();
				}
				catch (Throwable t) {
					t.printStackTrace();
				}
				currentGraphics = null;
			}

 			if (!printThreadStarted || printThreadFinished)
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
			throw new PrinterAbortException();
		else if (endJob)
			return Printable.NO_SUCH_PAGE;

		graphicsInfo.pageFormat = f;
		graphicsInfo.pageIndex = i;

		Graphics2D graphics = (Graphics2D)g;

		// Normalize to device resolution
		graphics.transform(graphics.getDeviceConfiguration().getNormalizingTransform());

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
	 * @param b <code>true</code> to show the print dialog or
	 *  <code>false</code> to not show the print dialog
	 * @return <code>true</code> if a print job was successfully created or
	 *  <code>false</code> if the user cancelled the print dialog
	 */
	public boolean startJob(VCLPageFormat p, boolean b) {

		// Detect if the user cancelled the print dialog
		if (b) {
			pageFormat = p;
			job = pageFormat.getPrinterJob();
			job.setPrintable(this, pageFormat.getPageFormat());
			if (job.printDialog()) {
				dialogAccepted = true;
			}
			else {
				dialogAccepted = false;
			}
		}
		else if (!b && dialogAccepted) {
			pageFormat.setEditable(false);
		}

		return dialogAccepted;

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
				printThread.start();
				printThreadStarted = true;
			}

			// Wait for the printing thread to gain the lock on the
			// graphics queue
			if (graphicsInfo.graphics == null && printThread != null && printThread.isAlive()) {
				try {
					graphicsInfo.wait();
				}
				catch (Throwable t) {}
			}

			// Get the current page's graphics context
			currentPage++;
			if (graphicsInfo.graphics != null && printThread != null && printThread.isAlive()) {
				// Set the origin to the origin of the printable area
				graphicsInfo.graphics.translate((int)graphicsInfo.pageFormat.getImageableX(), (int)graphicsInfo.pageFormat.getImageableY());

				// Rotate the page if necessary
				int orientation = graphicsInfo.pageFormat.getOrientation();
				Rectangle imageableBounds = new Rectangle(pageFormat.getImageableBounds());
				imageableBounds.x = 0;
				imageableBounds.y = 0;
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
					imageableBounds = new Rectangle(imageableBounds.y, imageableBounds.x, imageableBounds.height, imageableBounds.width);
				}
				else if (o != VCLPageFormat.ORIENTATION_PORTRAIT && orientation == PageFormat.PORTRAIT ) {
					graphicsInfo.graphics.translate(0, (int)graphicsInfo.pageFormat.getImageableHeight());
					graphicsInfo.graphics.rotate(Math.toRadians(-90));
					rotatedPage = true;
					imageableBounds = new Rectangle(imageableBounds.y, imageableBounds.x, imageableBounds.height, imageableBounds.width);
				}

				// Scale to printer resolution
				Dimension pageResolution = pageFormat.getPageResolution();
				if (rotatedPage)
					graphicsInfo.graphics.scale((double)72 / pageResolution.height, (double)72 / pageResolution.width);
				else
					graphicsInfo.graphics.scale((double)72 / pageResolution.width, (double)72 / pageResolution.height);

				currentGraphics = new VCLGraphics(graphicsInfo.graphics, pageFormat, rotatedPage);
			}
			else {
				currentGraphics = null;
			}

			// Mac OS X creates two graphics for each page so we need to
			// ignore the first and only print to the second
			if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX && currentPage % 2 != 0)
				return null;
		}

		return currentGraphics;

	}

	class GraphicsInfo {

		Graphics2D graphics = null;

		PageFormat pageFormat = null;

		int pageIndex = -1;

	}

}
