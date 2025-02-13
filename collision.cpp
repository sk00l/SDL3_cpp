#define SDL_MAIN_HANDLED
#include "collision.h"


bool checkCollision(SDL_FRect a, SDL_FRect b) {
	return SDL_HasRectIntersectionFloat(&a, &b);
}