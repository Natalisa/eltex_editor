#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <curses.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void sig_winch(int signo){
  struct winsize size;
  ioctl(fileno(stdout), TIOCGWINSZ, (char*) &size);
  resizeterm(size.ws_row, size.ws_col);
}

int main(int argc, char **argv){
  WINDOW *wnd;
  WINDOW *textwnd;
  WINDOW *menuwnd;
  initscr();
  signal(SIGWINCH, sig_winch);
  cbreak();
  curs_set(1);
  keypad(stdscr,1);
  refresh();
  struct winsize size;
  ioctl(fileno(stdout), TIOCGWINSZ, (char*) &size);
  wnd = newwin( size.ws_row, size.ws_col, 0, 0);

  //Нижнее меню
  menuwnd = derwin(wnd, 3, size.ws_col, size.ws_row-3, 0);
  // wbkgd(menuwnd,COLOR_PAIR(0));
  wmove(menuwnd,1,1);
  wprintw(menuwnd, "Save ^O ");
  wprintw(menuwnd, "Load ^L ");
  wprintw(menuwnd, "Exit ^X");
  box(menuwnd,'|','-');

  //Рабочее поле
  textwnd = derwin(wnd, size.ws_row-3, size.ws_col, 0, 0);
  // box(textwnd,'|','-');
  scrollok(textwnd,true);

  wrefresh(wnd);
  wrefresh(menuwnd);
  int temp, part = 1;
  int pos = 0, col = 0, row = 0;
  char *buf = (char*)malloc(sizeof(char)*255);
  noecho();//отключает вывод символа при вводе
  if(argc == 2){//загрузка при входе если выбран файл
    FILE* file;
    struct stat status;
    file = fopen(argv[1],"rb");
    stat(argv[1],&status);
    buf = realloc (buf, sizeof(char)*status.st_size);
    pos = status.st_size;
    part = (int)status.st_size/255;
    fread(buf,1,status.st_size,file);
    fclose(file);
    wmove(textwnd,0,0);
    wprintw(textwnd,"%s",buf);
    wrefresh(textwnd);
  }
  while(true){
    temp = (int)getch();
     printf("%d ",temp);
    switch (temp){
      case KEY_LEFT: {
        col--;
        wmove(textwnd,row,col);
        wrefresh(textwnd);
      }; break;

      case KEY_RIGHT: {
        col++;
        wmove(textwnd,row,col);
        wrefresh(textwnd);
      }; break;
      case KEY_UP: {
        if(row>0)
          row--;
        wmove(textwnd,row,col);
        wrefresh(textwnd);
      }; break;
      case KEY_DOWN: {
        row++;
        wmove(textwnd,row,col);
        wrefresh(textwnd);
      }; break;

      case 127: {//Удаление символа
        pos--;
        buf[pos] = ' ';
        col--;
        wmove(textwnd,row,col);
        wprintw(textwnd,"%c",buf[pos]);
        wrefresh(textwnd);
      }; break;

      case 15: {//CTRL+O сохранение
        FILE* file;
        if(argc == 1)
          file = fopen("./temp.bin","wb");
        else
          file = fopen(argv[1],"wb");
        // fprintf(file,"%s",buf);
        fwrite(buf,1,pos,file);
        fclose(file);
      }; break;

      case 12: {//CTRL+L загрузка
        FILE* file;
        struct stat status;
        if(argc == 1){
          file = fopen("temp.bin","rb");
          stat("temp.bin",&status);
        } else {
          file = fopen(argv[1],"rb");
          stat(argv[1],&status);
        }
        buf = realloc (buf, sizeof(char)*status.st_size);
        pos = status.st_size;
        part = (int)status.st_size/255;
        fread(buf,1,status.st_size,file);
        fclose(file);
        wmove(textwnd,0,0);
        wprintw(textwnd,"%s",buf);
        wrefresh(textwnd);
      }; break;

      case 10: {//ENTER
        row++;
        wmove(textwnd,row,0);
        buf[pos] = (char)temp;
        pos++;
        col = 0;
        if(row >= size.ws_row){
          wscrl(textwnd,1);
        }
      }; break;

      case 24: {//CTRL+X
        delwin(menuwnd);
        delwin(wnd);
        move(9, 0);
        refresh();
        endwin();
        free(buf);
        //exit(EXIT_SUCCESS);
        return 0;
      }; break;

      default: {
        if(pos>=255*part){//Довыделение памяти
          part++;
          buf = realloc (buf, sizeof(char)*255*part);
        }
        buf[pos] = (char)temp;
        if(col>=size.ws_col){//Перевод строки
          row++;
          col = 0;
        }
        wmove(textwnd,row,col);
        wprintw(textwnd,"%c",buf[pos]);
        wrefresh(textwnd);
        pos++;
        col++;
      }; break;
    }
  }
  return 0;
}
