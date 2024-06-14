#include "framework.h"
#include <stdint.h>
#include <stdio.h>
#include "parameters.h"

    
// helpers
#define INDEX(X,Y) ((X) + ((Y)*SIZE_X))
#define DISCRETE_POSITION(A) INDEX(((size_t)A->x), ((size_t)A->y))
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))


void init_grid(uint8_t* data_map, float* trail_map, agent_t* agents) {

    for (size_t i = 0; i < NUM_PARTICLES; i++) {
        data_map[i] = AGENT_COLOR;
        trail_map[i] = 0;
    }
    for (size_t i = NUM_PARTICLES; i < SIZE_X * SIZE_Y; i++) {
        data_map[i] = EMPTY_COLOR;
        trail_map[i] = 0;
    }
    for (size_t i = 0; i < SIZE_X*SIZE_Y; i++) {
        size_t index = next_random() % (SIZE_X * SIZE_Y);
        uint8_t tmp = data_map[i];
        data_map[i] = data_map[index];
        data_map[index] = tmp;
    }
    size_t agent_idx = 0;
    for (size_t i = 0; i < SIZE_X; i++) {
        for (size_t j = 0; j < SIZE_Y; j++) {
            if(data_map[INDEX(i,j)] == AGENT_COLOR){
                agents[agent_idx].x = i; 
                agents[agent_idx].y = j;
                agents[agent_idx].angle = next_random() % 360;
                agent_idx++;
            }
        }
    }
}

bool pixel_is_valid(agent_t* agent){
    return (agent->x >= 0) && (agent->x < SIZE_X) && (agent->y >= 0) && (agent->y < SIZE_Y);
}

void deposit_trail(float* trail_map, agent_t* agent){
    trail_map[DISCRETE_POSITION(agent)] += DEP_T;
}

void motor_stage(uint8_t* data_map, float* trail_map, agent_t* agent) {
    agent_t next_pos = position_from_agent(agent, STEP_SIZE, 0);
    
    if (!pixel_is_valid(&next_pos)){
        // clamp position
        next_pos.x = MAX(0.0, MIN(next_pos.x, SIZE_X - 1));
        next_pos.y = MAX(0.0, MIN(next_pos.y, SIZE_Y - 1));
        
        // rotate
        if ((next_pos.x < 0) || (next_pos.x > SIZE_X)){ // vertical border
            rotate_agent(agent, 180 - agent->angle);
        } else {
            rotate_agent(agent, 270 - agent->angle);
        }
    }
    // data_map[DISCRETE_POSITION(agent)] = EMPTY_COLOR; // invalidate current position
    agent->x = next_pos.x;
    agent->y = next_pos.y;
    data_map[DISCRETE_POSITION(agent)] = AGENT_COLOR; // set new position

    // deposit trail in new location
    deposit_trail(trail_map, agent);
}

/* --------------------------------------- */

float sample_trail_map(float* trail_map, agent_t* agent, float angle) {
    agent_t pos = position_from_agent(agent, SENSOR_OFFSET, angle);
    return pixel_is_valid(&pos) ? trail_map[DISCRETE_POSITION((&pos))]: -1;
}

void sensory_stage(float* trail_map, agent_t* agent) {
    float f = sample_trail_map(trail_map, agent, 0);
    float fl = sample_trail_map(trail_map, agent, -SENSOR_ANGLE);
    float fr = sample_trail_map(trail_map, agent, SENSOR_ANGLE);
    if (f > fl && f > fr) {
        // keep direction
    } else if (f < fl && f < fr) {
        // rotate randomly
        rotate_agent(agent, next_random() % 2 ? ROTATION_ANGLE : -ROTATION_ANGLE);
    } else if (fl < fr) { 
        // rotate right
        rotate_agent(agent, ROTATION_ANGLE);
    } else if (fr < fl) { 
        // rotate left
        rotate_agent(agent, -ROTATION_ANGLE);
    } else {
        // keep direction
    }
}

// convolute kernel
void convolute_trails(float** trail_map, float** trail_map_buff, float* conv_map){
    
    float* tmp = (*trail_map_buff);
    (*trail_map_buff) = (*trail_map);
    (*trail_map) = tmp;

    const unsigned tile_size = 64;

    for (size_t ii = 1; ii < SIZE_X - 1; ii += tile_size) {
        for (size_t jj = 1; jj < SIZE_Y - 1; jj += tile_size) {
            for (size_t i = ii; i < MIN(ii + tile_size, SIZE_X - 1); i++) {
                for (size_t j = jj; j < MIN(jj + tile_size, SIZE_Y - 1); j++) {
                    (*trail_map)[INDEX(i, j)] = 0;
                    for (size_t n = 0; n < 3; n++) {
                        for (size_t m = 0; m < 3; m++) {
                            (*trail_map)[INDEX(i, j)] += (*trail_map_buff)[INDEX(i + n - 1, j + m - 1)] * conv_map[n + m*3];
                        }
                    }
                }
            }
        }
    }
    // zero borders
    for (size_t i = 0; i < 2; i++) {
        for (size_t j = 0; j < SIZE_Y; j++) {
            (*trail_map)[INDEX(i*(SIZE_X-1), j)] = 0;
        }
    }
    for (size_t i = 0; i < 2; i++) {
        for (size_t j = 0; j < SIZE_X; j++) {
            (*trail_map)[INDEX(j, i*(SIZE_Y-1))] = 0;
        }
    }
}

