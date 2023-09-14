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

// 阅读事项：
//   1. 阅读别人的代码时，记得要执行，改一改，玩一玩，看看会发生什么，而不是一直看着代码
//   2. 我的代码习惯总是把重要的放在最上面(除非是一些必要的声明条件，比如前向声明)，所以大部分实现细节都在文件最底下
//   3. 我习惯把大的代码分成一小块一小块，模块化的函数，这样有利于不同层面上快速思考，而不会因为信息过载而头脑CPU过载（哈哈哈）
//   4. 我所有东西的命名都是有意义的，比如display_map（显示地图）一定只会做一件事情，那就是显示地图，不多不少，没有那种奇怪的副作用
//   5. 我这个代码不关心速度，不关心效率，只关心我的可读性与实际作用，对我来说，速度与效率只有在真的有明显需求时，才去研究，否则就是浪费时间涅
//   6. 

int Map[m_X][m_Y] = {0};

// 玩家允许的输入
const int valid_inputs[] = {'a', 'd', 'w', 's', 'q'};

// Forward Declaration (前向声明)

// ==== 主要函数 ==== //
void display_map();
void try_push_map_cells(char input);
void spawn_two_or_four_on_rand_empty_cell();

bool try_push_cells(int& currentCell, int& targetCell,
    int& previousCell, bool atEnd);

// ==== Utilities 工具函数 ==== //
template<typename T>
void swap_references(T& a, T& b);

bool is_valid_input(char character);
int get_highest_value_on_map();

bool has_valid_moves();
bool map_contains(int value);

// ==== 控制台管理 ==== //
void clear_screen();
void position_cursor_to_home();

// ==== 简单问问题函数 ==== //
bool prompt_question(const char* question);


// 用于控制台开启raw模式， 关闭canon模式（意思就是我一按，控制台马上反应）
// 只能够应用于Linux
#ifdef __linux__
struct termios orig_termios;

// 关闭raw模式
void disable_raw_mode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

// 开启raw模式
void enable_raw_mode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disable_raw_mode); // 程序结束时，的回调函数 (大白话：事件)
  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ECHO | ICANON);

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
#endif

int main(void) {
  // 使用当前时间来当成cpp 随机数字生成器的种子
  // 注: 如果这一行删掉，就会等同于std::srand(1);
  //  意味着每一次都是同一个随机数列
  std::srand(time(NULL));

#ifdef __linux__
  enable_raw_mode();
#endif 

  spawn_two_or_four_on_rand_empty_cell();

  std::cout << "== 2048  Game ==\n";

  std::cout << "High score: " << get_highest_value_on_map() << '\n';

  display_map();

  while (true) {
    // 这里用std::tolower让输入的char统统变成小写
    // 好处在于WASD 会被理解为wasd
    char lowerCasedKeyboardInput = std::tolower(getchar());

    // 检测是否是有意义的输入, 否则压根不做任何事情
    // 这里我用 == false, 而不是 !(xxx) 因为可读性更高
    // 也不会影响性能，何乐而不为？
    if (is_valid_input(lowerCasedKeyboardInput) == false) {
      continue;
    }

    // 控制台清理干净, 并且把控制台的输出指针放到第一行第一列
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
    // 注意：x从左到右遍历
    // 但i， 是从右到左推
    // 这个逻辑很重要, 遍历要相反，否则会出现某些数字被提前推但没有推完整的结果
    for (int y = 0; y < m_Y; y++)
      for (int x = 0; x <m_X; x++) {
        // 如果你不知道这个int&什么作用
        // 这个类型后面加&的意思是引用
        // 类似于指针，因为是直接访问Map[x][y]修改的
        // 这句话可以理解成 int* currentCellPtr = &Map[x][y];
        // 然后当你使用时：
        // currentCell = 1;
        // 等同于上面指针的:
        // (*currentCellPtr) = 1;
        // 也可以理解为:
        // Map[x][y] = 1;
        // 通过“引用”变量直接修改Map[x][y]的内容
        int& currentCell = Map[x][y];

        // 如果当前想要推的数字是空位，那没有意义，空位本来就不用推，跳过
        if (currentCell == 0) continue;

        // push current cell to the left side if there is still space
        // merge by multiplying two if the pairs match
        // stop if we cant merge
        // i往x遍历的反方向去遍历（这很重要）
        for (int i = x - 1; i >= 0; i--) {
          int& targetCell = Map[i][y];
          int& previousCell = Map[i + 1][y];

          auto atEnd = (i == 0);

          // 如果你想明白我这个的原理是什么，我写的比较抽象昂
          // 这里try_push_cells理解为尝试去推一次，当前的数字，然后只关心当前的搜索与当前的数字
          // try_push_cells不会去搜索，但是会告诉你，推成功了没有，如果没有，那么会返回false
          // 这样我们就可以让关于i的for循环继续遍历，往左搜索，看看有没有适合能够推的位置
          auto pushingFinished =
            try_push_cells(currentCell, targetCell,
                previousCell, atEnd);

          // 如果推成功了，那么直接跳出往左搜寻的i循环遍历，没必要搜寻了，推完了涅
          if(pushingFinished) {
            break;
          }
        }
      }
  }

  // 注意：x从右到左遍历
  // 但i， 是从左到右推
  // 这个逻辑很重要, 跟a输入的逻辑一样
  if (input == 'd') {
    for (int y = 0; y < m_Y; y++)
      for (int x = m_X - 1; x >= 0; x--) {
        // 你会发现逻辑其实跟a差不多，都是try_push_cells函数的功劳哈哈哈
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
  
  // 注意：y也是与a和d相同，从上往下遍历
  // 而i，我说过了，要相反，从下往上遍历（因为要往上推）
  if (input == 'w') {
    for (int x = 0; x <m_X; x++)
      for (int y = 0; y < m_Y; y++) {
        auto& currentCell = Map[x][y];

        if (currentCell == 0) continue;

        // push current cell to the left side if there is still space
        // merge by multiplying two if the pairs match
        // stop if we cant merge
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

  // 这里我就不多说了，看a
  if (input == 's') {
    for (int x = 0; x <m_X; x++)
      for (int y = m_Y - 1; y >= 0; y--) {
        auto& currentCell = Map[x][y];

        if (currentCell == 0) continue;

        // push current cell to the left side if there is still space
        // merge by multiplying two if the pairs match
        // stop if we cant merge
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

// 用于检测整个地图玩家是否还能够移动
bool has_valid_moves() {
  // 原理很好理解， 对于每一个数字，我们看看是否能够与他的上下左右融合在一起（意思就是数值相同）
  // 但凡有一个数字能够融合，那么就代表玩家还能够移动，那么直接快速返回true
  // 但凡有一个数字0,那就代表玩家一定能够移动（因为是空位），也是直接快速返回true
  // 假如遍历完，那么就一定代表所有以上情况均没有满足，那么返回false

  for (int y = 0; y < m_Y; y++)
    for (int x = 0; x <m_X; x++)
    {
      int currentCell = Map[x][y];

      if (currentCell == 0)
        return true;

      // 这里要注意：我们对于地图边缘的格子（数字）都要关心是否他本来就没有上下左右的格子（因为都在边缘了嘛）

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

// 用于显示字体数字颜色的，windows要显示的话可能要开启虚拟控制台模式（virtual）
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

// 这个用于好看的输入，这样地图就不会因为数字而歪掉
// 例子: （以下例子均使用pad_right_ref(character, 4)）来演示
// 2 -> "2" 变成     "2   " (总字符串长度4)
// 16 -> "16" 变成   "16  " (总字符串长度4)
// 128 -> "16" 变成  "128 " (总字符串长度4)
// 2048 -> "16" 变成 "2048" (总字符串长度4)
// 通过在字符串右边添加空格，来达到一致长度
void pad_right_ref(std::string &str, const size_t num, const char paddingChar = ' ')
{
  if(num > str.size())
      str.insert(str.size(), num - str.size(), paddingChar);
}

void display_map() {
  for (int y = 0; y < m_Y; y++) {
    for (int x = 0; x <m_X; x++) {
      auto currentCell = Map[x][y];

      // 如pad_right_ref那里解释的一样， 先转换数值变成字符串
      auto cellAsString = std::to_string(currentCell);

      // 再添加需要的空格，使其对其长度
      pad_right_ref(cellAsString, 4);

      if(currentCell == 0) {
          std::cout << "\x1B[37m" << cellAsString; // 输出灰色
      }
      else {
        // log2意思就是，以2为底的log
        // 这个用处在于对于每一个2, 4, 8, 16, ..., 2048 数字，都能够获得一个1, 2, 3, 4, ..., 11 的顺序数字
        int colorSelect = (int)std::log2(currentCell);

        // 然后这个就跟上面的0前面那个奇怪的字符串一样，用于跟控制台说，这个后面要显示什么颜色
        std::cout << colorCodes[colorSelect - 1] << cellAsString;
      }

      // 这个在所有颜色后面都要加，用来重置颜色
      // 可以理解为我的世界里面的颜色双s符号，当你双s 然后加个颜色后，如果不用这个（双s 加 r）
      // 那么后面所有的字都会染成你前面设置的颜色
      std::cout << "\033[0m ";
    }

    std::cout << "\n";
  }
}

// 我就不解释这些了，短的很好理解
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

// 让随机数更好理解， 返回一个包含于区间 [inclusiveMin, exclusiveMax) 的随机整数整数
int random_int_ranged(int inclusiveMin, int exclusiveMax) {
  return std::rand() % (exclusiveMax - inclusiveMin) + inclusiveMin;
}

// 搜索算法很简单
// 先获取一个包含于 [1, m_X * m_Y] 的随机整数a
// 再通过无限遍历，递减a, 直到a等于0时，那么就用那个数到的格子放一个{2, 4}集合里随机一个数字
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
         // 这里故意写2 + 1 是为了明显的告诉代码阅读者说：2是包含在随机数里的，才加1
         Map[x][y] = random_int_ranged(1, 2 + 1) * 2;
         return;
        }
      }
  }

  std::cout << "looped all but empty cells didnt deduct to 0";
}

// 从上到下(2) 的 清理整个屏幕 (J)
void clear_screen() {
  std::cout << "\033[2J";
}

// 移动控制台输出光标到 第一行,第一列(我所谓的home的意思就是这个)
void position_cursor_to_home() {
  std::cout << "\033[1;1H";
}

// 利用引用变量的特性，去交换a与b变量的数据内容
template<typename T>
void swap_references(T& a, T& b) {
  int tempSwapBuffer = a;

  a = b;
  b = tempSwapBuffer;
}

// 大概是代码里最重要的函数了
// 逻辑就是我之前录像给你看的算法，我并没有改多少
bool try_push_cells(int& currentCell, int& targetCell, int& previousCell, bool atEnd) {
  // 如果指定搜索的数字是 0
  if (targetCell == 0) {
    // 而且搜索已经到那一行的结尾了
    if(atEnd) {
      // 那么直接替换
      swap_references(currentCell, targetCell);
      return true;
    }
    // 否则的如果不是结尾， 那么直接返回false，意思就是算法不会与0替换，而是不关心

    return false;
  }
  // 如果制定搜索的数字不是0

  // 而且当前准备想推的数字 与 搜索到的不是0的数字不相等（不能融合）
  if (targetCell != currentCell) {
    // 那么他前面一个数值(previousCell)替换
    // 注：不要误会了，我这里可没有跟targetCell替换
    swap_references(currentCell, previousCell);

    return true;
  }
  // 否则 当前准备想推的数字 与 搜索到的不是0的数字相等
  // 那么代表可以融合

  targetCell = 2 * currentCell;
  currentCell = 0;

  return true;
}

// 打字解释注释真累
// 下面的我就不解释了，贼好理解

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

