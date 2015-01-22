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

#include "libcrp.h"   //cryptographic library: EC25519, Keccak SpongeWrap +SPRNG, base64 etc.
#include "tcp.h"      //transport (sockets layer)
#include "crypto.h"   //cryptographic protocols (key agreement, autentification etc.)
#include "cntrls.h"   //users interface (menu, commands etc.)
#include "audio.h"    //audio low_level input/output: alsa for Linux, wave for Windows  
#include "codecs.h"   //audio processing (codecs wrapper, packetizer, jitter buffer etc.)

extern char crp_state; //state of crypto protocol (from crypto.c)
extern char sound_loop; //flag of sound selftest (from cntrls.c)

//Main procedure is one-threaded (except wave thread for win32) cycle
//asynchronosly poll sound input device, network sockets and keyboard input
int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;
 unsigned char bbuf[540]; //work buffer
 int i, job=0;
 char c;

 randInit(0, 0); //SPRNG initialization
 loadmenu(); //loading menu items from file
 doclr(); //clear command string
 sock_init(); //initialize network interface
 setaudio(); //load default audio settings from config file
 sp_init(); //initialize audio codecs
 if(!soundinit()) //initialize audio
 {
  printf("Error initing sound!\r\n");
  return 0;
 }
 tty_rawmode(); //initialize console

 //main cicle
 while(1)
 {
  //process sound output
  job=go_snd(0);
  //process sound input
  i=do_snd(bbuf); //check for sount packet ready
  if(i>1) //if sound packet encoded
  {
   if(sound_loop) go_snd(bbuf); //sound self-test: decode and play packet
   else if(crp_state>2) //or send sound to remote
   {
    i=do_data(bbuf, (unsigned char*)&c); //encrypt packet, returns pkt len
    if(i>0) do_send(bbuf, i, c); //send packet
   }
  }
  if(i) job=1; //set flag for audio job
  //process network input
  i=do_read(bbuf); //read pkt from network, returns <0 if no pkt or pkt len
  if(i) job+=2;
  if(i>0) i=go_data(bbuf, i); //decrypt pkt, specifies length for udp, returns data len
  if(i>0) i=go_pkt(bbuf, i); //process pkt, return data len of answer
  if(i>0) i=do_data(bbuf, (unsigned char*)&c); //encrypt answer, returns pkt len
  if(i>0) do_send(bbuf, i, c); //send answer

  //process console input
  i=do_char(); //process char or command
  if(i) job+=4;
  if(i==1) break; //break command
  if(!job) psleep(1);
 }
 
 printf("Bye!!!\r\n");
 fflush(stdout);
 tty_normode(); //back to normal terminal mode
 disconnect(); //terminate all network connections
 soundterm(); //stop audio
 sp_fine(); //finalize audio codecs
 randDestroy(); //finalize SPRNG ans save seed
 return 0;
}





  
  
