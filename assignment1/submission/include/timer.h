#ifndef INC_TIMER_H_
#define INC_TIMER_H_

#define UNUSED(x) (void)(x)


// Struct decleration
struct t_eventData{
    int myData;
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

#endif /* INC_TIMER_H_ */