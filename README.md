# capp
A header-only C++ utility to define multi-service, multi-threaded applications in a compact, easy-to-read manner.


## Concepts
An `Application` contains at least one thread and each thread contains at least one `Worker`.
Idle strategies are able to be defined for each thread to determine how the thread behaves in the absense of work.
A `Worker` is meant to be non-blocking, require no input arguments, and return a result code.

### Worker Implementations
A `Worker` can either be defined as a generic `std::function<int()>` or a class that defines `int operator()()`.

### Worker Result Codes
Result codes have different meanings if positive, zero, or negative.
* Positive: There may be more work to do immediately. This tells the thread to call back again without going to the idle strategy.
* Zero: No work was done or we know there will be no more work to accomplish immediately. This will trigger a call to the idle strategy.
* Negative: Returning a negative result code or exception from a worker will stop all threads.

### Idle Strategy Implementations
Idle strategies are defined as `std::function<void()>`. 
Some idle strategies provided out of the box in `capp/idle.h`, but you are free to roll your own.
The default idle strategy is `Sleep{1ms}`, so it is recommended to override for high-performance applications.
Provided idle strategies exist in the `capp::idle` namespace and are defined as follows:
* BusySpin: `asm volatile("pause\n" : : : "memory")`
* Yield: `std::this_thread::yield()`
* Sleep: `std::this_thread::sleep_for(duration)`
* NoOp: empty function


## Quick Start

### One Thread One Worker
The following example prints "Hello World!" every second.
This is due to returning a `0` result code and using a `Sleep{1s}` idle strategy as the application default.
```
capp::Application app;
app.set_default_idle_strategy(idle::Sleep{1s})
   .add_thread({"my_worker", []() { printf("Hello World!\n"); return 0; }})
   .start()
   .join();
```

### Two Threads
The following example prints "Hello World!" every second and "Hola, Mundo!" every 5 seconds from different threads.
This is due to returning a `0` result code and using different `Sleep` idle strategies on each thread.
```
capp::Application app;
app.add_thread({"worker 1", []() { printf("Hello World!\n"); return 0; }}, idle::Sleep{1s})
   .add_thread({"worker 2", []() { printf("Hola, Mundo!\n"); return 0; }}, idle::Sleep{5s})
   .start()
   .join(); 
```

### One Thread Multiple Workers
The following example prints "Hello World!" and "Hola, Mundo!" every second from the same thread but from different workers.
This is due to returning a `0` result code and setting a `Sleep{1s}` idle strategy for the thread.
```
capp::Application app;
app.add_thread("thread 1", 
        {"worker 1", []() { printf("Hello World!\n"); return 0; }},
        {"worker 2", []() { printf("Hola, Mundo!\n"); return 0; }}, 
        idle::Sleep{1s})
   .start()
   .join(); 
```

## Stopping an Application

### Stop Method
Gracefully stopping threads will occur automatically during `capp::Application` destruction.
Additionally, `stop_async()` and `stop()` methods are also provided to give additional control over object lifecycle when appropriate.

### Exit Codes
Any time a worker forces an application to termine by throwing an exception or returning a negative result code, that code will be discoverable via `Application::exit_code()`.
This means that `Application::exit_code()` can directly drive `main()`'s return code for abnormal termination.
For example:
```
int main() {
  capp::Application app;
  app.set_logger(std::make_shared<StderrLogger>())
     .add_thread({"worker", MyWorker{}}
     .start()
     .join(); 
  return app.exit_code();
```

### Signal Handling
Signal handling is outside of the scope of this library, but it is recommended that process signals are handled to simply call `Application::stop()` when appropriate.


## Logging
By default, logging is disabled.
Logging can be enabled by calling `Application::set_logger(std::shared_ptr<Logger>)`.
`capp/stderr_logger.h` provides a very simple Logger implementation that prints to stderr.
`capp::Logger` is a very simple interface defined in `capp/logger.h`, so you are encouraged to roll your own implementation for your logging framework of choice.


## Internals
Internal implementation details exist in the `capp::detail` namespace.
These files, functions, and classes may change any time, without warning.


## Development
This project uses Bazel.
The `ide/` directory provides VS Code projects for developing in a Docker container that will auto-install all required development dependencies.
Run `code ide/linux` or `code ide/selinux` from the repo root and follow the prompts to start in a container and install extensions to get up and running quickly.

