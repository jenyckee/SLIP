                     /*-----------------------------------*/
                     /*             >>>Slip<<<            */
                     /*            Theo D'Hondt           */
                     /*     VUB Software Languages Lab    */
                     /*             (c) 2010              */
                     /*-----------------------------------*/
                     /*     version 12: smart caching     */
                     /*-----------------------------------*/
                     /*               memory              */
                     /*-----------------------------------*/

/*------------------------------ public types --------------------------------*/

typedef           void * ADR_type;
typedef  unsigned long   CEL_type;
typedef unsigned short   BYT_type;
typedef           char   CHR_type;
typedef         double   FLO_type;
typedef       long int   LNG_type;
typedef           void   NIL_type;
typedef    signed long   SGN_type;
typedef           char * TXT_type;
typedef  unsigned long   UNS_type;

typedef union PTR { CEL_type    cel;
                    union PTR * ptr; } * PTR_type;

/*------------------------------ public constants ----------------------------*/

enum { Memory_Cell_Bias         = 0x00000004,
       Memory_Immediate_Maximum = 0x3FFFFFFF,
       Memory_Void_Value        = 0x00000000 };

static const UNS_type Memory_Cell_Size = sizeof(CEL_type);

/*------------------------------ public prototypes ---------------------------*/

UNS_type      Memory_Available(NIL_type);
BYT_type          Memory_Claim(UNS_type);
PTR_type        Memory_Collect(PTR_type);
PTR_type           Memory_Copy(PTR_type);
LNG_type  Memory_Get_Immediate(PTR_type);
UNS_type       Memory_Get_Size(PTR_type);
BYT_type        Memory_Get_Tag(PTR_type);
NIL_type     Memory_Initialize(ADR_type,
                               UNS_type);
BYT_type   Memory_Is_Immediate(PTR_type);
PTR_type     Memory_Make_Chunk(BYT_type,
                               UNS_type);
PTR_type Memory_Make_Immediate(LNG_type);
NIL_type        Memory_Set_Tag(PTR_type,
                               BYT_type);