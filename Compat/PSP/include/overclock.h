#ifndef OVERCLOCK_H
#define OVERCLOCK_H

#define    DEFAULT_FREQUENCY        333
#define    MAX_ALLOWED_FREQUENCY    483

extern int currFreq, targetFreq;
extern void (*origSetClockFrequency)(int cpu, int bus);

void initOverclock();
void overclockHandler(int cpu, int bus);
void doOverclock();
void cancelOverclock();

#endif