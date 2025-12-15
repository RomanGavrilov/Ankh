#pragma once

#include <cassert>
#include <cstdint>
#include <memory>
#include <mutex>
#include <type_traits>
#include <utility>
#include <vector>

namespace ankh
{
    class DeferredDeletionQueue
    {
      private:
        class DeferredTask
        {
          public:
            DeferredTask() = default;

            DeferredTask(const DeferredTask &) = delete;
            DeferredTask &operator=(const DeferredTask &) = delete;

            DeferredTask(DeferredTask &&) noexcept = default;
            DeferredTask &operator=(DeferredTask &&) noexcept = default;

            template <typename F> DeferredTask(F &&f)
            {
                using T = std::decay_t<F>;
                static_assert(std::is_invocable_r_v<void, T>,
                              "DeferredDeletionQueue task must be callable as void()");

                m_self = std::make_unique<Model<T>>(std::forward<F>(f));
            }

            void operator()()
            {
                if (m_self)
                    m_self->call();
            }

            explicit operator bool() const noexcept
            {
                return static_cast<bool>(m_self);
            }

          private:
            struct Concept
            {
                virtual ~Concept() = default;
                virtual void call() = 0;
            };

            template <typename T> struct Model final : Concept
            {
                explicit Model(T &&t)
                    : fn(std::move(t))
                {
                }
                explicit Model(const T &t)
                    : fn(t)
                {
                }
                void call() override
                {
                    fn();
                }
                T fn;
            };

            std::unique_ptr<Concept> m_self;
        };

      public:
        using Fn = DeferredTask;

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
                m_frames.emplace_back(std::make_unique<FrameBucket>());
        }

        uint32_t frames_in_flight() const
        {
            return static_cast<uint32_t>(m_frames.size());
        }

        // Enqueue move-only tasks safely.
        template <typename F> void enqueue(uint32_t frameIndex, F &&fn)
        {
            assert(!m_frames.empty() && "DeferredDeletionQueue used before initialization");

            frameIndex %= static_cast<uint32_t>(m_frames.size());
            auto &bucket = *m_frames[frameIndex];

            {
                std::scoped_lock lock(bucket.mutex);
                bucket.items.emplace_back(std::forward<F>(fn));
            }
        }

        template <typename F>
        void enqueue_after(uint32_t currentFrame, uint32_t delayFrames, F &&fn)
        {
            assert(!m_frames.empty());

            const uint32_t index =
                (currentFrame + delayFrames) % static_cast<uint32_t>(m_frames.size());

            enqueue(index, std::forward<F>(fn));
        }

        void flush(uint32_t frameIndex)
        {
            if (m_frames.empty())
                return;

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
                return;

            for (uint32_t i = 0; i < static_cast<uint32_t>(m_frames.size()); ++i)
                flush(i);
        }

      private:
        struct FrameBucket
        {
            std::mutex mutex;
            std::vector<Fn> items;
        };

        mutable std::mutex m_globalMutex;
        std::vector<std::unique_ptr<FrameBucket>> m_frames;
    };

} // namespace ankh
