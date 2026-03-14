#ifndef _PIZZA_H_
#define _PIZZA_H_

#define N_TEL 2
#define N_COOK 2
#define N_OVEN 10
#define N_DELIVERER 10
#define N_ORDER_LOW 1
#define N_ORDER_HIGH 5

#define T_ORDER_LOW 1
#define T_ORDER_HIGH 5
#define T_PAYMENT_LOW 1
#define T_PAYMENT_HIGH 3

#define T_PREP 1
#define T_BAKE 10
#define T_PACK 1
#define T_DEL_LOW 5
#define T_DEL_HIGH 15

#define P_M 35
#define P_P 25
#define P_S 40

#define P_FAIL 5

#define C_M 10
#define C_P 11
#define C_S 12

#define NSEC_IN_MIN 6000000000L

#include <stdint.h>
#include <time.h>
#include <pthread.h>

struct order {
    struct timespec start_time;
    struct timespec prep_time;
    struct timespec bake_end_time;
    struct timespec delivery_end_time;
    uint8_t n_margarita;
    uint8_t n_peperoni;
    uint8_t n_special;
};

struct lock {
    pthread_mutex_t mtx;
    pthread_cond_t cond;
    int avail;
};

struct locks {
    struct lock* tel;
    struct lock* cook;
    struct lock* oven;
    struct lock* deliverer;
    pthread_mutex_t stat_mtx;
};

struct threads_arg {
    unsigned int id;
    unsigned int seed;
};

struct statistics {
    uint64_t total_income;
    uint64_t sold_margarita;
    uint64_t sold_peperoni;
    uint64_t sold_special;
    uint64_t successful;
    uint64_t failed;
    uint64_t total_service_time;
    uint64_t max_service_time;
    uint64_t total_cooling_time;
    uint64_t max_cooling_time;
};

void* malloc_check(size_t size, char* msg);
void usage();
int parse_input(const char* arg1, const char* arg2, unsigned int* customers, unsigned int* seed);
void* order_start (void* arg);
long time_diff_in_minutes(struct timespec* start,struct timespec*end);

#endif
