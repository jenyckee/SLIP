                     /*-----------------------------------*/
                     /*             >>>Slip<<<            */
                     /*            Theo D'Hondt           */
                     /*     VUB Software Languages Lab    */
                     /*             (c) 2010              */
                     /*-----------------------------------*/
                     /*     version 12: smart caching     */
                     /*-----------------------------------*/
                     /*               main                */
                     /*-----------------------------------*/

#include "SlipGrammar.h"
        
/*------------------------------ public constants ----------------------------*/

enum { Main_Buffer_Size    = 256,
       Main_Default_Margin =  20 };

static const TXT_type Main_Anonymous_String   = "anonymous";
static const TXT_type Main_Application_String = "application";
static const TXT_type Main_Begin_String       = "begin";
static const TXT_type Main_Define_String      = "define";
static const TXT_type Main_If_String          = "if";
static const TXT_type Main_Lambda_String      = "lambda";
static const TXT_type Main_Quote_String       = "quote";
static const TXT_type Main_Set_String         = "set!";
static const TXT_type Main_While_String       = "while";

/*------------------------------ public prototypes ---------------------------*/

NIL_type                  Main_Claim(UNS_type);
EXP_type                  Main_Error(TXT_type);
EXP_type        Main_Error_Procedure(TXT_type,
                                     PRC_type);
EXP_type Main_Error_Procedure_Vararg(TXT_type,
                                     PRZ_type);
EXP_type           Main_Error_Symbol(TXT_type,
                                     SYM_type);
EXP_type              Main_Error_Tag(TXT_type,
                                     TAG_type);
EXP_type             Main_Error_Text(TXT_type,
                                     TXT_type);
NIL_type            Main_Fatal_Error(TXT_type);
TXT_type                   Main_Load(TXT_type);
NIL_type                  Main_Print(TXT_type);
TXT_type                   Main_Read(NIL_type);
UNS_type                Main_Reclaim(NIL_type);
NIL_type               Main_Register(REF_type);
TXT_type                   Main_Read(NIL_type);
PAI_type                Main_Reverse(PAI_type);
NIL_type         Main_Survival_Claim(REF_type,
                                     UNS_type);
EXP_type              Main_Terminate(NIL_type);

/*------------------------------ public variables ----------------------------*/

extern CHR_type Main_Buffer[];

extern SYM_type Main_Anonymous;
extern SYM_type Main_Begin;
extern SYM_type Main_Define;
extern ENV_type Main_Empty_Environment;
extern FRM_type Main_Empty_Frame;
extern GLB_type Main_Empty_Global;
extern PAI_type Main_Empty_Pair;
extern THR_type Main_Empty_Thread;
extern VEC_type Main_Empty_Vector;
extern FLS_type Main_False;
extern SYM_type Main_If;
extern SYM_type Main_Lambda;
extern NUL_type Main_Null;
extern NBR_type Main_One;
extern SYM_type Main_Quote;
extern SYM_type Main_Set;
extern TRU_type Main_True;
extern USP_type Main_Unspecified;
extern SYM_type Main_While;
extern NBR_type Main_Zero;