# include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gifenc.h"


typedef int32_t angle_t; // sign bit is used to invalidate 
#define MAX_ANGLE 360

typedef uint8_t trail_t;

typedef int32_t pixel_index_t;

typedef struct {
    pixel_index_t x;
    pixel_index_t y;
} position_t;


// Framework environmnent parameters
#define SIZE_X 400
#define SIZE_Y 600
#define NUM_PARTICLES 5000
#define ITERATIONS 1000

// agent parameters
#define SENSOR_ANGLE FROM_DEG(22.5) // angle
#define ROTATION_ANGLE FROM_DEG(22.5) // angle
#define SENSOR_OFFSET 3 // pixels
#define SENSOR_WIDTH 1 // pixels 
#define STEP_SIZE 1 // pixels per frame 
#define DATA_MAP_EMPTY -1
#define DEP_T 50 // deposition per frame
    
// helpers
// #define FROM_DEG(X) (X*256)/360
// #define TO_DEG(X) (X*360)/256
// #define TO_RAD(X) (X*3.14159)/256.0
#define FROM_DEG(X) (angle_t)X
#define TO_DEG(X) (angle_t)X 
#define TO_RAD(X) (X*3.14159)/360.0
#define INDEX(X,Y) (X + Y*SIZE_X)
#define INDEX_POS(P) INDEX(P->x, P->y)

/*  Written in 2019 by David Blackman and Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

See <http://creativecommons.org/publicdomain/zero/1.0/>. */

#include <stdint.h>

/* This is xoshiro128++ 1.0, one of our 32-bit all-purpose, rock-solid
   generators. It has excellent speed, a state size (128 bits) that is
   large enough for mild parallelism, and it passes all tests we are aware
   of.

   For generating just single-precision (i.e., 32-bit) floating-point
   numbers, xoshiro128+ is even faster.

   The state must be seeded so that it is not everywhere zero. */

static uint32_t s[4];

static inline uint32_t rotl(const uint32_t x, int k) {
    return (x << k) | (x >> (32 - k));
}

uint32_t next_random(void) {
    const uint32_t result = rotl(s[0] + s[3], 7) + s[0];

    const uint32_t t = s[1] << 9;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;

    s[3] = rotl(s[3], 11);

    return result;
}

void init_grid(angle_t* data_map, trail_t* trail_map, position_t* agents) {

    for (size_t i = 0; i < NUM_PARTICLES; i++) {
        data_map[i] = FROM_DEG((int)((next_random() / (float)UINT32_MAX)*360));
        trail_map[i] = 0;
    }
    for (size_t i = NUM_PARTICLES; i < SIZE_X * SIZE_Y; i++) {
        data_map[i] = -1;
        trail_map[i] = 0;
    }
    for (size_t i = 0; i < SIZE_X*SIZE_Y; i++) {
        size_t index = next_random() % (SIZE_X * SIZE_Y);
        angle_t tmp = data_map[i];
        data_map[i] = data_map[index];
        data_map[index] = tmp;
    }
    size_t agent_idx = 0;
    for (pixel_index_t i = 0; i < SIZE_X; i++) {
        for (pixel_index_t j = 0; j < SIZE_Y; j++) {
            if(data_map[INDEX(i,j)] != -1){
                agents[agent_idx].x = i; 
                agents[agent_idx].y = j;
                agent_idx++;
            }
        }
    }

}

bool pixel_is_valid(angle_t* data_map, position_t* pos){
    return (data_map[INDEX_POS(pos)] == DATA_MAP_EMPTY) && (pos->x >= 0) && (pos->x < SIZE_X) && (pos->y >= 0) && (pos->y < SIZE_Y);
}

position_t pixel_from_position(position_t* pos, angle_t angle, float distance){
    return (position_t){.x = pos->x + cos(TO_RAD(angle)) * (distance + 0.5), .y =  pos->y + sin(TO_RAD(angle)) * (distance + 0.5)};
}

bool move_forward(angle_t* data_map, position_t* pos) { 
    angle_t angle = data_map[INDEX_POS(pos)];
    position_t new_pos = pixel_from_position(pos, angle, STEP_SIZE);
                                             

    if(pixel_is_valid(data_map, &new_pos)){
        data_map[INDEX_POS(pos)] = DATA_MAP_EMPTY; // invalidate current position
        data_map[INDEX_POS((&new_pos))] = angle; // set new position
        *pos = new_pos;
        return true;
    }
    return false; 
}

void deposit_trail(trail_t* trail_map, position_t* pos){
    trail_map[INDEX_POS(pos)] += DEP_T;
}

void motor_stage(angle_t* data_map, trail_t* trail_map, position_t* pos) {
    if (move_forward(data_map, pos)) { // if move is possible
        // deposit trail in new location
        deposit_trail(trail_map, pos);
    } else { // if didn't move
        data_map[INDEX_POS(pos)] = FROM_DEG((int)((next_random() / (float)UINT32_MAX)*360));
    }
}

/* --------------------------------------- */

void sample_trail_map(angle_t* data_map, trail_t* trail_map, position_t* pos, trail_t *f, trail_t *fl, trail_t *fr) {
    position_t fpos = pixel_from_position(pos, data_map[INDEX_POS(pos)], SENSOR_OFFSET);
    position_t flpos = pixel_from_position(pos, data_map[INDEX_POS(pos)] + FROM_DEG(-SENSOR_ANGLE), SENSOR_OFFSET);
    position_t frpos = pixel_from_position(pos, data_map[INDEX_POS(pos)] + FROM_DEG(SENSOR_ANGLE), SENSOR_OFFSET);
    *f = trail_map[INDEX_POS((&fpos))];
    *fl = trail_map[INDEX_POS((&flpos))];
    *fr = trail_map[INDEX_POS((&frpos))];
}

void sensory_stage(angle_t* data_map, trail_t* trail_map, position_t* pos) {
    trail_t f, fl, fr;
    sample_trail_map(data_map, trail_map, pos, &f, &fl, &fr);
    if (f > fl && f > fr) {
        // keep direction
    } else if (f < fl && f < fr) {
        // rotate randomly
    } else if (fl < fr) { 
        // rotate right
        data_map[INDEX_POS(pos)] = (data_map[INDEX_POS(pos)] + ROTATION_ANGLE) % FROM_DEG(360);
    } else if (fr < fl) { 
        // rotate left
        data_map[INDEX_POS(pos)] = (data_map[INDEX_POS(pos)] + (FROM_DEG(360) - ROTATION_ANGLE)) % FROM_DEG(360);
    } else {
        // keep direction
    }
}

/* --------------------------------------- */

void convolute_trails(trail_t* trail_map, trail_t* next_trail_map, float* conv_map){
    for (pixel_index_t i = 1; i < SIZE_X - 1; i++) {
        for (pixel_index_t j = 1; j < SIZE_Y - 1; j++) {
            next_trail_map[INDEX(i, j)] = 0;
            for (pixel_index_t n = 0; n < 3; n++) {
                for (pixel_index_t m = 0; m < 3; m++) {
                    next_trail_map[INDEX(i, j)] += trail_map[INDEX(i - 1 + n, j - 1 + m)] * conv_map[n + m*3];
                }
            }
        }
    }
    // borders
    for (pixel_index_t i = 0; i < 2; i++) {
        for (pixel_index_t j = 0; j < 2; j++) {
            next_trail_map[INDEX(i*(SIZE_X-1), j*(SIZE_Y-1))] = 0;
            for (pixel_index_t n = 0; n < 3; n++) {
                for (pixel_index_t m = 0; m < 3; m++) {
                    int ii = i - 1 + n;
                    int jj = j - 1 + m;
                    if(ii >= 0 && ii < 2 && jj >= 0 && jj < 2)
                        next_trail_map[INDEX(i*(SIZE_X-1), j*(SIZE_Y-1))] += trail_map[INDEX(ii*(SIZE_X-1), jj*(SIZE_Y-1))] * conv_map[i + j*3];
                }
            }
        }
    }
}

/* ---------------------------------------------- */

void* init_animation(){
    /* create a GIF */
    ge_GIF *gif = ge_new_gif(
        "slime.gif",  /* file name */
        SIZE_X, SIZE_Y,           /* canvas size */
        (uint8_t []) {  /* palette */
            0xFF, 0xFF, 0xFF, // Start with all 0xFF values
            0xFC, 0xFF, 0xFF, // Decrease the first value
            0xF8, 0xFF, 0xFF,
            0xF4, 0xFF, 0xFF,
            0xF0, 0xFF, 0xFF,
            0xEC, 0xFF, 0xFF,
            0xE8, 0xFF, 0xFF,
            0xE4, 0xFF, 0xFF,
            0xE0, 0xFF, 0xFF,
            0xDC, 0xFF, 0xFF,
            0xD8, 0xFF, 0xFF,
            0xD4, 0xFF, 0xFF,
            0xD0, 0xFF, 0xFF,
            0xCC, 0xFF, 0xFF,
            0xC8, 0xFF, 0xFF,
            0xC4, 0xFF, 0xFF,
            0xC0, 0xFF, 0xFF,
            0xBC, 0xFF, 0xFF,
            0xB8, 0xFF, 0xFF,
            0xB4, 0xFF, 0xFF,
            0xB0, 0xFF, 0xFF,
            0xAC, 0xFF, 0xFF,
            0xA8, 0xFF, 0xFF,
            0xA4, 0xFF, 0xFF,
            0xA0, 0xFF, 0xFF,
            0x9C, 0xFF, 0xFF,
            0x98, 0xFF, 0xFF,
            0x94, 0xFF, 0xFF,
            0x90, 0xFF, 0xFF,
            0x8C, 0xFF, 0xFF,
            0x88, 0xFF, 0xFF,
            0x84, 0xFF, 0xFF,
            0x80, 0xFF, 0xFF,
            0x7C, 0xFF, 0xFF,
            0x78, 0xFF, 0xFF,
            0x74, 0xFF, 0xFF,
            0x70, 0xFF, 0xFF,
            0x6C, 0xFF, 0xFF,
            0x68, 0xFF, 0xFF,
            0x64, 0xFF, 0xFF,
            0x60, 0xFF, 0xFF,
            0x5C, 0xFF, 0xFF,
            0x58, 0xFF, 0xFF,
            0x54, 0xFF, 0xFF,
            0x50, 0xFF, 0xFF,
            0x4C, 0xFF, 0xFF,
            0x48, 0xFF, 0xFF,
            0x44, 0xFF, 0xFF,
            0x40, 0xFF, 0xFF,
            0x3C, 0xFF, 0xFF,
            0x38, 0xFF, 0xFF,
            0x34, 0xFF, 0xFF,
            0x30, 0xFF, 0xFF,
            0x2C, 0xFF, 0xFF,
            0x28, 0xFF, 0xFF,
            0x24, 0xFF, 0xFF,
            0x20, 0xFF, 0xFF,
            0x1C, 0xFF, 0xFF,
            0x18, 0xFF, 0xFF,
            0x14, 0xFF, 0xFF,
            0x10, 0xFF, 0xFF,
            0x0C, 0xFF, 0xFF,
            0x08, 0xFF, 0xFF,
            0x00, 0x00, 0x00, // black idx = 63
        },
        6,              /* palette depth == log2(# of colors) */
        -1,             /* no transparency */
        0               /* infinite loop */
    );
    return gif;
}

void step_animation(void* animation, angle_t* data_map, trail_t* trail_map){
    ge_GIF *gif = animation;

    for (size_t i = 0; i < SIZE_X; i++) {
        for (size_t j = 0; j < SIZE_Y; j++) {
            gif->frame[INDEX(i,j)] = (int)((trail_map[INDEX(i,j)] / (float)256) * 62);
            if(data_map[INDEX(i, j)] != -1){
                gif->frame[INDEX(i,j)] = 63;
            }
            // gif->frame[INDEX(i,j)] = i % 3 == 0 || j % 4 == 0 ? 0 : 1;
        }
    }
    ge_add_frame(gif, 5);

    // printf("\n\n");
    // for (size_t i = 0; i < SIZE_Y; i++) {
    //     for (size_t j = 0; j < SIZE_X; j++) {
    //         printf("% 3d", data_map[INDEX(j,i)]);
    //     }
    //     printf("\n");
    // }
}

void stop_animation(void* animation){
     ge_close_gif(animation);
}


void simulate() {
    angle_t *data_map = malloc(sizeof(angle_t) * SIZE_X * SIZE_Y);
    trail_t *trail_map = malloc(sizeof(trail_t) * SIZE_X * SIZE_Y);
    trail_t *trail_map2 = malloc(sizeof(trail_t) * SIZE_X * SIZE_Y);
    position_t *agents = malloc(sizeof(position_t) * NUM_PARTICLES);
    float conv_map[9] = {.025f,.1f,.025f,
                         .1f,.5f,.1f,
                         .025f,.1f,.025f};

    init_grid(data_map, trail_map, agents);

    void* animation = init_animation();
    step_animation(animation, data_map, trail_map);


    // main loop
    for (size_t n = 0; n < ITERATIONS; n++) {
        convolute_trails(trail_map, trail_map2, conv_map);
        trail_t* tmp = trail_map2;
        trail_map2 = trail_map;
        trail_map = tmp;
        for (size_t i = 0; i < NUM_PARTICLES; i++) {
            sensory_stage(data_map, trail_map, &(agents[i]));
        }
        for (size_t i = 0; i < NUM_PARTICLES; i++) {
            motor_stage(data_map, trail_map, &(agents[i]));
        }
        step_animation(animation, data_map, trail_map);
    }

    stop_animation(animation);
    free(data_map);
    free(trail_map);
    free(trail_map2);
    free(agents);
}

int main(int argc, char *argv[]) { 

    s[0] = 123;
    s[1] = 314;
    s[2] = 743;
    s[3] = 776;

    simulate();

    return EXIT_SUCCESS; 
}
