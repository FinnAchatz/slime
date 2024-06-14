typedef struct {
    float x, y;
    float angle;
} agent_t;

agent_t position_from_agent(agent_t* agent, float step, float rotation);
void rotate_agent(agent_t*, float);
