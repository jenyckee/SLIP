                     /*-----------------------------------*/
                     /*             >>>Slip<<<            */
                     /*            Theo D'Hondt           */
                     /*     VUB Software Languages Lab    */
                     /*             (c) 2010              */
                     /*-----------------------------------*/
                     /*     version 12: smart caching     */
                     /*-----------------------------------*/
                     /*              compiler             */
                     /*-----------------------------------*/

#include <stdio.h>

#include "SlipMain.h"

#include "SlipCompile.h"
#include "SlipDictionary.h"
#include "SlipStack.h"

/*------------------------------ private macros ------------------------------*/

#define chunk_size(TYP)                                                        \
  ((sizeof(TYP) - sizeof(THR))/ sizeof(BTS_type))

#define survive_default(SRV)                                                   \
  Main_Survival_Claim((REF_type)&SRV, Main_Default_Margin)

/*------------------------------ private constants ---------------------------*/

static const TXT_type AL1_error_string = "at least one expression required";
static const TXT_type E1X_error_string = "exactly one expression required";
static const TXT_type ILT_error_string = "illegal expression type";
static const TXT_type ITF_error_string = "improperly terminated form";
static const TXT_type IPA_error_string = "invalid parameter";
static const TXT_type IVP_error_string = "invalid pattern in form";
static const TXT_type IVV_error_string = "missing variable in form";
static const TXT_type IXT_error_string = "invalid expression type";
static const TXT_type MSP_error_string = "missing pattern in form";
static const TXT_type MSV_error_string = "invalid variable in form";
static const TXT_type TFX_error_string = "too few expressions in form";
//const static TXT_type TMG_error_string = "too many global variables";
static const TXT_type TMX_error_string = "too many expressions in form";
static const TXT_type VNF_error_string = "variable not found";

/*------------------------------ private prototypes --------------------------*/

static EXP_type compile_expression(EXP_type);
static EXP_type     compile_inline(EXP_type);
static VEC_type   compile_operands(PAI_type,
                                   TXT_type);
static EXP_type compile_parameters(PAI_type,
                                   TXT_type);
static EXP_type   compile_sequence(PAI_type,
                                   TXT_type);

/*----------------------------------------------------------------------------*/
/*------------------------------------ application ---------------------------*/
/*----------------------------------------------------------------------------*/

static EXP_type compile_application(PAI_type Application)
  { APL_type compiled_application;
    EXP_type compiled_operator,
             operands,
             operator;
    PAI_type application;
    VEC_type compiled_operands;
    TAG_type tag;
    operands = Application->cdr;
    tag = Tag_of(operands);
    switch (tag)
      { case NUL_tag:
          operator = Application->car;
          compiled_operator = compile_expression(operator);
          survive_default(compiled_operator);
          compiled_application = make_APL(compiled_operator,
                                          Main_Empty_Vector);
          return compiled_application;
        case PAI_tag:
          application = Stack_Push(Application);
          operator = application->car;
          operands = application->cdr;
          Stack_Poke(operands);
          compiled_operator = compile_expression(operator);
          operands = Stack_Peek();
          Stack_Poke(compiled_operator);
          compiled_operands = compile_operands(operands,
                                               Main_Application_String);
          survive_default(compiled_operands);
          compiled_operator = Stack_Pop();
          compiled_application = make_APL(compiled_operator,
                                          compiled_operands);
          return compiled_application; }
    return Main_Error_Text(ITF_error_string,
                           Main_Application_String); }

/*----------------------------------------------------------------------------*/
/*------------------------------------ begin ---------------------------------*/
/*----------------------------------------------------------------------------*/

static EXP_type compile_begin(PAI_type Operands)
  { BEG_type compiled_begin;
    EXP_type compiled_sequence;
    TAG_type tag;
    tag = Tag_of(Operands);
    switch (tag)
      { case NUL_tag:
          return Main_Error_Text(AL1_error_string,
                                 Main_Begin_String);
        case PAI_tag:
          compiled_sequence = compile_sequence(Operands,
                                               Main_Begin_String);
          survive_default(compiled_sequence);
          compiled_begin = make_BEG(compiled_sequence);
          return compiled_begin; }
    return Main_Error_Text(ITF_error_string,
                           Main_Begin_String); }

/*----------------------------------------------------------------------------*/
/*------------------------------------ define --------------------------------*/
/*----------------------------------------------------------------------------*/

static DFF_type compile_define_function(EXP_type Operands)
  { EXP_type compiled_body,
             compiled_definition,
             residue;
    NBR_type frame_size,
             offset,
             param_count;
    PAI_type body,
             operands,
             parameters,
             pattern;
    SYM_type symbol,
             vararg;
    TAG_type tag;
    UNS_type raw_frame_size,
             raw_offset,
             raw_param_count;
    tag = Tag_of(Operands);
    switch (tag)
      { case NUL_tag:
          return Main_Error_Text(MSP_error_string,
                                 Main_Define_String);
        case PAI_tag:
          operands = Stack_Push(Operands);
          pattern = operands->car;
          symbol = pattern->car;
          if (!is_SYM(symbol))
            return Main_Error_Text(IVV_error_string,
                                   Main_Define_String);
          Main_Claim(Main_Default_Margin);
          raw_offset = Dictionary_Define(symbol);
          Dictionary_Enter_Nested_Scope();
          operands = Stack_Peek();
          pattern = operands->car;
          parameters = pattern->cdr;
          residue = compile_parameters(parameters,
                                       Main_Define_String);
          operands = Stack_Peek();
          pattern = operands->car;
          body    = operands->cdr;
          if (!is_PAI(body))
            return Main_Error_Text(AL1_error_string,
                                   Main_Define_String);
          symbol = pattern->car;
          Stack_Poke(symbol);
          raw_param_count = Dictionary_Get_Frame_Size();
          tag = Tag_of(residue);
          switch (tag)
            { case NUL_tag:
                compiled_body = compile_sequence(body,
                                                 Main_Define_String);
                raw_frame_size = Dictionary_Exit_Nested_Scope();
                survive_default(compiled_body);
                offset = make_NBR(raw_offset);
                param_count = make_NBR(raw_param_count);
                frame_size = make_NBR(raw_frame_size);
                symbol = Stack_Pop();
                compiled_definition = make_DFF(symbol,
                                               offset,
                                               param_count,
                                               frame_size,
                                               compiled_body);
                return compiled_definition;
              case SYM_tag:
                survive_default(body);
                vararg = residue;
                Dictionary_Define(vararg);
                compiled_body = compile_sequence(body,
                                                 Main_Define_String);
                raw_frame_size = Dictionary_Exit_Nested_Scope();
                survive_default(compiled_body);
                offset = make_NBR(raw_offset);
                param_count = make_NBR(raw_param_count);
                frame_size = make_NBR(raw_frame_size);
                symbol = Stack_Pop();
                compiled_definition = make_DFZ(symbol,
                                               offset,
                                               param_count,
                                               frame_size,
                                               compiled_body);
                return compiled_definition; }
          return Main_Error_Text(IPA_error_string,
                                 Main_Define_String); }
    return Main_Error_Text(ITF_error_string,
                           Main_Define_String); }

static DFV_type compile_define_variable(PAI_type Operands)
  { DFV_type compiled_definition;
    EXP_type compiled_expression,
             expression,
             residue;
    NBR_type offset;
    PAI_type expressions,
             operands;
    SYM_type symbol;
    TAG_type tag;
    UNS_type raw_offset;
    expressions = Operands->cdr;
    tag = Tag_of(expressions);
    switch (tag)
      { case NUL_tag:
          return Main_Error_Text(TFX_error_string,
                                 Main_Define_String);
        case PAI_tag:
          residue = expressions->cdr;
          tag = Tag_of(residue);
          switch (tag)
            { case NUL_tag:
                operands = Stack_Push(Operands);
                expressions = operands->cdr;
                expression = expressions->car;
                compiled_expression = compile_expression(expression);
                survive_default(compiled_expression);
                operands = Stack_Pop();
                symbol = operands->car;
                raw_offset = Dictionary_Define(symbol);
                offset = make_NBR(raw_offset);
                compiled_definition = make_DFV(offset,
                                               compiled_expression);
                return compiled_definition;
              case PAI_tag:
                return Main_Error_Text(TMX_error_string,
                                       Main_Define_String); }
          return Main_Error_Text(ITF_error_string,
                                 Main_Define_String); }
    return Main_Error_Text(ITF_error_string,
                           Main_Define_String); }

static EXP_type compile_define(PAI_type Operands)
  { EXP_type compiled_definition,
             pattern;
    TAG_type tag;
    tag = Tag_of(Operands);
    switch (tag)
      { case NUL_tag:
          return Main_Error_Text(MSP_error_string,
                                 Main_Define_String);
        case PAI_tag:
          pattern = Operands->car;
          tag = Tag_of(pattern);
          switch (tag)
            { case PAI_tag:
                compiled_definition = compile_define_function(Operands);
                return compiled_definition;
              case SYM_tag:
                compiled_definition = compile_define_variable(Operands);
                return compiled_definition; }
          return Main_Error_Text(IVP_error_string,
                                 Main_Define_String); }
    return Main_Error_Text(ITF_error_string,
                           Main_Define_String); }

/*----------------------------------------------------------------------------*/
/*------------------------------------ if ------------------------------------*/
/*----------------------------------------------------------------------------*/

static EXP_type compile_if(PAI_type Operands)
  { EXP_type alternative,
             compiled_alternative,
             compiled_consequent,
             compiled_if,
             compiled_predicate,
             consequent,
             predicate,
             residue;
    PAI_type alternatives,
             clauses,
             operands;
    TAG_type tag;
    tag = Tag_of(Operands);
    switch (tag)
      { case NUL_tag:
          return Main_Error_Text(TFX_error_string,
                                 Main_If_String);
        case PAI_tag:
          clauses = Operands->cdr;
          tag = Tag_of(clauses);
          switch (tag)
            { case NUL_tag:
                return Main_Error_Text(TFX_error_string,
                                       Main_If_String);
              case PAI_tag:
                operands = Stack_Push(Operands);
                predicate = operands->car;
                compiled_predicate = compile_expression(predicate);
                survive_default(compiled_predicate);
                operands = Stack_Peek();
                Stack_Poke(compiled_predicate);
                clauses = operands->cdr;
                alternatives = clauses->cdr;
                tag = Tag_of(alternatives);
                switch (tag)
                  { case NUL_tag:
                      consequent = clauses->car;
                      compiled_consequent = compile_inline(consequent);
                      compiled_predicate = Stack_Pop();
                      compiled_if = make_IFZ(compiled_predicate,
                                             compiled_consequent);
                      return compiled_if;
                    case PAI_tag:
                      residue = alternatives->cdr;
                      if (!is_NUL(residue))
                        return Main_Error_Text(TMX_error_string,
                                               Main_If_String);
                      clauses = Stack_Push(clauses);
                      consequent = clauses->car;
                      compiled_consequent = compile_inline(consequent);
                      clauses = Stack_Peek();
                      alternatives = clauses->cdr;
                      alternative = alternatives->car;
                      Stack_Poke(compiled_consequent);
                      compiled_alternative = compile_inline(alternative);
                      compiled_consequent = Stack_Pop();
                      compiled_predicate = Stack_Pop();
                      compiled_if = make_IFF(compiled_predicate,
                                             compiled_consequent,
                                             compiled_alternative);
                      return compiled_if; }
                return Main_Error_Text(ITF_error_string,
                                       Main_If_String); }
          return Main_Error_Text(ITF_error_string,
                                 Main_If_String); }
    return Main_Error_Text(ITF_error_string,
                           Main_If_String); }

/*----------------------------------------------------------------------------*/
/*------------------------------ inline expression ---------------------------*/
/*----------------------------------------------------------------------------*/

static EXP_type compile_inline(EXP_type Expression)
  { EXP_type compiled_expression,
             expression;
    NBR_type frame_size;
    UNS_type raw_frame_size;
    expression = Stack_Push(Expression);
    Dictionary_Enter_Nested_Scope();
    compiled_expression = compile_expression(expression);
    raw_frame_size = Dictionary_Exit_Nested_Scope();
    survive_default(compiled_expression);
    if (raw_frame_size == 0)
      { expression = Stack_Pop();
        return compile_expression(expression); }   
    Stack_Zap();
    frame_size = make_NBR(raw_frame_size);
    return make_THK(compiled_expression,
                    frame_size); }
                        
/*----------------------------------------------------------------------------*/
/*------------------------------ inline sequence -----------------------------*/
/*----------------------------------------------------------------------------*/

static EXP_type compile_inline_sequence(PAI_type Sequence,
                                        TXT_type Error_string)
  { EXP_type compiled_sequence;
    NBR_type frame_size;
    PAI_type sequence;
    UNS_type raw_frame_size;
    sequence = Stack_Push(Sequence);
    Dictionary_Enter_Nested_Scope();
    compiled_sequence = compile_sequence(sequence,
                                         Error_string);
    raw_frame_size = Dictionary_Exit_Nested_Scope();
    survive_default(compiled_sequence);
    if (raw_frame_size == 0)
      { sequence = Stack_Pop();
        return compile_sequence(sequence,
                                Error_string); }   
    Stack_Zap();
    frame_size = make_NBR(raw_frame_size);
    return make_THK(compiled_sequence,
                    frame_size); }
                        
/*----------------------------------------------------------------------------*/
/*------------------------------------ lambda --------------------------------*/
/*----------------------------------------------------------------------------*/

static EXP_type compile_lambda(PAI_type Operands)
  { EXP_type compiled_body,
             compiled_lambda,
             parameters,
             residue;
    NBR_type frame_size,
             param_count;
    PAI_type body,
             operands;
    TAG_type tag;
    UNS_type raw_frame_size,
             raw_param_count;
    tag = Tag_of(Operands);
    switch (tag)
      { case NUL_tag:
          return Main_Error_Text(TFX_error_string,
                                 Main_Lambda_String);
        case PAI_tag:
          Stack_Push(Operands);
          Main_Claim(Main_Default_Margin);
          Dictionary_Enter_Nested_Scope();
          operands = Stack_Peek();
          parameters = operands->car;
          body       = operands->cdr;
          Stack_Poke(body);
          residue = compile_parameters(parameters,
                                       Main_Lambda_String);
          raw_param_count = Dictionary_Get_Frame_Size();
          tag = Tag_of(residue);
          switch (tag)
            { case NUL_tag:
                body = Stack_Pop();
                compiled_body = compile_sequence(body,
                                                 Main_Lambda_String);
                raw_frame_size = Dictionary_Exit_Nested_Scope();
                survive_default(compiled_body);
                param_count = make_NBR(raw_param_count);
                frame_size = make_NBR(raw_frame_size);
                compiled_lambda = make_LMB(param_count,
                                           frame_size,
                                           compiled_body);
                return compiled_lambda;
              case SYM_tag:
                survive_default(residue);
                Dictionary_Define(residue);
                body = Stack_Pop();
                compiled_body = compile_sequence(body,
                                                 Main_Lambda_String);
                raw_frame_size  = Dictionary_Exit_Nested_Scope();
                survive_default(compiled_body);
                param_count = make_NBR(raw_param_count);
                frame_size =make_NBR(raw_frame_size);
                compiled_lambda = make_LMZ(param_count,
                                           frame_size,
                                           compiled_body);
                return compiled_lambda; }
          return Main_Error_Text(IPA_error_string,
                                 Main_Define_String); }
    return Main_Error_Text(ITF_error_string,
                           Main_Lambda_String); }

/*----------------------------------------------------------------------------*/
/*---------------------------------- operands --------------------------------*/
/*----------------------------------------------------------------------------*/

static VEC_type compile_operands(PAI_type Operands,
                                 TXT_type Error_string)
  { EXP_type compiled_expression,
             expression;
    PAI_type operands;
    UNS_type count;
    VEC_type compiled_operands;
    operands = Operands;
    count = 0;
    do
      { operands = Stack_Push(operands);
        expression = operands->car;
        compiled_expression = compile_expression(expression);
        operands = Stack_Peek();
        operands = operands->cdr;
        Stack_Poke(compiled_expression);
        ++count; }
    while (is_PAI(operands));
    if (!is_NUL(operands))
      return Main_Error_Text(ITF_error_string,
                             Error_string);
    Main_Claim(count);
    compiled_operands = make_VEC(count);
    for (;
         count > 0;
         count--)
      { compiled_expression = Stack_Pop();
        compiled_operands[count] = compiled_expression; }
    return compiled_operands; }

/*----------------------------------------------------------------------------*/
/*-------------------------------- parameters --------------------------------*/
/*----------------------------------------------------------------------------*/

static EXP_type compile_parameters(PAI_type Parameters,
                                   TXT_type Error_string)
  { EXP_type parameter;
    PAI_type parameters;
    for (parameters = Parameters;
         is_PAI(parameters);
         parameters = parameters->cdr)
      { survive_default(parameters);
        parameter = parameters->car;
        if (!is_SYM(parameter))
          return Main_Error_Text(IPA_error_string,
                                 Error_string);
        Dictionary_Define(parameter); }
    return parameters; }

/*----------------------------------------------------------------------------*/
/*------------------------------------ quote ---------------------------------*/
/*----------------------------------------------------------------------------*/

static EXP_type compile_quote(PAI_type Operands)
  { EXP_type expression,
             residue;
    QUO_type compiled_quotation;
    TAG_type tag;
    tag = Tag_of(Operands);
    switch (tag)
      { case NUL_tag:
          return Main_Error_Text(E1X_error_string,
                                 Main_Quote_String);
        case PAI_tag:
          expression = Operands->car;
          residue    = Operands->cdr;
          if (!is_NUL(residue))
            return Main_Error_Text(TMX_error_string,
                                   Main_Quote_String);
          survive_default(expression);
          compiled_quotation = make_QUO(expression);
          return compiled_quotation; }                         /* clone!!! */
    return Main_Error_Text(ITF_error_string,
                           Main_Quote_String); }

/*----------------------------------------------------------------------------*/
/*---------------------------------- sequence --------------------------------*/
/*----------------------------------------------------------------------------*/

static EXP_type compile_sequence(PAI_type Sequence,
                                 TXT_type Error_string)
  { EXP_type compiled_expression,
             expression;
    PAI_type sequence;
    SEQ_type compiled_sequence;
    UNS_type count;
    sequence = Sequence->cdr;
    if (is_NUL(sequence))
      { expression = Sequence->car;
        compiled_expression = compile_expression(expression);
        return compiled_expression; }
    sequence = Sequence;
    for (count = 0;
         is_PAI(sequence);
         count++)
      { sequence = Stack_Push(sequence);
        expression = sequence->car;
        compiled_expression = compile_expression(expression);
        sequence = Stack_Peek();
        sequence = sequence->cdr;
        Stack_Poke(compiled_expression); }
    if (!is_NUL(sequence))
      return Main_Error_Text(ITF_error_string,
                             Error_string);
    Main_Claim(count);
    compiled_sequence = make_SEQ(count);
    for (;
         count > 0;
         count--)
      { compiled_expression = Stack_Pop();
        compiled_sequence[count] = compiled_expression; }
    return compiled_sequence; }

/*----------------------------------------------------------------------------*/
/*------------------------------------ set -----------------------------------*/
/*----------------------------------------------------------------------------*/

static EXP_type compile_set(PAI_type Operands)
  { EXP_type compiled_expression,
             compiled_set,
             expression;
    NBR_type offset,
             scope;
    PAI_type expressions,
             operands,
             residue;
    SYM_type variable;
    TAG_type tag;
    UNS_type raw_offset,
             raw_scope;
    tag = Tag_of(Operands);
    switch (tag)
      { case NUL_tag:
          return Main_Error_Text(MSV_error_string,
                                 Main_Set_String);
        case PAI_tag:
          variable = Operands->car;
          if (is_SYM(variable))
            { expressions = Operands->cdr;
              tag = Tag_of(expressions);
              switch (tag)
                { case NUL_tag:
                    return Main_Error_Text(E1X_error_string,
                                           Main_Set_String);
                  case PAI_tag:
                    residue = expressions->cdr;
                    if (is_NUL(residue))
                      { operands = Stack_Push(Operands);
                        expressions = operands->cdr;
                        expression = expressions->car;
                        compiled_expression = compile_expression(expression);
                        operands = Stack_Pop();
                        variable = operands->car;
                        raw_offset = Dictionary_Lexical_Address(variable,
                                                            &raw_scope);
                        if (raw_offset == 0)
                          return Main_Error_Symbol(VNF_error_string,
                                                   variable);
                        survive_default(compiled_expression);
                        offset = make_NBR(raw_offset);
                        if (raw_scope == 0)
                          compiled_set = make_STL(offset,
                                                  compiled_expression);
                        else
                          { scope = make_NBR(raw_scope);
                            compiled_set = make_STG(scope,
                                                    offset,
                                                    compiled_expression); }
                        return compiled_set; }
                    return Main_Error_Text(TMX_error_string,
                                           Main_Set_String); }
              return Main_Error_Text(ITF_error_string,
                                     Main_Set_String); }
          return Main_Error_Text(IVV_error_string,
                                 Main_Set_String); }
    return Main_Error_Text(ITF_error_string,
                           Main_Set_String); }

/*----------------------------------------------------------------------------*/
/*------------------------------------ symbol --------------------------------*/
/*----------------------------------------------------------------------------*/

static EXP_type compile_symbol(SYM_type Variable)
  { EXP_type compiled_variable;
    NBR_type offset,
             scope;
    UNS_type raw_offset,
             raw_scope;
    raw_offset = Dictionary_Lexical_Address(Variable,
                                            &raw_scope);
    if (raw_offset == 0)
      return Main_Error_Symbol(VNF_error_string,
                               Variable);
    Main_Claim(Main_Default_Margin);
    if (raw_scope == 0)
      { offset = make_NBR(raw_offset);
        compiled_variable = make_LCL(offset); }
    else
      { scope  = make_NBR(raw_scope);
        offset = make_NBR(raw_offset);
        compiled_variable = make_GLB(scope,
                                     offset); }
    return compiled_variable; }

/*----------------------------------------------------------------------------*/
/*----------------------------------- value ----------------------------------*/
/*----------------------------------------------------------------------------*/

static EXP_type compile_value(EXP_type Value)
  { return Value; }

/*----------------------------------------------------------------------------*/
/*----------------------------------- while ----------------------------------*/
/*----------------------------------------------------------------------------*/

static EXP_type compile_while(PAI_type Operands)
  { EXP_type compiled_body,
             compiled_predicate,
             predicate;
    PAI_type body,
             operands;
    TAG_type tag;
    WHI_type compiled_while;
    tag = Tag_of(Operands);
    switch (tag)
      { case NUL_tag:
          return Main_Error_Text(TFX_error_string,
                                 Main_While_String);
        case PAI_tag:
          body = Operands->cdr;
          tag = Tag_of(body);
          switch (tag)
            { case NUL_tag:
                return Main_Error_Text(TFX_error_string,
                                       Main_While_String);
              case PAI_tag:
                survive_default(Operands);
                operands = Stack_Push(Operands);
                predicate = operands->car;
                compiled_predicate = compile_inline(predicate);
                operands = Stack_Peek();
                Stack_Poke(compiled_predicate);
                body = operands->cdr;
                compiled_body = compile_inline_sequence(body,
                                                        Main_While_String);
                compiled_predicate = Stack_Pop();
                compiled_while = make_WHI(compiled_predicate,
                                          compiled_body);
                return compiled_while; }
          return Main_Error_Text(ITF_error_string,
                                 Main_While_String); }
    return Main_Error_Text(ITF_error_string,
                           Main_While_String); }

/*----------------------------------------------------------------------------*/
/*------------------------------------ form ----------------------------------*/
/*----------------------------------------------------------------------------*/

static EXP_type compile_form(PAI_type Form)
  { EXP_type operands,
             operator;
    operator = Form->car;
    operands = Form->cdr;
    if (operator == Main_Begin)
          return compile_begin(operands);
    if (operator == Main_Define)
         return compile_define(operands);
    if (operator == Main_If)
             return compile_if(operands);
    if (operator == Main_Lambda)
         return compile_lambda(operands);
    if (operator == Main_Quote)
          return compile_quote(operands);
    if (operator == Main_Set)
            return compile_set(operands);
    if (operator == Main_While)
          return compile_while(operands);
    return compile_application(Form); }

/*----------------------------------------------------------------------------*/
/*------------------------------------ expression ----------------------------*/
/*----------------------------------------------------------------------------*/

static EXP_type compile_expression(EXP_type Expression)
  { TAG_type tag;
    tag = Tag_of(Expression);
    switch (tag)
      { case PAI_tag:
            return compile_form(Expression);
        case SYM_tag:
          return compile_symbol(Expression);
        case CHA_tag:
        case FLS_tag:
        case NBR_tag:
        case NUL_tag:
        case REA_tag:
        case STR_tag:
        case TRU_tag:
        case VEC_tag:
           return compile_value(Expression);
        case APL_tag:
        case BEG_tag:
        case CNT_tag:
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
        case NAT_tag:
        case PRC_tag:
        case PRZ_tag:
        case QUO_tag:
        case STG_tag:
        case STL_tag:
        case THK_tag:
        case THR_tag:
        case THr_tag:
        case USP_tag:
        case WHI_tag:
          Main_Error_Tag(ILT_error_string,
                         tag); }
    return Main_Error_Tag(IXT_error_string,
                          tag); }

/*------------------------------ public functions ----------------------------*/

EXP_type Compile_Compile(EXP_type Expression)
  { EXP_type compiled_expression;
    compiled_expression = compile_expression(Expression);
    return compiled_expression; }

NIL_type Compile_Initialize(NIL_type)
  { return; }
