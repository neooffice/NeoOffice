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
import java.awt.EventQueue;
import java.awt.Toolkit;
import java.awt.event.InputEvent;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.awt.event.PaintEvent;
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
	 * The cached system event queue.
	 */
	private VCLEventQueue.FilteredEventQueue eventQueue = null;

	/**
	 * The queue of cached events.
	 */
	private VCLEventQueue.Queue queue = new VCLEventQueue.Queue();

	/**
	 * Construct a VCLEventQueue and make it the system queue.
	 */
	public VCLEventQueue() {

		VCLGraphics.setAutoFlush(true);
		eventQueue = new VCLEventQueue.FilteredEventQueue(this);
		Toolkit.getDefaultToolkit().getSystemEventQueue().push(eventQueue);

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
				Class c = Class.forName("com.sun.star.vcl.macosx.VCLQuitHandler");
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

		// Allow the Java event queue to dispatch pending events first
		Thread.currentThread().yield();

		if (!wait && queue.head == null)
			return null;

		synchronized (queue) {
			if (wait && queue.head == null) {
				try {
					queue.wait(10);
				}
				catch (InterruptedException ie) {}
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
		VCLGraphics.setAutoFlush(false);
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
				case VCLEvent.SALEVENT_MOUSEBUTTONDOWN:
				case VCLEvent.SALEVENT_MOUSEBUTTONUP:
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

	/**
	 * The <code>FilteredEventQueue</code> class is a subclass of the
	 * <code>EventQueue</code> class that filters and modifies events at
	 * dispatch time.
	 */
	final class FilteredEventQueue extends EventQueue {

		/**
		 * The mouse drag flag.
		 */
		private boolean inMouseDrag = false;

		/**
		 * The mouse pressed dispatched flag.
		 */
		private boolean mousePressedDispatched = false;

		/**
		 * The cached mouse pressed event.
		 */
		private MouseEvent mousePressedEvent  = null;

		/**
		 * The wait for mouse released flag.
		 */
		private boolean waitForMouseReleased = false;

		/**
		 * The cached <code>VCLEventQueue</code>.
		 */
		private VCLEventQueue queue = null;

		/**
		 * Construct a FilteredEventQueue.
		 */
		FilteredEventQueue(VCLEventQueue q) {

			super();
			queue = q;

		}

		/**
		 * Dispatch the event using Java's default dispatching process and then
		 * cache the event for handling by the C++ code.
		 *
		 * @param event the AWTEvent
		 */
		protected void dispatchEvent(AWTEvent event) {

			int id = event.getID();
			if (VCLPlatform.getPlatform() == VCLPlatform.PLATFORM_MACOSX) {
				switch (id) {
					case KeyEvent.KEY_PRESSED:
					case KeyEvent.KEY_RELEASED:
					case KeyEvent.KEY_TYPED:
					{
						KeyEvent e = (KeyEvent)event;
						int modifiers = e.getModifiers();
						if ((modifiers & InputEvent.CTRL_MASK) == InputEvent.CTRL_MASK) {
							// Switch the key char back to the character that
							// was actually pressed instead of the ASCII
							// Ctrl+Key key char that the event has resolved to
							char keyChar = e.getKeyChar();
							if (keyChar >= 1 && keyChar <= 26)
							{
								if ((modifiers & InputEvent.SHIFT_MASK) == InputEvent.SHIFT_MASK)
									keyChar += 64;
								else
									keyChar += 96;
							}
							int keyCode = e.getKeyCode();
							event = e = new KeyEvent(e.getComponent(), id, e.getWhen(), modifiers, e.getKeyCode(), keyChar);
						}
						// Treat the Mac OS X command key as a control key and
						// the control key as the meta key
						if ((modifiers & (InputEvent.CTRL_MASK | InputEvent.META_MASK)) == (InputEvent.CTRL_MASK | InputEvent.META_MASK)) {
							; // No switching is needed
						}
						else if ((modifiers & InputEvent.CTRL_MASK) == InputEvent.CTRL_MASK) {
							modifiers = (modifiers & ~InputEvent.CTRL_MASK) | InputEvent.META_MASK;
							int keyCode = e.getKeyCode();
							if (keyCode == KeyEvent.VK_CONTROL)
								keyCode = KeyEvent.VK_META;
							event = e = new KeyEvent(e.getComponent(), id, e.getWhen(), modifiers, keyCode, e.getKeyChar());
						}
						else if ((modifiers & InputEvent.META_MASK) == InputEvent.META_MASK) {
							modifiers = (modifiers & ~InputEvent.META_MASK) | InputEvent.CTRL_MASK;
							int keyCode = e.getKeyCode();
							if (keyCode == KeyEvent.VK_META)
								keyCode = KeyEvent.VK_CONTROL;
							event = e = new KeyEvent(e.getComponent(), id, e.getWhen(), modifiers, keyCode, e.getKeyChar());
						}
						if ((modifiers & (InputEvent.SHIFT_MASK | InputEvent.CTRL_MASK)) == (InputEvent.SHIFT_MASK | InputEvent.CTRL_MASK)) {
							// It appears that Java on Mac OS X suppresses the
							// key typed event for some unreserved
							// Command+Shift+Key combinations so we need to
							// ignore the key typed events and generate them
							// when there is a key released event
							if (id == KeyEvent.KEY_RELEASED) {
								char keyChar = e.getKeyChar();
								if (keyChar != KeyEvent.CHAR_UNDEFINED)
									super.dispatchEvent(new KeyEvent(e.getComponent(), KeyEvent.KEY_TYPED, e.getWhen(), modifiers, KeyEvent.VK_UNDEFINED, keyChar));
							}
							else if (id == KeyEvent.KEY_TYPED) {
								return;
							}
						}
						break;
					}
					case MouseEvent.MOUSE_CLICKED:
					case MouseEvent.MOUSE_DRAGGED:
					case MouseEvent.MOUSE_ENTERED:
					case MouseEvent.MOUSE_EXITED:
					case MouseEvent.MOUSE_MOVED:
					case MouseEvent.MOUSE_RELEASED:
					{
						MouseEvent e = (MouseEvent)event;
						int modifiers = e.getModifiers();
						if ((modifiers & (InputEvent.BUTTON2_MASK | InputEvent.BUTTON3_MASK)) != 0) {
							// If button 2 or button 3 is activated by pressing
							// the Alt or Meta keys, switch it to button 1
							modifiers = (modifiers & ~(InputEvent.BUTTON2_MASK | InputEvent.BUTTON3_MASK)) | InputEvent.BUTTON1_MASK;
							event = e = new MouseEvent(e.getComponent(), id, e.getWhen(), modifiers, e.getX(), e.getY(), e.getClickCount(), false);
						}
						if (mousePressedEvent != null) {
							// Dispatch the cached mouse pressed event since it
							// has not been dispatched yet
							super.dispatchEvent(mousePressedEvent);
							mousePressedDispatched = true;
							mousePressedEvent = null;
						}
						if (id == MouseEvent.MOUSE_RELEASED) {
							if (waitForMouseReleased) {
								// Ignore this event since a mouse released
								// event was dispatched with the last mouse
								//  pressed event
								waitForMouseReleased = false;
								return;
							}
							inMouseDrag = false;
						}
						else {
							// If the waitForMouseReleased flag is true, ignore
							// all other mouse events until the next mouse
							// released event
							if (waitForMouseReleased)
								return;
							if (id == MouseEvent.MOUSE_DRAGGED)
								inMouseDrag = true;
							else {
								// If the mouse is in drag mode, ignore all
								// other mouse events until the next mouse
								// released event
								if (inMouseDrag)
									return;
							}
						}
						break;
					}
					case MouseEvent.MOUSE_PRESSED:
					{
						MouseEvent e = (MouseEvent)event;
						int modifiers = e.getModifiers();
						if ((modifiers & (InputEvent.BUTTON2_MASK | InputEvent.BUTTON3_MASK)) != 0) {
							// If button 2 or button 3 is activated by pressing
							// the Alt or Meta keys, switch it to button 1
							modifiers = (modifiers & ~(InputEvent.BUTTON2_MASK | InputEvent.BUTTON3_MASK)) | InputEvent.BUTTON1_MASK;
							event = e = new MouseEvent(e.getComponent(), id, e.getWhen(), modifiers, e.getX(), e.getY(), e.getClickCount(), false);
						}
						// If mouse button 1 is held down for a short time
						// without any movement, switch this event to button 2
						if (!mousePressedDispatched) {
							if (System.currentTimeMillis() >= e.getWhen() + 1000) {
								mousePressedDispatched = false;
								mousePressedEvent = null;
								waitForMouseReleased = true;
								super.dispatchEvent(new MouseEvent(e.getComponent(), MouseEvent.MOUSE_PRESSED, e.getWhen(), InputEvent.BUTTON2_MASK, e.getX(), e.getY(), e.getClickCount(), true));
								super.dispatchEvent(new MouseEvent(e.getComponent(), MouseEvent.MOUSE_RELEASED, e.getWhen(), InputEvent.BUTTON2_MASK, e.getX(), e.getY(), e.getClickCount(), true));
							}
							else {
								mousePressedEvent = e;
								waitForMouseReleased = false;
								postEvent(event);
							}
						}
						else {
							mousePressedDispatched = false;
						}
						// Mouse pressed events are never dispatched here since
						// they are dispatched with the other mouse event types
						return;
					}
					default:
						break;
				}
			}

			try {
				super.dispatchEvent(event);
			}
			catch (Throwable t) {
				t.printStackTrace();
			}

			// Flush to any areas the Java has painted
			if (id == PaintEvent.PAINT || id == PaintEvent.UPDATE) {
				PaintEvent e = (PaintEvent)event;
				VCLFrame frame = VCLFrame.getVCLFrame(e.getComponent());
				if (frame != null)
					frame.getGraphics().addToFlush(e.getUpdateRect());
            }

		}

	}

}
