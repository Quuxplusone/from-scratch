#pragma once

#include <linux/futex.h>
#include <syscall.h>
#include <unistd.h>

inline int futex_wait(void *addr, int block_if_value_is)
{
    return syscall(SYS_futex, addr, FUTEX_WAIT, block_if_value_is, nullptr, nullptr, 0);
}

inline int futex_wake_one(void *addr)
{
    return syscall(SYS_futex, addr, FUTEX_WAKE, 1, nullptr, nullptr, 0);
}

inline int futex_wake_all(void *addr)
{
    return syscall(SYS_futex, addr, FUTEX_WAKE, 2147483647, nullptr, nullptr, 0);
}
