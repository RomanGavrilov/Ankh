#pragma once

#include <cassert>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace ankh
{
    class DeferredDeletionQueue
    {
      public:
        using Fn = std::function<void()>;

        explicit DeferredDeletionQueue(uint32_t framesInFlight = 0)
        {
            reset(framesInFlight);
        }

        DeferredDeletionQueue(const DeferredDeletionQueue &) = delete;
        DeferredDeletionQueue &operator=(const DeferredDeletionQueue &) = delete;
        DeferredDeletionQueue(DeferredDeletionQueue &&) = delete;
        DeferredDeletionQueue &operator=(DeferredDeletionQueue &&) = delete;

        ~DeferredDeletionQueue()
        {
#ifndef NDEBUG
            for (const auto &frame : m_frames)
            {
                assert(frame && frame->items.empty() &&
                       "DeferredDeletionQueue destroyed with pending GPU deletions");
            }
#endif
        }

        void reset(uint32_t framesInFlight)
        {
            std::scoped_lock lock(m_globalMutex);

#ifndef NDEBUG
            for (const auto &frame : m_frames)
            {
                assert(frame && frame->items.empty() &&
                       "DeferredDeletionQueue::reset() called with pending deletions");
            }
#endif

            m_frames.clear();
            m_frames.reserve(framesInFlight);

            for (uint32_t i = 0; i < framesInFlight; ++i)
            {
                m_frames.emplace_back(std::make_unique<FrameBucket>());
            }
        }

        uint32_t frames_in_flight() const
        {
            return static_cast<uint32_t>(m_frames.size());
        }

        void enqueue(uint32_t frameIndex, Fn fn)
        {
#ifndef NDEBUG
            assert(!m_frames.empty() && "DeferredDeletionQueue used before initialization");
#endif
            frameIndex %= static_cast<uint32_t>(m_frames.size());
            auto &bucket = *m_frames[frameIndex];

            {
                std::scoped_lock lock(bucket.mutex);
                bucket.items.emplace_back(std::move(fn));
            }
        }

        void enqueue_after(uint32_t currentFrame, uint32_t delayFrames, Fn fn)
        {
#ifndef NDEBUG
            assert(!m_frames.empty());
#endif
            const uint32_t index =
                (currentFrame + delayFrames) % static_cast<uint32_t>(m_frames.size());

            enqueue(index, std::move(fn));
        }

        void flush(uint32_t frameIndex)
        {
            if (m_frames.empty())
            {
                return;
            }

            frameIndex %= static_cast<uint32_t>(m_frames.size());
            auto &bucket = *m_frames[frameIndex];

            std::vector<Fn> toExecute;
            {
                std::scoped_lock lock(bucket.mutex);
                toExecute.swap(bucket.items);
            }

            for (auto &fn : toExecute)
            {
                fn();
            }
        }

        void flush_all()
        {
            if (m_frames.empty())
            {
                return;
            }

            for (uint32_t i = 0; i < static_cast<uint32_t>(m_frames.size()); ++i)
            {
                flush(i);
            }
        }

      private:
        struct FrameBucket
        {
            FrameBucket() = default;
            FrameBucket(const FrameBucket &) = delete;
            FrameBucket &operator=(const FrameBucket &) = delete;

            std::mutex mutex;
            std::vector<Fn> items;
        };

        mutable std::mutex m_globalMutex;
        std::vector<std::unique_ptr<FrameBucket>> m_frames;
    };

} // namespace ankh
