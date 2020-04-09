#include <ncurses.h>

int main() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    return 1;
}