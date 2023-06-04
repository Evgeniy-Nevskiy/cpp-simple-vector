#pragma once

#include <stdexcept>
#include <algorithm>
#include <cassert>
#include <cstdlib>

template <typename Type>
class ArrayPtr
{
public:
    ArrayPtr() = default;

    explicit ArrayPtr(size_t size)
    {
        if (size)
        {
            simple_vector_ = new Type[size]();
        }
        else
        {
            simple_vector_ = nullptr;
        }
    }

    ArrayPtr(const ArrayPtr &) = delete;

    ArrayPtr &operator=(const ArrayPtr &) = delete;

    ArrayPtr(ArrayPtr &&other)
    {
        std::swap(simple_vector_, other.simple_vector_);
        other.simple_vector_ = nullptr;
    }

    ArrayPtr &operator=(ArrayPtr &&other)
    {
        if (this != &other)
        {
            std::swap(simple_vector_, other.simple_vector_);
        }
        return *this;
    }

    ~ArrayPtr()
    {
        if (simple_vector_)
        {
            delete[] simple_vector_;
        }
    }

    [[nodiscard]] Type *Release() noexcept
    {
        Type *helper = simple_vector_;
        simple_vector_ = nullptr;
        return helper;
    }

    Type *Get() const noexcept
    {
        return simple_vector_;
    }

    Type &operator[](size_t index) noexcept
    {
        return simple_vector_[index];
    }

    const Type &operator[](size_t index) const noexcept
    {
        return simple_vector_[index];
    }

    explicit operator bool() const
    {
        return simple_vector_;
    }

    void swap(ArrayPtr &other) noexcept
    {
        std::swap(simple_vector_, other.simple_vector_);
    }

private:
    Type *simple_vector_ = nullptr;
};
