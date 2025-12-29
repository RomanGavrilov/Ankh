#pragma once
#include "utils/gpu-retirement-queue.hpp"
#include <memory>
#include <vector>

namespace ankh
{
    template <class T> inline void retire_owned(GpuRetirementQueue &q, GpuSignal s, T obj)
    {
        q.retire_after(s, [o = std::move(obj)]() mutable {});
    }

    template <class T>
    inline void retire_owned(GpuRetirementQueue &q, GpuSignal s, std::unique_ptr<T> &&p)
    {
        q.retire_after(s, [pp = std::move(p)]() mutable {});
    }

    template <class HandleT, class DestroyFn>
    inline void retire_handles(GpuRetirementQueue &q,
                               GpuSignal s,
                               std::vector<HandleT> &&handles,
                               DestroyFn destroy)
    {
        q.retire_after(s,
                       [hs = std::move(handles), destroy = std::move(destroy)]() mutable
                       {
                           for (auto h : hs)
                               if (h)
                               {
                                   destroy(h);
                               }
                       });
    }

} // namespace ankh
