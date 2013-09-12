#include <stdio.h>
#include <ruby.h>
#include <SDL/SDL.h>

#include "gamer.h"
#include "surface.h"
#include "graphics.h"
#include "image.h"
#include "event.h"

VALUE gam_mGamer;

VALUE gamer_get_error()
{
	return rb_str_new2(SDL_GetError());
}

VALUE gamer_audio_works()
{
	Uint32 winit = SDL_WasInit(INIT_STUFF);

	if(winit & SDL_INIT_AUDIO)
		return Qtrue;
	else
		return Qfalse;
}

VALUE gamer_video_works()
{
	Uint32 winit = SDL_WasInit(INIT_STUFF);

	if(winit & SDL_INIT_VIDEO)
		return Qtrue;
	else
		return Qfalse;
}

void Init_gamer()
{
	Sint32 init_res;	// The return value of SDL_Init
	
	/* Create a Gamer module */
	gam_mGamer = rb_define_module("Gamer");
	rb_define_module_function(gam_mGamer, "get_error", gamer_get_error, 0);
	/* Set up the Surface class */
	Init_surface(gam_mGamer);
	/* Set up the Event class */
	Init_event(gam_mGamer);
	/* Set up the Image class */
	Init_image(gam_mGamer);
	/* Set up the Graphics class */
	Init_graphics(gam_mGamer);

	/* Try and initialize SDL */
	init_res = SDL_Init(INIT_STUFF);

	/* If not everything went as planned, log the error */
	if(init_res < 0)
	{
		if(rb_respond_to(gam_mGamer, rb_intern("log")))
			rb_funcall(gam_mGamer, rb_intern("log"), 1, rb_str_new2(SDL_GetError()));
	}

	atexit(SDL_Quit);

	rb_define_module_function(gam_mGamer, "audio?", gamer_audio_works, 0);
	rb_define_module_function(gam_mGamer, "video?", gamer_video_works, 0);
}

