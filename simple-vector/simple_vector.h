#pragma once

#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include "array_ptr.h"

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity_to_reserve)
    :capacity_to_reserve_(capacity_to_reserve) {
    }
    
    size_t Get() {
        return capacity_to_reserve_;
    }
private:
    size_t capacity_to_reserve_ = 0;
};
 
ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;
    
    SimpleVector(ReserveProxyObj cap)
    :items_(nullptr), size_(0), capacity_(cap.Get()) {
    }

    explicit SimpleVector(size_t size)
    :items_(size), size_(size), capacity_(size) {
        std::generate(items_.Get(), items_.Get() + size, [](){return Type();});
    }

    SimpleVector(size_t size, const Type& value)
    :items_(size), size_(size), capacity_(size) {
        std::fill(items_.Get(), items_.Get() + size, value);
    }

    SimpleVector(std::initializer_list<Type> init) 
    :items_(init.size()), size_(init.size()), capacity_(init.size()) {
        ArrayPtr<Type> temp(init.size());
        std::copy(init.begin(), init.end(), temp.Get());
        items_.swap(temp);
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0/* ? true : false*/;
    }

    Type& operator[](size_t index) noexcept {
        return items_[index];
    }

    const Type& operator[](size_t index) const noexcept {     
        return items_[index];
    }

    Type& At(size_t index) {
        if (index < size_) {
            return items_[index];
        } 
        else {
            throw std::out_of_range("out_of_range");
        }
    }

    const Type& At(size_t index) const {
        if (index < size_) {
            return items_[index];
        } 
        else {
            throw std::out_of_range("out_of_range");
        }
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        }
        else if (new_size <= capacity_) {
            std::generate(items_.Get() + size_, items_.Get() + new_size, [](){return Type();});
            size_ = new_size;
        }
        else {      
            ArrayPtr<Type> temp(new_size);
            for (size_t i = 0; i < size_; ++i) {
                temp[i] = std::move(items_[i]);
                // в данном случае использование Reserve вызывает ошибки, связанные с копированием. Прошу согласовать в таком виде.
            }

            std::generate(temp.Get() + size_, temp.Get() + new_size, [](){return Type();});
            items_.swap(temp);    
            size_ = new_size;
            capacity_ = new_size;
        }   
    }
    
    void Reserve(size_t new_capacity) {
        if (capacity_ < new_capacity) {           
            ArrayPtr<Type> temp(new_capacity);
            std::move(items_.Get(), items_.Get() + size_, temp.Get());
            items_.swap(temp);
            capacity_ = new_capacity;
        }
    }

    Iterator begin() noexcept {
        return items_.Get();
    }

    Iterator end() noexcept {
        return items_.Get() + size_;
    }

    ConstIterator begin() const noexcept {
        return items_.Get();
    }

    ConstIterator end() const noexcept {
        return items_.Get() + size_;
    }

    ConstIterator cbegin() const noexcept {
        return items_.Get();
    }

    ConstIterator cend() const noexcept {
        return items_.Get() + size_;
    }
    
    SimpleVector(const SimpleVector& other) 
    :items_(other.size_), size_(other.size_), capacity_(other.size_) {
        std::copy(other.begin(), other.end(), begin());
    }
    
    SimpleVector(SimpleVector&& other) {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        items_.swap(other.items_);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            Resize(rhs.GetSize());
            auto temp = rhs;
            items_.swap(temp.items_);
        }
        return *this;
    }

	void PushBack(const Type& item) {
        if (capacity_ == 0 || capacity_ == size_) {
            Reserve(std::max(static_cast<size_t>(1), capacity_ * 2));
        }
        *end() = item;
        ++size_;
    }

    void PushBack(Type&& item) {
        if (capacity_ == 0 || capacity_ == size_) {
            Reserve(std::max(static_cast<size_t>(1), capacity_ * 2));
        }
        *end() = std::move(item);
        ++size_;
    }
        
    Iterator Insert(Iterator pos, const Type& value) {
        if (pos < begin() || pos > end()) {
            return nullptr;
        }
        size_t num = pos - begin();
        if (capacity_ == 0 || capacity_ == size_) {
            Reserve(std::max<size_t>(1, capacity_ * 2));
        }
        Iterator pos_res = items_.Get() + num;
        std::move_backward(pos_res, end(), end() + 1);
        ++size_;
        *pos_res = value;
        return pos_res;
    }
    
    Iterator Insert(Iterator pos, Type&& value) {
        if (pos < begin() || pos > end()) {
            return nullptr;
        }
        size_t num = pos - begin();
        if (capacity_ == 0 || capacity_ == size_) {
            Reserve(std::max<size_t>(1, capacity_ * 2));
        }
        Iterator pos_res = items_.Get() + num;
        std::move_backward(pos_res, end(), end() + 1);
        ++size_;
        *pos_res = std::move(value);
        
        return pos_res;
    }
    
    void PopBack() noexcept {
        if (size_) {
            --size_;
        }
        else std::cerr << "No elements";
    }

    Iterator Erase(ConstIterator pos) {
        if (pos < begin() || pos > end()) {
            return nullptr;
        }
        size_t pos_n = std::distance(begin(), const_cast<Iterator>(pos));
        for (size_t i = pos_n + 1; i < size_; ++i) {
            items_[i - 1] = std::move(items_[i]);
        }
        --size_;
        return begin() + pos_n;
    }

    void swap(SimpleVector& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        items_.swap(other.items_);
    }
    
private:
    ArrayPtr<Type> items_;

    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
 
template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}
 
template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
 
template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (!(lhs > rhs));
}
 
template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}
 
template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs <= lhs;
}