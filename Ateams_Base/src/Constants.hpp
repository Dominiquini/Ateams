#ifndef _CONSTANTS_
#define _CONSTANTS_

#if defined(_WIN64) || defined(WIN64)
  #define IS_WINDOWS true
  #define IS_LINUX false
#elif defined(__linux__) || defined(__unix__)
  #define IS_WINDOWS false
  #define IS_LINUX true
#endif

#define COLOR_BLACK "[30m"
#define COLOR_RED "[31m"
#define COLOR_GREEN "[32m"
#define COLOR_YELLOW "[33m"
#define COLOR_BLUE "[34m"
#define COLOR_MAGENTA "[35m"
#define COLOR_CYAN "[36m"
#define COLOR_WHITE "[37m"
#define COLOR_DEFAULT "[39m"

#define ASCII_CLEAR_LINE "\033[K"
#define ASCII_PREVIOUS_LINE "\033[F"

#define CHAR_BEEP '\a'
#define CHAR_BACKSPACE '\b'
#define CHAR_NEWLINE '\n'
#define CHAR_TAB '\t'
#define CHAR_CARRIAGE_RETURN '\r'

#define RANDOM_TYPE -1

#define RANDOM_THREAD_SAFE false

#define CONTROL_MANAGEMENT_UPDATE_INTERVAL 500

#define CONTROL_MAX_ERROR_SIZE 4096

#define MAX_ITERATIONS 1000000
#define MAX_POPULATION 1000
#define MAX_THREADS 512

#define HEURISTIC_SELECTION_MAX_ATTEMPTS 50

#define HEURISTIC_ALLOW_TERMINATION true

#define HEURISTIC_NAME_MAX_LENGTH 16
#define HEURISTIC_INFO_MAX_LENGTH 512

#endif
