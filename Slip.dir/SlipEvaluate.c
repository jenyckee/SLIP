                     /*-----------------------------------*/
                     /*             >>>Slip<<<            */
                     /*            Theo D'Hondt           */
                     /*     VUB Software Languages Lab    */
                     /*             (c) 2010              */
                     /*-----------------------------------*/
                     /*     version 12: smart caching     */
                     /*-----------------------------------*/
                     /*              evaluate             */
                     /*-----------------------------------*/

#include "SlipMain.h"

#include "SlipEnvironment.h"
#include "SlipEvaluate.h"
#include "SlipThread.h"

/*------------------------------ private macros ------------------------------*/

#define begin_frame(SIG)                                                       \
  typedef struct SIG * SIG##_type;                                             \
  struct SIG { CEL_type hdr;                                                   \
               NBR_type tid;                                                   \
               THR_type thr;                                                   \
               EXP_type tcs

#define end_frame(SIG)                                                         \
  } SIG;                                                                       \
static const UNS_type SIG##_size = ((sizeof(SIG)-sizeof(THR))/sizeof(CEL_type))

#define frame_slot(SIG, NAM)                                                   \
  SIG##_type NAM

#define survive_default(SRV)                                                   \
  Main_Survival_Claim((REF_type)&SRV, Main_Default_Margin)

#define survive_dynamic(SRV, SIZ)                                              \
  Main_Survival_Claim((REF_type)&SRV, SIZ)

/*------------------------------ private constants ---------------------------*/

static const TXT_type BRQ_error_string = "boolean value required";
static const TXT_type CRQ_error_string = "continuation requires single value";
static const TXT_type IFS_error_string = "illegal frame size";
static const TXT_type ILT_error_string = "illegal expression type";
static const TXT_type ITA_error_string = "improperly terminated application";
//static const TXT_type ITP_error_string = "improperly terminated procedure";
//static const TXT_type IPA_error_string = "invalid parameter";
static const TXT_type IXT_error_string = "invalid expression type";
static const TXT_type PNR_error_string = "procedure or native required";
static const TXT_type TFO_error_string = "too few operands in application";
static const TXT_type TMO_error_string = "too many operands in application";

/*------------------------------ private prototypes --------------------------*/

static EXP_type          evaluate_bindings(PRC_type,
                                           VEC_type,
                                           EXP_type);
static EXP_type              evaluate_body(EXP_type,
                                           VEC_type,
                                           VEC_type,
                                           EXP_type);
static EXP_type evaluate_continuation_call(CNT_type,
                                           VEC_type,
                                           EXP_type);
static EXP_type        evaluate_expression(EXP_type,
                                           EXP_type);
static EXP_type       evaluate_native_call(NAT_type,
                                           VEC_type,
                                           EXP_type);
static EXP_type          evaluate_sequence(VEC_type,
                                           EXP_type);
static EXP_type   evaluate_vararg_bindings(PRZ_type,
                                           VEC_type,
                                           EXP_type);

/*------------------------------ private functions ---------------------------*/

static EXP_type continue_with(EXP_type Value)
  { THR_type thread;
    CCC_type thread_function;
    EXP_type call_status;
    NBR_type thread_id;
    thread = Thread_Peek();
    thread_id   = thread->tid;
    call_status = thread->tcs;
    thread_function = Thread_Retrieve(thread_id);
    return thread_function(Value,
                           call_status); }

static SGN_type list_length(PAI_type List)
  { PAI_type pair;
    SGN_type count;
    count = 0;
    for (pair = List;
         is_PAI(pair);
         pair = pair->cdr)
      count++;
    if (is_NUL(pair))
      return count;
    return -1; }

/*----------------------------------------------------------------------------*/
/*------------------------------------ application ---------------------------*/
/*----------------------------------------------------------------------------*/

static const TXT_type Application_String = "application";

static NBR_type Continue_application;

begin_frame(aPL);
 frame_slot(VEC, opd);
end_frame(aPL);

static EXP_type continue_application(EXP_type Procedure,
                                     EXP_type Tail_call)
  { aPL_type application_thread;
    TAG_type tag;
    VEC_type operands;
    survive_default(Procedure);
    application_thread = (aPL_type)Thread_Peek();
    operands = application_thread->opd;
    tag = Tag_of(Procedure);
    switch (tag)
      { case CNT_tag:
          return evaluate_continuation_call(Procedure,
                                            operands,
                                            Tail_call);
        case NAT_tag:
          return       evaluate_native_call(Procedure,
                                            operands,
                                            Tail_call);
        case PRC_tag:
          return          evaluate_bindings(Procedure,
                                            operands,
                                            Tail_call);
        case PRZ_tag:
          return   evaluate_vararg_bindings(Procedure,
                                            operands,
                                            Tail_call); }
    return Main_Error_Text(PNR_error_string,
                           Application_String); }

/*----------------------------------------------------------------------------*/

static EXP_type evaluate_application(APL_type Application,
                                     EXP_type Tail_call)
  { aPL_type application_thread;
    EXP_type expression;
    VEC_type operands;
    expression = Application->exp;
    operands   = Application->opr;
    application_thread = (aPL_type)Thread_Push(Continue_application,
                                               Tail_call,
                                               aPL_size);
    application_thread->opd = operands;
    return evaluate_expression(expression,
                               Main_False); }

static NIL_type initialize_application(NIL_type)
  { Continue_application = Thread_Register(continue_application,
                                           aPL_size); }

/*----------------------------------------------------------------------------*/
/*------------------------------------ begin ---------------------------------*/
/*----------------------------------------------------------------------------*/

static EXP_type evaluate_begin(BEG_type Begin,
                               EXP_type Tail_call)
  { EXP_type sequence;
    sequence = Begin->seq;
    return evaluate_expression(sequence,
                               Tail_call); }

/*----------------------------------------------------------------------------*/
/*---------------------------------- bindings --------------------------------*/
/*----------------------------------------------------------------------------*/

static NBR_type Continue_bindings;

begin_frame(bND);
 frame_slot(PRC, prc);
 frame_slot(VEC, opd);
 frame_slot(VEC, frm);
 frame_slot(NBR, pos);
end_frame(bND);

static EXP_type continue_bindings(EXP_type Value,
                                  EXP_type Tail_call) 
  { bND_type binding_thread;
    EXP_type expression,
             tail_call;
    NBR_type param_count,
             position;
    PRC_type procedure;
    VEC_type body,
             operands;
    UNS_type index,
             raw_param_count;
    VEC_type environment,
             frame;
    survive_default(Value);
    binding_thread = (bND_type)Thread_Peek();
    procedure = binding_thread->prc;
    frame     = binding_thread->frm;
    position  = binding_thread->pos;
    index = get_NBR(position);
    frame[index] = Value;
    param_count = procedure->par;
    raw_param_count = get_NBR(param_count);
    if (index < raw_param_count)
      { binding_thread = (bND_type)Thread_Keep();
        position = make_NBR(++index);
        operands = binding_thread->opd;
        binding_thread->pos = position;
        expression = operands[index];
        tail_call = (index == raw_param_count)? Tail_call : Main_False;
        return evaluate_expression(expression,
                                   tail_call); }
    body        = procedure->bod;
    environment = procedure->env;
    return evaluate_body(body,
                         environment,
                         frame,
                         Tail_call); }

/*----------------------------------------------------------------------------*/

static EXP_type evaluate_bindings(PRC_type Procedure,
                                  VEC_type Operands,
                                  EXP_type Tail_call)
  { bND_type binding_thread;
    EXP_type expression,
             tail_call;
    NBR_type frame_size,
             param_count;
    UNS_type operand_size,
             raw_frame_size,
             raw_param_count;
    VEC_type body,
             environment,
             frame,
             operands;
    param_count = Procedure->par;
    raw_param_count = get_NBR(param_count);
    operand_size = size_VEC(Operands);
    if (raw_param_count > operand_size)
      return Main_Error_Procedure(TFO_error_string,
                                  Procedure);
    if (raw_param_count < operand_size)
      return Main_Error_Procedure(TMO_error_string,
                                  Procedure);
    frame_size = Procedure->siz;
    raw_frame_size = get_NBR(frame_size);
    if (raw_frame_size == 0)
      { body        = Procedure->bod;
        environment = Procedure->env;
        return evaluate_body(body,
                             environment,
                             Main_Empty_Vector,
                             Tail_call); }
    if (raw_param_count == 0)
      { survive_dynamic(Procedure,
                        raw_frame_size + Main_Default_Margin);
        body        = Procedure->bod;
        environment = Procedure->env;
        frame = Environment_Make_Frame(raw_frame_size);
        return evaluate_body(body,
                             environment,
                             frame,
                             Tail_call); }
    binding_thread = (bND_type)Thread_Poke(Continue_bindings,
                                           Tail_call,
                                           bND_size);
    binding_thread->prc = Procedure;
    binding_thread->opd = Operands;
    survive_dynamic(binding_thread,
                    raw_frame_size);
    frame = Environment_Make_Frame(raw_frame_size);
    binding_thread->frm = frame;
    binding_thread->pos = Main_One;
    operands = binding_thread->opd;
    expression = operands[1];
    tail_call = (operand_size == 1)? Tail_call : Main_False;
    return evaluate_expression(expression,
                               tail_call); }

static NIL_type initialize_bindings(NIL_type)
  { Continue_bindings = Thread_Register(continue_bindings,
                                        bND_size); }

/*----------------------------------------------------------------------------*/
/*-------------------------------------- body --------------------------------*/
/*----------------------------------------------------------------------------*/

static NBR_type Continue_body;
static NBR_type Continue_inline;

begin_frame(bOD);
 frame_slot(VEC, env);
 frame_slot(VEC, frm);
end_frame(bOD);

static EXP_type continue_body(EXP_type Value,
                              EXP_type Tail_call)
  { bOD_type body_thread;
    VEC_type environment,
             frame;
    body_thread = (bOD_type)Thread_Peek();
    environment = body_thread->env;
    frame       = body_thread->frm;
    Thread_Zap();
    Environment_Release_Frame();
    Environment_Replace(environment,
                        frame);
    return Value; }

/*----------------------------------------------------------------------------*/

static EXP_type evaluate_body(EXP_type Body, 
                              VEC_type Environment,
                              VEC_type Frame,
                              EXP_type Tail_call)
  { bOD_type body_thread;
    VEC_type environment,
             frame;
    if (is_FLS(Tail_call))
      { body_thread = (bOD_type)Thread_Poke(Continue_body,
                                            Main_False,
                                            bOD_size);
        environment = Environment_Get_Environment();
        frame = Environment_Get_Frame();
        body_thread->env = environment;
        body_thread->frm = frame; }
    else
      { Environment_Release_Frame();
        Thread_Zap(); }
    Environment_Replace(Environment,
                        Frame);
    return evaluate_expression(Body,
                               Main_True); }

static EXP_type evaluate_body_push(VEC_type Body, 
                                   VEC_type Environment,
                                   VEC_type Frame,
                                   EXP_type Tail_call)
  { bOD_type body_thread;
    VEC_type environment,
             frame;
    if (is_FLS(Tail_call))
      { body_thread = (bOD_type)Thread_Push(Continue_body,
                                            Main_False,
                                            bOD_size);
        environment = Environment_Get_Environment();
        frame = Environment_Get_Frame();
        body_thread->env = environment;
        body_thread->frm = frame; }
    else
      Environment_Release_Frame();
    Environment_Replace(Environment,
                        Frame);
    return evaluate_expression(Body,
                               Main_True); }

static EXP_type evaluate_inline(EXP_type Expression,
                                UNS_type Frame_size,
                                EXP_type Tail_call)
  { bOD_type body_thread;
    UNS_type environment_size;
    VEC_type environment,
             frame;
    environment_size = Environment_Get_Environment_size();
    survive_dynamic(Expression,
                    environment_size + Frame_size + Main_Default_Margin);
    if (is_FLS(Tail_call))
      { body_thread = (bOD_type)Thread_Push(Continue_inline,
                                            Main_False,
                                            bOD_size);
        environment = Environment_Get_Environment();
        frame = Environment_Get_Frame();
        body_thread->env = environment;
        body_thread->frm = frame; }
    environment = Environment_Grow_Environment();
    frame = Environment_Make_Frame(Frame_size);
    Environment_Replace(environment,
                        frame);
    return evaluate_expression(Expression,
                               Main_True); }

static NIL_type initialize_body(NIL_type)
  { Continue_body = Thread_Register(continue_body,
                                    bOD_size);
    Continue_inline = Thread_Register(continue_body,
                                      bOD_size); }

/*----------------------------------------------------------------------------*/
/*-------------------------------- continuation call -------------------------*/
/*----------------------------------------------------------------------------*/

static NBR_type Continue_continuation_call;

begin_frame(cCL);
 frame_slot(CNT, cnt);
end_frame(cCL);

static EXP_type continue_continuation_call(EXP_type Value,
                                           EXP_type Tail_call) 
  { cCL_type continuation_call_thread;
    CNT_type continuation;
    THR_type thread;
    VEC_type environment,
             frame;
    continuation_call_thread = (cCL_type)Thread_Peek();
    continuation = continuation_call_thread->cnt;
    Thread_Zap();
    environment = continuation->env;
    frame       = continuation->frm;
    thread      = continuation->thr;
    Environment_Release_Frame();
    Environment_Replace(environment,
                        frame);
    Thread_Replace(thread);
    return continue_with(Value); }

/*----------------------------------------------------------------------------*/

static EXP_type evaluate_continuation_call(CNT_type Continuation,
                                           VEC_type Operands,
                                           EXP_type Tail_call)
  { cCL_type continuation_call_thread;
    EXP_type expression;
    UNS_type operand_size;
    operand_size = size_VEC(Operands);
    if (operand_size != 1)
      return Main_Error(CRQ_error_string);
    continuation_call_thread = (cCL_type)Thread_Poke(Continue_continuation_call,
                                                     Tail_call,
                                                     cCL_size);
    continuation_call_thread->cnt = Continuation;
    expression = Operands[1];
    return evaluate_expression(expression,
                               Main_True); }

static NIL_type initialize_continuation_call(NIL_type)
  { Continue_continuation_call = Thread_Register(continue_continuation_call,
                                                 cCL_size); }

/*----------------------------------------------------------------------------*/
/*------------------------------------ define --------------------------------*/
/*----------------------------------------------------------------------------*/

static NBR_type Continue_define;

begin_frame(dEF);
 frame_slot(NBR, ofs);
end_frame(dEF);

static EXP_type continue_define(EXP_type Value,
                                EXP_type Tail_call) 
  { dEF_type define_thread;
    NBR_type offset;
    UNS_type raw_offset;
    define_thread = (dEF_type)Thread_Peek();
    offset = define_thread->ofs;
    Thread_Zap();
    raw_offset = get_NBR(offset);
    return Environment_Local_Set(raw_offset,
                                 Value); }

/*----------------------------------------------------------------------------*/

static EXP_type evaluate_define_function(DFF_type Define) 
  { EXP_type body;
    NBR_type frame_size,
             offset,
             param_count;
    PRC_type procedure;
    SYM_type name;
    UNS_type environment_size,
             raw_offset;
    VEC_type environment;
    environment_size = Environment_Get_Environment_size();
    survive_dynamic(Define,
                    environment_size + Main_Default_Margin);
    environment = Environment_Grow_Environment();
    name        = Define->nam;
    offset      = Define->ofs;
    param_count = Define->par;
    frame_size  = Define->siz;
    body        = Define->bod;
    procedure = make_PRC(name,
                         param_count,
                         frame_size,
                         body,
                         environment);
    raw_offset = get_NBR(offset);
    return Environment_Local_Set(raw_offset,
                                 procedure); }

static EXP_type evaluate_define_function_vararg(DFZ_type Define)
  { EXP_type body;
    NBR_type frame_size,
             offset,
             param_count;
    PRZ_type procedure;
    SYM_type name;
    UNS_type environment_size,
             raw_offset;
    VEC_type environment;
    environment_size = Environment_Get_Environment_size();
    survive_dynamic(Define,
                    environment_size + Main_Default_Margin);
    environment = Environment_Grow_Environment();
    name        = Define->nam;
    offset      = Define->ofs;
    param_count = Define->par;
    frame_size  = Define->siz;
    body        = Define->bod;
    procedure = make_PRZ(name,
                         param_count,
                         frame_size,
                         body,
                         environment);
    raw_offset = get_NBR(offset);
    return Environment_Local_Set(raw_offset,
                                 procedure); }

static EXP_type evaluate_define_variable(DFV_type Define,
                                         EXP_type Tail_call) 
  { dEF_type define_thread;
    EXP_type expression;
    NBR_type offset;
    offset     = Define->ofs;
    expression = Define->exp;
    define_thread = (dEF_type)Thread_Push(Continue_define,
                                          Tail_call,
                                          dEF_size);
    define_thread->ofs = offset;
    return evaluate_expression(expression,
                               Main_False); }

static NIL_type initialize_define(NIL_type)
  { Continue_define = Thread_Register(continue_define,
                                      dEF_size); }

/*----------------------------------------------------------------------------*/
/*------------------------------------ direct_application --------------------*/
/*----------------------------------------------------------------------------*/

static const TXT_type Direct_Application_String = "direct application";

static EXP_type evaluate_direct_bindings(VEC_type Vector,
                                         EXP_type Tail_call)
  { EXP_type value;
    NBR_type frame_size,
             param_count;
    PAI_type arguments;
    PRC_type procedure;
    UNS_type raw_frame_size,
             raw_param_count,
             position;
    VEC_type body,
             environment,
             frame;
    procedure = Vector[1];
    frame_size = procedure->siz;
    raw_frame_size = get_NBR(frame_size);
    survive_dynamic(Vector,
                    raw_frame_size);
    procedure = Vector[1];
    arguments = Vector[2];
    param_count = procedure->par;
    body        = procedure->bod;
    environment = procedure->env;
    raw_param_count = get_NBR(param_count);
    if (raw_frame_size == 0)
      frame = Main_Empty_Vector;
    else
      { frame = Environment_Make_Frame(raw_frame_size);
        for (position = 1;
             position <= raw_param_count;
             position++)
          { if (!is_PAI(arguments))
              return Main_Error_Procedure(TFO_error_string,
                                          procedure);
            value     = arguments->car;
            arguments = arguments->cdr;
            frame[position] = value; }}
    if (!is_NUL(arguments))
      return Main_Error_Procedure(TMO_error_string,
                                  procedure);
    return evaluate_body_push(body,
                              environment,
                              frame,
                              Tail_call); }

static EXP_type evaluate_direct_native_call(VEC_type Vector,
                                            EXP_type Tail_call)
  { EXP_type value;
    FUN_type function;
    NAT_type native;
    PAI_type arguments;
    SGN_type length;
    TXT_type name;
    UNS_type position;
    VEC_type vector;
    native    = Vector[1];
    arguments = Vector[2];
    function = native->fun;
    length = list_length(arguments);
    switch (length)
      { case -1:
          name = native->nam;
          return Main_Error_Text(ITA_error_string,
                                 name);
        case 0:
          vector = Main_Empty_Vector;
          break;
        default:
          survive_dynamic(arguments,
                          length + Main_Default_Margin);
          vector = Environment_Make_Frame(length);
          for (position = 1;
               position <= length;
               position++)
            { value     = arguments->car;
              arguments = arguments->cdr;
              vector[position] = value; }}
    value = function(vector,
                     Tail_call);
    Environment_Release_Vector(vector);
    return value; }

static EXP_type evaluate_direct_vararg_bindings(VEC_type Vector,
                                                EXP_type Tail_call)
  { EXP_type value;
    NBR_type frame_size,
             param_count;
    PAI_type arguments;
    PRZ_type procedure;
    UNS_type raw_frame_size,
             raw_param_count,
             position;
    VEC_type body,
             environment,
             frame;
    procedure = Vector[1];
    frame_size = procedure->siz;
    raw_frame_size = get_NBR(frame_size);
    survive_dynamic(Vector,
                    raw_frame_size + Main_Default_Margin);
    procedure = Vector[1];
    arguments = Vector[2];
    param_count = procedure->par;
    body        = procedure->bod;
    environment = procedure->env;
    raw_param_count = get_NBR(param_count);
    if (raw_frame_size == 0)
      return Main_Error_Procedure_Vararg(IFS_error_string,
                                         procedure);
    frame = Environment_Make_Frame(raw_frame_size);
    for (position = 1;
         position <= raw_param_count;
         position++)
      { if (!is_PAI(arguments))
          return Main_Error_Procedure_Vararg(TFO_error_string,
                                             procedure);
        value     = arguments->car;
        arguments = arguments->cdr;
        frame[position] = value; }
    frame[position] = arguments;
    return evaluate_body_push(body,
                              environment,
                              frame,
                              Tail_call); }

/*----------------------------------------------------------------------------*/

static EXP_type evaluate_direct_application(VEC_type Vector,
                                            EXP_type Tail_call)
  { EXP_type value;
    TAG_type tag;
    value = Vector[1];
    tag = Tag_of(value);
    switch (tag)
      { case NAT_tag:
          return     evaluate_direct_native_call(Vector,
                                                 Tail_call);
        case PRC_tag:
          return        evaluate_direct_bindings(Vector,
                                                 Tail_call);
        case PRZ_tag:
          return evaluate_direct_vararg_bindings(Vector,
                                                 Tail_call); }
    return Main_Error_Text(PNR_error_string,
                           Direct_Application_String); }

/*----------------------------------------------------------------------------*/
/*------------------------------------ global variable -----------------------*/
/*----------------------------------------------------------------------------*/

static EXP_type evaluate_global_variable(GLB_type Global) 
  { NBR_type offset,
             scope;
    UNS_type raw_scope,
             raw_offset;
    scope  = Global->scp;
    offset = Global->ofs;
    raw_scope = get_NBR(scope);
    raw_offset = get_NBR(offset);
    return Environment_Global_Get(raw_scope,
                                  raw_offset); }

/*----------------------------------------------------------------------------*/
/*------------------------------------ if ------------------------------------*/
/*----------------------------------------------------------------------------*/

static NBR_type Continue_if_double;

begin_frame(iFD);
 frame_slot(EXP, cns);
 frame_slot(EXP, alt);
end_frame(iFD);

static EXP_type continue_if_double(EXP_type Boolean,
                                   EXP_type Tail_call) 
  { iFD_type if_thread;
    EXP_type alternative,
             consequent;
    TAG_type tag;
    survive_default(Boolean);
    tag = Tag_of(Boolean);
    if_thread = (iFD_type)Thread_Peek();
    switch (tag)
      { case FLS_tag:
          alternative = if_thread->alt;
          Thread_Zap();
          return evaluate_expression(alternative,
                                     Tail_call);
        case TRU_tag:
          consequent = if_thread->cns;
          Thread_Zap();
          return evaluate_expression(consequent,
                                     Tail_call); }
    return Main_Error_Text(BRQ_error_string,
                           Main_If_String); }

/*----------------------------------------------------------------------------*/

static NBR_type Continue_if_single;

begin_frame(iFS);
 frame_slot(EXP, cns);
end_frame(iFS);

static EXP_type continue_if_single(EXP_type Boolean,
                                   EXP_type Tail_call) 
  { iFS_type if_thread;
    EXP_type consequent;
    TAG_type tag;
    survive_default(Boolean);
    tag = Tag_of(Boolean);
    if_thread = (iFS_type)Thread_Peek();
    switch (tag)
      { case FLS_tag:
          Thread_Zap();
          return Main_Unspecified;
        case TRU_tag:
          consequent = if_thread->cns;
          Thread_Zap();
          return evaluate_expression(consequent,
                                     Tail_call); }
    return Main_Error_Text(BRQ_error_string,
                           Main_If_String); }
/*----------------------------------------------------------------------------*/

static EXP_type evaluate_if_double(IFF_type If,
                                   EXP_type Tail_call) 
  { iFD_type if_thread;
    EXP_type alternative,
             consequent,
             predicate;
    predicate   = If->prd;
    consequent  = If->cns;
    alternative = If->alt;
    if_thread = (iFD_type)Thread_Push(Continue_if_double,
                                      Tail_call,
                                      iFD_size);
    if_thread->cns = consequent;
    if_thread->alt = alternative;
    return evaluate_expression(predicate,
                               Main_False); }

static EXP_type evaluate_if_single(IFZ_type If,
                                   EXP_type Tail_call) 
  { iFS_type if_thread;
    EXP_type consequent,
             predicate;
    predicate  = If->prd;
    consequent = If->cns;
    if_thread = (iFS_type)Thread_Push(Continue_if_single,
                                      Tail_call,
                                      iFS_size);
    if_thread->cns = consequent;
    return evaluate_expression(predicate,
                               Main_False); }

static NIL_type initialize_if(NIL_type)
  { Continue_if_double = Thread_Register(continue_if_double,
                                         iFS_size);
    Continue_if_single = Thread_Register(continue_if_single,
                                         iFD_size); }

/*----------------------------------------------------------------------------*/
/*------------------------------------ lambda --------------------------------*/
/*----------------------------------------------------------------------------*/

static EXP_type evaluate_lambda(LMB_type Lambda)
  { EXP_type body;
    NBR_type frame_size,
             param_count;
    UNS_type environment_size;
    VEC_type environment;
    PRC_type procedure;
    environment_size = Environment_Get_Environment_size();
    survive_dynamic(Lambda,
                    environment_size + Main_Default_Margin);
    environment = Environment_Grow_Environment();
    param_count = Lambda->par;
    frame_size  = Lambda->siz;
    body        = Lambda->bod;
    procedure = make_PRC(Main_Anonymous,
                         param_count,
                         frame_size,
                         body,
                         environment);
    return procedure; }

static EXP_type evaluate_lambda_vararg(LMZ_type Lambda)
  { EXP_type body;
    NBR_type frame_size,
             param_count;
    UNS_type environment_size;
    VEC_type environment;
    PRZ_type procedure;
    environment_size = Environment_Get_Environment_size();
    survive_dynamic(Lambda,
                    environment_size + Main_Default_Margin);
    environment = Environment_Grow_Environment();
    param_count = Lambda->par;
    frame_size  = Lambda->siz;
    body        = Lambda->bod;
    procedure = make_PRZ(Main_Anonymous,
                         param_count,
                         frame_size,
                         body,
                         environment);
    return procedure; }

/*----------------------------------------------------------------------------*/
/*------------------------------------ local variable ------------------------*/
/*----------------------------------------------------------------------------*/

static EXP_type evaluate_local_variable(LCL_type Local)
  { NBR_type offset;
    UNS_type raw_offset;
    offset = Local->ofs;
    raw_offset = get_NBR(offset);
    return Environment_Local_Get(raw_offset); }

/*----------------------------------------------------------------------------*/
/*-------------------------------- native call -------------------------------*/
/*----------------------------------------------------------------------------*/

static NBR_type Continue_native_call;

begin_frame(nCL);
 frame_slot(VEC, opd);
 frame_slot(VEC, vec);
 frame_slot(NAT, nat);
 frame_slot(NBR, pos);
end_frame(nCL);

static EXP_type continue_native_call(EXP_type Value,
                                     EXP_type Tail_call) 
  { nCL_type native_call_thread;
    EXP_type expression,
             tail_call,
             value;
    FUN_type function;
    NAT_type native;
    NBR_type position;
    UNS_type index,
             size_vector;
    VEC_type operands,
             vector;
    native_call_thread = (nCL_type)Thread_Peek();
    vector   = native_call_thread->vec;
    position = native_call_thread->pos;
    index = get_NBR(position),
    size_vector = size_VEC(vector);
    vector[index] = Value;
    survive_default(native_call_thread);
    if (index++ < size_vector)
      { native_call_thread = (nCL_type)Thread_Keep();
        operands = native_call_thread->opd;
        expression = operands[index];
        position = make_NBR(index);
        native_call_thread->pos = position;
        tail_call = (index == size_vector)? Tail_call : Main_False;
        return evaluate_expression(expression,
                                   tail_call); }
    native = native_call_thread->nat;
    function = native->fun;
    Thread_Zap();
    value = function(vector,
                     Tail_call);
    Environment_Release_Vector(vector);
    return value; }

/*----------------------------------------------------------------------------*/

static EXP_type evaluate_native_call(NAT_type Native,
                                     VEC_type Operands,
                                     EXP_type Tail_call)
  { nCL_type native_call_thread;
    EXP_type expression,
             tail_call;
    FUN_type function;
    UNS_type operand_size;
    VEC_type vector;
    operand_size = size_VEC(Operands);
    if (operand_size == 0)
      { function = Native->fun;
        Thread_Zap();
        return function(Main_Empty_Vector,
                        Tail_call); }
    survive_dynamic(Operands,
                    operand_size + Main_Default_Margin);
    vector = Environment_Make_Frame(operand_size);
    native_call_thread = (nCL_type)Thread_Poke(Continue_native_call,
                                               Tail_call,
                                               nCL_size);
    native_call_thread->opd = Operands;
    native_call_thread->vec = vector;
    native_call_thread->nat = Native;
    native_call_thread->pos = Main_One;
    expression = Operands[1];
    tail_call = (operand_size == 1)? Tail_call : Main_False;
    return evaluate_expression(expression,
                               tail_call); }

static NIL_type initialize_native_call(NIL_type)
  { Continue_native_call = Thread_Register(continue_native_call,
                                           nCL_size); }

/*----------------------------------------------------------------------------*/
/*------------------------------------ quote ---------------------------------*/
/*----------------------------------------------------------------------------*/

static EXP_type evaluate_quote(QUO_type Quote)
  { EXP_type expression;
    expression = Quote->exp;
    return expression; }        /* clone!!! */

/*----------------------------------------------------------------------------*/
/*---------------------------------- sequence --------------------------------*/
/*----------------------------------------------------------------------------*/

static NBR_type Continue_sequence;

begin_frame(sEQ);
 frame_slot(SEQ, seq);
 frame_slot(NBR, pos);
end_frame(sEQ);

static EXP_type continue_sequence(EXP_type Ignore,
                                  EXP_type Tail_call)
  { sEQ_type sequence_thread;
    EXP_type expression;
    NBR_type position;
    SEQ_type sequence;
    UNS_type index,
             size;
    Main_Claim(Main_Default_Margin);
    sequence_thread = (sEQ_type)Thread_Peek();
    sequence = sequence_thread->seq;
    position = sequence_thread->pos;
    index = get_NBR(position) + 1;
    expression = sequence[index];
    size = size_SEQ(sequence);
    if (index < size)
      { sequence_thread = (sEQ_type)Thread_Keep();
        position = make_NBR(index);
        sequence_thread->pos = position;
        return evaluate_expression(expression,
                                   Main_False); }
    Thread_Zap();
    return evaluate_expression(expression,
                               Tail_call); }

/*----------------------------------------------------------------------------*/

static EXP_type evaluate_sequence(SEQ_type Sequence,
                                  EXP_type Tail_call)
  { sEQ_type sequence_thread;
    EXP_type expression;
    UNS_type size;
    size = size_SEQ(Sequence);
    expression = Sequence[1];
    if (size > 1)
      { sequence_thread = (sEQ_type)Thread_Push(Continue_sequence,
                                                Tail_call,
                                                sEQ_size);
        sequence_thread->seq = Sequence;
        sequence_thread->pos = Main_One;
        return evaluate_expression(expression,
                                   Main_False); }
    survive_default(expression);
    return evaluate_expression(expression,
                               Tail_call); }

static NIL_type initialize_sequence(NIL_type)
  { Continue_sequence = Thread_Register(continue_sequence,
                                        sEQ_size); }

/*----------------------------------------------------------------------------*/
/*------------------------------------ set global ----------------------------*/
/*----------------------------------------------------------------------------*/

static NBR_type Continue_set_global;

begin_frame(sTG);
 frame_slot(NBR, scp);
 frame_slot(NBR, ofs);
end_frame(sTG);

static EXP_type continue_set_global(EXP_type Value,
                                    EXP_type Tail_call)
  { sTG_type set_thread;
    NBR_type offset,
             scope;
    UNS_type raw_offset,
             raw_scope;
    set_thread = (sTG_type)Thread_Peek();
    scope  = set_thread->scp;
    offset = set_thread->ofs;
    Thread_Zap();
    raw_scope = get_NBR(scope);
    raw_offset = get_NBR(offset);
    return Environment_Global_Set(raw_scope,
                                  raw_offset,
                                  Value); }

/*----------------------------------------------------------------------------*/

static EXP_type evaluate_set_global(STG_type Set) 
  { sTG_type set_thread;
    EXP_type expression;
    NBR_type offset,
             scope;
    scope  = Set->scp;
    offset = Set->ofs;
    expression = Set->exp;
    set_thread = (sTG_type)Thread_Push(Continue_set_global,
                                       Main_False,
                                       sTG_size);
    set_thread->scp = scope;
    set_thread->ofs = offset;
    return evaluate_expression(expression,
                               Main_False); }

static NIL_type initialize_set_global(NIL_type)
  { Continue_set_global = Thread_Register(continue_set_global,
                                          sTG_size); }

/*----------------------------------------------------------------------------*/
/*------------------------------------ set local -----------------------------*/
/*----------------------------------------------------------------------------*/

static NBR_type Continue_set_local;

begin_frame(sTL);
 frame_slot(NBR, ofs);
end_frame(sTL);

static EXP_type continue_set_local(EXP_type Value,
                                   EXP_type Tail_call)
  { sTL_type set_thread;
    NBR_type offset;
    UNS_type raw_offset;
    set_thread = (sTL_type)Thread_Peek();
    offset = set_thread->ofs;
    Thread_Zap();
    raw_offset = get_NBR(offset);
    return Environment_Local_Set(raw_offset,
                                 Value); }

/*----------------------------------------------------------------------------*/

static EXP_type evaluate_set_local(STL_type Set)
  { sTL_type set_thread;
    EXP_type expression;
    NBR_type offset;
    offset     = Set->ofs;
    expression = Set->exp;
    set_thread = (sTL_type)Thread_Push(Continue_set_local,
                                       Main_False,
                                       sTL_size);
    set_thread->ofs = offset;
    return evaluate_expression(expression,
                               Main_False); }

static NIL_type initialize_set_local(NIL_type)
  { Continue_set_local = Thread_Register(continue_set_local,
                                         sTL_size); }

/*----------------------------------------------------------------------------*/
/*------------------------------------ value ---------------------------------*/
/*----------------------------------------------------------------------------*/

static EXP_type evaluate_value(EXP_type Value)
  { return Value; }

/*----------------------------------------------------------------------------*/
/*------------------------------- vararg_bindings ----------------------------*/
/*----------------------------------------------------------------------------*/

static NBR_type Continue_vararg,
                Continue_vararg_bindings;

begin_frame(vRG);
 frame_slot(PRZ, prc);
 frame_slot(VEC, frm);
 frame_slot(VEC, opd);
 frame_slot(PAI, lst);
 frame_slot(NBR, pos);
end_frame(vRG);

static EXP_type continue_vararg(EXP_type Value,
                                EXP_type Tail_call) 
  { vRG_type binding_thread;
    EXP_type expression,
             tail_call;
    NBR_type param_count,
             position;
    PAI_type list;
    PRZ_type procedure;
    VEC_type environment,
             frame,
             operands;
    UNS_type index,
             operand_size,
             raw_param_count;
    VEC_type body;
    survive_default(Value);
    binding_thread = (vRG_type)Thread_Peek();
    operands = binding_thread->opd;
    list     = binding_thread->lst;
    position = binding_thread->pos;
    index = get_NBR(position);
    operand_size = size_VEC(operands);
    list = make_PAI(Value,
                    list);
    if (index < operand_size)
      { binding_thread = (vRG_type)Thread_Keep();
        position = make_NBR(++index);
        binding_thread->lst = list;
        binding_thread->pos = position;
        expression = operands[index];
        tail_call = (index == operand_size)? Tail_call : Main_False;
        return evaluate_expression(expression,
                                   tail_call); }
    procedure = binding_thread->prc;
    frame     = binding_thread->frm;
    param_count = procedure->par;
    body        = procedure->bod;
    environment = procedure->env;
    raw_param_count = get_NBR(param_count);
    list = Main_Reverse(list);
    frame[raw_param_count + 1] = list;
    return evaluate_body(body,
                         environment,
                         frame,
                         Tail_call); }

static EXP_type continue_vararg_bindings(EXP_type Value,
                                         EXP_type Tail_call)
  { vRG_type binding_thread;
    EXP_type expression,
             tail_call;
    NBR_type param_count,
             position;
    PRZ_type procedure;
    VEC_type environment,
             frame,
             operands;
    UNS_type index,
             operand_size,
             raw_param_count;
    VEC_type body;
    survive_default(Value);
    binding_thread = (vRG_type)Thread_Peek();
    procedure = binding_thread->prc;
    frame     = binding_thread->frm;
    operands  = binding_thread->opd;
    position  = binding_thread->pos;
    index = get_NBR(position);
    frame[index++] = Value;
    operand_size = size_VEC(operands);
    if (index > operand_size)
      { body        = procedure->bod;
        environment = procedure->env;
        frame[index] = Main_Empty_Pair;
        return evaluate_body(body,
                             environment,
                             frame,
                             Tail_call); }
    param_count = procedure->par;
    raw_param_count = get_NBR(param_count);
    if (index > raw_param_count)
    binding_thread = (vRG_type)Thread_Patch(Continue_vararg);
    position = make_NBR(index);
    binding_thread->lst = Main_Empty_Pair;
    binding_thread->pos = position;
    expression = operands[index];
    tail_call = (index == operand_size)? Tail_call : Main_False;
    return evaluate_expression(expression,
                               tail_call); }

/*----------------------------------------------------------------------------*/

static EXP_type evaluate_vararg_bindings(PRZ_type Procedure,
                                         VEC_type Operands,
                                         EXP_type Tail_call)
  { vRG_type binding_thread;
    NBR_type c_function;
    EXP_type expression,
             tail_call;
    NBR_type frame_size,
             param_count;
    UNS_type operand_size,
             raw_frame_size,
             raw_param_count;
    VEC_type body,
             environment,
             frame,
             operands;
    param_count = Procedure->par;
    raw_param_count = get_NBR(param_count);
    operand_size = size_VEC(Operands);
    if (raw_param_count > operand_size)
      return Main_Error_Procedure_Vararg(TFO_error_string,
                                         Procedure);
    frame_size = Procedure->siz;
    raw_frame_size = get_NBR(frame_size);
    if (operand_size == 0)
      { survive_dynamic(Procedure,
                        raw_frame_size + Main_Default_Margin);
        body        = Procedure->bod;
        environment = Procedure->env;
        frame = Environment_Make_Frame(raw_frame_size);
        frame[1] = Main_Empty_Pair;
        return evaluate_body(body,
                             environment,
                             frame,
                             Tail_call); }
    if (raw_param_count == 0)
      c_function = Continue_vararg;
    else
      c_function = Continue_vararg_bindings;
    binding_thread = (vRG_type)Thread_Poke(c_function,
                                           Tail_call,
                                           vRG_size);
    binding_thread->prc = Procedure;
    binding_thread->opd = Operands;
    binding_thread->lst = Main_Empty_Pair;
    binding_thread->pos = Main_One;
    survive_dynamic(binding_thread,
                    raw_frame_size);
    frame = Environment_Make_Frame(raw_frame_size);
    binding_thread->frm = frame;
    operands = binding_thread->opd;
    expression = operands[1];
    tail_call = (operand_size == 1)? Tail_call : Main_False;
    return evaluate_expression(expression,
                               tail_call); }

static NIL_type initialize_vararg_bindings(NIL_type)
  { Continue_vararg = Thread_Register(continue_vararg,
                                      vRG_size);
    Continue_vararg_bindings = Thread_Register(continue_vararg_bindings,
                                               vRG_size); }

/*----------------------------------------------------------------------------*/
/*------------------------------------ thunk ---------------------------------*/
/*----------------------------------------------------------------------------*/

static EXP_type evaluate_thunk(THK_type Thunk,
                               EXP_type Tail_call)
  { EXP_type expression;
    NBR_type frame_size;
    UNS_type raw_frame_size;
    frame_size = Thunk->siz;
    expression = Thunk->exp;
    raw_frame_size = get_NBR(frame_size);
    return evaluate_inline(expression,
                           raw_frame_size,
                           Tail_call); }
                                
/*----------------------------------------------------------------------------*/
/*------------------------------------ while ---------------------------------*/
/*----------------------------------------------------------------------------*/

static NBR_type Continue_while_body,
                Continue_while_predicate;

begin_frame(wHI);
 frame_slot(EXP, prd);
 frame_slot(VEC, bod);
 frame_slot(EXP, res);
end_frame(wHI);

static EXP_type continue_while_predicate(EXP_type Boolean,
                                         EXP_type Tail_call)
  { wHI_type while_thread;
    EXP_type body,
             result;
    while_thread = (wHI_type)Thread_Peek();
    if (is_FLS(Boolean))
      { result = while_thread->res;
        Thread_Zap();
        return result; }
    Main_Claim(Main_Default_Margin);
    while_thread = (wHI_type)Thread_Patch(Continue_while_body);
    body = while_thread->bod;
    return evaluate_expression(body,
                               Main_False); }

static EXP_type continue_while_body(EXP_type Value,
                                    EXP_type Tail_call) 
  { wHI_type while_thread;
    EXP_type predicate;
    survive_default(Value);
    while_thread = (wHI_type)Thread_Peek();
    predicate = while_thread->prd;
    while_thread = (wHI_type)Thread_Patch(Continue_while_predicate);
    while_thread->res = Value;
    return evaluate_expression(predicate,
                               Main_False); }

/*----------------------------------------------------------------------------*/

static EXP_type evaluate_while(WHI_type While)
  { wHI_type while_thread;
    EXP_type body,
             predicate;
    predicate = While->prd;
    body      = While->bod;
    while_thread = (wHI_type)Thread_Push(Continue_while_predicate, 
                                         Main_False,
                                         wHI_size);
    while_thread->prd = predicate;
    while_thread->bod = body;
    while_thread->res = Main_Unspecified;
    return evaluate_expression(predicate,
                               Main_False); }

static NIL_type initialize_while(NIL_type)
  { Continue_while_predicate = Thread_Register(continue_while_predicate,
                                               wHI_size);
    Continue_while_body = Thread_Register(continue_while_body,
                                          wHI_size); }

/*----------------------------------------------------------------------------*/
/*------------------------------------ expression ----------------------------*/
/*----------------------------------------------------------------------------*/

static EXP_type evaluate_expression(EXP_type Expression,
                                    EXP_type Tail_call) 
  { TAG_type tag;
    survive_default(Expression);
    tag = Tag_of(Expression);
    switch (tag)
      { case APL_tag:
          return            evaluate_application(Expression,
                                                 Tail_call);
        case BEG_tag:
          return                  evaluate_begin(Expression,
                                                 Tail_call);
        case DFF_tag:
          return        evaluate_define_function(Expression);
        case DFV_tag:
          return        evaluate_define_variable(Expression,
                                                 Tail_call);
        case DFZ_tag:
          return evaluate_define_function_vararg(Expression);
        case GLB_tag:
          return        evaluate_global_variable(Expression);
        case IFF_tag:
          return              evaluate_if_double(Expression,
                                                 Tail_call);
        case IFZ_tag:
          return              evaluate_if_single(Expression,
                                                 Tail_call);
        case LCL_tag:
          return         evaluate_local_variable(Expression);
        case LMB_tag:
          return                 evaluate_lambda(Expression);
        case LMZ_tag:
          return          evaluate_lambda_vararg(Expression);
        case QUO_tag:
          return                  evaluate_quote(Expression);
        case SEQ_tag:
          return               evaluate_sequence(Expression,
                                                 Tail_call);
        case STG_tag:
          return             evaluate_set_global(Expression);
        case STL_tag:
          return              evaluate_set_local(Expression);
        case THK_tag:
          return                  evaluate_thunk(Expression,
                                                 Tail_call);
        case WHI_tag:
          return                  evaluate_while(Expression);
        case CHA_tag:
        case CNT_tag:
        case FLS_tag:
        case NAT_tag:
        case NBR_tag:
        case NUL_tag:
        case PAI_tag:
        case PRC_tag:
        case PRZ_tag:
        case REA_tag:
        case STR_tag:
        case TRU_tag:
        case USP_tag:
        case VEC_tag:
            return                evaluate_value(Expression);
        case ENV_tag:
        case FRM_tag:
        case SYM_tag:
        case THR_tag:
        case THr_tag:
          return Main_Error_Tag(ILT_error_string,
                                tag); }
    return Main_Error_Tag(IXT_error_string,
                          tag); }

/*------------------------------ public functions ----------------------------*/

EXP_type Evaluate_Apply(VEC_type Vector,
                        EXP_type Tail_call)
  { return evaluate_direct_application(Vector,
                                       Tail_call); }

EXP_type Evaluate_Continue(EXP_type Value)
  { return continue_with(Value); }

EXP_type Evaluate_Evaluate(EXP_type Expression)
  { return evaluate_expression(Expression,
                               Main_False); }

NIL_type Evaluate_Initialize(NIL_type)
  { initialize_application();
    initialize_bindings();
    initialize_body();
    initialize_continuation_call();
    initialize_define();
    initialize_if();
    initialize_native_call();
    initialize_sequence();
    initialize_set_global();
    initialize_set_local();
    initialize_vararg_bindings();
    initialize_while(); }
