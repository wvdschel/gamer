#include <SDL/SDL_gfxPrimitives.h>
#include "graphics.h"
#include "surface.h"
#include "image.h"

VALUE gam_cGraphics;

void Init_graphics(VALUE module)
{
	gam_cGraphics = rb_define_class_under(module, "Graphics", rb_cObject);
	rb_define_alloc_func(gam_cGraphics, graphics_allocate);
	
	rb_define_method(gam_cGraphics, "initialize", graphics_initialize, 1);
	
	rb_define_method(gam_cGraphics, "color=", graphics_set_color, 1);
	rb_define_method(gam_cGraphics, "color", graphics_get_color, 0);
	rb_define_method(gam_cGraphics, "bgcolor=", graphics_set_bgcolor, 1);
	rb_define_method(gam_cGraphics, "bgcolor", graphics_get_bgcolor, 0);
	
	rb_define_method(gam_cGraphics, "lock", graphics_lock, 0);
	rb_define_method(gam_cGraphics, "unlock", graphics_unlock, 0);
	
	rb_define_method(gam_cGraphics, "clear", graphics_clear, 0);
	rb_define_method(gam_cGraphics, "clear_rectangle", graphics_clear_rect, 4);
	rb_define_method(gam_cGraphics, "clear_rect", graphics_clear_rect, 4);
	
	rb_define_method(gam_cGraphics, "draw_pixel", graphics_draw_pixel, 2);
	rb_define_method(gam_cGraphics, "draw_line", graphics_draw_line, 4);
	
	rb_define_method(gam_cGraphics, "draw_rect", graphics_draw_rect, 4);
	rb_define_method(gam_cGraphics, "fill_rect", graphics_fill_rect, 4);
	rb_define_method(gam_cGraphics, "draw_rectangle", graphics_draw_rect, 4);
	rb_define_method(gam_cGraphics, "fill_rectangle", graphics_fill_rect, 4);
	
	rb_define_method(gam_cGraphics, "draw_circle", graphics_draw_circle, 3);
	rb_define_method(gam_cGraphics, "fill_circle", graphics_fill_circle, 3);
	
	rb_define_method(gam_cGraphics, "draw_ellipse", graphics_draw_ellipse, 4);
	rb_define_method(gam_cGraphics, "fill_ellipse", graphics_fill_ellipse, 4);
	
	rb_define_method(gam_cGraphics, "draw_pie", graphics_draw_pie, 5);
	rb_define_method(gam_cGraphics, "fill_pie", graphics_fill_pie, 5);
	
	rb_define_method(gam_cGraphics, "draw_polygon", graphics_draw_polygon, 2);
	rb_define_method(gam_cGraphics, "fill_polygon", graphics_fill_polygon, 2);
	
	rb_define_method(gam_cGraphics, "draw_image", graphics_draw_image, 3);
	rb_define_method(gam_cGraphics, "draw_image_section", graphics_draw_image_part, 7);
}

VALUE graphics_allocate(VALUE klass)
{
	graphics_t *g = malloc(sizeof(graphics_t));
	g->surface 	= Qnil;
	g->sdl_surf = NULL;
	/* Default colors: black on white */
	g->fg_color = 0xFFFFFFFF;
	g->bg_color = 0x000000FF;
	return Data_Wrap_Struct(klass, graphics_mark, graphics_free, g);
}

void graphics_free(graphics_t *g)
{
	free(g);
}

void graphics_mark(graphics_t *g)
{
	rb_gc_mark(g->surface);
}

VALUE graphics_initialize(VALUE object, VALUE surface)
{
	graphics_t *g;
	VALUE surface_class, image_class;
	surface_class 	= rb_const_get(rb_const_get(rb_cModule, rb_intern("Gamer")), rb_intern("Surface"));
	image_class		= rb_const_get(rb_const_get(rb_cModule, rb_intern("Gamer")), rb_intern("Image"));
	Data_Get_Struct(object, graphics_t, g);
	
	if(CLASS_OF(surface) == surface_class)
	{
		surface_t *s;
		Data_Get_Struct(surface, surface_t, s);
		
		g->surface  = surface;
		g->sdl_surf = s->surf;
		g->mutex	= s->mutex;
	}
	else if(CLASS_OF(surface) == image_class)
	{
		image_t *s;
		Data_Get_Struct(surface, image_t, s);
		
		g->surface	= surface;
		g->sdl_surf = s->image;
		g->mutex	= s->mutex;
	}
	else
		rb_raise(rb_eArgError, "the surface parameter is supposed to be either a Gamer::Image or a Gamer::Surface");
	
	return object;
}

/* 
 * To make sure no unexpected behaviour is caused by several draw operations, we lock the graphics context using the
 * mutex from the surface. We also swap the buffers after unlocking.
 */

VALUE graphics_lock(VALUE object)
{
	graphics_t *g;
	Data_Get_Struct(object, graphics_t, g);
	
	/* Lock the mutex */
	rb_funcall(g->mutex, rb_intern("lock"), 0);
	
	return Qnil;
}

VALUE graphics_unlock(VALUE object)
{
	graphics_t *g;
	Data_Get_Struct(object, graphics_t, g);
	
	/* Unlock the mutex */
	rb_funcall(g->mutex, rb_intern("unlock"), 0);
	
	/* Flip the buffers */
	SDL_Flip(g->sdl_surf);
	
	return Qnil;
}

VALUE graphics_set_color(VALUE object, VALUE color)
{
	graphics_t *g;
	Data_Get_Struct(object, graphics_t, g);
	
	g->fg_color = NUM2UINT(color);
	
	return Qnil;
}

VALUE graphics_get_color(VALUE object)
{
	graphics_t *g;
	Data_Get_Struct(object, graphics_t, g);
	
	return UINT2NUM(g->fg_color);
}

VALUE graphics_set_bgcolor(VALUE object, VALUE color)
{
	graphics_t *g;
	Data_Get_Struct(object, graphics_t, g);
	
	g->bg_color = NUM2UINT(color);
	
	return Qnil;
}

VALUE graphics_get_bgcolor(VALUE object)
{
	graphics_t *g;
	Data_Get_Struct(object, graphics_t, g);
	
	return UINT2NUM(g->bg_color);
}

VALUE graphics_clear(VALUE object)
{
	graphics_t *g;
	
	Data_Get_Struct(object, graphics_t, g);
	
	boxColor(g->sdl_surf, 0, 0, g->sdl_surf->w, g->sdl_surf->h, g->bg_color);
	
	return Qnil;
}

VALUE graphics_clear_rect(VALUE object, VALUE x1, VALUE y1, VALUE x2, VALUE y2)
{
	graphics_t *g;
	Sint16 x1_c, x2_c, y1_c, y2_c;
	
	Data_Get_Struct(object, graphics_t, g);
	
	x1_c = NUM2INT(x1);
	y1_c = NUM2INT(y1);
	x2_c = NUM2INT(x2);
	y2_c = NUM2INT(y2);
	
	boxColor(g->sdl_surf, x1_c, y1_c, x2_c, y2_c, g->bg_color);
	
	return Qnil;
}

VALUE graphics_draw_pixel(VALUE object, VALUE x, VALUE y)
{
	graphics_t *g;
	Sint16 x_c, y_c;
	
	Data_Get_Struct(object, graphics_t, g);
	
	x_c = NUM2INT(x);
	y_c = NUM2INT(y);
	
	pixelColor(g->sdl_surf, x_c, y_c, g->fg_color);
	
	return Qnil;
}

VALUE graphics_draw_line(VALUE object, VALUE x1, VALUE y1, VALUE x2, VALUE y2)
{
	graphics_t *g;
	Sint16 x_c1, x_c2, y_c1, y_c2;
	
	Data_Get_Struct(object, graphics_t, g);
	x_c1 = NUM2INT(x1);
	y_c1 = NUM2INT(y1);
	x_c2 = NUM2INT(x2);
	y_c2 = NUM2INT(y2);
	
	if(x_c1 == x_c2)
		vlineColor(g->sdl_surf, x_c1, y_c1, y_c2, g->fg_color);
	else if(y_c1 == y_c2)
		hlineColor(g->sdl_surf, x_c1, x_c2, y_c1, g->fg_color);
	else
		aalineColor(g->sdl_surf, x_c1, y_c1, x_c2, y_c2, g->fg_color);
	
	return Qnil;
}

VALUE graphics_draw_rect(VALUE object, VALUE x1, VALUE y1, VALUE x2, VALUE y2)
{
	graphics_t *g;
	Sint16 x_c1, x_c2, y_c1, y_c2;
	
	Data_Get_Struct(object, graphics_t, g);
	x_c1 = NUM2INT(x1);
	y_c1 = NUM2INT(y1);
	x_c2 = NUM2INT(x2);
	y_c2 = NUM2INT(y2);
	
	rectangleColor(g->sdl_surf, x_c1, y_c1, x_c2, y_c2, g->fg_color);
	
	return Qnil;
}

VALUE graphics_fill_rect(VALUE object, VALUE x1, VALUE y1, VALUE x2, VALUE y2)
{
	graphics_t *g;
	Sint16 x_c1, x_c2, y_c1, y_c2;
	
	Data_Get_Struct(object, graphics_t, g);
	x_c1 = NUM2INT(x1);
	y_c1 = NUM2INT(y1);
	x_c2 = NUM2INT(x2);
	y_c2 = NUM2INT(y2);
	
	boxColor(g->sdl_surf, x_c1, y_c1, x_c2, y_c2, g->fg_color);
	
	return Qnil;
}

VALUE graphics_draw_circle(VALUE object, VALUE x, VALUE y, VALUE radius)
{
	graphics_t *g;
	Sint16 r_c, x_c, y_c;
	
	Data_Get_Struct(object, graphics_t, g);
	r_c  = NUM2INT(radius);
	x_c = NUM2INT(x);
	y_c = NUM2INT(y);
	
	aacircleColor(g->sdl_surf, x_c, y_c, r_c, g->fg_color);
	
	return Qnil;
}

VALUE graphics_fill_circle(VALUE object, VALUE x, VALUE y, VALUE radius)
{
	graphics_t *g;
	Sint16 r_c, x_c, y_c;
	
	Data_Get_Struct(object, graphics_t, g);
	r_c  = NUM2INT(radius);
	x_c = NUM2INT(x);
	y_c = NUM2INT(y);
	
	filledCircleColor(g->sdl_surf, x_c, y_c, r_c, g->fg_color);
	
	return Qnil;
}

VALUE graphics_draw_ellipse(VALUE object, VALUE x, VALUE y, VALUE rx, VALUE ry)
{
	graphics_t *g;
	Sint16 rx_c, ry_c, x_c, y_c;
	
	Data_Get_Struct(object, graphics_t, g);
	rx_c = NUM2INT(rx);
	ry_c = NUM2INT(ry);
	x_c  = NUM2INT(x);
	y_c  = NUM2INT(y);
	
	aaellipseColor(g->sdl_surf, x_c, y_c, rx_c, ry_c, g->fg_color);
	
	return Qnil;
}

VALUE graphics_fill_ellipse(VALUE object, VALUE x, VALUE y, VALUE rx, VALUE ry)
{
	graphics_t *g;
	Sint16 rx_c, ry_c, x_c, y_c;
	
	Data_Get_Struct(object, graphics_t, g);
	rx_c = NUM2INT(rx);
	ry_c = NUM2INT(ry);
	x_c  = NUM2INT(x);
	y_c  = NUM2INT(y);
	
	filledEllipseColor(g->sdl_surf, x_c, y_c, rx_c, ry_c, g->fg_color);
	
	return Qnil;
}

VALUE graphics_draw_pie(VALUE object, VALUE x, VALUE y, VALUE radius, VALUE start, VALUE end)
{
	graphics_t *g;
	Sint16 r_c, x_c, y_c, start_c, end_c;
	
	Data_Get_Struct(object, graphics_t, g);
	r_c		= NUM2INT(radius);
	start_c = NUM2INT(start);
	end_c	= NUM2INT(end);
	x_c		= NUM2INT(x);
	y_c		= NUM2INT(y);
	
	pieColor(g->sdl_surf, x_c, y_c, r_c, start_c, end_c, g->fg_color);
	
	return Qnil;
}

VALUE graphics_fill_pie(VALUE object, VALUE x, VALUE y, VALUE radius, VALUE start, VALUE end)
{
	graphics_t *g;
	Sint16 r_c, x_c, y_c, start_c, end_c;
	
	Data_Get_Struct(object, graphics_t, g);
	r_c		= NUM2INT(radius);
	start_c = NUM2INT(start);
	end_c	= NUM2INT(end);
	x_c		= NUM2INT(x);
	y_c		= NUM2INT(y);
	
	filledPieColor(g->sdl_surf, x_c, y_c, r_c, start_c, end_c, g->fg_color);
	
	return Qnil;
}


VALUE graphics_draw_bezier(VALUE object, VALUE xs, VALUE ys, VALUE steps)
{
	graphics_t *g;
	struct RArray *rx_array, *ry_array;
	Sint16 *x_array, *y_array, steps_c;
	int i;
	
	rx_array = RARRAY(xs);
	ry_array = RARRAY(ys);
	
	if(rx_array->len != ry_array->len)
		rb_raise(rb_eArgError, "x and y should have the same number of elements");
	
	Data_Get_Struct(object, graphics_t, g);
	
	x_array = malloc(sizeof(Sint16) * rx_array->len);
	y_array = malloc(sizeof(Sint16) * rx_array->len);
	
	for(i = 0; i < rx_array->len; i++)
	{
		x_array[i] = NUM2INT(rx_array->ptr[i]);
		y_array[i] = NUM2INT(ry_array->ptr[i]);
	}
	
	steps_c = NUM2INT(steps);
	
	bezierColor(g->sdl_surf, x_array, y_array, rx_array->len, steps_c, g->fg_color);
	
	return Qnil;
}

VALUE graphics_draw_polygon(VALUE object, VALUE xs, VALUE ys)
{
	graphics_t *g;
	struct RArray *rx_array, *ry_array;
	Sint16 *x_array, *y_array;
	int i;
	
	rx_array = RARRAY(xs);
	ry_array = RARRAY(ys);
	
	if(rx_array->len != ry_array->len)
		rb_raise(rb_eArgError, "xs and ys should have the same number of elements");
	
	Data_Get_Struct(object, graphics_t, g);
	
	x_array = malloc(sizeof(Sint16) * rx_array->len);
	y_array = malloc(sizeof(Sint16) * rx_array->len);
	
	for(i = 0; i < rx_array->len; i++)
	{
		x_array[i] = NUM2INT(rx_array->ptr[i]);
		y_array[i] = NUM2INT(ry_array->ptr[i]);
	}
	
	aapolygonColor(g->sdl_surf, x_array, y_array, rx_array->len, g->fg_color);
	
	return Qnil;
}

VALUE graphics_fill_polygon(VALUE object, VALUE xs, VALUE ys)
{

	graphics_t *g;
	struct RArray *rx_array, *ry_array;
	Sint16 *x_array, *y_array;
	int i;
	
	rx_array = RARRAY(xs);
	ry_array = RARRAY(ys);
	
	if(rx_array->len != ry_array->len)
		rb_raise(rb_eArgError, "xs and ys should have the same number of elements");
	
	Data_Get_Struct(object, graphics_t, g);
	
	x_array = malloc(sizeof(Sint16) * rx_array->len);
	y_array = malloc(sizeof(Sint16) * rx_array->len);
	
	for(i = 0; i < rx_array->len; i++)
	{
		x_array[i] = NUM2INT(rx_array->ptr[i]);
		y_array[i] = NUM2INT(ry_array->ptr[i]);
	}
	
	filledPolygonColor(g->sdl_surf, x_array, y_array, rx_array->len, g->fg_color);
	
	return Qnil;
}

VALUE graphics_draw_image(VALUE object, VALUE image, VALUE x, VALUE y)
{
	SDL_Rect dest;
	image_t *img;
	graphics_t *gfx;
	
	dest.x = NUM2INT(x);
	dest.y = NUM2INT(y);
	
	Data_Get_Struct(object, graphics_t, gfx);
	Data_Get_Struct(image, image_t, img);
	
	SDL_BlitSurface(img->image, NULL, gfx->sdl_surf, &dest);
	
	return Qnil;
}

VALUE graphics_draw_image_part(VALUE object, VALUE image, VALUE dx, VALUE dy, VALUE sx, VALUE sy, VALUE sw, VALUE sh)
{
	SDL_Rect dest, src;
	image_t *img;
	graphics_t *gfx;
	
	dest.x = NUM2INT(dx);
	dest.y = NUM2INT(dy);
	
	src.x = NUM2INT(sx);
	src.y = NUM2INT(sy);
	src.w = NUM2INT(sw);
	src.h = NUM2INT(sh);
	
	Data_Get_Struct(object, graphics_t, gfx);
	Data_Get_Struct(image, image_t, img);
	
	SDL_BlitSurface(img->image, &src, gfx->sdl_surf, &dest);
	
	return Qnil;
}
