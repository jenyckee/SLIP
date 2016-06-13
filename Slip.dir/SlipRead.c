                     /*-----------------------------------*/
                     /*             >>>Slip<<<            */
                     /*            Theo D'Hondt           */
                     /*     VUB Software Languages Lab    */
                     /*             (c) 2010              */
                     /*-----------------------------------*/
                     /*     version 12: smart caching     */
                     /*-----------------------------------*/
                     /*              reader               */
                     /*-----------------------------------*/

#include <stdlib.h>
#include <string.h>

#include "SlipMain.h"

#include "SlipPool.h"
#include "SlipRead.h"
#include "SlipScan.h"
#include "SlipStack.h"

/*------------------------------ private prototypes --------------------------*/

static EXP_type read_expression(NIL_type);

/*------------------------------ private constants ---------------------------*/

static const TXT_type IRB_error_string = "illegal right bracket";
static const TXT_type IRP_error_string = "illegal right parenthesis";
static const TXT_type IUP_error_string = "illegal use of period";
static const TXT_type MRP_error_string = "missing right parenthesis";
static const TXT_type PEP_error_string = "premature end of program";
static const TXT_type XCT_error_string = "excess tokens";

/*------------------------------ private variables ---------------------------*/

static SCA_type Token;

/*------------------------------ private functions ---------------------------*/

static NIL_type next_token(NIL_type)
  { Token = Scan_Next(); }

static EXP_type next_token_and_return(EXP_type Exp)
  { next_token();
    return Exp; }

/*----------------------------------------------------------------------------*/

static EXP_type read_bracket(NIL_type)
  { Main_Error(IRB_error_string);
    return Main_Null; }

static CHA_type read_character(NIL_type)
  { CHA_type character;
    CHR_type raw_character;
    Main_Claim(Main_Default_Margin);
    raw_character = Scan_String[0];
    character = make_CHA(raw_character);
    return next_token_and_return(character); }

static EXP_type read_end_program(NIL_type)
  { Main_Error(PEP_error_string);
	  return Main_Null; }

static FLS_type read_false(NIL_type)
  { return next_token_and_return(Main_False); }

static PAI_type read_list(NIL_type)
  { EXP_type expression;
    PAI_type pair;
    UNS_type count;
    next_token();
    for (count = 0;
         ;
         count++)
      if (Token == RPR_token)
        { Stack_Push(Main_Null);
          break; }
      else
      { expression = read_expression();
        Stack_Push(expression);
        if (Token == PER_token)
          { next_token();
            expression = read_expression();
            Stack_Push(expression);
            if (Token != RPR_token)
              Main_Error(MRP_error_string);
            count++;
            break; }}
    next_token();
    for (pair = Stack_Pop();
         count > 0;
         count--)
      { Main_Survival_Claim((REF_type)&pair,
                            Main_Default_Margin);
        expression = Stack_Pop();
        pair = make_PAI(expression,
                        pair); }
    return pair; }

static NBR_type read_number(NIL_type)
  { LNG_type raw_number;
    NBR_type number;
    raw_number = atoll(Scan_String);
    number = make_NBR(raw_number);
		return next_token_and_return(number); }

static EXP_type read_parenthesis(NIL_type)
  { Main_Error(IRP_error_string);
    return Main_Null; }

static REA_type read_real(NIL_type)
  { FLO_type raw_real;
    REA_type real;
    raw_real = atof(Scan_String);
    Main_Claim(Main_Default_Margin);
    real = make_REA(raw_real);
    return next_token_and_return(real); }

static EXP_type read_period(NIL_type)
  { Main_Error(IUP_error_string);
	  return Main_Null; }

static EXP_type read_quote(NIL_type)
  { EXP_type expression;
    PAI_type pair;
    next_token();
		expression = read_expression();
    Main_Survival_Claim((REF_type)&expression,
                        Main_Default_Margin);
    pair = make_PAI(expression,
                    Main_Null);
    pair = make_PAI(Main_Quote,
                    pair);
    return pair; }

static STR_type read_string(NIL_type)
  { STR_type string;
    UNS_type size;
    size = size_STR(Scan_String);
    Main_Claim(size);
    string = make_STR(Scan_String);
    return next_token_and_return(string); }

static SYM_type read_symbol(NIL_type)
  { SYM_type symbol;
    symbol = Pool_Enter(Scan_String);
    return next_token_and_return(symbol); }

static TRU_type read_true(NIL_type)
  { return next_token_and_return(Main_True); }

static VEC_type read_vector(NIL_type)
  { EXP_type expression;
    UNS_type count;
    VEC_type vector;
    next_token();
    if (Token == RBR_token)
      vector = Main_Empty_Vector;
    else
      { for (count = 0;
             Token != RBR_token;
             count++)
          { expression = read_expression();
 	          Stack_Push(expression); }
        Main_Claim(count);
        for (vector = make_VEC(count);
             count > 0;
             count--)
          vector[count] = Stack_Pop(); }
    return next_token_and_return(vector); }

static EXP_type read_expression(NIL_type)
  { switch (Token)
      { case CHA_token:
          return read_character();
        case END_token:
          return read_end_program();
        case FLS_token:
          return read_false();
        case IDT_token:
          return read_symbol();
        case LBR_token:
          return read_vector();
        case LPR_token:
          return read_list();
        case NBR_token:
          return read_number();
        case PER_token:
          return read_period();
        case QUO_token:
          return read_quote();
        case RBR_token:
          return read_bracket();
        case RPR_token:
          return read_parenthesis();
        case REA_token:
          return read_real();
        case STR_token:
          return read_string();
        case TRU_token:
          return read_true(); }
	  return Main_Null; }

/*----------------------------- public functions -----------------------------*/

NIL_type Read_Initialize(NIL_type)
  { Scan_Initialize(); }

EXP_type Read_Parse(TXT_type Input)
  { EXP_type expression;
    Scan_Preset(Input);
    next_token();
    expression = read_expression();
    if (Token != END_token)
      Main_Error(XCT_error_string);
    return expression; }
