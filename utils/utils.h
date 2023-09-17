#pragma once

#include <string>

int random_int_ranged(int inclusiveMin, int exclusiveMax);

void clear_screen();
void position_cursor_to_home();

void pad_right_ref(std::string &str, const size_t num, const char paddingChar = ' ');

template<typename T>
void swap_references(T& a, T& b);

bool prompt_question(const char* question);
