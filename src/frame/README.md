FrameAllocator
 └─ owns 1 persistently mapped Buffer
 └─ sub-allocates per-frame slices
 └─ offsets are tracked per FrameSlot
 └─ GPU reuse is gated by explicit GpuSignal