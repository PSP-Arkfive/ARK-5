#ifndef OVERCLOCK_H
#define OVERCLOCK_H

#define    DEFAULT_FREQUENCY        333
#define    THEORETICAL_FREQUENCY    463

extern int current_frequency;
extern void (*origSetClockFrequency)(int cpu, int bus);

void initOverclock();
void overclockHandler(int cpu, int bus);
void doOverclock();
int cancelOverclock();

#endif