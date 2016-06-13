                     /*-----------------------------------*/
                     /*             >>>Slip<<<            */
                     /*            Theo D'Hondt           */
                     /*     VUB Software Languages Lab    */
                     /*             (c) 2010              */
                     /*-----------------------------------*/
                     /*     version 12: smart caching     */
                     /*-----------------------------------*/
                     /*              threads              */
                     /*-----------------------------------*/

/*------------------------------ public prototypes ---------------------------*/

NIL_type      Thread_Clear(NIL_type);
NIL_type      Thread_Flush(NIL_type);
NIL_type Thread_Initialize(NIL_type);
THR_type       Thread_Keep(NIL_type);
THR_type       Thread_Mark(NIL_type);
THR_type      Thread_Patch(NBR_type);
THR_type       Thread_Peek(NIL_type);
THR_type       Thread_Poke(NBR_type, 
                           EXP_type,
                           UNS_type);
THR_type       Thread_Push(NBR_type, 
                           EXP_type,
                           UNS_type);
NBR_type   Thread_Register(CCC_type,
                           UNS_type);
NIL_type    Thread_Replace(THR_type);
CCC_type   Thread_Retrieve(NBR_type);
NIL_type        Thread_Zap(NIL_type);