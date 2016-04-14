/*************************************************************************
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * $RCSfile$
 * $Revision$
 *
 * This file is part of NeoOffice.
 *
 * NeoOffice is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * NeoOffice is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 3 along with NeoOffice.  If not, see
 * <http://www.gnu.org/licenses/gpl-3.0.txt>
 * for a copy of the GPLv3 License.
 *
 * Modified August 2013 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/

#ifndef _SV_NATIVEWIDGETS_HXX
#define _SV_NATIVEWIDGETS_HXX

#include <rtl/ustring.hxx>
#include <vcl/dllapi.h>
#include <tools/gen.hxx>

/* Control Types:
 *
 *   Specify the overall, whole control
 *   type (as opposed to parts of the
 *   control if it were composite).
 */

typedef sal_uInt32		ControlType;

// Normal PushButton/Command Button
#define CTRL_PUSHBUTTON			1

// Normal single radio button
#define CTRL_RADIOBUTTON			2

// Normal single checkbox
#define CTRL_CHECKBOX			10

// Combobox, i.e. a ListBox
// that allows data entry by user
#define CTRL_COMBOBOX			20

// Control that allows text entry
#define CTRL_EDITBOX			30

// Control that allows text entry, but without the usual border
// Has to be handled separately, because this one cannot handle
// HAS_BACKGROUND_TEXTURE, which is drawn in the edit box'es
// border window.
#define CTRL_EDITBOX_NOBORDER	31

// Control that allows text entry
// ( some systems distingish between single and multi line edit boxes )
#define CTRL_MULTILINE_EDITBOX 32

// Control that pops up a menu,
// but does NOT allow data entry
#define CTRL_LISTBOX			35

// An edit field together with two little 
// buttons on the side (aka spin field)
#define CTRL_SPINBOX			40

// Two standalone spin buttons
// without an edit field
#define CTRL_SPINBUTTONS		45

// A single tab
#define CTRL_TAB_ITEM			50

// The border around a tab area,
// but without the tabs themselves.
// May have a gap at the top for
// the active tab
#define CTRL_TAB_PANE			55

// Background of a Tab Pane
#define CTRL_TAB_BODY			56

// Normal scrollbar, including
// all parts like slider, buttons
#define CTRL_SCROLLBAR			60

// Border around a group of related
// items, perhaps also displaying
// a label of identification
#define CTRL_GROUPBOX			70

// A separator line
#define CTRL_FIXEDLINE			80

// A rectangular border, like a
// Tab Pane, but without the
// possible gap for a tab
#define CTRL_FIXEDBORDER		90

// A toolbar control with buttons and a grip
#define CTRL_TOOLBAR		    100

// The menubar
#define CTRL_MENUBAR		    120
// popup menu
#define CTRL_MENU_POPUP         121

// The statusbar
#define CTRL_STATUSBAR		    130
#define CTRL_PROGRESS           131
// Progress bar for the intro window
// (aka splash screen), in case some
// wants native progress bar in the 
// application but not for the splash
// screen (used in desktop/)
#define CTRL_INTROPROGRESS      132

// tool tips
#define CTRL_TOOLTIP            140

// to draw the implemented theme
#define CTRL_WINDOW_BACKGROUND  150

//to draw border of frames natively
#define CTRL_FRAME              160

// for nodes in listviews
// used in svtools/source/contnr/svtreebx.cxx
#define CTRL_LISTNODE           170
// nets between elements of listviews
// with nodes
#define CTRL_LISTNET            171

#ifdef USE_JAVA

// hierarchical tree control expand/collapse handle
#define CTRL_DISCLOSUREBTN		230

// list view header cell appearing above columns in lists
#define CTRL_LISTVIEWHEADER		240

// native frame to draw around list views.  Default is sunken Win95 well.
#define CTRL_LISTVIEWBOX		250

#endif	// USE_JAVA


/* Control Parts:
 *
 *   Uniquely identify a part of a control,
 *   for example the slider of a scroll bar.
 */

typedef sal_uInt32		ControlPart;

#define PART_ENTIRE_CONTROL		1
#define PART_WINDOW             5       // the static listbox window containing the list
#define PART_BUTTON				100
#define PART_BUTTON_UP			101
#define PART_BUTTON_DOWN			102	// Also for ComboBoxes/ListBoxes
#define PART_BUTTON_LEFT			103
#define PART_BUTTON_RIGHT		104
#define PART_ALL_BUTTONS    		105
#define PART_TRACK_HORZ_LEFT		200
#define PART_TRACK_VERT_UPPER		201
#define PART_TRACK_HORZ_RIGHT		202
#define PART_TRACK_VERT_LOWER		203
#define PART_TRACK_HORZ_AREA		204
#define PART_TRACK_VERT_AREA		205
#define PART_THUMB_HORZ			210 // Also used as toolbar grip
#define PART_THUMB_VERT			211 // Also used as toolbar grip
#define PART_MENU_ITEM              250
#define PART_MENU_ITEM_CHECK_MARK   251
#define PART_MENU_ITEM_RADIO_MARK   252
#define PART_LISTVIEWHEADER_SORT_MARK	150	// used for list view headers to indicate whether they draw an ascending/descending sort indicator or whether VCL should handle the indicator

/*  #i77549#
    HACK: for scrollbars in case of thumb rect, page up and page down rect we
    abuse the HitTestNativeControl interface. All theming engines but aqua
    are actually able to draw the thumb according to our internal representation.
    However aqua draws a little outside. The canonical way would be to enhance the
    HitTestNativeControl passing a ScrollbarValue additionally so all necessary
    information is available in the call.
    .    
    However since there is only this one small exception we will deviate a little and
    instead pass the respective rect as control region to allow for a small correction.
    
    So all places using HitTestNativeControl on PART_THUMB_HORZ, PART_THUMB_VERT,
    PART_TRACK_HORZ_LEFT, PART_TRACK_HORZ_RIGHT, PART_TRACK_VERT_UPPER, PART_TRACK_VERT_LOWER
    do not use the control rectangle as region but the actuall part rectangle, making
    only small deviations feasible.
*/

/** The edit field part of a control, e.g. of the combo box.

    Currently used just for combo boxes and just for GetNativeControlRegion().
    It is valid only if GetNativeControlRegion() supports PART_BUTTON_DOWN as
    well.
*/
#define PART_SUB_EDIT           300

// For controls that require the entire background
// to be drawn first, and then other pieces over top.
// (GTK+ scrollbars for example).  Control region passed
// in to draw this part is expected to be the entire
// area of the control.
// A control may respond to one or both.
#define PART_DRAW_BACKGROUND_HORZ		1000
#define PART_DRAW_BACKGROUND_VERT		1001

// GTK+ also draws tabs right->left since there is a 
// hardcoded 2 pixel overlap between adjacent tabs
#define PART_TABS_DRAW_RTL			3000

// For themes that do not want to have the focus
// rectangle part drawn by VCL but take care of the
// whole inner control part by themselves
// eg, listboxes or comboboxes or spinbuttons
#define HAS_BACKGROUND_TEXTURE  4000

// For scrollbars that have 3 buttons (most KDE themes)
#define HAS_THREE_BUTTONS       5000

#define PART_BACKGROUND_WINDOW  6000
#define PART_BACKGROUND_DIALOG  6001

//to draw natively the border of frames
#define PART_BORDER             7000

/* Control State:
 *
 *   Specify how a particular part of the control
 *   is to be drawn.  Constants are bitwise OR-ed
 *   together to compose a final drawing state.
 *   A _disabled_ state is assumed by the drawing
 *   functions until an ENABLED or HIDDEN is passed
 *   in the ControlState.
 */

typedef sal_uInt32		ControlState;

#define CTRL_STATE_ENABLED		0x0001
#define CTRL_STATE_FOCUSED		0x0002
#define CTRL_STATE_PRESSED		0x0004
#define CTRL_STATE_ROLLOVER		0x0008
#define CTRL_STATE_HIDDEN		0x0010
#define CTRL_STATE_DEFAULT		0x0020
#define CTRL_STATE_SELECTED		0x0040
#if defined USE_JAVA && defined MACOSX
#define CTRL_STATE_INACTIVE		0x0100
#endif	// USE_JAVA && MACOSX
#define CTRL_CACHING_ALLOWED	0x8000  // set when the control is completely visible (i.e. not clipped)

/* ButtonValue:
 *
 *   Identifies the tri-state value options
 *   that buttons allow
 */

enum ButtonValue {
	BUTTONVALUE_DONTKNOW,
	BUTTONVALUE_ON,
	BUTTONVALUE_OFF,
	BUTTONVALUE_MIXED
};

#ifdef __cplusplus

/* ScrollbarValue:
 *
 *   Value container for scrollbars.
 */
class VCL_DLLPUBLIC ScrollbarValue
{
	public:
		long			mnMin;
		long			mnMax;
		long			mnCur;
		long			mnVisibleSize;
		Rectangle		maThumbRect;
		Rectangle		maButton1Rect;
		Rectangle		maButton2Rect;
		ControlState	mnButton1State;
		ControlState	mnButton2State;
		ControlState	mnThumbState;
		ControlState	mnPage1State;
		ControlState	mnPage2State;

		inline ScrollbarValue()
				{
					mnMin = 0; mnMax = 0; mnCur = 0; mnVisibleSize = 0;
					mnButton1State = 0; mnButton2State = 0;
					mnThumbState = 0; mnPage1State = 0; mnPage2State = 0;
				};
		inline ~ScrollbarValue() {};
};

/* TabitemValue:
 *
 *   Value container for tabitems.
 */

/* TABITEM constants are OR-ed together */
#define TABITEM_NOTALIGNED     0x000   // the tabitem is an inner item
#define TABITEM_LEFTALIGNED    0x001   // the tabitem is aligned with the left  border of the TabControl
#define TABITEM_RIGHTALIGNED   0x002   // the tabitem is aligned with the right border of the TabControl
#define TABITEM_FIRST_IN_GROUP 0x004   // the tabitem is the first in group of tabitems
#define TABITEM_LAST_IN_GROUP  0x008   // the tabitem is the last in group of tabitems

class VCL_DLLPUBLIC TabitemValue
{
	public:
        unsigned int    mnAlignment;

		inline TabitemValue()
				{
					mnAlignment = 0;
				};
		inline ~TabitemValue() {};

        BOOL isLeftAligned()  { return (mnAlignment & TABITEM_LEFTALIGNED) != 0; }
        BOOL isRightAligned() { return (mnAlignment & TABITEM_RIGHTALIGNED) != 0; }
        BOOL isBothAligned()  { return isLeftAligned() && isRightAligned(); }
        BOOL isNotAligned()   { return (mnAlignment & (TABITEM_LEFTALIGNED | TABITEM_RIGHTALIGNED)) == 0; }
        BOOL isFirst()        { return (mnAlignment & TABITEM_FIRST_IN_GROUP) != 0; }
        BOOL isLast()         { return (mnAlignment & TABITEM_LAST_IN_GROUP) != 0; }
};

/* SpinbuttonValue:
 *
 *   Value container for spinbuttons to paint both buttons at once.
 *   Note: the other parameters of DrawNativeControl will have no meaning
 *         all parameters for spinbuttons are carried here
 */
class VCL_DLLPUBLIC SpinbuttonValue
{
	public:
		Rectangle		maUpperRect;
		Rectangle		maLowerRect;
		ControlState	mnUpperState;
		ControlState	mnLowerState;
		int			mnUpperPart;
		int			mnLowerPart;

		inline SpinbuttonValue()
				{
					mnUpperState = mnLowerState = 0;
				};
		inline ~SpinbuttonValue() {};
};

/*	Toolbarvalue:
 *
 *  Value container for toolbars detailing the grip position
 */
class ToolbarValue
{
public:
    ToolbarValue()  { mbIsTopDockingArea = FALSE; }
    Rectangle			maGripRect;
    BOOL                mbIsTopDockingArea; // indicates that this is the top aligned dockingarea
                                            // adjacent to the menubar
};

/*	MenubarValue:
 *
 *  Value container for menubars specifying height of adjacent docking area
 */
class MenubarValue
{
public:
    MenubarValue() { maTopDockingAreaHeight=0; }
    int             maTopDockingAreaHeight;
};

/*  ProgressbarValue:
 *
 *  Value container for progressbars indicating task completion
 */
class ProgressbarValue
{
	public:
		BOOL	mbIndeterminate;	// indcates if the progress bar is indeterminate (unknown action length) or determinate (able to determine completion)
		double	mdPercentComplete;	// percentage in range [0.0, 100.0], only used for determinate progress bars
		
		ProgressbarValue()
			{
				mbIndeterminate = FALSE;
				mdPercentComplete = 0;
			};
		
		~ProgressbarValue() {};
};

#define DISCLOSUREBTN_CLOSED	0		// group for this control is currently closed, displaying the container only
#define DISCLOSUREBTN_OPEN		1		// group for this control is currently expanded, displaying all elements within the container

#define DISCLOSUREBTN_ALIGN_LEFT	0	// disclosure controls appear to the left of any items in the left margin of the column
#define DISCLOSUREBTN_ALIGN_RIGHT	1	// disclosure contorls appear to the right of any items in the right margin of the column

/*	DisclosureBtnValue:
 *
 *	Value container for disclosure buttons used for control of expanding/collapsing
 *	tree views
 */
class DisclosureBtnValue
{
	public:
		USHORT mnOpenCloseState;		// indicates whether the controls associated container is open or closed, DISCLOSURE_OPEN/CLOSED
		USHORT mnAlignment;				// indicates whether the disclosure control appears to the left or right of containers
		BOOL mbHasChildren;				// true if the node has any children, false if not
		
		DisclosureBtnValue()
			{
				mnOpenCloseState = DISCLOSUREBTN_CLOSED;
				mnAlignment = DISCLOSUREBTN_ALIGN_LEFT;
				mbHasChildren = 1;
			};
		
		~DisclosureBtnValue() {};
};

#define LISTVIEWHEADER_SORT_DONTKNOW	0	// indicates the current sort of the column is unknown
#define LISTVIEWHEADER_SORT_DESCENDING	1	// indicates the column is sorted in descending order
#define LISTVIEWHEADER_SORT_ASCENDING	2	// indicates the column is sorted in ascending order
#define LISTVIEWHEADER_SORT_UNSORTED	3	// indicates the column is not sorted

/*	ListViewHeaderValue
 *
 *	Value container for list view header cells drawn above columns of scrollable
 *	lists.  Indicates whether the clumn is the primary sort column and
 *	any direction of the sort
 */
class ListViewHeaderValue
{
	public:
		BOOL mbPrimarySortColumn;		// true if the column is the primary active column on which data is sorted, false if it is a secondary informational column
		USHORT mnSortDirection;			// sort direction for the column.
		
		ListViewHeaderValue()
			{
				mbPrimarySortColumn = FALSE;
				mnSortDirection = LISTVIEWHEADER_SORT_DONTKNOW;
			};
		
		~ListViewHeaderValue() {};
};

/*	PushButtonValue:
 *
 *  Value container for pushbuttons specifying additional drawing hints
 */
class PushButtonValue
{
public:
PushButtonValue() : mbBevelButton( false ), mbSingleLine( true ) {}
    bool            mbBevelButton:1;
    bool            mbSingleLine:1;
};

/* ImplControlValue:
 *
 *   Generic value container for all control parts.
 */

class ImplControlValue
{
	friend class SalFrame;

	private:
		ButtonValue	mTristate;	// Tristate value: on, off, mixed
		rtl::OUString	mString;		// string value
		long			mNumber;		// numeric value
		void *		mOptionalVal;	// optional control-specific value

	public:
		inline ImplControlValue( ButtonValue nTristate, rtl::OUString sString, long nNumeric, void * aOptVal ) \
								{ mTristate = nTristate; mString = sString; mNumber = nNumeric; mOptionalVal = aOptVal; };
		inline ImplControlValue( ButtonValue nTristate, rtl::OUString sString, long nNumeric ) \
								{ mTristate = nTristate; mString = sString; mNumber = nNumeric; mOptionalVal = NULL; };
		explicit ImplControlValue( ButtonValue nTristate )
			: mTristate(nTristate), mNumber(0), mOptionalVal(NULL) {}
		explicit ImplControlValue( rtl::OUString& rString )
			: mTristate(BUTTONVALUE_DONTKNOW), mString(rString), mNumber(0), mOptionalVal(NULL) {}
		explicit ImplControlValue( long nNumeric )
			: mTristate(BUTTONVALUE_DONTKNOW), mNumber( nNumeric), mOptionalVal(NULL) {}
		explicit ImplControlValue( void* aOptVal )
	 		: mTristate(BUTTONVALUE_DONTKNOW), mNumber(0), mOptionalVal(aOptVal) {}
		inline ImplControlValue()
			: mTristate(BUTTONVALUE_DONTKNOW), mNumber(0), mOptionalVal(NULL) {}

		inline ~ImplControlValue() { mOptionalVal = NULL; };

		inline ButtonValue		getTristateVal( void ) const { return mTristate; }
		inline void			setTristateVal( ButtonValue nTristate ) { mTristate = nTristate; }

		inline const rtl::OUString&	getStringVal( void ) const { return mString; }
		inline void			setStringVal( rtl::OUString sString ) { mString = sString; }

		inline long			getNumericVal( void ) const { return mNumber; }
		inline void			setNumericVal( long nNumeric ) { mNumber = nNumeric; }

		inline void *			getOptionalVal( void ) const { return mOptionalVal; }
		inline void			setOptionalVal( void * aOptVal ) { mOptionalVal = aOptVal; }
};

#endif	/* __cplusplus */

#endif

