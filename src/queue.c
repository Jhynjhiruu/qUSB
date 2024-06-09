#include <ultra64.h>

#include "queue.h"

OSMesgQueue pi_mesg_queue;
OSMesg pi_mesg_buf[MESG_BUF_SIZE];

OSMesgQueue si_mesg_queue;
OSMesg si_mesg_buf[MESG_BUF_SIZE];

OSMesgQueue vi_mesg_queue;
OSMesg vi_mesg_buf[1];
OSMesg vi_retrace_mesg;

OSMesgQueue controller_mesg_queue;