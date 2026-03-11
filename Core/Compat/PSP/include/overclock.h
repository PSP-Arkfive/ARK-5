#ifndef OVERCLOCK_H
#define OVERCLOCK_H

void initOverclock();
void overclockHandler(int cpu, int bus);
void doOverclock();
int cancelOverclock();

#endif