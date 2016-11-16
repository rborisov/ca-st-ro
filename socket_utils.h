#ifndef __MNTSRV_UTILS_H__
#define __MNTSRV_UTILS_H__

#define _MNTPORT_ 9213

int srv_init();
void srv_close();
void mainthread(void);

#endif
