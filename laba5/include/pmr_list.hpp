#pragma once
#include <memory_resource>
#include <memory>
#include <cstddef>
#include <iterator>
#include <type_traits>

template<typename T>
class pmr_list {
    struct Node {
        T value;
        Node* prev;
        Node* next;
        template<typename U>
        Node(U&& v): value(std::forward<U>(v)), prev(nullptr), next(nullptr) {}
    };
    using NodeAlloc = std::pmr::polymorphic_allocator<Node>;
    NodeAlloc alloc_;
    Node* head_;
    Node* tail_;
    std::size_t sz_;
    using Traits = std::allocator_traits<NodeAlloc>;
public:
    struct iterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using reference = T&;
        using pointer = T*;
        using difference_type = std::ptrdiff_t;
    private:
        Node* cur_;
    public:
        iterator() : cur_(nullptr) {}
        explicit iterator(Node* n) : cur_(n) {}
        reference operator*() const { return cur_->value; }
        pointer operator->() const { return std::addressof(cur_->value); }
        iterator& operator++() { if (cur_) cur_ = cur_->next; return *this; }
        iterator operator++(int) { iterator tmp = *this; ++*this; return tmp; }
        bool operator==(const iterator& o) const { return cur_ == o.cur_; }
        bool operator!=(const iterator& o) const { return cur_ != o.cur_; }
        Node* node() const { return cur_; }
    };
    pmr_list(std::pmr::memory_resource* mr = std::pmr::get_default_resource())
        : alloc_(mr), head_(nullptr), tail_(nullptr), sz_(0) {}
    ~pmr_list() { clear(); }
    bool empty() const noexcept { return sz_ == 0; }
    std::size_t size() const noexcept { return sz_; }
    iterator begin() noexcept { return iterator(head_); }
    iterator end() noexcept { return iterator(nullptr); }
    void push_back(const T& v) { emplace_back(v); }
    void push_back(T&& v) { emplace_back(std::move(v)); }
    template<typename... Args>
    void emplace_back(Args&&... args) {
        Node* n = alloc_.allocate(1);
        Traits::construct(alloc_, n, std::forward<Args>(args)...);
        n->prev = tail_;
        n->next = nullptr;
        if (tail_) tail_->next = n;
        tail_ = n;
        if (!head_) head_ = n;
        ++sz_;
    }
    void pop_back() {
        if (!tail_) return;
        Node* rem = tail_;
        tail_ = rem->prev;
        if (tail_) tail_->next = nullptr; else head_ = nullptr;
        Traits::destroy(alloc_, rem);
        alloc_.deallocate(rem, 1);
        --sz_;
    }
    void push_front(const T& v) { emplace_front(v); }
    void push_front(T&& v) { emplace_front(std::move(v)); }
    template<typename... Args>
    void emplace_front(Args&&... args) {
        Node* n = alloc_.allocate(1);
        Traits::construct(alloc_, n, std::forward<Args>(args)...);
        n->prev = nullptr;
        n->next = head_;
        if (head_) head_->prev = n;
        head_ = n;
        if (!tail_) tail_ = n;
        ++sz_;
    }
    void pop_front() {
        if (!head_) return;
        Node* rem = head_;
        head_ = rem->next;
        if (head_) head_->prev = nullptr; else tail_ = nullptr;
        Traits::destroy(alloc_, rem);
        alloc_.deallocate(rem, 1);
        --sz_;
    }
    void clear() {
        Node* cur = head_;
        while (cur) {
            Node* nx = cur->next;
            Traits::destroy(alloc_, cur);
            alloc_.deallocate(cur, 1);
            cur = nx;
        }
        head_ = tail_ = nullptr;
        sz_ = 0;
    }
    iterator erase(iterator it) {
        Node* n = it.node();
        if (!n) return end();
        Node* nx = n->next;
        Node* pv = n->prev;
        if (pv) pv->next = nx; else head_ = nx;
        if (nx) nx->prev = pv; else tail_ = pv;
        Traits::destroy(alloc_, n);
        alloc_.deallocate(n, 1);
        --sz_;
        return iterator(nx);
    }
};
