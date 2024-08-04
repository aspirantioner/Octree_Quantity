#ifndef _COMMON_HPP_
#define _COMMON_HPP_
#include <stdint.h>

uint8_t get_bit(uint8_t val,uint8_t bit){
    return (val&(1<<bit))!=0?1:0;
}

#endif