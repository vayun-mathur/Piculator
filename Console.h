#pragma once


#include <iostream>
#include <windows.h>

#define BLACK           30  
#define RED             31  
#define GREEN           32  
#define YELLOW          33  
#define BLUE            34  
#define MAGENTA         35  
#define CYAN            36  
#define WHITE           37  
#define BRIGHT_BLACK    90  
#define BRIGHT_RED      91  
#define BRIGHT_GREEN    92  
#define BRIGHT_YELLOW   93  
#define BRIGHT_BLUE     94  
#define BRIGHT_MAGENTA  95
#define BRIGHT_CYAN     96
#define BRIGHT_WHITE    97

template<typename... Args>
void printf_color(int color_code, std::string s, Args ... args) {
	printf(("\x1B[" + std::to_string(color_code) + "m" + s + "\033[0m").c_str(), args...);
    fflush(stdout);
}

std::string print_num_commas(uint64_t n) {
    if (n < 1000) {
        return std::to_string(n);
    }
    char* buff = new char[4];
    sprintf(buff, ",%03llu", n % 1000);
    return print_num_commas(n / 1000) + buff;
}