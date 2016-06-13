                     /*-----------------------------------*/
                     /*             >>>Slip<<<            */
                     /*            Theo D'Hondt           */
                     /*     VUB Software Languages Lab    */
                     /*             (c) 2010              */
                     /*-----------------------------------*/
                     /*     version 12: smart caching     */
                     /*-----------------------------------*/
                     /*               pool                */
                     /*-----------------------------------*/

#include <string.h>

#include "SlipMain.h"

#include "SlipPool.h"

/*------------------------------ private constants ---------------------------*/

enum {   Initial_pool_size = 64,
       Pool_size_increment = 32 };

/*------------------------------ private variables ---------------------------*/

static VEC_type Pool_vector;

static UNS_type Pool_size;
static UNS_type Top_of_pool;

/*------------------------------ public functions ----------------------------*/

SYM_type Pool_Enter(TXT_type Text) 
  { SYM_type symbol;
    TXT_type text;
    UNS_type index,
             size;
    VEC_type vector;
    size = size_SYM(Text);
    for (index = 1;
         index < Top_of_pool;
         index++)
      { symbol = Pool_vector[index];
        text = symbol->txt;
        if (strcmp(Text,
                   text) == 0)
          return symbol; }
    if (Top_of_pool == Pool_size)
      { Pool_size += Pool_size_increment;
        Main_Claim(Pool_size);
        vector = Pool_vector;
        Pool_vector = init_VEC(Pool_size);
        for (index = 1;
             index < Top_of_pool;
             index++)
          Pool_vector[index] = vector[index]; }
    Main_Claim(size);
    symbol = make_SYM(Text);
    Pool_vector[Top_of_pool++] = symbol;
    return symbol; }

NIL_type Pool_Initialize(NIL_type) 
  { Pool_size = Initial_pool_size;
    Top_of_pool = 1;
    Pool_vector = init_VEC(Initial_pool_size);
    Main_Register((REF_type)&Pool_vector); }

SYM_type Pool_Initially_Enter(TXT_type Text) 
  { SYM_type symbol;
    symbol = make_SYM(Text);
    Pool_vector[Top_of_pool++] = symbol;
    return symbol; }