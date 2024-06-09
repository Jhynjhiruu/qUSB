#ifndef _QUEUE_H
#define _QUEUE_H

#include <ultra64.h>

#define MESG_BUF_SIZE (200)

extern OSMesgQueue pi_mesg_queue;
extern OSMesg pi_mesg_buf[MESG_BUF_SIZE];

extern OSMesgQueue si_mesg_queue;
extern OSMesg si_mesg_buf[MESG_BUF_SIZE];

extern OSMesgQueue vi_mesg_queue;
extern OSMesg vi_mesg_buf[1];
extern OSMesg vi_retrace_mesg;

extern OSMesgQueue controller_mesg_queue;

#endif