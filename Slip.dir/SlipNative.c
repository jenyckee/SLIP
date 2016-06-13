                     /*-----------------------------------*/
                     /*             >>>Slip<<<            */
                     /*            Theo D'Hondt           */
                     /*     VUB Software Languages Lab    */
                     /*             (c) 2010              */
                     /*-----------------------------------*/
                     /*     version 12: smart caching     */
                     /*-----------------------------------*/
                     /*              natives              */
                     /*-----------------------------------*/

#include <math.h>
#include <stdio.h>
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

enum { Natives_table_size = 42 };

static const TXT_type AL1_error_string = "at least one argument required";
static const TXT_type AL2_error_string = "at least two arguments required";
static const TXT_type AM1_error_string = "at most one argument required";
static const TXT_type AM2_error_string = "at most two arguments required";
static const TXT_type ARN_error_string = "arguments restricted to numbers";
static const TXT_type EX0_error_string = "no arguments allowed";
static const TXT_type EX1_error_string = "exactly one argument required";
static const TXT_type EX2_error_string = "exactly two arguments required";
static const TXT_type EX3_error_string = "exactly three arguments required";
static const TXT_type IOR_error_string = "index out of range";
static const TXT_type NAL_error_string = "not a list";
static const TXT_type NAN_error_string = "not a number";
static const TXT_type NAP_error_string = "not a pair";
static const TXT_type NAS_error_string = "not a string";
static const TXT_type NAV_error_string = "not a vector";
static const TXT_type NNN_error_string = "non-negative number required";
static const TXT_type NTO_error_string = "native table overflow";

/*------------------------------ private variables ---------------------------*/

static TXT_type Natives_table [Natives_table_size];

/*------------------------------ private functions ---------------------------*/

static NIL_type enter_native_name(TXT_type String,
                                  UNS_type Offset)
  { if (Offset >= Natives_table_size)
      Main_Fatal_Error(NTO_error_string);
    Natives_table[Offset - 1] = String; }

static TXT_type get_native_name(UNS_type Offset)
  { if (Offset >= Natives_table_size)
      return 0;
    return Natives_table[Offset - 1]; }

static NIL_type native_define(TXT_type String,
                              FUN_type Function)
  { EXP_type native;
    SYM_type variable;
    UNS_type raw_offset;
    variable = Pool_Initially_Enter(String);
    native = make_NAT(Function,
                      String);
    raw_offset = Dictionary_Define(variable);
    enter_native_name(String,
                      raw_offset);
    Environment_Local_Set(raw_offset,
                          native); }

/*----------------------------------------------------------------------------*/
/*------------------------------ variables -----------------------------------*/
/*----------------------------------------------------------------------------*/

/*------------------------------ meta-circularity-level ----------------------*/

static const TXT_type mcl_string = "circularity-level";

static NIL_type initialize_circularity_level(NIL_type)
  { SYM_type variable;
    UNS_type raw_offset;
    variable = Pool_Initially_Enter(mcl_string);
    raw_offset = Dictionary_Define(variable);
    enter_native_name(mcl_string,
                      raw_offset);
    Environment_Local_Set(raw_offset,
                          Main_Zero); }

/*----------------------------------------------------------------------------*/
/*------------------------------ arithmetic ----------------------------------*/
/*----------------------------------------------------------------------------*/

/*------------------------------ + -------------------------------------------*/

static const TXT_type add_string = "+";

static EXP_type add_floats(FLO_type Raw_real,
                           VEC_type Vector,
                           UNS_type Index)
  { EXP_type value;
    FLO_type raw_real,
             raw_real_sum;
    LNG_type raw_number;
    REA_type real;
    TAG_type tag;
    UNS_type index,
             size;
    raw_real_sum = Raw_real;
    size = size_VEC(Vector);
    for (index = Index;
         index <= size;
         index++)
      { value = Vector[index];
        tag = Tag_of(value);
        switch (tag)
          { case NBR_tag:
              raw_number = get_NBR(value);
              raw_real_sum += raw_number;
              break;
            case REA_tag:
              real = value;
              raw_real = real->flo;
              raw_real_sum += raw_real;
              break;
            default:
              return Main_Error_Text(ARN_error_string,
                                     add_string); }}
    return make_REA(raw_real_sum); }

static EXP_type add_integers(NBR_type Number,
                             VEC_type Vector)
  { EXP_type value;
    FLO_type raw_real,
             raw_real_sum;
    LNG_type raw_number,
             raw_number_sum;
    REA_type real;
    TAG_type tag;
    UNS_type index,
             size;
    raw_number_sum = get_NBR(Number);
    size = size_VEC(Vector);
    for (index = 2;
         index <= size;
         index++)
      { value = Vector[index];
        tag = Tag_of(value);
        switch (tag)
          { case NBR_tag:
              raw_number = get_NBR(value);
              raw_number_sum += raw_number;
              break;
            case REA_tag:
              real = value;
              raw_real = real->flo;
              raw_real_sum = raw_number_sum + raw_real;
              return add_floats(raw_real_sum,
                                Vector,
                                index + 1);
            default:
              return Main_Error_Text(ARN_error_string,
                                     add_string); }}
    return make_NBR(raw_number_sum); }

static EXP_type add_native(VEC_type Vector,
                           EXP_type Tail_call)
  { EXP_type value;
    FLO_type raw_real;
    REA_type real;
    TAG_type tag;
    UNS_type size;
    size = size_VEC(Vector);
    if (size == 0)
      return Main_Zero;
    value = Vector[1];
    if (size == 1)
      return value;
    tag = Tag_of(value);
    switch (tag)
      { case NBR_tag:
          return add_integers(value,
                              Vector);
        case REA_tag:
          real = value;
          raw_real = real->flo;
          return add_floats(raw_real,
                            Vector,
                            2); }
    return Main_Error_Text(ARN_error_string,
                           add_string); }
                                 
static NIL_type initialize_add_native(NIL_type)
  { native_define(add_string,
                  add_native); }

/*------------------------------ - -------------------------------------------*/

static const TXT_type sub_string = "-";

static EXP_type subtract_floats(FLO_type Raw_real,
                                VEC_type Vector,
                                UNS_type Index)
  { EXP_type value;
    FLO_type raw_real,
             raw_real_sum;
    LNG_type raw_number;
    REA_type real;
    TAG_type tag;
    UNS_type index,
             size;
    raw_real_sum = Raw_real;
    size = size_VEC(Vector);
    for (index = Index;
         index <= size;
         index++)
      { value = Vector[index];
        tag = Tag_of(value);
        switch (tag)
          { case NBR_tag:
              raw_number = get_NBR(value);
              raw_real_sum -= raw_number;
              break;
            case REA_tag:
              real = value;
              raw_real = real->flo;
              raw_real_sum -= raw_real;
              break;
            default:
              return Main_Error_Text(ARN_error_string,
                                     sub_string); }}
    return make_REA(raw_real_sum); }

static EXP_type subtract_integers(NBR_type Number,
                                  VEC_type Vector)
  { EXP_type value;
    FLO_type raw_real,
             raw_real_sum;
    LNG_type raw_number,
             raw_number_sum;
    REA_type real;
    TAG_type tag;
    UNS_type index,
             size;
    raw_number_sum = get_NBR(Number);
    size = size_VEC(Vector);
    for (index = 2;
         index <= size;
         index++)
      { value = Vector[index];
        tag = Tag_of(value);
        switch (tag)
          { case NBR_tag:
              raw_number = get_NBR(value);
              raw_number_sum -= raw_number;
              break;
            case REA_tag:
              real = value;
              raw_real = real->flo;
              raw_real_sum = raw_number_sum - raw_real;
              return subtract_floats(raw_real_sum,
                                     Vector,
                                     index + 1);
            default:
              return Main_Error_Text(ARN_error_string,
                                     sub_string); }}
    return make_NBR(raw_number_sum); }

static EXP_type subtract_native(VEC_type Vector,
                                EXP_type Tail_call)
  { EXP_type value;
    FLO_type raw_real;
    LNG_type raw_number;
    REA_type real;
    TAG_type tag;
    UNS_type size;
    size = size_VEC(Vector);
    if (size == 0)
      return Main_Error_Text(AL1_error_string,
                             sub_string);
    value = Vector[1];
    tag = Tag_of(value);
    if (size == 1)
      { switch (tag)
          { case NBR_tag:
              raw_number  = get_NBR(value);
              return make_NBR(- raw_number);
            case REA_tag:
              real = value;
              raw_real = real->flo;
              return make_REA(- raw_real); }
        return Main_Error_Text(ARN_error_string,
                               sub_string); }
    switch (tag)
      { case NBR_tag:
          return subtract_integers(value,
                                   Vector);
        case REA_tag:
          real = value;
          raw_real = real->flo;
          return subtract_floats(raw_real,
                                 Vector,
                                 2); }
    return Main_Error_Text(ARN_error_string,
                           sub_string); }                                 

static NIL_type initialize_subtract_native(NIL_type)
  { native_define(sub_string,
                  subtract_native); }

/*------------------------------ * -------------------------------------------*/

static const TXT_type mul_string = "*";

static EXP_type multiply_floats(FLO_type Raw_real,
                                VEC_type Vector,
                                UNS_type Index)
  { EXP_type value;
    FLO_type raw_real,
             raw_real_product;
    LNG_type raw_number;
    REA_type real;
    TAG_type tag;
    UNS_type index,
             size;
    raw_real_product = Raw_real;
    size = size_VEC(Vector);
    for (index = Index;
         index <= size;
         index++)
      { value = Vector[index];
        tag   = Tag_of(value);
        switch (tag)
          { case NBR_tag:
              raw_number = get_NBR(value);
              raw_real_product *= raw_number;
              break;
            case REA_tag:
              real = value;
              raw_real = real->flo;
              raw_real_product *= raw_real;
              break;
            default:
              return Main_Error_Text(ARN_error_string,
                                     mul_string); }}
    return make_REA(raw_real_product); }

static EXP_type multiply_integers(NBR_type Number,
                                  VEC_type Vector)
  { EXP_type value;
    FLO_type raw_real,
             raw_real_product;
    LNG_type raw_number,
             raw_number_product;
    REA_type real;
    TAG_type tag;
    UNS_type index,
             size;
    raw_number_product = get_NBR(Number);
    size = size_VEC(Vector);
    for (index = 2;
         index <= size;
         index++)
      { value  = Vector[index];
        tag = Tag_of(value);
        switch (tag)
          { case NBR_tag:
              raw_number = get_NBR(value);
              raw_number_product *= raw_number;
              break;
            case REA_tag:
              real = value;
              raw_real = real->flo;
              raw_real_product = raw_number_product * raw_real;
              return multiply_floats(raw_real_product,
                                     Vector,
                                     index + 1);
            default:
              return Main_Error_Text(ARN_error_string,
                                     mul_string); }}
    return make_NBR(raw_number_product); }

static EXP_type multiply_native(VEC_type Vector,
                                EXP_type Tail_call)
  { EXP_type value;
    FLO_type raw_real;
    REA_type real;
    TAG_type tag;
    UNS_type size;
    size = size_VEC(Vector);
    if (size == 0)
      return Main_One;
    value = Vector[1];
    if (size == 1)
      return value;
    tag = Tag_of(value);
    switch (tag)
      { case NBR_tag:
          return multiply_integers(value,
                                   Vector);
        case REA_tag:
          real = value;
          raw_real = real->flo;
          return multiply_floats(raw_real,
                                 Vector,
                                 2); }
    return Main_Error_Text(ARN_error_string,
                           mul_string); }
                           
static NIL_type initialize_multiply_native(NIL_type)
  { native_define(mul_string,
                  multiply_native); }

/*------------------------------ / -------------------------------------------*/

static const TXT_type div_string = "/";

static EXP_type divide_floats(FLO_type Raw_real,
                              VEC_type Vector)
  { EXP_type value;
    FLO_type raw_real,
             raw_real_product;
    LNG_type raw_number;
    REA_type real;
    TAG_type tag;
    UNS_type index,
             size;
    raw_real_product = Raw_real;
    size = size_VEC(Vector);
    for (index = 2;
         index <= size;
         index++)
      { value = Vector[index];
        tag = Tag_of(value);
        switch (tag)
          { case NBR_tag:
              raw_number = get_NBR(value);
              raw_real_product /= raw_number;
              break;
            case REA_tag:
              real = value;
              raw_real = real->flo;
              raw_real_product /= raw_real;
              break;
            default:
              return Main_Error_Text(ARN_error_string,
                                     div_string); }}
    return make_REA(raw_real_product); }

static EXP_type divide_native(VEC_type Vector,
                              EXP_type Tail_call)
  { EXP_type value;
    FLO_type raw_real;
    LNG_type raw_number;
    REA_type real;
    TAG_type tag;
    UNS_type size;
    size = size_VEC(Vector);
    if (size == 0)
      return Main_Error_Text(AL1_error_string,
                             div_string);
    value = Vector[1];
    tag = Tag_of(value);
    if (size == 1)
      { switch (tag)
          { case NBR_tag:
              raw_number = get_NBR(value);
              return make_REA(1.0 / raw_number);
            case REA_tag:
              real = value;
              raw_real = real->flo;
              return make_REA(1.0 / raw_real); }
        return Main_Error_Text(ARN_error_string,
                               div_string); }
    switch (tag)
      { case NBR_tag:
          return divide_floats(get_NBR(value),
                               Vector);
        case REA_tag:
          real = value;
          raw_real = real->flo;
          return divide_floats(raw_real,
                               Vector); }
    return Main_Error_Text(ARN_error_string,
                           div_string); }

static NIL_type initialize_divide_native(NIL_type)
  { native_define(div_string,
                  divide_native); }

/*------------------------------ quotient ------------------------------------*/

static const TXT_type quo_string = "quotient";

static EXP_type quotient_native(VEC_type Vector,
                                EXP_type Tail_call)
  { LNG_type raw_left,
             raw_number,
             raw_right;
    NBR_type left,
             number,
             right;
    UNS_type size;
    size = size_VEC(Vector);
    if (size != 2)
      return Main_Error_Text(EX2_error_string,
                             quo_string);
    left = Vector[1];
    right = Vector[2];
    if (!is_NBR(left) || !is_NBR(right))
      return Main_Error_Text(ARN_error_string,
                             quo_string);
    raw_left = get_NBR(left);
    raw_right = get_NBR(right);
    raw_number = raw_left / raw_right;
    number = make_NBR(raw_number);
    return number; }

static NIL_type initialize_quotient_native(NIL_type)
  { native_define(quo_string,
                  quotient_native); }

/*------------------------------ remainder -----------------------------------*/

static const TXT_type rem_string = "remainder";

static EXP_type remainder_native(VEC_type Vector,
                                 EXP_type Tail_call)
  { LNG_type raw_left,
             raw_number,
             raw_right;
    NBR_type left,
             number,
             right;
    UNS_type size;
    size = size_VEC(Vector);
    if (size != 2)
      return Main_Error_Text(EX2_error_string,
                             rem_string);
    left = Vector[1];
    right = Vector[2];
    if (!is_NBR(left) || !is_NBR(right))
      return Main_Error_Text(ARN_error_string,
                             rem_string);
    raw_left = get_NBR(left);
    raw_right = get_NBR(right);
    raw_number = raw_left % raw_right;
    number = make_NBR(raw_number);
    return number; }

static NIL_type initialize_remainder_native(NIL_type)
  { native_define(rem_string,
                  remainder_native); }
                  
/*----------------------------------------------------------------------------*/
/*----------------------- transcendent functions -----------------------------*/
/*----------------------------------------------------------------------------*/

/*--------------------------------- sqrt -------------------------------------*/

static const TXT_type sqt_string = "sqrt";

static EXP_type sqrt_native(VEC_type Vector,
                            EXP_type Tail_call)
  { EXP_type value;
    FLO_type raw_float;
    LNG_type raw_number;
    REA_type real;
    TAG_type tag;
    UNS_type size;
    size = size_VEC(Vector);
    if (size != 1)
      return Main_Error_Text(EX1_error_string,
                             sqt_string);
    value = Vector[1];
    tag = Tag_of(value);
    switch (tag)
      { case NBR_tag:
          raw_number = get_NBR(value);
          raw_float = sqrt(raw_number);
          break;
        case REA_tag:
          real = value;
          raw_float = real->flo;
          raw_float = sqrt(raw_float);
          break;
        default:
          return Main_Error_Text(ARN_error_string,
                                 sqt_string); }
    return make_REA(raw_float); }
        
static NIL_type initialize_sqrt_native(NIL_type)
  { native_define(sqt_string,
                  sqrt_native); }

/*----------------------------------------------------------------------------*/
/*------------------------------ comparison ----------------------------------*/
/*----------------------------------------------------------------------------*/

/*------------------------------ = -------------------------------------------*/

static const TXT_type eql_string = "=";

static EXP_type equal_floats(FLO_type Raw_real,
                             VEC_type Vector,
                             UNS_type Index)
  { EXP_type value;
    FLO_type raw_previous_real,
             raw_real;
    REA_type real;
    TAG_type tag;
    UNS_type index,
             size;
    raw_previous_real = Raw_real;
    size = size_VEC(Vector);
    for (index = Index;
         index <= size;
         index++)
      { value = Vector[index];
        tag = Tag_of(value);
        switch (tag)
          { case NBR_tag:
              raw_real = get_NBR(value);
              break;
            case REA_tag:
              real = value;
              raw_real = real->flo;
              break;
            default:
              return Main_Error_Text(ARN_error_string,
                                     eql_string); }
        if (raw_previous_real != raw_real)
          return Main_False;
        raw_previous_real = raw_real; }
    return Main_True; }
   
static EXP_type equal_integers(NBR_type Number,
                               VEC_type Vector)
  { EXP_type value;
    FLO_type raw_real;
    LNG_type raw_number,
             raw_previous_number;
    REA_type real;
    TAG_type tag;
    UNS_type index,
             size;
    raw_previous_number = get_NBR(Number);
    size = size_VEC(Vector);
    for (index = 2;
         index <= size;
         index++)
      { value = Vector[index];
        tag = Tag_of(value);
        switch (tag)
          { case NBR_tag:
              raw_number = get_NBR(value);
              if (raw_previous_number != raw_number)
                return Main_False;
              break;
            case REA_tag:
              real = value;
              raw_real = real->flo;
              if (raw_previous_number != raw_real)
                return Main_False;
              return equal_floats(raw_real,
                                  Vector,
                                  index + 1);
            default:
              return Main_Error_Text(ARN_error_string,
                                     eql_string); }
        raw_previous_number = raw_number; }
    return Main_True; }
    
static EXP_type equal_native(VEC_type Vector,
                             EXP_type Tail_call)
  { EXP_type value;
    FLO_type raw_real;
    REA_type real;
    TAG_type tag;
    UNS_type size;
    size = size_VEC(Vector);
    if (size < 2)
      return Main_Error_Text(AL2_error_string,
                             eql_string);
    value = Vector[1];
    tag = Tag_of(value);
    switch (tag)
      { case NBR_tag:
          return equal_integers(value,
                                Vector);
        case REA_tag:
          real = value;
          raw_real = real->flo;
          return equal_floats(raw_real,
                              Vector,
                              2); }
    return Main_Error_Text(ARN_error_string,
                           eql_string); }
    
static NIL_type initialize_equal_native(NIL_type)
  { native_define(eql_string,
                  equal_native); }

/*------------------------------ > -------------------------------------------*/

static const TXT_type grt_string = ">";

static EXP_type greater_floats(FLO_type Raw_real,
                               VEC_type Vector,
                               UNS_type Index)
  { EXP_type value;
    FLO_type raw_previous_real,
             raw_real;
    REA_type real;
    TAG_type tag;
    UNS_type index,
             size;
    raw_previous_real = Raw_real;
    size = size_VEC(Vector);
    for (index = Index;
         index <= size;
         index++)
      { value = Vector[index];
        tag = Tag_of(value);
        switch (tag)
          { case NBR_tag:
              raw_real = get_NBR(value);
              break;
            case REA_tag:
              real = value;
              raw_real = real->flo;
              break;
            default:
              return Main_Error_Text(ARN_error_string,
                                     grt_string); }
        if (raw_previous_real <= raw_real)
          return Main_False;
        raw_previous_real = raw_real; }
   return Main_True; }

static EXP_type greater_integers(NBR_type Number,
                                 VEC_type Vector)
  { EXP_type value;
    FLO_type raw_real;
    LNG_type raw_number,
             raw_previous_number;
    REA_type real;
    TAG_type tag;
    UNS_type index,
             size;
    raw_previous_number = get_NBR(Number);
    size = size_VEC(Vector);
    for (index = 2;
         index <= size;
         index++)
      { value = Vector[index];
        tag = Tag_of(value);
        switch (tag)
          { case NBR_tag:
              raw_number = get_NBR(value);
              if (raw_previous_number <= raw_number)
                return Main_False;
              break;
            case REA_tag:
              real = value;
              raw_real = real->flo;
              if (raw_previous_number <= raw_real)
                return Main_False;
              return greater_floats(raw_real,
                                    Vector,
                                    index + 1);
            default:
              return Main_Error_Text(ARN_error_string,
                                     grt_string); }
        raw_previous_number = raw_number; }
    return Main_True; }

static EXP_type greater_native(VEC_type Vector,
                               EXP_type Tail_call)
  { EXP_type value;
    FLO_type raw_real;
    REA_type real;
    TAG_type tag;
    UNS_type size;
    size = size_VEC(Vector);
    if (size < 2)
      return Main_Error_Text(AL2_error_string,
                             grt_string);
    value = Vector[1];
    tag = Tag_of(value);
    switch (tag)
      { case NBR_tag:
          return greater_integers(value,
                                  Vector);
        case REA_tag:
          real = value;
          raw_real = real->flo;
          return greater_floats(raw_real,
                                Vector,
                                2); }
    return Main_Error_Text(ARN_error_string,
                           grt_string); }

static NIL_type initialize_greater_native(NIL_type)
  { native_define(grt_string,
                  greater_native); }

/*------------------------------ < -------------------------------------------*/

static const TXT_type lss_string = "<";

static EXP_type less_floats(FLO_type Raw_real,
                            VEC_type Vector,
                            UNS_type Index)
  { EXP_type value;
    FLO_type raw_previous_real,
             raw_real;
    REA_type real;
    TAG_type tag;
    UNS_type index,
             size;
    raw_previous_real = Raw_real;
    size = size_VEC(Vector);
    for (index = Index;
         index <= size;
         index++)
      { value = Vector[index];
        tag = Tag_of(value);
        switch (tag)
          { case NBR_tag:
              raw_real = get_NBR(value);
              break;
            case REA_tag:
              real = value;
              raw_real = real->flo;
              break;
            default:
              return Main_Error_Text(ARN_error_string,
                                     lss_string); }
        if (raw_previous_real >= raw_real)
          return Main_False;
        raw_previous_real = raw_real; }
   return Main_True; }

static EXP_type less_integers(NBR_type Number,
                              VEC_type Vector)
  { EXP_type value;
    FLO_type raw_real;
    LNG_type raw_number,
             raw_previous_number;
    REA_type real;
    TAG_type tag;
    UNS_type index,
             size;
    raw_previous_number = get_NBR(Number);
    size = size_VEC(Vector);
    for (index = 2;
         index <= size;
         index++)
      { value = Vector[index];
        tag = Tag_of(value);
        switch (tag)
          { case NBR_tag:
              raw_number = get_NBR(value);
              if (raw_previous_number >= raw_number)
                return Main_False;
              break;
            case REA_tag:
              real = value;
              raw_real = real->flo;
              if (raw_previous_number >= raw_real)
                return Main_False;
              return less_floats(raw_real,
                                 Vector,
                                 index + 1);
            default:
              return Main_Error_Text(ARN_error_string,
                                     lss_string); }
        raw_previous_number = raw_number; }
    return Main_True; }
    
static EXP_type less_native(VEC_type Vector,
                            EXP_type Tail_call)
  { EXP_type value;
    FLO_type raw_real;
    REA_type real;
    TAG_type tag;
    UNS_type size;
    size = size_VEC(Vector);
    if (size < 2)
      return Main_Error_Text(AL2_error_string,
                             lss_string);
    value = Vector[1];
    tag = Tag_of(value);
    switch (tag)
      { case NBR_tag:
          return less_integers(value,
                               Vector);
        case REA_tag:
          real = value;
          raw_real = real->flo;
          return less_floats(raw_real,
                             Vector,
                             2); }
    return Main_Error_Text(ARN_error_string,
                           lss_string); }
    
static NIL_type initialize_less_native(NIL_type)
  { native_define(lss_string,
                  less_native); }

/*------------------------------ >= ------------------------------------------*/

static const TXT_type geq_string = ">=";

static EXP_type greater_or_equal_floats(FLO_type Raw_real,
                                        VEC_type Vector,
                                        UNS_type Index)
  { EXP_type value;
    FLO_type raw_previous_real,
             raw_real;
    REA_type real;
    TAG_type tag;
    UNS_type index,
             size;
    raw_previous_real = Raw_real;
    size = size_VEC(Vector);
    for (index = Index;
         index <= size;
         index++)
      { value = Vector[index];
        tag = Tag_of(value);
        switch (tag)
          { case NBR_tag:
              raw_real = get_NBR(value);
              break;
            case REA_tag:
              real = value;
              raw_real = real->flo;
              break;
            default:
              return Main_Error_Text(ARN_error_string,
                                     geq_string); }
        if (raw_previous_real < raw_real)
          return Main_False;
        raw_previous_real = raw_real; }
   return Main_True; }

static EXP_type greater_or_equal_integers(NBR_type Number,
                                          VEC_type Vector)
  { EXP_type value;
    FLO_type raw_real;
    LNG_type raw_number,
             raw_previous_number;
    REA_type real;
    TAG_type tag;
    UNS_type index,
             size;
    raw_previous_number = get_NBR(Number);
    size = size_VEC(Vector);
    for (index = 2;
         index <= size;
         index++)
      { value = Vector[index];
        tag = Tag_of(value);
        switch (tag)
          { case NBR_tag:
              raw_number = get_NBR(value);
              if (raw_previous_number < raw_number)
                return Main_False;
              break;
            case REA_tag:
              real = value;
              raw_real = real->flo;
              if (raw_previous_number < raw_real)
                return Main_False;
              return greater_or_equal_floats(raw_real,
                                             Vector,
                                             index + 1);
            default:
              return Main_Error_Text(ARN_error_string,
                                     geq_string); }
        raw_previous_number = raw_number; }
    return Main_True; }

static EXP_type greater_or_equal_native(VEC_type Vector,
                                        EXP_type Tail_call)
  { EXP_type value;
    FLO_type raw_real;
    REA_type real;
    TAG_type tag;
    UNS_type size;
    size = size_VEC(Vector);
    if (size < 2)
      return Main_Error_Text(AL2_error_string,
                             geq_string);
    value = Vector[1];
    tag = Tag_of(value);
    switch (tag)
      { case NBR_tag:
          return greater_or_equal_integers(value,
                                           Vector);
        case REA_tag:
          real = value;
          raw_real = real->flo;
          return greater_or_equal_floats(raw_real,
                                         Vector,
                                         2); }
    return Main_Error_Text(ARN_error_string,
                           geq_string); }
    
static NIL_type initialize_greater_or_equal_native(NIL_type)
  { native_define(geq_string,
                  greater_or_equal_native); }

/*------------------------------ <= ------------------------------------------*/

static const TXT_type leq_string = "<=";

static EXP_type less_or_equal_floats(FLO_type Raw_real,
                                     VEC_type Vector,
                                     UNS_type Index)
  { EXP_type value;
    FLO_type raw_previous_real,
             raw_real;
    REA_type real;
    TAG_type tag;
    UNS_type index,
             size;
    raw_previous_real = Raw_real;
    size = size_VEC(Vector);
    for (index = Index;
         index <= size;
         index++)
      { value = Vector[index];
        tag = Tag_of(value);
        switch (tag)
          { case NBR_tag:
              raw_real = get_NBR(value);
              break;
            case REA_tag:
              real = value;
              raw_real = real->flo;
              break;
            default:
              return Main_Error_Text(ARN_error_string,
                                     leq_string); }
        if (raw_previous_real > raw_real)
          return Main_False;
        raw_previous_real = raw_real; }
   return Main_True; }

static EXP_type less_or_equal_integers(NBR_type Number,
                                       VEC_type Vector)
  { EXP_type value;
    FLO_type raw_real;
    LNG_type raw_number,
             raw_previous_number;
    REA_type real;
    TAG_type tag;
    UNS_type index,
             size;
    raw_previous_number = get_NBR(Number);
    size = size_VEC(Vector);
    for (index = 2;
         index <= size;
         index++)
      { value = Vector[index];
        tag = Tag_of(value);
        switch (tag)
          { case NBR_tag:
              raw_number = get_NBR(value);
              if (raw_previous_number > raw_number)
                return Main_False;
              break;
            case REA_tag:
              real = value;
              raw_real = real->flo;
              if (raw_previous_number > raw_real)
                return Main_False;
              return less_or_equal_floats(raw_real,
                                          Vector,
                                          index + 1);
            default:
              return Main_Error_Text(ARN_error_string,
                                     grt_string); }
        raw_previous_number = raw_number; }
    return Main_True; }
    
static EXP_type less_or_equal_native(VEC_type Vector,
                                     EXP_type Tail_call)
  { EXP_type value;
    FLO_type raw_real;
    REA_type real;
    TAG_type tag;
    UNS_type size;
    size = size_VEC(Vector);
    if (size < 2)
      return Main_Error_Text(AL2_error_string,
                             leq_string);
    value = Vector[1];
    tag = Tag_of(value);
    switch (tag)
      { case NBR_tag:
          return less_or_equal_integers(value,
                                        Vector);
        case REA_tag:
          real = value;
          raw_real = real->flo;
          return less_or_equal_floats(raw_real,
                                      Vector,
                                      2); }
    return Main_Error_Text(ARN_error_string,
                           leq_string); }
    
static NIL_type initialize_less_or_equal_native(NIL_type)
  { native_define(leq_string,
                  less_or_equal_native); }

/*------------------------------ eq? -----------------------------------------*/

static const TXT_type eqq_string = "eq?";

static EXP_type equivalent_native(VEC_type Vector,
                                  EXP_type Tail_call)
  { ADR_type left,
             right;
    UNS_type size;
    left  = Vector[1],
    right = Vector[2];
    size = size_VEC(Vector);
    if (size != 2)
      return Main_Error_Text(EX2_error_string,
                             eqq_string);
    if (left == right)
      return Main_True;
    return Main_False; }

static NIL_type initialize_equivalent_native(NIL_type)
  { native_define(eqq_string,
                  equivalent_native); }

/*----------------------------------------------------------------------------*/
/*------------------------------ booleans ------------------------------------*/
/*----------------------------------------------------------------------------*/

/*-------------------------------- not ---------------------------------------*/

static const TXT_type not_string = "not";

static EXP_type not_native(VEC_type Vector,
                           EXP_type Tail_call)
  { EXP_type value;
    UNS_type size;
    size = size_VEC(Vector);
    if (size != 1)
      return Main_Error_Text(EX1_error_string,
                             not_string);
    value = Vector[1];
    if (is_FLS(value))
      return Main_True;
    return Main_False; }

static NIL_type initialize_not_native(NIL_type)
  { native_define(not_string,
                  not_native); }

/*----------------------------------------------------------------------------*/
/*------------------------------ predicates ----------------------------------*/
/*----------------------------------------------------------------------------*/

/*------------------------------ null? ---------------------------------------*/

static const TXT_type inl_string = "null?";

static EXP_type is_null_native(VEC_type Vector,
                               EXP_type Tail_call)
  { EXP_type expression;
    UNS_type size;
    size = size_VEC(Vector);
    if (size != 1)
      return Main_Error_Text(EX1_error_string,
                             inl_string);
    expression = Vector[1];
    if (is_NUL(expression))
      return Main_True;
    return Main_False; }

static NIL_type initialize_is_null_native(NIL_type)
  { native_define(inl_string,
                  is_null_native); }

/*------------------------------ pair? ---------------------------------------*/

static const TXT_type ipr_string = "pair?";

static EXP_type is_pair_native(VEC_type Vector,
                               EXP_type Tail_call)
  { EXP_type expression;
    UNS_type size;
    size = size_VEC(Vector);
    if (size != 1)
      return Main_Error_Text(EX1_error_string,
                             ipr_string);
    expression = Vector[1];
    if (is_PAI(expression))
      return Main_True;
    return Main_False; }

static NIL_type initialize_is_pair_native(NIL_type)
  { native_define(ipr_string,
                  is_pair_native); }

/*------------------------------ symbol? -------------------------------------*/

static const TXT_type ism_string = "symbol?";

static EXP_type is_symbol_native(VEC_type Vector,
                                 EXP_type Tail_call)
  { EXP_type expression;
    UNS_type size;
    size = size_VEC(Vector);
    if (size != 1)
      return Main_Error_Text(EX1_error_string,
                             ism_string);
    expression = Vector[1];
    if (is_SYM(expression))
      return Main_True;
    return Main_False; }

static NIL_type initialize_is_symbol_native(NIL_type)
  { native_define(ism_string,
                  is_symbol_native); }

/*------------------------------ vector? -------------------------------------*/

static const TXT_type ivc_string = "vector?";

static EXP_type is_vector_native(VEC_type Vector,
                                 EXP_type Tail_call)
  { EXP_type expression;
    UNS_type size;
    size = size_VEC(Vector);
    if (size != 1)
      return Main_Error_Text(EX1_error_string,
                             ivc_string);
    expression = Vector[1];
    if (is_VEC(expression))
      return Main_True;
    return Main_False; }

static NIL_type initialize_is_vector_native(NIL_type)
  { native_define(ivc_string,
                  is_vector_native); }

/*----------------------------------------------------------------------------*/
/*------------------------------ pairs ---------------------------------------*/
/*----------------------------------------------------------------------------*/

/*------------------------------ cons ----------------------------------------*/

static const TXT_type cns_string = "cons";

static EXP_type cons_native(VEC_type Vector,
                            EXP_type Tail_call)
  { EXP_type car,
             cdr;
    UNS_type size;
    size = size_VEC(Vector);
    if (size != 2)
      return Main_Error_Text(EX2_error_string,
                             cns_string);
    car = Vector[1];
    cdr = Vector[2];
    return make_PAI(car,
                    cdr); }

static NIL_type initialize_cons_native(NIL_type)
  { native_define(cns_string,
                  cons_native); }

/*------------------------------ car -----------------------------------------*/

static const TXT_type car_string = "car";

static EXP_type car_native(VEC_type Vector,
                           EXP_type Tail_call)
  { EXP_type value;
    PAI_type pair;
    UNS_type size;
    size = size_VEC(Vector);
    if (size != 1)
      return Main_Error_Text(EX1_error_string,
                             car_string);
    pair = Vector[1];
    if (!is_PAI(pair))
      return Main_Error_Text(NAP_error_string,
                             car_string);
    value = pair->car;
    return value; }

static NIL_type initialize_car_native(NIL_type)
  { native_define(car_string,
                  car_native); }

/*------------------------------ cdr -----------------------------------------*/

static const TXT_type cdr_string = "cdr";

static EXP_type cdr_native(VEC_type Vector,
                           EXP_type Tail_call)
  { EXP_type value;
    PAI_type pair;
    UNS_type size;
    size = size_VEC(Vector);
    if (size != 1)
      return Main_Error_Text(EX1_error_string,
                             cdr_string);
    pair = Vector[1];
    if (!is_PAI(pair))
      return Main_Error_Text(NAP_error_string,
                             cdr_string);
    value = pair->cdr;
    return value; }

static NIL_type initialize_cdr_native(NIL_type)
  { native_define(cdr_string,
                  cdr_native); }

/*------------------------------ set-car! ------------------------------------*/

static const TXT_type sca_string = "set-car!";

static EXP_type set_car_native(VEC_type Vector,
                               EXP_type Tail_call)
  { EXP_type value;
    PAI_type pair;
    UNS_type size;
    size = size_VEC(Vector);
    if (size != 2)
      return Main_Error_Text(EX2_error_string,
                             sca_string);
    pair = Vector[1];
    value = Vector[2];
    if (!is_PAI(pair))
      return Main_Error_Text(NAP_error_string,
                             sca_string);
    pair->car = value;
    return pair; }

static NIL_type initialize_set_car_native(NIL_type)
  { native_define(sca_string,
                  set_car_native); }

/*------------------------------ set-cdr! ------------------------------------*/

static const TXT_type scd_string = "set-cdr!";

static EXP_type set_cdr_native(VEC_type Vector,
                               EXP_type Tail_call)
  { EXP_type value;
    PAI_type pair;
    UNS_type size;
    size = size_VEC(Vector);
    if (size != 2)
      return Main_Error_Text(EX2_error_string,
                             scd_string);
    pair = Vector[1];
    value = Vector[2];
    if (!is_PAI(pair))
      return Main_Error_Text(NAP_error_string,
                             scd_string);
    pair->cdr = value;
    return pair; }

static NIL_type initialize_set_cdr_native(NIL_type)
  { native_define(scd_string,
                  set_cdr_native); }

/*------------------------------ assoc ---------------------------------------*/

static const TXT_type ass_string = "assoc";

static EXP_type assoc_native(VEC_type Vector,
                             EXP_type Tail_call)
  { EXP_type value;
    PAI_type list,
             pair;
    TAG_type tag;
    UNS_type size;
    size = size_VEC(Vector);
    if (size != 2)
      return Main_Error_Text(EX2_error_string,
                             ass_string);
    value = Vector[1];
    list = Vector[2];
    for (;;)
      { tag = Tag_of(list);
        switch (tag)
          { case NUL_tag:
              return Main_False;
            case PAI_tag:
              pair = list->car;
              if (!is_PAI(pair))
                return Main_Error_Text(NAP_error_string,
                                       ass_string);
              if (value == pair->car)
                return pair;
              list = list->cdr;
              break; 
            default:
              return Main_Error_Text(NAL_error_string,
                                     ass_string); }}}

static NIL_type initialize_assoc_native(NIL_type)
  { native_define(ass_string,
                  assoc_native); }

/*----------------------------------------------------------------------------*/
/*-------------------------------- vectors -----------------------------------*/
/*----------------------------------------------------------------------------*/

/*------------------------------ make-vector ---------------------------------*/

static const TXT_type mkv_string = "make-vector";

static EXP_type make_vector_native(VEC_type Vector,
                                   EXP_type Tail_call)
  { EXP_type value;
    LNG_type raw_number;
    NBR_type number;
    UNS_type index,
             size;
    VEC_type vector;
    size = size_VEC(Vector);
    if (size == 0)
      return Main_Error_Text(AL1_error_string,
                             mkv_string);
    if (size > 2)
      return Main_Error_Text(AM2_error_string,
                             mkv_string);
    value = (size == 1)? Main_Unspecified: Vector[2];
    number = Vector[1];
    if (!is_NBR(number))
      return Main_Error_Text(NAN_error_string,
                             mkv_string);
    raw_number = get_NBR(number);
    if (raw_number < 0)
      return Main_Error_Text(NNN_error_string,
                             mkv_string);
    if (raw_number == 0)
      return Main_Empty_Vector;
    survive_dynamic(value,
                    raw_number);
    vector = make_VEC(raw_number);
    for (index = 1;
         index <= raw_number;
         index++)
      vector[index] = value; 
    return vector; }

static NIL_type initialize_make_vector_native(NIL_type)
  { native_define(mkv_string,
                  make_vector_native); }

/*------------------------------ vector-ref ---------------------------------*/

static const TXT_type vrf_string = "vector-ref";

static EXP_type vector_ref_native(VEC_type Vector,
                                  EXP_type Tail_call)
  { EXP_type value;
    LNG_type raw_number;
    NBR_type number;
    UNS_type size;
    VEC_type vector;
    size = size_VEC(Vector);
    if (size != 2)
      return Main_Error_Text(EX2_error_string,
                             vrf_string);
    vector = Vector[1];
    if (!is_VEC(vector))
      return Main_Error_Text(NAV_error_string,
                             vrf_string);
    size = size_VEC(vector);
    number = Vector[2];
    if (!is_NBR(number))
      return Main_Error_Text(NAN_error_string,
                             vrf_string);
    raw_number = get_NBR(number);
    if (raw_number < 0)
      return Main_Error_Text(NNN_error_string,
                             vrf_string);
    if (raw_number >= size)
      return Main_Error_Text(IOR_error_string,
                             vrf_string);
    value = vector[raw_number + 1];
    return value; }

static NIL_type initialize_vector_ref_native(NIL_type)
  { native_define(vrf_string,
                  vector_ref_native); }

/*------------------------------ vector-set! ---------------------------------*/

static const TXT_type vst_string = "vector-set!";

static EXP_type vector_set_native(VEC_type Vector,
                                  EXP_type Tail_call)
  { EXP_type value;
    LNG_type raw_number;
    NBR_type number;
    UNS_type size;
    VEC_type vector;
    size = size_VEC(Vector);
    if (size != 3)
      return Main_Error_Text(EX3_error_string,
                             vst_string);
    vector = Vector[1];
    if (!is_VEC(vector))
      return Main_Error_Text(NAV_error_string,
                             vst_string);
    size = size_VEC(vector);
    number = Vector[2];
    if (!is_NBR(number))
      return Main_Error_Text(NAN_error_string,
                             vst_string);
    raw_number = get_NBR(number);
    if (raw_number < 0)
      return Main_Error_Text(NNN_error_string,
                             vst_string);
    if (raw_number >= size)
      return Main_Error_Text(IOR_error_string,
                             vst_string);
    value =  Vector[3];
    vector[raw_number + 1] = value;
    return vector; }

static NIL_type initialize_vector_set_native(NIL_type)
  { native_define(vst_string,
                  vector_set_native); }

/*------------------------------ vector --------------------------------------*/

static const TXT_type vec_string = "vector";

static EXP_type vector_native(VEC_type Vector,
                              EXP_type Tail_call)
  { UNS_type index,
             size;
    VEC_type vector;
    size = size_VEC(Vector);
    if (size == 0)
      return Main_Empty_Vector;
    survive_dynamic(Vector,
                    size);
    vector = make_VEC(size);
    for (index = 1;
         index <= size;
         index++)
      vector[index] = Vector[index]; 
    return vector; }

static NIL_type initialize_vector_native(NIL_type)
  { native_define(vec_string,
                  vector_native); }

/*------------------------------ vector-length -------------------------------*/

static const TXT_type vcl_string = "vector-length";

static EXP_type vector_length_native(VEC_type Vector,
                                     EXP_type Tail_call)
  { EXP_type value;
    UNS_type size;
    VEC_type vector;
    size = size_VEC(Vector);
    if (size != 1)
      return Main_Error_Text(EX1_error_string,
                             vcl_string);
    vector = Vector[1];
    if (!is_VEC(vector))
      return Main_Error_Text(NAV_error_string,
                             vcl_string);
    size = size_VEC(vector);
    value = make_NBR(size);
    return value; }

static NIL_type initialize_vector_length_native(NIL_type)
  { native_define(vcl_string,
                  vector_length_native); }

/*----------------------------------------------------------------------------*/
/*------------------------------ control -------------------------------------*/
/*----------------------------------------------------------------------------*/

/*------------------------------ apply ---------------------------------------*/

static const TXT_type apl_string = "apply";

static EXP_type apply_native(VEC_type Vector,
                             EXP_type Tail_call)
  { UNS_type size;
    size = size_VEC(Vector);
    if (size != 2)
      return Main_Error_Text(EX2_error_string,
                             apl_string);
    return Evaluate_Apply(Vector,
                          Tail_call); }

static NIL_type initialize_apply_native(NIL_type)
  { native_define(apl_string,
                  apply_native); }

/*------------------------------ map -----------------------------------------*/

static const TXT_type map_string = "map";

static NBR_type Continue_map;

begin_frame(mAP);
 frame_slot(VEC, vec);
 frame_slot(PAI, arg);
 frame_slot(PAI, lst);
end_frame(mAP);

static EXP_type continue_map(EXP_type Value,
                             EXP_type Tail_call)
  { mAP_type map_thread;
    EXP_type value;
    PAI_type arguments,
             list,
             pair;
    TAG_type tag;
    VEC_type vector;
    survive_default(Value);
    map_thread = (mAP_type)Thread_Peek();
    arguments = map_thread->arg;
    list      = map_thread->lst;
    tag = Tag_of(arguments);
    list = make_PAI(Value,
                    list);
    switch (tag)
      { case NUL_tag:
          Thread_Zap();
          list = Main_Reverse(list);
          return list;
        case PAI_tag:
          value     = arguments->car;
          arguments = arguments->cdr;
          pair = make_PAI(value,
                          Main_Empty_Pair);
          vector = map_thread->vec;
          vector[2] = pair;
          map_thread = (mAP_type)Thread_Keep();
          map_thread->arg = arguments;
          map_thread->lst = list;
          return Evaluate_Apply(vector,
                                Main_False); }       /* optimize if last call */
    return Main_Error_Text(NAL_error_string,
                           map_string); }

/*----------------------------------------------------------------------------*/

static EXP_type map_native(VEC_type Vector,
                           EXP_type Tail_call)
  { mAP_type map_thread;
    EXP_type procedure,
             value;
    PAI_type arguments,
             pair;
    TAG_type tag;
    UNS_type size;
    VEC_type vector;
    size = size_VEC(Vector);
    if (size != 2)
      return Main_Error_Text(EX2_error_string,
                             map_string);
    arguments = Vector[2];
    tag = Tag_of(arguments);
    switch (tag)
      { case NUL_tag:
          return Main_Empty_Pair;
        case PAI_tag:
          procedure = Vector[1];
          value     = arguments->car;
          arguments = arguments->cdr;
          vector = Environment_Make_Frame(2);
          pair = make_PAI(value,
                          Main_Empty_Pair);
          vector[1] = procedure;
          vector[2] = pair;
          map_thread = (mAP_type)Thread_Push(Continue_map,
                                             Tail_call,
                                             mAP_size);
          map_thread->vec = vector;
          map_thread->arg = arguments;
          map_thread->lst = Main_Empty_Pair;
          return Evaluate_Apply(vector,
                                Main_False); }     /* optimize if only 1 call */
    return Main_Error_Text(NAL_error_string,
                            map_string); }
static NIL_type initialize_map_native(NIL_type)
  { Continue_map = Thread_Register(continue_map,
                                   mAP_size);
    native_define(map_string,
                  map_native); }

/*------------------------------ eval ----------------------------------------*/

static const TXT_type evl_string = "eval";

static NBR_type Continue_eval;

begin_frame(eVA);
 frame_slot(VEC, env);
 frame_slot(VEC, frm);
end_frame(eVA);

static EXP_type continue_eval(EXP_type Value,
                              EXP_type Tail_call)
  { eVA_type eval_thread;
    VEC_type environment,
             frame;
    Environment_Release_Frame();
    eval_thread = (eVA_type)Thread_Peek();
    environment = eval_thread->env;
    frame       = eval_thread->frm;
    Thread_Zap();
    Environment_Replace(environment,
                        frame);
    return Value; }

static EXP_type eval_native(VEC_type Vector,
                            EXP_type Tail_call)
  { eVA_type eval_thread;
    EXP_type compiled_expression,
             expression;
    UNS_type size;
    size = size_VEC(Vector);
    if (size != 1)
      return Main_Error_Text(EX1_error_string,
                             evl_string);
    expression = Vector[1];
    compiled_expression = Compile_Compile(expression);
    survive_default(compiled_expression);
    eval_thread = (eVA_type)Thread_Push(Continue_eval,
                                        Main_False,
                                        eVA_size);
    eval_thread->env = Environment_Get_Environment();
    eval_thread->frm = Environment_Get_Frame();
    Environment_Rollback();
    return Evaluate_Evaluate(compiled_expression); }

static NIL_type initialize_eval_native(NIL_type)
  { Continue_eval = Thread_Register(continue_eval,
                                    eVA_size);
    native_define(evl_string,
                  eval_native); }

/*------------------------------ call-cc -------------------------------------*/

static const TXT_type ccc_string = "call-cc";

static EXP_type call_cc_native(VEC_type Vector,
                               EXP_type Tail_call)
  { CNT_type continuation;
    EXP_type procedure;
    PAI_type arguments;
    THR_type thread;
    UNS_type raw_size;
    VEC_type environment,
             frame,
             vector;
    raw_size = size_VEC(Vector);
    if (raw_size != 1)
      return Main_Error_Text(EX1_error_string,
                             ccc_string);
    procedure = Vector[1];
    environment = Environment_Get_Environment();
    frame = Environment_Get_Frame();
    thread = Thread_Mark();
    continuation = make_CNT(environment,
                            frame,
                            thread);
    arguments = make_PAI(continuation,
                         Main_Null);
    vector = Environment_Make_Frame(2);
    vector[1] = procedure;
    vector[2] = arguments;
    return Evaluate_Apply(vector,
                          Tail_call); }

static NIL_type initialize_call_cc_native(NIL_type)
  { native_define(ccc_string,
                  call_cc_native); }

/*----------------------------------------------------------------------------*/
/*------------------------------- input --------------------------------------*/
/*----------------------------------------------------------------------------*/

/*------------------------------- read ---------------------------------------*/

static const TXT_type rdd_string = "read";

static EXP_type read_native(VEC_type Vector,
                            EXP_type Tail_call)
  { EXP_type expression;
    STR_type name;
    TXT_type input,
             text;
    UNS_type size;
    size = size_VEC(Vector);
    if (size > 1)
      return Main_Error_Text(AM1_error_string,
                             rdd_string);
    if (size == 0)
      input = Main_Read();
    else
      { name = Vector[1];
        if (!is_STR(name))
          return Main_Error_Text(NAS_error_string,
                                 rdd_string);
        text = name->txt;
        input = Main_Load(text); }
    expression = Read_Parse(input);
    return expression; }

static NIL_type initialize_read_native(NIL_type)
  { native_define(rdd_string,
                  read_native); }

/*----------------------------------------------------------------------------*/
/*------------------------------ output --------------------------------------*/
/*----------------------------------------------------------------------------*/

/*------------------------------ display -------------------------------------*/

static const TXT_type dis_string = "display";

static EXP_type display_native(VEC_type Vector,
                               EXP_type Tail_call)
  { EXP_type value;
    UNS_type size;
    size = size_VEC(Vector);
    if (size != 1)
      return Main_Error_Text(EX1_error_string,
                             dis_string);
    value = Vector[1];
    Print_Display(value);
    return Main_Unspecified; }

static NIL_type initialize_display_native(NIL_type)
  { native_define(dis_string,
                  display_native); }

/*------------------------------ newline -------------------------------------*/

static const TXT_type nwl_string = "newline";

static EXP_type newline_native(VEC_type Vector,
                               EXP_type Tail_call)
  { UNS_type size;
    size = size_VEC(Vector);
    if (size != 0)
      return Main_Error_Text(EX0_error_string,
                             nwl_string);
    Print_Newline();
    return Main_Unspecified; }

static NIL_type initialize_newline_native(NIL_type)
  { native_define(nwl_string,
                  newline_native); }

/*------------------------------ pretty --------------------------------------*/

static const TXT_type prt_string = "pretty";

static EXP_type pretty_native(VEC_type Vector,
                              EXP_type Tail_call)
  { EXP_type compiled_expression,
             expression;
    UNS_type size;
    size = size_VEC(Vector);
    if (size != 1)
      return Main_Error_Text(EX1_error_string,
                             prt_string);
    expression = Vector[1];
    compiled_expression = Compile_Compile(expression);
    Print_Newline();
    Print_Print(compiled_expression);
    Print_Newline();
    return Main_Unspecified; }

static NIL_type initialize_pretty_native(NIL_type)
  { native_define(prt_string,
                  pretty_native); }

/*----------------------------------------------------------------------------*/
/*------------------------------ service -------------------------------------*/
/*----------------------------------------------------------------------------*/

/*------------------------------- clock --------------------------------------*/

static const TXT_type clk_string = "clock";

static EXP_type clock_native(VEC_type Vector,
                             EXP_type Tail_call)
  { NBR_type number;
    UNS_type size,
             seconds;
    size = size_VEC(Vector);
    if (size != 0)
      return Main_Error_Text(EX0_error_string,
                             clk_string);
    seconds = time(NULL);
    number = make_NBR(seconds);
    return number; }

static NIL_type initialize_clock_native(NIL_type)
  { native_define(clk_string,
                  clock_native); }

/*------------------------------ collect -------------------------------------*/

static const TXT_type col_string = "collect";

static EXP_type collect_native(VEC_type Vector,
                               EXP_type Tail_call)
  { UNS_type available,
             size;
    size = size_VEC(Vector);
    if (size != 0)
      return Main_Error_Text(EX0_error_string,
                             col_string);
    available = Main_Reclaim();
    return make_NBR(available); }

static NIL_type initialize_collect_native(NIL_type)
  { native_define(col_string,
                  collect_native); }

/*--------------------------------- exit -------------------------------------*/

static const TXT_type exi_string = "exit";

static EXP_type exit_native(VEC_type Vector,
                            EXP_type Tail_call)
  { UNS_type size;
    size = size_VEC(Vector);
    if (size != 0)
      return Main_Error_Text(EX0_error_string,
                             exi_string);
    return Main_Terminate(); }

static NIL_type initialize_exit_native(NIL_type)
  { native_define(exi_string,
                  exit_native); }

/*------------------------------ public functions ----------------------------*/

TXT_type Native_Get_Name(UNS_type Offset)
  { return get_native_name(Offset); }

NIL_type Native_Initialize(NIL_type)
  { initialize_add_native();
    initialize_apply_native();
    initialize_assoc_native();
    initialize_call_cc_native();
    initialize_car_native();
    initialize_cdr_native();
    initialize_circularity_level();
    initialize_clock_native();
    initialize_collect_native();
    initialize_cons_native();
    initialize_display_native();
    initialize_divide_native();
    initialize_equal_native();
    initialize_equivalent_native();
    initialize_eval_native();
    initialize_exit_native();
    initialize_greater_native();
    initialize_greater_or_equal_native();
    initialize_is_null_native();
    initialize_is_pair_native();
    initialize_is_symbol_native();
    initialize_is_vector_native();
    initialize_less_native();
    initialize_less_or_equal_native();
    initialize_make_vector_native();
    initialize_map_native();
    initialize_multiply_native();
    initialize_newline_native();
    initialize_not_native();
    initialize_pretty_native();
    initialize_quotient_native();
    initialize_read_native();
    initialize_remainder_native();
    initialize_set_car_native();
    initialize_set_cdr_native();
    initialize_sqrt_native();
    initialize_subtract_native();
    initialize_vector_length_native();
    initialize_vector_native();
    initialize_vector_ref_native();
    initialize_vector_set_native(); }
