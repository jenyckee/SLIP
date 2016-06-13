                     /*-----------------------------------*/
                     /*             >>>Slip<<<            */
                     /*            Theo D'Hondt           */
                     /*     VUB Software Languages Lab    */
                     /*             (c) 2010              */
                     /*-----------------------------------*/
                     /*     version 12: smart caching     */
                     /*-----------------------------------*/
                     /*               stack               */
                     /*-----------------------------------*/

#include "SlipMain.h"

#include "SlipStack.h"

/*------------------------------ private constants ---------------------------*/

enum {   Initial_stack_size = 64,
       Stack_size_increment = 32 };

/*------------------------------ private variables ---------------------------*/

static VEC_type Stack_vector;

static UNS_type Stack_size;
static UNS_type Top_of_stack;

/*------------------------------ public functions ----------------------------*/

NIL_type Stack_Empty(NIL_type)
  { Top_of_stack = 0; }

NIL_type Stack_Initialize(NIL_type)
  { Stack_size = Initial_stack_size;
    Top_of_stack = 0;
    Stack_vector = init_VEC(Initial_stack_size);
    Main_Register((REF_type)&Stack_vector); }

EXP_type Stack_Peek(NIL_type)
  { return Stack_vector[Top_of_stack]; }

NIL_type Stack_Poke(EXP_type Expression)
  { Stack_vector[Top_of_stack] = Expression; }

EXP_type Stack_Pop(NIL_type)
  { EXP_type expression;
    expression = Stack_vector[Top_of_stack];
    Stack_vector[Top_of_stack--] = Main_Null;
    return expression; }

EXP_type Stack_Push(EXP_type Expression)
  { UNS_type index;
    VEC_type vector;
    Stack_vector[++Top_of_stack] = Expression;
    if (Top_of_stack == Stack_size)
      { Stack_size += Stack_size_increment;
        Main_Claim(Stack_size);
        vector = init_VEC(Stack_size);
        for (index = 1;
             index <= Top_of_stack;
             index++)
          vector[index] = Stack_vector[index];
        Stack_vector = vector;
        return Stack_Peek(); }
    return Expression; }

NIL_type Stack_Zap(NIL_type)
  { Top_of_stack--; }

