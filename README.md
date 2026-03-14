# Pizzeria Order Simulation with Threads in C

A multithreaded simulation of a pizzeria ordering system developed in **C** using **POSIX threads (`pthreads`)**, designed to demonstrate core concepts of **operating systems**, **concurrency**, **thread synchronization**, and **resource management**.

This project models the lifecycle of customer orders in a pizza store, where multiple orders are processed concurrently while competing for limited shared resources such as telephone operators, cooks, ovens, and delivery drivers.

---

## Overview

This project simulates the real-time operation of a pizza restaurant under concurrent customer demand.

Each customer is represented by a separate **thread**, and each order passes through a sequence of production and delivery stages:

1. Telephone order reception
2. Payment processing
3. Food preparation
4. Baking
5. Packaging
6. Delivery
7. Driver return

Because the pizzeria has a limited number of employees and equipment, orders may need to wait until the required resources become available. The system uses synchronization mechanisms to coordinate access to shared resources and to ensure correct updates of global statistics.

The project is a strong educational example of how operating systems concepts can be applied to the simulation of a real service pipeline.

---

## Key Features

* Multithreaded simulation with one thread per customer
* Concurrent order handling using **POSIX threads**
* Shared resource management with limited availability
* Synchronization using **mutexes** and **condition variables**
* Randomized customer behavior and order composition
* Payment success/failure handling
* Dynamic waiting for cooks, ovens, and delivery drivers
* Calculation of store statistics and performance metrics
* Simulation of preparation, baking, packaging, and delivery times

---

## Educational Goals

This project was developed to demonstrate important concepts from **Operating Systems** and **Concurrent Programming**, including:

* Thread creation and management
* Mutual exclusion with mutexes
* Condition synchronization with condition variables
* Safe access to shared data structures
* Resource contention between concurrent threads
* Simulation of service systems with bounded resources
* Collection of aggregate statistics from parallel execution

It is especially suitable as an academic project for courses such as:

* Operating Systems
* Concurrent Programming
* Systems Programming
* Parallel Computing

---

## Problem Scenario

The pizzeria serves customer orders with a fixed number of available resources:

* Telephone operators receive and register orders
* Cooks prepare pizzas
* Ovens bake pizzas
* Delivery drivers package and deliver orders

Each customer places an order containing a random number of pizzas, and each pizza belongs to one of the available categories:

* Margherita
* Pepperoni
* Special

The simulation includes:

* random order size
* random pizza types
* random payment duration
* random delivery time
* a small probability of payment failure

As more customers place orders concurrently, threads may be blocked while waiting for limited resources, making the simulation a practical example of synchronization and scheduling in shared systems.

---

## Project Structure

```text
Operating_Systems/
├── pizza.c
├── pizza.h
├── README.md
└── .gitignore
```

### File Descriptions

#### `pizza.c`

Contains the full implementation of the simulation, including:

* thread logic
* synchronization
* order processing
* timing
* statistics collection
* main program flow

#### `pizza.h`

Contains:

* constant definitions
* configuration parameters
* structure declarations
* function declarations

#### `README.md`

Project documentation for GitHub, including architecture, execution instructions, and explanation of the simulation.

---

## Technologies Used

* **C**
* **POSIX Threads (`pthread`)**
* **Mutexes (`pthread_mutex_t`)**
* **Condition Variables (`pthread_cond_t`)**
* **Standard C libraries**
* **Time and random number utilities**

---

## Simulation Parameters

The simulation is configured through predefined constants in `pizza.h`.

### Available Resources

* `N_TEL = 2` → number of telephone operators
* `N_COOK = 2` → number of cooks
* `N_OVEN = 10` → number of ovens
* `N_DELIVERER = 10` → number of delivery drivers

### Order Size

* `N_ORDER_LOW = 1` → minimum pizzas per order
* `N_ORDER_HIGH = 5` → maximum pizzas per order

### Timing Parameters

* `T_ORDER_LOW = 1` → minimum delay between customer arrivals
* `T_ORDER_HIGH = 5` → maximum delay between customer arrivals
* `T_PAYMENT_LOW = 1` → minimum payment duration
* `T_PAYMENT_HIGH = 3` → maximum payment duration
* `T_PREP = 1` → preparation time per pizza
* `T_BAKE = 10` → baking time
* `T_PACK = 1` → packaging time per pizza
* `T_DEL_LOW = 5` → minimum delivery time
* `T_DEL_HIGH = 15` → maximum delivery time

### Pizza Type Probabilities

* `P_M = 35` → probability of Margherita
* `P_P = 25` → probability of Pepperoni
* `P_S = 40` → probability of Special

### Payment Failure Probability

* `P_FAIL = 5` → probability of order cancellation due to failed payment

### Pizza Prices

* `C_M = 10` → Margherita price
* `C_P = 11` → Pepperoni price
* `C_S = 12` → Special price

These constants make the system easy to modify and extend for testing different load conditions.

---

## Core Data Structures

The project uses a set of structured data types to model the state of the system.

### `struct order`

Represents a single customer order and stores information such as:

* order start time
* preparation completion time
* baking completion time
* delivery completion time
* number of pizzas of each type

This structure is used for tracking the progress and timing of each order.

### `struct lock`

Represents a shared resource category.

It contains:

* a mutex
* a condition variable
* a counter of available units

This abstraction is used for all limited resources in the pizzeria.

### `struct locks`

Groups all resource locks together:

* telephone operators
* cooks
* ovens
* delivery drivers
* statistics mutex

This centralizes synchronization primitives and keeps the implementation organized.

### `struct threads_arg`

Stores per-thread arguments:

* customer ID
* random seed

Each thread receives its own copy of these values.

### `struct statistics`

Stores the global performance data of the simulation, including:

* total revenue
* successful orders
* failed orders
* pizzas sold per category
* total and maximum service time
* total and maximum cooling time

This enables the production of summary statistics after all threads finish execution.

---

## System Workflow

The full order lifecycle is implemented inside the thread routine and models the stages of a real pizzeria service pipeline.

### 1. Customer Arrival

Each new customer is created as a separate thread.

A random delay is inserted between thread creations in order to simulate customers arriving over time rather than all at once.

### 2. Telephone Operator Assignment

A customer must first acquire an available telephone operator.

If no operator is available, the thread waits on a condition variable until one becomes free.

### 3. Order Creation

Once connected, the customer creates a random order:

* between 1 and 5 pizzas
* each pizza randomly assigned a type

The order cost is computed based on the selected pizzas.

### 4. Payment Processing

The customer spends a random amount of time completing payment.

At the end of this stage, the payment may fail with a predefined probability.

If the payment fails:

* the order is cancelled
* the telephone operator is released
* the failed order counter is updated
* the thread terminates

### 5. Cook Assignment

For successful payments, the order proceeds to preparation.

A cook must be acquired before preparation can begin. If no cook is available, the thread waits.

### 6. Pizza Preparation

Preparation time depends on the number of pizzas in the order:

`preparation time = T_PREP × number_of_pizzas`

This stage models the active work performed by the cook.

### 7. Oven Assignment

After preparation, the order requires oven space.

The system waits until enough ovens are available for all pizzas in the order. This means that larger orders may wait longer if oven capacity is limited.

### 8. Baking

The pizzas bake for a fixed duration.

When baking finishes, the simulation records the baking completion time, which is later used to calculate the cooling time.

### 9. Delivery Driver Assignment

Once baking is complete, the order waits for an available delivery driver.

Only when a driver becomes available can the order proceed to packaging and delivery.

### 10. Packaging

Packaging time depends on the number of pizzas:

`packaging time = T_PACK × number_of_pizzas`

At the end of this stage, the order is considered ready to leave the store.

### 11. Delivery

The order is delivered to the customer using a random delivery time.

This stage simulates travel time from the store to the destination.

### 12. Driver Return

After delivering the order, the delivery driver must also return to the store before becoming available again.

This models the full occupation time of the delivery resource.

### 13. Statistics Update

When the order is fully completed, the thread updates shared store statistics in a protected critical section.

---

## Synchronization Strategy

One of the most important aspects of the project is the synchronization logic.

Each limited resource is modeled with:

* one mutex
* one condition variable
* one availability counter

The general pattern is:

1. Lock the resource mutex
2. While the resource is unavailable, wait on the condition variable
3. Reserve the required units
4. Unlock the mutex

When the resource is released:

1. Lock the mutex
2. Increase availability
3. Signal waiting threads
4. Unlock the mutex

This design ensures:

* safe concurrent access
* correct waiting behavior
* proper coordination between customer threads
* accurate simulation of limited real-world resources

---

## Statistics Collected

At the end of the simulation, the program prints a set of aggregate statistics.

These include:

* total income of the store
* number of successful orders
* number of failed orders
* number of Margherita pizzas sold
* number of Pepperoni pizzas sold
* number of Special pizzas sold
* average service time
* maximum service time
* average cooling time
* maximum cooling time

### Meaning of the Metrics

#### Service Time

The total time from the moment the customer places the order until the order is delivered.

#### Cooling Time

The time between the end of baking and the final delivery to the customer.

This metric reflects product freshness and delivery efficiency.

Together, these statistics provide insight into both the operational throughput and the customer experience of the simulated pizzeria.

---

## Important Functions

### `usage()`

Prints the correct program usage and exits when the provided arguments are invalid.

### `malloc_check()`

Wrapper around memory allocation that checks for allocation failure and terminates safely if memory cannot be allocated.

### `parse_input()`

Validates and parses command-line arguments.

It ensures that:

* the number of customers is valid
* the seed is valid
* invalid or malformed input is rejected

### `get_random()`

Generates thread-safe pseudo-random numbers using `rand_r()` and a per-thread seed.

This avoids shared global random state and makes concurrent execution safer.

### `time_diff_in_minutes()`

Calculates time differences between simulation events and converts them into the time unit used for reporting.

### `order_start(void *arg)`

The main thread routine of the simulation.

It contains the full lifecycle of a customer order, including:

* resource acquisition
* payment
* preparation
* baking
* packaging
* delivery
* statistics update

### `main()`

Responsible for:

* parsing input
* initializing resources and data structures
* creating customer threads
* waiting for all threads to finish
* printing final statistics
* releasing allocated memory

---

## How to Compile

Use GCC with pthread support enabled.

```bash
gcc -Wall -Werror -pthread -g -o pizza pizza.c -lm
```

---

## How to Run

The program expects two command-line arguments:

1. number of customers
2. initial random seed

Example:

```bash
./pizza 100 10
```

This runs the simulation for:

* `100` customers
* seed `10`

---

## Example Execution Flow

A typical execution may look like this:

1. Customer threads are created one by one
2. Some customers immediately reach a telephone operator
3. Others wait because all operators are busy
4. Successful orders move to cooks and ovens
5. Large orders may wait longer for enough ovens
6. After baking, orders wait for available delivery drivers
7. Orders are packaged and delivered
8. Drivers return and become available again
9. Final store statistics are printed after all orders complete

This creates a realistic pipeline of dependent stages with limited parallel capacity.

---

## Why This Project Is Interesting

This project stands out because it combines several classic operating systems concepts into one coherent simulation.

It is not just a simple thread creation exercise. It models a complete service workflow with:

* concurrent execution
* shared resource contention
* blocking and waiting
* synchronization primitives
* critical sections
* performance metrics

The simulation captures both the computational aspect of multithreading and the practical behavior of a real-world queueing system.

---

## Learning Outcomes

By studying or implementing this project, one can better understand:

* how threads interact in a shared environment
* how mutexes prevent race conditions
* how condition variables coordinate waiting threads
* how bounded resources affect throughput
* how concurrency impacts response times and performance
* how to design thread-safe aggregate statistics

---

## Possible Extensions

The current design provides a strong foundation for future enhancements.

Possible extensions include:

* support for more pizza categories
* dynamic number of workers/resources
* priority handling for orders
* different preparation times by pizza type
* cancellation after long waiting times
* real queue statistics
* graphical visualization of the simulation
* logging to files for performance analysis

These additions could make the simulation even more realistic and suitable for deeper experimental study.

---

## Repository Goals

This repository showcases:

* a complete multithreaded simulation in C
* practical use of pthread synchronization
* modeling of shared system resources
* order lifecycle processing under concurrency
* collection of performance and business statistics
* a strong academic example for operating systems coursework

---

## Authors


* Konstantinos Gougas


---

## Final Notes

This project demonstrates how core operating systems concepts can be translated into a practical simulation of a real-world service environment. By combining threads, synchronization, shared resources, and performance metrics, it provides a complete and educational example of concurrent programming in C.
