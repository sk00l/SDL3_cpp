#define SDL_MAIN_USE_CALLBACKS 1 

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <vector>
#include <SDL3_ttf/SDL_ttf.h>

//own headers

#include "collision.h"

//global varaibles

//varabile for window for the game, and rendered varaiable for drawing objects
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;

//resolutiuon of the screen
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 400;

//game loop contraol
bool running = true;

//dino(player) properties
//using SDL float rect we gave dino  position x= 100, y = 100 and the size w *h = 50 * 50
//otger variables explains itself, Uint8 to sore rgb value
SDL_FRect dino = { 100, 300, 50, 50 };
bool isJumping = false;
float velocity = 0.0f;
const float GRAVITY = 0.5f;
const float JUMP_STRENGTH = -10.0f;
Uint8 dinoR = 0, dinoG = 0, dinoB = 0;

//similarly ground properties
//used two ground for infinite scrolling effect
//groud1 with with size of screen_width * 50;
SDL_FRect ground1 = { 0, 350, SCREEN_WIDTH, 50 };
SDL_FRect ground2 = { SCREEN_WIDTH, 350, SCREEN_WIDTH, 50 };
const float GROUND_SPEED = 5.0f;

//obstacle class so that i can make multiple obstable.just oop things resuable blah blahhhhhhhhh    
class Obstacle {
public:
    SDL_FRect rect;
    SDL_FRect line; // vertivle line above the obstacle for scoreboard implementaion(wip)
    Uint8 r, g, b;

    Obstacle(float x, float y, float w, float h) {
        rect = { x, y, w, h };
        line = { x + w / 2 - 2, y - 200, 4, 200 };
        r = 255; g = 0; b = 0;
    }

    void update() {
        rect.x -= GROUND_SPEED;
        line.x -= GROUND_SPEED;
        if (rect.x + rect.w <= 0) {
            rect.x = SCREEN_WIDTH + (rand() % 100);
            line.x = rect.x + rect.w / 2 - 2;
        }
    }

    void render(SDL_Renderer* renderer) {
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderFillRect(renderer, &rect);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 
        SDL_RenderFillRect(renderer, &line);
    }
};

struct ObstacleNode {
    Obstacle obstacle;
    ObstacleNode* next;

    ObstacleNode(float x, float y, float w, float h)
        : obstacle(x,y,w,h), next(nullptr){}
};

class ObstacleList {
public:
    ObstacleNode* head;

    ObstacleList() : head(nullptr) {}

    void addObstacle(float x, float y, float w, float h) {
        ObstacleNode* newNode = new ObstacleNode(x, y, w, h);
        if (!head) {
            head = newNode;
        }
        else {
            ObstacleNode* temp = head;
            while (temp->next) temp = temp->next;
            temp->next = newNode;
        }
    }

    void updateObstacles() {
        ObstacleNode* temp = head;
        ObstacleNode* prev = nullptr;

        while (temp) {
            temp->obstacle.update();

            if (temp->obstacle.rect.x + temp->obstacle.rect.w <= 0) {
                // Remove off-screen obstacle
                if (prev) prev->next = temp->next;
                else head = temp->next;

                delete temp;
                temp = (prev) ? prev->next : head;
            }
            else {
                prev = temp;
                temp = temp->next;
            }
        }
    }

    void render(SDL_Renderer* renderer) {
        ObstacleNode* temp = head;
        while (temp) {
            temp->obstacle.render(renderer);
            temp = temp->next;
        }
    }
};

ObstacleList obstacles;



void update() {
    if (isJumping) {
        dino.y += velocity;
        velocity += GRAVITY;

        if (dino.y >= 300) {
            dino.y = 300;
            isJumping = false;
            velocity = 0;
        }
    }

    ground1.x -= GROUND_SPEED;
    ground2.x -= GROUND_SPEED;
    if (ground1.x + SCREEN_WIDTH <= 0) ground1.x = SCREEN_WIDTH;
    if (ground2.x + SCREEN_WIDTH <= 0) ground2.x = SCREEN_WIDTH;

    obstacles.updateObstacles();

    // Collision check
    ObstacleNode* temp = obstacles.head;
    while (temp) {
        if (checkCollision(dino, temp->obstacle.rect) || checkCollision(dino, temp->obstacle.line)) {
            dinoR = rand() % 256;
            dinoG = rand() % 256;
            dinoB = rand() % 256;
        }
        temp = temp->next;
    }

    // **Add a new obstacle periodically**
    static int frameCounter = 0;
    frameCounter++;
    if (frameCounter >= 100) {  // Every ~1.6 seconds
        obstacles.addObstacle(SCREEN_WIDTH + (rand() % 100), 320, 30, 30);
        frameCounter = 0;
    }
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    SDL_SetAppMetadata("Dino Game", "1.0", "com.dinogamee.dino-game");



    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Dino Game", SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer)) {
        SDL_Log("Couldnt create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Create multiple obstacles
    obstacles.addObstacle(SCREEN_WIDTH + 200, 320, 30, 30);
    obstacles.addObstacle(SCREEN_WIDTH + 500, 320, 30, 30);
    obstacles.addObstacle(SCREEN_WIDTH + 800, 320, 30, 30);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.key == SDLK_SPACE && !isJumping) {
            isJumping = true;
            velocity = JUMP_STRENGTH;
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    update();

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &ground1);
    SDL_RenderFillRect(renderer, &ground2);

    obstacles.render(renderer);

    SDL_SetRenderDrawColor(renderer, dinoR, dinoG, dinoB, 255);
    SDL_RenderFillRect(renderer, &dino);

    SDL_RenderPresent(renderer);
    SDL_Delay(16);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
