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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "crypto.h"
#include "cntrls.h"
#include "codecs.h"
#include "tcp.h"
//#include "audio.h"

#include <stdarg.h>

#define RINGTIME 30  //time in sec for wait answer
#define BOOK "contacts.txt" //default addressbook filename

//Extern Configuration
extern int sp_jit;  //jitter compensation in mS
extern int sp_npp;		//noise reduction
extern int sp_agc;		//mike auto gain
extern int sp_voc;		//vocoder mode
extern int vad_tail;		//silence transmitted (frames)
extern int vad_signal;	//signal before mute
extern int vad_level; //level of 3-tone signal (end of remote transmition) 
extern int rc_level; //trashhold for change slowest onion connection
extern char onion_flag; //flag of onion connection status
extern char crp_state; //crypto protocol state (crypto.c)
extern char vox_level; //level of voice active detection (codecs.c)
extern int bytes_received;
extern int bytes_sended;  //bytes counters (tcp.c)
extern float up_bitrate;
extern float down_bitrate; //communication bitrate (tcp.c) 
extern char sound_test; //flag of continuous jitter notification (codecs.c)

//Extern buffers
extern char book_name[32]; //filename of current addressbook
extern char command_str[256]; //command for set encryption
extern char password[32];  //preshared password

char sound_loop=0;      //flag of sound testing
int menu=0;             //menu pointer
int menuitem=0;         //menu iteam pointer
int lastchar=0;         //last inputted char
char cmdbuf[256];       //console input buffer
int cmdptr=0;           //pointer to last inputted char
char menuhdr[10][64];   //menu headers
char menustr[10][10][64]; //menu iteams

unsigned int esc_time=0; //time after asc char detected
unsigned int esc_key=0;  //chars getted after esc during some time
char old_char=0;        //last char for flush keyboard while TAB holded
char next_char=0;       //nrext char emulated by remote
#define DEFCONF "conf.txt"  //configuration filename
char confname[32]=DEFCONF; //name of current configuration flag

#ifdef _WIN32

#include <stddef.h>
 #include <stdlib.h>
 #include <basetsd.h>
 #include <stdint.h>
 #include <windows.h>
 #include <time.h>
 #include <conio.h>

 #define close fclose
 #define usleep Sleep

#else  //Linux
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <fcntl.h>
 #include <assert.h>
 #include <string.h>
 #include <ctype.h>
 #include <time.h>
 #include <sys/types.h>
 #include <sys/time.h>
 #include <sys/wait.h>
 #include <sys/stat.h>
 #include <unistd.h>
 #include <errno.h>
 #include <signal.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <netinet/tcp.h>
 #include <arpa/inet.h>
 #include <netdb.h>
 #include <termios.h>
 #include <termio.h>

 static struct termio old_term_params;
#endif



 //*****************************************************************************
 //set raw mode for terminal (Linux only)
void tty_rawmode(void)
{
#ifndef _WIN32   
   struct termio term_params;

   ioctl(fileno(stdin), TCGETA, &old_term_params);
   term_params = old_term_params;
   term_params.c_iflag &= ~(ICRNL|IXON|IXOFF);	// no cr translation 
   term_params.c_iflag &= ~(ISTRIP);   // no stripping of high order bit 
   term_params.c_oflag &= ~(OPOST);    // no output processing 	
   term_params.c_lflag &= ~(ISIG|ICANON|ECHO); // raw mode 
   term_params.c_cc[4] = 1;  // satisfy read after 1 char 
   ioctl(fileno(stdin), TCSETAF, &term_params);
   fcntl(fileno(stdin), F_SETFL, O_NDELAY);
#endif
 return;
}


//*****************************************************************************
// Restore tty mode (Linux only)
void tty_normode(void)
{
#ifndef _WIN32   
 ioctl(fileno(stdin), TCSETAF, &old_term_params);
#endif
 return;
}

//*****************************************************************************
//Parse config file for param and copy value to param, return length of value
//zero if not found and error code if no config file
int  parseconf(char* param)
{
 FILE *fpp;
 char buf[256];
 char* p=NULL;
 int i;

 //open configuration file
 fpp = fopen(confname, "rt");
 if (fpp == NULL)
 {
  perror("Cannot open config file");
  return -1;
 }
  //read it sting-by-string
  while( fgets(buf, sizeof(buf), fpp) )
  {
   if((buf[0]=='#')||(buf[0]==0)) continue; //skip comments and emty stings
   p=strstr(buf, param); //search specified parameter in string
   if(!p) continue; //process next string up to eof
   p=strchr(buf, '='); //search separator
   if(!p) continue; //no value
   p++; //set pointer to value
   break;
  }
  fclose(fpp);
  param[0]=0; //clear input string
  if(p) //if parameter found
  {  //truncate value string to first space or end of string
   for(i=0;i<(int)strlen(p);i++)
   if( (p[i]=='\r')||(p[i]=='\n')||(p[i]==' ') ) break;
   p[i]=0;
   strncpy(param, p, 31); //replace input parameter by it's value
   param[31]=0;
  }
  return (strlen(param)); //length of value's string or null
}

//parse application command line arguments
void parsecmdline(int argc, char **argv)
{
 int j;
 char str[256];
 if(argc>1) for(j=1; j<argc; j++) //for each argv
 {
  if(argv[j][0]!='-') continue; //option -Xparameter
  if(argv[j][1]=='Y') set_access(argv[j]+2, 0); //access to current secret key
  else if(argv[j][1]=='F')
  {
   strncpy(confname, argv[j]+2, 31); //config file name
   confname[31]=0;
   web_printf("Configuration file '%s' will be used\r\n", confname);
  }
 }
 memset(str, 0, 256);
}


//*****************************************************************************
//clears string
void doclr(void)
{
   int i;
   printf("\r");
   for(i=0;i<cmdptr;i++) printf(" ");
   printf("\r");
   cmdptr=0;
}

//*****************************************************************************
//process users cmd from menu iteam
int docmd(char* s)
{
 char* p;
 p=strchr(s, ':');   //search command
 //check for validity
 if((s[0]!='*')||(strlen(s)<4)||(p<(s+2)))
 {
  doclr(); //clear command string on screen and in buffer
  return 0;
 }
 p++; //poiter to command
 strncpy(cmdbuf, p, 255); //output command from menu iteam
 cmdbuf[255]=0;
 cmdptr=strlen(cmdbuf);  //set command length
 return cmdptr;     //return length
}

//*****************************************************************************
//initialize menu: load commands and items
void loadmenu(void)
{
 FILE* fl;
 char buf[256];
 int i,j;

 //parce address book filename from config or set defautl
 strcpy(buf, "AddressBook"); //parameter name
 i=parseconf(buf); //get parameter from ini file
 if((i>0)&&(i<32)) strcpy(book_name, buf); //use it if specified
 else strcpy(book_name, BOOK);  //or set default

 memset(menustr, 0, sizeof(menustr));
 fl=fopen("menu.txt", "rt"); //try to open dummy menu file
 if(!fl) return;
 while(!feof(fl))  //loads menu commands and items
 {
      fgets(buf, 66, fl); //load next string
      if(strlen(buf)>1) //skip empty strings
      {
       for(i=0;i<(int)strlen(buf);i++) if(buf[i]<28) buf[i]=0; //truncate to first unprintable char
       i=buf[0]-0x30; //menu index (0-9)
       j=buf[1]-0x30; //item index (0-9)
       if((i>=0)&&(i<11)&&(j>=0)&&(j<11)) //cifers
       {
        if(j<10) strncpy(menustr[i][j], buf+2, 63); //menu item
        else strncpy(menuhdr[i], buf+2, 63); //menu header (command)
        menustr[i][j][63]=0;
        menuhdr[i][63]=0;
       }
      }
 }
 fclose(fl); //close dummy file
}

//*****************************************************************************
//find adress by name using addressbook
int doaddr(void)
{
 FILE* fl;
 int i,k,l;
 char* p;
 char buf[256];
 if((cmdbuf[0]!='-')||(cmdbuf[1]!='E')) return 0; //marker for this operation
 printf("\r\n");
 k=0;
 if(!cmdbuf[2]) //empty name: output last used address
 {
  if(command_str[0]) //if adress already outputted before
  {
   p=command_str; //use this address
   k=1; //flag OK
  }
 }
 else
 {
  sprintf(buf, "%s%s", KEYDIR, book_name); //add path
  fl=fopen(buf, "rt"); //try to open addressbook file
  if(!fl)
  {
   web_printf("! Contact list not found\r\n");
   return 0;
  }
  l=strlen(cmdbuf+2); //length of name
  while(!feof(fl)) //process file
  {
   fgets(buf, 256, fl); //load next string
   if(buf[0]!='[') continue;
   for(i=0;i<(int)strlen(buf);i++) if(buf[i]<28) buf[i]=0; //reject to first unprintable
   if( memcmp(buf+1, cmdbuf+2, l) ) continue; //continue if name not matched pattern
   k=2; //flag for OK
   break;
  }
  fclose(fl); //close addressbook
 }

 if(!k) //no results
 {
  web_printf("Not found\r\n");
  doclr();
  return 0;
 }
 else if(k==2)
 {
  p=strchr(buf,']');
  if(p)
  {
   p[0]=0;
   p++;
  }
  else p=buf;
  p=strchr(p, '-'); //first option in book-string
  if(!p) p=buf;
  buf[0]=0;

  sprintf(cmdbuf, "-N%s %s", buf+1, p);  //use resut as a command for call
  web_printf((char*)cmdbuf); //print to screen
  cmdptr=strlen(cmdbuf); //set length: now address ready for call by press Enter
 }
 else
 {
  sprintf(cmdbuf, "%s", p);  //use resut as a command for call
  web_printf((char*)cmdbuf); //print to screen
  cmdptr=strlen(cmdbuf); //set length: now address ready for call by press Enter
 }
 return k;
}

//*****************************************************************************
//show addressbook items with fiter
void showaddr(void)
{
 FILE* fl;
 int i,k;
 char buf[256];
 if((cmdbuf[0]!='-')||(cmdbuf[1]!='V')) return; //marker for this operation
 printf("\r\n");
 sprintf(buf, "%s%s", KEYDIR, book_name); //add path
 fl=fopen(buf, "rt"); //try to open addressbook file
 if(!fl)
 {
  web_printf("! Contact list not found\r\n");
  return;
 }

 k=0;
 while(!feof(fl)) //process file
 {
  buf[0]=0;
  fgets(buf, 256, fl); //load next string
  if(buf[0]!='[') continue;
  for(i=0;i<(int)strlen(buf);i++) if(buf[i]<28) buf[i]=0; //reject to first unprintable symbol
  if(cmdbuf[2]) //if string not null
  {
   if(!strstr(buf, cmdbuf+2)) continue; //find pattern
  }
  web_printf("%s\r\n", buf); //show string with matched pattern
  k++; //counter
 }
 if(!k) web_printf("! Names not found\r\n"); //no one
 fclose(fl); //close addressbook file
 doclr(); //clear work field on screen
}

//*****************************************************************************
//change PTT mode
void doptt(int c)
{
 if(c==KEY_TAB) push_ptt();//ptt key: enable tx
 else if(c==KEY_ENTER) // on/off tx
 {
  if(crp_state==2) //answer incoming call
  {
   sound_loop=0;
   off_tx();
   do_ans(1);
   return;
  }
  else switch_tx();
 }
 else if(c==KEY_STAB) go_vad();//vad activation: shift+tab
 return;
}

//*****************************************************************************
//proceed key sharing operation while connection established
void sendkey(char* keyname)
{
 unsigned char bb[512];
 int t, l=0;
 char c;
 strncpy((char*)bb, keyname, 511); //key filename from command
 bb[511]=0;
 do
 { //now bb[0]!=0 then key loaded and process inited
  t=do_key(bb); //key paccket prepared
  if(t>0) l=do_data(bb, (unsigned char*)&c); //encrypts
  if(l>0) do_send(bb, l, c);  //sends to remote
  bb[0]=0;  //next do_key will be generates subsequent packets of this key
 }
 while(t==500); //while a not last packet
 if(t==504) web_printf("Key '%s' sent\r\n", keyname);
 else web_printf("! Error sending key\r\n");
}

//*****************************************************************************
//send chat message
void dochat(void)
{
 unsigned char bb[264];
 int l;
 char c;
 cmdbuf[255]=0;
 if(strlen(cmdbuf)<254) //chatting message length restricition
 {
  memset(bb, 0, sizeof(bb)); //clear
  bb[0]=TYPE_CHAT|0x80;    //set chat message header
  strcpy((char*)(bb+1), cmdbuf); //message body
  l=do_data(bb, (unsigned char*)&c);     //encrypt
  if(l>0) do_send(bb, l, c);  //send to remote
  printf("\r<%s\r\n", cmdbuf); //otput to local terminal
 }
 doclr();  //clear command string
}

//*****************************************************************************
//output next name from adressbook as a menu iteam
int dolist(int c)
{
 char* p;
 int n, i, j;
 FILE* fl;
 char buf[256];
 //check for key is left or right
 if((c!=KEY_LEFT)&&(c!=KEY_RIGHT)) return 0;
 //check for menu iteam
 if((!cmdptr)||(cmdbuf[0]!='*')) return 0;
 //search number of current line in adressbook
 p=strchr(cmdbuf, '#');
 if(p) n=atoi(++p); else n=-1;
 //use next line (up/down)
 if(c!=KEY_RIGHT) n--; else n++;
 if(n<0) n=0;
 //load name from book by line pointer
 sprintf(buf, "%s%s", KEYDIR, book_name); //add path
 fl=fopen(buf, "rt"); //try to open addressbook file
 if(!fl)
 {
  web_printf("! Contact list not found\r\n");
  return 0;
 }
 i=-1; //line counter
 j=n; //line to search
 while(!feof(fl)) //process file
 {
  buf[0]=0;
  fgets(buf, 256, fl); //load next string
  if(buf[0]!='[') continue; //skip comments and emty strings
  i++; //from 0
  if(i<j) continue; //process up to poiter
  p=strchr(buf, ']');  //search name
  if(!p) break;
  p[0]=0; //seprate name
  i=-1; //set flag: OK
  break;
 }
 fclose(fl); //close adressbook file
 if(i<0)
 {
  buf[0]='E'; //if OK, output -E connamd
  n=j; //set current line number for next
 }
 else
 {
  buf[0]='V'; //not found: output -V command
  buf[1]=0;
 }

 doclr(); //clear field

 //output menu iteam + command
 sprintf((char*)cmdbuf, "%s/use#%d:-%s", menuhdr[menu], n, buf);
 cmdptr=strlen((char*)cmdbuf); //set buffer pointer to end of string
 web_printf((char*)cmdbuf); //add menu string to screen
 return 1;
}


//*****************************************************************************
//menu navigatiomn
void domenu(int c)
{
 //check for menu 9: addressbook navigation
 if(menu==9)
 {
  if(dolist(c)) return; //proceed adressbook navigation
 }

 //change menu and item
 if((lastchar==KEY_UP)||(lastchar==KEY_DOWN)||(lastchar==KEY_LEFT)||(lastchar==KEY_RIGHT)) //first pressing not change menu/iteam, recall only
 {
  if(c==KEY_DOWN) menu++;
  else if(c==KEY_UP) menu--;
  else if(c==KEY_LEFT) menuitem--;
  else if(c==KEY_RIGHT) menuitem++;
 }

 if((c==KEY_UP)||(c==KEY_DOWN)) menuitem=0; //new menu always started from iteam 0

 //check menu and item
 if(menu<0)
 {
  menu=9; //roll menus down
  while( (!menuhdr[menu][0]) && menu ) menu--; //skip emty menus
 }
 if(menuitem<0) //roll items left
 {
  menuitem=9;
  while( (!menustr[menu][menuitem][0]) && menuitem ) menuitem--; //skip emty items
 }
 if((!menuhdr[menu][0])||(menu>9)) menu=0; //roll menus up
 if((!menustr[menu][menuitem][0])||(menuitem>9)) menuitem=0; //roll items rigth

 //print item
 doclr(); //clear field
 sprintf((char*)cmdbuf, menuhdr[menu]); //add menu string to buffer
 sprintf((char*)cmdbuf+strlen(cmdbuf), menustr[menu][menuitem]); //add iteam to buffer
 cmdptr=strlen((char*)cmdbuf); //set buffer pointer to end of string
 web_printf((char*)cmdbuf); //add menu string to screen
}

//*****************************************************************************
//look for ESC-sequence (control keys) and returns input char, null or ctrl code(1-8)
int goesc(int cc)
{
 unsigned int ii;

 if(cc==KEY_ESC)  //if esc char detected
 {
  if(crp_state==2) //reject incoming call
  {
   do_ans(0);
   return 0;
  }
  ii=getmsec(); //time now
  if(ii>esc_time) //if elapsed more then 1000 ms after last char
  {
   esc_time=ii+256; //set next time 1 sec in future
   esc_key=KEY_ESC; //init ctrl sequence
   cc=0; //no current char
  } //else current char is esc
  else esc_key=0; //break esc sequence and return esc
 }
 else if(cc)//other char detected
 {
  if(esc_key) //already existed some sequence
  {
   ii=getmsec(); //time now
   if(ii>esc_time) esc_key=0; //if elapsed more then 1000 ms after last char: reset sequence
   else //less then 1000 mS after last char: add char to sequence
   {
    esc_key=(esc_key<<8)+(unsigned char)cc; //add char
    ii=0; //clears for ctrl code
    
    switch(esc_key) //check for compleet sequence  and use ctrl code
    {
     case EKEY_UP:
     {
      ii=KEY_UP;
     }
     break;
     case EKEY_DOWN:
     {
      ii=KEY_DOWN;
     }
     break;
     case EKEY_RIGHT:
     {
      ii=KEY_RIGHT;
     }
     break;
     case EKEY_LEFT:
     {
      ii=KEY_LEFT;
     }
     break;
     case EKEY_STAB:
     {
      ii=KEY_STAB;
     }
     break;
     case EKEY_INS:
     {
      ii=KEY_INS;
     }
     break;
     case EKEY_DEL:
     {
      ii=KEY_DEL;
     }
     break;
    }

    if(ii) //if compleet sequence matched
    {
     cc=ii; //return resulting ctrl code
     esc_key=0;
     esc_time=0; //reset sequence
    }
    else cc=0; //sequence in process: no chars returns yes

   }
  }
 }
 return cc; //resulting char or ctrl code
}


//*****************************************************************************
//process command typed in terminal
int parsecmd(void)
{
 int i = 0;
 char str[256];
 char c;

 if(cmdbuf[0]!='-') return 0;
 printf("\r\n");

 //search for other options
 if(cmdbuf[1]=='N') //connect
 {
  if(!cmdbuf[2]) //clear last call command including secret data in it
  {
   memset(command_str, 0, 256); //clear command string
   web_printf("Call history cleared\r\n"); 
  }
  else
  {
   strncpy(command_str, cmdbuf, 255);
   command_str[255]=0;
   sound_loop=0;
   do_connect(command_str);
  }
  return 0;
 }
 else if(cmdbuf[1]=='O') //-O: swith back to TCP (addr must be empty)
 {
  if(!cmdbuf[2] && onion_flag)
  {
   stopudp(); //disable udp socket
   dochat();  //send "-O" invite for disabling udp socket on remote side
  }
  else
  {
   //emulate command -N -Oaddr -I
   sound_loop=0;
   sprintf(command_str, "-N -I %s", cmdbuf);
   do_connect(command_str);
   return 0;
  }
 }
 else if((cmdbuf[1]=='U')&&cmdbuf[2])
 {
  //emulate command -N -Uaddr -I
   sound_loop=0;
   sprintf(command_str, "-N -I %s", cmdbuf);
   do_connect(command_str);
   return 0;
 }
 else if((cmdbuf[1]=='T')&&cmdbuf[2])
 {
  //emulate command -N -Taddr -I
   sound_loop=0;
   sprintf(command_str, "-N -I %s", cmdbuf);
   do_connect(command_str);
   return 0;
 }
 else if(cmdbuf[1]=='R') //tests
 {
  if(!cmdbuf[2])
  {
   if(sound_loop)
   {
    //soundrec(0);
    sound_loop=0;
    sound_test=0;
    web_printf("Voice test canceled\r\n");
   }
  }
  else if(cmdbuf[2]=='V') //voice test
  {
   //soundrec(1);
   sound_loop=1;
   web_printf("Voice test running\r\n");
  }
  else if(cmdbuf[2]=='I')
  {
   set_encoder(0);
   get_decoder(0);
   if(vox_level) web_printf("VOX level is %d%\r\n", vox_level);
   else web_printf("VOX uses SPEEX VAD detector\r\n");
   web_printf("3tone signal level is %d%\r\n", vad_level*10/128);
   web_printf("Noise signal level is %d%\r\n", vad_signal*10);
   if(sp_jit>0) web_printf("Current Jitter compensation=%d mS\r\n", sp_jit/8);
   else if(!sp_jit) web_printf("Current Jitter compensation is automatic\r\n");
   else web_printf("Jitter buffer not used\r\n");
   get_jitter();
   web_printf("Current AGC=%d, NPP=%d, VOC=%d\r\n", sp_agc, sp_npp, sp_voc);
   web_printf("Vocoder in ");
   if(!sp_voc) web_printf("inactive");
   else if(sp_voc==1) web_printf("unvoiced");
   else if(sp_voc==2) web_printf("hight");
   else if(sp_voc==3) web_printf("deep");
   else web_printf("robot %d", sp_voc+2);
   web_printf(" mode \r\n");
   if(rc_level>0) web_printf("Current Onion doubling change interval is %d Sec\r\n", rc_level);
   else web_printf("Onion doubling is disabling now\r\n");
   get_names();
   i=get_sockstatus();

   if(!crp_state) web_printf("No connection now\r\n");
   else
   {
    if(crp_state&1) web_printf("Outgoing ");
    else web_printf("Incoming ");
    web_printf(" connection ");
    if(crp_state>2) web_printf("established");
    else web_printf("initialized");
    if(onion_flag) web_printf(" over Tor");
    else if((i==1)||(i==2)) web_printf(" over UDP");
    else web_printf(" over TCP");
    if(onion_flag>1) web_printf(" (onion verified OK)");
    if((i&4)&&(i&8)) web_printf(" Doubling active now");
    if((i&0xC)&&(i&2)) web_printf(" Switched to UDP");
    printf("\r\n");
   }
   web_printf("Up: %0.3f MB (%0.1f kbit/s). Down: %0.3f MB (%0.1f kbit/s)\r\n",
             (float)bytes_sended/1000000, up_bitrate,
             (float)bytes_received/1000000, down_bitrate);
  }
  else if(cmdbuf[2]=='L')
  {//measure latency
   if(crp_state>2) //if connection established now
   {
    str[0]=1; //syn request must be generates
    i=do_syn((unsigned char*)str);
    c=str[0]; //udp=tcp
    if(i>0) if(i>0) do_send((unsigned char*)str, 9, c); //send ping  currently used channel
    web_printf("Ping remote...\r\n");
   }
  }
  else if(cmdbuf[2]=='S')
  { //stun request over udp listener for info about it's external interface
    do_stun(cmdbuf+3);
  }
  //enable notification of bufferig status (debug mode)
  else if(cmdbuf[2]=='B') sound_test=1;
  //VAD mode
  else if(cmdbuf[2]=='A') go_vad();
  //voice transmission control
  else if(cmdbuf[2]=='T') switch_tx(); //off
  else if(cmdbuf[2]=='0')
  {
   push_ptt();  //temporary on
   switch_tx(); //continiosly off
  }
  else if(cmdbuf[2]=='1') //on
  {
   //soundrec(1); //run audio input
   off_tx(); //temporary off
   switch_tx(); //continiosly on  
  }
 }
 else if(cmdbuf[1]=='V') showaddr(); //view addrressbook
 else if(cmdbuf[1]=='E') return (doaddr()); //convert nick to address using addressbook
 else if(cmdbuf[1]=='X') return -32767; //-C terminate call
 else if(cmdbuf[1]=='H')
 {
  if((crp_state!=2)||(cmdbuf[2]=='X')) disconnect(); //-H: hung up
  else do_ans(0); //gracefully reject incoming call waiting for answer
 }
 else if(cmdbuf[1]=='P') //-Ppass: apply passphrase (if ommited, clear passphrase)
 {
  if(strlen(cmdbuf+2)<32)
  {
   memset(password, 0, 32);
   strncpy(password, cmdbuf+2, 31);
   password[31]=0;
   if(!password[0]) web_printf("Password cleared\r\n");
   else if(crp_state<3) web_printf("Store password: '%s'\r\n", password);
   else
   { //send au1
    str[0]=0;
    i=do_au((unsigned char*)str);
    if(i>0) i=do_data((unsigned char*)str, (unsigned char*)&c);
    if(i>0) do_send((unsigned char*)str, i, c);
   }
  }
 }
 else if(cmdbuf[1]=='Y') //-Ysecret_key_access_
 {
  if(cmdbuf[2]==' ') //clear access
  {
   set_access("", 0);
  }
  else if(!cmdbuf[2]) //check access
  {
   check_access();
  }
  else set_access(cmdbuf+2, 0);  //set access
 }
 else if(cmdbuf[1]=='C') //-Ccodec_num: apply encoder
 {
  if(cmdbuf[2])
  {
   i=atoi(cmdbuf+2);
   if((i>0)&&(i<19)) set_encoder(i);
   else if(cmdbuf[2]=='I') get_decoder(0);
   else
   {
    i=set_encoder(0);
    web_printf("Coder number is %d. Avaliable are 1-18\r\n", i);
   }
  }
  else
  {
   //apply codec from profile
   strcpy(str, "VoiceCodec");
   if(parseconf(str)>0) i=atoi(str);
   if((i<=0)&&(i>18)) i=7;
   set_encoder(i);
  }
 }                     
else if(cmdbuf[1]=='G') //-Gvox  set vox level (0-100%)
 {
  if(cmdbuf[2])
  {
   vox_level=atoi(cmdbuf+2);
   if(vox_level>100) vox_level=100;
   if(vox_level<=0)
   {
    vox_level=0;
    web_printf("Voice active detector enabled\r\n", vox_level);
   }
   else web_printf("Vox level set to %d%\r\n", vox_level);
  }
  else if(vox_level) web_printf("Vox level is %d%\r\n", vox_level);
  else web_printf("Vox uses voice active detector\r\n", vox_level);
 }
 else if(cmdbuf[1]=='M') //-Mvad signal level (0-9)
 {
  if(!cmdbuf[2])
  {
   vad_signal=0;
   web_printf("Noise signal disabled \r\n");
  }
  else if(cmdbuf[2]=='?')
  {
   web_printf("3tone signal level is %d%\r\n", vad_level*10/128);
   web_printf("Noise signal level is %d%\r\n", vad_signal*10);
  }
  else
  {
   if((cmdbuf[2]>=0x30)&&(cmdbuf[2]<=0x39))
   {
    vad_level=128*(cmdbuf[2]-0x30);
    web_printf("3tone signal level set to %d%\r\n", vad_level*10/128);
   }
   if((cmdbuf[3]>=0x30)&&(cmdbuf[3]<=0x39))
   {
    vad_signal=cmdbuf[3]-0x30;
    web_printf("Noise signal level set to %d%\r\n", vad_signal*10);
   }
  }
 }
 else if(cmdbuf[1]=='J') //apply jitter compensation value
 {
  if(!cmdbuf[2])
  {
   if(sp_jit) web_printf("Jitter compensation set to auto\r\n");
   sp_jit=0;
  }
  else
  {
   i=atoi(cmdbuf+2);
   if(i==-1)
   {
    sp_jit=-1;
    web_printf("Jitter buffer disabled!\r\n");
   }
   else if(i)
   {
    sp_jit=i*8;
    web_printf("Apply Jitter compensation %d mS\r\n", sp_jit/8);
   }
   else
   {
    if(sp_jit>0) web_printf("Current Jitter compensation=%d mS\r\n", sp_jit/8);
    else if(!sp_jit) web_printf("Curren Jitter compensation is auto\r\n");
    else web_printf("Jitter buffer not used\r\n");
   }
  }
 }
 else if(cmdbuf[1]=='Q') //apply voice processing parameters
 {
  //agc, npp, vocoder
  //no parameter: both agc and npp on, vocoder unchanged
  //0-both off, vocoder unchanged, 1-agc on, 2-npp on, >2 - vocoder on
  //-1-agc off, -2-npp off, <-2-vocoder off, ?-status info
  i=0;
  if(!cmdbuf[2])
  {
   if(!sp_agc)
   {
    web_printf("Automatic gain control activated!\r\n");
    sp_agc=1;
   }
   if(!sp_npp)
   {
    web_printf("Noise supressing activated!\r\n");
    sp_npp=1;
   }
   speex_p(sp_npp,sp_agc); //set denoise and agc
  }
  else if(cmdbuf[2]=='?')
  {
   web_printf("Current AGC=%d, NPP=%d, VOC=%d\r\n", sp_agc, sp_npp, sp_voc);
  }
  else if(cmdbuf[2]=='0')
  {
   if(sp_agc)
   {
    web_printf("Automatic gain control deactivated!\r\n");
    sp_agc=0;
   }
   if(sp_npp)
   {
    web_printf("Noise supressing deactivated!\r\n");
    sp_npp=0;
   }
   speex_p(sp_npp,sp_agc); //set denoise and agc
  }
  else i=atoi(cmdbuf+2);

  if((i==1)&&(!sp_agc))
  {
   web_printf("Automatic gain control activated!\r\n");
   sp_agc=1;
   speex_p(sp_npp,sp_agc); //set denoise and agc
  }
  else if((i==2)&&(!sp_npp))
  {
   web_printf("Noise supressing activated!\r\n");
   sp_npp=1;
   speex_p(sp_npp,sp_agc); //set denoise and agc
  }
  else if((i==-1)&&(sp_agc))
  {
   web_printf("Automatic gain control deactivated!\r\n");
   sp_agc=0;
   speex_p(sp_npp,sp_agc); //set denoise and agc
  }
  else if((i==-2)&&(sp_npp))
  {
   web_printf("Noise supressing deactivated!\r\n");
   sp_npp=0;
   speex_p(sp_npp,sp_agc); //set denoise and agc
  }
  else if(i<-2)
  {
   web_printf("Vocoder deactivated!\r\n");
   sp_voc=0;
  }
  else if((i>2)&&(i<255))
  {
   sp_voc=i-2;
   web_printf("Vocoder activated: ");
   if(sp_voc==1) web_printf("unvoiced");
   else if(sp_voc==2) web_printf("hight");
   else if(sp_voc==3) web_printf("deep");
   else web_printf("robot %d", sp_voc+2);
   web_printf(" mode \r\n");
  }
 }
 else if(cmdbuf[1]=='F') //load specified profile and apply all parameters from it
 {
  //if no files specified load initing profile
  setaudio();
 }
 else if(cmdbuf[1]=='W') //-T[double_interval]  in sec
 {
  //set doubling enabling and specify change interval
  if((!cmdbuf[2])||(cmdbuf[2]=='0'))
  {
   web_printf("Onion doubling disabled!\r\n");
   rc_level=0;
  }
  else
  {
   i=atoi(cmdbuf+2);
   if(i>0)
   {
    rc_level=i;
    web_printf("Apply Onion doubling change interval %d Sec\r\n", rc_level);
   }
   else if(cmdbuf[2]=='?')
   {
    if(rc_level>0) web_printf("Current Onion doubling change interval is %d Sec\r\n", rc_level);
    else web_printf("Onion doubling is disabling now\r\n");
   }
   else if(cmdbuf[2]=='R') reset_dbl();//reset doubling
   else if(cmdbuf[2]=='I') init_dbl(); //init doubling
  }
 }

 //Followed commands works only during connection
 if(crp_state<3) return 0; //follow work only on state>3


 if(cmdbuf[1]=='K') //-Kkey: send key (if ommited, send own key specified in conf)
 {
  if(!cmdbuf[2]) //name ommited: try load from config default our name
  {
   strcpy(cmdbuf+2, "Our_name");
   if( parseconf(cmdbuf+2)<=0) cmdbuf[2]=0;
   else cmdptr=strlen(cmdbuf);  
  }
  if(!cmdbuf[2]) web_printf("Key name must be specified!\r\n");
  else sendkey(cmdbuf);
 }

 //Followed commands works only during onion connection
 if(!onion_flag) return 0; //follow work only if onion connection used

 if(cmdbuf[1]=='S') //-Sstun: switch to UDP (if ommited, use default stun from conf)
 {
  //switch from onion to udp direct:
  //creates udp socket if not exist, send STUN request,
  //and send invite with local UDP interface to remote
  do_nat(cmdbuf);
 }

 return 0;
}

//*****************************************************************************
//process inputted char
int gochar(int c)
{
 int i;
 if(!c) return 0; //no char for processing
 switch(c)
 {
  case KEY_ENTER: //menu, command, chat or swith talk/mute
  {
   cmdbuf[cmdptr]=0; //terminate c-string in buffer

   if(cmdbuf[0]=='*')
   {
    i=docmd(cmdbuf); //menu item must started from '*'
    if(!i) break;
   }

   if(cmdbuf[0]=='-') //command must started from '-'
   {
    i=parsecmd();
    if(!i) doclr();  //except -E command OK;
    else if(i==-32767) c=KEY_BREAK;
    break;
   }
   else if(!cmdbuf[0]) doptt(c); //switch talk/mute
   else dochat(); //send string to chat
  }
  break;
  case KEY_DEL: //clear string
  {
   doclr(); //claer inputted chars
  }
  break;
  case KEY_ESC:
  {
   c=KEY_BREAK;//hung up, exit
   break;
  }
  case KEY_TAB: //hold for push, release for pause
  case KEY_STAB: //SHIFT+TAB: vox mode on
  {
   doptt(c);
  }
  break;
  case KEY_LEFT:  //select iteam in current menu
  case KEY_RIGHT:
  case KEY_UP:  //select menu
  case KEY_DOWN:
  {
   domenu(c);  //menu navigation
  }
  break;
  case KEY_BACK:   //corrects string: delete last character
  {
   if(cmdptr) //if inputted string not empty
   {
    web_printf("%c %c", 8, 8); //delete from screen
    cmdptr--; //delete from buffer
   }
  }
  break;
  default:    //print char to screen ans store to buffer
  {
   if( (cmdptr<255)&&(c>27) ) //only printable chars
   {
    if(!cmdptr)  printf("\r     \r"); //clear field at screen before first char
    cmdbuf[cmdptr]=c; //store char in buffer
    cmdptr++; //buffer pointer
    printf("%c",c); //print char at screen
   }
  }
 }
 lastchar=c; //remembers last imputted char (for best menu navigation etc.)
 return c;
}


//*****************************************************************************
//read char from console input asynchronosly
int do_char(void)
{
 char c;
 int j;
 
 //get char from raw terminal
 do
 {
  c=0;
  j=0;
#ifdef _WIN32
  if(kbhit()) //if key was pressed
  {
   j=getch(); //read char
   c=j;  //convert int to char type
   if((!c)||(c==-32)) c=KEY_ESC;  //zero is control char now
  }
#else
  j = read(fileno(stdin), &c, 1); //read asynchronosly
#endif
 }
 while((c==KEY_TAB)&&(old_char==KEY_TAB)); //flush TAB after holding
 old_char=c; //for flushing TAB
 //if((j<=0)||(!c)) usleep(0);
 c=goesc(c); //process esc-sequences as control characters
 if(next_char) //remote emulate char
 {
  j=1;         //length for one char
  c=next_char; //inserted remote char
  lastchar=next_char; //for menu navigation
  next_char=0; //once
 }
 if((j>0)&&(c>0)) //have a character
 {
  c=gochar(c); //process character
  fflush(stdout); //print notifications
  if(c==KEY_BREAK) return 1; // break (ctrl+C)
  else return -1;
 }
 return 0;
}

 //*****************************************************************************
//aplly remote command: string or #ascii_char
void setcmd(char* cmdstr)
{
 int i;
 cmdstr[255]=0;
 if(cmdstr[0]=='#') //command mode: one char ascii code
 {
  next_char=(char)atoi(cmdstr+1); //negative for >127
  if(next_char==10) //convert <LF> to <CR> + clear command string
  {
   cmdbuf[0]=0;
   cmdptr=0;
   next_char=KEY_ENTER;
  }
 }
 else  //websocket mode
 { //truncate to first non-printable symbol 
  for(i=0;i<(int)strlen(cmdstr);i++) if((cmdstr[i]<32)&&(cmdstr[i]>0)) cmdstr[i]=0;
  strncpy(cmdbuf, cmdstr, 255); //apply incoming string as a command
  cmdbuf[255]=0;
  cmdptr=strlen(cmdbuf);  //length
  if(cmdptr) next_char=KEY_ENTER; //emulate enter key to execute command
 }
}
//output like printf to stdout and to control port
void web_printf(char* s, ...)
{
    char st[256];
    va_list ap;

    va_start(ap, s);  //parce arguments
    vsprintf(st, s, ap); //printf arg list to array
    printf("%s", st);  //out to stdout
    sendweb(st); //out to control port
    va_end(ap);  //clear arg list
    return;
}

 


