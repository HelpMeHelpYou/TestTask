# Register bank

This file describes the current median-device register bank and its image descriptor. The common code definitions are in [`include/register_map.h`](include/register_map.h), used by both host and model.

## Representation

Each register is one `uintptr_t` value. Addresses are native pointers within the same process. The logical offsets below are spaced eight bytes apart; they do not describe the physical layout of a packed MMIO structure. The register array has six slots, indexed by `offset / kRegisterSpacing`.

Only listed offsets are valid. The simple register accessor assumes a valid `Register` value; it does not validate arbitrary offsets. There are no fields narrower than a register and no bit masks or byte-order conversion in this simulation.

All registers start at zero when a device is constructed. There is no reset command; the same instance can accept successive completed jobs.

## Register table

Access is from the host's perspective. Configuration registers retain their values after completion. The host sets them before submitting each job.

| Offset | Register | Access | Initial value | Meaning and valid values |
| --- | --- | --- | ---: | --- |
| `0x00` | `InputAddress` | Read/write | 0 | Address of the first input byte; non-null before START. |
| `0x08` | `OutputAddress` | Read/write | 0 | Address of the first output byte; non-null and non-overlapping with input. |
| `0x10` | `ParamsAddress` | Read/write | 0 | Address of the image descriptor below; non-null before START. |
| `0x18` | `Radius` | Read/write | 0 | Median radius, integer 0 through 8 inclusive. Window size is `(2 * Radius + 1)^2`. |
| `0x20` | `Command` | Write | 0 | Writing 1 requests a job and signals the model thread. Other values produce an invalid-argument completion. |
| `0x28` | `Status` | Read | 0 | Model result: 0 = Idle (initial state), 1 = Done, 2 = InvalidArgument. Read only after the completion interrupt. |
| `0x30` | `FilterSelect` | Read/write | 0 | 0 = median, 1 = Sobel. Other values are invalid. |
| `0x38` | `EdgeGain` | Read/write | 0 | Represents real-valued gain in [0.0, 1.0] using an unsigned integer code. Byte presentation is real value myltiplyed by 1 << 10 (Q22.10) |


Access permissions describe the protocol; the small accessor methods do not enforce read-only or write-only permissions.

## Image descriptor

`ParamsAddress` points to `ImageParams` from [`include/median_filter.h`](include/median_filter.h). It is a native C structure shared by the two threads, not serialized data.

| Field | Type | Meaning |
| --- | --- | --- |
| `width` | `int` | Visible pixels per row, 1 through 4096. |
| `height` | `int` | Visible rows, 1 through 4096. |
| `input_stride` | `int` | Bytes between input rows; at least `width`. |
| `output_stride` | `int` | Bytes between output rows; at least `width`. |

Pixels are unsigned 8-bit monochrome values. Input and output strides may differ. Each buffer contains at least `(height - 1) * stride + width` bytes. Only visible output pixels are written; padding and trailing bytes remain unchanged. Input remains unchanged.

The host supplies valid allocations, non-overlapping buffers, and a valid descriptor. Basic checks cover null pointers, equal input/output pointers, dimensions, strides, radius, and command. The model does not validate arbitrary addresses, allocation lengths, or partial overlaps.

