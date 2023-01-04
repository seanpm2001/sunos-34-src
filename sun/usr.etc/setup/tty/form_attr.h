/*	@(#)form_attr.h 1.1 86/09/25 SMI	*/

/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#include <sunwindow/attr.h>

#define FORM_ATTR(type, ordinal)     ATTR(ATTR_PKG_UNUSED_FIRST, type, ordinal)

typedef enum {
    FORM_INPUT_OUTPUT_ITEM	= FORM_ATTR(ATTR_OPAQUE, 1), 
    FORM_OUTPUT_ONLY_ITEM	= FORM_ATTR(ATTR_OPAQUE, 2), 
    FORM_CURRENT_IO_ITEM	= FORM_ATTR(ATTR_OPAQUE, 3),
    FORM_CURSES_WINDOW		= FORM_ATTR(ATTR_OPAQUE, 4),
    FORM_FIRST_ROW		= FORM_ATTR(ATTR_INT, 5),
    FORM_LAST_ROW		= FORM_ATTR(ATTR_INT, 6),
    FORM_REFRESH		= FORM_ATTR(ATTR_NO_VALUE, 7),
    FORM_DISPLAY		= FORM_ATTR(ATTR_INT, 8),
    FORM_WIN_SIZE		= FORM_ATTR(ATTR_INT, 9),
    FORM_DISPLAY_FUNC		= FORM_ATTR(ATTR_OPAQUE, 10),
    FORM_DISPLAY_HACK		= FORM_ATTR(ATTR_NO_VALUE, 11),
    
    
    ITEM_ROW 			= FORM_ATTR(ATTR_INT, 20), 
    ITEM_COL 			= FORM_ATTR(ATTR_INT, 21), 
    ITEM_HILIGHTED		= FORM_ATTR(ATTR_INT, 22), 
    ITEM_DISPLAYED		= FORM_ATTR(ATTR_INT, 23), 
    ITEM_LABEL			= FORM_ATTR(ATTR_STRING, 24), 
    ITEM_CLIENT_DATA		= FORM_ATTR(ATTR_OPAQUE, 25), 
    ITEM_FORM			= FORM_ATTR(ATTR_OPAQUE, 26), 
    ITEM_NOTIFY_PROC		= FORM_ATTR(ATTR_FUNCTION_PTR, 27), 
    ITEM_NOTIFY_LEVEL		= FORM_ATTR(ATTR_INT, 28),
    ITEM_LABEL_LENGTH		= FORM_ATTR(ATTR_NO_VALUE, 29),
    ITEM_SET_CURSOR		= FORM_ATTR(ATTR_NO_VALUE, 30),
    ITEM_VALUE			= FORM_ATTR(ATTR_INT, 31),
    ITEM_VALUE_COL		= FORM_ATTR(ATTR_INT, 32),
    ITEM_TYPE			= FORM_ATTR(ATTR_FUNCTION_PTR, 33),
    ITEM_SUB_ITEM		= FORM_ATTR(ATTR_INT, 34),
    
    ITEM_TEXT_FIELD_SIZE	= FORM_ATTR(ATTR_INT, 41), 		
    
    ITEM_CHOICE_STRING		= FORM_ATTR(ATTR_STRING, 50), 		
    ITEM_CHOICE_CYCLE_DISPLAY	= FORM_ATTR(ATTR_INT, 51), 		
    ITEM_CHOICE_RESET		= FORM_ATTR(ATTR_NO_VALUE, 52),
    
    ITEM_TOGGLE_STRING		= FORM_ATTR(ATTR_STRING, 60), 		
    ITEM_TOGGLE_CYCLE_DISPLAY	= FORM_ATTR(ATTR_INT, 61), 		
    ITEM_TOGGLE_RESET		= FORM_ATTR(ATTR_NO_VALUE, 62),
    
    FORM_LAST_ATTR
} Form_attribute;


typedef void	(*Void_func)();

typedef char	**Form_avlist;

typedef	enum {
    CURSOR_CONTROL_NEXT_ITEM, 
    CURSOR_CONTROL_STAY_PUT, 
    CURSOR_CONTROL_IGNORE 
} Cursor_control;
	
