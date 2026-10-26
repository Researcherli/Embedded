# Queue implementations and learning demos

## Goal

Complete both queue examples in `Data_Structures/4_queue` and make them useful as
standalone learning programs. Each public operation should be called from its
own `main` and demonstrate its effect through readable console output.

## Design

Keep the circular-array queue and linked-list queue as separate, self-contained
C programs. Rename `array_queue.c.c` to `array_queue.c` so Clang recognizes it
as C source by its extension. Define each program's required types and helpers
locally; the linked-list version must not require an external `ListNode` or
`printArray` implementation.

Preserve the existing queue operations. Demonstrate construction, enqueue,
front access, dequeue, length, empty-state checks, printing (and array
conversion for the array queue), and destruction. Output should show FIFO
ordering, the array queue's circular wraparound and full-capacity behavior,
and safe empty-queue behavior.

## Error handling

An empty `peek` or `pop` prints a clear message and returns `INT_MAX`, allowing
the demo to continue instead of terminating at an assertion. Reject invalid
array capacities and handle allocation failures without dereferencing a null
pointer. A failed linked-list node allocation must not change queue state.
Printing an empty queue and converting an empty array queue must be safe.

## Validation

Compile each file independently with `clang -g -Wall -Wextra`, then run both
programs and confirm their output demonstrates each operation, FIFO ordering,
array wraparound/full behavior, and empty-queue handling without warnings or
runtime errors.
