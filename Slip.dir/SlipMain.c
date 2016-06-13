                     /*-----------------------------------*/
                     /*             >>>Slip<<<            */
                     /*            Theo D'Hondt           */
                     /*     VUB Software Languages Lab    */
                     /*             (c) 2010              */
                     /*-----------------------------------*/
                     /*     version 12: smart caching     */
                     /*-----------------------------------*/
                     /*               main                */
                     /*-----------------------------------*/

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "SlipSlip.h"

#include "SlipMain.h"

#include "SlipCompile.h"
#include "SlipDictionary.h"
#include "SlipEnvironment.h"
#include "SlipEvaluate.h"
#include "SlipNative.h"
#include "SlipPool.h"
#include "SlipPrint.h"
#include "SlipRead.h"
#include "SlipScan.h"
#include "SlipStack.h"
#include "SlipThread.h"

/*------------------------------ private constants ---------------------------*/

enum { Root_size = 12 };

static const UNS_type Minimum_number_of_cells = 2048;

static const TXT_type IMM_error_string = "insufficient memory";
static const TXT_type TMR_error_string = "too many root registrations";

/*------------------------------ private types -------------------------------*/

typedef enum { Initiate_loop  = 0,
               Terminate_loop = 1,
               Proceed_loop   = 2} TRA_type;

typedef jmp_buf EXI_type;

/*------------------------------ private variables ---------------------------*/

static VEC_type Root;

static EXP_type Survivor;

static NBR_type Continue_print;
static UNS_type Root_counter;
static UNS_type Total_number_of_cells;
static EXI_type Trampoline;

static REF_type Force_collect[Root_size];

/*------------------------------ public variables ----------------------------*/

CHR_type Main_Buffer[Main_Buffer_Size];

SYM_type Main_Anonymous;
SYM_type Main_Begin;
SYM_type Main_Define;
ENV_type Main_Empty_Environment;
FRM_type Main_Empty_Frame;
GLB_type Main_Empty_Global;
PAI_type Main_Empty_Pair;
THR_type Main_Empty_Thread;
VEC_type Main_Empty_Vector;
FLS_type Main_False;
SYM_type Main_If;
SYM_type Main_Lambda;
NUL_type Main_Null;
NBR_type Main_One;
SYM_type Main_Quote;
SYM_type Main_Set;
TRU_type Main_True;
USP_type Main_Unspecified;
SYM_type Main_While;
NBR_type Main_Zero;

/*------------------------------ collect functions ---------------------------*/

static NIL_type after_collect(NIL_type)
  { EXP_type expression;
    REF_type reference;
    UNS_type index;
    for (index = 0;
         index < Root_counter;
         index++)
      { reference = Force_collect[index];
        expression = Root[index + 1];
        *reference = expression; }}
        
static NIL_type before_collect(NIL_type)
  { REF_type reference;
    UNS_type index;
    for (index = 0;
         index < Root_counter;
         index++)
      { reference = Force_collect[index];
        Root[index + 1] = *reference; }}
        
/*------------------------------ private functions ---------------------------*/

static EXP_type continue_print(EXP_type Value)
  { Print_Print(Value);
    longjmp(Trampoline,
            Proceed_loop);
    return Main_Unspecified; }

static NIL_type read_eval_print(NIL_type)
  { EXP_type compiled_expression,
             expression,
             value;
    TRA_type status;
    TXT_type input;
    Stack_Empty();
    Slip_Print("\n>>>");
    Slip_Read(&input);
    Dictionary_Checkpoint();
    if ((status = setjmp(Trampoline)) == Initiate_loop)
      { Main_Claim(Main_Default_Margin);
        expression = Read_Parse(input);
        Main_Survival_Claim((REF_type)&expression,
                            Main_Default_Margin);
        compiled_expression = Compile_Compile(expression);
        Main_Survival_Claim((REF_type)&compiled_expression,
                            Main_Default_Margin);
        Thread_Clear();
        Thread_Push(Continue_print,
                    Main_False,
                    0);
        if ((status = setjmp(Trampoline)) == Initiate_loop)
          { value = Evaluate_Evaluate(compiled_expression);
            for (;;)
              value = Evaluate_Continue(value); }
        if (status == Terminate_loop)
          { Environment_Release_Frame();
            Environment_Rollback();
            Thread_Clear(); }}
    if (status == Terminate_loop)
      Dictionary_Rollback(); }

/*------------------------------ hidden functions ---------------------------*/

static UNS_type reclaim(UNS_type Margin)
  { UNS_type after,
             before,
             ticks;
    FLO_type time;
    before = Memory_Available();
    ticks = clock();
    Thread_Flush();
    Environment_Flush();
    before_collect();
    Root = (VEC_type)Memory_Collect((PTR_type)Root);
    after = Memory_Available();
    if (after < Margin)
      Main_Fatal_Error(IMM_error_string);
    after_collect();
    Main_Anonymous = Pool_Enter(Main_Anonymous_String);
    Main_Begin     = Pool_Enter(Main_Begin_String    );
    Main_Define    = Pool_Enter(Main_Define_String   );
    Main_If        = Pool_Enter(Main_If_String       );
    Main_Lambda    = Pool_Enter(Main_Lambda_String   );
    Main_Quote     = Pool_Enter(Main_Quote_String    );
    Main_Set       = Pool_Enter(Main_Set_String      );
    Main_While     = Pool_Enter(Main_While_String    );
    ticks = clock() - ticks;
    time = (FLO_type)ticks / CLOCKS_PER_SEC;
    sprintf(Main_Buffer, 
            "before = %lu after = %lu total = %lu time = %#.6f",
            before, after, Total_number_of_cells, time);
    Slip_Log(Main_Buffer); 
    return (after - before); }

/*------------------------------ public functions ----------------------------*/

NIL_type Main_Claim(UNS_type Margin)
  { if (Memory_Claim(Margin + 1))
      reclaim(Margin); }
      
EXP_type Main_Error(TXT_type Error)
  { Slip_Print(Error);
    longjmp(Trampoline,
            Terminate_loop);
    return Main_Unspecified; }

EXP_type Main_Error_Procedure(TXT_type Error ,
                              PRC_type Procedure)
  { SYM_type symbol;
    symbol = Procedure->nam;
    return Main_Error_Symbol(Error,
                             symbol); }

EXP_type Main_Error_Procedure_Vararg(TXT_type Error ,
                                     PRZ_type Procedure)
  { SYM_type symbol;
    symbol = Procedure->nam;
    return Main_Error_Symbol(Error,
                             symbol); }

EXP_type Main_Error_Symbol(TXT_type Error ,
                           SYM_type Symbol)
  { TXT_type text;
    text = Symbol->txt;
    return Main_Error_Text(Error,
                           text); }

EXP_type Main_Error_Tag(TXT_type Error,
                        TAG_type Tag )
  { sprintf(Main_Buffer, "%s: %d", Error, Tag);
    return Main_Error(Main_Buffer); }

EXP_type Main_Error_Text(TXT_type Error,
                         TXT_type Text )
  { sprintf(Main_Buffer, "%s: %s", Error, Text);
    return Main_Error(Main_Buffer); }

NIL_type Main_Fatal_Error(TXT_type Error)
  { Slip_Print(Error);
    exit(0); }

TXT_type Main_Load(TXT_type Name)
  { TXT_type input;
    Slip_Load(Name,
              &input);
    return input; }

NIL_type Main_Print(TXT_type Text)
  { Slip_Print(Text); }

TXT_type Main_Read(NIL_type)
  { TXT_type input;
    Slip_Read(&input);
    return input; }

UNS_type Main_Reclaim(NIL_type)
  { return reclaim(0); }
  
NIL_type Main_Register(REF_type Reference)
  { if (Root_counter == Root_size)
      Main_Fatal_Error(TMR_error_string);
    Force_collect[Root_counter++] = Reference; }

PAI_type Main_Reverse(PAI_type This)
  { PAI_type next,
             pair,
             previous;
    previous = Main_Empty_Pair;
    for (pair = This;
         is_PAI(pair);
         pair = next)
      { next = pair->cdr;
        pair->cdr = previous;
        previous = pair; }
    return previous; }

NIL_type Main_Survival_Claim(REF_type Expression,
                             UNS_type Margin)
  { if (Memory_Claim(Margin + 1))
      { Survivor = *Expression;
        reclaim(Margin);
        *Expression = Survivor;
        Survivor = Main_Null; }}
      
EXP_type Main_Terminate(NIL_type)
  { longjmp(Trampoline,
            Terminate_loop);
    return Main_Unspecified; }

/*------------------------------ exported functions --------------------------*/

void Slip_REP(char * Memory,
              int    Size)
  { Total_number_of_cells = Size / Memory_Cell_Size;
    if (Total_number_of_cells < Minimum_number_of_cells)
      Main_Fatal_Error(IMM_error_string);
    Memory_Initialize(Memory,
                      Total_number_of_cells);
    Root = init_VEC(Root_size);
    Root_counter = 0;
    Pool_Initialize();
    Main_Empty_Vector       = make_VEC(0);
    Main_False              = make_FLS();
    Main_Null               = make_NUL();
    Main_One                = make_NBR(1);
    Main_True               = make_TRU();
    Main_Unspecified        = make_USP();
    Main_Zero               = make_NBR(0);
    Main_Empty_Environment  = (ENV_type)Main_Null;
    Main_Empty_Frame        = (FRM_type)Main_Null;
    Main_Empty_Global       = (GLB_type)Main_Null;
    Main_Empty_Pair         = (PAI_type)Main_Null;
    Main_Empty_Thread       = (THR_type)Main_Null;
    Main_Anonymous          = Pool_Initially_Enter(Main_Anonymous_String);
    Main_Begin              = Pool_Initially_Enter(Main_Begin_String    );
    Main_Define             = Pool_Initially_Enter(Main_Define_String   );
    Main_If                 = Pool_Initially_Enter(Main_If_String       );
    Main_Lambda             = Pool_Initially_Enter(Main_Lambda_String   );
    Main_Quote              = Pool_Initially_Enter(Main_Quote_String    );
    Main_Set                = Pool_Initially_Enter(Main_Set_String      );
    Main_While              = Pool_Initially_Enter(Main_While_String    );
    Survivor                = Main_Null;
    Main_Register((REF_type)&Survivor);
    Main_Register((REF_type)&Main_Empty_Vector);
    Stack_Initialize();
    Dictionary_Initialize();
    Environment_Initialize();
    Thread_Initialize();
    Read_Initialize();
    Compile_Initialize();
    Evaluate_Initialize();
    Native_Initialize();
    Print_Initialize();
    Continue_print = Thread_Register((CCC_type)continue_print,
                                     0);
    Slip_Print("cpSlip/c version 12: smart caching");
    for (;;)
      read_eval_print();
    Slip_Print("Slip terminated"); }
