
The algorithm implemented by `scratch::mutex` is found on page 7 of
[Ulrich Drepper's "Futexes are Tricky"](https://www.akkadia.org/drepper/futex.pdf),
published 2011-11-05.

Unlike `std::mutex`, these mutexes are extremely lightweight and non-configurable.
`std::mutex` is invariably based on `pthread_mutex_t`, which is heavyweight and
highly configurable.

A thread that tries to lock an already-locked mutex must "wait", which in this
context means to "go to sleep", which means "to deschedule oneself", which is something
that can happen only with the help of the thread manager — in our case, the OS kernel.
Therefore, a thread that tries to lock an already-locked mutex must enlist the help
of the OS kernel's thread manager — that is, it must make a
[system call](https://en.wikipedia.org/wiki/System_call).

Suppose you observe that some particular mutex's workload is so infrequent that
in practice there is never anybody "waiting" for it. We call such mutexes
"uncontended," and we'd like them to be as fast as possible. Specifically, we'd
like to avoid the overhead of a system call when unlocking an uncontended mutex.

Therefore, `scratch::mutex` stores a three-state "flag" indicating whether it's
`UNLOCKED`, `LOCKED_WITHOUT_WAITERS` (that is, locked but uncontended),
or `LOCKED_WITH_WAITERS` (that is, locked but having been contended at some point
since the last time it was in the `UNLOCKED` state).
An uncontended mutex will only ever be in the first two of these states.

A `scratch::mutex` stores *only* its three-state flag; it doesn't store any data
indicating "by whom" it has been locked. This implies:

- There is no way to ask a `scratch::mutex` for the identity of its "owning thread",
because the concept doesn't exist. Similarly, there's no way to ask a `std::mutex`
for its "owner", either.

- `scratch::mutex` is "non-recursive." A single thread running the consecutive
statements `m.lock(); m.lock();` will invariably try to lock `m` while it is already
locked, and thus the thread will deadlock with itself. This behavior is permitted
by the Standard, since locking a mutex that you already own falls into the category
of undefined behavior.

- If thread A locks `m`, and then thread B runs `m.unlock()`, thread B will
succeed and the mutex will become unlocked. This behavior is permitted by the
Standard, since unlocking a mutex that you don't actually own falls into the category
of undefined behavior. However, this is fundamentally different from `pthread_mutex_t`'s
behavior: `pthread_mutex_t` does store the identity of the locking thread, and
generally will not do the right thing if you try to unlock a mutex that you don't
own.

- You might be tempted to conclude that because `scratch::mutex` has no concept
of "owning thread", it therefore has no concept of "thread", and would work equally
well with OS threads, greenthreads, etc. *That would be incorrect.* Remember, the
entire job of a mutex is to "wait" — which means "deschedule this thread" — which
implies not only the concept of a "thread" but also the concept of "descheduling."
You can't have the concept of a "mutex" unless you have already decided on the
interface to your thread manager. We chose "Linux kernel threads, on a Linux kernel
that is modern enough to support the
[futex](http://man7.org/linux/man-pages/man7/futex.7.html) system call."
