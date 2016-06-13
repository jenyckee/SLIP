                     /*-----------------------------------*/
                     /*             >>>Slip<<<            */
                     /*            Theo D'Hondt           */
                     /*     VUB Software Languages Lab    */
                     /*             (c) 2010              */
                     /*-----------------------------------*/
                     /*     version 12: smart caching     */
                     /*-----------------------------------*/
                     /*               grammar             */
                     /*-----------------------------------*/

#include "SlipMemory.h"

/*------------------------------ public types --------------------------------*/

typedef enum { APL_tag = 0x00<<1 | 0x0,                     /* 00 */
               BEG_tag = 0x01<<1 | 0x0,                     /* 02 */
               CNT_tag = 0x02<<1 | 0x0,                     /* 04 */
               DFF_tag = 0x03<<1 | 0x0,                     /* 06 */
               DFV_tag = 0x04<<1 | 0x0,                     /* 08 */
               DFZ_tag = 0x05<<1 | 0x0,                     /* 10 */
               ENV_tag = 0x06<<1 | 0x0,                     /* 12 */
               FRM_tag = 0x07<<1 | 0x0,                     /* 14 */
               GLB_tag = 0x08<<1 | 0x0,                     /* 16 */
               IFF_tag = 0x09<<1 | 0x0,                     /* 18 */
               IFZ_tag = 0x0A<<1 | 0x0,                     /* 20 */
               LCL_tag = 0x0B<<1 | 0x0,                     /* 22 */
               LMB_tag = 0x0C<<1 | 0x0,                     /* 24 */
               LMZ_tag = 0x0D<<1 | 0x0,                     /* 26 */
               PAI_tag = 0x0E<<1 | 0x0,                     /* 28 */
               PRC_tag = 0x0F<<1 | 0x0,                     /* 30 */
               PRZ_tag = 0x10<<1 | 0x0,                     /* 32 */
               QUO_tag = 0x11<<1 | 0x0,                     /* 34 */
               SEQ_tag = 0x12<<1 | 0x0,                     /* 36 */
               STG_tag = 0x13<<1 | 0x0,                     /* 38 */
               STL_tag = 0x14<<1 | 0x0,                     /* 40 */
               THK_tag = 0x15<<1 | 0x0,                     /* 42 */
               THR_tag = 0x16<<1 | 0x0,                     /* 44 */
               THr_tag = 0x17<<1 | 0x0,                     /* 46 */
               VEC_tag = 0x18<<1 | 0x0,                     /* 48 */
               VEc_tag = 0x19<<1 | 0x0,                     /* 50 */
               WHI_tag = 0x1A<<1 | 0x0,                     /* 52 */

               CHA_tag = 0x00<<1 | 0x1,                     /* 01 */
               FLS_tag = 0x01<<1 | 0x1,                     /* 03 */
               NAT_tag = 0x02<<1 | 0x1,                     /* 05 */
               NUL_tag = 0x03<<1 | 0x1,                     /* 07 */
               NBR_tag = 0x04<<1 | 0x1,                     /* 09 */
               REA_tag = 0x05<<1 | 0x1,                     /* 11 */
               STR_tag = 0x06<<1 | 0x1,                     /* 13 */
               SYM_tag = 0x07<<1 | 0x1,                     /* 15 */
               TRU_tag = 0x08<<1 | 0x1,                     /* 17 */
               USP_tag = 0x09<<1 | 0x1 } TAG_type;          /* 19 */

typedef struct APL * APL_type;
typedef struct BEG * BEG_type;
typedef struct CHA * CHA_type;
typedef struct CNT * CNT_type;
typedef struct DFF * DFF_type;
typedef struct DFV * DFV_type;
typedef struct DFZ * DFZ_type;
typedef struct ENV * ENV_type;
typedef struct FRM * FRM_type;
typedef struct GLB * GLB_type;
typedef struct IFF * IFF_type;
typedef struct IFZ * IFZ_type;
typedef struct LCL * LCL_type;
typedef struct LMB * LMB_type;
typedef struct LMZ * LMZ_type;
typedef struct NAT * NAT_type;
typedef struct PAI * PAI_type;
typedef struct QUO * QUO_type;
typedef struct PRC * PRC_type;
typedef struct PRZ * PRZ_type;
typedef struct REA * REA_type;
typedef struct STG * STG_type;
typedef struct STL * STL_type;
typedef struct STR * STR_type;
typedef struct SYM * SYM_type;
typedef struct THK * THK_type;
typedef struct THR * THR_type;
typedef struct WHI * WHI_type;

typedef NIL_type * EXP_type;

typedef EXP_type   FLS_type;
typedef EXP_type   NBR_type;
typedef EXP_type   NUL_type;
typedef EXP_type * REF_type;
typedef EXP_type * SEQ_type;
typedef EXP_type   TRU_type;
typedef EXP_type   USP_type;
typedef EXP_type * VEC_type;
typedef EXP_type * VEc_type;

typedef THR_type * CRF_type;

typedef UNS_type * URF_type;

typedef EXP_type (* CCC_type) (EXP_type, EXP_type);
typedef EXP_type (* FUN_type) (VEC_type, EXP_type);

/*-------------------------------- public prototypes -------------------------*/

TAG_type   Tag_of(EXP_type);
EXP_type Clone_of(EXP_type);

/*------------------------------ public declarations -------------------------*/

typedef
  struct APL { CEL_type hdr;
               EXP_type exp;
               VEC_type opr; } APL;
BYT_type   is_APL(EXP_type);
APL_type make_APL(EXP_type,
                  VEC_type);

typedef
  struct BEG { CEL_type hdr;
               EXP_type seq; } BEG;
BYT_type   is_BEG(EXP_type);
BEG_type make_BEG(EXP_type);

typedef
  struct CHA { CEL_type hdr;
               CHR_type chr; } CHA;
BYT_type   is_CHA(EXP_type);
CHA_type make_CHA(CHR_type);

typedef
  struct CNT { CEL_type hdr;
               VEC_type env;
               VEC_type frm;
               THR_type thr; } CNT;
BYT_type   is_CNT(EXP_type);
CNT_type make_CNT(VEC_type,
                  VEC_type,
                  THR_type);
                  
typedef
  struct DFF { CEL_type hdr;
               SYM_type nam;
               NBR_type ofs;
               NBR_type par;
               NBR_type siz;
               EXP_type bod; } DFF;
BYT_type   is_DFF(EXP_type);
DFF_type make_DFF(SYM_type,
                  NBR_type,
                  NBR_type,
                  NBR_type,
                  EXP_type);

typedef
  struct DFV { CEL_type hdr;
               NBR_type ofs;
               EXP_type exp; } DFV;
BYT_type   is_DFV(EXP_type);
DFV_type make_DFV(NBR_type,
                  EXP_type);

typedef
  struct DFZ { CEL_type hdr;
               SYM_type nam;
               NBR_type ofs;
               NBR_type par;
               NBR_type siz;
               EXP_type bod; } DFZ;
BYT_type   is_DFZ(EXP_type);
DFZ_type make_DFZ(SYM_type,
                  NBR_type,
                  NBR_type,
                  NBR_type,
                  EXP_type);

typedef
  struct ENV { CEL_type hdr;
               FRM_type frm;
               NBR_type siz;
               ENV_type env; } ENV;
BYT_type   is_ENV(EXP_type);
ENV_type make_ENV(FRM_type,
                  NBR_type,
                  ENV_type);

BYT_type   is_FLS(EXP_type);
FLS_type make_FLS(NIL_type);

typedef
  struct FRM { CEL_type hdr;
               SYM_type var;
               FRM_type frm; } FRM;
BYT_type   is_FRM(EXP_type);
FRM_type make_FRM(SYM_type,
                  FRM_type);

typedef
  struct GLB { CEL_type hdr;
               NBR_type scp;
               NBR_type ofs; } GLB;
BYT_type   is_GLB(EXP_type);
GLB_type make_GLB(NBR_type,
                  NBR_type);

typedef
  struct IFF { CEL_type hdr;
               EXP_type prd;
               EXP_type cns;
               EXP_type alt; } IFF;
BYT_type   is_IFF(EXP_type);
IFF_type make_IFF(EXP_type,
                  EXP_type,
                  EXP_type);

typedef
  struct IFZ { CEL_type hdr;
               EXP_type prd;
               EXP_type cns; } IFZ;
BYT_type   is_IFZ(EXP_type);
IFZ_type make_IFZ(EXP_type,
                  EXP_type);

typedef
  struct LCL { CEL_type hdr;
               NBR_type ofs; } LCL;
BYT_type   is_LCL(EXP_type);
LCL_type make_LCL(NBR_type);

typedef
  struct LMB { CEL_type hdr;
               NBR_type par;
               NBR_type siz;
               EXP_type bod; } LMB;
BYT_type   is_LMB(EXP_type);
LMB_type make_LMB(NBR_type,
                  NBR_type,
                  EXP_type);

typedef
  struct LMZ { CEL_type hdr;
               NBR_type par;
               NBR_type siz;
               EXP_type bod; } LMZ;
BYT_type   is_LMZ(EXP_type);
LMZ_type make_LMZ(NBR_type,
                  NBR_type,
                  EXP_type);

typedef
  struct NAT { CEL_type hdr;
               FUN_type fun;
               CHR_type nam[]; } NAT;
BYT_type   is_NAT(EXP_type);
NAT_type make_NAT(FUN_type,
                  TXT_type);

BYT_type   is_NBR(EXP_type);
NBR_type make_NBR(LNG_type);
LNG_type  get_NBR(NBR_type);

BYT_type   is_NUL(EXP_type);
NUL_type make_NUL(NIL_type);

typedef
  struct PAI { CEL_type hdr;
               EXP_type car;
               EXP_type cdr; } PAI;
BYT_type   is_PAI(EXP_type);
PAI_type make_PAI(EXP_type,
                  EXP_type);

typedef
  struct PRC { CEL_type hdr;
               SYM_type nam;
               NBR_type par;
               NBR_type siz;
               EXP_type bod;
               VEC_type env; } PRC;
BYT_type   is_PRC(EXP_type);
PRC_type make_PRC(SYM_type,
                  NBR_type,
                  NBR_type,
                  EXP_type,
                  VEC_type);

typedef
  struct PRZ { CEL_type hdr;
               SYM_type nam;
               NBR_type par;
               NBR_type siz;
               EXP_type bod;
               VEC_type env; } PRZ;
BYT_type   is_PRZ(EXP_type);
PRZ_type make_PRZ(SYM_type,
                  NBR_type,
                  NBR_type,
                  EXP_type,
                  VEC_type);

typedef
  struct QUO { CEL_type hdr;
               EXP_type exp; } QUO;
BYT_type   is_QUO(EXP_type);
QUO_type make_QUO(EXP_type);

typedef
  struct REA { CEL_type hdr;
               FLO_type flo; } REA;
BYT_type   is_REA(EXP_type);
REA_type make_REA(FLO_type);

BYT_type   is_SEQ(EXP_type);
SEQ_type make_SEQ(UNS_type);
UNS_type size_SEQ(VEC_type);

typedef
  struct STG { CEL_type hdr;
               NBR_type scp;
               NBR_type ofs;
               EXP_type exp; } STG;
BYT_type   is_STG(EXP_type);
STG_type make_STG(NBR_type,
                  NBR_type,
                  EXP_type);

typedef
  struct STL { CEL_type hdr;
               NBR_type ofs;
               EXP_type exp; } STL;
BYT_type   is_STL(EXP_type);
STL_type make_STL(NBR_type,
                  EXP_type);

typedef
  struct STR { CEL_type hdr;
               CHR_type txt[]; } STR;
BYT_type   is_STR(EXP_type);
STR_type make_STR(TXT_type);
UNS_type size_STR(TXT_type);

typedef
  struct SYM { CEL_type hdr;
               CHR_type txt[]; } SYM;
BYT_type   is_SYM(EXP_type);
SYM_type make_SYM(TXT_type);
UNS_type size_SYM(TXT_type);

typedef
  struct THK { CEL_type hdr;
               EXP_type exp;
               NBR_type siz; } THK;
BYT_type   is_THK(EXP_type);
THK_type make_THK(EXP_type,
                  NBR_type);

typedef
  struct THR { CEL_type hdr;
               NBR_type tid;
               THR_type thr;
               EXP_type tcs;
               EXP_type exp[]; } THR;
BYT_type   is_THR(EXP_type);
BYT_type marked_THR(THR_type);
THR_type make_THR(NBR_type,
                  THR_type,
                  EXP_type,
                  UNS_type);
UNS_type size_THR(THR_type);
NIL_type mark_THR(THR_type);

BYT_type   is_TRU(EXP_type);
TRU_type make_TRU(NIL_type);

BYT_type   is_USP(EXP_type);
USP_type make_USP(NIL_type);

BYT_type   is_VEC(EXP_type);
BYT_type marked_VEC(VEC_type);
VEC_type make_VEC(UNS_type);
VEC_type init_VEC(UNS_type);
UNS_type size_VEC(VEC_type);
NIL_type mark_VEC(VEC_type);

typedef
  struct WHI { CEL_type hdr;
               EXP_type prd;
               EXP_type bod; } WHI;
BYT_type   is_WHI(EXP_type);
WHI_type make_WHI(EXP_type,
                  EXP_type);
