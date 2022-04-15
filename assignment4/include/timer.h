#ifndef INC_TIMER_H_
#define INC_TIMER_H_

#define UNUSED(x) (void)(x)

#include <stdbool.h>

// Struct decleration
struct t_eventData{
    bool myData;
};



// function declreations
void timer_init_create();
void timer_init_signal();
void timer_register_sig_handle();
void timer_start();
void timer_stop();
void timer_init();

void timer_init_sigaction();
void timer_init_sigevent();

bool timer_get_flag();
void timer_clear_flag();

#endif /* INC_TIMER_H_ */