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

#include "SlipMain.h"
#include "SlipThread.h"

/*------------------------------ private constants ---------------------------*/

enum { Cache_size = 6,
       Table_size = 19 };

static const TXT_type TCT_error_string = "thread cache too small";
static const TXT_type TMT_error_string = "too many thread registrations";

/*------------------------------ private variables ---------------------------*/

static THR_type Cache[Cache_size];
static UNS_type Counter;
static THR_type Thread_Current;
static CCC_type Thread_Table[Table_size];

/*------------------------------ private functions ---------------------------*/

static NIL_type release_thread(THR_type Thread)
  { UNS_type thread_size;
    if (!marked_THR(Thread))
      { thread_size = size_THR(Thread);
        Thread->thr = Cache[thread_size];
        Cache[thread_size] = Thread; }}

static THR_type acquire_thread(UNS_type Thread_size,
                               NBR_type Thread_id,
                               THR_type Thread,
                               EXP_type Call_status)
  { THR_type thread;
    thread = Cache[Thread_size];
    if (is_NUL(thread))
      return make_THR(Thread_id,
                      Thread,
                      Call_status,
                      Thread_size);
    Cache[Thread_size] = thread->thr;
    thread->tid = Thread_id;
    thread->thr = Thread;
    thread->tcs = Call_status; 
    return thread; }

static NIL_type flush_cache(NIL_type)
  { UNS_type index;
    for (index = 0;
         index < Cache_size;
         index++)
       Cache[index] = Main_Empty_Thread; }

static NIL_type flush_thread(NIL_type)
  { THR_type next_thread,
             thread;
    for (thread = Thread_Current;
         !is_NUL(thread);
         thread = next_thread)
      { next_thread = thread->thr;
        release_thread(thread); }
    Thread_Current = Main_Empty_Thread; }

/*------------------------------ public functions ----------------------------*/

NIL_type Thread_Clear(NIL_type)
  { Thread_Current = Main_Empty_Thread; }

NIL_type Thread_Flush(NIL_type)
  { flush_cache(); }

NIL_type Thread_Initialize(NIL_type)
  { Thread_Clear();
    Thread_Flush();
    Counter = 0;
    Main_Register((REF_type)&Thread_Current); }
                          
THR_type Thread_Keep(NIL_type)
  { if (marked_THR(Thread_Current))
      Thread_Current = Clone_of(Thread_Current);
    return Thread_Current; }
    
THR_type Thread_Mark(NIL_type)
  { THR_type thread;
    for (thread = Thread_Current;
         !is_NUL(thread);
         thread = thread->thr)
      mark_THR(thread); 
    return Thread_Current; }

THR_type Thread_Patch(NBR_type Thread_id)
  { if (marked_THR(Thread_Current))
      Thread_Current = Clone_of(Thread_Current);
    Thread_Current->tid = Thread_id; 
    return Thread_Current; }

THR_type Thread_Peek(NIL_type)
  { return Thread_Current; }

THR_type Thread_Poke(NBR_type Thread_id,
                     EXP_type Call_status,
                     UNS_type Size)
  { THR_type thread;
    thread = Thread_Current->thr;
    release_thread(Thread_Current); 
    Thread_Current = acquire_thread(Size,
                                    Thread_id,
                                    thread,
                                    Call_status);
    return Thread_Current; }
        
THR_type Thread_Push(NBR_type Thread_id,
                     EXP_type Call_status,
                     UNS_type Size)
  { Thread_Current = acquire_thread(Size,
                                    Thread_id,
                                    Thread_Current,
                                    Call_status);
    return Thread_Current; }

NBR_type Thread_Register(CCC_type C_function,
                         UNS_type Thread_size)
  { NBR_type thread_id;
    if (Counter == Table_size)
      Main_Fatal_Error(TMT_error_string);
    if (Thread_size >= Cache_size)
      Main_Fatal_Error(TCT_error_string);
    Thread_Table[Counter] = C_function;
    thread_id = make_NBR(Counter++);
    return thread_id; }

NIL_type Thread_Replace(THR_type Thread)
  { flush_thread();        
    Thread_Current = Thread; }

CCC_type Thread_Retrieve(NBR_type Thread_id)
  { UNS_type raw_thread_id;
    raw_thread_id = get_NBR(Thread_id);
    return Thread_Table[raw_thread_id]; }

NIL_type Thread_Zap(NIL_type)
  { THR_type thread;
    thread = Thread_Current;
    Thread_Current = thread->thr;
    release_thread(thread); }
