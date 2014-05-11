/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

/**
 * @file    src/gwin/frame.c
 * @brief   GWIN sub-system frame code.
 *
 * @defgroup Frame Frame
 * @ingroup GWIN
 *
 * @{
 */

#include "gfx.h"

#if GFX_USE_GWIN && GWIN_NEED_FRAME

/* Some values for the default render */
#define BORDER_X		5
#define BORDER_Y		30
#define BUTTON_X		20
#define BUTTON_Y		20

/* Some useful macros for data type conversions */
#define gh2obj			((GFrameObject *)gh)

/* Forware declarations */
void gwinFrameDraw_Std(GWidgetObject *gw, void *param);
static void _callbackBtn(void *param, GEvent *pe);

static void _frameDestroy(GHandle gh) {
	/* Deregister the button callback */
	geventRegisterCallback(&gh2obj->gl, NULL, NULL);
	geventDetachSource(&gh2obj->gl, NULL);

	/* call the gcontainer standard destroy routine */
	_gcontainerDestroy(gh);
}

#if 0 && GINPUT_NEED_MOUSE
	static void _mouseDown(GWidgetObject *gw, coord_t x, coord_t y) {
	
	}

	static void _mouseUp(GWidgetObject *gw, coord_t x, coord_t y) {

	}

	static void _mouseMove(GWidgetObject *gw, coord_t x, coord_t y) {
	
	}
#endif

static const gcontainerVMT frameVMT = {
	{
		{
			"Frame",					// The classname
			sizeof(GFrameObject),		// The object size
			_frameDestroy,				// The destroy routie
			_gcontainerRedraw,			// The redraw routine
			0,							// The after-clear routine
		},
		gwinFrameDraw_Std,				// The default drawing routine
		#if GINPUT_NEED_MOUSE
			{
				0,//_mouseDown,				// Process mouse down event
				0,//_mouseUp,				// Process mouse up events
				0,//_mouseMove,				// Process mouse move events
			},
		#endif
		#if GINPUT_NEED_TOGGLE
			{
				0,						// 1 toggle role
				0,						// Assign Toggles
				0,						// Get Toggles
				0,						// Process toggle off events
				0,						// Process toggle on events
			},
		#endif
		#if GINPUT_NEED_DIAL
			{
				0,						// 1 dial roles
				0,						// Assign Dials
				0,						// Get Dials
				0,						// Process dial move events
			},
		#endif
	},
	0,									// Adjust the relative position of a child (optional)
	0,									// Adjust the size of a child (optional)
	0,									// A child has been added (optional)
	0,									// A child has been deleted (optional)
};

GHandle gwinGFrameCreate(GDisplay *g, GFrameObject *fo, GWidgetInit *pInit, uint32_t flags) {
	if (!(fo = (GFrameObject *)_gcontainerCreate(g, &fo->gc, pInit, &frameVMT)))
		return 0;

	fo->btnClose = 0;
	fo->btnMin = 0;
	fo->btnMax = 0;

	/* Buttons require a border */
	if ((flags & (GWIN_FRAME_CLOSE_BTN|GWIN_FRAME_MINMAX_BTN)))
		flags |= GWIN_FRAME_BORDER;

	/* apply flags */
	fo->gc.g.flags |= flags;

	/* create and initialize the listener if any button is present. */
	if ((flags & (GWIN_FRAME_CLOSE_BTN|GWIN_FRAME_MINMAX_BTN))) {
		geventListenerInit(&fo->gl);
		gwinAttachListener(&fo->gl);
		geventRegisterCallback(&fo->gl, _callbackBtn, (GHandle)fo);
	}

	/* create close button if necessary */
	if ((flags & GWIN_FRAME_CLOSE_BTN)) {
		GWidgetInit wi;

		gwinWidgetClearInit(&wi);
		wi.g.show = TRUE;
		wi.g.parent = &fo->gc.g;

		wi.g.x = fo->gc.g.width - BORDER_X - BUTTON_X;
		wi.g.y = (BORDER_Y - BUTTON_Y) / 2;
		wi.g.width = BUTTON_X;
		wi.g.height = BUTTON_Y;
		wi.text = "X";
		fo->btnClose = gwinGButtonCreate(g, 0, &wi);
	}

	/* create minimize and maximize buttons if necessary */
	if ((flags & GWIN_FRAME_MINMAX_BTN)) {
		GWidgetInit wi;

		gwinWidgetClearInit(&wi);
		wi.g.show = TRUE;
		wi.g.parent = &fo->gc.g;

		wi.g.x = (flags & GWIN_FRAME_CLOSE_BTN) ? fo->gc.g.width - 2*BORDER_X - 2*BUTTON_X : fo->gc.g.width - BORDER_X - BUTTON_X;
		wi.g.y = (BORDER_Y - BUTTON_Y) / 2;
		wi.g.width = BUTTON_X;
		wi.g.height = BUTTON_Y;
		wi.text = "O";
		fo->btnMin = gwinGButtonCreate(g, 0, &wi);

		wi.g.x = (flags & GWIN_FRAME_CLOSE_BTN) ? fo->gc.g.width - 3*BORDER_X - 3*BUTTON_X : fo->gc.g.width - BORDER_X - BUTTON_X;
		wi.g.y = (BORDER_Y - BUTTON_Y) / 2;
		wi.g.width = BUTTON_X;
		wi.g.height = BUTTON_Y;
		wi.text = "_";
		fo->btnMax = gwinGButtonCreate(g, 0, &wi);
	}

	gwinSetVisible(&fo->gc.g, pInit->g.show);

	return &fo->gc.g;
}

/* Process a button event */
static void _callbackBtn(void *param, GEvent *pe) {
	switch (pe->type) {
		case GEVENT_GWIN_BUTTON:
			if (((GEventGWinButton *)pe)->button == ((GFrameObject*)(GHandle)param)->btnClose)
				gwinDestroy((GHandle)param);

			else if (((GEventGWinButton *)pe)->button == ((GFrameObject*)(GHandle)param)->btnMin) {
				;/* ToDo */

			} else if (((GEventGWinButton *)pe)->button == ((GFrameObject*)(GHandle)param)->btnMax) {
				;/* ToDo */
			}

			break;

		default:
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Default render routines                                                                       //
///////////////////////////////////////////////////////////////////////////////////////////////////

static const GColorSet* _getDrawColors(GWidgetObject *gw) {
	if (!(gw->g.flags & GWIN_FLG_SYSENABLED))
		return &gw->pstyle->disabled;
	//if ((gw->g.flags & GBUTTON_FLG_PRESSED))
	//	return &gw->pstyle->pressed;

	return &gw->pstyle->enabled;
}

void gwinFrameDraw_Std(GWidgetObject *gw, void *param) {
	const GColorSet		*pcol;
	color_t			border;
	color_t			background;
	(void)param;

	if (gw->g.vmt != (gwinVMT *)&frameVMT)
		return;

	pcol = _getDrawColors(gw);

	// do some magic to make the background lighter than the widgets. Fix this somewhen.
	border = HTML2COLOR(0x2698DE);
	background = HTML2COLOR(0xEEEEEE);

	// Render the actual frame (with border, if any)
	if (gw->g.flags & GWIN_FRAME_BORDER) {
		gdispGFillArea(gw->g.display, gw->g.x, gw->g.y, gw->g.width, gw->g.height, border);
		gdispGFillArea(gw->g.display, gw->g.x + BORDER_X, gw->g.y + BORDER_Y, gw->g.width - 2*BORDER_X, gw->g.height - BORDER_Y - BORDER_X, background);
	} else {
		// This ensure that the actual frame content (it's children) render at the same spot, no mather whether the frame has a border or not
		gdispGFillArea(gw->g.display, gw->g.x + BORDER_X, gw->g.y + BORDER_Y, gw->g.width, gw->g.height, background);
	}

	// Render frame title - if any
	if (gw->text != NULL) {
		coord_t text_y;

		text_y = ((BORDER_Y - gdispGetFontMetric(gw->g.font, fontHeight))/2);

		gdispGDrawString(gw->g.display, gw->g.x + BORDER_X, gw->g.y + text_y, gw->text, gw->g.font, pcol->text);
	}
}

#endif  /* (GFX_USE_GWIN && GWIN_NEED_FRAME) || defined(__DOXYGEN__) */
/** @} */
