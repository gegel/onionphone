// Contact: <torfone@ukr.net>
// Author: Van Gegel
//
// THIS IS A FREE SOFTWARE
//
// This software is released under GNU LGPL:
//
// * LGPL 3.0 <http://www.gnu.org/licenses/lgpl.html>
//
// You're free to copy, distribute and make commercial use
// of this software under the following conditions:
//
// * You have to cite the author (and copyright owner): Van Gegel
// * You have to provide a link to the author's Homepage: <http://torfone.org/>
//
///////////////////////////////////////////////

// KEY codes and ESC-sequences for Windows and Linux
#ifdef _WIN32

#define KEY_INS 1
#define KEY_DEL 2
#define KEY_BREAK 127
#define KEY_UP 4
#define KEY_DOWN 5
#define KEY_RIGHT 6
#define KEY_LEFT 7
#define KEY_STAB 3
#define KEY_TAB 9
#define KEY_ENTER 13
#define KEY_ESC 27
#define KEY_BACK 8

#define EKEY_UP 0x00001B48
#define EKEY_DOWN 0x00001B50
#define EKEY_RIGHT 0x00001B4D
#define EKEY_LEFT 0x00001B4B
#define EKEY_STAB 0x00001B94
#define EKEY_INS 0x00001B52
#define EKEY_DEL 0x00001B53

#else //Linux

#define KEY_INS 1
#define KEY_DEL 2
#define KEY_BREAK 3
#define KEY_UP 4
#define KEY_DOWN 5
#define KEY_RIGHT 6
#define KEY_LEFT 7
#define KEY_STAB 8
#define KEY_TAB 9
#define KEY_ENTER 13
#define KEY_ESC 27
#define KEY_BACK 127

#define EKEY_UP 0x001B5B41
#define EKEY_DOWN 0x001B5B42
#define EKEY_RIGHT 0x001B5B43
#define EKEY_LEFT 0x001B5B44
#define EKEY_STAB 0x001B5B5A
#define EKEY_INS 0x1B5B327E
#define EKEY_DEL 0x1B5B337E

#endif


int  parseconf(char* param);
void parsecmdline(int argc, char **argv);
void doclr(void);
int docmd(char* s);
void loadmenu(void);
int doaddr(void);
void showaddr(void);
void doptt(int c);
void sendkey(char* keyname);
void dochat(void);
int dolist(int c);
void domenu(int c);
int goesc(int cc);
int gochar(int c);
int parsecmd(void);
int do_char(void);
void tty_rawmode(void);
void tty_normode(void);
void setcmd(char* cmdstr);
void web_printf(char* s, ...);
/*
#define PPP(x,...)  \
    char* bp=(char*)malloc(256);  \
    sprintf(bp, __VA_ARGS__);   \
    printf(bp);  \
    free(bp);
*/

