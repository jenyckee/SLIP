                     /*-----------------------------------*/
                     /*             >>>Slip<<<            */
                     /*            Theo D'Hondt           */
                     /*     VUB Software Languages Lab    */
                     /*             (c) 2010              */
                     /*-----------------------------------*/
                     /*     version 12: smart caching     */
                     /*-----------------------------------*/
                     /*            environment            */
                     /*-----------------------------------*/

#include <stdio.h>

#include "SlipMain.h"
#include "SlipEnvironment.h"

/*------------------------------ private constants ---------------------------*/

enum { Cache_size  = 16,
       Global_size = 128 };

/*------------------------------ private variables ---------------------------*/

static VEC_type Cache[Cache_size];

static VEC_type Current_environment;
static VEC_type Current_frame;
static VEC_type Global_frame;

/*------------------------------ private functions ---------------------------*/

static NIL_type release_vector(VEC_type Vector)
  { UNS_type size; 
    size = size_VEC(Vector);
    if (size == 0)
      return;
    if (size >= Cache_size)
      return;
    if (!marked_VEC(Vector))
      { Vector[1] = Cache[size];
        Cache[size] = Vector; }}

static VEC_type acquire_vector(UNS_type Size)
  { VEC_type vector;
    if (Size == 0)
      return Main_Empty_Vector;
    if (Size >= Cache_size)
      return init_VEC(Size);
    vector = Cache[Size];
    if (is_NUL(vector))
      return init_VEC(Size);
    Cache[Size] = vector[1];
    return vector; }

static NIL_type flush_cache(NIL_type)
  { UNS_type index;
    for (index = 0;
         index < Cache_size;
         index++)
       Cache[index] = Main_Null; }

/*------------------------------ public functions ----------------------------*/

NIL_type Environment_Flush(NIL_type)
  { flush_cache(); }

UNS_type Environment_Get_Environment_size(NIL_type)
  { return (size_VEC(Current_environment) + 1); }
  
VEC_type Environment_Get_Environment(NIL_type)
  { return Current_environment; }

VEC_type Environment_Get_Frame(NIL_type)
  { return Current_frame; }

EXP_type Environment_Global_Get(UNS_type Scope,
                                UNS_type Offset)                      
  { EXP_type value;
    VEC_type frame;
    frame = Current_environment[Scope];
    value = frame[Offset];
    return value; }

BYT_type Environment_Global_Overflow(UNS_type Size)
  { return (Size >= Global_size); }

EXP_type Environment_Global_Set(UNS_type Scope,
                                UNS_type Offset,
                                EXP_type Value)
  { VEC_type frame;
    frame = Current_environment[Scope];
    frame[Offset] = Value; 
    return Value; }

VEC_type Environment_Grow_Environment(NIL_type)
  { UNS_type index,
            size;
    VEC_type environment,
             frame;
    size = size_VEC(Current_environment);
    environment = acquire_vector(++size);
    for (index = 1;
         index < size;
         index++)
      { frame = Current_environment[index];
        environment[index] = frame; }
    mark_VEC(Current_frame);
    environment[size] = Current_frame;
    return environment; }

NIL_type Environment_Initialize(NIL_type) 
  { Current_environment = Main_Empty_Vector;
    Global_frame = Current_frame = init_VEC(Global_size);
    flush_cache();
    Main_Register((REF_type)&Current_environment);
    Main_Register((REF_type)&Current_frame);
    Main_Register((REF_type)&Global_frame); }

EXP_type Environment_Local_Get(UNS_type Offset)
  { return Current_frame[Offset]; }

EXP_type Environment_Local_Set(UNS_type Offset,
                               EXP_type Value)
  { Current_frame[Offset] = Value;
    return Value; }

VEC_type Environment_Make_Frame(UNS_type Size)
  { return acquire_vector(Size); }

NIL_type Environment_Release_Frame(NIL_type)
  { release_vector(Current_frame); } 
  
NIL_type Environment_Release_Vector(VEC_type Vector)
  { release_vector(Vector); }

NIL_type Environment_Replace(VEC_type Environment,
                             VEC_type Frame)
  { Current_environment = Environment;
    Current_frame = Frame; }

NIL_type Environment_Rollback(NIL_type)
  { Current_environment = Main_Empty_Vector;
    Current_frame = Global_frame; }