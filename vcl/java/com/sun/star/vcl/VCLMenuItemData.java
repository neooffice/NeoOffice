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
 
import java.lang.String;
import java.lang.System;
import java.util.ArrayList;
import java.util.Iterator;
import java.lang.Exception;
import java.lang.IllegalArgumentException;
import java.awt.Menu;
import java.awt.MenuItem;
import java.awt.CheckboxMenuItem;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.MenuShortcut;
import com.sun.star.vcl.VCLEvent;
import com.sun.star.vcl.VCLEventQueue;
import com.sun.star.vcl.VCLMenuBar;
import com.sun.star.vcl.VCLFrame;

 /**
  * Instances of this class are used to hold information needed to construct a menu item
  * or menu corresponding to VCL menu items.  Menus are considered to be ArrayLists of these
  * items.
  */
public final class VCLMenuItemData {
    /**
     * Delagate object that performs work for us, if applicable
     */
    private VCLMenuItemData delegate = null;
    
    /**
     * Object that uses this menu item as its delegate.  Used to synchronize
     * data changes back to parent objects.
     */
    private VCLMenuItemData delegateForObject = null;
    
    /**
     * Set the delegate object for this instance
     *
     * @param d	new delegate
     */
    public void setDelegate(VCLMenuItemData d) {
        delegate=d;
        if (d != null)
            d.delegateForObject=this;
    }
    
    /**
     * Fetch the delegate object for this instance
     *
     * @return delegate object reference or null if there is no delegate
     */
    public VCLMenuItemData getDelegate() {
	return(delegate);
    }
    
    /**
     * Unicode string that corresponds to the title
     */
    private String title=new String();
    
    /**
     * Keyboard shortcut to use
     */
    private MenuShortcut keyboardShortcut=null;
    
    /**
     * Identifier that is used in Sal events to identify this specific menu item in VCL events
     */
    private short vclID=0;
    
    /**
     * Cookie needed to make the association between the C++ VCL Menu object that is spawning
     * Sal events.  Used in conjunction with the id.  Note that this is actually a void *
     * pointer (!).  For now we'll use the int Java type which i s32 bits
     */
    private int vclMenuCookie=0;
    
    /**
     * True if this item is a separator, false if not
     */
    private boolean isSeparator=false;
    
    /**
     * True if this item is a submenu, false if not
     */
    private boolean isSubmenu=false;
    
    /**
     * If the item is a submenu, ArrayList containing all of the menu items comprising the menu.
     * The items are stored as VCLMenuItemData references.
     */
    private java.util.ArrayList menuItems=new ArrayList();
    
    /**
     * If the item has been inserted into menus, this ArrayList holds
     * backreferences to the parent menus.  The backreferences are to the
     * VCLMenuItemData objects for the parent menus.
     */
    private java.util.ArrayList parentMenus=new ArrayList();
    
    /**
     * True if this item is enabled, false if not
     */
    private boolean isEnabled=true;
    
    /**
     * True if this item is a checkbox, false if not
     */
    private boolean isCheckbox=false;
    
    /**
     * True if this item is checked, false if it is not.  If the item is checked, it should also be a checkbox
     */
    private boolean isChecked=false;
    
    /**
     * ArrayList of AWT objects that have been generated for this set of menu item data and are being managed
     * by it.
     */
    private java.util.ArrayList awtPeers=new ArrayList();
    
    /**
     * Construct and initialize a new <b>VCLMenuItemData</b> instance
     *
     * @param newTitle		initial value of the title of the menu item
     * @param separator		true if the item is to be a separator, false if it is to be any other type of item
     * @param id		identifier associated with this item that's require for posting VCL events
     * @param cookie		cookie used in Sal events to tie the id to a specific menu
     */
    public VCLMenuItemData(String newTitle, boolean separator, short id, int cookie) {
        if(separator)
        {
            isSeparator=true;
            title=new String("-");
        }
        else if(newTitle!=null)
	{
            title=new String(newTitle);
	}
	
        vclID=id;
        vclMenuCookie=cookie;
    }
    
    /**
     * Clean up as many references to this item and its peers as we can to allow
     * garbage collection and destruction of their heavyweight peers.
     *
     * Note that this does not instruct an item's delegate object to be
     * disposed;  delegates must manually be disposed.
     */
    public void dispose() {
	unregisterAllAWTPeers();
	keyboardShortcut=null;
	menuItems=null;
	title=null;
    if(delegateForObject!=null)
        delegateForObject.setDelegate(null);
    }
        
    /**
     * Fetch the current title of the menu item.
     */
    public String getTitle() {
        if(delegate!=null)
            return(delegate.getTitle());
            
        return(title);
    };
    
    /**
     * Change the title of the menu item.  Any AWT peers will be automatically updated.
     *
     * @param newTitle		new title
     */
    public void setTitle(String newTitle) {
        if(delegate!=null) {
            delegate.setTitle(newTitle);
            return;
        }
        
        if(!isSeparator) {
            if(newTitle!=null)
                title=new String(newTitle);
            else
                title=new String();
            
            // if we're a delegate for an object, set the title for our
            // parent object so if the delegate gets switched the new delegate
            // can retain the title.
            
            if(delegateForObject!=null)
                delegateForObject.title=new String(title);
            
	    if(!awtPeers.isEmpty()) {
		Iterator e=awtPeers.iterator();
		while(e.hasNext()) {
		    MenuItem m=(MenuItem)e.next();
		    m.setLabel(title);
		}
	    }
        }
    }
    
    /**
     * Change the keyboard shortcut of the menu item.  Any AWT peers will be automatically updated.
     *
     * @param key	VCL keycode of the new key to use as the shortcut
     * @param useShift  true if the shift key should additionally be required
     *			for the shortcut, false if just command is needed
     *			as modifier.
     */
    public void setKeyboardShortcut(int key, boolean useShift) {
        if(delegate!=null) {
            delegate.setKeyboardShortcut(key, useShift);
            return;
        }
        
        int newShortcut=VCLEvent.convertVCLKeyCode(key);
        if(newShortcut!=0) {
            keyboardShortcut=new MenuShortcut(newShortcut, useShift);
            if(!awtPeers.isEmpty()) {
                Iterator e=awtPeers.iterator();
                while(e.hasNext()) {
                    MenuItem m=(MenuItem)e.next();
                    m.setShortcut(keyboardShortcut);
                }
            }
        }
    }
    
    /**
     * Fetch the current enabled state of the menu item.
     */
    public boolean getEnabled() {
        if(delegate!=null)
            return(delegate.getEnabled());
            
        return(isEnabled);
    }
    
    /**
     * Change the current enabled state of the menu item.  Any AWT peers will be automatically updated.
     *
     * @param newEnabled	true if the item should be enabled, false if it should be disabled
     */
    public void setEnabled(boolean newEnabled) {
        if(delegate!=null) {
            delegate.setEnabled(newEnabled);
            return;
        }
        
        isEnabled=newEnabled;
        
        // if we're the delegate for an object, make that underlying object's
        // state change as well so if the delegate gets switched the enabled
        // state can get retained
        
        if(delegateForObject!=null)
            delegateForObject.isEnabled=newEnabled;
        
	if(!awtPeers.isEmpty()) {
	    Iterator e=awtPeers.iterator();
	    while(e.hasNext()) {
		MenuItem m=(MenuItem)e.next();
		if(isEnabled)
		    m.enable();
		else
		    m.disable();
	    }
        }
    }
    
    /**
     * Get the id corresponding to the menu item that should be used to refer to this menu item in VCL
     * events.
     */
    public short getVCLID() {
        if(delegate!=null)
            return(delegate.getVCLID());
            
        return(vclID);
    }
    
    /**
     * Get the cookie corresponding to the menu item that should be used to refer to this menu in VCL
     * events.
     */
    public int getVCLCookie() {
        if(delegate!=null)
            return(delegate.getVCLCookie());
            
        return(vclMenuCookie);
    }
    
    /**
     * Fetch the current checked state of the menu item
     */
    public boolean getChecked() {
        if(delegate!=null)
            return(delegate.getChecked());
            
        return(isChecked);
    }
    
    /**
     * Change the current checked state of the menu item.  Note that changing the checked state may have
     * side effects that invalidate the AWT peers.  Some items, such as separators and items that are actually
     * submenus, cannot be checked.
     *
     * @param newCheck	true if the item should be checked, false if unchecked
     * @return true if the state change requires AWT peers to be refreshed
     *  to appear correctly, false if all required changes have been
     *  propogated to their peers
     */
    public boolean setChecked(boolean newCheck) {
        if(delegate!=null) {
            return(delegate.setChecked(newCheck));
        }
        
        boolean peersInvalidated=false;
        
        isChecked=newCheck;
        if(!isCheckbox)
        {
            if(isChecked)
            {
                isCheckbox=true;
                
		// we were just set to checked, so we need to use instances of CheckMenuItem AWT objects instead
                // of regular MenuItems.  We need to invalidate our peers.
                
                if(!awtPeers.isEmpty())
                {
                    unregisterAllAWTPeers();
                    peersInvalidated=true;
                }
            }
        }
        else
        {
            // change state of our checkbox peers
            
	    if(!awtPeers.isEmpty()) {
		Iterator e=awtPeers.iterator();
		while(e.hasNext())
		{
		    CheckboxMenuItem cMI=(CheckboxMenuItem)e.next();
		    cMI.setState(isChecked);
		}
	    }
        }
        
        return(peersInvalidated);
    }
    
    /**
     * Determine if the item is currently a menu or a regular item.  Adding the first menu item will
     * change an object into a menu
     *
     * @return true if object is a menu, false if just a single item
     */
    public boolean isMenu() {
        if(delegate!=null)
            return(delegate.isMenu());
        
        return(isSubmenu);
    }
    
    /**
     * Mark a menu item as designated for a submenu prior to the insertion of any elements into it.
     * This should only be used by VCLMenu constructors.
     *
     * @return true if the peers were invalidated and need dto be reinserted,
     *  false if all changes hav been properly sent to peers
     */
    protected boolean makeMenu() {
        if(delegate!=null) {
            return(delegate.makeMenu());
        }
        boolean peersInvalidated=false;
        isSubmenu=true;
        if(!awtPeers.isEmpty())
        {
            unregisterAllAWTPeers();
                peersInvalidated=true;
        }
        return(peersInvalidated);
    }
    
    /**
     * Add in a new menu item at a particular position in the menu.  Note that adding a menu item may
     * result in invalidating the AWT peers as it indicates the item must in fact be a menu.
     *
     * @param newItem	item to be added into the menu
     * @param nPos	position at which the item should be inserted.  Any item in that position or
     *			occuring after that position will be pushed down in the menu order
     * @return true if the peers were invalidated and must be reinserted into
     *  their parents, false if the changes have already successfully propogated
     *  to any AWT peers.
     */
    public boolean addMenuItem(VCLMenuItemData newItem, short nPos) {
        if(delegate!=null) {
            return(delegate.addMenuItem(newItem, nPos));
        }
        
        boolean peersInvalidated=false;
        
        if(nPos < 0)
            nPos=(short)menuItems.size();
        
        menuItems.add(nPos, newItem);
	newItem.parentMenus.add(this);
        if(!isSubmenu)
        {
                isSubmenu=true;
            if(!awtPeers.isEmpty())
            {
                unregisterAllAWTPeers();
                peersInvalidated=true;
            }
        }
        else
        {
	    if(!awtPeers.isEmpty()) {
		Iterator e=awtPeers.iterator();
		while(e.hasNext())
		{
		    Menu m=(Menu)e.next();
		    m.insert((MenuItem)newItem.createAWTPeer(), nPos);
                    
                    // Java 1.3.1 AWT has problems inserting checkmark menu
                    // items that are already checked, so manually toggle
                    // the checkmark after the item has been added to a menu.
                    // When in a menu, the checkbox state can be set properly.
                    
                    if(newItem.getChecked()) {
                        peersInvalidated=(peersInvalidated || newItem.setChecked(true));
                    }
		}
	    }
        }
        
        return(peersInvalidated);
    }
    
    /**
     * Remove a menu item at a particular position.  This only applies for menu style menu items.
     *
     * @param nPos	position of item to delete
     */
    public void removeMenuItem(short nPos) {
        if(delegate!=null) {
            delegate.removeMenuItem(nPos);
            return;
        }
        
        if(nPos < 0)
            nPos=(short)menuItems.size();
        
	((VCLMenuItemData)menuItems.get(nPos)).parentMenus.remove(this);
        menuItems.remove(nPos);
        if(!awtPeers.isEmpty())
        {
            Iterator e=awtPeers.iterator();
            while(e.hasNext())
            {
                Menu m=(Menu)e.next();
                MenuItem obj=m.getItem(nPos);
                m.remove(nPos);
                obj.removeNotify();
            }
        }
    }
    
    /**
     * Retrieve a menu item at a particular position.  This only applies for menu style menu items.
     *
     * @param nPos	position of item to retrieve
     */
    public VCLMenuItemData getMenuItem(short nPos) {
        if(delegate!=null) {
            return(delegate.getMenuItem(nPos));
        }
                
        return((VCLMenuItemData)menuItems.get(nPos));
    }
    
    /**
     * Determine the position of a specific menu item.  This only applies for
     * menu style menu items.  Comparison is done on a reference level, *not*
     * for items of equivalent contents.  The references must match for the
     * item to be found.
     *
     * @param item  item whose position should be retrieved
     * @return index of the item in the menu or -1 if the item is not in the
     * menu
     */
    public short getMenuItemIndex(VCLMenuItemData item) {
	if(delegate!=null) {
	    return(delegate.getMenuItemIndex(item));
	}
		
	short toReturn=-1;
	for(short i=0; i<menuItems.size(); i++) {
	    if(menuItems.get(i)==item) {
		toReturn=i;
		break;
	    }
	}
	
	return(toReturn);
    }
    
    /**
     * Fetch the number of menu items in this menu
     *
     * @return number of menu item elements
     */
    public short getNumMenuItems() {
        if(delegate!=null) {
            return(delegate.getNumMenuItems());
        }
        
        return((short)menuItems.size());
    }
    
    /**
     * Subclass of AWT MenuItem that allows the item to respond to AWT menu selections by posting
     * appropriate events into the VCL event queue.
     *
     * @see MenuItem
     * @see VCLEventQueue
     */
    static final class VCLAWTMenuItem extends MenuItem implements ActionListener {
        /**
         * Menu item data associated with this AWT item
         */
        private VCLMenuItemData d;
        
        /**
         * Construct a new <code>VCLAWTMenuItem</code> instance
         *
         * @param title		initial title of the menu item (may be changed later)
         * @param data		VCLMenuItemData holding the information needed to bind the AWT item to a VCL item
         */
        public VCLAWTMenuItem(String title, VCLMenuItemData data) {
            super(title);
            d=data;
            addActionListener(this);
        }
        
		/**
         * Respond to menu item choices by posting appropriate events into the
		 * event queue.
		 *
		 * @param e event spawning this action
		 */
		public void actionPerformed(ActionEvent e) {

			// Cache the shortcut if there is one
			if (d.keyboardShortcut!=null)
				VCLFrame.setLastMenuShortcutPressed(d.keyboardShortcut);

			VCLMenuBar mb=VCLMenuBar.findVCLMenuBar(this);
			if (mb!=null)
				mb.getEventQueue().postCachedEvent(new VCLEvent(VCLEvent.SALEVENT_MENUCOMMAND, mb.getFrame(), d.getVCLID(), d.getVCLCookie()));

		}

	}
    
    /**
     * Subclass of AWT CheckboxMenuItem that allows the item to respond to AWT menu selections by posting
     * appropriate events into the VCL event queue.
     *
     * @see CheckboxMenuItem
     * @see VCLEventQueue
     */
    static final class VCLAWTCheckboxMenuItem extends CheckboxMenuItem implements ActionListener {
        /**
         * Menu item data associated with this AWT item
         */
        private VCLMenuItemData d;
        
        /**
         * Construct a new <code>VCLAWTCheckboxMenuItem</code> instance
         *
         * @param title		initial title of the menu item (may be changed later)
         * @param data		VCLMenuItemData holding the information needed to bind the AWT item to a VCL item
         * @param state         initial checked state of the menu item
         */
        public VCLAWTCheckboxMenuItem(String title, VCLMenuItemData data, boolean state) {
            super(title, state);
            d=data;
            addActionListener(this);
        }
        
        /**
         * Respond to menu item choices by posting appropriate events into the queue.
         *
         * @param e	event spawning this action.  Ignored.
         */
        public void actionPerformed(ActionEvent e) {
            VCLMenuBar mb=VCLMenuBar.findVCLMenuBar(this);
            if(mb!=null) {
                mb.getEventQueue().postCachedEvent(new VCLEvent(VCLEvent.SALEVENT_MENUCOMMAND, mb.getFrame(), d.getVCLID(), d.getVCLCookie()));
            }
        }
    }
    
    /**
     * Using all of the current settings, construct an AWT object that is most appropriate for these settings.
     * While the resulting object is guaranteed to be a reference to a MenuItem or one of its subclasses, up to
     * three types may be returned:  MenuItem, CheckboxMenuItem, and Menu.  The returned object will be added
     * onto internal queues to be automatically synchronized with changes made to this menu item.
     *
     * @return AWT MenuItem for this object
     */
    public Object createAWTPeer() {
        if(delegate!=null)
            return(delegate.createAWTPeer());
            
        Object toReturn=null;
        
        if(isCheckbox)
        {
            VCLAWTCheckboxMenuItem cmi=new VCLAWTCheckboxMenuItem(getTitle(), this, getChecked());
            if(getEnabled())
                cmi.enable();
            else
                cmi.disable();
            if(keyboardShortcut!=null)
                cmi.setShortcut(keyboardShortcut);
            toReturn=(Object)cmi;
        }
        else if(isSubmenu)
        {
            Menu mn=new Menu(getTitle());
            if(getEnabled())
                mn.enable();
            else
                mn.disable();
            Iterator items=menuItems.iterator();
            while(items.hasNext()) {
                VCLMenuItemData i=(VCLMenuItemData)items.next();
                mn.add((MenuItem)i.createAWTPeer());
            }
            toReturn=(Object)mn;
        }
        else if(isSeparator)
        {
            // separator is a menu item with a label of a dash
            MenuItem sep=new MenuItem("-");
            toReturn=(Object)sep;
        }
        else
        {
            VCLAWTMenuItem mi=new VCLAWTMenuItem(getTitle(), this);
            if(getEnabled())
                mi.enable();
            else
                mi.disable();
            if(keyboardShortcut!=null)
                mi.setShortcut(keyboardShortcut);
            toReturn=(Object)mi;
        }
        
        // add the new peer onto our internal tracking lists
        
        if(toReturn!=null)
            awtPeers.add(toReturn);
        
        return(toReturn);
    }
    
    /**
     * Reinsert new peer objects for the menu item into all of the registered
     * parent menus for the menu item.
     */
    public void refreshAWTPeersInParentMenus() {
	if(parentMenus.isEmpty())
	    return;
	
	Iterator parents=parentMenus.iterator();
	while(parents.hasNext()) {
	    VCLMenuItemData parent=(VCLMenuItemData)parents.next();
                short menuPos=parent.getMenuItemIndex(this);
                if(menuPos >= 0) {
                    parent.removeMenuItem(menuPos);
                    parent.addMenuItem(this, menuPos); // creates a new peer
                }
	}
    }
    
    /**
     * Unregister a single AWT peer.  This indicates that the AWT object is no longer being used and
     * does not need to be tracked any longer.
     *
     * @param o		peer object to stop managing
     */
    public void unregisterAWTPeer(Object o) {
        if(delegate!=null) {
            delegate.unregisterAWTPeer(o);
            return;
        }
        
        MenuItem mi=(MenuItem)o;
        awtPeers.remove(awtPeers.indexOf(o));
        mi.removeNotify();
    }
    
    /**
     * Unregister all AWT peer objects that have been created from the menu item data.  If the
     * item data corresponds to a submenu, all of the submenu peers will also be unregistered.
     */
    public void unregisterAllAWTPeers() {
        if(delegate!=null) {
            delegate.unregisterAllAWTPeers();
            return;
        }
        
	// remove notifiers to allow GC to reclaim these objects quicker
	
	if((awtPeers!=null) && !awtPeers.isEmpty()) {
	    Iterator peers=awtPeers.iterator();
	    while(peers.hasNext()) {
		MenuItem mi=(MenuItem)peers.next();
		if(mi instanceof VCLAWTMenuItem) {
		    mi.removeActionListener((VCLAWTMenuItem)mi);
		} else if(mi instanceof VCLAWTCheckboxMenuItem) {
		    mi.removeActionListener((VCLAWTCheckboxMenuItem)mi);
		}
                mi.removeNotify();
	    }
	    
	    awtPeers.clear();
	}
	
        if(isSubmenu && (menuItems!=null) && !menuItems.isEmpty()) {
            Iterator e=menuItems.iterator();
            while(e.hasNext()) {
                ((VCLMenuItemData)e.next()).unregisterAllAWTPeers();
            }
        }
    }
}
