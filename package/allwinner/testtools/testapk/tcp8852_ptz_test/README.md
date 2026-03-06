# TCP8852 Driver User API Documentation

## Overview
This document describes the user-space API for the TCP8852 motor driver, which provides an interface for controlling TCP8852-based motor systems. This header file defines constants, structures, and ioctl commands necessary for communicating with the TCP8852 driver.

## Channel Flags
These constants define the channel identifiers for the TCP8852 driver, allowing control of individual or combined channels.

| Constant | Value | Description |
|----------|-------|-------------|
| `CH_NONE` | 0 | No channel selected |
| `CH1` | (1 << 0) | Channel 1 |
| `CH2` | (1 << 1) | Channel 2 |
| `CH3` | (1 << 2) | Channel 3 |
| `CH4` | (1 << 3) | Channel 4 |
| `CH12` | 0b0011 | Combined channels 1 and 2 |
| `CH34` | 0b1100 | Combined channels 3 and 4 |

### Channel Combination Check
`CH_CONTAINS(combined, ch)` - Macro to check if a specific channel is included in a combined channel mask.

## Microstep Divisions
These constants define the microstep resolution settings for stepper motors.

| Constant | Value | Description |
|----------|-------|-------------|
| `MSTEP_DIVIDE_64` | 0b000 | 1/64 microstep resolution |
| `MSTEP_DIVIDE_128` | 0b001 | 1/128 microstep resolution |
| `MSTEP_DIVIDE_256` | 0b010 | 1/256 microstep resolution |

## Motor Operating Modes
These constants define the different operating modes available for motor control.

| Constant | Value | Description |
|----------|-------|-------------|
| `DIRECT_OUT` | 0b000 | Direct Output Control Mode |
| `BLDC` | 0b001 | Brushed DC Motor Drive |
| `PHASE_1_2` | 0b010 | 1-2 Phase Excitation Mode |
| `PHASE_2_2` | 0b011 | 2-2 Phase Excitation Mode |
| `ABSOLUTE_POSITION` | 0b101 | Absolute Position Mode |
| `AUTO` | 0b110 | Auto Control Mode |
| `RELATIVE_POSITION` | 0b111 | Relative Position Mode |

## Motor Motion Control
These constants define the motor motion states.

| Constant | Value | Description |
|----------|-------|-------------|
| `OPEN_CIRCUIT` | 0 | Open Circuit (motor not energized) |
| `FOREWARD` | 1 | Forward motion |
| `INVERSION` | 2 | Reverse motion |
| `BRAKE` | 3 | Brake (dynamic braking) |

## Clock Pre-divider Values
These constants define the pre-divider settings for clock frequency control.

| Constant | Value | Description |
|----------|-------|-------------|
| `CLOCK_PRE_DIVIDE_1` | 0b000 | 1x division (no division) |
| `CLOCK_PRE_DIVIDE_2` | 0b001 | 2x division |
| `CLOCK_PRE_DIVIDE_4` | 0b010 | 4x division |
| `CLOCK_PRE_DIVIDE_8` | 0b011 | 8x division |
| `CLOCK_PRE_DIVIDE_16` | 0b100 | 16x division |
| `CLOCK_PRE_DIVIDE_32` | 0b101 | 32x division |
| `CLOCK_PRE_DIVIDE_64` | 0b110 | 64x division |
| `CLOCK_PRE_DIVIDE_128` | 0b111 | 128x division |

## Clock Divider Values
These constants define the secondary divider settings for clock frequency control, allowing for fine-tuning of the clock speed.

| Constant | Value | Description |
|----------|-------|-------------|
| `CLOCK_DIVIDE_1` | 0b00000 | 1x division (no division) |
| `CLOCK_DIVIDE_2` | 0b00001 | 2x division |
| ... | ... | ... |
| `CLOCK_DIVIDE_32` | 0b11111 | 32x division |

## Speed Pre-divider Values
These constants define the pre-divider settings for speed control.

| Constant | Value | Description |
|----------|-------|-------------|
| `SPEED_PRE_DIVIDE_1` | 0b000 | 1x division (no division) |
| `SPEED_PRE_DIVIDE_2` | 0b001 | 2x division |
| ... | ... | ... |
| `SPEED_PRE_DIVIDE_128` | 0b111 | 128x division |

## Speed Divider Values
These constants define the secondary divider settings for speed control, allowing for fine-tuning of motor speed.

| Constant | Value | Description |
|----------|-------|-------------|
| `SPEED_DIVIDE_1` | 0b00000 | 1x division (no division) |
| `SPEED_DIVIDE_2` | 0b00001 | 2x division |
| ... | ... | ... |
| `SPEED_DIVIDE_32` | 0b11111 | 32x division |

## Low Power Modes
These constants define the different low power modes available.

| Constant | Value | Description |
|----------|-------|-------------|
| `NORMAL_MODE` | 0 | Normal Operation Mode |
| `SOFTWARE_LOW_POWER` | 1 | Software Low Power Mode |
| `HARDWARE_LOW_POWER` | 2 | Hardware Low Power Mode |

## IOCTL Commands
These commands are used to communicate with the TCP8852 driver through the ioctl interface.

| Command | Description | Argument Type |
|---------|-------------|--------------|
| `TCP8852_GET_CHIP_ID` | Retrieve the chip ID | `uint16_t` |
| `TCP8852_SET_CHANNEL_MODE` | Set the operating mode for a channel | `struct tcp8852_channel_cmd` |
| `TCP8852_SET_CHANNEL_ACTION` | Set the motion action for a channel | `struct tcp8852_channel_cmd` |
| `TCP8852_SET_CHANNEL_PWM` | Set PWM value for a channel | `struct tcp8852_channel_cmd` |
| `TCP8852_SET_CHANNEL_MICROSTEP` | Set microstep resolution for a channel | `struct tcp8852_channel_cmd` |
| `TCP8852_SET_CHANNEL_CLOCK` | Set clock parameters for a channel | `struct tcp8852_clock_cmd` |
| `TCP8852_SET_CHANNEL_SPEED` | Set speed parameters for a channel | `struct tcp8852_speed_cmd` |
| `TCP8852_SET_CHANNEL_SPEEDCNT` | Set speed count for a channel | `struct tcp8852_channel_cmd` |
| `TCP8852_SET_CHANNEL_PHASE` | Set phase count for a channel | `struct tcp8852_channel_cmd` |
| `TCP8852_SET_CHANNEL_CYCLE` | Set cycle count for a channel | `struct tcp8852_channel_cmd` |
| `TCP8852_SET_IR_CUT` | Set IRCUT parameters | `struct tcp8852_ircut_cmd` |
| `TCP8852_SET_LOW_POWER` | Set low power mode | `int` |
| `TCP8852_SET_GLOBAL_ENABLE` | Set global enable state | `int` |
| `TCP8852_SET_FREQUENCY_JITTER` | Set frequency jitter | `int` |
| `TCP8852_GET_CHANNEL_STATUS` | Get status information for a channel | `struct tcp8852_channel_status` |
| `TCP8852_GET_DEVICE_STATUS` | Get overall device status | `struct tcp8852_device_status` |
| `TCP8852_RESET_REGISTERS` | Reset all registers to default values | None |

## Data Structures

### `struct tcp8852_channel_cmd`
Structure used for basic channel commands.

```c
struct tcp8852_channel_cmd {
    uint8_t channel;  // Channel number
    uint16_t value;   // Parameter value
};
```

### `struct tcp8852_clock_cmd`
Structure used for clock configuration commands.

```c
struct tcp8852_clock_cmd {
    uint8_t channel;  // Channel number
    uint8_t pre_div;  // Pre-divider
    uint8_t div;      // Divider count
};
```

### `struct tcp8852_speed_cmd`
Structure used for speed configuration commands.

```c
struct tcp8852_speed_cmd {
    uint8_t channel;  // Channel number
    uint8_t pre_div;  // Pre-divider
    uint8_t div;      // Divider count
};
```

### `struct tcp8852_ircut_cmd`
Structure used for IRCUT control commands.

```c
struct tcp8852_ircut_cmd {
    uint8_t enable;   // Enable flag
    uint8_t action;   // Action type
};
```

### `struct tcp8852_channel_status`
Structure returned by `TCP8852_GET_CHANNEL_STATUS` containing channel status information.

```c
struct tcp8852_channel_status {
    uint8_t channel;      // Channel number
    uint8_t mode;         // Operating mode
    uint8_t action;       // Motion state
    uint16_t pwm;         // PWM value
    uint16_t phase_count; // Phase count
    uint16_t speed_count; // Speed count
    uint16_t cycle_count; // Cycle count
};
```

### `struct tcp8852_device_status`
Structure returned by `TCP8852_GET_DEVICE_STATUS` containing overall device status information.

```c
struct tcp8852_device_status {
    uint16_t chip_id;        // Chip ID
    uint8_t global_enable;   // Global enable status
    uint8_t low_power_mode;  // Low power mode
    uint8_t ircut_enable;    // IRCUT enable status
    uint8_t ircut_action;    // IRCUT action status
};
```

## Usage Example
Below is a simple example of how to use the TCP8852 API to control a motor channel:

```c
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include "uapi_tcp8852.h"

int main() {
    int fd;
    struct tcp8852_channel_cmd cmd;
    uint16_t chip_id;
    
    // Open the device file
    fd = open("/dev/tcp8852", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return -1;
    }
    
    // Get chip ID
    if (ioctl(fd, TCP8852_GET_CHIP_ID, &chip_id) < 0) {
        perror("Failed to get chip ID");
        close(fd);
        return -1;
    }
    printf("TCP8852 Chip ID: 0x%04X\n", chip_id);
    
    // Set channel 1 to use BLDC mode
    cmd.channel = CH1;
    cmd.value = BLDC;
    if (ioctl(fd, TCP8852_SET_CHANNEL_MODE, &cmd) < 0) {
        perror("Failed to set channel mode");
        close(fd);
        return -1;
    }
    
    // Set channel 1 to move forward
    cmd.value = FOREWARD;
    if (ioctl(fd, TCP8852_SET_CHANNEL_ACTION, &cmd) < 0) {
        perror("Failed to set channel action");
        close(fd);
        return -1;
    }
    
    // Close the device
    close(fd);
    return 0;
}
```

## Notes
- Always ensure proper error checking when using ioctl commands
- Some operations may require specific channel configurations before they can be executed
- Consult the TCP8852 datasheet for detailed information about the hardware capabilities and limitations