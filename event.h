#ifndef GAMER_EVENT_H
#define GAMER_EVENT_H

#include <ruby.h>
#include <SDL/SDL.h>

#include "surface.h"

typedef struct event_s
{
	Uint8		initialized;
	SDL_Event	event;
} event_t;

enum event_types {
	KEYDOWN, KEYUP, MOUSEDOWN, MOUSEUP, MOUSEMOVE, QUIT, ACTIVE, EXPOSE, RESIZE
};

void Init_event(VALUE module);

VALUE event_allocate(VALUE klass);
void event_free(event_t *evt);
void event_mark(event_t *evt);

VALUE event_get_type(VALUE object);

VALUE event_get_mouse_x(VALUE object);
VALUE event_get_mouse_y(VALUE object);
VALUE event_get_mouse_button(VALUE object, VALUE btn);

VALUE event_get_keycode(VALUE object);
VALUE event_get_unicode(VALUE object);

VALUE wrap_event(SDL_Event sdl_ev, surface_t *surface);

#endif
