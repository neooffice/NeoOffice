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

import com.sun.star.vcl.VCLFrame;
import java.awt.MenuBar;
import java.awt.Frame;
import java.awt.Window;
import java.util.ArrayList;
import java.util.Iterator;
import java.awt.Menu;
import java.awt.MenuItem;
import com.sun.star.vcl.VCLMenuItemData;
import java.util.Stack;
import com.sun.star.vcl.VCLEventQueue;

/**
 * The Java class that implements methods for SalMenu C++ objects that are functioning as
 * menubars.
 * <p>
 * @version 	$Revision$ $Date$
 * @author 	    $Author$
 */
public final class VCLMenuBar {
    /**
     * AWT MenuBar object which we will be calling
     */
    private MenuBar awtMenuBar = null;
    
    /**
     * VCLFrame with which this menubar is associated
     */
    private VCLFrame frame = null;
    
    /**
     * Queue to which all menu events for this menubar should be posted.
     */
    private VCLEventQueue queue = null;
    
    /**
     * ArrayList of internal VCLMenuItemData objects.  Each menu is managed by an individual VCLMenuItemData
     * object.
     */
    private ArrayList menus = new ArrayList();
        
    /**
     * Construct a new VCLMenuBar instance.  Before the menubar can be displayed it must be attached to a frame.
     *
	 * @param q event queue into which VCL menu events for this menubar's menu items should be placed
     * @see setFrame
     */
     public VCLMenuBar(VCLEventQueue q) {
        awtMenuBar=new MenuBar();
        addNewMenuBar(this);
		queue=q;
     }
     
     /**
      * Called on C++ object destruction for notification that any native objects can be destructed.  Free
      * any AWT objects and set references to NULL to allow garbage collection to cleanup at a later time.
      */
     public void dispose() {
        removeMenuBar(this);
     	awtMenuBar=null;
	menus=null;
	queue=null;
	frame=null;
     }
     
    /**
     * Associate a menubar with a frame.  The menubar must be associated with a frame before it becomes visible
     * to the user.
     *
     * @param f	VCLFrame with which the menubar is to be associated
     */
    public void setFrame(VCLFrame f) {
      	frame=f;
        Window win=frame.getWindow();
        if(win instanceof Frame)
            ((Frame)win).setMenuBar(awtMenuBar);
    }
    
    /**
     * Return the frame to which the menubar is attached.
     *
     * @return VCLFrame that should respond to MenuBar invocatins
     */
    public VCLFrame getFrame() {
        return(frame);
    }
    
    /** 
     * Return the queue to which events should be posted for this menubar
     *
     * @return VCLEventQueue that should receive events
     */
    protected VCLEventQueue getEventQueue() {
        return(queue);
    }
    
    /**
     * Returns the AWT menu bar associated with this object
     *
     * @return AWt MenuBar
     */
    public MenuBar getAWTMenuBar() {
        return awtMenuBar;
    }
    
    /**
     * Insert a new item into the menubar.  VCL attaches menus to items where the items provide names
     * and enabled state and the menus the contents.  We will use the items temporarily to hold any name
     * and enabled state given by VCL until they attach a menu, at which time the item will receive a
     * delegate menu.
     *
     * @param menuItem	menu item to be inserted
     * @param nPos	position in the menubar where the menu should be added
     */
    public void addMenuItem(VCLMenuItemData menuItem, int nPos) {
        Stack menusToReinsert=null;
        
        if((nPos < 0) || (nPos == 65535))
            nPos=menus.size();
        
        if(!menus.isEmpty() && (nPos < menus.size())) {
            // we can't insert menus in the middle of a menubar, so we have to remove the tail ones first
            // and then reinsert them after we add the new Menu
            
            menusToReinsert=new Stack();
            for(int i=menus.size()-1; i >= nPos; i--) {
                menusToReinsert.push(awtMenuBar.getMenu(i));
                awtMenuBar.remove(i);
            }
        }
        
        menus.add(nPos, menuItem);
        
        // if we were passed an object that isn't yet a menu, insert a dummy menu object as a placeholder.
        // if we were passed a menu, insert its peer.
        
        if(menuItem.isMenu())
            awtMenuBar.add((Menu)menuItem.createAWTPeer());
        else
            awtMenuBar.add(new Menu(menuItem.getTitle()));
        
        // reinsert any menus we need to add
        
        if(menusToReinsert!=null) {
            while(!menusToReinsert.empty()) {
                awtMenuBar.add((Menu)menusToReinsert.pop());
            }
        }
    }
    
    /**
     * Remove a menu at a specific index.
     *
     * @param nPos	index of menu to remove
     */
    public void removeMenu( int nPos ) {
        Menu m=awtMenuBar.getMenu(nPos);
        awtMenuBar.remove(nPos);
        menus.remove(nPos);
        m.removeNotify();
    }
    
    /**
     * Change the menu contents associated with the menu at the given index.  The menu title and enabled
     * state will remain the same.
     *
     * @param newMenu	object providing the new contents of the menu
     * @param nPos	position in the menu ordering where the new menu should be added
     */
    public void changeMenu( VCLMenuItemData newMenu, int nPos ) {
        if( nPos < menus.size() ) {
            VCLMenuItemData oldMenu=(VCLMenuItemData)menus.get(nPos);
            newMenu.setTitle(oldMenu.getTitle());
            newMenu.setEnabled(oldMenu.getEnabled());
            
            removeMenu( nPos );
            addMenuItem( newMenu, nPos );
        }
    }
    
    /**
     * Enable the menu at a given index.
     *
     * @param nPos	position to enable
     */
    public void enableMenu( int nPos, boolean enable ) {
        if( nPos < menus.size() ) {
            VCLMenuItemData menu=(VCLMenuItemData)menus.get(nPos);
            menu.setEnabled(enable);
            if(!menu.isMenu()) {
                // we have a dummy item currently in the menubar that isn't being managed, so flip its
                // state manually
                
                if(enable)
                    awtMenuBar.getMenu(nPos).enable();
                else
                    awtMenuBar.getMenu(nPos).disable();
            }
        }
    }
    
    /**
     * Regenerate all of the menubars and recreate the peers of the menus.  We may need to do this if
     * the set of peers that's currently in the menubar becomes invalid.
     */
    public void regenerateMenuBar() {
        for(int i=awtMenuBar.countMenus()-1; i>=0; i--) {
            Menu m=awtMenuBar.getMenu(i);
            awtMenuBar.remove(i);
            m.removeNotify();
        }
        Iterator e=menus.iterator();
        while(e.hasNext()) {
            VCLMenuItemData m=(VCLMenuItemData)e.next();
            if(m.isMenu())
                awtMenuBar.add((Menu)m.createAWTPeer());
            else
                awtMenuBar.add(new Menu(m.getTitle()));
        }
    }
    
    /**
     * Used to keep track of all active menubars.  Must be accessed using synchronized methods!
     */
    private static ArrayList activeMenubars=new ArrayList();
    
    /**
     * Called when a new VCLMenuBar object is created to insert it into our tracking ArrayList
     */
    private static void addNewMenuBar(VCLMenuBar o) {
        activeMenubars.add(o);
    }
    
    /**
     * Called when a VCLMenuBar object is destroyed to remove it from our tracking ArrayList
     */
    private static void removeMenuBar(VCLMenuBar o) {
        activeMenubars.remove(activeMenubars.indexOf(o));
    }
    
    /**
     * Given an AWT MenuItem or subclass, locate the specific VCLMenuBar object that contains that item
     *
     * @param item	item to be searched for
     * @return VCLFrame whose menubar is associated with the item, or null if the item could not
     *	be located in any menubar associated with a VCLFrame.
     */
    public static VCLMenuBar findVCLMenuBar(MenuItem item) {
        Iterator menuBars=activeMenubars.iterator();
        while(menuBars.hasNext()) {
            VCLMenuBar vmb=(VCLMenuBar)menuBars.next();
            if(vmb.getAWTMenuBar()==null)
                continue;
            
            // we'll locate the item by checking object references directliy.  We don't want equivalence
            // in content, we want identical objects.
            
            MenuBar mb=vmb.getAWTMenuBar();
            for(int i=0; i<mb.countMenus(); i++)
            {
                Menu m=mb.getMenu(i);
                if((m==item) || menuContainsItem(m, item))
                    return(vmb);
            }
        }
        return(null);
    }
    
    /**
     * Because a menu may contain a number of submenus, we need to recurse through the entire
     * AWT menu structure to perform our object location.  This function recursively looks at
     * the items of a menu and submenu to determine if the menu contains the item.
     *
     * @param m	menu to be searched
     * @param item	item being searched for
     * @return true if the menu contains the item, false if not
     */
    private static boolean menuContainsItem(Menu m, MenuItem item) {
        for(int i=0; i<m.countItems(); i++) {
            MenuItem mi=m.getItem(i);
            if(mi==item)
                return(true);
            if((mi instanceof Menu) && menuContainsItem((Menu)mi, item))
                return(true);
        }
        return(false);
    }
    
    /**
     * At runtime, the AWT peers implementing menus and menu items may need to have their classes changed
     * on the fly.  When this happens, we'll destroy and recreate all of the menubars to make sure each
     * menu bar contains peers of the proper classes.
     */
    public static void regenerateAllMenuBars() {
        Iterator menuBars=activeMenubars.iterator();
        while(menuBars.hasNext()) {
            VCLMenuBar mb=(VCLMenuBar)menuBars.next();
            mb.regenerateMenuBar();
        }
    }
    
    /**
     * Determine if a menu is one of the top-level menus of a menubar.
     *
     * @param item  item to check if it is a top level menu of a menubar
     * @return true if the item is in a menubar, false if the item is either
     *	a submenu or a menu item
     */
    public static boolean isTopLevelMenu(VCLMenuItemData item) {
	Iterator menuBars=activeMenubars.iterator();
	while(menuBars.hasNext()) {
	    VCLMenuBar mb=(VCLMenuBar)menuBars.next();
	    if(mb.menus!=null) {
		Iterator e=mb.menus.iterator();
		while(e.hasNext()) {
		    if(item==e.next())
			return(true);
		}
	    }
	}
	return(false);
    }
    
    /**
     * Bugs in the 1.3.1 VM on Mac OS X prevent the state of a checkbox menu
     * item from being set properly unless its peer is already instantiated.
     * We need to periodically run through all of the checkbox menu items in
     * the menubar and perform the initial set of the checkbox menu item
     * states after their peers are created.  (Bug 182)
     */
    public void syncCheckboxMenuItemState() {
	Iterator menuIter=menus.iterator();
	while(menuIter.hasNext()) {
	    ((VCLMenuItemData)menuIter.next()).fixCheckboxMenuItemState();
	}
    }
}
