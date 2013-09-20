#ifndef _THREAD_CONNECT_H
#define _THREAD_CONNECT_H

#define _REENTRANT

#include <pthread.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <mysql/mysql.h>
#include <unistd.h>

#include "main.h"

void* threadConnect(void *arg);

#endif
