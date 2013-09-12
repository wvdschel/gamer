#include "surface.h"
#include "graphics.h"
#include "event.h"

VALUE gam_cSurface;

Uint8 some_surface_visible;

void Init_surface(VALUE module)
{
	some_surface_visible = 0;
	
	/* We need Thread and Mutex for event handling and synchronizing drawing operations */
	rb_require("thread");
	
	gam_cSurface = rb_define_class_under(module, "Surface", rb_cObject);
	rb_define_alloc_func(gam_cSurface, surface_allocate);
	
	rb_define_method(gam_cSurface, "initialize", surface_initialize, 3);
	rb_define_method(gam_cSurface, "set_size", surface_set_size, 2);
	rb_define_method(gam_cSurface, "width", surface_get_width, 0);
	rb_define_method(gam_cSurface, "width=", surface_set_width, 1);
	rb_define_method(gam_cSurface, "height", surface_get_height, 0);
	rb_define_method(gam_cSurface, "height=", surface_set_height, 1);
	rb_define_method(gam_cSurface, "depth", surface_get_depth, 0);
	rb_define_method(gam_cSurface, "depth=", surface_set_depth, 1);
	rb_define_method(gam_cSurface, "windowed?", surface_is_windowed, 0);
	rb_define_method(gam_cSurface, "fullscreen=", surface_set_fullscreen, 1);
	rb_define_method(gam_cSurface, "fullscreen?", surface_is_fullscreen, 0);
	rb_define_method(gam_cSurface, "show", surface_show, 0);
	rb_define_method(gam_cSurface, "visible?", surface_is_shown, 0);
	rb_define_method(gam_cSurface, "close", surface_close, 0);
	rb_define_method(gam_cSurface, "show_cursor=", surface_show_cursor, 1);
	rb_define_method(gam_cSurface, "show_cursor?", surface_is_cursor_shown, 0);
	rb_define_method(gam_cSurface, "draw", surface_draw, 0);
	rb_define_method(gam_cSurface, "listener=", surface_set_eventlistener, 1);
	rb_define_method(gam_cSurface, "listener", surface_get_eventlistener, 0);
	rb_define_method(gam_cSurface, "title=", surface_set_title, 1);
	rb_define_method(gam_cSurface, "title", surface_get_title, 0);
	rb_define_method(gam_cSurface, "poll_for_event", surface_poll_for_event, 0);
	rb_define_method(gam_cSurface, "keyboard_focus?", surface_has_keyboardfocus, 0);
	rb_define_method(gam_cSurface, "mouse_focus?", surface_has_mousefocus, 0);
	rb_define_method(gam_cSurface, "iconified?", surface_is_iconified, 0);
	rb_define_method(gam_cSurface, "mouse_x", surface_mouse_x, 0);
	rb_define_method(gam_cSurface, "mouse_y", surface_mouse_y, 0);
	rb_define_method(gam_cSurface, "mouse_button?", surface_mouse_button, 1);
}


VALUE surface_allocate(VALUE klass)
{
	VALUE mtxClass;
	surface_t *s = malloc(sizeof(surface_t));
	s->surf 	= NULL;
	s->title	= NULL;
	s->listener = Qnil;
	s->width 	= 320;
	s->height	= 240;
	s->depth	= 0;
	s->fullscreen = 0;
	s->resizable  = 0;
	
	mtxClass 	= rb_const_get(rb_cObject, rb_intern("Mutex"));
	s->mutex	= rb_class_new_instance(0, NULL, mtxClass);
	
	return Data_Wrap_Struct(klass, surface_mark, surface_free, s);
}

void surface_mark(surface_t *s)
{
	rb_gc_mark(s->listener);
}

void surface_free(surface_t *s)
{
	/* Destroy the window */
	if (s->surf != NULL)
	{
		surface_close_internal(s);
		some_surface_visible = 0;
		SDL_FreeSurface(s->surf);
	}
	free(s->title);
	free(s);
}

VALUE surface_initialize(VALUE object, VALUE title, VALUE width, VALUE height)
{
	surface_set_title(object, title);
	surface_set_size(object, width, height);
	return object;
}

Sint8 surface_set_mode(surface_t *s)
{
	Uint32 flags = SDL_DOUBLEBUF;
	if(s->fullscreen)
		flags |= SDL_FULLSCREEN;
	else if(s->resizable)
		flags |= SDL_RESIZABLE;
	
	s->surf = SDL_SetVideoMode(s->width, s->height, s->depth, flags | SDL_HWSURFACE);
	if(s->surf == NULL)
	{
		s->surf = SDL_SetVideoMode(s->width, s->height, 0, flags | SDL_SWSURFACE);
		if(s->surf == NULL)
			return 1;
	}
	return 0;
}

VALUE surface_set_size(VALUE object, VALUE width, VALUE height)
{
	surface_t *s;
	Uint32 new_width, old_width, new_height, old_height;
	
	Data_Get_Struct(object, surface_t, s);
	
	new_width = NUM2UINT(width);
	old_width = s->width;
	s->width  = new_width;
	
	new_height	= NUM2UINT(height);
	old_height	= s->height;
	s->height	= new_height;
	
	// Apply the changes when the window is visible
	if(s->surf != NULL)
	{
		/* Undo the changes if the video mode fails to change the resolution */
		if(surface_set_mode(s))
		{
			s->width	= old_width;
			s->height	= old_height;
			if(surface_set_mode(s))
			{
				/* Uh-oh, can't revert to the old mode, either */
				rb_raise(rb_eArgError, "Could not change the resolution to the"
				" requested size, and couldn't revert to the previous settings either");
			} else {
				/* Can't change the video mode, raise an error */
				rb_raise(rb_eArgError, "Could not change the resolution to the requested size");
			}
		}
	}
	
	return Qnil;
}

VALUE surface_set_width(VALUE object, VALUE width)
{
	surface_t *s;	
	Data_Get_Struct(object, surface_t, s);
	
	surface_set_size(object, width, UINT2NUM(s->height));
	
	return UINT2NUM(s->width);
}

VALUE surface_get_width(VALUE object)
{
	surface_t *s;
	Data_Get_Struct(object, surface_t, s);
	return UINT2NUM(s->width);
}

VALUE surface_set_height(VALUE object, VALUE height)
{
	surface_t *s;
	Data_Get_Struct(object, surface_t, s);
	
	surface_set_size(object, UINT2NUM(s->width), height);
	
	return UINT2NUM(s->height);
}

VALUE surface_get_height(VALUE object)
{
	surface_t *s;
	Data_Get_Struct(object, surface_t, s);
	return UINT2NUM(s->height);
}

VALUE surface_set_depth(VALUE object, VALUE depth)
{
	surface_t *s;
	Uint32 new_depth, old_depth;
	
	Data_Get_Struct(object, surface_t, s);
	
	new_depth	= NUM2UINT(depth);
	old_depth	= s->depth;
	s->depth	= new_depth;
	
	/* Apply the changes when the window is visible */
	if(s->surf != NULL)
	{
		/* Undo the changes if the video mode fails to change the resolution */
		if(surface_set_mode(s))
		{
			s->depth = old_depth;
			if(surface_set_mode(s))
			{
				/* Uh-oh, can't revert to the old mode, either */
				rb_raise(rb_eArgError, "Could not change the video mode to the requested"
				"depth, and couldn't revert to the previous settings either");
			} else {
				/* Can't change the video mode, raise an error */
				rb_raise(rb_eArgError, "Could not change the resolution to the requested size");
			}
		}
	}
	return UINT2NUM(s->width);
}

VALUE surface_get_depth(VALUE object)
{
	surface_t *s;
	Data_Get_Struct(object, surface_t, s);
	return UINT2NUM(s->depth);
}

VALUE surface_set_fullscreen(VALUE object, VALUE new_val)
{
	surface_t *s;
	Data_Get_Struct(object, surface_t, s);
	
	if(new_val != Qtrue && new_val != Qfalse)
		rb_raise(rb_eArgError, "Boolean value expected");
	
	if(s->fullscreen == (new_val == Qtrue ? 0 : 1))
	{
		s->fullscreen = (new_val == Qtrue ? 1 : 0);
		
		if(s->surf != NULL)
		{
			/* Undo the changes if the video mode fails to change the resolution */
			if(surface_set_mode(s))
			{
				s->fullscreen = (new_val == Qtrue ? 0 : 1);
				if(surface_set_mode(s))
					/* Uh-oh, can't revert to the old mode, either */
					rb_raise(rb_eArgError, "Could not toggle fullscreen mode on or off, and couldn't switch back either");
				else
					/* Can't change the video mode, raise an error */
					rb_raise(rb_eArgError, "Could not toggle fullscreen mode on or off");
			}
		}
	}
	
	return Qnil;
}

VALUE surface_is_fullscreen(VALUE object)
{
	surface_t *s;
	Data_Get_Struct(object, surface_t, s);
	
	if(s->fullscreen)
		return Qtrue;
	else
		return Qfalse;
}

VALUE surface_is_windowed(VALUE object)
{
	surface_t *s;
	Data_Get_Struct(object, surface_t, s);
	
	if(s->fullscreen)
		return Qfalse;
	else
		return Qtrue;
}

VALUE surface_has_mousefocus(VALUE object)
{
	surface_t *s;
	Data_Get_Struct(object, surface_t, s);
	
	if(s->surf == NULL)
		return Qfalse;
	
	if(SDL_GetAppState() & SDL_APPMOUSEFOCUS)
		return Qfalse;
	else
		return Qtrue;
}

VALUE surface_has_keyboardfocus(VALUE object)
{
	surface_t *s;
	Data_Get_Struct(object, surface_t, s);
	
	if(s->surf == NULL)
		return Qfalse;
	
	if(SDL_GetAppState() & SDL_APPINPUTFOCUS)
		return Qtrue;
	else
		return Qfalse;
}

VALUE surface_is_iconified(VALUE object)
{
	surface_t *s;
	Data_Get_Struct(object, surface_t, s);
	
	if(s->surf == NULL)
		return Qfalse;
	
	if(SDL_GetAppState() & SDL_APPACTIVE)
		return Qtrue;
	else
		return Qfalse;
}

VALUE surface_mouse_x(VALUE object)
{
	surface_t *s;
	int x;
	Data_Get_Struct(object, surface_t, s);
	
	if(s->surf == NULL)
		return INT2NUM(0);
	
	SDL_GetMouseState(&x, NULL);
	
	return INT2NUM(x);
}

VALUE surface_mouse_y(VALUE object)
{
	surface_t *s;
	int y;
	Data_Get_Struct(object, surface_t, s);
	
	if(s->surf == NULL)
		return INT2NUM(0);
	
	SDL_GetMouseState(&y, NULL);
	
	return INT2NUM(y);
}

VALUE surface_mouse_button(VALUE object, VALUE btn)
{
	surface_t *s;
	Data_Get_Struct(object, surface_t, s);
	
	if(s->surf == NULL)
		return Qfalse;
	
	if(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(NUM2UINT(btn)))
		return Qtrue;
	else
		return Qfalse;
}

VALUE surface_show(VALUE object)
{
	surface_t *s;
	Data_Get_Struct(object, surface_t, s);
	
	if(some_surface_visible && s->surf == NULL)
		rb_raise(rb_eRuntimeError, "You can't display several surfaces simultaniously");
	else
	{	
		some_surface_visible = 1;
		if(s->title != NULL)
			SDL_WM_SetCaption(s->title, "SDL");
		
		if(surface_set_mode(s))
		{
			some_surface_visible = 0;
			rb_raise(rb_eRuntimeError, "Could not set video mode");
		}
		else
		{
			rb_funcall(object, rb_intern("start_main_loop"), 0);
			return Qnil;
		}
	}
}

VALUE surface_is_shown(VALUE object)
{
	surface_t *s;
	Data_Get_Struct(object, surface_t, s);
	
	if(s->surf != NULL)
		return Qtrue;
	else
		return Qfalse;
}

VALUE surface_close(VALUE object)
{
	surface_t *s;
	Data_Get_Struct(object, surface_t, s);
	
	if(s->surf != NULL)
	{
		surface_close_internal(s);
		some_surface_visible = 0;
	}
	
	return Qnil;
}

void surface_close_internal(surface_t* surface)
{
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	SDL_InitSubSystem(SDL_INIT_VIDEO);
	surface->surf = NULL;
}

VALUE surface_show_cursor(VALUE object, VALUE new_val)
{
	surface_t *s;
	int res;
	
	Data_Get_Struct(object, surface_t, s);
	
	if(new_val != Qtrue && new_val != Qfalse)
		rb_raise(rb_eArgError, "Boolean value expected");
	
	if(new_val == Qtrue)
		res = SDL_ShowCursor(SDL_ENABLE);
	else
		res = SDL_ShowCursor(SDL_DISABLE);
		
	if(res == SDL_ENABLE)
		return Qtrue;
	else
		return Qfalse;
}

VALUE surface_is_cursor_shown(VALUE object)
{
	surface_t *s;
	Data_Get_Struct(object, surface_t, s);

	if(SDL_ShowCursor(SDL_QUERY) == SDL_ENABLE)
		return Qtrue;
	else
		return Qfalse;
}

VALUE surface_draw(VALUE object)
{
	surface_t *s;
	Data_Get_Struct(object, surface_t, s);
	if(s->surf != NULL)
	{
		VALUE graph, graphClass;
		graphClass 	= rb_const_get(rb_const_get(rb_cModule, rb_intern("Gamer")), rb_intern("Graphics"));
		graph		= rb_class_new_instance(1, &object, graphClass);
		
		graphics_lock(graph);
		rb_yield(graph);
		/* needs to be ensured */
		graphics_unlock(graph);
	}
	
	return Qnil;
}

VALUE surface_set_title(VALUE object, VALUE title)
{
	surface_t *s;
	VALUE title_string;
	Data_Get_Struct(object, surface_t, s);
	
	title_string = rb_funcall(title, rb_intern("to_s"), 0);
	s->title = StringValuePtr(title_string);
	
	SDL_WM_SetCaption(s->title, "SDL");
	
	return Qnil;
}

VALUE surface_get_title(VALUE object)
{
	surface_t *s;
	Data_Get_Struct(object, surface_t, s);
	
	if(s->title != NULL)
		return rb_str_new2(s->title);
	else
		return Qnil;
}

VALUE surface_set_eventlistener(VALUE object, VALUE listener)
{
	surface_t *s;
	Data_Get_Struct(object, surface_t, s);
	
	s->listener = listener;
	return Qnil;
}

VALUE surface_get_eventlistener(VALUE object)
{
	surface_t *s;
	Data_Get_Struct(object, surface_t, s);
	return s->listener;
}

VALUE surface_poll_for_event(VALUE object)
{
	SDL_Event	sdl_event;
	surface_t *s;
	Data_Get_Struct(object, surface_t, s);
	
	if(SDL_PollEvent(&sdl_event))
	{
		VALUE event;
		event = wrap_event(sdl_event, s);
		return event;
	}
	else
		return Qnil;
}
