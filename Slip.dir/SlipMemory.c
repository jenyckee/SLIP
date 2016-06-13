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

#include "SlipSlip.h"
#include "SlipMemory.h"

/*------------------------------ private constants ---------------------------*/

static const CEL_type Busy_mask      = 0x00000002;
static const CEL_type Free_mask      = 0xFFFFFFFD;
static const CEL_type Immediate_mask = 0x00000001;
static const UNS_type Size_max       = 0x00FFFFFF;
static const CEL_type Tag_mask       = 0x000000FC;
static const CEL_type Three_bit_mask = 0x00000007;
static const CEL_type Two_bit_mask   = 0x00000003;

enum { Busy_shift      = 1,
       Immediate_shift = 1,
       Size_shift      = 8,
       Tag_shift       = 2 };

enum { bp_bits = 0x0,
       bP_bits = 0x1,
       Bp_bits = 0x2,
       BP_bits = 0x3 };

enum { rbp_bits = 0x0,
       rBp_bits = 0x2,
       rBP_bits = 0x3,
       Rbp_bits = 0x4,
       RBp_bits = 0x6,
       RBP_bits = 0x7 };

/*------------------------------ private variables ---------------------------*/

static PTR_type Free_pointer;
static PTR_type Head_pointer;
static PTR_type Tail_pointer;

/*------------------------------ private functions ---------------------------*/

static CEL_type make_header(BYT_type Tag,
                            UNS_type Size)
  { return ((CEL_type)Size << Size_shift) | ((CEL_type)Tag << Tag_shift); }

static BYT_type is_busy(CEL_type Cell)
  { return (Cell & Busy_mask) >> Busy_shift; }

static CEL_type free_size(UNS_type Size)
  { return (CEL_type)Size << Size_shift; }

static BYT_type get_last_2_bits(CEL_type Cell)
  { return Cell & Two_bit_mask; }

static BYT_type get_last_3_bits(CEL_type Cell)
  { return Cell & Three_bit_mask; }

static UNS_type get_size(CEL_type Cell)
  { return (UNS_type)(Cell >> Size_shift); }

static CEL_type make_busy(PTR_type Pointer)
  { return (CEL_type)Pointer | Busy_mask; }

static PTR_type make_free(CEL_type Cell)
  { return (PTR_type)(Cell & Free_mask); }

static NIL_type sweep_and_mark(NIL_type)
  { CEL_type bits;
    PTR_type current,
             pointer;
    UNS_type size;
    for (current = Head_pointer;;)
      { bits = current->cel;
        switch (get_last_2_bits(bits))
          { case bp_bits:
              pointer = current->ptr;
              if ((pointer < Head_pointer) ||
                  (pointer > Tail_pointer))
                { current--;
                  continue; }
              bits = pointer->cel;
              switch (get_last_3_bits(bits))
                { case rbp_bits:
                    pointer->cel = make_busy(current);
                    current->cel = bits;
                    size = get_size(bits);
                    if (size)
                      current = pointer + size;
                    else
                      current--;
                    continue;
                  case rBp_bits:
                  case RBp_bits:
                    pointer = make_free(bits);
                    bits = pointer->cel;
                  case Rbp_bits:
                    pointer->cel = make_busy(current);
                    current->cel = bits;
                    current--; }
              continue;
            case Bp_bits:
              current = make_free(bits);
              if (current == Head_pointer)
                return;
            case bP_bits:
            case BP_bits:
              current--; }}}

static NIL_type traverse_and_update(NIL_type)
  { CEL_type bits;
    PTR_type destination,
             free,
             source,
             this;
    UNS_type accumulated_size,
             size;
    source = destination = Head_pointer + 1;
    while (source < Free_pointer)
      { bits = source->cel;
        if (is_busy(bits))
          { do
              { this = make_free(bits);
                bits = this->cel;
                this->ptr = destination; }
            while (is_busy(bits));
            source->cel = make_busy((PTR_type)bits);
            size = get_size(bits) + 1;
            source += size;
            destination += size; }
        else
          { free = source;
            accumulated_size = 0;
            do
              { size = get_size(bits) + 1;
                if ((accumulated_size + size) >= Size_max)
                  break;
                accumulated_size += size;
                source += size;
                if (source >= Free_pointer)
                  break;
                bits = source->cel; }
            while (!is_busy(bits));
            free->cel = free_size(accumulated_size - 1); }}}

static NIL_type traverse_and_compact(NIL_type)
  { CEL_type bits;
    PTR_type destination,
             source;
    UNS_type size;
    source = destination = Head_pointer + 1;
    while (source < Free_pointer)
      { bits = (source++)->cel;
        size = get_size(bits);
        if (is_busy(bits))
          { (destination++)->ptr = make_free(bits);
            while (size--)
              (destination++)->cel = (source++)->cel; }
        else
          source += size; }
    Free_pointer = destination; }

/*------------------------------ public functions ----------------------------*/

UNS_type Memory_Available(NIL_type)
  { return (Tail_pointer - Free_pointer); }

BYT_type Memory_Claim(UNS_type Margin)
  { return (Tail_pointer - Free_pointer < Margin); }

PTR_type Memory_Collect(PTR_type Root)
  { Head_pointer->ptr = Root;
    sweep_and_mark();
    traverse_and_update();
    traverse_and_compact();
    return Head_pointer->ptr; }

PTR_type Memory_Copy(PTR_type Pointer)
  { PTR_type pointer;
    UNS_type index,
             size;
    pointer = Free_pointer;
    size = Pointer->cel >> Size_shift;
    for (index = 0;
         index <= size;
         index++)
       (pointer + index)->cel = (Pointer + index)->cel;
    Free_pointer += size + 1;
    return pointer; }

LNG_type Memory_Get_Immediate(PTR_type Pointer)
  { SGN_type number;
    number = (SGN_type)Pointer;
    return number >> Immediate_shift; }

UNS_type Memory_Get_Size(PTR_type Pointer)
  { return Pointer->cel >> Size_shift; }

BYT_type Memory_Get_Tag(PTR_type Pointer)
  { return (Pointer->cel & Tag_mask) >> Tag_shift; }

NIL_type Memory_Initialize(ADR_type Address,
                           UNS_type Size)
  { Head_pointer = (PTR_type)Address;
    Free_pointer = Head_pointer + 1;
    Tail_pointer = Head_pointer + Size; }

BYT_type Memory_Is_Immediate(PTR_type Pointer)
  { CEL_type bits;
    bits = (CEL_type)Pointer;
    return (bits & Immediate_mask); }

PTR_type Memory_Make_Chunk(BYT_type Tag,
                           UNS_type Size)
  { PTR_type pointer;
    pointer = Free_pointer;
    Free_pointer->cel = make_header(Tag,
                                    Size);
    Free_pointer += Size + 1;
    return pointer; }

PTR_type Memory_Make_Immediate(LNG_type Long)
  { CEL_type bits;
    bits = ((CEL_type)Long << Immediate_shift) | Immediate_mask;
    return (PTR_type) bits; }
    
NIL_type Memory_Set_Tag(PTR_type Pointer,
                        BYT_type Tag)
  { UNS_type size;
    size = Pointer->cel >> Size_shift;
    Pointer->cel = make_header(Tag,
                               size); }