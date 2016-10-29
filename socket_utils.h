#ifndef __MNTSRV_UTILS_H__
#define __MNTSRV_UTILS_H__

#define _MNTPORT_ 9213

int mntsrv_init();
void mntsrv_poll();
void mntsrv_close();

#endif
