#include <ruby.h>

#define INIT_STUFF (SDL_INIT_AUDIO|SDL_INIT_VIDEO)

VALUE gamer_get_error();
VALUE gamer_audio_works();
VALUE gamer_video_works();

void Init_gamer();
