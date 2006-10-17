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

import java.awt.AWTEvent;
import java.awt.Component;
import java.awt.Container;
import java.awt.DefaultKeyboardFocusManager;
import java.awt.EventQueue;
import java.awt.Insets;
import java.awt.KeyboardFocusManager;
import java.awt.Panel;
import java.awt.Rectangle;
import java.awt.Toolkit;
import java.awt.Window;
import java.awt.event.FocusEvent;
import java.awt.event.InputEvent;
import java.awt.event.InputMethodEvent;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.awt.event.PaintEvent;
import java.awt.event.WindowEvent;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.util.Collections;

/**
 * An class that subclass that intercepts Java events and caches them for
 * handling by the SalInstance C++ class.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public final class VCLEventQueue implements Runnable {

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
	 * The GC_INTERVAL constant.
	 */
	public final static long GC_INTERVAL = 60000;

	/**
	 * The last adjusted mouse modifiers.
	 */
	private int lastAdjustedMouseModifiers = 0;

	/**
	 * The next garbage collection.
	 */
	private long nextGC = 0;

	/**
	 * The shutdown disabled flag.
	 */
	private boolean shutdownDisabled = false;

	/**
	 * The list of queues.
	 */
	private VCLEventQueue.Queue[] queueList = new VCLEventQueue.Queue[2];

	/**
	 * The printing flag.
	 */
	private boolean printing = false;

	/**
	 * Construct a VCLEventQueue and make it the system queue.
	 */
	public VCLEventQueue() {

		// Create the list of queues
		queueList[0] = new VCLEventQueue.Queue();
		queueList[1] = new VCLEventQueue.Queue();

		// Swap in our own event queue
		Toolkit.getDefaultToolkit().getSystemEventQueue().push(new VCLEventQueue.NoExceptionsEventQueue(this));

		// Load platform specific event handlers
		try {
			Class c = Class.forName("com.sun.star.vcl.macosx.VCLApplicationListener");
			Constructor ctor = c.getConstructor(new Class[]{ getClass() });
			ctor.newInstance(new Object[]{ this });
		}
		catch (Throwable t) {
			t.printStackTrace();
		}

		// Set keyboard focus manager
		KeyboardFocusManager kfm = KeyboardFocusManager.getCurrentKeyboardFocusManager();
		kfm.setCurrentKeyboardFocusManager(new NoEnqueueKeyboardFocusManager());

		// Set the keyboard focus manager so that Java's default focus
		// switching key events are passed are not consumed
		kfm = KeyboardFocusManager.getCurrentKeyboardFocusManager();
		kfm.setDefaultFocusTraversalKeys(KeyboardFocusManager.FORWARD_TRAVERSAL_KEYS, Collections.EMPTY_SET);
		kfm.setDefaultFocusTraversalKeys(KeyboardFocusManager.BACKWARD_TRAVERSAL_KEYS, Collections.EMPTY_SET);
		kfm.setDefaultFocusTraversalKeys(KeyboardFocusManager.UP_CYCLE_TRAVERSAL_KEYS, Collections.EMPTY_SET);
		kfm.setDefaultFocusTraversalKeys(KeyboardFocusManager.DOWN_CYCLE_TRAVERSAL_KEYS, Collections.EMPTY_SET);

	}

	/**
	 * Determine if there are any cached events.
	 *
	 * @param type one or more of the type constants
	 * @return <code>true</code> if there are any events of the specified type
	 *  else <code>false</code>
	 */
	public boolean anyCachedEvent(int type) {

		synchronized (queueList) {
			for (int i = 0; i < queueList.length; i++) {
				VCLEventQueue.Queue queue = queueList[i];

				if (queue.head == null)
					continue;

				VCLEventQueue.QueueItem eqi = queue.head;
				while (eqi != null) {
					if (!eqi.remove && (type & eqi.type) != 0)
						return true;
					eqi = eqi.next;
				}
			}
		}

		return false;

	}

	/**
	 * Dispatches the next event in the Java event queue. Note that this
	 * method will do nothing if the current thread is not the Java dispatch
	 * thread.
	 */
	public void dispatchNextEvent() {

		if (EventQueue.isDispatchThread()) {
			try {
				VCLEventQueue.NoExceptionsEventQueue eventQueue = (VCLEventQueue.NoExceptionsEventQueue)Toolkit.getDefaultToolkit().getSystemEventQueue();

				// Post a dummy, low priority event to ensure that we don't
				// block if there are no pending events
				PaintEvent e = new PaintEvent(new Container(), PaintEvent.PAINT, new Rectangle());
				eventQueue.postEvent(e);
				AWTEvent nextEvent;
				while ((nextEvent = eventQueue.getNextEvent()) != e)
					eventQueue.dispatchEvent(nextEvent);
			}
			catch (Throwable t) {
				t.printStackTrace();
			}
		}

	}

	/**
	 * Returns the last adjusted mouse modifiers.
	 *
	 * @return the last adjusted mouse modifiers
	 */
	int getLastAdjustedMouseModifiers() {

		return lastAdjustedMouseModifiers;

	}

	/**
	 * Get the next cached event. If there are no cached events and the
	 * <code>wait</code> parameter is <code>true</code>, this method waits
	 * a short time to allow an event to become available in the cache.
	 *
	 * @param wait the number of milliseconds to wait for an event to be added
	 *  to the queue
	 * @param awtEvents <code>true</code> to use to the <code>AWTEvent</code>
	 *  queue and <code>false</code> to use the non-<code>AWTEvent</code> queue
	 * @return the next cached <code>VCLEvent</code> instance
	 */
	public VCLEvent getNextCachedEvent(long wait, boolean awtEvents) {

		VCLEventQueue.Queue queue = (awtEvents ? queueList[0] : queueList[1]);

		synchronized (queueList) {
			if (wait > 0 && queueList[0].head == null && queueList[1].head == null) {
				// Since we are going to block, this is a good time to run the
				// garbage collector
				long currentTime = System.currentTimeMillis();
				if (currentTime >= nextGC) {
					System.gc();
					nextGC = System.currentTimeMillis() + VCLEventQueue.GC_INTERVAL;
				}
				try {
					queueList.wait(wait);
				}
				catch (Throwable t) {}
			}

			VCLEventQueue.QueueItem eqi = null;
			eqi = queue.head;
			if (eqi != null) {
				queue.head = queue.head.next;
				if (eqi == queue.keyInput)
					queue.keyInput = null;
				else if (eqi == queue.moveResize)
					queue.moveResize = null;
				else if (eqi == queue.paint)
					queue.moveResize = queue.paint = null;
				
			}
			if (queue.head == null)
				queue.keyInput = queue.moveResize = queue.paint = queue.tail = null;
			return eqi != null ? eqi.event : null;
		}

	}

	/**
	 * Add an event to the cache.
	 *
     * @param event the event to add to the cache
	 */
	public void postCachedEvent(VCLEvent event) {

		int id = event.getID();
		if (shutdownDisabled && id == VCLEvent.SALEVENT_SHUTDOWN) {
			event.cancelShutdown();
			return;
		}

		VCLEventQueue.Queue queue = (event.isAWTEvent() ? queueList[0] : queueList[1]);

		// Add the event to the cache
		VCLEventQueue.QueueItem newItem = new VCLEventQueue.QueueItem(event);
		synchronized (queueList) {
			// Coalesce events
			switch (id) {
				case VCLEvent.SALEVENT_CLOSE:
					{
						VCLEventQueue.QueueItem eqi = queue.head;
						while (eqi != null) {
							if (eqi.event.getID() == VCLEvent.SALEVENT_CLOSE && eqi.event.getFrame() == newItem.event.getFrame())
								return;
							eqi = eqi.next;
						}
					}
					break;
				case VCLEvent.SALEVENT_EXTTEXTINPUT:
					{
						// Reduce flicker when backspacing through uncommitted
						// text
						if (queue.tail != null && queue.tail.event.getID() == VCLEvent.SALEVENT_EXTTEXTINPUT && queue.tail.event.getText() == null)
							queue.tail.remove = true;
					}
					break;
				case VCLEvent.SALEVENT_KEYINPUT:
					{
						if (queue.keyInput != null && !queue.keyInput.remove && queue.keyInput.event.getFrame() == newItem.event.getFrame() && queue.keyInput.event.getKeyChar() == newItem.event.getKeyChar() && queue.keyInput.event.getKeyCode() == newItem.event.getKeyCode() && queue.keyInput.event.getModifiers() == newItem.event.getModifiers()) {
							queue.keyInput.remove = true;
							newItem.event.addRepeatCount((short)1);
						}
						queue.keyInput = newItem;
					}
					break;
				case VCLEvent.SALEVENT_KEYUP:
					{
						queue.keyInput = null;
					}
					break;
				case VCLEvent.SALEVENT_MOUSEMOVE:
					{
						if (queue.tail != null && !queue.tail.remove && queue.tail.event.getID() == VCLEvent.SALEVENT_MOUSEMOVE && queue.tail.event.getFrame() == newItem.event.getFrame())
							queue.tail.remove = true;
					}
					break;
				case VCLEvent.SALEVENT_WHEELMOUSE:
					{
						if (queue.tail != null && !queue.tail.remove && queue.tail.event.getID() == VCLEvent.SALEVENT_WHEELMOUSE && queue.tail.event.getFrame() == newItem.event.getFrame()) {
							queue.tail.remove = true;
							newItem.event.addWheelRotation(queue.tail.event.getWheelRotation());
						}
					}
					break;
				case VCLEvent.SALEVENT_MOVERESIZE:
					{
						if (queue.moveResize != null && !queue.moveResize.remove && queue.moveResize.event.getFrame() == newItem.event.getFrame())
							queue.moveResize.remove = true;
						queue.moveResize = newItem;
					}
					break;
				case VCLEvent.SALEVENT_PAINT:
					{
						if (queue.paint != null && !queue.paint.remove && queue.paint.event.getFrame() == newItem.event.getFrame()) {
							queue.paint.remove = true;
							Rectangle oldBounds = queue.paint.event.getUpdateRect();
							if (oldBounds != null) {
								Rectangle newBounds = newItem.event.getUpdateRect();
								if (newBounds != null)
									newItem.event.setUpdateRect(oldBounds.union(newBounds));
								else
									newItem.event.setUpdateRect(oldBounds);
							}
						}
						queue.paint = newItem;
					}
					break;
				default:
					break;
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
				case VCLEvent.SALEVENT_EXTTEXTINPUT:
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

			queueList.notifyAll();
		}

	}

	/**
	 * Remove all events in the cache associated with the specified frame
	 * pointer.
	 *
     * @param frame the frame pointer
	 */
	void removeCachedEvents(long frame) {

		synchronized (queueList) {
			for (int i = 0; i < queueList.length; i++) {
				VCLEventQueue.Queue queue = queueList[i];

				if (queue.head == null)
					continue;

				VCLEventQueue.QueueItem eqi = queue.head;
				while (eqi != null) {
					if (eqi.event.getFrame() == frame)
						eqi.remove = true;
					eqi = eqi.next;
				}

				// Purge removed events from the front of the queue
				while (queue.head != null && queue.head.remove)
					queue.head = queue.head.next;
				if (queue.head == null)
					queue.tail = null;
			}
		}

	}

	/**
	 * Runnable method that performs nothing. This method is used for passing
	 * this class to the <code>EventQueue.invokeLater()</code> method so that
	 * we can prevent blocking in the {@link #dispatchNextEvent()} method.
	 */
	public void run() {}

	/**
	 * Sets the last adjusted mouse modifiers.
	 *
	 * @param m the last adjusted mouse modifiers
	 */
	void setLastAdjustedMouseModifiers(int m) {

		lastAdjustedMouseModifiers = m;

	}

	/**
	 * Sets the shutdown disabled flag.
	 *
	 * @param p <code>true</code> to disable shutdown otherwise
	 *  <code>false</code>
	 */
	public void setShutdownDisabled(boolean b) {

		shutdownDisabled = b;

		synchronized (queueList) {
			for (int i = 0; i < queueList.length; i++) {
				VCLEventQueue.Queue queue = queueList[i];

				if (queue.head == null)
					continue;

				VCLEventQueue.QueueItem eqi = queue.head;
				while (eqi != null) {
					if (eqi.event.getID() == VCLEvent.SALEVENT_SHUTDOWN) {
						eqi.event.cancelShutdown();
						eqi.remove = true;
					}
					eqi = eqi.next;
				}
				// Purge removed events from the front of the queue
				while (queue.head != null && queue.head.remove)
					queue.head = queue.head.next;
				if (queue.head == null)
					queue.tail = null;
					
			}
		}

	}

	/**
	 * The <code>NoExceptionEventQueue</code> class prevents unhandled
	 * exceptions from stopping the event queue.
	 */
	final class NoExceptionsEventQueue extends EventQueue {

		/**
		 * The <code>VCLEventQueue</code>.
		 */
		private VCLEventQueue queue = null;

		/**
		 * Construct a <code>NoExceptionsEventQueue</code> instance.
		 *
		 * @param q the <code>VCLEventQueue</code>
		 */
		NoExceptionsEventQueue(VCLEventQueue q) {

			queue = q;

		}

		/**
		 * Dispatch an event.
		 *
		 * @param event the event to dispatch
		 */
		protected void dispatchEvent(AWTEvent event) {

			try {
				super.dispatchEvent(event);

				// The modifiers for mouse released events contain the
				// modifiers after the event has occurred so we need to
				// replace the modifiers with the modifiers that were released
				if (event instanceof KeyEvent) {
					int id = event.getID();
					if (id == KeyEvent.KEY_PRESSED || id == KeyEvent.KEY_RELEASED) {
						int modifiers = queue.getLastAdjustedMouseModifiers();
						if (modifiers != 0) {
							modifiers &= ~( InputEvent.SHIFT_DOWN_MASK | InputEvent.CTRL_DOWN_MASK | InputEvent.ALT_DOWN_MASK | InputEvent.META_DOWN_MASK );
							queue.setLastAdjustedMouseModifiers(modifiers | ((KeyEvent)event).getModifiersEx());
						}
					}
				}
				else if (event instanceof MouseEvent) {
					int id = event.getID();
					if (id == MouseEvent.MOUSE_PRESSED || id == MouseEvent.MOUSE_RELEASED)
						queue.setLastAdjustedMouseModifiers(((MouseEvent)event).getModifiersEx());
				}
			}
			catch (Throwable t) {
				t.printStackTrace();
			}

		}

	}

	/**
	 * The <code>Queue</code> object holds pointers to the beginning and end of
	 * one internal queue.
	 */
	final class Queue {

		VCLEventQueue.QueueItem head = null;

		VCLEventQueue.QueueItem keyInput = null;

		VCLEventQueue.QueueItem moveResize = null;

		VCLEventQueue.QueueItem paint = null;

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

	/**
	 * The <code>NoEnqueueKeyboardFocusManager</code> is a subclass of
	 * the <code>DefaultKeyboardFocusManager</code> class that does not enqueue
	 * any key events.
	 */
	final class NoEnqueueKeyboardFocusManager extends DefaultKeyboardFocusManager {

		/**
		 * Fix bug 1715 by ensuring that the focus owner does not get set to
		 * null if there is a permanent focus owner.
		 */
		public boolean dispatchEvent(AWTEvent e) {

			boolean ret = super.dispatchEvent(e);

			if (getFocusOwner() == null) {
				Component c = getPermanentFocusOwner();
				if (c != null)
					c.requestFocusInWindow();
			}

			return ret;

		}

	}

}
