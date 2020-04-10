
#include <ncurses.h>			/* ncurses.h includes stdio.h */  
#include <string.h> 

#define DISPLAY_RESOLUTION_HORIZONTAL 64
#define DISPLAY_RESOLUTION_VERTICAL 32
 
int main()
{
    //Init ncurses
    int ch;
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
    waddch(win,'#');
    waddstr(win, "THIS IS S TEXT");
    wrefresh(win);
    getch();

    werase(win);
    wrefresh(win);
    
    getch();
    endwin();

    return 0;
}