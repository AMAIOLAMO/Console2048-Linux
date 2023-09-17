#include "utils.h"

#include <iostream>
#include <cstdlib>

int random_int_ranged(int inclusiveMin, int exclusiveMax) {
  return std::rand() % (exclusiveMax - inclusiveMin) + inclusiveMin;
}

void clear_screen() {
  std::cout << "\033[2J";
}

void position_cursor_to_home() {
  std::cout << "\033[1;1H";
}

template<typename T>
void swap_references(T& a, T& b) {
  int tempSwapBuffer = a;

  a = b;
  b = tempSwapBuffer;
}

void pad_right_ref(std::string &str, const size_t num, const char paddingChar) {
  if(num > str.size())
      str.insert(str.size(), num - str.size(), paddingChar);

}

bool prompt_question(const char* question) {
  std::cout << question;

  auto lowerCasedInput = std::tolower(getchar());

  if(lowerCasedInput == 'y') {
    std::cout << "\n";
    return true;
  }

  std::cout << "\n";
  return false;
}
