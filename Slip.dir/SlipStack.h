                     /*-----------------------------------*/
                     /*             >>>Slip<<<            */
                     /*            Theo D'Hondt           */
                     /*     VUB Software Languages Lab    */
                     /*             (c) 2010              */
                     /*-----------------------------------*/
                     /*     version 12: smart caching     */
                     /*-----------------------------------*/
                     /*               stack               */
                     /*-----------------------------------*/

/*------------------------------ public prototypes ---------------------------*/

NIL_type            Stack_Empty(NIL_type);
NIL_type       Stack_Initialize(NIL_type);
EXP_type             Stack_Peek(NIL_type);
NIL_type             Stack_Poke(EXP_type);
EXP_type              Stack_Pop(NIL_type);
EXP_type             Stack_Push(EXP_type);
NIL_type              Stack_Zap(NIL_type);
