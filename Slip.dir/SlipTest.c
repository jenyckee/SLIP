                     /*-----------------------------------*/
                     /*             >>>Slip<<<            */
                     /*            Theo D'Hondt           */
                     /*     VUB Software Languages Lab    */
                     /*             (c) 2010              */
                     /*-----------------------------------*/
                     /*     version 12: smart caching     */
                     /*-----------------------------------*/
                     /*               test                */
                     /*-----------------------------------*/

#include <stdio.h>

#include "SlipSlip.h"

enum { Buffer_size = 10000,
       Memory_size = 1000000000 };

static char Buffer[Buffer_size];
static char Memory[Memory_size];

void Slip_Log(char * message)
  { return;
    printf("... %s\n", message); }

void Slip_Load(char  * text,
               char ** input)
  { FILE * stream = fopen(text, "r");
    *input = Buffer;
    Buffer[0] = 0;
    if (stream)
      { unsigned index;
        char character;
        for (index = 0;;)
          { int result = fscanf(stream, "%c", &character);
            if (result == EOF)
              { Buffer[index] = 0;
                fclose(stream);
                return; }
            else
              Buffer[index++] = character; }}
    else
      printf("%s not found\n", text); }

void Slip_Print(char * string)
  { printf("%s", string); }

void Slip_Read(char ** input)
  { unsigned index;
    char character;
    for (index = 0;;)
      { scanf("%c", &character);
        if (character == '\n')
          { Buffer[index] = 0;
            *input = Buffer;
            return; }
        Buffer[index++] = character; }}

int main (int argc, const char * argv[])
  { Slip_REP(Memory, Memory_size);
    return 0; }