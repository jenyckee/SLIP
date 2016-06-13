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

/*------------------------------ public prototypes ---------------------------*/

EXP_type      Evaluate_Apply(VEC_type,
                             EXP_type);
EXP_type   Evaluate_Continue(EXP_type);
EXP_type   Evaluate_Evaluate(EXP_type);
NIL_type Evaluate_Initialize(NIL_type);