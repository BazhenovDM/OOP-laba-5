#pragma once
#include <memory_resource>
#include <list>
#include <cstddef>
#include <new>
#include <cstdint>

class FixedListMemoryResource : public std::pmr::memory_resource {
    struct Block { void* ptr; std::size_t size; };
    char* buffer_;
    std::size_t capacity_;
    char* cursor_;
    char* limit_;
    std::list<Block> allocated_;
    std::list<Block> freed_;
protected:
    void* do_allocate(std::size_t bytes, std::size_t alignment) override {
        if (bytes == 0) bytes = 1;
        for (auto it = freed_.begin(); it != freed_.end(); ++it) {
            if ((reinterpret_cast<std::uintptr_t>(it->ptr) % alignment) == 0 && it->size >= bytes) {
                void* res = it->ptr;
                allocated_.push_back({res, bytes});
                freed_.erase(it);
                return res;
            }
        }
        void* p = cursor_;
        std::size_t space = static_cast<std::size_t>(limit_ - cursor_);
        if (std::align(alignment, bytes, p, space)) {
            void* out = p;
            cursor_ = static_cast<char*>(p) + bytes;
            allocated_.push_back({out, bytes});
            return out;
        }
        throw std::bad_alloc();
    }
    void do_deallocate(void* p, std::size_t bytes, std::size_t) override {
        if (!p) return;
        for (auto it = allocated_.begin(); it != allocated_.end(); ++it) {
            if (it->ptr == p) {
                freed_.push_back(*it);
                allocated_.erase(it);
                return;
            }
        }
    }
    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }
public:
    explicit FixedListMemoryResource(std::size_t total_bytes)
        : buffer_(nullptr), capacity_(total_bytes), cursor_(nullptr), limit_(nullptr) {
        buffer_ = static_cast<char*>(::operator new(total_bytes));
        cursor_ = buffer_;
        limit_ = buffer_ + total_bytes;
    }
    ~FixedListMemoryResource() override {
        allocated_.clear();
        freed_.clear();
        ::operator delete(buffer_);
    }
    std::size_t used() const noexcept {
        return static_cast<std::size_t>(cursor_ - buffer_);
    }
    std::size_t capacity() const noexcept { return capacity_; }
};
