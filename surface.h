#ifndef GAMER_SURFACE_H
#define GAMER_SURFACE_H

#include <stdio.h>
#include <ruby.h>
#include <SDL/SDL.h>

typedef struct surface_s
{
	SDL_Surface		*surf;
	Uint32			width, height;
	Uint8			depth;
	Uint8			fullscreen;
	Uint8			resizable;
	char			*title;
	VALUE			listener;
	VALUE			mutex;
} surface_t;

void Init_surface();

VALUE surface_allocate(VALUE klass);
void surface_free(surface_t *s);
void surface_mark(surface_t *s);

VALUE surface_initialize(VALUE surface, VALUE title, VALUE width, VALUE height);

VALUE surface_set_width(VALUE object, VALUE width);
VALUE surface_get_width(VALUE object);
VALUE surface_set_height(VALUE object, VALUE height);
VALUE surface_get_height(VALUE object);
VALUE surface_set_size(VALUE object, VALUE width, VALUE height);

VALUE surface_set_depth(VALUE object, VALUE depth);
VALUE surface_get_depth(VALUE object);

VALUE surface_set_fullscreen(VALUE object, VALUE new_val);
VALUE surface_is_fullscreen(VALUE object);
VALUE surface_is_windowed(VALUE object);

VALUE surface_has_mousefocus(VALUE object);
VALUE surface_has_keyboardfocus(VALUE object);
VALUE surface_is_iconified(VALUE object);

VALUE surface_mouse_x(VALUE object);
VALUE surface_mouse_y(VALUE object);
VALUE surface_mouse_button(VALUE object, VALUE btn);

VALUE surface_show(VALUE object);
VALUE surface_is_shown(VALUE object);

VALUE surface_close(VALUE object);
/* 
 * To close a window from the surface_free function, we need a version of
 * surface_close that can be called with just a surface_t*
 */
void surface_close_internal(surface_t* surface);

VALUE surface_show_cursor(VALUE object, VALUE new_val);
VALUE surface_is_cursor_shown(VALUE object);

VALUE surface_set_eventlistener(VALUE object, VALUE listener);
VALUE surface_get_eventlistener(VALUE object);

VALUE surface_get_event(VALUE object);

VALUE surface_set_title(VALUE object, VALUE title);
VALUE surface_get_title(VALUE object);

VALUE surface_draw(VALUE object);

Sint8 surface_set_mode(surface_t *s);

VALUE surface_poll_for_event(VALUE object);

#endif
