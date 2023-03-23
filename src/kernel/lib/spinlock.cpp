#include "spinlock.h"

Spinlock::Spinlock()
{
	Release();
}

void Spinlock::Lock()
{
	while (__atomic_test_and_set(&locked, __ATOMIC_ACQUIRE));
}

void Spinlock::Release()
{
	__atomic_clear(&locked, __ATOMIC_RELEASE);
}
