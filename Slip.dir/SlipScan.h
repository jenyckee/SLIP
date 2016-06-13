                     /*-----------------------------------*/
                     /*             >>>Slip<<<            */
                     /*            Theo D'Hondt           */
                     /*     VUB Software Languages Lab    */
                     /*             (c) 2010              */
                     /*-----------------------------------*/
                     /*     version 12: smart caching     */
                     /*-----------------------------------*/
                     /*              scanner              */
                     /*-----------------------------------*/

/*------------------------------ public types --------------------------------*/

typedef enum { CHA_token =  0,                           /* character         */
               END_token =  1,                           /* eof               */
               FLS_token =  2,                           /* false             */
               IDT_token =  3,                           /* identifier        */
               LBR_token =  4,                           /* left bracket      */
               LPR_token =  5,                           /* left parenthesis  */
               NBR_token =  6,                           /* number            */
               PER_token =  7,                           /* period            */
               QUO_token =  8,                           /* quote             */
               RBR_token =  9,                           /* right bracket     */
               RPR_token = 10,                           /* right parenthesis */
               REA_token = 11,                           /* real              */
               STR_token = 12,                           /* string            */
               TRU_token = 13 } SCA_type;                /* true              */

/*------------------------------ public prototypes ---------------------------*/

NIL_type Scan_Initialize(NIL_type);
SCA_type       Scan_Next(NIL_type);
NIL_type     Scan_Preset(TXT_type);

/*------------------------------ public variables ----------------------------*/

extern CHR_type Scan_String[];
