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

package com.sun.star.vcl.macosx;

import com.sun.star.vcl.VCLEvent;
import com.sun.star.vcl.VCLEventQueue;

import com.apple.eawt.Application;
import com.apple.eawt.ApplicationEvent;
import com.apple.eawt.ApplicationListener;

import java.awt.Frame;
import java.io.File;

/** 
 * A Java class that implements the <code>ApplicationListener</code> interface.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public class VCLApplicationListener implements ApplicationListener {

	/**
	 * The event queue.
	 */
	private VCLEventQueue queue = null;

	/**
	 * Constructs a new <code>VCLApplicationListener</code> instance.
	 * 
	 * @param q the event queue to post events to
	 */
	public VCLApplicationListener(VCLEventQueue q) {

		queue = q;

		// Register an instance of this class as the quit handler
		Application application = Application.getApplication();
		application.setEnabledAboutMenu(true);
		application.setEnabledPreferencesMenu(true);
		application.addApplicationListener(this);

	}

	/**
	 * Called to handle about operations.
	 *
	 * @param event the event
	 */
	public void handleAbout(ApplicationEvent event) {

		queue.postCachedEvent(new VCLEvent(VCLEvent.SALEVENT_ABOUT, null, 0));
		event.setHandled(true);

	}

	/**
	 * Called when the application is launched.
	 *
	 * @param event the event
	 */
	public void handleOpenApplication(ApplicationEvent event) {}

	/**
	 * Called when an a file is opened from the Finder.
	 *
	 * @param event the event
	 */
	public void handleOpenFile(ApplicationEvent event) {

		File file = new File(event.getFilename());
		if (file.exists() && !file.isDirectory())
			queue.postCachedEvent(new VCLEvent(VCLEvent.SALEVENT_OPENDOCUMENT, null, 0, file.getAbsolutePath()));
		event.setHandled(true);

	}

	/**
	 * Called to handle preferences operations.
	 *
	 * @param event the event
	 */
	public void handlePreferences(ApplicationEvent event) {

		queue.postCachedEvent(new VCLEvent(VCLEvent.SALEVENT_PREFS, null, 0));
		event.setHandled(true);

	}

	/**
	 * Called when an a file is printed from the Finder.
	 *
	 * @param event the event
	 */
	public void handlePrintFile(ApplicationEvent event) {

		File file = new File(event.getFilename());
		if (file.exists() && !file.isDirectory())
			queue.postCachedEvent(new VCLEvent(VCLEvent.SALEVENT_PRINTDOCUMENT, null, 0, file.getAbsolutePath()));
		event.setHandled(true);

	}

	/**
	 * Called when the application is about to quit.
	 *
	 * @param event the event
	 */
	public void handleQuit(ApplicationEvent event) {

		Application application = Application.getApplication();
		application.setEnabledAboutMenu(false);
		application.setEnabledPreferencesMenu(false);

		VCLEvent shutdownEvent = new VCLEvent(VCLEvent.SALEVENT_SHUTDOWN, null, 0);
		queue.postCachedEvent(shutdownEvent);

		// Wait for event to be dispatched in the C++ code. Note that if the
		// event is successfully processed, this loop will never finish and
		// the process will exit.
		while (!shutdownEvent.isShutdownCancelled()) {
			queue.dispatchNextEvent();
			Thread.yield();
		}

		event.setHandled(false);

		application.setEnabledAboutMenu(true);
		application.setEnabledPreferencesMenu(true);

	}

	/**
	 * Called when the application is actived.
	 *
	 * @param event the event
	 */
	public void handleReOpenApplication(ApplicationEvent event) {}

}
