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

/*------------------------------ public prototypes ---------------------------*/

NIL_type                Environment_Flush(NIL_type);
VEC_type      Environment_Get_Environment(NIL_type);
UNS_type Environment_Get_Environment_size(NIL_type);
VEC_type            Environment_Get_Frame(NIL_type);
EXP_type           Environment_Global_Get(UNS_type,
                                          UNS_type);
BYT_type      Environment_Global_Overflow(UNS_type);
EXP_type           Environment_Global_Set(UNS_type,
                                          UNS_type,
                                          EXP_type);
VEC_type     Environment_Grow_Environment(NIL_type);
NIL_type           Environment_Initialize(NIL_type);
EXP_type            Environment_Local_Get(UNS_type);
EXP_type            Environment_Local_Set(UNS_type,
                                          EXP_type);
VEC_type           Environment_Make_Frame(UNS_type);
VEC_type          Environment_Make_Frame(UNS_type);
NIL_type        Environment_Release_Frame(NIL_type);
NIL_type       Environment_Release_Vector(VEC_type);
NIL_type              Environment_Replace(VEC_type,
                                          VEC_type);
NIL_type             Environment_Rollback(NIL_type);


