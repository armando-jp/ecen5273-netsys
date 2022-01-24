#ifndef INC_UTILS_H_
#define INC_UTILS_H_

#include <stdbool.h>
#include <stdint.h>

// function declerations
void print_hex(const char *s);
static bool is_number(char *str);
bool is_ip(char *ip_str);
bool is_port(char *port_str);

#endif /*INC_UTILS_H_*/