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

import java.lang.reflect.Constructor;

/**
 * An class that subclass that intercepts Java events and caches them for
 * handling by the SalInstance C++ class.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public final class VCLEventQueue {

    /** 
     * INPUT_MOUSE constant.
     */
	public final static int INPUT_MOUSE	= 0x0001;

    /** 
     * INPUT_KEYBOARD constant.
     */
	public final static int INPUT_KEYBOARD = 0x0002;

    /** 
     * INPUT_PAINT constant.
     */
	public final static int INPUT_PAINT = 0x0004;

    /** 
     * INPUT_TIMER constant.
     */
	public final static int INPUT_TIMER = 0x0008;

    /** 
     * INPUT_OTHER constant.
     */
	public final static int INPUT_OTHER = 0x0010;

    /** 
     * INPUT_MOUSEANDKEYBOARD constant.
     */
	public final static int INPUT_MOUSEANDKEYBOARD = VCLEventQueue.INPUT_MOUSE | VCLEventQueue.INPUT_KEYBOARD;

    /** 
     * INPUT_ANY constant.
     */
	public final static int INPUT_ANY = VCLEventQueue.INPUT_MOUSE | VCLEventQueue.INPUT_KEYBOARD | VCLEventQueue.INPUT_PAINT | VCLEventQueue.INPUT_TIMER | VCLEventQueue.INPUT_OTHER;

	/**
	 * The queue of cached events.
	 */
	private boolean autoFlush = true;

	/**
	 * The queue of cached events.
	 */
	private VCLEventQueue.Queue queue = new VCLEventQueue.Queue();

	/**
	 * Construct a VCLEventQueue and make it the system queue.
	 */
	public VCLEventQueue() {

		VCLGraphics.setAutoFlush(true);

		// Load platform specific event handlers
		if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX) {
			try {
				Class c = Class.forName("com.sun.star.vcl.macosx.VCLOpenApplicationHandler");
				Constructor ctor = c.getConstructor(new Class[]{ getClass() });
				ctor.newInstance(new Object[]{ this });
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			try {
				Class c = Class.forName("com.sun.star.vcl.macosx.VCLOpenDocumentHandler");
				Constructor ctor = c.getConstructor(new Class[]{ getClass() });
				ctor.newInstance(new Object[]{ this });
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
			try {
				Class c = Class.forName("com.sun.star.vcl.macosx.VCLPrintDocumentHandler");
				Constructor ctor = c.getConstructor(new Class[]{ getClass() });
				ctor.newInstance(new Object[]{ this });
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
		}

	}

	/**
	 * Determine if there are any cached events.
	 *
	 * @param type one or more of the type constants
	 * @return <code>true</code> if there are any events of the specified type
	 *  else <code>false</code>
	 */
	public boolean anyCachedEvent(int type) {

		if (queue.head == null)
			return false;

		synchronized (queue) {
			VCLEventQueue.QueueItem eqi = queue.head;
			while (eqi != null) {
				if (!eqi.remove && (type & eqi.type) != 0)
					return true;
				eqi = eqi.next;
			}
			return false;
		}

	}

	/**
	 * Get the next cached event. If there are no cached events and the
	 * <code>wait</code> parameter is <code>true</code>, this method waits
	 * a short time to allow an event to become available in the cache.
	 *
	 * @param wait if <code>true</code>, wait a short time to allow an event
	 *  to be added to the queue
	 * @return the next cached <code>VCLEvent</code> instance
	 */
	public VCLEvent getNextCachedEvent(boolean wait) {

		if (autoFlush) {
			// Turn off auto flushing
			autoFlush = false;
			VCLGraphics.setAutoFlush(false);
			
			// Load platform specific event handlers
			if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX) {
				try {
					Class c = Class.forName("com.sun.star.vcl.macosx.VCLQuitHandler");
					Constructor ctor = c.getConstructor(new Class[]{ getClass() });
					ctor.newInstance(new Object[]{ this });
				}
				catch (Throwable t) {
					t.printStackTrace();
				}
			}
		}
			
		if (!wait && queue.head == null)
			return null;

		synchronized (queue) {
			if (wait && queue.head == null) {
				try {
					queue.wait(100);
				}
				catch (Throwable t) {}
			}
			VCLEventQueue.QueueItem eqi = null;
			eqi = queue.head;
			if (eqi != null) {
				queue.head = queue.head.next;
				if (eqi == queue.mouseMove)
					queue.mouseMove = null;
			}
			if (queue.head == null)
				queue.mouseMove = queue.tail = null;
			return eqi != null ? eqi.event : null;
		}

	}

	/**
	 * Add an event to the cache.
	 *
     * @param event the event to add to the cache
	 */
	public void postCachedEvent(VCLEvent event) {

		// Add the event to the cache
		VCLEventQueue.QueueItem newItem = new VCLEventQueue.QueueItem(event);
		int id = newItem.event.getID();
		synchronized (queue) {
			// Coalesce mouse move events
			if (id == VCLEvent.SALEVENT_MOUSEMOVE) {
				if (queue.mouseMove != null)
					queue.mouseMove.remove = true;
				queue.mouseMove = newItem;
			}
			// Purge removed events from the front of the queue
			while (queue.head != null && queue.head.remove)
				queue.head = queue.head.next;
			if (queue.head != null) {
				queue.tail.next = newItem;
				queue.tail = newItem;
			}
			else {
				queue.head = queue.tail = newItem;
			}
			// Update status flags
			switch (id) {
				case VCLEvent.SALEVENT_KEYINPUT:
					newItem.type = VCLEventQueue.INPUT_KEYBOARD;
					break;
				case VCLEvent.SALEVENT_MOUSEMOVE:
				case VCLEvent.SALEVENT_MOUSELEAVE:
				case VCLEvent.SALEVENT_MOUSEBUTTONUP:
				case VCLEvent.SALEVENT_MOUSEBUTTONDOWN:
					newItem.type = VCLEventQueue.INPUT_MOUSE;
					break;
				case VCLEvent.SALEVENT_PAINT:
					newItem.type = VCLEventQueue.INPUT_PAINT;
					break;
				default:
					newItem.type = VCLEventQueue.INPUT_OTHER;
					break;
			}
			queue.notifyAll();
		}

	}

	/**
	 * Remove all events in the cache associated with the specified frame
	 * pointer.
	 *
     * @param frame the frame pointer
	 */
	void removeCachedEvents(long frame) {

		if (queue.head == null)
			return;

		synchronized (queue) {
			VCLEventQueue.QueueItem eqi = queue.head;
			while (eqi != null) {
				if (eqi.event.getFrame() == frame)
					eqi.remove = true;
				eqi = eqi.next;
			}
			// Purge removed events from the front of the queue
			while (queue.head != null && queue.head.remove)
				queue.head = queue.head.next;
		}

	}

	/**
	 * The <code>Queue</code> object holds pointers to the beginning and end of
	 * one internal queue.
	 */
	final class Queue {

		VCLEventQueue.QueueItem head = null;

		VCLEventQueue.QueueItem mouseMove = null;

		VCLEventQueue.QueueItem tail = null;

	}

	/**
	 * The <code>QueueItem</code> object is a wrapper for <code>VCLEvent</code>
	 * instances.
	 */
	final class QueueItem {

		VCLEvent event = null;

		QueueItem next = null;

		boolean remove = false;

		int type = 0;

		QueueItem(VCLEvent event) {

			this.event = event;

		}

	}

}
