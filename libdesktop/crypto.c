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
//This file contains hight-level cryprographic routines:
//Initial key exchange V02.a
//adds wPFS for origiantor's Id protection (26.10.14)

#ifdef _WIN32 
 #include <stddef.h>
 #include <stdlib.h>
 #include <basetsd.h>
 #include <stdint.h>
 #include <windows.h>
 #include <time.h>

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
  (void)tz;
  FILETIME ft;
  const __int64 DELTA_EPOCH_IN_MICROSECS= 11644473600000000;
  unsigned __int64 tmpres = 0;
  unsigned __int64 tmpres_h = 0;

  if (NULL != tv)
  {
    GetSystemTimeAsFileTime(&ft);

    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;

    //converting file time to unix epoch
    tmpres /= 10;  //convert into microseconds
    tmpres -= DELTA_EPOCH_IN_MICROSECS;

    tmpres_h=tmpres / 1000000UL; //sec
    tv->tv_sec = (int)(tmpres_h);
    tv->tv_usec = (int)(tmpres % 1000000UL);
  }
  return 0;
}
#else //Linux
 #include <time.h>
 #include <sys/time.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libcrp.h"
#include "tcp.h"
#include "codecs.h"
#include "cntrls.h"
#include "sha1.h"
#include "crypto.h"

#define RINGTIME 30  //time in sec for wait user's answer
#define REQTIME 5  //time in sec for wait originator's ID

//Definition of IKE packets fields

//REQ packet (48 bytes total):
#define PUBP buf      //0: autentification DH-key(32 bytes)
#define COMT buf+32   //32: commitment (16 bytes)

//ANS packet (80 bytes total):
#define PREF buf      //0: hidden id (16 bytes)
#define PUBX buf+16   //16: encryption DH-key(32 bytes)
#define NONCE buf+48  //48: nonce (32 bytes)

//ACK packet (16 bytes total):
#define MMAC buf      //0: authenticator (16 bytes)

//====================GLOBAL VARIABLES====================

//Length of data area (in bytes, without header byte and 4 MAC bytes)
//of packets for each packet type
short pktlen[32]={

//      UNKN    MELPE   CD21    LPC10   MELP    CD22    CELP    AMR
        -1,     50,     56,     77,     63,     72,     108,    -1,

//      LPC     GSMHR   G723    G729    GSMEFR  GSMFR   ILBC    BV16
        126,    112,    96,     110,    124,    99,     100,    80,

//      OPUS    SILK    SPEEX   KEY     KLAST   CHAT    DAT     INV
        -1,     -1,     -1,     500,    504,    256,    506-4,  12-4,

//      SYN     REQ     ANS     ACK     AUREQ   AUANS   AUACK   VBR
        8-4,    48-4,   80-4,  16-4,   18-4,   24-4,   6-4,    -1

};

 char command_str[256]={0}; //command string for outgoing call
 char their_name[32]={0}; //name of remote party
 char our_name[32]={0}; //our name (for this call)
 char their_id[32]={0}; //Id of remote party
 char our_id[32]={0}; //Our Id
 unsigned char their_stamp[16]; //Key Stamp of remote party
 unsigned char our_stamp[16]; //Our Key Stamp
 unsigned char their_key[32]; //Long-term public key of remote party
 char book_name[32]={0}; //Name of our address book file

 unsigned char answer[128]; //storage for answer packet waiting answer confirmation
 unsigned char key_access[16]; //secret for access to current secret key
 unsigned char session_key[40]; //autentification + encryption symmetric key
 unsigned char au_data[32]; //protocols data, invites for onion address verification
 unsigned char their_p[64]; //received session authentication key
 unsigned char their_x[64]; //received session encryption key
 unsigned char our_p[64];  //generated session authentication key
 unsigned char our_x[64];  //generated session encryption key
 unsigned char their_nonce[32];   //their received nonce
 unsigned char our_nonce[32];     //our generated nonce
 unsigned char aux_key[16];     //session auxillary key for signs session DH parameters
 unsigned char crp_temp[32];    //temporary storage for crypto computations

 char password[32]={0}; //common passphrase
 char their_onion[32]={0}; //their onion extracted from received invite or connection
 char our_onion[32]={0}; //our onion from config
 char local_ip[32]={0};  //our IP interface autodetected on start
 //protocol state (during IKE):
 //0 -> -1 -> 1 -> -3 -> 3 for originator
 //0 -> -2 -> 2 -> -4 -> 4 for acceptor
 char crp_state=0;
 char invite_tcp=0; //received invite with remote onion address
 unsigned int in_ctr=0; //counter of incoming packets
 unsigned int out_ctr=0; //counter of outgoing packets
 int udp_counter=0; //counter of TCP->UDP tries

 //from tcp.c
 extern char onion_flag; //status of onion connection
 extern int rc_level;  //onion doubling interval
 //from codecs.c
 extern char redundant; //single packetloss flag for UDP transport
 int bad_mac=0; //counter of bad autentificating packets

 struct timeval TM;     //time fixation
 FILE* F=0;              //file for key reading/writing
 FILE* F1=0;
 KECCAK512_DATA spng;  //keccak state: global for perfomance
 //*****************************************************************************

 //returns timestamp: average mS*10
unsigned int getmsec(void)
{
     struct timeval tt1;
     gettimeofday(&tt1, NULL);
     return (unsigned int) (((tt1.tv_sec)<<8) | ((tt1.tv_usec)>>12));
}
//*****************************************************************************

//returns timestamp: Sec
int getsec(void)
{
 #ifdef _WIN32
    time_t tt;
    time(&tt);
    return (int) tt;
 #else
    struct timeval tt1;
    gettimeofday(&tt1, NULL);
    return (int) (tt1.tv_sec);
 #endif
}
//*****************************************************************************

//suspend excuting of the thread  for paus msec
void psleep(int paus)
{
 #ifdef _WIN32
    Sleep(paus);
 #else
    usleep(paus*10);
 #endif
}

//*****************************************************************************

 //get packet data length for specified packets type
 short lenbytype(unsigned char type)
 {
  return pktlen[type&0x1F];
 }
 //*****************************************************************************

 //find packet type for actual data length
 unsigned char typebylen(short len)
 {
  unsigned char i;
  for(i=0;i<32;i++) if(len==pktlen[i]) break;
  if(i==32) i=TYPE_UNKNOWN;
  return i;
 }
 //*****************************************************************************

 //outputs to screen our name and onion address
 void get_names(void)
 {
  web_printf("Default our name is '%s'\r\n", our_name);
  web_printf("Our onion address is '%s'\r\n", our_onion);
  if(crp_state>2)
  {
   web_printf("Connected to '%s'\r\n", their_name);
   if(onion_flag) web_printf("Remote onion address is '%s'\r\n", their_onion);
  }
 }
//*****************************************************************************

 //parse LAST option (-Nname etc.) from string
 //input: option[0] is option's char
 //output: option c-cstring ("name" etc.) in option
 //returns strings length or -1 if not found
 int get_opt(const char* str, char* option)
 {
   int i;
   char c;
   char* p=0;

   if(strlen(str)<2) return -1;
   c=option[0]; //option's char
   option[0]=0; //clear output
   for(i=0;i<(int)(strlen(str)-1);i++) //look for option
   {
    if(str[i]!='-') continue; //must be as -Nname
    if(str[i+1]!=c) continue;
    p=(char*)(str+i+2);  //pointer to option data
   }
   if(!p) return -1; //p points to last option's data
   strncpy(option, p, 31); //copy to output
   option[31]=0;
   for(i=0;i<(int)strlen(option);i++) //truncate to first space
   {
    if(option[i]==' ')
    {
     option[i]=0;
     break;
    }
   }
   return(strlen(option)); //returns output's length
 }
//*****************************************************************************

 //parse last [name] in str, outputs name
 //returns outputs length or -1 if not found
 int get_keyname(const char* str, char* keyname)
 {
  int i, j, into=0, find=0;
  char* p=keyname;
  keyname[0]=0;
  j=0;
//search for [, collect chars up to ] or spase
  for(i=0;i<(int)strlen(str);i++)
  {
   if((str[i]==']')||(str[i]==' ')) //we are out of block now
   {
    into=0; //out of block flag
    find=1; //set found flag
   }
   if(into)
   {
    (*p++)=str[i]; //collect chars into block
    if((j++)>30) break; //restrict length to 32 bytes
   }
   if(str[i]=='[')  //we are enter in block now
   {
    p=keyname; //reset output string
    into=1;   //in block flag
   }
  }
  p[0]=0;   //termanate output string
  if(!find) return -1;  //if not found
  else return(strlen(keyname));  //output length
 }
//*****************************************************************************

 //parse last #nick in str, outputs nick
 //returns outputs length or -1 if not found
 int get_nickname(const char* str, char* nickname)
 {
  int i, j, into=0, find=0;
  char* p=nickname;
  nickname[0]=0;
  j=0;
  //search for #, collect chars up to spase
  for(i=0;i<(int)strlen(str);i++)
  {
   if(str[i]==' ') //we are out of block now
   {
    into=0; //out of block flag
   }
   if(into)
   {
    (*p++)=str[i]; //collect chars into block
     if((j++)>30) break; //restrict length to 32 bytes
   }
   if(str[i]=='#') //we are enter in block now
   {
    p=nickname; //reset output string
    into=1;  //in block flag
    find=1; //set found flag
   }
  }
  p[0]=0;   //termanate output string
  if(!find) return -1;  //if not found
  else return(strlen(nickname));  //output length
 }
//*****************************************************************************

 //parse {b64_stamp} in str, outputs 16 bytes of stamp
 //returns outputs length or -1 if not found
 int get_keystamp(const char* str, unsigned char* stamp)
 {
   if(16==b64dstr(str, stamp, 16)) return 16;  //contacts keystamp IDn
   else return 0;
 }
 //*****************************************************************************

 //load ECDH-key {b64_key} from specified key file in key directory
 //outputs binary key 32 bytes
 //returns 32 if OK, 0 if b64_key not found and -1 if file not found
 int get_key(const char* keyname, unsigned char* key)
 {
  FILE* fl=0;  //key file descriptor
  char str[64]; //for b64_key
  //open key file
  sprintf(str, "%s%s", KEYDIR, keyname); //add path
  if(!(fl = fopen(str, "rt" ))) //open keyfile
  {
   web_printf("! Key file '%s' not found!\r\n", str);
   return -1;
  }
  str[0]=0;
  while(!feof(fl)) //load strings from file
  {
   fgets(str, sizeof(str), fl);
   if(str[0]!='{') continue; //chech for first char is '{'
   break;  //find first block
  }
  fclose(fl);
  if(str[0]=='{')
  {
   if(32==b64dstr(str, key, 32)) return 32; //decode b64->binary
  }
  return 0;   //returns key length or 0 if block not found
 }
//*****************************************************************************

 //load info-string "#info..." from specified key file in key directory
 //returns length if OK, 0 if info string not found and -1 if file not found
 int get_keyinfo(const char* keyname, char* str)
 {
  FILE* fl=0;  //key file descryptor
  int i;
  //open key file
  sprintf(str, "%s%s", KEYDIR, keyname); //add path
  if(!(fl = fopen(str, "rt" ))) //open keyfile
  {
   web_printf("! Key file '%s' not found!\r\n", str);
   return 0;
  }
  str[0]=0;
  while(!feof(fl))  //load strings from file
  {
   fgets(str, 255, fl);
   str[255]=0;
   if(str[0]!='#') continue;  //chech for first char is '#'
   break; //find first block
  }
  fclose(fl);
  if(str[0]!='#') return 0;
  //truncate to first \r or \n
  for(i=0;i<(int)strlen(str);i++)
  {
   if( (str[i]=='\r')||(str[i]=='\n') )
   {
    str[i]=0;
    break;
   }
  }
  return strlen(str);  //returns info length or 0 if block not found
 }
//*****************************************************************************

 //load LAST specified contacts string from specified address-book file
 //returns length or 0 if string not found or -1 if bookfile not found
 int get_bookstr(const char* book, const char* name, char* res)
 {
  FILE* fl=0;
  int len;
  char str[256];
  int i;
  len=strlen(name); //length of contact name for comparise
  //open addressbook file
  sprintf(str, "%s%s", KEYDIR, book); //add path
  if(!(fl = fopen(str, "rt" ))) //open addressbook
  {
   web_printf("! Address book '%s' not found!\r\n", str);
   return -1;
  }
  str[0]=0;
  res[0]=0;
  while(!feof(fl)) //loads strings from adressbook
  {
   fgets(str, sizeof(str), fl);
   str[255]=0;
   if(str[0]!='[') continue; //check for first char is '['
   if(!memcmp(str+1, name, len)) strcpy(res, str); //compare contacts name
  }
  fclose(fl);
  if(!res[0]) return 0; //return  0 if contact not found
  for(i=0; i<(int)strlen(res); i++)
  {
   if( (res[i]=='\r')||(res[i]=='\n') )
   {
    res[i]=0;
    break;
   }
  }
  return strlen(res); //returns string length
 }
//*****************************************************************************

//derives key from password for accessing to private key file
//inputs: pas - password string, keybody - destination
//(or default global destination if NULL specified)
 void set_access(char* pas, unsigned char* keybody)
 {
  int i;
  #define PKDFCOUNT 32768

  if(!keybody) keybody=key_access; //use degault destination for acces key
  if(!pas[0]) //clear access key if password string empty
  {
   xmemset(keybody, 0, 16);
   web_printf("Key access cleared\r\n");
  }
  else  //derives key from password
  {
   Sponge_init(&spng, 0, 0, 0, 0);
   //absorb salt
   Sponge_data(&spng, (const BYTE*)"$OnionPhone_salt", 16, 0, SP_NORMAL);
   //absorb password string many times
   for(i=0; i<PKDFCOUNT; i++)
   Sponge_data(&spng, (const BYTE*)pas, strlen(pas), 0, SP_NORMAL);
   Sponge_finalize(&spng, keybody, 16); //squize key
   web_printf("Key access applied\r\n");
  }
 }
 //*****************************************************************************

 //check access to specified secret key
 int check_access(void)
 {
  //check our name is now specified globally
  if(our_name[0])
  {  //try decrypt secret key file
   if(32==get_seckey(our_name, 0)) //or use default our name from config
   {
    web_printf("Access to secret key '%s' is OK\r\n", our_name);
    return 1;
   }
   else web_printf("! No access to secret key '%s' now!\r\n", our_name);
  }
  else web_printf("! Our name for secret key not specified!\r\n");
  return 0;
 }
 //*****************************************************************************

 //load specified secret key from file 'keyname.sec' to seckey[32]
 //returns number of bytes loaded (must be 32)
 //=================================================================
 //Normally secret key must be encrypted and saved with IV and MAC
 //In this case for each loading password required
 //This is not user-frendly thats why we use unecrypted secret key file
 //and strongly recomended to put OnionPhone folder to encrypted
 //container or use disk encryprion
 //=================================================================
 int get_seckey(const char* name, unsigned char* seckey)
 {
   char str[256];
   volatile unsigned char aukey[64]; //access|nonce|mac|our_mac
   volatile unsigned char tmpkey[32];
   FILE* fl=0;
   int i;

   if(!seckey) seckey=(unsigned char*)tmpkey; //check mode - decrypt locally onle

   sprintf(str, "%s%s.sec", KEYDIR, name); //add path
   if(!(fl = fopen(str, "rb" ))) return 0; //open specified keyfile

   //load seckey byte-by-byte
   for(i=0; i<32; i++) //load seckey byte-by-byte
   {
    seckey[i]=getc(fl);
    if(feof(fl)) break;
   }
   if(i<32) return 0;  //key not compleet

   //try load nonce+mac byte-by-byte
   for(i=0; i<32; i++) //load seckey byte-by-byte
   {
    aukey[i+16]=getc(fl);
    if(feof(fl)) break;
   }
   if(i&&(i<32)) return 0; //au-data existed but not compleet
   fclose(fl);

   if(i) //if keyfile length more then 32 bytes (encrypted format)
   {  //process as encrypted secret key

    //search -Ysecret option in command string
    //(password for temporary access to secret key)
    str[0]='Y';
    i=get_opt(command_str, str);

    //try access specified for default key
    memcpy((void*)aukey, key_access, 16);  //use default access
    //computes mac
    Sponge_init(&spng, 0, 0, 0, 0);
    Sponge_data(&spng, seckey, 32, 0, SP_NORMAL); //absorb secret key body
    Sponge_data(&spng, (const BYTE*)aukey, 32, 0, SP_NORMAL); //absorb access|nonce
    Sponge_finalize(&spng, (BYTE*)aukey+48, 16); //our mac

    //check MAC and try access specified in command line
    if((i>0)&&(memcmp((const void*)(aukey+32), (const void*)(aukey+48), 16)))
    {
     set_access(str, (unsigned char*)aukey);  //computes temporary access to key
     //computes mac
     Sponge_init(&spng, 0, 0, 0, 0);
     Sponge_data(&spng, seckey, 32, 0, SP_NORMAL); //absorb secret key body
     Sponge_data(&spng, (const BYTE*)aukey, 32, 0, SP_NORMAL); //absorb access|nonce
     Sponge_finalize(&spng, (BYTE*)aukey+48, 16); //our mac
    }
    //check MAC
    if(memcmp((const void*)(aukey+32), (const void*)(aukey+48), 16))
    {
     web_printf("! Wrong access to our secret key!\r\n");
     return 0;
    }

    //decrypt secret key's body
    Sponge_init(&spng, (const BYTE*)aukey, 32, 0, 0); //absorb access|nonce
    Sponge_data(&spng, seckey, 32, seckey, SP_NOABS); //decrypt secret key in duplex mode
    xmemset((void*)aukey, 0, 64);
    Sponge_finalize(&spng, 0, 0);
   }
   if(seckey==tmpkey) xmemset(seckey, 0, 32);
   return 32; //key bytes
 }
//*****************************************************************************

 //load first matched contacts string from specified address-book
 //returns length or 0 if string not found or -1 if bookfile not found
 //their Da in crp_temp, their P in their_p, our q in our_p
 //Acceptor (B) checks for each contact from addressbook:
 // Dn =? H(IDn | P^b | P^q)
 int search_bookstr(const char* book, char* ourname, const unsigned char* buf, char* res)
 {
	 (void)buf;
   int i;
   FILE* fl=0;
   unsigned char our_secret[32];
   unsigned char guess_secret[32];
   unsigned char efemeral_secret[32];
   unsigned char iskeys=0;
   unsigned char work[32];
   char key_id[32];
   unsigned char key_stamp[16];

   //computes efemeral secret
   curve25519_donna(efemeral_secret, our_p, their_p); //P^q

   //load our secret key to work
   if(ourname[0]) i=get_seckey(ourname, work); //b
   else i=0;
   if(i==32) //if secret loaded
   {
    iskeys|=1; //set out secret flag
    curve25519_donna(our_secret, work, their_p); //P^b
   }

   //load guest secret key
   i=get_seckey(NONAME, work);  //g
   if(i==32) //if secret loaded
   {
    iskeys|=2; //set guest secret flag
    curve25519_donna(guess_secret, work, their_p); //P^g
   }
   if(!iskeys) return -2;  //no secrets: returns error code -2

   //open bookfile
   sprintf(res, "%s%s", KEYDIR, book); //add path
   if(!(fl = fopen(res, "rt" )))
   {
    web_printf("! Address book '%s' not found!\r\n", res);
    return -1; //if specified addressbook not found return error code -1
   }

   //load contacts string-by-string from bookfile to eof
   while(!feof(fl))
   {
    fgets(res, 256, fl); //load contact's string from addressbook
    res[255]=0;
     //find key owner nickname from book
    i=get_nickname(res, key_id); //find #nickname from contacts key
    if(i<=0) continue;

    i=get_keystamp(res, key_stamp);
    if(i!=16) continue;

    if(iskeys&1) //if our secret was be loaded
    {    //computes Dn=H(IDn | P^b | P^q)
     Sponge_init(&spng, 0, 0, 0, 0);
     Sponge_data(&spng, key_stamp, 16, 0, SP_NORMAL); //IDn is key stamp insteed user name
     Sponge_data(&spng, our_secret, 32, 0, SP_NORMAL); //P^b
     Sponge_data(&spng, efemeral_secret, 32, 0, SP_NORMAL); //P^q
     Sponge_finalize(&spng, work, 16);
     if(!memcmp(work, crp_temp, 16)) //check for computes Prf equals Prf from packet
     {
      iskeys|=4;  //set flag OK
      break;
     }
    }
    //also check for guest secret key (if it was be loaded)
    if(iskeys&2)
    {
     Sponge_init(&spng, 0, 0, 0, 0);
     Sponge_data(&spng, key_stamp, 16, 0, SP_NORMAL); //IDn from book
     Sponge_data(&spng, guess_secret, 32, 0, SP_NORMAL); //P^g
     Sponge_data(&spng, efemeral_secret, 32, 0, SP_NORMAL); //P^q
     Sponge_finalize(&spng, work, 16);
     if(!memcmp(work, crp_temp, 16))
     {
      strcpy(ourname, NONAME);
      iskeys|=4;
      break;
     }
    }
   }
   if(!(iskeys&4)) return 0; //if no one contact from book matched returns 0

  //current contact matched
   for(i=0; i<(int)strlen(res); i++)
   {
    if( (res[i]=='\r')||(res[i]=='\n') ) //truncate contact's string
    {
     res[i]=0;
     break;
    }
   }
   return strlen(res); //returns length of matched contact's string
 }
//*****************************************************************************


 //===============TOP-LEVEL======================================

 //reset encryption state
 void reset_crp(void)
 {
  char str[256];
  xmemset(their_name, 0,32);
  xmemset(their_id, 0, 32);
  xmemset(our_id, 0, 32);
  xmemset(their_stamp, 0, 16);
  xmemset(our_stamp, 0, 16);
  strcpy(str, "Our_name");
  if( parseconf(str)>0 )  //search interface address in conf-file
  {
   strncpy(our_name, str, 31);
   our_name[31]=0;
  }

  xmemset(password, 0,32);
  xmemset(session_key, 0, sizeof(session_key)); //clear session keys
  xmemset(au_data, 0, 32);   //clear autentification data
  xmemset(aux_key, 0, 16); //clear aux_key
  xmemset(their_nonce, 0, 32); //clear nonce
  xmemset(our_nonce, 0, 32);
  xmemset(our_p, 0, 64);
  xmemset(our_x, 0, 64);
  xmemset(their_p, 0, 64);
  xmemset(their_x, 0, 64); //clear session public keys
  xmemset(&spng, 0, sizeof(spng));
  xmemset(&TM, 0, sizeof(TM));
  xmemset(crp_temp, 0, 32);   //clear temporary storage
  crp_state=0;          //set initial state
  in_ctr=0;            //clear counter of incoming packets
  out_ctr=0;           //clear counter of outgoing packets
  invite_tcp=0;
  if(F) fclose(F);
  F=0;
  if(F1) fclose(F1);
  F1=0;
 }
//*****************************************************************************

//Initial IKE procedure
//originator only (crp_state=0):
        //input: -Ntheir_name -Iour_name
        //output: P, Ca=H(X|Na)
 int do_req(unsigned char* pkt)
 {
  //A is originator
  
  //REQ packet (48 bytes):
  //PUBP=buf : autentification DH-key(32 bytes)
  //COMT=buf+32 : commitment (16 bytes)

  int i;
  unsigned char* buf=pkt+1; //pointer to outputted data area (skip type byte)
  char str[256];

  char contact[64]; //name of requested contact
  char bookname[64]; //adressbook file
  char bookstr[256]; //contacts string from adressbook
  char ourname[64];  //own name used for contact

  //init state
  reset_crp();
  //set our identity default
  strncpy(ourname, our_name, 31);
  ourname[31]=0;

  //parce contact string for -Nname, use NONAME default
  str[0]='N'; //remote contact's name option
  i=get_opt(command_str, str);
  if(i<0)
  {
   web_printf("! Contact name not specified for connection!\r\n");
   disconnect();
   return 0; //no name specified: command not for connect
  }
  else if(!i)
  {
   sprintf(contact, "%s", NONAME ); //not specified: use 'anonimus' for their name
  }
  else if(i<64) sprintf(contact, "%s", str ); //use specified contact name
  else
  {
   web_printf("! Contact name too long!\r\n");
   disconnect();
   return 0; //no name specified: command not for connect
  }

  //parse bookname -Bbookname, use BOOK default
  str[0]='B';
  i=get_opt(command_str, str);
  if(i<1) sprintf(bookname, "%s", book_name);
  else if(i<64) sprintf(bookname, "%s", str );
  else
  {
   web_printf("! Address book name too long!\r\n");
   disconnect();
   return 0; //no name specified: command not for connect
  }

  //search string by contact name in address book
  i=get_bookstr(bookname, contact, bookstr);
  if(i<1)
  {
    web_printf("! Contact '%s' not found in adressbook '%s'\r\n", contact, bookname);
    disconnect();
    return 0;
  }

  //parse book string for trust option
  str[0]='L';
  i=get_opt(bookstr, str);
  if(i<1) web_printf("! Warning! Untrusted contact!\r\n");
  else if(i>0) web_printf("Contact's trust level: %s\r\n", str);

   //find key owner's nickname from book
  i=get_nickname(bookstr, str); //find #nickname from contacts key
  if((i>0)&&(i<32)) strcpy(their_id, str); //acceptors id (for originator)
  else
  {
   web_printf("! Nickname for their key not found in adressbook!\r\n");
   disconnect();
   return 0;
  }

  //extract their key stamp
  i=get_keystamp(bookstr, their_stamp); //extract {b64_stamp} as bytes from  book string
  if(i!=16)
  {
   web_printf("! Stamp of their key not found / not correct in adressbook!\r\n");
   disconnect();
   return 0;
  }

  //parse identity option from book
  str[0]='I';
  i=get_opt(bookstr, str);
  if(!i) strcpy(ourname, NONAME); //use 'anonimus' insteed own pubkey
  else if((i<32)&&(i>0)) strcpy(ourname, str);
  //parse identity option from command (overrided book)
  str[0]='I';
  i=get_opt(command_str, str);
  if(!i) strcpy(ourname, NONAME); //use 'anonimus' insteed own pubkey
  else if((i<32)&&(i>0)) strcpy(ourname, str);

  //Parse password from book
  str[0]='P';
  i=get_opt(bookstr, str);
  if((i>2)&&(i<32)) strcpy(password,str);
  else password[0]=0;
  //Parse password from command (overrided book)
  str[0]='P';
  i=get_opt(command_str, str);
  if((i>2)&&(i<32)) strcpy(password,str);

  //load contact's ECDH public key
  sprintf(str, "%s", contact);
  i=get_key(str, their_key);
  if( (i!=32) && their_id[0] ) //try to load from keyfile specified by produser
  {
   strncpy(contact, their_id, 31);
   contact[31]=0;
   sprintf(str, "%s", contact);
   i=get_key(str, their_key); //B: store their pubKey (B)
   if(i!=32)
   {
    web_printf("! Key '%s'(%s) not found!\r\n", contact, their_id);
    disconnect();
    return 0;
   }
  }
  //find string of our used key (own or anonimus)
  i=get_bookstr(bookname, ourname, bookstr);
  if(i<1)
  {
    web_printf("! Our key '%s' not found in adressbook!\r\n", ourname);
    disconnect();
    return 0;
  }
  //get our nickname
  i=get_nickname(bookstr, str); //find #nickname from contacts key
  if((i>0)&&(i<32)) strcpy(our_id, str);  //originators id (for originator)
  else
  {
   web_printf("! Nickname for our key not found in adressbook!\r\n");
   disconnect();
   return 0;
  }

  //extract our key stamp
  i=get_keystamp(bookstr, our_stamp); //extract {b64_stamp} as bytes from  book string
  if(i!=16)
  {
   web_printf("! Stamp of our key not found / not correct in adressbook!\r\n");
   disconnect();
   return 0;
  }

  //store participants names for this call
  strncpy(our_name, ourname, 31);
  our_name[31]=0;
  strncpy(their_name, contact, 31);
  their_name[31]=0;
  //****************************************************
  //-----------------Protocol running by originator (A)
  //****************************************************

  //generate authentication pair: p at random, computes P to output PUBP
  randFetch(our_p, 32); //p
  get_pubkey(PUBP, our_p); //P
  //generate authentication pair: x at random, computes X to output PUBP
  randFetch(our_x, 32); //x
  get_pubkey(crp_temp, our_x); //X
  //peek nonce at random
  randFetch(our_nonce, 32);  //Na

  //computes commitment for X and nonce -> COMT
  //C=H(X|Na)
  Sponge_init(&spng, 0, 0, 0, 0); //init hash
  //add X to hash
  Sponge_data(&spng, crp_temp, 32, 0, SP_NORMAL); //X
  //add nonce to hash
  Sponge_data(&spng, our_nonce, 32, 0, SP_NORMAL); //Na
  //computes hash to output COMT (16 bytes)
  Sponge_finalize(&spng, COMT, 16); //commitment Ca

  //notify call
  web_printf("Outgoing call from '%s' to '%s', waiting for answer...\n", ourname, contact);

  //check access to used secret key
  if(!check_access())
  {
   web_printf("! Call is terminated\r\n");
   disconnect();
   return 0;
  }

  //set state -1: originator wait for KEY
  crp_state=-1;
  //get our time now as connection time
  settimeout(REQTIME);

  //set packet type and return fixed length
  pkt[0]=(TYPE_REQ | 0x80);
  //memcpy(answer, pkt, 5+lenbytype(TYPE_REQ)); //for dubles of reuest
  return (lenbytype(TYPE_REQ)); //48 bytes

  //output=REQ:  PUBP[32], COMT[16]
  //our_p, our_x contains private keys p, x
  //our_nonce contain Na
  //A -> B:  REQ
 }
//*****************************************************************************

 //for acceptor (crp_state==0):
 //input=REQ: PUBP[32], COMT[16]
 //output=REQ: PUBP[32], COMT[16]
 //-----------------------------
 //for originator (crp_state==-1)
 //input=REQ: PUBP[32], COMT[16]
 //output=ANS: PREF[16], PUBX[32], NONCE[32]

 int go_req(unsigned char* pkt)
 {
  //B is acceptor

  //REQ packet (48 bytes):
  //PUBP=buf : autentification DH-key(32 bytes)
  //COMT=buf+32 : commitment (16 bytes)

  //ANS packet (80 bytes):
  //PREF=buf : hidden id (16 bytes)
  //PUBX=buf+16 : encryption DH-key(32 bytes)
  //NONCE=buf+48 : nonce (32 bytes)

  unsigned char* buf=pkt+1; //pointer to outputted data area (skip type byte)

  //check for state
  if(crp_state==0) //acceptor side (B)
  {
   //****************************************************
  //-----------------Protocol running by acceptor (B)
  //****************************************************
   //B accepts REQ packet: PUBP[32], COMT[16] (48 bytes)
   //and send REQ packet to answer (48 bytes)

   //copy received data from A
   memcpy(their_p, PUBP, 32); // their P
   memcpy(their_x, COMT, 16); //their commitment Ca

   //generate authentication pair: q at random, computes Q to output PUBP
   randFetch(our_p, 32); //q
   get_pubkey(PUBP, our_p); //Q
   //generate authentication pair: y at random, computes Y to output PUBP
   randFetch(our_x, 32); //y
   get_pubkey(crp_temp, our_x); //Y
   //peek nonce at random
   randFetch(our_nonce, 32);  //Nb

   //computes commitment for Y and nonce -> COMT
   //C=H(Y|Nb)
   Sponge_init(&spng, 0, 0, 0, 0); //init hash
   //add Y to hash
   Sponge_data(&spng, crp_temp, 32, 0, SP_NORMAL); //Y
   //add nonce to hash
   Sponge_data(&spng, our_nonce, 32, 0, SP_NORMAL); //Nb
   //computes hash to output COMT (16 bytes)
   Sponge_finalize(&spng, COMT, 16); //commitment Cb

   //notify initiation
   web_printf("Incoming request processed\r\n");
   //set state -2: acceptors wait for ID
   crp_state=-2;
   //get our time now as connection time
   settimeout(REQTIME);

  //set packet type and return fixed length
  pkt[0]=(TYPE_REQ | 0x80);
  return (lenbytype(TYPE_REQ)); //48 bytes

  //output=REQ:  PUBP[32], COMT[16]  (48 bytes)
  //our_p, our_x contains private keys q, y
  //their_p, their_x contain public keys P and commitment Ca
  //our_nonce contain Nb
  //B -> A:  REQ
  }
  else if(crp_state==-1) //originator side (A)
  {
   //****************************************************
   //-----------------Protocol processed by originator (A)
  //****************************************************
   //A accepts REQ packet: PUBP[32], COMT[16] (48 bytes)
   //and send ANS packet to answer: PREF[16], PUBX[32], NONCE[32]  (80 bytes)

   //copies received data from B
   memcpy(their_p, PUBP, 32); // their Q
   memcpy(their_x, COMT, 16); //their commitment Cb

   //computes preffix (hidden Id)  Da=H(IdA|B^p|Q^p) -> PREF
   //in fact this is equal to  ElGamal encryption of our keystamp for remote party
   //efemeral secret Q^p adds wPFS to originator's ID protection
   Sponge_init(&spng, 0, 0, 0, 0); //init hash
   //add our id to hash
   Sponge_data(&spng, our_stamp, 16, 0, SP_NORMAL); //Our Stamp IdA
   //add B^p to hash
   curve25519_donna(crp_temp, our_p, their_key); //B^p
   Sponge_data(&spng, crp_temp, 32, 0, SP_NORMAL);
   //add Q^p to hash
   curve25519_donna(crp_temp, our_p, their_p); //Q^p
   Sponge_data(&spng, crp_temp, 32, 0, SP_NORMAL);
   //computes hash to output PREF (16 bytes)
   Sponge_finalize(&spng, PREF, 16); //hidden ID

   //computes our encryption public key
   get_pubkey(PUBX, our_x); //X

   //copies our nonce
   memcpy(NONCE, our_nonce, 32);

   //put to *buf  TYPE_ANS
   pkt[0]=(TYPE_ANS | 0x80);
   crp_state=1; //set state 1: originator wait for ID
   settimeout(RINGTIME); //set reset timeout
   return (lenbytype(TYPE_ANS));

   //output=ANS:  PREF[16], PUBX[32], NONCE[32]  (80 bytes)
   //our_p, our_x still contains private keys p, x
   //their_p, their_x contain public keys Q and commitment Cb
   //our_nonce contain Na
   //A -> B: ANS
  }
  else return 0;
 }
 //*****************************************************************************

 //for acceptor (crp_state==-2):
 //input=ANS: PREF[16], PUBX[32], NONCE[32]
 //output=ANS: PREF[16], PUBX[32], NONCE[32]
 //-----------------------------
 //for originator (crp_state==1)
 //input=ANS: PREF[16], PUBX[32], NONCE[32]
 //output=ACK: MMAC[16]
 int go_ans(unsigned char* pkt)
 {
  //ANS packet (80 bytes):
  //PREF=buf : hidden id (16 bytes)
  //PUBX=buf+16 : encryption DH-key(32 bytes)
  //NONCE=buf+48 : nonce (32 bytes)

  //ACK packet (16 bytes):
  //MMAC=buf : authenticator (16 bytes)

  unsigned char* buf=pkt+1;
  char str[256];
  int i, reject=0;

  char bookfile[64]; //adressbook filename
  char bookstr[256]; //contacts string from adressbook
  char keyname[64];  //their public key used for connection
  char ourname[64];  //own name used for contact

  struct tm *ttime;
  time_t ltime;

  //check for state
  if(crp_state==-2)
  {
   //****************************************************
   //-----------------Protocol processed by acceptor (B)
   //****************************************************
   //B accepts ANS packet: PREF[16], PUBX[32], NONCE[32]  (80 bytes)
   //and send ANS packet to answer

   //check commitment is valid?
   //C=H(X|Na)
   Sponge_init(&spng, 0, 0, 0, 0); //init hash
   //add X to hash
   Sponge_data(&spng, PUBX, 32, 0, SP_NORMAL); //X
   //add nonce to hash
   Sponge_data(&spng, NONCE, 32, 0, SP_NORMAL); //Na
   //computes hash (16 bytes)
   Sponge_finalize(&spng, crp_temp, 16); //commitment Ca
   if(memcmp(crp_temp, their_x, 16))
   {
    web_printf("! Commitment failure!\r\n");
    disconnect();
    return 0;
   }
   //copy incoming data
   memcpy(their_x, PUBX, 32); //X
   memcpy(their_nonce, NONCE, 32);   //Na
   memcpy(crp_temp, PREF, 16);       //Da

   //search sender name in adressbook for ourkey receiver
   strncpy(ourname, our_name, 31);
   ourname[31]=0;
   strncpy(bookfile, book_name, 31);
   bookfile[31]=0;
   //now we check each contact from adressbook matches for
   //received hidden id (for our default key and guest key)
   i=search_bookstr(bookfile, ourname, buf, bookstr);
   if(i<1)
   {  //sender not finded: not in book!
    web_printf("! Incoming call from/to unknown, guest will be rejected\r\n");
    //now we set flag and process incoming call like from guest
    //for time alignment. Later we will use the fake aux_key and continue the protocol
    //Thus, the originator can not distinguish the cause of the interrupt protocol in step 3.
    reject=1; //set rejection flag
    strcpy(ourname, NONAME); //our name is guest
    i=get_bookstr(bookfile, ourname, bookstr); //their name is guest
    if(i<1)
    {
     disconnect();
     return 0;
    }
   }

   //get keys owner
   i=get_nickname(bookstr, str); //find #nickname from contacts key
   if((i>0)&&(i<32)) strcpy(their_id, str); //originator'd id (for acceptor)
   else
   {
    web_printf("! Nickname for their key not found in adressbook!\r\n");
    disconnect();
    return 0;
   }

   //extract their key stamp IdA
   i=get_keystamp(bookstr, their_stamp); //extract {b64_stamp} as bytes from  book string
   if(i!=16)
   {
    web_printf("! Stamp of their key not found / not correct in adressbook!\r\n");
    disconnect();
    return 0;
   }

   //get name of sender
   i=get_keyname(bookstr, keyname);
   if((i>0)&&(i<32)) strcpy(keyname, str);
   else
   {
    web_printf("! Adressbook error!\r\n");
    disconnect();
    return 0;
   }

   //notification
   if(!reject) web_printf("Incoming call to '%s' from '%s' ( '%s' )", ourname, keyname, their_id);
   //time notification
#ifdef _WIN32
   time(&ltime);
#else
   ltime=getsec(); //get mktime
#endif
   ttime=localtime(&ltime); // convert to localtime
   if(ttime) web_printf(" at %02d:%02d", ttime->tm_hour, ttime->tm_min);

   //check trust level option -L for this contact
   str[0]='L'; //ignore option
   i=get_opt(bookstr, str);
   if(i<0) sprintf(str, "?");
   else if(!i) sprintf(str, "0");
   web_printf(" TrustLevel: %s\r\n", str);
   if((!str[1])&&(str[0]=='X'))
   {
    web_printf("! Rejected by trust level (ignored)\r\n");
    reject=1;
   }
   if(!reject) web_printf("Press ENTER to answer\r");
   else web_printf("Press ENTER to reject\r");
   fflush(stdout);

   //Look for password for this contact
   str[0]='P';
   i=get_opt(bookstr, str);
   if((i>2)&&(i<32)) strcpy(password, str);
   else password[0]=0;

   //load their ECDH public key
   i=get_key(keyname, their_key); //A
   if(i!=32)
   {
    web_printf("! Contact's keyfile '%s' not found!\r\n", keyname);
    disconnect();
    return 0;
   }

   //search string for our name in address book
   i=get_bookstr(bookfile, ourname, bookstr);
   if(i<=0)
   {
    web_printf("! Our key not found in adressbook!\r\n");
    disconnect();
    return 0;
   }

   //get our nickname
   i=get_nickname(bookstr, str); //find #nickname from our key
   if((i>0)&&(i<32)) strcpy(our_id, str); //acceptors'd id (for acceptor)
   else
   {
    web_printf("! Nickname for our key not found in adressbook!\r\n");
    disconnect();
    return 0;
   }

   //extract our key stamp IdB
   i=get_keystamp(bookstr, our_stamp); //extract {b64_stamp} as bytes from  book string
   if(i!=16)
   {
    web_printf("! Stamp of our key not found / not correct in adressbook!\r\n");
    disconnect();
    return 0;
   }

   //save actual our_name and their_name uses in current session
   strncpy(their_name, keyname, 31);
   their_name[31]=0;
   strncpy(our_name, ourname, 31);
   our_name[31]=0;

   //load our secret key
   i=get_seckey(our_name, session_key); //temporary store b
   if(i!=32)
   {
    web_printf("! Secret keyfile '%s' not found or invalid!\r\n", ourname);
    disconnect();
    return 0;
   }

   //computes aux_key K=H(A^q|P^b|P^q)
   Sponge_init(&spng, 0, 0, 0, 0);
   //computes token A^q and add to hash
   curve25519_donna(crp_temp, our_p, their_key); //A^q
   Sponge_data(&spng, crp_temp, 32, 0, SP_NORMAL);
   //computes token P^b and add to hash
   curve25519_donna(crp_temp, session_key, their_p); //P^b
   Sponge_data(&spng, crp_temp, 32, 0, SP_NORMAL);
   //computes autentification secret P^q and add to hash
   curve25519_donna(crp_temp, our_p, their_p); //P^q
   Sponge_data(&spng, crp_temp, 32, 0, SP_NORMAL);
   Sponge_finalize(&spng, aux_key, 16); //aux_key

   //computes session key s = H(X^y) xor nonceA xor nonceB
   curve25519_donna(session_key, our_x, their_x); //X^y
   Sponge_init(&spng, 0, 0, 0, 0);
   Sponge_data(&spng, session_key, 32, 0, SP_NORMAL);
   Sponge_finalize(&spng, session_key, 32); //secret
   for(i=0;i<32;i++) session_key[i]^=(our_nonce[i]^their_nonce[i]);

   //computes preffix (hidden Id)  Db=H(IdB|A^q|P^q) -> PREF
   //in fact this is equal to  ElGamal encryption of our keystamp for remote party
   //efemeral secret Q^p adds wPFS to originator's ID protection
   Sponge_init(&spng, 0, 0, 0, 0); //init hash
   //add our id to hash
   Sponge_data(&spng, our_stamp, 16, 0, SP_NORMAL); //Our Stamp IdB
   //add A^q to hash
   curve25519_donna(crp_temp, our_p, their_key); //A^q
   Sponge_data(&spng, crp_temp, 32, 0, SP_NORMAL);
   //add P^q to hash
   curve25519_donna(crp_temp, our_p, their_p); //P^q
   Sponge_data(&spng, crp_temp, 32, 0, SP_NORMAL);
   //computes hash to output PREF (16 bytes)
   Sponge_finalize(&spng, PREF, 16); //hidden ID

   //copies our nonce
   memcpy(NONCE, our_nonce, 32);
   //computes our encryption public key
   get_pubkey(PUBX, our_x); //Y
   //replace private keys
   memcpy(our_x, PUBX, 32);
   get_pubkey(crp_temp, our_p); //Q
   memcpy(our_p, crp_temp, 32);
   //alters the key if rejection flag
   if(reject)
   {
    randFetch(aux_key, 16);
    randFetch(PREF, 16);
    randFetch(session_key, 32);
   }
   else //time aligment
   {
    randFetch(answer, 16);
    randFetch(answer+16, 16);
    randFetch(answer+32, 32);
   }
   
   //put to *buf  TYPE_ANS
   pkt[0]=(TYPE_ANS | 0x80);
   crp_state=2; //set state 2: acceptor wait MAC
   settimeout(RINGTIME); //set reset timeout
   i=lenbytype(TYPE_ANS);
   memcpy(answer, pkt, i+5); //store answer ready for sending

   //for auto answering
   strcpy(str, "Auto_answer");
   parseconf(str);
   if(str[0]=='1') return (lenbytype(TYPE_ANS));
   else  return 0; //wait for manual permission by pressing Enter
   //output=ANS:  PREF[16], PUBX[32], NONCE[32]  (80 bytes)
   //our_p, our_x contains our public keys Q and Y
   //their_p, their_x contain their public keys P and X
   //our_nonce contain Nb, their_nonce contain Na
   //aux_key containe 128 bits verification key
   //session_key contains 128+128 bits session keys
   //au_data contains 128+128 bits onion identificators
   //after manual incoming call acception B -> A: ANS
  }
  else if(crp_state==1)
  {
   //****************************************************
   //-----------------Protocol processed by initiator (A)
   //****************************************************
   //A accepts ANS packet: PREF[16], PUBX[32], NONCE[32]  (80 bytes)
   //and send ACK packet to answer: MMAC[16] (16 bytes)

   //check commitment is valid?
   //C=H(Y|Nb)
   Sponge_init(&spng, 0, 0, 0, 0); //init hash
   //add Y to hash
   Sponge_data(&spng, PUBX, 32, 0, SP_NORMAL); //Y
   //add nonce to hash
   Sponge_data(&spng, NONCE, 32, 0, SP_NORMAL); //Nb
   //computes hash (16 bytes)
   Sponge_finalize(&spng, crp_temp, 16); //commitment Cb
   if(memcmp(crp_temp, their_x, 16))
   {
    web_printf("! Commitment failure!\r\n");
    disconnect();
    return 0;
   }

   //copy incoming data
   memcpy(their_x, PUBX, 32); //Y
   memcpy(their_nonce, NONCE, 32);   //Nb

   //load our secret key
   i=get_seckey(our_name, session_key); //temporary store b
   if(i!=32)
   {
    web_printf("! Secret keyfile '%s' not found or invalid!\r\n", ourname);
    disconnect();
    return 0;
   }

   //computes and checks prefix PREF =? H(IdB | Q^a | Q^p)
   Sponge_init(&spng, 0, 0, 0, 0);
   Sponge_data(&spng, their_stamp, 16, 0, SP_NORMAL);
   curve25519_donna(crp_temp, session_key, their_p); //Q^a
   Sponge_data(&spng, crp_temp, 32, 0, SP_NORMAL);
   curve25519_donna(crp_temp, our_p, their_p); //Q^p
   Sponge_data(&spng, crp_temp, 32, 0, SP_NORMAL);
   Sponge_finalize(&spng, crp_temp, 16); //Db
   if(memcmp(PREF, crp_temp, 16))
   {
    web_printf("! Answer from unknown, terminating call\r\n");
    disconnect();
    return 0;
   }

   //computes aux_key K=H(Q^a|B^p|Q^p)
   Sponge_init(&spng, 0, 0, 0, 0);
   //computes token Q^a and add to hash
   curve25519_donna(crp_temp, session_key, their_p); //Q^a
   Sponge_data(&spng, crp_temp, 32, 0, SP_NORMAL);
   //computes token B^p and add to hash
   curve25519_donna(crp_temp, our_p, their_key); //B^p
   Sponge_data(&spng, crp_temp, 32, 0, SP_NORMAL);
   //computes autentification secret Q^p and add to hash
   curve25519_donna(crp_temp, our_p, their_p); //Q^p
   Sponge_data(&spng, crp_temp, 32, 0, SP_NORMAL);
   Sponge_finalize(&spng, aux_key, 16); //aux_key

   //computes session key s = H(Y^x) xor nonceA xor nonceB
   curve25519_donna(session_key, our_x, their_x); //Y^x
   Sponge_init(&spng, 0, 0, 0, 0);
   Sponge_data(&spng, session_key, 32, 0, SP_NORMAL);
   Sponge_finalize(&spng, session_key, 32); //secret
   for(i=0;i<32;i++) session_key[i]^=(our_nonce[i]^their_nonce[i]);

   //computes our public keys and replace our private keys
   get_pubkey(crp_temp, our_p); //P
   memcpy(our_p, crp_temp, 32);
   get_pubkey(crp_temp, our_x); //X
   memcpy(our_x, crp_temp, 32);

   //computes authentificator M=H(K|P|Q|X|Y|Na|Nb)
   Sponge_init(&spng, 0, 0, 0, 0);
   Sponge_data(&spng, aux_key, 16, 0, SP_NORMAL); //K
   Sponge_data(&spng, our_p, 32, 0, SP_NORMAL); //P
   Sponge_data(&spng, their_p, 32, 0, SP_NORMAL); //Q
   Sponge_data(&spng, our_x, 32, 0, SP_NORMAL); //X
   Sponge_data(&spng, their_x, 32, 0, SP_NORMAL); //Y
   Sponge_data(&spng, our_nonce, 32, 0, SP_NORMAL); //Na
   Sponge_data(&spng, their_nonce, 32, 0, SP_NORMAL); //Nb
   Sponge_finalize(&spng, MMAC, 16); //our authentificator

   settimeout(REQTIME); //set timeout
   pkt[0]=(TYPE_ACK | 0x80);
   crp_state=-3; // set state -3: originator wait MAC
   return (lenbytype(TYPE_ACK));
   //output=ACK:  MMAC[16]  (16 bytes)
   //our_p, our_x contains our public keys Q and Y
   //their_p, their_x contain their public keys P and X
   //our_nonce contain Nb, their_nonce contain Na
   //aux_key containe 128 bits verification key
   //session_key contains 128+128 bits session keys
   //au_data contains 128+128 bits onion identificators
   //A -> B: ACK
  }
  else
  {
   web_printf("! Unexpected connection answer received!\r\n");
   disconnect();
   return 0; //check for state sweetable for ans
  }
 }
//*****************************************************************************

 //accept incoming connection manually
 //early prepared answer packet placed in answer[]
 void do_ans(int ok)
 {
  char c;
  int i;
  if(crp_state!=2)
  {
   web_printf("! Unexpected answer command!\r\n");
   return;
  }
  if(!ok) //alters key for rejection
  {
   randFetch(aux_key, 16);
   randFetch(answer+1, 16); //PREF
   randFetch(session_key, 32);
   web_printf("Incoming connection rejected\r\n");
  }
  else web_printf("Incoming connection accepted\r\n");
  i=do_data(answer, (unsigned char*)&c); //encrypt answer, returns pkt len
  if(i>0) do_send(answer, i, c); //send answer
  answer[0]=0;

 }
//*****************************************************************************

 //for acceptor (crp_state==2):
 //input=ACK: MMAC[16]
 //output=ACK: MMAK[16]
 //-----------------------------
 //for originator (crp_state==-3)
 //input=ACK: MMAC[16]
 //optionally output=ACK: MMAK[16] with aux_key
 //-----------------------------
 //for acceptor (crp_state==4):
 //input=ACK: MMAC[16] with aux_key
 //no output
 int go_ack(unsigned char* pkt)
 {
  int i;
  char c;
  unsigned char* buf=pkt+1;

  //ACK packet (16 bytes):
  //MMAC=buf  : authenticator (16 bytes)

  //check for state
  if(crp_state==2)
  {
   //****************************************************
   //-----------------Protocol processed by acceptor (B)
   //****************************************************
   //B accepts ACK packet: MMAC[16]  (16 bytes)
   //and check it and send ACK packet to answer

   //check authentificator M=H(K|P|Q|X|Y|Na|Nb)
   Sponge_init(&spng, 0, 0, 0, 0);
   Sponge_data(&spng, aux_key, 16, 0, SP_NORMAL); //K
   Sponge_data(&spng, their_p, 32, 0, SP_NORMAL); //P
   Sponge_data(&spng, our_p, 32, 0, SP_NORMAL); //Q
   Sponge_data(&spng, their_x, 32, 0, SP_NORMAL); //X
   Sponge_data(&spng, our_x, 32, 0, SP_NORMAL); //Y
   Sponge_data(&spng, their_nonce, 32, 0, SP_NORMAL); //Na
   Sponge_data(&spng, our_nonce, 32, 0, SP_NORMAL); //Nb
   Sponge_finalize(&spng, crp_temp, 16); //our authentificator
   if(memcmp(MMAC, crp_temp, 16))
   {
    web_printf("! Identification failure\r\n");
    disconnect();
    return 0;
   }

   //computes authentificator M=H(K|Q|P|Y|X|Nb|Na)
   Sponge_init(&spng, 0, 0, 0, 0);
   Sponge_data(&spng, aux_key, 16, 0, SP_NORMAL); //K
   Sponge_data(&spng, our_p, 32, 0, SP_NORMAL); //Q
   Sponge_data(&spng, their_p, 32, 0, SP_NORMAL); //P
   Sponge_data(&spng, our_x, 32, 0, SP_NORMAL); //Y
   Sponge_data(&spng, their_x, 32, 0, SP_NORMAL); //X
   Sponge_data(&spng, our_nonce, 32, 0, SP_NORMAL); //Nb
   Sponge_data(&spng, their_nonce, 32, 0, SP_NORMAL); //Na
   Sponge_finalize(&spng, MMAC, 16); //our authentificator

   //Print wordlist (for anti-MitM biometric voice autentification)
   Sponge_init(&spng, 0, 0, 0, 0);
   Sponge_data(&spng, session_key, 16, 0, SP_NORMAL); //K
   Sponge_finalize(&spng, au_data, 4);  //h
   web_printf("Control words: ' %s - %s - %s - %s '\r\n",
    (getword(au_data[0])), (getword(256+au_data[1])),
    (getword(au_data[2])), (getword(256+au_data[3]))
   );
   fflush(stdout);

   //prepare onion handshake prefixes
   au_data[0]=0xFF; //flag
   Sponge_init(&spng, 0, 0, 0, 0);
   Sponge_data(&spng, au_data, 1, 0, SP_NORMAL); //flag
   Sponge_data(&spng, session_key, 16, 0, SP_NORMAL); //Kb
   Sponge_finalize(&spng, au_data, 32);

   //set state -4 for obligate MAC revealing: acceptor ready
   //or state 4 (comleet) for optional revealing
   crp_state=4;  //acceptor compleet IKE
   settimeout(REQTIME); //set timeout
   if(crp_state==4)
   {
    web_printf("Incoming connection is ready\r\n");
    xmemset(aux_key, 0, 16); //clear aux_key
   }
   pkt[0]=(TYPE_ACK | 0x80);
   return (lenbytype(TYPE_ACK));
   //output=ACK:  MMAC[16]  (16 bytes)
  }
  else if(crp_state==-3)
  {
   //****************************************************
   //-----------------Protocol processed by originator (A)
   //****************************************************
   //A accepts ACK packet: MMAC[16]  (16 bytes)
   //check it and send:
        //-optionally key revealing
        //-password au request if password defined
        //-onion au request as a chat packet

   //check authentificator M=H(K|Q|P|Y|X|Nb|Na)
   Sponge_init(&spng, 0, 0, 0, 0);
   Sponge_data(&spng, aux_key, 16, 0, SP_NORMAL); //K
   Sponge_data(&spng, their_p, 32, 0, SP_NORMAL); //Q
   Sponge_data(&spng, our_p, 32, 0, SP_NORMAL); //P
   Sponge_data(&spng, their_x, 32, 0, SP_NORMAL); //Y
   Sponge_data(&spng, our_x, 32, 0, SP_NORMAL); //X
   Sponge_data(&spng, their_nonce, 32, 0, SP_NORMAL); //Nb
   Sponge_data(&spng, our_nonce, 32, 0, SP_NORMAL); //Na
   Sponge_finalize(&spng, crp_temp, 16); //our authentificator
   if(memcmp(MMAC, crp_temp, 16))
   {
    web_printf("! Identification failure\r\n");
    disconnect();
    return 0;
   }

   //Print wordlist (for anti-MitM biometric voice autentification)
   Sponge_init(&spng, 0, 0, 0, 0);
   Sponge_data(&spng, session_key, 16, 0, SP_NORMAL); //K
   Sponge_finalize(&spng, au_data, 4);  //h
   web_printf("Control words: ' %s - %s - %s - %s '\r\n",
    (getword(au_data[0])), (getword(256+au_data[1])),
    (getword(au_data[2])), (getword(256+au_data[3]))
   );
   fflush(stdout);

   //prepare onion handshake prefixes
   au_data[0]=0xFF; //flag
   Sponge_init(&spng, 0, 0, 0, 0);
   Sponge_data(&spng, au_data, 1, 0, SP_NORMAL); //flag
   Sponge_data(&spng, session_key, 16, 0, SP_NORMAL); //Kb
   Sponge_finalize(&spng, au_data, 32);

   crp_state=3; //originator copmpleet IKE
   web_printf("Outgoing connection is ready\r\n");
   //key revealing
   strcpy((char*)pkt, "Key_reveal");
   parseconf((char*)pkt);
   if(pkt[0]=='1')
   {
    memcpy(buf, aux_key, 16); //Reveal aux_key
    pkt[0]=(TYPE_ACK | 0x80);
    i=do_data(pkt, (unsigned char*)&c); //process it
    if(i>0) do_send(pkt, i, c); //send it
    web_printf("MAC key revealed\r\n");
   }
   xmemset(aux_key, 0, 16); //clear aux_key

   //init password identification (by common preshared passphrase)
   pkt[0]=0;  //use passphrase from password[32]
   i=do_au(pkt); //prepare au request if password specified
   if(i>0) i=do_data(pkt, (unsigned char*)&c); //process it
   if(i>0) do_send(pkt, i, c); //send it

   //init onion verification
   if ((!onion_flag) || (rc_level==-1)) return 0; //check conditions
   xmemset(pkt, 0, 256); //clear buffer
   pkt[0]=TYPE_CHAT|0x80;  //prepare chat packet type
   sprintf((char*)(pkt+1), "-W%s", our_onion);
   return (lenbytype(TYPE_CHAT));  //will be sended
  }
  else if(crp_state==4)
  {
   //****************************************************
   //-----------------Protocol processed by acceptor (B)
   //****************************************************
   //B accepts ACK packet: MMAC[16]  (16 bytes) with revealed key
   //and send check it

   //check aux_key revealing
   if(memcmp(MMAC, aux_key, 16))
   {
    web_printf("! Revealed MAC key does not match!\r\n");
   }
   else
   {
    crp_state=4; //set compleet state
    web_printf("MAC key revealed by remote side\r\n");
    xmemset(aux_key, 0, 16); //clear aux_key
   }
  }
  else
  {
   web_printf("! Unexpected connection ACK received!\r\n");
   disconnect();
   return 0; //check for state sweetable for ans
  }
 return 0;
 }
//*****************************************************************************

 //generate au packet
 //input: passphrase
 //output: au_req packet
 //returns len
 int do_au(unsigned char* pkt)
 {
  //Initiator of password authentication (sender) S:
  //Let originators password: pass|p|q

  unsigned char org=0; //flag 1 for originator of call (A) and 0 for acceptor (B)
  unsigned char* buf=pkt+1;

  //overwrite using specified passphrase
  if(pkt[0])
  {
   strncpy(password, (char*)pkt, 31);
   password[31]=0;
  }
  //check password minimum 3 chars
  if(strlen(password)<3) return 0;
  //check for status and set originator flag
  if(crp_state<2)
  {
   web_printf("! Authentication impossible: connection is not established!\r\n");
   return 0;
  }
  else if(crp_state==3) org=(!org); //set originator flag

  web_printf("Password authentication is initiated\r\n");
  //computes first autentificator AU1s=H(org|s|pass)
  Sponge_init(&spng, 0, 0, 0, 0);
  Sponge_data(&spng, &org, 1, 0, SP_NORMAL);
  Sponge_data(&spng, session_key, 16, 0, SP_NORMAL);
  Sponge_data(&spng, (const BYTE*)password, strlen(password)-2, 0, SP_NORMAL);
  Sponge_finalize(&spng, buf, 18);
  pkt[0]=(TYPE_AUREQ | 0x80);
  return (lenbytype(TYPE_AUREQ)); //16
 }
//*****************************************************************************

 int go_aureq(unsigned char* pkt)
 {
  //Receivers side (R):
  //Let receivers password: pass|q|p

  //received: AU1s (16 bytes)
  unsigned char org=1;  //call originator's flag
  unsigned char* buf=pkt+1;
  unsigned char work[18];

  //check for packet type
  if(pkt[0]!=(TYPE_AUREQ|0x80))
  {
   web_printf("! Received packet is not an autentification request type!\r\n");
   return 0;
  }
  //check password minimum 3 chars
  if(strlen(password)<3)
  {
   web_printf("Remote party suggests to execute password authentication\r\n");
   return 0;
  }
  //check for status and set originator flag
  if(crp_state<2)
  {
   web_printf("! Authentication is impossible: connection is not established!\r\n");
   return 0;
  }
  else if(crp_state==3) org=(!org);

  //check first autentificator AU1s=?H(org|s|pass)
  Sponge_init(&spng, 0, 0, 0, 0);
  Sponge_data(&spng, &org, 1, 0, SP_NORMAL);
  Sponge_data(&spng, session_key, 16, 0, SP_NORMAL);
  Sponge_data(&spng, (const BYTE*)password, strlen(password)-2, 0, SP_NORMAL); //without last two chars
  Sponge_finalize(&spng, work, 18);
  if(memcmp(buf, work, 18))
  {
   web_printf("! Authentication failure!\n");
   fflush(stdout);
   return 0;
  }
  org=(!org);

  //computes first receiver's autentificator AU1r=H(org+16|s|pass)
  org+=16;
  Sponge_init(&spng, 0, 0, 0, 0);
  Sponge_data(&spng, &org, 1, 0, SP_NORMAL);
  Sponge_data(&spng, session_key, 16, 0, SP_NORMAL);
  Sponge_data(&spng, (const BYTE*)password, strlen(password)-2, 0, SP_NORMAL); //without last two chars
  Sponge_finalize(&spng, buf, 18);

  //computes second receiver's autentificator AU2r=H(org+16|s|pass|p)
  Sponge_init(&spng, 0, 0, 0, 0);
  Sponge_data(&spng, &org, 1, 0, SP_NORMAL);
  Sponge_data(&spng, session_key, 16, 0, SP_NORMAL);
  Sponge_data(&spng, (const BYTE*)password, strlen(password)-2, 0, SP_NORMAL);   //without last two chars
  Sponge_data(&spng, (const BYTE*)password+strlen(password)-1, 1, 0, SP_NORMAL); //add last char
  Sponge_finalize(&spng, buf+18, 6); //

  pkt[0]=(TYPE_AUANS | 0x80);
  return (lenbytype(TYPE_AUANS)); //24
 }
//*****************************************************************************

 int go_auans(unsigned char* pkt)
 {
  //initiators side S:

  //received AU1r, AU2r (18+6 bytes)
  unsigned char org=1;
  unsigned char* buf=pkt+1;
  unsigned char work[18];

  //check for packet type
  if(pkt[0]!=(TYPE_AUANS|0x80))
  {
   web_printf("! Received packet is not an autentification answer type!\r\n");
   return 0;
  }
  //check password minimum 3 chars
  if(strlen(password)<3) return 0;

  //check for status and set originator flag
  if(crp_state<2)
  {
   web_printf("! Sending authentication answer is impossible: connection is not established!\r\n");
   return 0;
  }
  else if(crp_state==3) org=(!org);

  //check first R's autentificator AU1r =? H(org+16|s|pass)
  org+=16;
  Sponge_init(&spng, 0, 0, 0, 0);
  Sponge_data(&spng, &org, 1, 0, SP_NORMAL);
  Sponge_data(&spng, session_key, 16, 0, SP_NORMAL);
  Sponge_data(&spng, (const BYTE*)password, strlen(password)-2, 0, SP_NORMAL); //without last two chars
  Sponge_finalize(&spng, work, 18);
  if(memcmp(buf, work, 18))
  {
   web_printf("! Authentication failure!\n");
   fflush(stdout);
   return 0;
  }

  //check secont R's authentificator AU2r =? H(org+16|s|pass|p)
  Sponge_init(&spng, 0, 0, 0, 0);
  Sponge_data(&spng, &org, 1, 0, SP_NORMAL);
  Sponge_data(&spng, session_key, 16, 0, SP_NORMAL);
  Sponge_data(&spng, (const BYTE*)password, strlen(password)-1, 0, SP_NORMAL); //without last char
  Sponge_finalize(&spng, work, 6);
  if(memcmp(buf+18, work, 6)) web_printf("! Authentication UNDER PRESSING!\r\n");
  else web_printf("Authentication OK\r\n");
  fflush(stdout);
  org-=16;

  org=(!org);

  //computes second S's authentificator AU2s=H(org|s|pass|q)
  Sponge_init(&spng, 0, 0, 0, 0);
  Sponge_data(&spng, &org, 1, 0, SP_NORMAL);
  Sponge_data(&spng, session_key, 16, 0, SP_NORMAL);
  Sponge_data(&spng, (const BYTE*)password, strlen(password)-2, 0, SP_NORMAL); //without last two chars
  Sponge_data(&spng, (const BYTE*)password+strlen(password)-1, 1, 0, SP_NORMAL); //add last char
  Sponge_finalize(&spng, buf, 6);

  pkt[0]=(TYPE_AUACK | 0x80);
  return (lenbytype(TYPE_AUACK)); //6
 }
//*****************************************************************************

 int go_auack(unsigned char* pkt)
 {
  //Receiver side R:

  //received: AU2s (6 bytes)
  unsigned char org=1;
  unsigned char* buf=pkt+1;
  unsigned char work[6];

  //check for packet type
  if(pkt[0]!=(TYPE_AUACK|0x80))
  {
   web_printf("! Received packet is not a autentification ACK type!\r\n");
   return 0;
  }
  //check password minimum 3 chars
  if(strlen(password)<3)
  {
   web_printf("! Going authentication ACK impossible: connection not established!\r\n");
   return 0;
  }
  //check for status and set originator flag
  if(crp_state<3) return 0;
  else if(crp_state==3) org=(!org);

  //check second S's autentificator AU2s =? H(org|s|pass|q)
  Sponge_init(&spng, 0, 0, 0, 0);
  Sponge_data(&spng, &org, 1, 0, SP_NORMAL);
  Sponge_data(&spng, session_key, 16, 0, SP_NORMAL);
  Sponge_data(&spng, (const BYTE*)password, strlen(password)-1, 0, SP_NORMAL); //without last char
  Sponge_finalize(&spng, work, 6);
  if(memcmp(buf, work, 6)) web_printf("! Authentication UNDER PRESSING!\r\n");
  else web_printf("Authentication OK\r\n");
  fflush(stdout);
  pkt[0]=0;
  return 0;
 }
//*****************************************************************************

 //process chat packet
 //input: pkt
 //output pkt - packet for answer, returns outgoing packets length >0
 //or returns 0
 int go_chat(unsigned char* pkt)
 {
  pkt[256]=0;

  //check for option defines
  if(pkt[1]=='-') //invites
  {
   if((pkt[2]=='U')||(pkt[2]=='S')) //TCP<>UDP invite
   {
    setaddr((char*)pkt);//check for TCP->UDP invite with internal adress
   }
   else if(pkt[2]=='O')
   {
    //Switch UDP->TCP
    stopudp();
    web_printf(" by remote command\r\n");
   }
   else if(pkt[2]=='W') //onion invites
   {
    if(!pkt[3]) //empty onion invite: originator must send answer
    {
     if((crp_state!=3)||(!onion_flag)||(rc_level==-1)) return 0; //processed by originator only
     xmemset(pkt, 0, 256);
     pkt[0]=TYPE_CHAT|0x80;
     sprintf((char*)(pkt+1), "-W%s", our_onion);
     return (lenbytype(TYPE_CHAT)); //send chat type packet with our onion adress (invite)
    }
    else
    {
     //their onion address received: reconnect outgoing connection
     //copy it to their_onion (if less then 31 bytes)
     if(strlen((const char*)pkt+3)<31)
     {
         strncpy(their_onion, (const char*)pkt+3, 31);
         their_onion[31]=0;
     }

     //check for permission, crp_state, onion_flg incoming tcp
     //also check for change T value>=0, tcp_insock_flag==SOCK_INUSE
     if(crp_state>2) reconecttor(); //reconect outgoing tcp to specified onion address
    }
   }
  }
  else web_printf(">%s\r\n", pkt+1); //text mesage
  return 0;
 }
//*****************************************************************************

 //process data packet before sending
 //input: pkt - packet type in pkt[0] and data
 //output: buf- packet for sending, type - for replacing pkt[0] for udp
 //returns length for sending in bytes
 int do_data(unsigned char* pkt, unsigned char* udp)
 {
  unsigned char* buf=pkt+1;
  int len=0;
  unsigned char c=0;
  unsigned char type=0;

  (*udp)=pkt[0];
  //check for packet's type
  if( (0xC0&pkt[0])==0xC0 ) type=TYPE_MELPE; //check for TYPE_MELPE
  else if(0x80&pkt[0]) type=(0x1F&pkt[0]);//cbr types
  else type=TYPE_VBR;  //set TYPE_VBR

  //find packet's length
  if(0x80&pkt[0]) len=lenbytype(type);
  else len=0x7F&pkt[0];

  //check for encryption needed: return unencrypted packet
  if(  (type>=TYPE_INV)&&(type<TYPE_VBR) ) return (len+MACLEN+1);
  //check for status
  if(crp_state<3) return 0;

  //add packet counter to symmetric encryption key
  memcpy(session_key+32, &out_ctr, 4);
  //add call originator flag (0-originator of call, 1-acceptor)
  if(crp_state==3) session_key[36]=0; else session_key[36]=1;
  //initialise sponge by key and counter  for encryption
  Sponge_init(&spng, session_key+16, 21, 0, 0);
  //encrypt packet in duplex mode
  Sponge_data(&spng, buf, len, buf, SP_NOABS);

  //encrypt 5 first bits for melpe
  if(type==TYPE_MELPE)
  {
   Sponge_data(&spng, 0, 1, &c, SP_NORMAL); //squeezing extra byte
   c&=0x1F;
   pkt[0]^=c;
  }

  //initilize sponge and computes MAC
  Sponge_init(&spng, 0, 0, 0, 0);
  //absorb symmetric encryption key
  Sponge_data(&spng, session_key+16, 21, 0, SP_NORMAL);
  //absorb all packet data for MAC
  Sponge_data(&spng, pkt, len+1, 0, SP_NORMAL);
  //add 4 bytes hash to end of packet
  Sponge_finalize(&spng, buf+len, MACLEN);

  //output last bits of counter for udp-packets
   c=0x7F&out_ctr; // 7 LSB of counter
   if((0xC0&pkt[0])==0xC0) //for melpe packet we uses only 2 LSBs
   {
    c<<=5; //2 counters's LSBs as bits 6-5
    c+=(0x1F&pkt[0]); //bits 4-0 are melpe data bits
   }
   if(0x80&pkt[0]) c|=0x80; //constant packet length flag
  (*udp)=c; //output byte for replacing first packet's byte in UDP-packets

  //increment outgoing packet counter (one way, never back!)
  out_ctr++;
  //returns new packet length
  return len+MACLEN+1;
 }
//*****************************************************************************

 //Process incoming data packet
 //input: udp or tcp pkt in buf,
 //length for udp(512+actual) or for tcp(actual len)
 //output: ready packet (type in pkt[0])
 //returns: clear data length (without header, mac etc.)
 int go_data(unsigned char* pkt, short len)
 {
  unsigned char type=0;
  char c=0;
  unsigned char* buf=pkt+1;
  unsigned int ctr=in_ctr; //current decryption counter
  unsigned char mac[MACLEN];

  //check for incoming packet UDP or TCP (for TCP len<512)
  if((len)&&(len<512)) //udp packets: recognize type by len, sync counter
  {
   if(len<5) return 0;  //minimal packet is 5 bytes for TYPE_AUACK
   len=len-MACLEN-1; //skip header byte and mac
   if(0x80&pkt[0]) type=typebylen(len); //cbr packets
   else type=TYPE_VBR; //vbr packets
   if(type==TYPE_UNKNOWN) return 0; //length not matched

   //check for encryption need, return data len including mac tail
   if(  (type>=TYPE_INV)&&(type<TYPE_VBR) ) return (len);

   //restore CTR by first byte
   c=0x7F&pkt[0]; //7 LSB of their counter from first byte of packet
   if(type==TYPE_MELPE) //only 2 synchro bits uses  for melpe
   {
    c>>=5;
    c-=(ctr&0x03);
    if(c<0) c+=4;
    pkt[0]|=0xE0; //sets bits for TYPE_MELPE
   }
   else //all other types: 7 bits of counter
   {
    if(type!=TYPE_VBR) pkt[0]=type|0x80; //set type on place of counter bits
    else pkt[0]=len;
    c-=(ctr&0x7F);
    if(c<-0x3F) c+=0x80;
   }
   //sync our incoming counter by their outgoing counter
   if(c==1) redundant=1; else redundant=0;
   ctr+=c;
  }

  //TCP incoming packet
  else
  {
   if(0x80&pkt[0]) //cbr types
   {
    if( (0xC0&pkt[0])==0xC0 ) type=TYPE_MELPE; //check for TYPE_MELPE
    else type=(0x1F&pkt[0]); //type from first byte
    len=lenbytype(type); //set data length by type
   }
   else //vbr type
   {
    len=0x7F&pkt[0]; //set data length from first byte
    type=TYPE_VBR;  //set TYPE_VBR
   }
  }

  //check for encryption need, return data len including mac tail
  if(  (type>=TYPE_INV)&&(type<TYPE_VBR) ) return (len);

  //check for state
  if(crp_state<2) return 0;

  //add packet counter to symmetric encryption key
  memcpy(session_key+32, &ctr, 4);
  //add originator flag
  if(crp_state==3) session_key[36]=1; else session_key[36]=0;
  Sponge_init(&spng, 0, 0, 0, 0);
  //absorb symmetric encryption key, counter, originator
  Sponge_data(&spng, session_key+16, 21, 0, SP_NORMAL);
  //absorb all packet data for MAC
  Sponge_data(&spng, pkt, len+1, 0, SP_NORMAL);
  //computes 4 bytes hash
  Sponge_finalize(&spng, mac, MACLEN);
  //compare mac
  if(memcmp(mac, buf+len, MACLEN))
  {
   bad_mac++; //counter of bad autentifications
   return 0;
  }
  bad_mac=0; //autentification OK - clear bad counter

  //initialise sponge by key, counter and originator  for encryption
  Sponge_init(&spng, session_key+16, 21, 0, 0);
  //decrypt packet in duplex mode
  Sponge_data(&spng, buf, len, buf, SP_NOABS);
  //decrypt 5 first bits for melpe
  if(type==TYPE_MELPE)
  {
   Sponge_data(&spng, 0, 1, (BYTE*)&c, SP_NORMAL); //squeezing extra byte
   c&=0x1F;
   pkt[0]^=c;
  }
  //finalize sponge
  Sponge_finalize(&spng, 0, 0);

  //update decryption counter
  in_ctr=ctr+1;

  //check for incoming connection too slow while onion doubling uses
  if(crp_state>2) checkdouble();
  return len;
 }
//*****************************************************************************

 //generate key packet for publication
 //input: buf - key name for init or empty for next
 //output: buf - packet to sending
 //returns: packet length (501 for reg, 505 for end)
 //-Kname
 int do_key(unsigned char* buf)
 {
  int i, len;
  char str[256];
  char str1[64];

  //check for status
  if(crp_state<3) return 0;

  //parse keyname from command
  str[0]='K';
  i=get_opt((char*)buf, str);
  if(i>0) //name specified: this is initial request
  {
   //open specified public key file in keys folder
   if(F) //close old file if it is opened (F is global value!)
   {
    fclose(F);
    F=0;
   }
   sprintf(str1, "%s%s", KEYDIR, str); //add path
   if(!(F = fopen(str1, "rb" ))) //open specified keyfile
   {
    web_printf("! Key '%s' not found!\r\n", str1);
    return 0;
   }
  }
  else if(!F) //file not specified but not opened early
  {
   web_printf("! Input error\r\n", str1);
   return 0;
  }

  //read data block 500 bytes
  buf[0]=(TYPE_KEY | 0x80); //set packet type for keydata
  len=lenbytype(TYPE_KEY);  //set packet's length
  for(i=1;i<=len;i++)  //load up to 500 bytes untill eof ocured
  {
   buf[i]=getc(F);
   if(feof(F)) break;
  }
  if(i>len) return len; //no eof: this is not last packet

  //this is last packet: finalize key publication
  for(;i<=len;i++) buf[i]=0; //fill zeroes to end of buffer
  fseek(F,0, SEEK_SET); //renew key file for calculates hash
  Sponge_init(&spng, 0, 0, 0, 0); //initialize hash for stamp
  while(!feof(F)) //process file byte-by-byte
  {
   (*buf) = getc(F);  //next char from keyfile
   if(!feof(F)) Sponge_data(&spng, buf, 1, 0, SP_NORMAL);  //absorbing all file data
  }
  Sponge_finalize(&spng, buf+len+1, 4); //put hash after data area
  fclose(F); //close keyfile
  F=0;

  //send finaling packet
  buf[0]=(TYPE_KEYLAST | 0x80); //set type for last key packet
  len=lenbytype(TYPE_KEYLAST);
  return len; //it's fixed length
 }
//*****************************************************************************

 //process incoming key or keylast packet
 //input: decrypted packet in buf
 //output: no
 int go_key(unsigned char* buf)
 {
  int i, len;
  char c;
  char str[256];
  char str1[256];
  char str2[256];
  unsigned char keyid[16];
  unsigned char lastid[16];

  //check for packet type
  if( (buf[0]!=(TYPE_KEY|0x80)) && (buf[0]!=(TYPE_KEYLAST|0x80))  ) return 0;
  //check for status
  if(crp_state<3) return 0;

  //open temporary public key file in keys folder
   if(!F1) //open file if it is not already opened (F is global value!)
   {
    sprintf(str, "%s%s", KEYDIR, "temp"); //add path
    if(!(F1 = fopen(str, "w+b" ))) //open specified keyfile
    {
     web_printf("! Error accepting key!\r\n", str);
     return 0;
    }
   }

   //save data from received packet
   len=lenbytype(TYPE_KEY);
   for(i=1;i<=len;i++)
   {
    if(!buf[i]) break; //only non-zero data in text key file!
    fputc(buf[i], F1);
   }
   if((0x1F&buf[0])!=TYPE_KEYLAST) return 0; //if this is not last key data

   //this was a last key data: computes keyid
   i=ftell(F1);
   if((i<44)||(i>5000)) //key length restriction
   {
    fclose(F1);
    return 0;
   }
   fseek(F1,0, SEEK_SET); //renew key file for calculates hash
   Sponge_init(&spng, 0, 0, 0, 0); //initialize hash for stamp
   while(!feof(F1)) //process file byte-by-byte
   {
    c = getc(F1);  //next char from keyfile
    if(!feof(F1)) Sponge_data(&spng, (const BYTE*)&c, 1, 0, SP_NORMAL);  //absorbing all file data
   }
   Sponge_finalize(&spng, keyid, 16); //computes hash (key ID)
   fclose(F1); //close keyfile
   F1=0;
   if(memcmp(buf+len+1, keyid, 4)) return 0; //bad checksum

   //get key owner
   i=get_keyinfo("temp", str); //get info atring from key
   if(i<1) return 0;
   str[128]=0; //truncate

   //get nick of key owner
   i=get_nickname(str, str1);
   if(i<1) return 0;

   //search for last key from this owner in address book
   i=get_bookstr(BOOK, str1, str2);
   if(i>0) //if exist
   {
    i=b64dstr(str2, lastid, 16); //extract ID of existed key
    if(i==16) //if it is valid
    { //compare last id with current
     if(!memcmp(keyid, lastid,16))
     {
      web_printf("! Key received: '%s', already exists\r\n", str1); //notification
      fflush(stdout);
      //delete temp file
      sprintf(str, "%s%s", KEYDIR, "temp"); //add path
      remove(str);
      return 0; //this is replay, exit
     }
    }
   }

 //add new key to addressbook
   sprintf(str2, "%s%s", KEYDIR, BOOK); //add path
   //open address book for append
   if(!(F1 = fopen(str2, "at" ))) //open specified bookfile
   {
    web_printf("! Error accepting key!\r\n", str);
    return 0;
   }
   //add key string for adressbook
   b64estr(keyid, 16, str2); //encode b64 keyid
   fprintf(F1,"[%s] %s -L %s\n", str1, str2, str); //add
   fclose(F1);
   F1=0;

 //rename temporary file to keyfile by owner's nick
   sprintf(str, "%s%s", KEYDIR, "temp"); //add path
   sprintf(str2, "%s%s", KEYDIR, str1); //add path
   i=rename(str, str2);
   if(!i) //if new name applied
   {
    web_printf("Key received from '%s'\r\n", str1); //notification
    fflush(stdout);
   }
   else remove(str);
   return 0;
 }
//*****************************************************************************

 //generates ping request packet
 //returns data length
 int do_syn(unsigned char* pkt)
 {
  //check for status
  if(crp_state<3) return -1;

  //set answer or request syn packet type
  if( (0x1F&pkt[0])==TYPE_SYN ) pkt[0]|=(TYPE_SYN | 0xA0); //answer
  else if(pkt[0])
  {
   pkt[0]=(TYPE_SYN | 0x80); //request
   gettimeofday(&TM, NULL); //save sending time
  }
  else
  {
   pkt[0]=(TYPE_SYN | 0x80); //final
   out_ctr=0xFFFFFFFF; //finalise counter
  }

   //present our outgoing counter
  memcpy(pkt+1, &out_ctr, 4);
  //add packet counter to symmetric encryption key
  memcpy(session_key+32, &out_ctr, 4);
  //add originator info
  if(crp_state==4) session_key[36]=3; else session_key[36]=2;
  Sponge_init(&spng, 0, 0, 0, 0);
  //absorb symmetric encryption key, ctr, originator
  Sponge_data(&spng, session_key+16, 21, 0, SP_NORMAL);
  //absorb syn type
  Sponge_data(&spng, pkt, 1, 0, SP_NORMAL);
  //add 4 bytes hash to end of packet
  Sponge_finalize(&spng, pkt+5, 4);
  return (lenbytype(TYPE_SYN)); //8
 }
//*****************************************************************************

 //process ping packet
 //returns ping answer packet or 0
 int go_syn(unsigned char* pkt)
 {
  //received: CTR, MAC (4+4 bytes)
  struct timeval time1;
  int t;

  //check for type
  if((0x9F&pkt[0])!=(TYPE_SYN|0x80)) return -2;
  //check for status
  if(crp_state<3) return -1;
  //add packet counter to symmetric encryption key
  memcpy(session_key+32, pkt+1, 4);
  //add originator info
  if(crp_state==4) session_key[36]=2; else session_key[36]=3;
  Sponge_init(&spng, 0, 0, 0, 0);
  //absorb symmetric encryption key, ctr, originator
  Sponge_data(&spng, session_key+16, 21, 0, SP_NORMAL);
  //absorb syn type
  Sponge_data(&spng, pkt, 1, 0, SP_NORMAL);
  //4 bytes hash
  Sponge_finalize(&spng, pkt+1, 4);
  //check mac
  if(!memcmp(pkt+5, pkt+1, 4))
  {
   //check presented conter is greater then our incoming counter
   if(   (*(unsigned int*)(session_key+32))  >  in_ctr  )
   //correct out incoming counter for their outgoing counter
   memcpy(&in_ctr, session_key+32, 4);
  }

  //ckeck for finalize packet
  if(in_ctr==0xFFFFFFFF)
  {
   disconnect();
   return -4; //fin packet
  }

  //if received packet is answer: computes latency
  if(0x20&pkt[0])
  {
   gettimeofday(&time1, NULL); //get our time now
   //computes differencies with our time now
   //and our time of request sending
   t=(int)(time1.tv_usec - TM.tv_usec); //microseconds
   if(t<0) //seconds bondaries
   {
    t=1000000-t;
    time1.tv_sec--;
   }
   t/=1000; //to milliseconds
   t+=(1000*(time1.tv_sec-TM.tv_sec)); //add seconds
   t/=2; //one-way latency is a half of two-way
   web_printf("Latency is %d mS ", t);
   fflush(stdout);
   return 0; //no answer
  }
  else return ( do_syn(pkt) );  //generate answer
 }
//*****************************************************************************

 //generates invite
 //pkt[0]=0 for 'busy' inv
 int do_inv(unsigned char* pkt)
 {
  if(!pkt[0]) xmemset(pkt+1, 0, 12); //clear for 'busy' inv
  else
  {
   if(crp_state<3) return 0; //check for crypto ready
   else if(crp_state==3) memcpy(pkt+1, au_data, 12);
   else memcpy(pkt+1, au_data+16, 12);
  }
  pkt[0]=TYPE_INV|0x80; //set type
  return (lenbytype(TYPE_INV)); //12
 }
//*****************************************************************************

 //process incoming invite
 int go_inv(unsigned char* pkt)
 {
  unsigned char work[12];
  int i;

  if(pkt[0]!=(TYPE_INV|0x80)) return 0; //check for pkt type
  xmemset(work, 0, 12);  //for checking zeroed inv
  if(!memcmp(pkt+1, work, 12)) return -1; //remote is 'busy'
  if(crp_state<3) return 0; //encryption not ready
  if(crp_state==3) i=memcmp(pkt+1, au_data+16, 12);
  else i=memcmp(pkt+1, au_data, 12);
  return (!i); //1 - invite OK
 }
//*****************************************************************************

 //packets wrapper
 //input: decrypted packet, type int pkt[0]
 //output: ret=0 for no operation
 // ret>0 and pkt for sending answer
 int go_pkt(unsigned char* pkt, int len)
 {
  unsigned char type=0;
  //find type
  if(0x80&pkt[0]) type=typebylen(len); //cbr packets
  else type=TYPE_VBR; //vbr packets
  if(type==TYPE_UNKNOWN) return 0;
  //wrap procedure for incoming packet type, returns length of our answer or 0
  if(  (type<=TYPE_SPEEX)||(type==TYPE_VBR) )
  {//voice processing
   go_snd(pkt); //pass packet to codec wrapper
   return 0;
  }
  if(type==TYPE_SYN) return(go_syn(pkt)); //ping and synchro
  if(type==TYPE_CHAT) return(go_chat(pkt)); //chat and invites
  if((type==TYPE_KEY)||(type==TYPE_KEYLAST)) return(go_key(pkt)); //key publication
  if(type==TYPE_AUREQ) return(go_aureq(pkt)); //autentification request
  if(type==TYPE_AUANS) return(go_auans(pkt)); //autentification answer
  if(type==TYPE_AUACK) return(go_auack(pkt)); //autentification accept
  if(type==TYPE_REQ) return(go_req(pkt)); //key agreement step 1
  if(type==TYPE_ANS) return(go_ans(pkt));  //key agreement step 2
  if(type==TYPE_ACK) return(go_ack(pkt));  //key agreement step 3
  //if(type==TYPE_DAT) return(go_dat(pkt));  //data packet
  return 0; //not sweetable type
 }




