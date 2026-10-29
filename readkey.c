/* Native persistence adapter. Original gameplay key names and checksum remain unchanged. GPL-3.0-or-later. */
#include "os.h"
#include "zarrosim.h"
#include <stdio.h>
#include <string.h>
#include <mmsystem.h>
char nome_del_file_su_cui_salvare[256];
void TabbozAddKey(char *key, char *value) { TabbozStoreWrite(key,value); }
char *TabbozReadKey(char *key, char *value, size_t capacity) { return TabbozStoreRead(key,value,capacity); }
char *RRKey(char *key) { static char value[255]; value[0]=0; TabbozReadKey(key,value,sizeof(value)); return value; }
void SpegniISuoni(void) { PlaySoundA(NULL,NULL,0); }
void TabbozPlaySound(int number) { if (!TabbozTesting) PlaySoundA(MAKEINTRESOURCEA(number),hInst,SND_ASYNC|SND_RESOURCE|SND_NODEFAULT); }
