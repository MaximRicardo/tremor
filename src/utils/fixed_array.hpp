#pragma once

#include <cstddef>
#include <memory>
#include <utility>

// allocates a fixed-size array on the heap
// requires that T has a default constructor
// no need for a destructor for the rule of 5 stuff cuz the unique_ptr frees
// itself.
template <typename T> class FixedArray {

    std::unique_ptr<T[]> elems;
    size_t n_elems = 0;

    FixedArray() = default;

public:
    explicit FixedArray(size_t size) : elems(new T[size]), n_elems(size) {};

    FixedArray(const FixedArray &other) : n_elems(other.n_elems)
    {
        for (auto &elem : other) {
            this->elems[&elem - other.data()] = elem;
        }
    }

    FixedArray(FixedArray &&other) : FixedArray()
    {
        swap(*this, other);
    }

    FixedArray &operator=(FixedArray other)
    {
        swap(*this, other);
        return *this;
    }

    T &operator[](size_t idx)
    {
        return this->elems[idx];
    }

    const T &operator[](size_t idx) const
    {
        return this->elems[idx];
    }

    T *data()
    {
        return this->elems.get();
    }

    const T *data() const
    {
        return this->elems.get();
    }

    size_t size() const
    {
        return this->n_elems;
    }

    T *begin()
    {
        return this->elems.get();
    }

    const T *begin() const
    {
        return this->elems.get();
    }

    T *end()
    {
        return &this->elems[this->n_elems];
    }

    const T *end() const
    {
        return &this->elems[this->n_elems];
    }

    bool empty() const
    {
        return this->n_elems == 0;
    }

    static void swap(FixedArray &a, FixedArray &b)
    {
        std::swap(a.elems, b.elems);
        std::swap(a.n_elems, b.n_elems);
    }
};
