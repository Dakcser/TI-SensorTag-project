enum states {
    START,
    LOOP,
    END,
} state;

enum events {
    START_LOOPING,
    PRINT_HELLO,
    STOP_LOOPING,
};

void step_state(enum events event);
void start_statemachine(int i);

