// prioritytest.c
// Demonstrates the priority-based scheduler in xv6.
// Tests:
//   1. setpriority() and getpriority() syscalls work correctly
//   2. Default priority is 50
//   3. Higher priority processes (lower number) finish before lower priority ones
//   4. Priority inheritance: child inherits parent priority

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// CPU-bound work to keep a process busy
static void
do_work(void)
{
  volatile long x = 0;
  for(long i = 0; i < 50000000L; i++)
    x += i;
  (void)x;
}

// -------------------------------------------------------
// Test 1: setpriority and getpriority syscalls
// -------------------------------------------------------
void
test1(void)
{
  printf("\n=== Test 1: setpriority() and getpriority() ===\n");

  int pid = getpid();

  // Check default priority
  int def = getpriority(pid);
  printf("  Default priority of pid %d = %d  (expected 50) --> %s\n",
         pid, def, def == 50 ? "PASS" : "FAIL");

  // Set to 10 and read back
  setpriority(pid, 10);
  int p10 = getpriority(pid);
  printf("  After setpriority(10): getpriority = %d  (expected 10) --> %s\n",
         p10, p10 == 10 ? "PASS" : "FAIL");

  // Set to 150 and read back
  setpriority(pid, 150);
  int p150 = getpriority(pid);
  printf("  After setpriority(150): getpriority = %d  (expected 150) --> %s\n",
         p150, p150 == 150 ? "PASS" : "FAIL");

  // Set to boundary value 0
  setpriority(pid, 0);
  int p0 = getpriority(pid);
  printf("  After setpriority(0):   getpriority = %d  (expected 0)   --> %s\n",
         p0, p0 == 0 ? "PASS" : "FAIL");

  // Set to boundary value 200
  setpriority(pid, 200);
  int p200 = getpriority(pid);
  printf("  After setpriority(200): getpriority = %d  (expected 200) --> %s\n",
         p200, p200 == 200 ? "PASS" : "FAIL");

  // Invalid value should return -1
  int bad = setpriority(pid, 201);
  printf("  setpriority(201) returns %d  (expected -1 = invalid) --> %s\n",
         bad, bad == -1 ? "PASS" : "FAIL");

  // Restore to default
  setpriority(pid, 50);
  printf("  Restored priority to 50.\n");
}

// -------------------------------------------------------
// Test 2: Priority inheritance (child inherits parent)
// -------------------------------------------------------
void
test2(void)
{
  printf("\n=== Test 2: Fork inherits parent priority ===\n");

  int pid = getpid();
  setpriority(pid, 30);
  printf("  Parent pid=%d priority set to 30\n", pid);

  int child = fork();
  if(child == 0){
    int cprio = getpriority(getpid());
    printf("  Child  pid=%d priority = %d  (expected 30) --> %s\n",
           getpid(), cprio, cprio == 30 ? "PASS" : "FAIL");
    exit(0);
  }
  wait(0);

  // Restore
  setpriority(pid, 50);
}

// -------------------------------------------------------
// Test 3: Scheduling order — higher priority finishes first
// -------------------------------------------------------
void
test3(void)
{
  printf("\n=== Test 3: Scheduling order by priority ===\n");
  printf("  Spawning 4 processes. Higher priority (lower number) should finish first.\n\n");

  int pids[4];
  int priorities[4] = {10, 50, 100, 150};

  // Fork all children first, then each sets its own priority
  for(int i = 0; i < 4; i++){
    pids[i] = fork();
    if(pids[i] == 0){
      // Child: set priority then do CPU work
      int mypid = getpid();
      int myprio = priorities[i];
      setpriority(mypid, myprio);
      printf("  [pid=%d] started  with priority=%d\n", mypid, myprio);
      do_work();
      printf("  [pid=%d] FINISHED with priority=%d\n", mypid, myprio);
      exit(0);
    }
  }

  // Parent waits for all
  for(int i = 0; i < 4; i++)
    wait(0);

  printf("\n  --> Processes with lower priority numbers should have finished first.\n");
  printf("  --> If finish order is 10, 50, 100, 150 then scheduler: PASS\n");
}

// -------------------------------------------------------
// Test 4: getpriority on non-existent PID returns -1
// -------------------------------------------------------
void
test4(void)
{
  printf("\n=== Test 4: getpriority on invalid PID ===\n");
  int bad = getpriority(9999);
  printf("  getpriority(9999) = %d  (expected -1) --> %s\n",
         bad, bad == -1 ? "PASS" : "FAIL");
}

// -------------------------------------------------------
// Main
// -------------------------------------------------------
int
main(void)
{
  printf("\n");
  printf("╔══════════════════════════════════════════╗\n");
  printf("║   xv6 Priority Scheduler - Test Suite   ║\n");
  printf("╚══════════════════════════════════════════╝\n");

  test1();
  test2();
  test3();
  test4();

  printf("\n=== All tests complete ===\n\n");
  exit(0);
}
