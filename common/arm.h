#ifndef GUARD_COMMON_ARM_H
#define GUARD_COMMON_ARM_H

/**
 * Arm state bits
 */
#define ARM_STATE_ERR         0
#define ARM_STATE_MOVING      1
#define ARM_STATE_UP          2
#define ARM_STATE_DOWN        4
#define ARM_STATE_MOVEV       8
#define ARM_STATE_MOVEUP      (ARM_STATE_MOVEV|ARM_STATE_UP)
#define ARM_STATE_MOVEDOWN    (ARM_STATE_MOVEV|ARM_STATE_DOWN)
#define ARM_STATE_MOVEV_MASK  (2|4|8)
#define ARM_STATE_CLOSED      16
#define ARM_STATE_OPEN        32
#define ARM_STATE_GRABMOVE    64
#define ARM_STATE_CLOSING     (ARM_STATE_GRABMOVE|ARM_STATE_CLOSED)
#define ARM_STATE_OPENING     (ARM_STATE_GRABMOVE|ARM_STATE_OPEN)
#define ARM_STATE_GRAB_MASK   (16|32|64)

/**
 * Arm command state
 * Error - forbidden state
 * Idle - nothing to do
 * New - command process beginning
 * Busy - command realization
 * Done - ready to fetch next command
 */
#define ARM_CMDSTATE_ERR  0
#define ARM_CMDSTATE_IDLE 1
#define ARM_CMDSTATE_NEW  2
#define ARM_CMDSTATE_BUSY 3
#define ARM_CMDSTATE_DONE 4

/// Arm commands
#define ARM_CMD_MOVE_N  1
#define ARM_CMD_MOVE_S  2
#define ARM_CMD_MOVE_E  3
#define ARM_CMD_MOVE_W  4
#define ARM_CMD_OPEN    5
#define ARM_CMD_CLOSE   6
#define ARM_CMD_LOWER   7
#define ARM_CMD_HIGHEN  8
/// Following command makes arm idle (awaiting new command set)
#define ARM_CMD_END     0

/// Arm IDs
#define ARM_ID_A 1
#define ARM_ID_B 2
#define ARM_ID_ILLEGAL 0

#endif // GUARD_COMMON_ARM_H
