#include <ruby.h>
#include <SDL/SDL.h>

#include "event.h"

VALUE gam_cEvent;

void Init_event(VALUE module)
{
	gam_cEvent = rb_define_class_under(module, "Event", rb_cObject);
	rb_define_alloc_func(gam_cEvent, event_allocate);
	
	rb_define_method(gam_cEvent, "type", event_get_type, 0);
	rb_define_method(gam_cEvent, "x", event_get_mouse_x, 0);
	rb_define_method(gam_cEvent, "y", event_get_mouse_x, 0);
	rb_define_method(gam_cEvent, "button", event_get_mouse_button, 1);
	rb_define_method(gam_cEvent, "keycode", event_get_keycode, 0);
	rb_define_method(gam_cEvent, "unicode", event_get_unicode, 0);
	
	rb_define_const(module, "KEYDOWN", INT2FIX(KEYDOWN));
	rb_define_const(module, "KEYUP", INT2FIX(KEYUP));
	rb_define_const(module, "MOUSEDOWN", INT2FIX(MOUSEDOWN));
	rb_define_const(module, "MOUSEUP", INT2FIX(MOUSEUP));
	rb_define_const(module, "MOUSEMOVE", INT2FIX(MOUSEMOVE));
	rb_define_const(module, "QUIT", INT2FIX(QUIT));
	rb_define_const(module, "EXPOSE", INT2FIX(EXPOSE));
	rb_define_const(module, "ACTIVE", INT2FIX(ACTIVE));
	rb_define_const(module, "RESIZE", INT2FIX(RESIZE));
}

VALUE event_allocate(VALUE klass)
{
	event_t *evt 		= malloc(sizeof(event_t));
	evt->initialized 	= 0;
	return Data_Wrap_Struct(klass, event_mark, event_free, evt);
}

void event_free(event_t *evt)
{
	free(evt);
}

void event_mark(event_t *evt)
{}

VALUE event_get_type(VALUE object)
{
	event_t *evt;
	
	Data_Get_Struct(object, event_t, evt);
	
	if(evt->initialized)
	{
		switch(evt->event.type)
		{
			case SDL_KEYDOWN:
				return INT2FIX(KEYDOWN);
			case SDL_KEYUP:
				return INT2FIX(KEYUP);
			case SDL_ACTIVEEVENT:
				return INT2FIX(ACTIVE);
			case SDL_MOUSEMOTION:
				return INT2FIX(MOUSEMOVE);
			case SDL_MOUSEBUTTONUP:
				return INT2FIX(MOUSEUP);
			case SDL_MOUSEBUTTONDOWN:
				return INT2FIX(MOUSEDOWN);
			case SDL_VIDEORESIZE:
				return INT2FIX(RESIZE);
			case SDL_VIDEOEXPOSE:
				return INT2FIX(EXPOSE);
			case SDL_QUIT:
				return INT2FIX(QUIT);
			default:
				return Qfalse;
		}
	} else
		return Qnil;
}

VALUE event_get_mouse_x(VALUE object)
{
	event_t *evt;
	
	Data_Get_Struct(object, event_t, evt);
	
	if(evt->event.type == SDL_MOUSEMOTION)
	{
		return UINT2NUM(evt->event.motion.x);
	}
	else if(evt->event.type == SDL_MOUSEBUTTONUP || evt->event.type == SDL_MOUSEBUTTONDOWN)
	{
		return UINT2NUM(evt->event.button.x);
	}
	else
		rb_raise(rb_eTypeError, "only MOUSEMOTION, MOUSEUP and MOUSEDOWN events can report mouse coordinates. Use Surface#mouse_x instead");
}

VALUE event_get_mouse_y(VALUE object)
{
	event_t *evt;
	
	Data_Get_Struct(object, event_t, evt);
	
	if(evt->event.type == SDL_MOUSEMOTION)
		return UINT2NUM(evt->event.motion.y);
	else if(evt->event.type == SDL_MOUSEBUTTONUP || evt->event.type == SDL_MOUSEBUTTONDOWN)
		return UINT2NUM(evt->event.button.y);
	else
		rb_raise(rb_eTypeError, "only MOUSEMOTION, MOUSEUP and MOUSEDOWN events can report mouse coordinates. Use Surface#mouse_y instead");
}

VALUE event_get_mouse_button(VALUE object, VALUE btn)
{
	Uint8 state;
	event_t *evt;
	
	Data_Get_Struct(object, event_t, evt);
	
	if(evt->event.type == SDL_MOUSEMOTION)
		state = evt->event.motion.state;
	if(evt->event.type == SDL_MOUSEBUTTONUP || evt->event.type == SDL_MOUSEBUTTONDOWN)
		state = evt->event.button.state;
	else
		rb_raise(rb_eTypeError, "only MOUSEMOTION, MOUSEUP and MOUSEDOWN events can report mouse button states. Use Surface#mouse_button? instead");
	
	if(state & SDL_BUTTON(NUM2UINT(btn)) )
		return Qtrue;
	else
		return Qfalse;
}

VALUE event_get_keycode(VALUE object)
{
	event_t *evt;
	Data_Get_Struct(object, event_t, evt);
	
	if(evt->event.type == SDL_KEYUP || evt->event.type == SDL_KEYDOWN)
		return INT2FIX(evt->event.key.keysym.sym);
	else
		rb_raise(rb_eTypeError, "only KEYUP and KEYDOWN events can report keycodes");
}

VALUE event_get_unicode(VALUE object)
{
	event_t *evt;
	Data_Get_Struct(object, event_t, evt);
	
	if(evt->event.type == SDL_KEYUP || evt->event.type == SDL_KEYDOWN)
		return INT2FIX(evt->event.key.keysym.unicode);
	else
		rb_raise(rb_eTypeError, "only KEYUP and KEYDOWN events can report unicodes");
}

VALUE wrap_event(SDL_Event sdl_event, surface_t *surface)
{
	event_t *evt;
	VALUE event;
	
	if(sdl_event.type == SDL_VIDEORESIZE)
	{
		surface->width = sdl_event.resize.w;
		surface->height = sdl_event.resize.h;
	}
	
	event = rb_class_new_instance(0, NULL, gam_cEvent);
	
	Data_Get_Struct(event, event_t, evt);
	evt->event = sdl_event;
	evt->initialized = 1;
	
	return event;
}
