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
 *  Edward Peterlin, September 2004
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 by Edward Peterlin (OPENSTEP@neooffice.org)
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

import java.awt.Component;
import java.awt.Menu;
import java.awt.MenuBar;
import java.awt.MenuItem;
import java.awt.Frame;
import java.awt.Window;
import java.util.Iterator;
import java.util.LinkedList;

/**
 * The Java class that implements methods for SalMenu C++ objects that are
 * functioning as menubars.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 		$Author$
 */
public final class VCLMenuBar extends Component {

	/**
	 * Used to keep track of all active menubars.
	 */
	private static LinkedList activeMenubars=new LinkedList();

	/**
	 * Called when a new VCLMenuBar object is created to insert it into our
	 * tracking list.
	 */
	private static synchronized void addNewMenuBar(VCLMenuBar o) {

		activeMenubars.add(o);

	}

	/**
	 * Called when a VCLMenuBar object is destroyed to remove it from our
	 * tracking list.
	 */
	private static synchronized void removeMenuBar(VCLMenuBar o) {

		activeMenubars.remove(activeMenubars.indexOf(o));

	}

	/**
	 * Given an AWT MenuItem or subclass, locate the specific VCLMenuBar object
	 * that contains that item.
	 *
	 * @param item item to be searched for
	 * @return VCLFrame whose menubar is associated with the item, or null if
	 *  the item could not be located in any menubar associated with a VCLFrame
	 */
	static synchronized VCLMenuBar findVCLMenuBar(MenuItem item) {

		Iterator menuBars=activeMenubars.iterator();
		while(menuBars.hasNext()) {
			VCLMenuBar vmb=(VCLMenuBar)menuBars.next();
			synchronized (vmb.getTreeLock()) {
				MenuBar mb=vmb.getAWTMenuBar();
				if(mb==null)
					continue;

				// we'll locate the item by checking object references directly.
				// We don't want equivalence in content, we want identical
				// objects.

				for(int i=0; i<mb.getMenuCount(); i++) {
					Menu m=mb.getMenu(i);
					if((m==item) || menuContainsItem(m, item))
						return(vmb);
				}
			}
		}

		return(null);

	}

	/**
	 * Because a menu may contain a number of submenus, we need to recurse
	 * through the entire AWT menu structure to perform our object location.
	 * This function recursively looks at the items of a menu and submenu to
	 * determine if the menu contains the item.
	 *
	 * @param m menu to be searched
	 * @param item item being searched for
	 * @return <code>true</code> if the menu contains the item,
	 *  <code>false</code> if not
	 */
	private static boolean menuContainsItem(Menu m, MenuItem item) {

		for(int i=0; i<m.getItemCount(); i++) {
			MenuItem mi=m.getItem(i);
			if(mi==item)
				return(true);
			if((mi instanceof Menu) && menuContainsItem((Menu)mi, item))
				return(true);
		}

		return(false);

	}

	/**
	 * Determine if a menu is one of the top-level menus of a menubar.
	 *
	 * @param item item to check if it is a top level menu of a menubar
	 * @return <code>true</code> if the item is in a menubar,
	 *  <code>false</code> if the item is either a submenu or a menu item
	 */
	static synchronized boolean isTopLevelMenu(VCLMenuItemData item) {

		Iterator menuBars=activeMenubars.iterator();
		while(menuBars.hasNext()) {
			VCLMenuBar mb=(VCLMenuBar)menuBars.next();
			synchronized (mb.getTreeLock()) {
				if(mb.menus!=null) {
					Iterator e=mb.menus.iterator();
					while(e.hasNext()) {
						if(item==e.next())
							return(true);
					}
				}
			}
		}

		return(false);

	}

	/**
	 * AWT MenuBar object which we will be calling.
	 */
	private MenuBar awtMenuBar = null;

	/**
	 * The disposed flag.
	 */
	private boolean disposed = false;

	/**
	 * VCLFrame with which this menubar is associated.
	 */
	private VCLFrame frame = null;

	/**
	 * Queue to which all menu events for this menubar should be posted.
	 */
	private VCLEventQueue queue = null;

	/**
	 * List of internal VCLMenuItemData objects. Each menu is managed by
	 * an individual VCLMenuItemData object.
	 */
	private LinkedList menus = new LinkedList();

	/**
	 * Construct a new VCLMenuBar instance. Before the menubar can be displayed
	 * it must be attached to a frame.
	 *
	 * @param q event queue into which VCL menu events for this menubar's menu
	 *  items should be placed
	 * @see setFrame
	 */
	public VCLMenuBar(VCLEventQueue q) {

		awtMenuBar=new MenuBar();
		addNewMenuBar(this);
		queue=q;

	}
	
	/**
	 * Called on C++ object destruction for notification that any native
	 * objects can be destructed. Free any AWT objects and set references to
	 * null to allow garbage collection to cleanup at a later time.
	 */
	public void dispose() {

		synchronized (getTreeLock()) {
			if (disposed)
				return;

			removeMenuBar(this);

			if(frame!=null) {
				MenuBar mb = frame.getMenuBar();
				if (mb != null && mb == awtMenuBar)
					frame.setMenuBar(null);
			}

			Iterator e=menus.iterator();
			int i = 0;
			while(e.hasNext()) {
				VCLMenuItemData m=(VCLMenuItemData)e.next();
				m.unregisterAWTPeer(awtMenuBar.getMenu(i++));
			}
 
		 	awtMenuBar=null;
			menus=null;
			queue=null;
			frame=null;

			disposed = true;
		}

	}
	
	/**
	 * Associate a menubar with a frame. The menubar must be associated with a
	 * frame before it becomes visible to the user.
	 *
	 * @param f	VCLFrame with which the menubar is to be associated
	 */
	public void setFrame(VCLFrame f) {

		if(frame!=null)
			frame.setMenuBar(null);
	  	frame=f;
		if(frame!=null)
			frame.setMenuBar(awtMenuBar);

	}

	/**
	 * Return the frame to which the menubar is attached.
	 *
	 * @return VCLFrame that should respond to MenuBar invocatins
	 */
	synchronized VCLFrame getFrame() {

		return(frame);

	}

	/**
	 * Return the queue to which events should be posted for this menubar.
	 *
	 * @return VCLEventQueue that should receive events
	 */
	synchronized VCLEventQueue getEventQueue() {

		return(queue);

	}

	/**
	 * Returns the AWT menu bar associated with this object.
	 *
	 * @return AWT MenuBar
	 */
	MenuBar getAWTMenuBar() {

		return awtMenuBar;

	}

	/**
	 * Insert a new item into the menubar. VCL attaches menus to items where
	 * the items provide names and enabled state and the menus the contents.
	 * We will use the items temporarily to hold any name and enabled state
	 * given by VCL until they attach a menu, at which time the item will
	 * receive a delegate menu.
	 *
	 * @param menuItem menu item to be inserted
	 * @param nPos position in the menubar where the menu should be added
	 */
	public void addMenuItem(VCLMenuItemData menuItem, short nPos) {

		synchronized (getTreeLock()) {
			short items = (short)menus.size();
			if(nPos < 0)
				nPos=items;

			if (nPos >= items) {
				// If this object is not yet a menu, insert a dummy as a
				// placeholder otherwise insert its peer.
				menus.add(nPos, menuItem);
				if(menuItem.isMenu())
					awtMenuBar.add((Menu)menuItem.createAWTPeer());
				else
					awtMenuBar.add(new Menu(menuItem.getTitle()));
			}
			else {
				MenuBar oldMenuBar = awtMenuBar;
				removeMenuBar(this);
				awtMenuBar = new MenuBar();
				addNewMenuBar(this);

				if (frame != null) {
					Window w = frame.getWindow();
					if (w instanceof Frame) {
						((Frame)w).setMenuBar(null);
						((Frame)w).setMenuBar(awtMenuBar);
					}
				}

				menus.add(nPos, menuItem);
				items = (short)menus.size();
				for(short i=0; i < items; i++) {
					// If this object is not yet a menu, insert a dummy as a
					// placeholder otherwise insert its peer.
					VCLMenuItemData mi = (VCLMenuItemData)menus.get(i);
					mi.unregisterAWTPeer(oldMenuBar.getMenu(i));
					if(mi.isMenu())
						awtMenuBar.add((Menu)mi.createAWTPeer());
					else
						awtMenuBar.add(new Menu(mi.getTitle()));
				}
			}
		}
	
	}

	/**
	 * Remove a menu at a specific index.
	 *
	 * @param nPos index of menu to remove
	 */
	public void removeMenu(short nPos) {

		synchronized (getTreeLock()) {
			if(nPos < 0)
				nPos=(short)menus.size();

			menus.remove(nPos);
			awtMenuBar.remove(nPos);
		}

	}

	/**
	 * Change the menu contents associated with the menu at the given index.
	 * The menu title and enabled state will remain the same.
	 *
	 * @param newMenu object providing the new contents of the menu
	 * @param nPos position in the menu ordering where the new menu should be
	 *  added
	 */
	public void changeMenu(VCLMenuItemData newMenu, short nPos) {

		synchronized (getTreeLock()) {
			if(nPos < 0)
				nPos=(short)menus.size();

			VCLMenuItemData oldMenu=(VCLMenuItemData)menus.get(nPos);
			newMenu.setTitle(oldMenu.getTitle());
			newMenu.setEnabled(oldMenu.getEnabled());
			menus.set(nPos, newMenu);
			if (!newMenu.reregisterAWTPeer(oldMenu, awtMenuBar)) {
				MenuBar oldMenuBar = awtMenuBar;
				removeMenuBar(this);
				awtMenuBar = new MenuBar();
				addNewMenuBar(this);

				if (frame != null) {
					Window w = frame.getWindow();
					if (w instanceof Frame) {
						((Frame)w).setMenuBar(null);
						((Frame)w).setMenuBar(awtMenuBar);
					}
				}

				short items = (short)menus.size();
				for(short i=0; i < items; i++) {
					// If this object is not yet a menu, insert a dummy as a
					// placeholder otherwise insert its peer.
					VCLMenuItemData mi = (VCLMenuItemData)menus.get(i);
					if(mi.isMenu())
						awtMenuBar.add((Menu)mi.createAWTPeer());
					else
						awtMenuBar.add(new Menu(mi.getTitle()));
				}
			}

			// let new menu provide contents, but retain reference
			// to old menu in the actual menubar.  Bug #175
			oldMenu.unregisterAllAWTPeers();
			oldMenu.setDelegate(newMenu);
		}

	}

	/**
	 * Enable the menu at a given index.
	 *
	 * @param nPos position to enable
	 */
	public void enableMenu(short nPos, boolean enable) {

		synchronized (getTreeLock()) {
			if(nPos < menus.size()) {
				VCLMenuItemData menu=(VCLMenuItemData)menus.get(nPos);
				menu.setEnabled(enable);
				if(!menu.isMenu()) {
					// we have a dummy item currently in the menubar that isn't
					// being managed, so flip its state manually
					if(enable)
						awtMenuBar.getMenu(nPos).enable();
					else
						awtMenuBar.getMenu(nPos).disable();
				}
			}
		}

	}
	
}
