#include "pizza.h"

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// 	global structs for locks and statistics
struct locks* locks; 
struct statistics* stats;

// 	usage preview function
void usage() {
    printf("usage: pizza number_of_customers initial_seed\n");
    printf("number_of_customers: positive integer\n");
    printf("initial_seed: positive integer\n");
    exit(1);
}

// 	malloc wrapper to check that memory was properly allocated
void* malloc_check(size_t size, char* msg) {
    void* p = malloc(size);
    if (!p) {
        printf("error: not enough memory to allocate %s\n", msg);
        exit(1);
    }
    return p;
}

/* 	function to parse the two program arguments and check 
	if they can be parsed as valid unsigned integers
*/
int parse_input(const char* arg1, const char* arg2, unsigned int* customers, unsigned int* seed) {
    char* endp;
    long value = 0;

    errno = 0;
    value = strtol(arg1, &endp, 0);
    if (errno != 0 || *endp != '\0') return -1;
    if (value <= 0 || value > UINT_MAX) return -1;
    *customers = (unsigned int)value;

    value = 0;
    value = strtol(arg2, &endp, 0);
    if (errno != 0 || *endp != '\0') return -1;
    if (value <= 0 || value > UINT_MAX) return -1;
    *seed = (unsigned int)value;
    return 0;
}

// 	function to get a random number in the range [low, high] with a given seed
int get_random(int low, int high, unsigned int* seed) {
    return rand_r(seed) % (high + 1 - low) + low;
}

//function to calculate the difference between two timespaces
long time_diff_in_minutes(struct timespec* start,struct timespec*end){
    return (long) round((double)((end->tv_sec - start->tv_sec)* 1000000000L + (end->tv_nsec - start->tv_nsec)) / NSEC_IN_MIN);
}

// 	start of the threads-orders
void* order_start(void* arg) {
    struct threads_arg* args = (struct threads_arg*) arg;
    struct order order;
    int n_pizzas;

    clock_gettime(CLOCK_REALTIME, &(order.start_time));
    printf("Thread %u just started with seed: %u at %s\n", args->id, args->seed, asctime(localtime(&(order.start_time.tv_sec))));

	// try and find an available call handler
    pthread_mutex_lock(&(locks->tel->mtx));
    while (locks->tel->avail <= 0) {
												
        pthread_cond_wait(&(locks->tel->cond), &(locks->tel->mtx));
    }
	// available call handler found
    locks->tel->avail--;
    pthread_mutex_unlock(&(locks->tel->mtx));

	// randomly assign the pizzas and their type
    n_pizzas = get_random(N_ORDER_LOW, N_ORDER_HIGH, &(args->seed));
    order.n_margarita = 0;
    order.n_peperoni = 0;
    order.n_special = 0;
    int tmp;
    for (int i = 0; i < n_pizzas; i++) {
        tmp = get_random(1, 100, &(args->seed));
        if (tmp <= P_M) order.n_margarita++;
        else if (tmp <= P_M + P_P) order.n_peperoni++;
        else order.n_special++;
    }
    int cost = order.n_margarita * C_M + order.n_peperoni * C_P + order.n_special * C_S;
    int fail = 0;
	
	// try to charge the customer
    sleep(get_random(T_PAYMENT_LOW, T_PAYMENT_HIGH, &(args->seed)));
    tmp = get_random(1, 100, &(args->seed));
    if (tmp <= P_FAIL) {
        printf("Order %d failed.\n", args->id);
        fail = 1;
    } else {
        printf("Order %d registered: Margarita %u, Peperoni %u, Special %u\n", args->id, order.n_margarita, order.n_peperoni, order.n_special);
    }

	// free the call-handler
    pthread_mutex_lock(&(locks->tel->mtx));
    locks->tel->avail++;
    pthread_cond_signal(&(locks->tel->cond));
    pthread_mutex_unlock(&(locks->tel->mtx));

	// if failed update the failed counter in statistics and return immediately
    if (fail){
		pthread_mutex_lock(&(locks->stat_mtx));
        stats->failed += 1;
        pthread_mutex_unlock(&(locks->stat_mtx));
        return NULL;
    }
		
    // try to find avavailable cook
	pthread_mutex_lock(&(locks->cook->mtx));
    while (locks->cook->avail <= 0) {
        pthread_cond_wait(&(locks->cook->cond), &(locks->cook->mtx));
    }
	// cook found
    locks->cook->avail--;
    pthread_mutex_unlock(&(locks->cook->mtx));

    // time to prepare the order
    sleep(T_PREP * n_pizzas);

    // wait for n_pizzas ovens to be available
    pthread_mutex_lock(&(locks->oven->mtx));
    while (locks->oven->avail < n_pizzas  ){
        pthread_cond_wait(&(locks->oven->cond), &(locks->oven->mtx));
    }
    // ovens became available 
    locks->oven->avail-=n_pizzas;
    pthread_mutex_unlock(&(locks->oven->mtx));

    // free the cook
    pthread_mutex_lock(&(locks->cook->mtx));
    locks->cook->avail++;
    pthread_cond_signal(&(locks->cook->cond));
    pthread_mutex_unlock(&(locks->cook->mtx));

    // wait for the pizzas to cook
    sleep(T_BAKE);
    // Record the bake end time
    clock_gettime(CLOCK_REALTIME, &(order.bake_end_time));
	
	// try to find an available delivery
	pthread_mutex_lock(&(locks->deliverer->mtx));
    while (locks->deliverer->avail <= 0) {
        pthread_cond_wait(&(locks->deliverer->cond), &(locks->deliverer->mtx));
    }
	// delivery found
    locks->deliverer->avail--;
    pthread_mutex_unlock(&(locks->deliverer->mtx));

    //free the ovens 
    pthread_mutex_lock(&(locks->oven->mtx));
    locks->oven->avail+=n_pizzas;
    pthread_cond_signal(&(locks->oven->cond));
    pthread_mutex_unlock(&(locks->oven->mtx));

	// time to pack the order
	sleep(T_PACK * n_pizzas);
    clock_gettime(CLOCK_REALTIME, &(order.prep_time));
    long packing_time = time_diff_in_minutes(&(order.start_time), &(order.prep_time));
    printf("Order %d prepared in %ld minutes\n", args->id , packing_time);

	// time to deliver to the customer
	int round_trip_time = get_random(T_DEL_LOW, T_DEL_HIGH, &(args->seed));
    sleep(round_trip_time);
	
	// Record the delivery end time and calculate the service and cooling times
    clock_gettime(CLOCK_REALTIME, &(order.delivery_end_time));
    long service_time = time_diff_in_minutes(&(order.start_time), &(order.delivery_end_time));
    long cooling_time = time_diff_in_minutes(&(order.bake_end_time), &(order.delivery_end_time));
    printf("Order %d delivered in %ld minutes \n", args->id , service_time);

	// time to come back from delivery
	sleep(round_trip_time);
	
	// free the delivery
    pthread_mutex_lock(&(locks->deliverer->mtx));
    locks->deliverer->avail++;
    pthread_cond_signal(&(locks->deliverer->cond));
    pthread_mutex_unlock(&(locks->deliverer->mtx));
	
	
	//update statistics for successful order
    pthread_mutex_lock(&(locks->stat_mtx));
	stats->successful += 1;
    stats->sold_margarita += order.n_margarita;
    stats->sold_peperoni += order.n_peperoni;
    stats->sold_special += order.n_special;
    stats->total_income += cost;
    stats->total_service_time += service_time;
    if (service_time > stats->max_service_time){
        stats->max_service_time = service_time;
    }
    stats->total_cooling_time += cooling_time;
    if(cooling_time > stats->max_cooling_time){
        stats->max_cooling_time = cooling_time;   
    }
    pthread_mutex_unlock(&(locks->stat_mtx));

    return NULL;
}


int main(int argc, char** argv) {
    unsigned int n_customers;
    unsigned int seed;
    pthread_t* threads;
    struct threads_arg* t_arg;

    if (argc != 3) usage();
    if (parse_input(argv[1], argv[2], &n_customers, &seed) != 0) {
        printf("Error in parsing\n");
        usage();
    }
	
	// initialize the required structs for the program
    threads = malloc_check(sizeof(pthread_t) * n_customers, "threads");
    t_arg = malloc_check(sizeof(struct threads_arg) * n_customers, "thread arguments");
    locks = malloc_check(sizeof(struct locks), "locks list");
    locks->tel = malloc_check(sizeof(struct lock), "tel locks");
    locks->cook = malloc_check(sizeof(struct lock), "cook locks");
    locks->oven = malloc_check(sizeof(struct lock), "oven locks");
    locks->deliverer = malloc_check(sizeof(struct lock), "deliverer locks");
    
    stats = malloc_check(sizeof(struct statistics), "statistics");
    stats->total_income = 0;
    stats->failed = 0;
    stats->successful = 0;
    stats->sold_margarita = 0;
    stats->sold_peperoni = 0;
    stats->sold_special = 0;
    stats->total_service_time = 0;
    stats->max_service_time = 0;
    stats->total_cooling_time = 0;
    stats->max_cooling_time = 0;
    
	// initialize locks required
    if (pthread_mutex_init(&(locks->tel->mtx), NULL)) {
        printf("Error: tel mtx initialization failed\n");
        exit(1);
    }
    if (pthread_cond_init(&(locks->tel->cond), NULL)) {
        printf("Error: tel cond initialization failed\n");
        exit(1);
    }
    locks->tel->avail = N_TEL;

    if (pthread_mutex_init(&(locks->deliverer->mtx), NULL)) {
        printf("Error: deliverer mtx initialization failed\n");
        exit(1);
    }
    if (pthread_cond_init(&(locks->deliverer->cond), NULL)) {
        printf("Error: deliverer cond initialization failed\n");
        exit(1);
    }
    locks->deliverer->avail = N_DELIVERER;

    if (pthread_mutex_init(&(locks->cook->mtx), NULL)) {
        printf("Error: cook mtx initialization failed\n");
        exit(1);
    }
    if (pthread_cond_init(&(locks->cook->cond), NULL)) {
        printf("Error: cook cond initialization failed\n");
        exit(1);
    }
    locks->cook->avail = N_COOK;

    if (pthread_mutex_init(&(locks->oven->mtx), NULL)) {
        printf("Error: oven mtx initialization failed\n");
        exit(1);
    }
    if (pthread_cond_init(&(locks->oven->cond), NULL)) {
        printf("Error: oven cond initialization failed\n");
        exit(1);
    }
    locks->oven->avail = N_OVEN;

	if(pthread_mutex_init(&(locks->stat_mtx), NULL)) {
        printf("Error: stat_mtx initialization failed\n");
        exit(1);
    }
	
	// start generating the threads-orders
    for (int i = 0; i < n_customers; i++) {
        (t_arg + i)->id = i;
        (t_arg + i)->seed = seed;
        if (pthread_create(threads + i, NULL, order_start, t_arg + i)) {
            printf("Error in thread creation\n");
            exit(1);
        }
        sleep(get_random(T_ORDER_LOW, T_ORDER_HIGH, &seed));
    }

	// wait for all orders to finish
    for (int i = 0; i < n_customers; i++) {
        if (pthread_join(*(threads + i), NULL)) {
            printf("Error in thread join\n");
            exit(1);
        }
    }
	
	// since all threads are done by now, we do not need to lock/unlock the stats mutex
    printf("All threads returned.\n");
    printf("Total income: %lu\n", stats->total_income);
    printf("Successful orders: %lu. Failed orders: %lu\n", stats->successful, stats->failed);
    printf("Pizzas sold: Margarita: %lu, Peperoni: %lu, Special: %lu\n", stats->sold_margarita, stats->sold_peperoni, stats->sold_special);

    if (stats->successful > 0){
        printf("Average service time: %lu minutes\n", (long) round(stats->total_service_time / stats->successful));
        printf("Maximum service time: %lu minutes\n", (long) round(stats->max_service_time));
        printf("Average cooling time: %lu minutes\n", (long) round(stats->total_cooling_time / stats->successful));
        printf("Maximum cooling time: %lu minutes\n", (long) round(stats->max_cooling_time));
    }

    free(threads);
    free(t_arg);
    free(locks->tel);
    free(locks->cook);
    free(locks->oven);
    free(locks->deliverer);
    free(locks);
    free(stats);

    return 0;
}