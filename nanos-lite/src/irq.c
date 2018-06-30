#include "common.h"
extern _RegSet* do_syscall(_RegSet *r);
extern _RegSet* schedule(_RegSet *prev);

static _RegSet* do_event(_Event e, _RegSet* r) {
   _RegSet *ret = NULL;
  //  panic(" qqqqqqqqq = %d", e.event);
  switch (e.event) {
    case _EVENT_SYSCALL:{
      do_syscall(r);
      ret =schedule(r);
      break;
    }
    case _EVENT_TRAP:{
      ret=schedule(r);
      break;
    }
    case _EVENT_IRQ_TIME:{
      // Log("this is event_irq_time \n");
      ret=schedule(r);
      break;
    }
    default: panic("Unhandled event ID = %d", e.event);
  }

  return ret;
}

void init_irq(void) {
  _asye_init(do_event);
}
