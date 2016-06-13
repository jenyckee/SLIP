                     /*-----------------------------------*/
                     /*             >>>Slip<<<            */
                     /*            Theo D'Hondt           */
                     /*     VUB Software Languages Lab    */
                     /*             (c) 2010              */
                     /*-----------------------------------*/
                     /*     version 12: smart caching     */
                     /*-----------------------------------*/
                     /*               print               */
                     /*-----------------------------------*/

#include <stdio.h>

#include "SlipMain.h"

#include "SlipNative.h"
#include "SlipPrint.h"

/*------------------------------ private macros ------------------------------*/

#define format(FORMAT, VALUE)                                                  \
  { snprintf(Main_Buffer,                                                      \
             Main_Buffer_Size,                                                 \
             FORMAT,                                                           \
             VALUE);                                                           \
    Main_Print(Main_Buffer); }

/*------------------------------ private constants ---------------------------*/

static const TXT_type ILT_error_string = "illegal expression type";
static const TXT_type IXT_error_string = "invalid expression type";

static const TXT_type   Character_display_text = "%c";
static const TXT_type Continuation_display_text = "<continuation>";
static const TXT_type  Empty_list_display_text = "()";
static const TXT_type       False_display_text = "#f";
static const TXT_type     Leftpar_display_text = "(";
static const TXT_type     Leftvec_display_text = "[";
static const TXT_type      Native_display_text = "<native %s>";
static const TXT_type     Newline_display_text = "\n";
static const TXT_type      Number_display_text = "%ld";
static const TXT_type      Period_display_text = " . ";
static const TXT_type   Procedure_display_text = "<procedure %s>";
static const TXT_type        Real_display_text = "%#.10g";
static const TXT_type    Rightpar_display_text = ")";
static const TXT_type    Rightvec_display_text = "]";
static const TXT_type       Space_display_text = " ";
//static const TXT_type      String_display_text = "%s";
static const TXT_type        True_display_text = "#t";
static const TXT_type Unspecified_display_text = "<unspecified>";


static const TXT_type    Character_print_text = "#\\%c";
static const TXT_type Continuation_print_text = "<continuation>";
//static const TXT_type   C_function_print_text = "<C function>";
static const TXT_type   Empty_list_print_text = "()";
static const TXT_type        False_print_text = "#f";
static const TXT_type      Leftpar_print_text = "(";
static const TXT_type      Leftvec_print_text = "[";
static const TXT_type     Variable_print_text = "var_%lu";
static const TXT_type Variable_bis_print_text = "_%lu";
static const TXT_type       Native_print_text = "<native %s>";
static const TXT_type      Newline_print_text = "\n";
static const TXT_type       Number_print_text = "%ld";
//static const TXT_type      Pattern_print_text = "<%d,%d>";
static const TXT_type       Period_print_text = " . ";
static const TXT_type    Procedure_print_text = "<procedure %s>";
static const TXT_type        Quote_print_text = "'";
static const TXT_type         Real_print_text = "%#.10g";
static const TXT_type     Rightpar_print_text = ")";
static const TXT_type     Rightvec_print_text = "]";
static const TXT_type        Space_print_text = " ";
static const TXT_type       String_print_text = "\"%s\"";
static const TXT_type       Symbol_print_text = "%s";
static const TXT_type         True_print_text = "#t";
static const TXT_type  Unspecified_print_text = "<unspecified>";

/*------------------------------ private variables ---------------------------*/

static UNS_type Indentation;
static UNS_type       Scope;

/*------------------------------ private prototypes --------------------------*/

static NIL_type display_expression(EXP_type);
static NIL_type   print_expression(EXP_type);
static NIL_type     print_sequence(SEQ_type);

/*------------------------------ auxiliary functions -------------------------*/

static NIL_type indent(NIL_type)
  { Indentation += 2; }

static NIL_type outdent(NIL_type)
  { Indentation -= 2; }

/*---------------------------private display functions -----------------------*/

static NIL_type display_character(CHA_type Character)
  { format(Character_display_text,
           Character->chr); }

static NIL_type display_continuation(CNT_type Continuation)
  { Main_Print(Continuation_display_text); }

static NIL_type display_false(NIL_type)
  { Main_Print(False_display_text); }

static NIL_type display_native(NAT_type Native)
  { TXT_type name = Native->nam;
    format(Native_display_text,
           name); }

static NIL_type display_newline(NIL_type)
  { Main_Print(Newline_display_text); }

static NIL_type display_null(NIL_type)
  { Main_Print(Empty_list_display_text); }

static NIL_type display_number(NBR_type Number)
  { LNG_type raw_number;
    raw_number = get_NBR(Number);
    format(Number_display_text,
           raw_number); }

static NIL_type display_pair(PAI_type Pair)
  { EXP_type residue,
             value;
    PAI_type list;
    TAG_type tag;
    Main_Print(Leftpar_display_text);
    for (list = Pair;
         ;
         )
      { value = list->car;
        residue = list->cdr;
        display_expression(value);
        tag = Tag_of(residue);
        switch (tag)
          { case NUL_tag:
              Main_Print(Rightpar_display_text);
              return;
            case PAI_tag:
              Main_Print(Space_display_text);
              list = residue;
              break;
            default:
              Main_Print(Period_display_text);
              display_expression(residue);
              Main_Print(Rightpar_display_text);
              return; }}}

static NIL_type display_procedure(PRC_type Procedure)
  { SYM_type name;
    TXT_type text;
    name = Procedure->nam;
    text = name->txt;
    format(Procedure_display_text,
           text); }

static NIL_type display_procedure_vararg(PRZ_type Procedure)
  { SYM_type name;
    TXT_type text;
    name = Procedure->nam;
    text = name->txt;
    format(Procedure_display_text,
           text); }

static NIL_type display_real(REA_type Real)
  { format(Real_display_text,
           Real->flo); }

static NIL_type display_string(STR_type String)
  { TXT_type text;
    text = String->txt;
    Main_Print(text); }

static NIL_type display_symbol(SYM_type Symbol)
  { TXT_type text;
    text = Symbol->txt;
    Main_Print(text); }

static NIL_type display_thunk(THK_type Thunk)
  { EXP_type expression;
    expression = Thunk->exp;
    display_expression(expression); }

static NIL_type display_true(NIL_type)
  { Main_Print(True_display_text); }

static NIL_type display_unspecified(NIL_type)
  { Main_Print(Unspecified_display_text); }

static NIL_type display_vector(VEC_type Vector)
  { EXP_type value;
    UNS_type index,
             size;
    size = size_VEC(Vector);
    Main_Print(Leftvec_display_text);
    for (index = 1;
         index <= size;
         index++)
      { value = Vector[index];
        display_expression(value);
        if (index < size)
          Main_Print(Space_display_text); }
    Main_Print(Rightvec_display_text); }

/*----------------------------------------------------------------------------*/

static NIL_type display_expression(EXP_type Value)
  { TAG_type tag;
    tag = Tag_of(Value);
    switch (tag)
      { case CHA_tag:
          display_character(Value);
          return;
        case CNT_tag:
          display_continuation(Value);
          return;
        case FLS_tag:
          display_false();
          return;
        case NAT_tag:
          display_native(Value);
          return;
        case NBR_tag:
          display_number(Value);
          return;
        case NUL_tag:
          display_null();
          return;
        case PAI_tag:
          display_pair(Value);
          return;
        case PRC_tag:
          display_procedure(Value);
          return;
        case PRZ_tag:
          display_procedure_vararg(Value);
          return;
        case REA_tag:
          display_real(Value);
          return;
        case STR_tag:
          display_string(Value);
          return;
        case SYM_tag:
          display_symbol(Value);
          return;
        case THK_tag:
          display_thunk(Value);
          return;
        case TRU_tag:
          display_true();
          return;
        case USP_tag:
          display_unspecified();
          return;
        case VEC_tag:
          display_vector(Value);
          return; 
        case APL_tag:
        case BEG_tag:
        case DFF_tag:
        case DFV_tag:
        case DFZ_tag:
        case ENV_tag:
        case FRM_tag:
        case GLB_tag:
        case IFF_tag:
        case IFZ_tag:
        case LCL_tag:
        case LMB_tag:
        case LMZ_tag:
        case QUO_tag:
        case SEQ_tag:
        case STG_tag:
        case STL_tag:
        case THR_tag:
        case THr_tag:
        case WHI_tag:
          Main_Error_Tag(ILT_error_string,
                         tag); }
    Main_Error_Tag(IXT_error_string,
                   tag); }
 
/*--------------------------- private print functions ------------------------*/

static NIL_type print_newline(NIL_type)
  { UNS_type index;
    Main_Print(Newline_print_text);
    for (index = 0;
         index < Indentation;
         index++)
       Main_Print(Space_print_text); }

static NIL_type print_operands(VEC_type Operands)
  { EXP_type expression;
    UNS_type size,
             index;
 	  size = size_VEC(Operands);
    for (index = 1;
         index <= size;
         index++)
      { Main_Print(Space_print_text);
        expression = Operands[index];
        print_expression(expression); }}

static NIL_type print_variable(UNS_type Scope,
                               UNS_type Offset)
  { TXT_type name;
    if (Scope == 1)
      { name = Native_Get_Name(Offset);
        if (name)
          { Main_Print(name);
            return; }}
    format(Variable_print_text,
           Scope);
    format(Variable_bis_print_text,
           Offset);  }

static NIL_type print_pattern(UNS_type Param_count)
  { UNS_type index;
    if (Param_count > 0)
      for (index = 1;
           index <= Param_count;
           index++)
        { if (index > 1)
            Main_Print(Space_print_text);
          print_variable(Scope,
                         index); }}

/*----------------------------------------------------------------------------*/

static NIL_type print_application(APL_type Application)
  { EXP_type operator;
    VEC_type operands;
    operator = Application->exp;
    operands = Application->opr;
    Main_Print(Leftpar_print_text);
    print_expression(operator);
    print_operands(operands);
    Main_Print(Rightpar_print_text); }

static NIL_type print_begin(BEG_type Begin)
  { VEC_type sequence;
    sequence = Begin->seq;
    Main_Print(Leftpar_print_text);
    Main_Print(Main_Begin_String);
    print_sequence(sequence);
    Main_Print(Rightpar_print_text); }

static NIL_type print_character(CHA_type Character)
  { CHR_type character;
    character = Character->chr;
    format(Character_print_text,
           character); }

static NIL_type print_continuation(CNT_type Continuation)
  { Main_Print(Continuation_print_text); }

static NIL_type print_define_function(DFF_type Define)
  { NBR_type offset,
             param_count;
    UNS_type raw_offset,
             raw_param_count;
    VEC_type body;
    offset      = Define->ofs;
    param_count = Define->par;
    body          = Define->bod;
    raw_offset = get_NBR(offset);
    raw_param_count = get_NBR(param_count);
    Main_Print(Leftpar_print_text);
    Main_Print(Main_Define_String);
    Main_Print(Space_print_text);
    Main_Print(Leftpar_print_text);
    print_variable(Scope,
                   raw_offset);
    Scope++;
    Main_Print(Space_print_text);
    print_pattern(raw_param_count);
    Main_Print(Rightpar_print_text);
    print_sequence(body);
    Scope--;
    Main_Print(Rightpar_print_text);  }

static NIL_type print_define_function_vararg(DFZ_type Define)
  { NBR_type offset,
             param_count;
    UNS_type raw_offset,
             raw_param_count;
    VEC_type body;
    offset      = Define->ofs;
    param_count = Define->par;
    body          = Define->bod;
    raw_offset = get_NBR(offset);
    raw_param_count = get_NBR(param_count);
    Main_Print(Leftpar_print_text);
    Main_Print(Main_Define_String);
    Main_Print(Space_print_text);
    Main_Print(Leftpar_print_text);
    print_variable(Scope,
                   raw_offset);
    Scope++;
    Main_Print(Space_print_text);
    print_pattern(raw_param_count);
    Main_Print(Period_print_text);
    print_variable(Scope,
                   raw_param_count + 1);
    Main_Print(Rightpar_print_text);
    print_sequence(body);
    Scope--;
    Main_Print(Rightpar_print_text);  }

static NIL_type print_define_variable(DFV_type Define)
  { EXP_type expression;
    NBR_type offset;
    UNS_type raw_offset;
    offset   = Define->ofs;
    raw_offset = get_NBR(offset);
    expression = Define->exp;
    Main_Print(Leftpar_print_text);
    Main_Print(Main_Define_String);
    Main_Print(Space_print_text);
    print_variable(Scope,
                   raw_offset);
    Main_Print(Space_print_text);
    print_expression(expression);
    Main_Print(Rightpar_print_text); }

static NIL_type print_false(NIL_type)
  { Main_Print(False_print_text); }

static NIL_type print_global(GLB_type Global)
  { NBR_type offset,
             scope;
    UNS_type raw_offset,
             raw_scope;
    scope  = Global->scp;
    offset = Global->ofs;
    raw_scope = get_NBR(scope);
    raw_offset = get_NBR(offset);
    print_variable(raw_scope,
                   raw_offset); }

static NIL_type print_if_double(IFF_type If)
  { EXP_type alternative,
             consequent,
             predicate;
    predicate   = If->prd;
    consequent  = If->cns;
    alternative = If->alt;
    Main_Print(Leftpar_print_text );
    Main_Print(Main_If_String);
    Main_Print(Space_print_text);
    print_expression(predicate);
    indent();
    print_newline();
    print_expression(consequent);
    print_newline();
    print_expression(alternative);
    Main_Print(Rightpar_print_text);
    outdent(); }

static NIL_type print_if_single(IFZ_type If)
  { EXP_type consequent,
             predicate;
    predicate  = If->prd;
    consequent = If->cns;
    Main_Print(Leftpar_print_text );
    Main_Print(Main_If_String);
    Main_Print(Space_print_text);
    print_expression(predicate);
    indent();
    print_newline();
    print_expression(consequent);
    Main_Print(Rightpar_print_text);
    outdent(); }

static NIL_type print_lambda(LMB_type Lambda)
  { NBR_type param_count;
    UNS_type raw_param_count;
    VEC_type body;
    param_count = Lambda->par;
    body          = Lambda->bod;
    raw_param_count = get_NBR(param_count);
    Main_Print(Leftpar_print_text);
    Main_Print(Main_Lambda_String);
    Main_Print(Space_print_text);
    Main_Print(Leftpar_print_text);
    Scope++;
    print_pattern(raw_param_count);
    Main_Print(Rightpar_print_text);
    print_sequence(body);
    Scope--;
    Main_Print(Rightpar_print_text);  }

static NIL_type print_lambda_vararg(LMZ_type Lambda)
  { NBR_type param_count;
    UNS_type raw_param_count;
    VEC_type body;
    param_count = Lambda->par;
    body          = Lambda->bod;
    raw_param_count = get_NBR(param_count);
    Main_Print(Leftpar_print_text);
    Main_Print(Main_Lambda_String);
    Main_Print(Space_print_text);
    Scope++;
    if (raw_param_count == 0)
      print_variable(Scope,
                     1);
    else
      { Main_Print(Leftpar_print_text);
        print_pattern(raw_param_count);
        Main_Print(Period_print_text);
        print_variable(Scope,
                       raw_param_count + 1);
        Main_Print(Rightpar_print_text); }
    print_sequence(body);
    Scope--;
    Main_Print(Rightpar_print_text);  }

static NIL_type print_local(LCL_type Local)
  { NBR_type offset;
    UNS_type raw_offset;
    offset = Local->ofs;
    raw_offset = get_NBR(offset);
    print_variable(Scope,
                   raw_offset); }

static NIL_type print_native(NAT_type Native)
  { TXT_type name = Native->nam;
    format(Native_print_text,
           name); }

static NIL_type print_null(NIL_type)
  { Main_Print(Empty_list_print_text); }

static NIL_type print_number(NBR_type Number)
  { LNG_type raw_number;
    raw_number = get_NBR(Number);
    format(Number_print_text,
           raw_number); }

static NIL_type print_pair(PAI_type Pair)
  { EXP_type residue,
             value;
    PAI_type list;
    TAG_type tag;
    Main_Print(Leftpar_print_text);
    for (list = Pair;
         ;
         )
      { value = list->car;
        residue = list->cdr;
        print_expression(value);
        tag = Tag_of(residue);
        switch (tag)
          { case NUL_tag:
              Main_Print(Rightpar_print_text);
              return;
            case PAI_tag:
              Main_Print(Space_print_text);
              list = residue;
              break;
            default:
              Main_Print(Period_print_text);
              print_expression(residue);
              Main_Print(Rightpar_print_text);
              return; }}}

static NIL_type print_procedure(PRC_type Procedure)
  { SYM_type name;
    TXT_type text;
    name = Procedure->nam;
    text = name->txt;
    format(Procedure_print_text,
           text); }

static NIL_type print_procedure_vararg(PRZ_type Procedure)
  { SYM_type name;
    TXT_type text;
    name = Procedure->nam;
    text = name->txt;
    format(Procedure_print_text,
           text); }

static NIL_type print_quote(QUO_type Quote)
  { EXP_type expression;
    expression = Quote->exp;
    Main_Print(Quote_print_text);
    print_expression(expression); }

static NIL_type print_real(REA_type Real)
  { format(Real_print_text,
           Real->flo); }

static NIL_type print_sequence(SEQ_type Sequence)
  { EXP_type expression;
    UNS_type index,
             size;
    size = size_VEC(Sequence);
    indent();
    for (index = 1;
         index <= size;
         index++)
      { print_newline();
        expression = Sequence[index];
        print_expression(expression); }
    outdent(); }

static NIL_type print_set_global(STG_type Set)
  { EXP_type expression;
    NBR_type offset,
             scope;
    UNS_type raw_offset,
             raw_scope;
    scope    = Set->scp;
    offset   = Set->ofs;
    expression = Set->exp;
    raw_scope = get_NBR(scope);
    raw_offset = get_NBR(offset);
    Main_Print(Leftpar_print_text);
    Main_Print(Main_Set_String);
    Main_Print(Space_print_text);
    print_variable(raw_scope,
                   raw_offset);
    Main_Print(Space_print_text);
    print_expression(expression);
    Main_Print(Rightpar_print_text); }

static NIL_type print_set_local(STL_type Set)
  { EXP_type expression;
    NBR_type offset;
    UNS_type raw_offset;
    offset   = Set->ofs;
    expression = Set->exp;
    raw_offset = get_NBR(offset);
    Main_Print(Leftpar_print_text);
    Main_Print(Main_Set_String);
    Main_Print(Space_print_text);
    print_variable(Scope,
                   raw_offset);
    Main_Print(Space_print_text);
    print_expression(expression);
    Main_Print(Rightpar_print_text); }

static NIL_type print_string(STR_type String)
  { TXT_type text;
    text = String->txt;
    format(String_print_text,
           text); }

static NIL_type print_symbol(SYM_type Symbol)
  { TXT_type text;
    text = Symbol->txt;
    format(Symbol_print_text,
           text); }

static NIL_type print_thunk(THK_type Thunk)
  { EXP_type expression;
    Scope++;
    expression = Thunk->exp;
    display_expression(expression); 
    Scope--; }

static NIL_type print_true(NIL_type)
  { Main_Print(True_print_text); }

static NIL_type print_unspecified(NIL_type)
  { Main_Print(Unspecified_print_text); }

static NIL_type print_vector(VEC_type Vector)
  { EXP_type value;
    UNS_type index,
             size;
    size = size_VEC(Vector);
    Main_Print(Leftvec_print_text);
    for (index = 1;
         index <= size;
         index++)
      { value = Vector[index];
        print_expression(value);
        if (index < size)
          Main_Print(Space_print_text); }
    Main_Print(Rightvec_print_text); }

static NIL_type print_while(WHI_type While)
  { EXP_type body,
             predicate;
    predicate  = While->prd;
    body       = While->bod;
    Main_Print(Leftpar_print_text);
    Main_Print(Main_While_String);
    Main_Print(Space_print_text);
    print_expression(predicate);
    indent();
    print_expression(body);
    Main_Print(Rightpar_print_text);
    outdent(); }

/*----------------------------------------------------------------------------*/

static NIL_type print_expression(EXP_type Expression)
  { TAG_type tag;
    tag = Tag_of(Expression);
    switch (tag)
      { case APL_tag:
          print_application(Expression);
          return;
        case BEG_tag:
          print_begin(Expression);
          return;
        case CHA_tag:
          print_character(Expression);
          return;
        case CNT_tag:
          print_continuation(Expression);
          return;
        case DFF_tag:
          print_define_function(Expression);
          return;
        case DFV_tag:
          print_define_variable(Expression);
          return;
        case DFZ_tag:
          print_define_function_vararg(Expression);
          return;
        case FLS_tag:
          print_false();
          return;
        case GLB_tag:
          print_global(Expression);
          return;
        case IFF_tag:
          print_if_double(Expression);
          return;
        case IFZ_tag:
          print_if_single(Expression);
          return;
        case LCL_tag:
          print_local(Expression);
          return;
        case LMB_tag:
          print_lambda(Expression);
          return;
        case LMZ_tag:
          print_lambda_vararg(Expression);
          return;
        case NAT_tag:
          print_native(Expression);
          return;
        case NBR_tag:
          print_number(Expression);
          return;
        case NUL_tag:
          print_null();
          return;
        case PAI_tag:
          print_pair(Expression);
          return;
        case PRC_tag:
          print_procedure(Expression);
          return;
        case PRZ_tag:
          print_procedure_vararg(Expression);
          return;
        case QUO_tag:
          print_quote(Expression);
          return;
        case REA_tag:
          print_real(Expression);
          return;
        case SEQ_tag:
          print_sequence(Expression);
          return;
        case STG_tag:
          print_set_global(Expression);
          return;
        case STL_tag:
          print_set_local(Expression);
          return;
        case STR_tag:
          print_string(Expression);
          return;
        case SYM_tag:
          print_symbol(Expression);
          return;
        case THK_tag:
          print_thunk(Expression);
          return;
        case TRU_tag:
          print_true();
          return;
        case USP_tag:
          print_unspecified();
          return;
        case VEC_tag:
          print_vector(Expression);
          return;
        case WHI_tag:
          print_while(Expression);
          return; 
        case ENV_tag:
        case FRM_tag:
        case THR_tag:
        case THr_tag:
          Main_Error_Tag(ILT_error_string,
                         tag); }
    Main_Error_Tag(IXT_error_string,
                   tag); }

/*------------------------------ public functions ----------------------------*/

NIL_type Print_Display(EXP_type Value)
  { display_expression(Value); }

NIL_type Print_Initialize(NIL_type)
  { return; }

NIL_type Print_Newline(NIL_type)
  { display_newline(); }

NIL_type Print_Print(EXP_type Value)
  { Indentation = 0;
    Scope = 1;
    print_expression(Value); }
