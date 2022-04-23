#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "utils.h"

void print_hex(const char *s)
{
  while(*s)
  {
      printf("0x%02x ", (unsigned int) *s++);
  }
  printf("\n");
}

static bool is_number(char *str)
{
    while(*str)
    {
        // if the character is not a number, return false
        if(!isdigit(*str))
        {
            return false;
        }
        // point to next character
        str++;
    }
    return true;
}

// only works for IPv4 addresses
bool is_ip(char *ip)
{
    // create a copy of the ip string, as this function is destructive.
    uint8_t ip_str_len = strlen(ip);
    char ip_str[ip_str_len];
    strcpy(ip_str, ip);

    char *ptr;
    uint8_t dots = 0;
    uint16_t num;

    if(ip_str == NULL)
    {
        return false;
    }

    // cut the string using dot delimiter
    ptr = strtok(ip_str, "."); 
    if(ptr == NULL)
    {
        return false;
    }

    while(ptr)
    {
        // check if sub string is holding only a number or not.
        if(!is_number(ptr))
        {
            return false;
        }
        // convert substring to number
        num = atoi(ptr); 
        
        if(num >= 0 && num <= 255)
        {
            
            // cut the next part of the string
            ptr = strtok(NULL, "."); 
            if(ptr != NULL)
            {
                dots++;
            }
        }
        else
        {
            return false;
        }
    }

    // if the number of dots is not 3, return false
    if(dots != 3)
    {
        return false;
    }

    return true;
}

bool is_port(char *port_str)
{
    uint32_t num;

    // check if string is holding only a number or not.
    if(!is_number(port_str))
    {
        return false;
    }

    // convert substring to number
    num = atoi(port_str); 

    if(num >= 0 && num <= 65535)
    {
        return true;
    }

    return false;
}

void get_filename(char *file_path)
{
    char *token;
    char file_name[30];
    token = strtok(file_path, "/");
    printf("file_path: %s\r\n", file_path);
    do
    {
        printf("token: %s\r\n", token);
        strcpy(file_name, token);
    } while ((token = strtok(NULL, "/")) != NULL);

    strcpy(file_path, file_name);
}