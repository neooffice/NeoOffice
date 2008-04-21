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

package com.sun.star.vcl;

import java.awt.AWTEvent;
import java.awt.Component;
import java.awt.Container;
import java.awt.DefaultKeyboardFocusManager;
import java.awt.EventQueue;
import java.awt.Frame;
import java.awt.Insets;
import java.awt.KeyboardFocusManager;
import java.awt.Panel;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.Toolkit;
import java.awt.Window;
import java.awt.event.ComponentEvent;
import java.awt.event.FocusEvent;
import java.awt.event.InputEvent;
import java.awt.event.InputMethodEvent;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseWheelEvent;
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
	 * The GC_DISPOSED_PIXELS constant.
	 */
	public final static long GC_DISPOSED_PIXELS = 1024 * 1024;

	/**
	 * The GC_INTERVAL_1 constant.
	 */
	public final static long GC_INTERVAL_1 = 60000;

	/**
	 * The GC_INTERVAL_2 constant.
	 */
	public final static long GC_INTERVAL_2 = GC_INTERVAL_1 / 4;

	/**
	 * The GC_INTERVAL_3 constant.
	 */
	public final static long GC_INTERVAL_3 = GC_INTERVAL_2 / 3;

	/**
	 * The GC_MEMORY_2 constant.
	 */
	public final static long GC_MEMORY_2 = 4 * GC_DISPOSED_PIXELS;

	/**
	 * The GC_MEMORY_3 constant.
	 */
	public final static long GC_MEMORY_3 = GC_MEMORY_2 / 2;

	/**
	 * The GC_DISPOSED_PIXELS garbage collection.
	 */
	private static long disposedPixels = 0;

	/**
	 * The next GC_INTERVAL_1 garbage collection.
	 */
	private static long nextGCInterval1 = 0;

	/**
	 * The next GC_INTERVAL_2 garbage collection.
	 */
	private static long nextGCInterval2 = 0;

	/**
	 * The next GC_INTERVAL_3 garbage collection.
	 */
	private static long nextGCInterval3 = 0;

	/**
	 * The next GC_MEMORY_2 garbage collection.
	 */
	private static long nextGCUseMemory2 = 0;

	/**
	 * The next GC_MEMORY_2 garbage collection.
	 */
	private static long nextGCUseMemory3 = 0;

	/**
	 * Post a custom mouse wheel event.
	 *
	 * @param o the <code>Window</code> peer
	 * @param x the x coordinate
	 * @param y the y coordinate
	 * @param rotationX the horizontal wheel rotation
	 * @param rotationY the vertical wheel rotation
	 * @param shiftDown <code>true</code> if the Shift key is pressed
	 * @param metaDown <code>true</code> if the Meta key is pressed
	 * @param altDown <code>true</code> if the Alt key is pressed
	 * @param controlDown <code>true</code> if the Control key is pressed
	 */
	public static void postMouseWheelEvent(Object o, int x, int y, int rotationX, int rotationY, boolean shiftDown, boolean metaDown, boolean altDown, boolean controlDown) {

		if (o == null || (rotationX == 0 && rotationY == 0))
			return;

		Window w = null;

		Frame[] frames = Frame.getFrames();
		for (int i = 0; i < frames.length; i++) {
			Window[] windows = frames[i].getOwnedWindows();
			for (int j = 0; j < windows.length; j++) {
				if (windows[j].getPeer() == o) {
					w = windows[j];
					break;
				}
			}

			if (w != null) {
				break;
			}
			else if (frames[i].getPeer() == o) {
				w = frames[i];
				break;
			}
		}

		if (w == null || !w.isVisible())
			return;

		// Fix bug 3030 by setting the modifiers. Note that we ignore the Shift
		// modifier as using it will disable horizontal scrolling.
		int modifiers = 0;
		if (metaDown)
			modifiers |= InputEvent.META_DOWN_MASK;
		if (altDown)
			modifiers |= InputEvent.ALT_DOWN_MASK;
		if (controlDown)
			modifiers |= InputEvent.CTRL_DOWN_MASK;

		// Note: no matter what buttons we press, the MouseWheelEvents in
		// Apple's JVMs always seem to have the following constant values:
		//   ScrollType == MouseWheelEvent.WHEEL_UNIT_SCROLL
		//   ScrollUnits == 1
		EventQueue eventQueue = Toolkit.getDefaultToolkit().getSystemEventQueue();
		if (rotationX != 0)
			eventQueue.postEvent(new MultidirectionalMouseWheelEvent(w, MouseEvent.MOUSE_WHEEL, System.currentTimeMillis(), modifiers, x, y, 0, false, MouseWheelEvent.WHEEL_UNIT_SCROLL, 1, rotationX, true));
		if (rotationY != 0)
			eventQueue.postEvent(new MultidirectionalMouseWheelEvent(w, MouseEvent.MOUSE_WHEEL, System.currentTimeMillis(), modifiers, x, y, 0, false, MouseWheelEvent.WHEEL_UNIT_SCROLL, 1, rotationY, false));

	}

	/**
	 * Run the garbage collector if necessary.
	 */
	static void runGCIfNeeded(long pixels) {

		boolean needToRunGC = false;

		if (pixels > 0)
			disposedPixels += pixels;

		if (disposedPixels >= GC_DISPOSED_PIXELS) {
			needToRunGC = true;
		}
		else {
			Runtime runtime = Runtime.getRuntime();
			long currentTime = System.currentTimeMillis();
			long currentUseMemory = runtime.totalMemory() - runtime.freeMemory();
			if (currentTime >= nextGCInterval1 || (currentTime >= nextGCInterval2 && currentUseMemory >= nextGCUseMemory2) || (currentTime >= nextGCInterval3 && currentUseMemory >= nextGCUseMemory3))
				needToRunGC = true;
		}

		if (needToRunGC) {
			System.gc();

			Runtime runtime = Runtime.getRuntime();
			long currentTime = System.currentTimeMillis();
			long currentUseMemory = runtime.totalMemory() - runtime.freeMemory();
			disposedPixels = 0;
			nextGCInterval1 = currentTime + GC_INTERVAL_1;
			nextGCInterval2 = currentTime + GC_INTERVAL_2;
			nextGCInterval3 = currentTime + GC_INTERVAL_3;
			nextGCUseMemory2 = currentUseMemory + GC_MEMORY_2;
			nextGCUseMemory3 = currentUseMemory + GC_MEMORY_3;
		}

	}

	/**
	 * The last adjusted mouse modifiers.
	 */
	private int lastAdjustedMouseModifiers = 0;

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
	 *
	 * @param isPanther <code>true</code> if we are running on Mac OS X 10.3.x
	 */
	public VCLEventQueue(boolean isPanther) {

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

		KeyboardFocusManager kfm = KeyboardFocusManager.getCurrentKeyboardFocusManager();

		// Set keyboard focus manager
		if (!isPanther) {
			kfm.setCurrentKeyboardFocusManager(new NoEnqueueKeyboardFocusManager(this));
			kfm = KeyboardFocusManager.getCurrentKeyboardFocusManager();
		}

		// Set the keyboard focus manager so that Java's default focus
		// switching key events are passed are not consumed
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
	 * thread or the application's main thread.
	 */
	public void dispatchNextEvent() {

		try {
			VCLEventQueue.NoExceptionsEventQueue eventQueue = (VCLEventQueue.NoExceptionsEventQueue)Toolkit.getDefaultToolkit().getSystemEventQueue();

			if (EventQueue.isDispatchThread()) {
				// Post a dummy, low priority event to ensure that we don't
				// block if there are no pending events
				PaintEvent e = new PaintEvent(new Container(), PaintEvent.PAINT, new Rectangle());
				eventQueue.postEvent(e);
				AWTEvent nextEvent;
				while ((nextEvent = eventQueue.getNextEvent()) != e)
					eventQueue.dispatchEvent(nextEvent);
			}
			else if (isApplicationMainThread()) {
				// Don't post or dispatch, just wait until there are no pending
				// events
				AWTEvent nextEvent;
				while ((nextEvent = eventQueue.peekEvent()) != null) {
					runApplicationMainThreadTimers();
					Thread.yield();
				}
			}
		}
		catch (Throwable t) {
			t.printStackTrace();
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
				VCLEventQueue.runGCIfNeeded(0);

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
	 * Returns <code>true</code> if the application has a delegate and it is
	 * not the default Java delegate.
	 *
	 * @return <code>true</code> if the application has a delegate and it is
	 *  not the default Java delegate
	 */
	public native boolean hasApplicationDelegate();

	/**
	 * Returns <code>true</code> if the application is active and a native
	 * modal window is not showing.
	 *
	 * @return <code>true</code> if the application is active and a native
	 *  modal window is not showing.
	 */
	native boolean isApplicationActive();

	/**
	 * Returns <code>true</code> if the current thread is the application's
	 * main thread.
	 *
	 * @return <code>true</code> if the  current thread is the application's
	 *  main thread
	 */
	public native boolean isApplicationMainThread();

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
						if (queue.tail != null && !queue.tail.remove && queue.tail.event.getID() == VCLEvent.SALEVENT_WHEELMOUSE && queue.tail.event.isHorizontal() == newItem.event.isHorizontal() && queue.tail.event.getFrame() == newItem.event.getFrame()) {
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
	 * Runs any pending native timers.
	 */
	native void runApplicationMainThreadTimers();

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
		 * The cached getLocation() method.
		 */
		private Method pointerInfoGetLocationMethod = null;

		/**
		 * The <code>VCLEventQueue</code>.
		 */
		private VCLEventQueue queue = null;

		/**
		 * The cached getPointerInfo() method.
		 */
		private Method toolkitGetPointerInfoMethod = null;

		/**
		 * Construct a <code>NoExceptionsEventQueue</code> instance.
		 *
		 * @param q the <code>VCLEventQueue</code>
		 */
		NoExceptionsEventQueue(VCLEventQueue q) {

			queue = q;

			// Load mouse info peer instance
			try {
				Class c = Class.forName("java.awt.MouseInfo");
				toolkitGetPointerInfoMethod = c.getMethod("getPointerInfo", new Class[]{});
				Object o = toolkitGetPointerInfoMethod.invoke(null, new Object[]{});
				pointerInfoGetLocationMethod = o.getClass().getMethod("getLocation", new Class[]{});
			}
			catch (Throwable t) {
				t.printStackTrace();
			}

		}

		/**
		 * Dispatch an event.
		 *
		 * @param event the event to dispatch
		 */
		protected void dispatchEvent(AWTEvent event) {

			try {
				// Avoid duplicate mouse wheel events by only allowing our
				// custom MultidirectionalMouseWheelEvent events to be
				// dispatched
				if (event instanceof MouseWheelEvent && !(event instanceof VCLEventQueue.MultidirectionalMouseWheelEvent)) {
					((MouseWheelEvent)event).consume();
					return;
				}

				super.dispatchEvent(event);

				if (event instanceof ComponentEvent && event.getID() == ComponentEvent.COMPONENT_MOVED && toolkitGetPointerInfoMethod != null && pointerInfoGetLocationMethod != null) {
					// Fix bug 2769 by creating synthetic mouse dragged events
					// when moving a window by dragging its title bar
					Component c = ((ComponentEvent)event).getComponent();
					if (c instanceof Window && c.isShowing()) {
						Window w = (Window)c;
						Point screenLocation = w.getLocationOnScreen();
						Rectangle bounds = new Rectangle(screenLocation.x, screenLocation.y, w.getSize().width, w.getInsets().top);
						if (!bounds.isEmpty()) {
							Object o = toolkitGetPointerInfoMethod.invoke(Toolkit.getDefaultToolkit(), new Object[]{});
							if (o != null) {
								Point mouseLocation = (Point)pointerInfoGetLocationMethod.invoke(o, new Object[]{});
								if (mouseLocation != null) {
									if (mouseLocation.x < bounds.x)
										mouseLocation.x = bounds.x;
									else if (mouseLocation.x >= bounds.x + bounds.width)
										mouseLocation.x = bounds.x + bounds.width;

									if (mouseLocation.y < bounds.y)
										mouseLocation.y = bounds.y;
									else if (mouseLocation.y >= bounds.y + bounds.height)
										mouseLocation.y = bounds.y + bounds.height;

									MouseEvent mouseDraggedEvent = new MouseEvent(w, MouseEvent.MOUSE_DRAGGED, System.currentTimeMillis(), MouseEvent.BUTTON1_MASK | MouseEvent.BUTTON1_DOWN_MASK, mouseLocation.x - screenLocation.x, mouseLocation.y - screenLocation.y, 1, false);
									super.dispatchEvent(mouseDraggedEvent);
								}
							}
						}
					}
				}
				else if (event instanceof KeyEvent) {
					// The modifiers for mouse released events contain the
					// modifiers after the event has occurred so we need to
					// replace the modifiers with the modifiers that were
					// released
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
					// The modifiers for mouse released events contain the
					// modifiers after the event has occurred so we need to
					// replace the modifiers with the modifiers that were
					// released
					int id = event.getID();
					if (id == MouseEvent.MOUSE_PRESSED || id == MouseEvent.MOUSE_RELEASED) {
						queue.setLastAdjustedMouseModifiers(((MouseEvent)event).getModifiersEx());
					}
				}
			}
			catch (Throwable t) {
				// Fix bug 2502 by setting the JVM's cached display list to
				// null and redispatching the event
				if (event instanceof sun.awt.PeerEvent) {
					try {
						VCLScreen.clearCachedDisplays();
						super.dispatchEvent(event);
					}
					catch (Throwable t2) {
						t2.printStackTrace();
					}
				}
				else {
					t.printStackTrace();
				}
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
		 * The queue.
		 */
		private VCLEventQueue queue = null;

		/**
		 * Construct a VCLEventQueue.NoEnqueueKeyboardFocusManager instance.
		 *
		 * @param q the queue
		 */
		NoEnqueueKeyboardFocusManager(VCLEventQueue q) {

			queue = q;

		}

		/**
		 * Fix bug 1715 by ensuring that the focus owner does not get set to
		 * null if there is a permanent focus owner.
		 */
		public boolean dispatchEvent(AWTEvent e) {

			boolean ret = super.dispatchEvent(e);

			// Fix bug 1924 by checking if a native modal window is showing
			if (getFocusOwner() == null && queue.isApplicationActive()) {
				Component c = getPermanentFocusOwner();
				if (c != null)
					c.requestFocusInWindow();
			}

			return ret;

		}

		/**
		 * Fix bug 1715 by never enqueue any events.
		 *
		 * @param after timestamp of current event, or the current, system time
		 *  if the current event has no timestamp, or the AWT cannot determine
		 *  which event is currently being handled
		 * @param untilFocused the component which will receive a FOCUS_GAINED
		 *  event before any pending key events
		 */
		protected void enqueueKeyEvents(long after, Component untilFocused) {}

	}

	/**
	 * The <code>NoEnqueueKeyboardFocusManager</code> is a subclass of
	 * the <code>DefaultKeyboardFocusManager</code> class that does not enqueue
	 * any key events.
	 */
	static final class MultidirectionalMouseWheelEvent extends MouseWheelEvent {

		/**
		 * The horizontal flag.
		 */
		private boolean horizontal = false;

		/**
		 * Construct a VCLEventQueue.MultidirectionalMouseWheelEvent instance.
		 *
		 * @param source the <code>Component</code> that originated the event
		 * @param id the integer that identifies the event
		 * @param when a long that gives the time the event occurred
		 * @param modifiers the modifier keys
		 * @param x the horizontal x coordinate
		 * @param y the vertical y coordinate
		 * @param clickCount the number of mouse clicks
		 * @param popupTrigger <code>true</code> if this event is a trigger
		 *  for a popup-menu
		 * @param scrollType the type of scrolling which should take place
		 * @param scrollAmount the number of units to be scrolled
		 * @param wheelRotation the amount of rotation
		 * @param h <code>true</code> if the scrolling is horizontal
		 */
		MultidirectionalMouseWheelEvent(Component source, int id, long when, int modifiers, int x, int y, int clickCount, boolean popupTrigger, int scrollType, int scrollAmount, int wheelRotation, boolean h) {

			super(source, id, when, modifiers, x, y, clickCount, popupTrigger, scrollType, scrollAmount, wheelRotation);
			horizontal = h;

		}

		/**
		 * Returns <code>true</code> if scrolling is horizontal.
		 *
		 * @return <code>true</code> if scrolling is horizontal
		 */
		public boolean isHorizontal() {

			return horizontal;

		}

	}

}
