/*
 * term-colors.h 
 */


#ifndef _TERM_COLORS_H_
#define _TERM_COLORS_H_
        
/* Definition for attributes */
#define ATTR_DULL	        "0"
#define ATTR_BRIGHT	        "1"
#define ATTR_DIM            "2"
#define ATTR_UNDERLINE      "3"
#define ATTR_BLINK          "4"
#define ATTR_REVERSE        "7"
#define ATTR_HIDDEN         "8"

/* Definition for colors */
#define FG_BLACK            "30"
#define FG_RED              "31"
#define FG_GREEN            "32"
#define FG_YELLOW           "33"
#define FG_BLUE             "34"
#define FG_VIOLET           "35"
#define FG_CYAN             "36"
#define FG_WHITE            "37"
#define BG_BLACK            "40"
#define BG_RED              "41"
#define BG_GREEN            "42"
#define BG_YELLOW           "43"
#define BG_BLUE             "44"
#define BG_VIOLET           "45"
#define BG_CYAN             "46"
#define BG_WHITE            "47"

/* Escape String Macros */
#define ESC                 "\033"
#define NORMAL              ESC "[0m"
#define ESC_FG(attr, fg) \
        ESC "[" attr ";" fg "m"
#define ESC_FG_BG(attr, fg, bg) \
        ESC "[" attr ";" fg ";" bg "m"

/* Shortcuts for Colored Text ( Bright and FG Only ) */
#define BLACK               ESC_FG(ATTR_DULL, FG_BLACK)
#define RED                 ESC_FG(ATTR_DULL, FG_RED)
#define GREEN               ESC_FG(ATTR_DULL, FG_GREEN)
#define YELLOW              ESC_FG(ATTR_DULL, FG_YELLOW)
#define BLUE                ESC_FG(ATTR_DULL, FG_BLUE)
#define VIOLET              ESC_FG(ATTR_DULL, FG_VIOLET)
#define CYAN                ESC_FG(ATTR_DULL, FG_CYAN)
#define WHITE               ESC_FG(ATTR_DULL, FG_WHITE)
#define BRIGHT_BLACK        ESC_FG(ATTR_BRIGHT, FG_BLACK)
#define BRIGHT_RED          ESC_FG(ATTR_BRIGHT, FG_RED)
#define BRIGHT_GREEN        ESC_FG(ATTR_BRIGHT, FG_GREEN)
#define BRIGHT_YELLOW       ESC_FG(ATTR_BRIGHT, FG_YELLOW)
#define BRIGHT_BLUE         ESC_FG(ATTR_BRIGHT, FG_BLUE)
#define BRIGHT_VIOLET       ESC_FG(ATTR_BRIGHT, FG_VIOLET)
#define BRIGHT_CYAN         ESC_FG(ATTR_BRIGHT, FG_CYAN)
#define BRIGHT_WHITE        ESC_FG(ATTR_BRIGHT, FG_WHITE)

/* 
 * Macro for coloring some text. Color must be or a string build with 
 * ESC_FG() or ESC_FG_BG() or a shortcut.
 *
 * Example: 1. Specifying only attribute and foreground color
 *              COLOR("Hello", ESC_FG(ATTR_UNDERLINE, FG_BLACK))
 *          2. Specifying attribute, foreground and background
 *              COLOR("Hello", ESC_FG_BG(ATTR_BRIGHT, FG_CYAN, BG_VIOLET))
 *          3. Using a shortcut
 *              COLOR("Hello", BRIGHT_YELLOW)
 */
#define COLOR(text, color)       \
    color text NORMAL


#endif /* _TERM_COLORS_H_ */

/* vim: set expandtab ts=4 sw=4 tw=80: */
