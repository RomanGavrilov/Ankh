#pragma once

#include "utils/gpu-resource-tracker.hpp"
#include <cstdint>
#include <type_traits>

namespace ankh
{
#ifndef NDEBUG
    template <typename HandleT> inline uint64_t gpu_handle_u64(HandleT h) noexcept
    {
        if constexpr (std::is_pointer_v<HandleT>)
        {
            return reinterpret_cast<uint64_t>(h);
        }
        else
        {
            return static_cast<uint64_t>(h);
        }
    }

    inline void gpu_track_create(GpuResourceTracker *t,
                                 const char *type,
                                 uint64_t handle,
                                 const char *name,
                                 const char *file,
                                 int line)
    {
        if (t)
        {
            t->on_create(type, handle, name, file, line);
        }
    }

    inline void gpu_track_destroy(GpuResourceTracker *t, uint64_t handle)
    {
        if (t)
        {
            t->on_destroy(handle);
        }
    }

#define ANKH_GPU_TRACK_CREATE(trackerPtr, typeStr, handle, nameStr)                                \
    ::ankh::gpu_track_create((trackerPtr),                                                         \
                             (typeStr),                                                            \
                             ::ankh::gpu_handle_u64((handle)),                                     \
                             (nameStr),                                                            \
                             __FILE__,                                                             \
                             __LINE__)

#define ANKH_GPU_TRACK_DESTROY(trackerPtr, handle)                                                 \
    ::ankh::gpu_track_destroy((trackerPtr), ::ankh::gpu_handle_u64((handle)))

#else
#define ANKH_GPU_TRACK_CREATE(...) ((void)0)
#define ANKH_GPU_TRACK_DESTROY(...) ((void)0)
#endif
} // namespace ankh
