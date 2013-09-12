#ifndef GAMER_GRAPHICS_H
#define GAMER_GRAPHICS_H

#include <ruby.h>
#include <SDL/SDL.h>

#include "surface.h"

typedef struct graphics_s
{
	VALUE		surface;
	SDL_Surface *sdl_surf;
	Uint32		fg_color,
				bg_color;
	VALUE		mutex;
} graphics_t;

void Init_graphics(VALUE module);

VALUE graphics_allocate(VALUE klass);
void graphics_free(graphics_t *g);
void graphics_mark(graphics_t *g);

VALUE graphics_initialize(VALUE object, VALUE surface);

VALUE graphics_lock(VALUE object);
VALUE graphics_unlock(VALUE object);

VALUE graphics_set_color(VALUE object, VALUE color);
VALUE graphics_get_color(VALUE object);

VALUE graphics_set_bgcolor(VALUE object, VALUE color);
VALUE graphics_get_bgcolor(VALUE object);

VALUE graphics_clear(VALUE object);
VALUE graphics_clear_rect(VALUE object, VALUE x1, VALUE y1, VALUE x2, VALUE y2);

VALUE graphics_draw_pixel(VALUE object, VALUE x, VALUE y);
VALUE graphics_draw_line(VALUE object, VALUE x1, VALUE y1, VALUE x2, VALUE y2);

VALUE graphics_draw_rect(VALUE object, VALUE x1, VALUE y1, VALUE x2, VALUE y2);
VALUE graphics_fill_rect(VALUE object, VALUE x1, VALUE y1, VALUE x2, VALUE y2);

VALUE graphics_draw_circle(VALUE object, VALUE x, VALUE y, VALUE radius);
VALUE graphics_fill_circle(VALUE object, VALUE x, VALUE y, VALUE radius);

VALUE graphics_draw_ellipse(VALUE object, VALUE x, VALUE y, VALUE rx, VALUE ry);
VALUE graphics_fill_ellipse(VALUE object, VALUE x, VALUE y, VALUE rx, VALUE ry);

VALUE graphics_draw_bezier(VALUE object, VALUE xs, VALUE ys, VALUE steps);

VALUE graphics_draw_pie(VALUE object, VALUE x, VALUE y, VALUE radius, VALUE start, VALUE end);
VALUE graphics_fill_pie(VALUE object, VALUE x, VALUE y, VALUE radius, VALUE start, VALUE end);

VALUE graphics_draw_polygon(VALUE object, VALUE xs, VALUE ys);
VALUE graphics_fill_polygon(VALUE object, VALUE xs, VALUE ys);

VALUE graphics_draw_image(VALUE object, VALUE image, VALUE x, VALUE y);
VALUE graphics_draw_image_part(VALUE object, VALUE image, VALUE dx, VALUE dy, VALUE sx, VALUE sy, VALUE sw, VALUE sh);

#endif
