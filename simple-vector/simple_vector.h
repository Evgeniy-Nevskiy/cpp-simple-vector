#pragma once

#include <initializer_list>
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <cassert>
#include <iterator>
#include "array_ptr.h"

class ReserveProxyObj
{
public:
    explicit ReserveProxyObj(size_t capacity_to_reserve) : capacity_(capacity_to_reserve) {}
    size_t Reserve_capacity()
    {
        return capacity_;
    }

private:
    size_t capacity_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve)
{
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector
{
public:
    using Iterator = Type *;
    using ConstIterator = const Type *;

    SimpleVector() noexcept = default;

    explicit SimpleVector(size_t size) : SimpleVector(size, std::move(Type{})) {}

    SimpleVector(size_t size, const Type &value) : simple_vector_(size), size_(size), capacity_(size)
    {
        std::fill(begin(), end(), value);
    }

    SimpleVector(std::initializer_list<Type> init) : simple_vector_(init.size()), size_(init.size()), capacity_(init.size())
    {
        std::copy(std::make_move_iterator(init.begin()), std::make_move_iterator(init.end()), begin());
    }

    SimpleVector(ReserveProxyObj capacity_to_reserve)
    {
        Reserve(capacity_to_reserve.Reserve_capacity());
    }

    SimpleVector(const SimpleVector &other) : size_(other.GetSize()), capacity_(other.GetCapacity())
    {
        ArrayPtr<Type> helper(other.size_);
        std::copy(other.begin(), other.end(), &helper[0]);
        simple_vector_.swap(helper);
    }

    SimpleVector(SimpleVector &&other) : simple_vector_(other.size_)
    {
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
        simple_vector_ = std::move(other.simple_vector_);
    }

    size_t GetSize() const noexcept
    {
        return size_;
    }

    size_t GetCapacity() const noexcept
    {
        return capacity_;
    }

    bool IsEmpty() const noexcept
    {
        return (size_ == 0);
    }

    Type &operator[](size_t index) noexcept
    {
        assert(index < size_);
        return simple_vector_[index];
    }

    const Type &operator[](size_t index) const noexcept
    {
        assert(index < size_);
        return simple_vector_[index];
    }

    Type &At(size_t index)
    {
        if (index < size_)
        {
            return simple_vector_[index];
        }
        else
        {
            throw std::out_of_range("Non-existent vector element.");
        }
    }

    const Type &At(size_t index) const
    {
        if (index < size_)
        {
            return simple_vector_[index];
        }
        else
        {
            throw std::out_of_range("Non-existent vector element.");
        }
    }

    void Clear() noexcept
    {
        size_ = 0;
    }

    void Resize(size_t new_size)
    {
        if (size_ >= new_size)
        {
            size_ = new_size;
            return;
        }
        else if (size_ < new_size && capacity_ > new_size)
        {
            for (auto iter = begin() + new_size; iter != end(); --iter)
            {
                *iter = std::move(Type{});
            }
            size_ = new_size;
            return;
        }
        else
        {
            ArrayPtr<Type> helper(new_size);
            std::move(begin(), end(), &helper[0]);
            simple_vector_.swap(helper);
            size_ = new_size;
            capacity_ = new_size * 2;
        }
    }

    Iterator begin() noexcept
    {
        return simple_vector_.Get();
    }

    Iterator end() noexcept
    {
        return simple_vector_.Get() + size_;
    }

    ConstIterator begin() const noexcept
    {
        return cbegin();
    }

    ConstIterator end() const noexcept
    {
        return cend();
    }

    ConstIterator cbegin() const noexcept
    {
        return ConstIterator(&simple_vector_[0]);
    }

    ConstIterator cend() const noexcept
    {
        return ConstIterator(&simple_vector_[size_]);
    }

    SimpleVector &operator=(const SimpleVector &rhs)
    {
        if (this != &rhs)
        {
            SimpleVector helper(rhs);
            swap(helper);
        }
        return *this;
    }

    void PushBack(const Type &item)
    {
        if (size_ < capacity_)
        {
            simple_vector_[size_] = item;
        }
        else
        {
            // если capacity_ будет 0
            auto new_capacity = std::max(size_t(1), 2 * capacity_);
            ArrayPtr<Type> helper(new_capacity);
            std::copy(&simple_vector_[0], &simple_vector_[size_], &helper[0]);
            helper[size_] = item;
            simple_vector_.swap(helper);
            capacity_ = new_capacity;
        }
        ++size_;
    }

    void PushBack(Type &&item)
    {
        if (size_ < capacity_)
        {
            simple_vector_[size_] = std::move(item);
        }
        else
        {
            // если capacity_ будет 0
            auto new_capacity = std::max(size_t(1), 2 * capacity_);
            ArrayPtr<Type> helper(new_capacity);
            std::move(&simple_vector_[0], &simple_vector_[size_], &helper[0]);
            helper[size_] = std::move(item);
            simple_vector_.swap(helper);
            capacity_ = new_capacity;
        }
        ++size_;
    }

    Iterator Insert(ConstIterator pos, const Type &value)
    {
        assert(pos >= cbegin() && pos <= cend());
        auto pos_element = std::distance(cbegin(), pos);
        if (size_ < capacity_)
        {
            std::copy_backward(pos, cend(), &simple_vector_[(size_ + 1)]);
            simple_vector_[pos_element] = value;
        }
        else
        {
            // если capacity_ будет 0
            auto new_capacity = std::max(size_t(1), 2 * capacity_);
            ArrayPtr<Type> helper(new_capacity);
            std::copy(&simple_vector_[0], &simple_vector_[pos_element], &helper[0]);
            std::copy_backward(pos, cend(), &helper[(size_ + 1)]);
            helper[pos_element] = value;
            simple_vector_.swap(helper);
            capacity_ = new_capacity;
        }
        ++size_;
        return Iterator{&simple_vector_[pos_element]};
    }

    Iterator Insert(ConstIterator pos, Type &&value)
    {
        assert(pos >= cbegin() && pos <= cend());
        auto no_const_pos = const_cast<Iterator>(pos);
        auto pos_element = std::distance(begin(), no_const_pos);
        if (size_ < capacity_)
        {
            std::move_backward(no_const_pos, end(), &simple_vector_[(size_ + 1)]);
            simple_vector_[pos_element] = std::move(value);
        }
        else
        {
            // если capacity_ будет 0
            auto new_capacity = std::max(size_t(1), 2 * capacity_);
            ArrayPtr<Type> helper(new_capacity);
            std::move(&simple_vector_[0], &simple_vector_[pos_element], &helper[0]);
            std::move_backward(no_const_pos, end(), &helper[(size_ + 1)]);
            helper[pos_element] = std::move(value);
            simple_vector_.swap(helper);
            capacity_ = new_capacity;
        }
        ++size_;
        return Iterator{&simple_vector_[pos_element]};
    }

    void PopBack() noexcept
    {
        if (size_)
        {
            --size_;
        }
    }

    Iterator Erase(ConstIterator pos)
    {
        assert(pos >= cbegin() && pos < cend());
        auto no_const_pos = const_cast<Iterator>(pos);
        auto pos_element = std::distance(begin(), no_const_pos);
        std::move(++no_const_pos, end(), &simple_vector_[pos_element]);
        --size_;
        return &simple_vector_[pos_element];
    }

    void swap(SimpleVector &other) noexcept
    {
        simple_vector_.swap(other.simple_vector_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Reserve(size_t new_capacity)
    {
        if (new_capacity > capacity_)
        {
            ArrayPtr<Type> helper(new_capacity);
            std::copy(begin(), end(), &helper[0]);
            simple_vector_.swap(helper);
            capacity_ = new_capacity;
        }
        else
        {
            return;
        }
    }

private:
    ArrayPtr<Type> simple_vector_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    if (lhs.GetSize() != rhs.GetSize())
        return false;
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    return !(lhs < rhs);
}
