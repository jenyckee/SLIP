                     /*-----------------------------------*/
                     /*             >>>Slip<<<            */
                     /*            Theo D'Hondt           */
                     /*     VUB Software Languages Lab    */
                     /*             (c) 2010              */
                     /*-----------------------------------*/
                     /*     version 12: smart caching     */
                     /*-----------------------------------*/
                     /*            dictionary             */
                     /*-----------------------------------*/

#include <stdio.h>

#include "SlipMain.h"
#include "SlipEnvironment.h"
#include "SlipDictionary.h"
        
/*------------------------------ private constants ---------------------------*/

static const TXT_type TMG_error_string = "too many global variables";

/*------------------------------ private variables ---------------------------*/

static ENV_type Current_environment;
static FRM_type Current_frame;
static FRM_type Global_frame;

static UNS_type Current_frame_size;
static UNS_type Current_scope_level;
static UNS_type Global_frame_size;

/*------------------------------ private functions ---------------------------*/

static UNS_type get_raw_offset(SYM_type Variable,
                           UNS_type Size,
                           FRM_type Frame)
  { FRM_type frame;
    UNS_type raw_offset = Size;
    for (frame = Frame;
         !is_NUL(frame);
         frame = frame->frm)
      { if (frame->var == Variable)
          return raw_offset;
        raw_offset--; }
    return 0; }

/*------------------------------ public functions ----------------------------*/

NIL_type Dictionary_Checkpoint(NIL_type)
  { Global_frame = Current_frame;
    Global_frame_size = Current_frame_size; }

UNS_type Dictionary_Define(SYM_type Variable)
  { if ((Current_scope_level == 0) &&
        Environment_Global_Overflow(Current_frame_size))
      Main_Error_Text(TMG_error_string,
                      Main_Define_String);
    Current_frame = make_FRM(Variable,
                             Current_frame);
    return ++Current_frame_size; }

NIL_type Dictionary_Enter_Nested_Scope(NIL_type)
  { NBR_type frame_size;
    frame_size = make_NBR(Current_frame_size);
    Current_environment = make_ENV(Current_frame,
                                   frame_size,
                                   Current_environment);
    Current_frame = Main_Empty_Frame;
    Current_frame_size = 0;
    Current_scope_level++; }

UNS_type Dictionary_Exit_Nested_Scope(NIL_type)
  { NBR_type frame_size;
    UNS_type previous_frame_size;
    previous_frame_size = Current_frame_size;
    frame_size = Current_environment->siz;
    Current_frame       = Current_environment->frm;
    Current_environment = Current_environment->env;
    Current_scope_level--;
    Current_frame_size = get_NBR(frame_size);
    return previous_frame_size; }

UNS_type Dictionary_Get_Frame_Size(NIL_type)
  { return Current_frame_size; }

NIL_type Dictionary_Initialize(NIL_type)
  { Current_environment = Main_Empty_Environment;
    Current_scope_level = 0;
    Global_frame = Current_frame = Main_Empty_Frame;
    Global_frame_size = Current_frame_size = 0;
    Main_Register((REF_type)&Current_environment);
    Main_Register((REF_type)&Current_frame);
    Main_Register((REF_type)&Global_frame); }

UNS_type Dictionary_Initially_Define(SYM_type Variable)
  { Current_frame = make_FRM(Variable,
                             Current_frame);
    return ++Current_frame_size; }

UNS_type Dictionary_Lexical_Address(SYM_type Variable,
                                    URF_type Scope   )
  { ENV_type environment;
    FRM_type frame;
    NBR_type frame_size;
    UNS_type raw_frame_size,
             raw_offset,
             raw_scope;
    raw_offset= get_raw_offset(Variable,
                       Current_frame_size,
                       Current_frame);
    if (raw_offset > 0)
      { *Scope = 0;
        return raw_offset; }
    if (Current_scope_level > 0)
      { raw_scope = Current_scope_level;
        for (environment = Current_environment;
             !is_NUL(environment);
             environment = environment->env)
          { frame_size = environment->siz;
            frame = environment->frm;
            raw_frame_size = get_NBR(frame_size);
            raw_offset = get_raw_offset(Variable,
                                raw_frame_size,
                                frame);
            if (raw_offset > 0)
              { *Scope = raw_scope;
                return raw_offset; }
            raw_scope--; }}
    return 0; }

NIL_type Dictionary_Rollback(NIL_type)
  { NBR_type frame_size;
    Main_Claim(Main_Default_Margin);
    Current_frame = Global_frame;
    Current_frame_size = Global_frame_size;
    frame_size = make_NBR(Current_frame_size);
    Current_environment = make_ENV(Current_frame,
                                   frame_size,
                                   Main_Empty_Environment);
    Current_scope_level = 0; }