#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "agent.h"
#include "random.h"

void init_grid(uint8_t* data_map, float* trail_map, agent_t* agents);
void motor_stage(uint8_t* data_map, float* trail_map, agent_t* agent);
void sensory_stage(float* trail_map, agent_t* agent);
void convolute_trails(float** trail_map, float** trail_map_buff, float* conv_map);
