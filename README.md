# simple-threadpool

This project features a simple threadpool monitor implementation in C using the pthreads library. It was developed as an exercise for the Udemy course [Concurrent Programming with Pthreads and C Language](https://www.udemy.com/course/pthreads) from [@romulosilvadeoliveira6349](https://github.com/romulosilvadeoliveira634).

## Features

- Threadpool initialization and graceful shutdown;
- Job submission and queue size monitoring.

## Files

- **threadpool-usage-example.c**: Contains an example usage of the threadpool for calculating prime numbers;
- **threadpool.c**: Implementation of the threadpool;
- **threadpool.h**: Header file for the threadpool.

## Usage

1. Compile the project using a C compiler. For example:
    ```sh
    gcc threadpool.c threadpool-usage-example.c -o threadpool.exe
    ```

2. Run it!
