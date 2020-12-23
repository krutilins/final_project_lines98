#ifndef RANDOM_H
#define RANDOM_H

#include <ctime>

#define random(num) (int)(((long)rand()*(num))/(RAND_MAX+1))
#define randomize() srand((unsigned)time(NULL))

#endif //RANDOM_H	