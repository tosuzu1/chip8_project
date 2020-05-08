
#include <ncurses.h>			/* ncurses.h includes stdio.h */  
#include <string.h> 
#include <sys/types.h>

#define DISPLAY_RESOLUTION_HORIZONTAL 64
#define DISPLAY_RESOLUTION_VERTICAL 32
 
int main()
{
    //Init ncurses
    int ch;
    char str[64];
    uint64_t draw_this = 0xff3a;
    int n;
    initscr();			// Start curses mode 		
    refresh();
    cbreak();			// Line buffering disabled
    keypad(stdscr, TRUE);		// Capture special key such f1 etc. 
    noecho();           //Supress echo

    WINDOW * win = newwin(DISPLAY_RESOLUTION_VERTICAL + 2, DISPLAY_RESOLUTION_HORIZONTAL + 2, 0, 0);
    box(win, 0 , 0);
    wrefresh(win);
    wmove(win, 1, 1);
 
    waddch(win,'3');
    wmove(win, 2, 1);
    waddch(win,'#');
    n = sprintf(str,"%064B",draw_this);
    waddstr(win, str);
    wrefresh(win);
    getch();
    

    //werase(win);
    wmove(win, 2, 1);
    waddstr(win, "A really long test test test");
    box(win, 0 , 0);
    wrefresh(win);
    
    getch();
    endwin();

    return 0;
}