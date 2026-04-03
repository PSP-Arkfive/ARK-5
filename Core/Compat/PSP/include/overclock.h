#ifndef OVERCLOCK_H
#define OVERCLOCK_H

extern int overclock_enabled;
extern void (*origSetClockFrequency)(int cpu, int bus);

void initOverclock();
void overclockHandler(int cpu, int bus);
void doOverclock();
int cancelOverclock();

#endif