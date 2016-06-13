                     /*-----------------------------------*/
                     /*             >>>Slip<<<            */
                     /*            Theo D'Hondt           */
                     /*     VUB Software Languages Lab    */
                     /*             (c) 2010              */
                     /*-----------------------------------*/
                     /*     version 12: smart caching     */
                     /*-----------------------------------*/
                     /*               grammar             */
                     /*-----------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SlipGrammar.h"

/*------------------------------ private macros ------------------------------*/

#define chunk_size(TYP)                                                        \
  (sizeof(TYP) / sizeof(CEL_type) - 1)

#define make_chunk_with_offset(TYP, SIZ)                                       \
  (TYP##_type)Memory_Make_Chunk(TYP##_tag, TYP##_size + SIZ)

#define make_chunk(TYP)                                                        \
  make_chunk_with_offset(TYP, 0)

/*------------------------------ private constants ---------------------------*/

enum { Immediate_null_value        = Memory_Void_Value + 0*Memory_Cell_Bias,
       Immediate_true_value        = Memory_Void_Value + 1*Memory_Cell_Bias,
       Immediate_false_value       = Memory_Void_Value + 2*Memory_Cell_Bias,
       Immediate_unspecified_value = Memory_Void_Value + 3*Memory_Cell_Bias,
       Immediate_boundary          = Memory_Void_Value + 3*Memory_Cell_Bias };

/*------------------------------ private functions ---------------------------*/

static BYT_type member_of(EXP_type Expression,
                          TAG_type Tag)
  { BYT_type tag;
    tag = Tag_of(Expression);
    return (tag == Tag); }

static UNS_type string_to_expression_size(TXT_type Raw_string)
  { return strlen(Raw_string) / sizeof(CEL_type) + 1; }

/*-------------------------------- public functions --------------------------*/

TAG_type Tag_of(EXP_type Expression)
  { UNS_type immediate;
    if (Memory_Is_Immediate(Expression))
      return NBR_tag;
    immediate = (UNS_type)Expression;
    if (immediate <= Immediate_boundary)
      switch (immediate)
        { case Immediate_null_value:
            return NUL_tag;
          case Immediate_true_value:
            return TRU_tag;
          case Immediate_false_value:
            return FLS_tag;
          case Immediate_unspecified_value:
            return USP_tag; }
    return (TAG_type)Memory_Get_Tag((PTR_type)Expression); }

EXP_type Clone_of(EXP_type Expression)
  { UNS_type immediate;
    if (Memory_Is_Immediate(Expression))
      return Expression;
    immediate = (UNS_type)Expression;
    if (immediate <= Immediate_boundary)
      return Expression;
    return (EXP_type)Memory_Copy((PTR_type)Expression); }

/*------------------------------ APL declarations ----------------------------*/

static const UNS_type APL_size = chunk_size(APL);

BYT_type is_APL(EXP_type Expression)
  { BYT_type tag = Tag_of(Expression);
    return (tag == APL_tag); }

APL_type make_APL(EXP_type Expression,
                  VEC_type Operands)
  { APL_type application = make_chunk(APL);
    application->exp = Expression;
    application->opr = Operands;
    return application; }

/*------------------------------ BEG declarations ----------------------------*/

static const UNS_type BEG_size = chunk_size(BEG);

BYT_type is_BEG(EXP_type Expression)
  { BYT_type tag = Tag_of(Expression);
    return (tag == BEG_tag); }

BEG_type make_BEG(EXP_type Sequence)
  { BEG_type begin = make_chunk(BEG);
    begin->seq = Sequence;
    return begin; }

/*------------------------------ CHA declarations ----------------------------*/

static const UNS_type CHA_size = chunk_size(CHA);

BYT_type is_CHA(EXP_type Expression)
  { return member_of(Expression,
                     CHA_tag); }

CHA_type make_CHA(CHR_type Raw_character)
  { CHA_type character = make_chunk(CHA);
    character->chr = Raw_character;
    return character; }

/*------------------------------ CNT declarations ----------------------------*/

static const UNS_type CNT_size = chunk_size(CNT);

BYT_type is_CNT(EXP_type Expression)
  { BYT_type tag = Tag_of(Expression);
    return (tag == CNT_tag); }

CNT_type make_CNT(VEC_type Environment,
                  VEC_type Frame,
                  THR_type Thread)
  { CNT_type thread = make_chunk(CNT);
    thread->env = Environment;
    thread->frm = Frame;
    thread->thr = Thread;
    return thread; }

/*------------------------------ DFF declarations ----------------------------*/

static const UNS_type DFF_size = chunk_size(DFF);

BYT_type is_DFF(EXP_type Expression)
  { BYT_type tag = Tag_of(Expression);
    return (tag == DFF_tag); }

DFF_type make_DFF(SYM_type Name,
                  NBR_type Offset,
                  NBR_type Param_count,
                  NBR_type Frame_size,
                  EXP_type Body)
  { DFF_type definition = make_chunk(DFF);
    definition->nam = Name;
    definition->ofs = Offset;
    definition->par = Param_count;
    definition->siz = Frame_size;
    definition->bod = Body;
    return definition; }

/*------------------------------ DFV declarations ----------------------------*/

static const UNS_type DFV_size = chunk_size(DFV);

BYT_type is_DFV(EXP_type Expression)
  { BYT_type tag = Tag_of(Expression);
    return (tag == DFV_tag); }

DFV_type make_DFV(NBR_type Offset,
                  EXP_type Expression)
  { DFV_type definition = make_chunk(DFV);
    definition->ofs = Offset;
    definition->exp = Expression;
    return definition; }

/*------------------------------ DFZ declarations ----------------------------*/

static const UNS_type DFZ_size = chunk_size(DFZ);

BYT_type is_DFZ(EXP_type Expression)
  { BYT_type tag = Tag_of(Expression);
    return (tag == DFZ_tag); }

DFZ_type make_DFZ(SYM_type Name,
                  NBR_type Offset,
                  NBR_type Param_count,
                  NBR_type Frame_size,
                  EXP_type Body)
  { DFZ_type definition = make_chunk(DFZ);
    definition->nam = Name;
    definition->ofs = Offset;
    definition->par = Param_count;
    definition->siz = Frame_size;
    definition->bod = Body;
    return definition; }

/*------------------------------ ENV declarations ----------------------------*/

static const UNS_type ENV_size = chunk_size(ENV);

BYT_type is_ENV(EXP_type Expression)
  { return member_of(Expression,
                     ENV_tag); }

ENV_type make_ENV(FRM_type Frame,
                  NBR_type Size,
                  ENV_type Environment)
  { ENV_type environment = make_chunk(ENV);
    environment->frm = Frame;
    environment->siz = Size;
    environment->env = Environment;
    return environment; }

/*------------------------------ FLS declarations ----------------------------*/

BYT_type is_FLS(EXP_type Expression)
  { return (Expression == (EXP_type)Immediate_false_value); }

FLS_type make_FLS(NIL_type)
  { return (FLS_type)Immediate_false_value; }

/*------------------------------ FRM declarations ----------------------------*/

const static UNS_type FRM_size = chunk_size(FRM);

BYT_type is_FRM(EXP_type Expression)
  { return member_of(Expression,
                     FRM_tag); }

FRM_type make_FRM(SYM_type Variable,
                  FRM_type Frame)
  { FRM_type frame = make_chunk(FRM);
    frame->var = Variable;
    frame->frm = Frame;
    return frame; }

/*------------------------------ GLB declarations ----------------------------*/

const static UNS_type GLB_size = chunk_size(GLB);

BYT_type is_GLB(EXP_type Expression)
  { return member_of(Expression,
                     GLB_tag); }

GLB_type make_GLB(NBR_type Scope,
                  NBR_type Offset)
  { GLB_type global = make_chunk(GLB);
    global->scp = Scope;
    global->ofs = Offset;
    return global; }

/*------------------------------ IFF declarations ----------------------------*/

static const UNS_type IFF_size = chunk_size(IFF);

BYT_type is_IFF(EXP_type Expression)
  { BYT_type tag = Tag_of(Expression);
    return (tag == IFF_tag); }

IFF_type make_IFF(EXP_type Predicate,
                  EXP_type Consequent,
                  EXP_type Alternative)
  { IFF_type if_ = make_chunk(IFF);
    if_->prd = Predicate;
    if_->cns = Consequent;
    if_->alt = Alternative;
    return if_; }

/*------------------------------ IFZ declarations ----------------------------*/

static const UNS_type IFZ_size = chunk_size(IFZ);

BYT_type is_IFZ(EXP_type Expression)
  { BYT_type tag = Tag_of(Expression);
    return (tag == IFZ_tag); }

IFZ_type make_IFZ(EXP_type Predicate,
                  EXP_type Consequent)
  { IFZ_type if_ = make_chunk(IFZ);
    if_->prd = Predicate;
    if_->cns = Consequent;
    return if_; }

/*------------------------------ LCL declarations ----------------------------*/

const static UNS_type LCL_size = chunk_size(LCL);

BYT_type is_LCL(EXP_type Expression)
  { return member_of(Expression,
                     LCL_tag); }

LCL_type make_LCL(NBR_type Offset)
  { LCL_type local = make_chunk(LCL);
    local->ofs = Offset;
    return local; }

/*------------------------------ LMB declarations ----------------------------*/

static const UNS_type LMB_size = chunk_size(LMB);

BYT_type is_LMB(EXP_type Expression)
  { BYT_type tag = Tag_of(Expression);
    return (tag == LMB_tag); }

LMB_type make_LMB(NBR_type Param_count,
                  NBR_type Frame_size,
                  EXP_type Body)
  { LMB_type lambda = make_chunk(LMB);
    lambda->par = Param_count;
    lambda->siz = Frame_size;
    lambda->bod = Body;
    return lambda; }

/*------------------------------ LMZ declarations ----------------------------*/

static const UNS_type LMZ_size = chunk_size(LMZ);

BYT_type is_LMZ(EXP_type Expression)
  { BYT_type tag = Tag_of(Expression);
    return (tag == LMZ_tag); }

LMZ_type make_LMZ(NBR_type Param_count,
                  NBR_type Frame_size,
                  EXP_type Body)
  { LMZ_type lambda = make_chunk(LMZ);
    lambda->par = Param_count;
    lambda->siz = Frame_size;
    lambda->bod = Body;
    return lambda; }

/*------------------------------ NAT declarations ----------------------------*/

static const UNS_type NAT_size = chunk_size(NAT);

BYT_type is_NAT(EXP_type Expression)
  { return member_of(Expression,
                     NAT_tag); }

NAT_type make_NAT(FUN_type Raw_function,
                  TXT_type Name)
  { NAT_type native;
    UNS_type size;
    size = string_to_expression_size(Name);
    native = make_chunk_with_offset(NAT,
                                    size);
    native->fun = Raw_function;
    strcpy(native->nam,
           Name);
    return native; }

/*------------------------------ NBR declarations ----------------------------*/

BYT_type is_NBR(EXP_type Expression)
  { return Memory_Is_Immediate(Expression); }

NBR_type make_NBR(LNG_type Raw_number)
  { NBR_type number;
    number = Memory_Make_Immediate(Raw_number);
    return number; }

LNG_type get_NBR(NBR_type Number)
  { LNG_type raw_number;
    raw_number = Memory_Get_Immediate(Number);
    return raw_number; }

/*------------------------------ NUL declarations ----------------------------*/

BYT_type is_NUL(EXP_type Expression)
  { return (Expression == Immediate_null_value); }

NUL_type make_NUL(NIL_type)
  { return Immediate_null_value; }

/*------------------------------ PAI declarations ----------------------------*/

static const UNS_type PAI_size = chunk_size(PAI);

BYT_type is_PAI(EXP_type Expression)
  { return member_of(Expression,
                     PAI_tag); }

PAI_type make_PAI(EXP_type Car,
                  EXP_type Cdr)
  { PAI_type pair = make_chunk(PAI);
    pair->car = Car;
    pair->cdr = Cdr;
    return pair; }

/*------------------------------ PRC declarations ----------------------------*/

static const UNS_type PRC_size = chunk_size(PRC);

BYT_type is_PRC(EXP_type Expression)
  { return member_of(Expression,
                     PRC_tag); }

PRC_type make_PRC(SYM_type Name,
                  NBR_type Param_count,
                  NBR_type Frame_size,
                  EXP_type Body,
                  VEC_type Environment)
  { PRC_type procedure = make_chunk(PRC);
    procedure->nam = Name;
    procedure->par = Param_count;
    procedure->siz = Frame_size;
    procedure->bod = Body;
    procedure->env = Environment;
    return procedure; }

/*------------------------------ PRZ declarations ----------------------------*/

static const UNS_type PRZ_size = chunk_size(PRZ);

BYT_type is_PRZ(EXP_type Expression)
  { return member_of(Expression,
                     PRZ_tag); }

PRZ_type make_PRZ(SYM_type Name,
                  NBR_type Param_count,
                  NBR_type Frame_size,
                  EXP_type Body,
                  VEC_type Environment)
  { PRZ_type procedure = make_chunk(PRZ);
    procedure->nam = Name;
    procedure->par = Param_count;
    procedure->siz = Frame_size;
    procedure->bod = Body;
    procedure->env = Environment;
    return procedure; }

/*------------------------------ QUO declarations ----------------------------*/

static const UNS_type QUO_size = chunk_size(QUO);

BYT_type is_QUO(EXP_type Expression)
  { BYT_type tag = Tag_of(Expression);
    return (tag == QUO_tag); }

QUO_type make_QUO(EXP_type Expression)
  { QUO_type quote = make_chunk(QUO);
    quote->exp = Expression;
    return quote; }

/*------------------------------ REA declarations ----------------------------*/

static const UNS_type REA_size = chunk_size(REA);

BYT_type is_REA(EXP_type Expression)
  { return member_of(Expression,
                     REA_tag); }

REA_type make_REA(FLO_type Raw_real)
  { REA_type real = make_chunk(REA);
    real->flo = Raw_real;
    return real; }

/*------------------------------ SEQ declarations ----------------------------*/

static const UNS_type SEQ_size = 0;

BYT_type is_SEQ(EXP_type Expression)
  { return member_of(Expression,
                     SEQ_tag); }

SEQ_type make_SEQ(UNS_type Size)
  { SEQ_type sequence = (SEQ_type)make_chunk_with_offset(SEQ,
                                                         Size);
    return sequence; }

UNS_type size_SEQ(SEQ_type Sequence)
  { return (UNS_type) Memory_Get_Size((PTR_type)Sequence); }
  
/*------------------------------ STG declarations ----------------------------*/

static const UNS_type STG_size = chunk_size(STG);

BYT_type is_STG(EXP_type Expression)
  { return member_of(Expression,
                     STG_tag); }

STG_type make_STG(NBR_type Scope,
                  NBR_type Offset,
                  EXP_type Expression)
  { STG_type set = make_chunk(STG);
    set->scp = Scope;
    set->ofs = Offset;
    set->exp = Expression;
    return set; }

/*------------------------------ STL declarations ----------------------------*/

static const UNS_type STL_size = chunk_size(STL);

BYT_type is_STL(EXP_type Expression)
  { return member_of(Expression,
                     STL_tag); }

STL_type make_STL(NBR_type Offset,
                  EXP_type Expression)
  { STL_type set = make_chunk(STL);
    set->ofs = Offset;
    set->exp = Expression;
    return set; }

/*------------------------------ STR declarations ----------------------------*/

static const UNS_type STR_size = chunk_size(STR);

BYT_type is_STR(EXP_type Expression)
  { return member_of(Expression,
                     STR_tag); }

STR_type make_STR(TXT_type Raw_string)
  { STR_type string;
    UNS_type size;
    size = string_to_expression_size(Raw_string);
    string = make_chunk_with_offset(STR,
                                    size);
    strcpy(string->txt,
           Raw_string);
    return string; }

UNS_type size_STR(TXT_type Raw_string)
  { return (UNS_type)string_to_expression_size(Raw_string); }

/*------------------------------ SYM declarations ----------------------------*/

static const UNS_type SYM_size = chunk_size(SYM);

BYT_type is_SYM(EXP_type Expression)
  { return member_of(Expression,
                     SYM_tag); }

SYM_type make_SYM(TXT_type Raw_string)
  { SYM_type symbol;
    UNS_type size;
    size = string_to_expression_size(Raw_string);
    symbol = make_chunk_with_offset(SYM,
                                    size);
    strcpy(symbol->txt,
           Raw_string);
    return symbol; }

UNS_type size_SYM(TXT_type Raw_string)
  { return (UNS_type)string_to_expression_size(Raw_string); }

/*------------------------------ THK declarations ----------------------------*/

static const UNS_type THK_size = chunk_size(THK);

BYT_type is_THK(EXP_type Expression)
  { return member_of(Expression,
                     THK_tag); }

THK_type make_THK(EXP_type Expression,
                  NBR_type Frame_size)
  { THK_type thunk = make_chunk(THK);
    thunk->exp = Expression;
    thunk->siz = Frame_size;
    return thunk; }

/*------------------------------ THR declarations ----------------------------*/

static const UNS_type THR_size = chunk_size(THR);

BYT_type is_THR(EXP_type Expression)
  { return member_of(Expression,
                     THR_tag); }

BYT_type marked_THR(THR_type Thread)
  { return member_of(Thread,
                     THr_tag); }

THR_type make_THR(NBR_type Thread_id,
                  THR_type Thread,
                  EXP_type Call_status,
                  UNS_type Size)
  { THR_type thread;
    thread = make_chunk_with_offset(THR,
                                    Size);
    thread->tid = Thread_id;
    thread->thr = Thread;
    thread->tcs = Call_status;
    return thread; }

UNS_type size_THR(THR_type Thread)
  { UNS_type size;
    size = Memory_Get_Size((PTR_type)Thread) + 1;
    return size - sizeof(THR)/sizeof(CEL_type); }

NIL_type mark_THR(THR_type Thread)
  { Memory_Set_Tag((PTR_type)Thread,
                   THr_tag); }
                                      
/*------------------------------ TRU declarations ----------------------------*/

BYT_type is_TRU(EXP_type Expression)
  { return (Expression == (EXP_type)Immediate_true_value); }

TRU_type make_TRU(NIL_type)
  { return (TRU_type)Immediate_true_value; }

/*------------------------------ USP declarations ----------------------------*/

BYT_type is_USP(EXP_type Expression)
  { return (Expression == (EXP_type)Immediate_unspecified_value); }

USP_type make_USP(NIL_type)
  { return (USP_type)Immediate_unspecified_value; }

/*------------------------------ VEC declarations ----------------------------*/

static const UNS_type VEC_size = 0;

BYT_type is_VEC(EXP_type Expression)
  { return member_of(Expression,
                     VEC_tag); }

BYT_type marked_VEC(VEC_type Vector)
  { return member_of(Vector,
                     VEc_tag); }

VEC_type make_VEC(UNS_type Size)
  { VEC_type vector = (VEC_type)make_chunk_with_offset(VEC,
                                                       Size);
    return vector; }

VEC_type init_VEC(UNS_type Size)
  { UNS_type index;
    VEC_type vector = (VEC_type)make_chunk_with_offset(VEC,
                                                       Size);
    for (index = 1;
         index <= Size;
         index++)
      vector[index] = Memory_Void_Value;
    return vector; }

UNS_type size_VEC(VEC_type Vector)
  { return (UNS_type) Memory_Get_Size((PTR_type)Vector); }

NIL_type mark_VEC(VEC_type Vector)
  { Memory_Set_Tag((PTR_type)Vector,
                   VEc_tag); }

/*------------------------------ WHI declarations ----------------------------*/

static const UNS_type WHI_size = chunk_size(WHI);

BYT_type is_WHI(EXP_type Expression)
  { BYT_type tag = Tag_of(Expression);
    return (tag == WHI_tag); }

WHI_type make_WHI(EXP_type Predicate,
                  EXP_type Body)
  { WHI_type while_ = make_chunk(WHI);
    while_->prd = Predicate;
    while_->bod = Body;
    return while_; }
