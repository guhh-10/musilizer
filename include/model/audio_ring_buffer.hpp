#pragma once
#include <array>
#include <atomic>
#include <algorithm>
#include <cstring>

template<int SIZE = 4096>
class AudioRingBuffer {
    static_assert((SIZE & (SIZE - 1)) == 0, "SIZE must be a power of two");
    
    private:
        static constexpr int MASK = SIZE - 1;

        std::array<float, SIZE> buf_{};
        std::atomic<int>        write_cursor_{0};
    
    public:
        void write(const float* samples, int count) {
            int to_write = std::min(count, SIZE);
            int start    = count - to_write;

            for (int i = 0; i < to_write; ++i)
                buf_[(write_cursor_.load(std::memory_order_relaxed) + i) & MASK] = samples[start + i];

            write_cursor_.fetch_add(to_write, std::memory_order_release);
        }

        int read_latest(float* out, int count) const {
            int written = write_cursor_.load(std::memory_order_acquire);
            int available = std::min(written, SIZE);
            int to_copy   = std::min(count, available);

        
            if (to_copy < count)
                std::memset(out, 0, (count - to_copy) * sizeof(float));

            int src_end = written;
            int src_start = src_end - to_copy;
            float* dst = out + (count - to_copy);

            for (int i = 0; i < to_copy; ++i)
                dst[i] = buf_[(src_start + i) & MASK];

            return to_copy;
        }
};