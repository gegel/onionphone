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
//This utility provides key management for X-tool assymetric keys
//ECDH25519 keys used (256 bits secret and puplic)

//Generates new keypair (binary secret key file
//and b64-text public keyfile includes additional info: name, options etc.
//After pair is generared owner must sign text of public key file by PGP
//and put signature in the end of keyfile
//After this keyfile content must not be edit, even one byte!
//But we can live rename keyfile before any using

//Add public keys to addressbook: some additional option specified for application
//can be added with key

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libcrp.h"


#define MAX_NAME 8 //max length of filename of key
#define MAX_BOOK 12 //max length of folename of address book
#define MAXINFO 128 //max length of info/arg string from key
#define KEYDIR "keys/" //folder for keys and addresbook files
#define BOOK "contacts.txt" //default addressbook filename

 KECCAK512_DATA spng;

//computes access from password string (PKDF)
 void set_access(char* pas, unsigned char* access)
 {
  int i;
  #define PKDFCOUNT 32768
  
   Sponge_init(&spng, 0, 0, 0, 0);
   Sponge_data(&spng, (const BYTE*)"$OnionPhone_salt", 16, 0, SP_NORMAL);
   for(i=0; i<PKDFCOUNT; i++)
   Sponge_data(&spng, (const BYTE*)pas, strlen(pas), 0, SP_NORMAL);
   Sponge_finalize(&spng, access, 16);
 }
 //******************************************************

//Loding 256 bits secret key from encrypted file
 int get_seckey(const char* name, char* oldpas, unsigned char* seckey)
 {
   char str[256];
   unsigned char aukey[64]; //access|nonce|mac|our_mac
   FILE* fl=0;
   int i;

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

   if(i) //process encrypted secret key
   {
     //check old key
    if(!oldpas)
    {
     printf("Secret key passphrase must be specified!\r\n");
     return 0;
    }
     set_access(oldpas, aukey);  //computes temporary access to key
     //computes mac
     Sponge_init(&spng, 0, 0, 0, 0);
     Sponge_data(&spng, seckey, 32, 0, SP_NORMAL); //absorb secret key body
     Sponge_data(&spng, aukey, 32, 0, SP_NORMAL); //absorb access|nonce
     Sponge_finalize(&spng, aukey+48, 16); //our mac

    //check mac
    if(memcmp(aukey+32, aukey+48, 16))
    {
     printf("Wrong secret key passphrase!\r\n");
     return 0;
    }

    //decrypt secret key's body
    Sponge_init(&spng, aukey, 32, 0, 0); //absorb access|nonce
    Sponge_data(&spng, seckey, 32, seckey, SP_NOABS); //decrypt secret key in duplex mode
    memset(aukey, 0, 64);
    Sponge_finalize(&spng, 0, 0);
   }
   return 32; //key bytes 
 }
//*************************************************************

//Saving 256 bits secret key to encrypted file
 int save_seckey(char* name, char* pass, unsigned char* secret)
 {
  char str[256];
  unsigned char aukey[64]; //access|nonce|mac|our_mac
  FILE* fl=0;

  //save to binary seckey file
  sprintf(str, "%s%s.sec", KEYDIR, name);
  if(!(fl = fopen(str, "wb" )))
  {
   printf("Error creating secret file!\r\n");
   return 0;
  }
  if(!pass) //write unencrypted secret key
  {
   if(32!=fwrite(secret,1,32,fl))
   {
    printf("Error writing secret file!\r\n");
    fclose(fl);
    if(!remove(str)) printf("Error removing bad secret file! Try do it manually\r\n");
    return 0;
   }
  }
  else //encrypt
  {
   memcpy(aukey, secret, 32); //key body
   set_access(pass, aukey+32); //acess
   randFetch(aukey+48, 16);  //nonce
   //encrypt secret key's body
   Sponge_init(&spng, aukey+32, 32, 0, 0); //absorb access|nonce
   Sponge_data(&spng, aukey, 32, aukey, SP_NOABS); //decrypt secret key in duplex mode
   //computes mac
   Sponge_init(&spng, 0, 0, 0, 0);
   Sponge_data(&spng, aukey, 32, 0, SP_NORMAL); //absorb secret key body
   Sponge_data(&spng, aukey+32, 32, 0, SP_NORMAL); //absorb access|nonce
   Sponge_finalize(&spng, aukey+32, 16); //mac
   //write to file
   if(32!=fwrite(aukey,1,32,fl)) //encrypted key body
   {
    printf("Error writing secret file!\r\n");
    fclose(fl);
    if(!remove(str)) printf("Error removing bad secret file! Try do it manually\r\n");
    return 0;
   }
   if(16!=fwrite(aukey+48,1,16,fl)) //nonce
   {
    printf("Error writing secret file!\r\n");
    fclose(fl);
    if(!remove(str)) printf("Error removing bad secret file! Try do it manually\r\n");
    return 0;
   }
   if(16!=fwrite(aukey+32,1,16,fl)) //mac
   {
    printf("Error writing secret file!\r\n");
    fclose(fl);
    if(!remove(str)) printf("Error removing bad secret file! Try do it manually\r\n");
    return 0;
   }
  }
  fclose(fl);
  return 1;
 }

//***************************************************
//Main procedure
int main(int argc, char **argv)
{
 char newkey=0; //-G key generation mode flag
 char str[256]; //work string
 char buf[256]; //work buffer
 unsigned char key[32]; //public key data
 unsigned char secret[32]; //secret key data
 char* name=0; //name  of contact/key_file
 char* pass=0; //acess to secret file
 char* book=BOOK; //-B  adrressbook file name
 int i;
 unsigned char c;
 FILE* F=0;


 //parse args
 for(i=1;i<argc;i++)
 {
  if(strlen(argv[i])<2) continue; //-X must be
  if(argv[i][0]!='-') continue;
  else if(argv[i][1]=='B') book=2+argv[i]; //address book file
  else if(argv[i][1]=='A') name=2+argv[i]; //add key file = contact name in addressbook
  else if(argv[i][1]=='G') //generate new keypair
  {
   newkey=1; //new key flag
   name=2+argv[i]; //new name
   if((i+1)<argc) if(argv[i+1][0]!='-') pass=argv[i+1];
  }
 //-------------------------------------
  else if(argv[i][1]=='R') //re-encode private key
  {
   newkey=-1; //re-encode key flag
   name=2+argv[i]; //name
   if((i+1)<argc) if(argv[i+1][0]!='-') pass=argv[i+1];
  }
  else if(argv[i][1]=='Y')
  {
   pass=2+argv[i];
  }
 }
 //-----------------------------------------

 //no args: output help info
 if(!name)
 {
  printf("OnionPhone key manager: creates new keypairs\r\n");
  printf(" and adds users public keys to address book\r\n\r\n");
  printf("To generate a new keypair:\r\n");
  printf("addkey -Gname [-Yaccess] [other option]\r\n");
  printf("Pubkey 'name' and seckey 'name.sec' will be created\r\n");
  printf("Seckey will be protected with password 'access'\r\n");
  printf("After the new keypair is generated you must sign the contents of pubkey\r\n");
  printf("using your long-term PGP key ant put the signature at the end of pubkey,\r\n");
  printf("then add pubkey to your address book using addkey in -A mode\r\n\r\n");
  printf("To add a public key to your address book:\r\n");
  printf("addkey -Aname [-Bbookfile] [other option]\r\n");
  printf("default bookfile is 'contacts'\r\n\r\n");
  printf("Other options pass to adressbook depend on the application\r\n");
  printf("The most common option is:\r\n:");
  printf("-U|-T|-Oadress:port for a contact (UDP, TCP, OnionHS)\r\n");
  printf("-L mark the contact untrusted, -LX to ignore incoming messages from it\r\n");
  printf("-I hide own identity (use 'guest') for this contact\r\n");
  printf("-Ppass common passphrase for this contact\r\n\r\n");
  printf("Later your can edit options in book file manually\r\n");
  printf("but can't edit key file after signing (rename only)\r\n");
  printf("To encrypt, re-encrypt or decrypts the private key:\r\n");
  printf("addkey -Rname [-Yaccess]\r\n");
  return 0;
 }

 //check length of name (max) and book(max)
 i=strlen(name);
 if((!i)||(i>MAX_NAME))
 {
  printf("Incorrect key_name length!\r\n");
  return 0;
 }
 i=strlen(book);
 if((!i)||(i>MAX_BOOK))
 {
  printf("Incorrect book_name length!\r\n");
  return 0;
 }


 //==================re-encrypt secret key
 if(newkey==-1)
 {
  char newname[256];
  char newpass[256];
  randInit(0, 0);
  //load secret key
  if(32!=get_seckey(name, pass, secret))
  {
   printf("Secret key not processed!\r\n");
   return 0;
  }
  //input new filename and access
  printf("Type new name for secret key '%s' or Enter to overwrite:\r\n", name);
  fgets(newname, sizeof(newname), stdin);
  if(newname[0]>32)
  {
   for(i=0;i<(int)strlen(newname);i++) if(newname[i]<32) newname[i]=0;
   name=newname;
  }
  printf("Type new password or Enter to saving unencrypted:\r\n");
  fgets(newpass, sizeof(newpass), stdin);
  if(newpass[0]>32)
  {
   for(i=0;i<(int)strlen(newpass);i++) if(newpass[i]<32) newpass[i]=0;
   pass=newpass;
  }
  else pass=0;
  //save secret key
  if(!save_seckey(name, pass, secret))
  {
   printf("Secret key saving error!\r\n");
   return 0;
  }
  else printf("Secret key was re-encrypted and saved as '%s'\r\n", name);
 }
//==================generate new keypair===============
 else if(newkey)
 {
  //check for seckey with specified name already exist
  sprintf(str, "%s%s.sec", KEYDIR, name);
  F=0;
  if((F = fopen(str, "rb" )))
  {
   printf("Specified secret file already exists!\r\n");
   fclose(F);
   return 0;
  }
  //check for pubkey with specified name already exist
  sprintf(str, "%s%s", KEYDIR, name);
  if((F = fopen(str, "rb" )))
  {
   printf("Specified key file already exists!\r\n");
   fclose(F);
   return 0;
  }
 //generate random secret 32 bytes
  randInit(0, 0);
  randFetch(secret, 32);

 //generation of 'guest' dymmy keypair for project:
 //strcpy(secret, "Guest secret http://torfone.org");
 //pass=0;

 //-------------------------------------------
  if(!save_seckey(name, pass, secret))
  {
   printf("Secret key saving error!\r\n");
   return 0;
  }
  //generate EC25519 public key
  get_pubkey(key, secret);
  memset(secret, 0, 32); //clear secret
  sprintf(str, "%s%s", KEYDIR, name);
  //creates new pubkey file
  if(!(F = fopen(str, "wt" )))
  {
   printf("Error creating key file!\r\n");
   return 0;
  }
  //generate key info string
  str[0]=0; //truncate string
  sprintf(str, "#%s", name); //#name
  //pass specified additional args to key info
  for(i=1;i<argc;i++)
  {
   if( (strlen(argv[i])<2)  || (strlen(argv[i])>31) ) continue;
   if(argv[i][0]!='-')continue; //only valid args, skip args of addkey
   if( (argv[i][1]=='A') || (argv[i][1]=='G') || (argv[i][1]=='B') || (argv[i][1]=='Y') ) continue;
   sprintf(str+strlen(str), " %s", argv[i]);
  }
  sprintf(str+strlen(str), "\n");
  //save info
  if(0>fputs(str, F))
  {
   printf("Error writing key file!\r\n");
   fclose(F);

   if(!remove(str)) printf("Error removing bad key file! Try do it manually\r\n");
   sprintf(str, "%s%s.sec", KEYDIR, name);
   if(!remove(str)) printf("Error removing corresponding secret file! Try do it manually\r\n");
   return 0;
  }
  //code ECDH public key using b64
  b64estr(key, 32, str);
  //save ECDH public key
  if(0>fputs(str, F))
  {
   printf("Error writing key file!\r\n");
   fclose(F);
   if(!remove(str)) printf("Error removing bad key file! Try do it manually\r\n");
   sprintf(str, "%s%s.sec", KEYDIR, name);
   if(!remove(str)) printf("Error removing corresponding secret file! Try do it manually\r\n");
   return 0;
  }
  fclose(F);
  
  printf("\r\nKeypair comletely created. Now you have to sign the content\r\n");
  printf("of the file '%s%s' (put PGP signature after the text).\r\n", KEYDIR, name);
  printf("Then you have to add it to your contacts book as own public key.\r\n");
  printf("Pass this key to remote contacts in any way. Don't change this key later!\r\n");
 }
 else
 {
//=============add new key to adrress book===========
  //open specified public key file
  sprintf(str, "%s%s", KEYDIR, name);
  if(!(F = fopen(str, "rb" )))
  {
   printf("Key file not found!\r\n");
   return 0;
  }
  //computes key-stamp
  Sponge_init(&spng, 0, 0, 0, 0); //initialize hash for stamp
  while(!feof(F)) //process file byte-by-byte
  {
   c = getc(F);  //next char from keyfile
   if(!feof(F)) Sponge_data(&spng, &c, 1, 0, SP_NORMAL);  //absorbing all file data
  }
  fclose(F);
  Sponge_finalize(&spng, key, 16); //get 16 bytes key stamp
  b64estr(key, 16, (char*)secret); //code it in b64
  sprintf((char*)key, "[%s]", name); //specified file/contact name

 //Load info string
  if(!(F = fopen(str, "rt" )))
  {
   printf("Key file not found!\r\n");
   return 0;
  }
  buf[0]=0;
  while(!feof(F))
  {
   fgets(buf, sizeof(buf), F);
   if(buf[0]=='#') break;
  }
  fclose(F);
  //check is found
  if(buf[0]!='#')
  {
   printf("Info string not found into key!\r\n");
   return 0;
  }
  //eliminate \r\n
  for(i=0;i<(int)strlen(buf);i++)
  if( (buf[i]==0x0D)||(buf[i]==0x0A) ) buf[i]=0;

  //open book file
  sprintf(str, "%s%s", KEYDIR, book);
  if(!(F = fopen(str, "r+t" )))
  {
   printf("Address book not found!\r\n");
   return 0;
  }
  //check for specified contact name already exist in book
  while(!feof(F))
  {
   fgets(str, sizeof(str), F);
   if(strstr(str, (char*)key))
   {
    printf("Contact name already exists in adressbook!\r\n");
    return 0;
   }
  }
  //pass additional options from args to contact
  str[0]=0;
  for(i=1;i<argc;i++)
  {
    if( (strlen(argv[i])<2)  || (strlen(argv[i])>31) ) continue;
    if(argv[i][0]!='-')continue; //skip invalid and special options
    //skip options special for addkey utility
    if( (argv[i][1]=='A') || (argv[i][1]=='G') || (argv[i][1]=='B')) continue;
    sprintf(str+strlen(str), " %s", argv[i]);
  }
  //form contacts field in book
  fprintf(F, "%s %s %s%s\n", (char*)key, (char*)secret, buf, str);
  fclose(F);
  //notification
  for(i=0;i<(int)strlen(buf);i++)
  if( (buf[i]==' ')||(buf[i]==0x0D)||(buf[i]==0x0A) ) buf[i]=0;
  printf("Added key from '%s' as %s", buf+1, (char*)key);
  printf("\r\n");
 }
 return 0;
}





  
  
