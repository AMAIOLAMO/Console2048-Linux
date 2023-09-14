#include <cmath>
#include <string>
#include <iomanip>
#include <iostream>

#include <stdlib.h>
#include <unistd.h>

#ifdef __linux__
#include <termios.h>
#endif

#define m_X 4
#define m_Y 4

int Map[m_X][m_Y] = {0};

const int valid_inputs[] = {'a', 'd', 'w', 's', 'q'};

void display_map();
void try_push_map_cells(char input);
void spawn_two_or_four_on_rand_empty_cell();

bool try_push_cells(int& currentCell, int& targetCell,
    int& previousCell, bool atEnd);

template<typename T>
void swap_references(T& a, T& b);

bool is_valid_input(char character);
int get_highest_value_on_map();

bool has_valid_moves();
bool map_contains(int value);

void clear_screen();
void position_cursor_to_home();

bool prompt_question(const char* question);


#ifdef __linux__
struct termios orig_termios;

void disable_raw_mode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disable_raw_mode);
  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ECHO | ICANON);

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
#endif

int main(void) {
  std::srand(time(NULL));

#ifdef __linux__
  enable_raw_mode();
#endif 

  spawn_two_or_four_on_rand_empty_cell();

  std::cout << "== 2048  Game ==\n";

  std::cout << "High score: " << get_highest_value_on_map() << '\n';

  display_map();

  while (true) {
    char lowerCasedKeyboardInput = std::tolower(getchar());

    if (is_valid_input(lowerCasedKeyboardInput) == false) {
      continue;
    }

    clear_screen();
    position_cursor_to_home();

    std::cout << "== 2048  Game ==\n";

    std::cout << "High score: " << get_highest_value_on_map() << '\n';

    try_push_map_cells(lowerCasedKeyboardInput);

    if (lowerCasedKeyboardInput == 'q') {
      auto promptResult = prompt_question("do you want to quit? (Y/N):");

      if (promptResult == true) {
        std::cout << "See you soon!";
        break;
      }
    }

    spawn_two_or_four_on_rand_empty_cell();

    display_map();

    if (has_valid_moves() == false) {
      std::cout << "\nGame over!";
      break;
    }

    if (get_highest_value_on_map() == 2048) {
      std::cout << "\n==== You Won! ====";
      break;
    }
  }
}

void try_push_map_cells(char input) {
  if (input == 'a') {
    for (int y = 0; y < m_Y; y++)
      for (int x = 0; x <m_X; x++) {
        int& currentCell = Map[x][y];

        if (currentCell == 0) continue;

        for (int i = x - 1; i >= 0; i--) {
          int& targetCell = Map[i][y];
          int& previousCell = Map[i + 1][y];

          auto atEnd = (i == 0);

          auto pushingFinished =
            try_push_cells(currentCell, targetCell,
                previousCell, atEnd);

          if(pushingFinished) {
            break;
          }
        }
      }
  }

  if (input == 'd') {
    for (int y = 0; y < m_Y; y++)
      for (int x = m_X - 1; x >= 0; x--) {
        int& currentCell = Map[x][y];

        if (currentCell == 0) continue;

        for (int i = x + 1; i < m_X; i++) {
          auto& targetCell = Map[i][y];
          auto& previousCell = Map[i - 1][y];

          auto atEnd = (i == m_X - 1);

          auto pushingFinished =
            try_push_cells(currentCell, targetCell,
                previousCell, atEnd);

          if(pushingFinished) {
            break;
          }
        }
      }
  }
  
  if (input == 'w') {
    for (int x = 0; x <m_X; x++)
      for (int y = 0; y < m_Y; y++) {
        auto& currentCell = Map[x][y];

        if (currentCell == 0) continue;

        for (int i = y - 1; i >= 0; i--) {
          auto& targetCell = Map[x][i];
          auto& previousCell = Map[x][i + 1];

          auto atEnd = (i == 0);

          auto pushingFinished =
            try_push_cells(currentCell, targetCell,
                previousCell, atEnd);

          if(pushingFinished) {
            break;
          }
        }
      }
  }

  if (input == 's') {
    for (int x = 0; x <m_X; x++)
      for (int y = m_Y - 1; y >= 0; y--) {
        auto& currentCell = Map[x][y];

        if (currentCell == 0) continue;

        for (int i = y + 1; i < m_Y; i++) {
          auto& targetCell = Map[x][i];
          auto& previousCell = Map[x][i - 1];

          auto atEnd = (i == m_Y - 1);

          auto pushingFinished =
            try_push_cells(currentCell, targetCell,
                previousCell, atEnd);

          if(pushingFinished) {
            break;
          }
        }
      }
  }
}

bool has_valid_moves() {

  for (int y = 0; y < m_Y; y++)
    for (int x = 0; x <m_X; x++)
    {
      int currentCell = Map[x][y];

      if (currentCell == 0)
        return true;

      // left
      if (x - 1 >= 0 &&
          currentCell == Map[x - 1][y])
        return true;

      // right
      if (x + 1 < m_X &&
          currentCell == Map[x + 1][y])
        return true;

      // up
      if (y - 1 >= 0 &&
          currentCell == Map[x][y - 1])
        return true;

      // down
      if (y + 1 < m_Y &&
          currentCell == Map[x][y + 1])
        return true;
    }

  return false;
}

const char* colorCodes[] = {
  "\x1B[31m", // 2
  "\x1B[32m", // 4
  "\x1B[33m", // 8
  "\x1B[34m", // 16
  "\x1B[35m", // 32
  "\x1B[36m", // 64
  "\x1B[36m", // 128
  "\x1B[36m", // 256
  "\x1B[93m", // 512

  "\033[3;42;30m",  // 1024
  "\033[3;43;30m",  // 2048
};

void pad_right_ref(std::string &str, const size_t num, const char paddingChar = ' ')
{
  if(num > str.size())
      str.insert(str.size(), num - str.size(), paddingChar);
}

void display_map() {
  for (int y = 0; y < m_Y; y++) {
    for (int x = 0; x <m_X; x++) {
      auto currentCell = Map[x][y];

      auto cellAsString = std::to_string(currentCell);

      pad_right_ref(cellAsString, 4);

      if(currentCell == 0) {
          std::cout << "\x1B[37m" << cellAsString; 
      }
      else {
        int colorSelect = (int)std::log2(currentCell);

        std::cout << colorCodes[colorSelect - 1] << cellAsString;
      }

      std::cout << "\033[0m ";
    }

    std::cout << "\n";
  }
}

bool is_valid_input(char character) {
  for(auto valid_input : valid_inputs) {
    if (valid_input == character) return true;
  }

  return false;
}

bool map_contains(int value) {
   for (int y = 0; y < m_Y; y++)
     for (int x = 0; x <m_X; x++) {
       if (Map[x][y] == value) return true;
     }
  //else

  return false;
}

int random_int_ranged(int inclusiveMin, int exclusiveMax) {
  return std::rand() % (exclusiveMax - inclusiveMin) + inclusiveMin;
}

void spawn_two_or_four_on_rand_empty_cell() {
  int emptyCellsToCount = random_int_ranged(1, m_X * m_Y + 1);

  if(map_contains(0) == false) {
    return;
  }


  while(emptyCellsToCount != 0) {
    for (int y = 0; y < m_Y; y++)
      for (int x = 0; x <m_X; x++) {
        auto currentCell = Map[x][y];

        if (currentCell == 0) {
          emptyCellsToCount --;
        }

        if (emptyCellsToCount == 0) {
         Map[x][y] = random_int_ranged(1, 2 + 1) * 2;
         return;
        }
      }
  }

  std::cout << "looped all but empty cells didnt deduct to 0";
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

bool try_push_cells(int& currentCell, int& targetCell, int& previousCell, bool atEnd) {
  if (targetCell == 0) {
    if(atEnd) {
      swap_references(currentCell, targetCell);
      return true;
    }

    return false;
  }

  if (targetCell != currentCell) {
    swap_references(currentCell, previousCell);

    return true;
  }

  targetCell = 2 * currentCell;
  currentCell = 0;

  return true;
}


int get_highest_value_on_map() {
  int highest = 0;

  for (int x = 0; x <m_X; x++)
    for (int y = 0; y < m_Y; y++)
      if (highest < Map[x][y]) highest = Map[x][y];

  return highest;
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

