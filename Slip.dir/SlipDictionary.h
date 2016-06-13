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

/*------------------------------ public prototypes ---------------------------*/

NIL_type         Dictionary_Checkpoint(NIL_type);
UNS_type             Dictionary_Define(SYM_type); 
NIL_type Dictionary_Enter_Nested_Scope(NIL_type); 
UNS_type  Dictionary_Exit_Nested_Scope(NIL_type);
UNS_type     Dictionary_Get_Frame_Size(NIL_type);
NIL_type         Dictionary_Initialize(NIL_type);
UNS_type   Dictionary_Initially_Define(SYM_type); 
UNS_type    Dictionary_Lexical_Address(SYM_type,
                                       URF_type);
NIL_type           Dictionary_Rollback(NIL_type); 
