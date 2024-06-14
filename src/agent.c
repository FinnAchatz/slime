#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "agent.h"

agent_t position_from_agent(agent_t* agent, float step, float rotation) { 
    return (agent_t){.x = agent->x + step * cos(agent->angle + rotation), 
                     .y = agent->y + step * sin(agent->angle + rotation)};
}

/* move agent step in angle direction (relative to agent rotation) 
 * Doesn't check if move is valid */
void move_agent(agent_t* agent, float step, float rotation) { 
    agent_t new_ag = position_from_agent(agent, step, rotation);
    agent->x = new_ag.x;
    agent->y = new_ag.y;
}

void rotate_agent(agent_t* agent, float angle){
    agent->angle += angle;
}
