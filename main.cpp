#include <algorithm>
#include <chrono>
#include <cmath>
#include <set>
#include <string>
#include <iomanip>
#include <iostream>

#include <stdlib.h>
#include <unistd.h>

#include <thread>
#include <vector>

#include "utils/utils.h"


#ifdef __linux__
#include <termios.h>
#endif

std::vector<int> grid = std::vector<int>();

std::vector<int> spawnableNumbers = {
  2, 4
};

std::string previousAction = "none";

int mapWidth = 1;
int mapHeight = 1;

int& get_cell_ref(int x, int y) {
  return grid[y * mapWidth + x];
}

int get_cell(int x, int y) {
  return grid[y * mapWidth + x];
}

const int valid_inputs[] = {'a', 'd', 'w', 's', 'q'};

void display_map();
void try_push_map_cells(char input);
void spawn_rand_number_on_empty_cell();
void display_colorized_number(int number);

bool try_push_cells(int& currentCell, int& targetCell,
    int& previousCell, bool atEnd);

bool is_valid_input(char character);
int get_highest_value_on_map();

bool has_valid_moves();
bool map_contains(int value);


#ifdef __linux__
struct termios orig_termios;

void disable_raw_mode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disable_raw_mode);
  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ICANON);

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
#endif

bool gameEnded = false;

void game_loop();
bool push_game_once(char input);

std::set<int> historyOfNumbers = std::set<int>();

void display_header_metrics();
void display_tail_metrics();

int main(void) {
  std::srand(time(NULL));

#ifdef __linux__
  enable_raw_mode();
#endif

  std::cout << "Welcome to 2048 :P\nWhat X dimension do you want your map to be: \n";
  std::cin >> mapWidth;

  std::cout << "\nY Dimension:";
  std::cin >> mapHeight;

  grid.resize(mapWidth * mapHeight);

  spawn_rand_number_on_empty_cell();

  clear_screen();
  position_cursor_to_home();

  display_header_metrics();

  display_map();

  display_tail_metrics();

  char currentInput = 'a';

  while (gameEnded == false) {
    game_loop();
  }
}

int currentSteps = 0;

// returns true if the game ended, whether or not it failed or win
void game_loop() {
  char lowerCaseInput = std::tolower(getchar());

  if (is_valid_input(lowerCaseInput) == false) {
    return;
  }

  previousAction = lowerCaseInput;

  clear_screen();
  position_cursor_to_home();

  try_push_map_cells(lowerCaseInput);

  display_header_metrics();

  if (lowerCaseInput == 'q') {
    auto promptResult = prompt_question("do you want to quit? (Y/N):");

    if (promptResult == true) {
      std::cout << "See you soon!";

      gameEnded = true;
      return;
    }
  }
  // else
  currentSteps += 1;

  spawn_rand_number_on_empty_cell();

  display_map();

  display_tail_metrics();

  if (has_valid_moves() == false) {
    std::cout << "\nGame over!";
    gameEnded = true;
    return;
  }
}

void try_push_map_cells(char input) {
  if (input == 'a') {
    for (int y = 0; y < mapHeight; y++)
      for (int x = 0; x <mapWidth; x++) {
        int& currentCell = get_cell_ref(x, y);

        if (currentCell == 0) continue;

        for (int i = x - 1; i >= 0; i--) {
          int& targetCell = get_cell_ref(i, y);
          int& previousCell = get_cell_ref(i + 1, y);

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
    for (int y = 0; y < mapHeight; y++)
      for (int x = mapWidth - 1; x >= 0; x--) {
        int& currentCell = get_cell_ref(x, y);

        if (currentCell == 0) continue;

        for (int i = x + 1; i < mapWidth; i++) {
          auto& targetCell = get_cell_ref(i, y);
          auto& previousCell = get_cell_ref(i - 1, y);

          auto atEnd = (i == mapWidth - 1);

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
    for (int x = 0; x <mapWidth; x++)
      for (int y = 0; y < mapHeight; y++) {
        auto& currentCell = get_cell_ref(x, y);

        if (currentCell == 0) continue;

        for (int i = y - 1; i >= 0; i--) {
          auto& targetCell = get_cell_ref(x, i);
          auto& previousCell = get_cell_ref(x, i + 1);

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
    for (int x = 0; x <mapWidth; x++)
      for (int y = mapHeight - 1; y >= 0; y--) {
        auto& currentCell = get_cell_ref(x, y);

        if (currentCell == 0) continue;

        for (int i = y + 1; i < mapHeight; i++) {
          auto& targetCell = get_cell_ref(x, i);
          auto& previousCell = get_cell_ref(x, i - 1);

          auto atEnd = (i == mapHeight - 1);

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
  for (int y = 0; y < mapHeight; y++)
    for (int x = 0; x < mapWidth; x++)
    {
      int currentCell = get_cell_ref(x, y);

      if (currentCell == 0)
        return true;

      // left
      if (x - 1 >= 0 &&
          currentCell == get_cell_ref(x - 1, y))
        return true;

      // right
      if (x + 1 < mapWidth &&
          currentCell == get_cell_ref(x + 1, y))
        return true;

      // up
      if (y - 1 >= 0 &&
          currentCell == get_cell_ref(x, y - 1))
        return true;

      // down
      if (y + 1 < mapHeight &&
          currentCell == get_cell_ref(x, y + 1))
        return true;
    }

  return false;
}

const char* color_codes[] = {
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

std::string horizontalBarCache = "";

void display_horizontal_bars() {
  if (horizontalBarCache == "") {
    for(int i = 0; i < mapWidth; i++) {
      horizontalBarCache += "+-----";
    }

    horizontalBarCache += "+\n";
  }
  // then
  
  std::cout << horizontalBarCache;
}

void display_colorized_number(int number) {
  int colorSelect = (int)std::log2(number);

  std::cout << color_codes[std::clamp(colorSelect - 1, 0, 10)] << number << "\033[0m";
}

void display_map() {
  for (int y = 0; y < mapHeight; y++) {
    display_horizontal_bars();
    
    for (int x = 0; x < mapWidth; x++) {
      auto currentCell = get_cell_ref(x, y);

      auto cellAsString = std::to_string(currentCell);

      pad_right_ref(cellAsString, 4);

      std::cout << "|";

      if(currentCell == 0) {
          std::cout << "    "; // "\x1B[37m" << cellAsString; 
      }
      else {
        int colorSelect = (int)std::log2(currentCell);

        std::cout << color_codes[std::clamp(colorSelect - 1, 0, 10)] << cellAsString;
        // std::cout << cellAsString;
      }

      const char* END_OF_COLOR = "\033[0m";

      std::cout << END_OF_COLOR << ' ';
    }

    std::cout << "|\n";
  }

  display_horizontal_bars();
}

bool is_valid_input(char character) {
  for(auto valid_input : valid_inputs) {
    if (valid_input == character) return true;
  }

  return false;
}

bool map_contains(int value) {
   for (int y = 0; y < mapHeight; y++)
     for (int x = 0; x <mapWidth; x++) {
       if (get_cell_ref(x, y) == value) return true;
     }
  //else

  return false;
}

void spawn_rand_number_on_empty_cell() {
  int emptyCellsToCount = random_int_ranged(1, mapWidth * mapHeight + 1);

  if(map_contains(0) == false) {
    return;
  }


  while(emptyCellsToCount != 0) {
    for (int y = 0; y < mapHeight; y++)
      for (int x = 0; x <mapWidth; x++) {
        auto currentCell = get_cell_ref(x, y);

        if (currentCell == 0) {
          emptyCellsToCount --;
        }

        if (emptyCellsToCount == 0) {
         // get_cell(x, y) = std::pow(2, random_int_ranged(1, 2 + 1));
         // get_cell_ref(x, y) = random_int_ranged(1, 5 + 1);
          get_cell_ref(x, y) = spawnableNumbers[random_int_ranged(0, spawnableNumbers.size())];
         return;
        }
      }
  }

  std::cout << "looped all but empty cells didnt deduct to 0";
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

  for (int x = 0; x <mapWidth; x++)
    for (int y = 0; y < mapHeight; y++)
      if (highest < get_cell_ref(x, y)) highest = get_cell_ref(x, y);

  return highest;
}

void update_number_history() {
  for (const auto& value : grid) {
    // cannot find the value in the history
    if(value == 0) continue;
    // else

    if(historyOfNumbers.find(value) == historyOfNumbers.end()) {
      historyOfNumbers.emplace(value);
    }
  }
}

void display_header_metrics() {
  std::cout << "== 2048  Game ==\n";

  std::cout << "Spawnable Numbers: ";
  for (const auto& value : spawnableNumbers) {
    display_colorized_number(value);
    std::cout << ' ';
  }
  std::cout << '\n';



  std::cout << "High score: ";
  display_colorized_number(get_highest_value_on_map());
  std::cout << '\n';



  update_number_history();

  std::cout << "number history: ";

  for(const auto& value : historyOfNumbers) {
    display_colorized_number(value);
    std::cout << ' ';
  }
  std::cout << '\n';
}

void display_tail_metrics() {
  std::cout << "Previous Action: " << previousAction << " | ";
  std::cout << "Steps took: " << currentSteps << '\n';
}
