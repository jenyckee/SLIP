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

#include <string.h>

#include "SlipMain.h"

#include "SlipScan.h"

/*------------------------------ private types ---------------------------*/

typedef enum { Apo =  0,       /* ' */
               Bks =  1,       /* \ */
               Dgt =  2,       /* 0 1 2 3 4 5 6 7 8 9 */
               Eol =  3,       /*   */
               Exp =  4,       /* e E */
               Fls =  5,       /* f F */
               Hsh =  6,       /* # */
               Ill =  7,       /*   */
               Opr =  8,       /* ! $ % & * / : < = > ? ^ _ ~ */
               Lbr =  9,       /* [ */
               Lpr = 10,       /* ( */
               Ltr = 11,       /* a A b B ... z Z */
               Mns = 12,       /* - */
               Nul = 13,       /*   */
               Per = 14,       /* . */
               Pls = 15,       /* + */
               Quo = 16,       /* " */
               Rbr = 17,       /* ] */
               Rpr = 18,       /* ) */
               Smc = 19,       /* ; */
               Tru = 20,       /* t T */
               Wsp = 21 } CAT_type;

typedef enum { bks_allowed = (1<<Bks),
               del_allowed = (1<<Eol)+
                             (1<<Lpr)+
                             (1<<Nul)+
                             (1<<Quo)+
                             (1<<Rpr)+
                             (1<<Smc)+
                             (1<<Wsp),
               dig_allowed = (1<<Dgt),
               end_allowed = (1<<Eol)+
                             (1<<Nul),
               esc_allowed = (1<<Bks)+
                             (1<<Quo),
               exp_allowed = (1<<Exp),
               fls_allowed = (1<<Fls),
               idt_allowed = (1<<Dgt)+
                             (1<<Exp)+
                             (1<<Fls)+
                             (1<<Ltr)+
                             (1<<Mns)+
                             (1<<Opr)+
                             (1<<Pls)+
                             (1<<Tru),
               lpr_allowed = (1<<Lpr),
               per_allowed = (1<<Per),
               quo_allowed = (1<<Quo),
               sgn_allowed = (1<<Mns)+
                             (1<<Pls),
               trm_allowed = (1<<Eol)+
                             (1<<Nul)+
                             (1<<Quo),
               tru_allowed = (1<<Tru),
               wsp_allowed = (1<<Eol)+
                             (1<<Wsp) } ALL_type;

/*------------------------------ private constants ---------------------------*/

enum { Buffer_size = 1024 };

static const TXT_type Newline_ch       = "\n";
static const TXT_type Newline_txt      = "newline";
static const TXT_type Space_ch         = " ";
static const TXT_type Space_txt        = "space";

static const TXT_type BFO_error_string = "buffer overflow";
static const TXT_type DRQ_error_string = "digit required";
static const TXT_type ICL_error_string = "invalid character literal";
static const TXT_type ILC_error_string = "illegal character";
static const TXT_type ILH_error_string = "illegal use of #";
static const TXT_type QRQ_error_string = "quote required";

static const CAT_type ASCII_Table[] =
      /*NUL SOH STX ETX EOT ENQ ACK BEL BS  HT  LF  VT  FF  CR  SO  SI */
      { Nul,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Wsp,Eol,Ill,Wsp,Eol,Ill,Ill,
      /*DLE DC1 DC2 DC3 DC4 NAK SYN ETB CAN EM  SUB ESC FS  GS  RS  US */
        Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,
      /*     !   "   #   $   %   &   '   (   )   *   +   ,   -   .   / */
        Wsp,Opr,Quo,Hsh,Opr,Opr,Opr,Apo,Lpr,Rpr,Opr,Pls,Ill,Mns,Per,Opr,
      /* 0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ? */
        Dgt,Dgt,Dgt,Dgt,Dgt,Dgt,Dgt,Dgt,Dgt,Dgt,Opr,Smc,Opr,Opr,Opr,Opr,
      /* @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O */
        Ill,Ltr,Ltr,Ltr,Ltr,Exp,Fls,Ltr,Ltr,Ltr,Ltr,Ltr,Ltr,Ltr,Ltr,Ltr,
      /* P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _ */
        Ltr,Ltr,Ltr,Ltr,Tru,Ltr,Ltr,Ltr,Ltr,Ltr,Ltr,Lbr,Bks,Rbr,Opr,Opr,
      /* `   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o */
        Ill,Ltr,Ltr,Ltr,Ltr,Exp,Fls,Ltr,Ltr,Ltr,Ltr,Ltr,Ltr,Ltr,Ltr,Ltr,
      /* p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~  DEL*/
        Ltr,Ltr,Ltr,Ltr,Tru,Ltr,Ltr,Ltr,Ltr,Ltr,Ltr,Ill,Ill,Ill,Opr,Ill,
      /*                                                               */
        Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,
      /*                                                               */
        Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,
      /*                                                               */
        Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,
      /*                                                               */
        Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,
      /*                                                               */
        Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,
      /*                                                               */
        Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,
      /*                                                               */
        Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,
      /*                                                               */
        Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill,Ill };

/*------------------------------ private variables ---------------------------*/

static TXT_type Input_String;
static UNS_type Cursor;
static UNS_type Input_Position;
static UNS_type Input_Length;

/*------------------------------ public variables ----------------------------*/

CHR_type Scan_String[Buffer_size];

/*------------------------------ private functions ---------------------------*/

static SCA_type error_and_return(TXT_type Error)
  { Main_Error(Error);
    return END_token; }

static CHR_type current_character(NIL_type)
  { CHR_type character;
    character = Input_String[Input_Position];
		return character; }

static BYT_type check(ALL_type Allowed)
  { CHR_type character;
    character = current_character();
		return (Allowed >> ASCII_Table[character]) & 1; }

static NIL_type next_character(NIL_type)
  { if (Input_Position < Input_Length)
      Input_Position++; }

static SCA_type next_character_return(SCA_type Token)
  { next_character();
    return Token; }

static NIL_type copy_char(CHR_type Character)
  { if (Cursor == Buffer_size)
      Main_Error(BFO_error_string);
    Scan_String[Cursor++] = Character; }

static NIL_type copy_and_get_char(NIL_type)
  { CHR_type character;
    character = current_character();
		copy_char(character);
    next_character(); }

static NIL_type get_while(ALL_type Allowed)
  { do
      next_character();
    while (check(Allowed)); }

static NIL_type get_until(ALL_type Allowed)
  { do
      next_character();
    while (!check(Allowed)); }

static NIL_type copy_and_get_while(ALL_type Allowed)
  { do
      copy_and_get_char();
    while (check(Allowed)); }

static NIL_type copy_and_get_until(ALL_type Allowed)
  { do
      copy_and_get_char();
    while (!check(Allowed)); }

static NIL_type stop_copy_text(NIL_type)
  { copy_char(0); }

static SCA_type stop_copy_text_return(SCA_type Token)
  { stop_copy_text();
    return Token; }

static BYT_type reserved_name(TXT_type Text)
  { return strcmp(Scan_String,
                  Text); }

static SCA_type character(NIL_type)
  { next_character();
    copy_and_get_until(del_allowed);
    stop_copy_text();
    if (reserved_name(Space_txt))
      strcpy(Scan_String,
             Space_ch);
    else if (reserved_name(Newline_txt))
      strcpy(Scan_String,
             Newline_ch);
    else if (strlen(Scan_String) != 1)
      return error_and_return(ICL_error_string);
    return CHA_token; }

static NIL_type integer(NIL_type)
  { if (!check(dig_allowed))
      Main_Error(DRQ_error_string);
    copy_and_get_while(dig_allowed); }

static SCA_type number(NIL_type)
  { SCA_type token;
    token = NBR_token;
    copy_and_get_while(dig_allowed);
    if (check(per_allowed))
      { token = REA_token;
        copy_and_get_char();
        integer(); }
    if (check(exp_allowed))
      { token = REA_token;
        copy_and_get_char();
        if (check(sgn_allowed))
          copy_and_get_char();
        integer(); }
    return stop_copy_text_return(token); }

static SCA_type Apo_fun(NIL_type)
  { return next_character_return(QUO_token); }

static SCA_type Hsh_fun(NIL_type)
  { next_character();
    if (check(tru_allowed))
      return next_character_return(TRU_token);
    if (check(fls_allowed))
      return next_character_return(FLS_token);
    if (check(bks_allowed))
      return character();
    return error_and_return(ILH_error_string); }

static SCA_type Idt_fun(NIL_type)
  { copy_and_get_while(idt_allowed);
    return stop_copy_text_return(IDT_token); }

static SCA_type Ill_fun(NIL_type)
  { return error_and_return(ILC_error_string); }

static SCA_type Lbr_fun(NIL_type)
  { return next_character_return(LBR_token); }

static SCA_type Lpr_fun(NIL_type)
  { return next_character_return(LPR_token); }

static SCA_type Nbr_fun(NIL_type)
  { return number(); }

static SCA_type Nul_fun(NIL_type)
  { return END_token; }

static SCA_type Per_fun(NIL_type)
  { return next_character_return(PER_token); }

static SCA_type Quo_fun(NIL_type)
  { next_character();
    while (!check(trm_allowed))
      copy_and_get_char();
    stop_copy_text();
    if (!check(quo_allowed))
      return error_and_return(QRQ_error_string);
    return next_character_return(STR_token); }

static SCA_type Rbr_fun(NIL_type)
  { return next_character_return(RBR_token); }

static SCA_type Rpr_fun(NIL_type)
  { return next_character_return(RPR_token); }

static SCA_type Sgn_fun(NIL_type)
  { copy_and_get_char();
    if (check(dig_allowed))
      return number();
    while (check(idt_allowed))
      copy_and_get_char();
    return stop_copy_text_return(IDT_token); }

static SCA_type Smc_fun(NIL_type)
  { get_until(end_allowed);
    next_character();
    return Scan_Next(); }

static SCA_type Wsp_fun(NIL_type)
  { get_while(wsp_allowed);
    return Scan_Next(); }

/*------------------------------ public functions ----------------------------*/

NIL_type Scan_Initialize(NIL_type)
  { return; }

SCA_type Scan_Next(NIL_type)
  { CAT_type category;
		CHR_type character;
    character = current_character();
    Cursor = 0;
    Scan_String[0] = 0;
    category = ASCII_Table[character];
    switch (category)
      { case Apo: return Apo_fun();
        case Bks: return Ill_fun();
        case Dgt: return Nbr_fun();
        case Eol: return Wsp_fun();
        case Exp: return Idt_fun();
        case Fls: return Idt_fun();
        case Hsh: return Hsh_fun();
        case Ill: return Ill_fun();
        case Opr: return Idt_fun();
        case Lpr: return Lpr_fun();
        case Lbr: return Lbr_fun();
        case Ltr: return Idt_fun();
        case Mns: return Sgn_fun();
        case Nul: return Nul_fun();
        case Per: return Per_fun();
        case Pls: return Sgn_fun();
        case Quo: return Quo_fun();
        case Rbr: return Rbr_fun();
        case Rpr: return Rpr_fun();
        case Smc: return Smc_fun();
        case Tru: return Idt_fun();
        case Wsp: return Wsp_fun(); }
    return Nul_fun(); }

NIL_type Scan_Preset(TXT_type Input)
  { Input_String = Input;
    Input_Position = 0;
    Input_Length = strlen(Input_String); }