#ifndef SRC_HPP
#define SRC_HPP

#include <stdexcept>
#include <typeinfo>
#include <typeindex>
#include <memory>

namespace sjtu {

// Forward declaration
class any_ptr;

// Base class for type erasure
class any_ptr_base {
public:
    virtual ~any_ptr_base() = default;
    virtual const std::type_info& type() const = 0;
    virtual void* data() = 0;
    virtual const void* data() const = 0;
};

// Storage class template for type-specific data
template<typename T>
class any_ptr_storage : public any_ptr_base {
private:
    T value;

public:
    template<typename... Args>
    any_ptr_storage(Args&&... args) : value(std::forward<Args>(args)...) {}

    const std::type_info& type() const override {
        return typeid(T);
    }

    void* data() override {
        return &value;
    }

    const void* data() const override {
        return &value;
    }
};

class any_ptr {
private:
    any_ptr_base* ptr;
    size_t* ref_count;

    void release() {
        if (ref_count && --(*ref_count) == 0) {
            delete ptr;
            delete ref_count;
        }
        ptr = nullptr;
        ref_count = nullptr;
    }

public:
    /**
     * @brief 默认构造函数，行为应与创建空指针类似
     */
    any_ptr() : ptr(nullptr), ref_count(nullptr) {}

    /**
     * @brief 拷贝构造函数，要求拷贝的层次为浅拷贝，即该对象与被拷贝对象的内容指向同一块内存
     * @param other
     */
    any_ptr(const any_ptr &other) : ptr(other.ptr), ref_count(other.ref_count) {
        if (ref_count) {
            ++(*ref_count);
        }
    }

    /**
     * @brief 构造函数 from pointer
     * @param ptr
     */
    template <class T>
    any_ptr(T *ptr) : ptr(new any_ptr_storage<T>(*ptr)), ref_count(new size_t(1)) {
        delete ptr; // Take ownership, so delete the original pointer
    }

    /**
     * @brief 析构函数，注意若一块内存被多个对象共享，那么只有最后一个析构的对象需要释放内存
     */
    ~any_ptr() {
        release();
    }

    /**
     * @brief 拷贝赋值运算符，要求拷贝的层次为浅拷贝，即该对象与被拷贝对象的内容指向同一块内存
     * @param other
     * @return any_ptr&
     */
    any_ptr &operator=(const any_ptr &other) {
        if (this != &other) {
            release();
            ptr = other.ptr;
            ref_count = other.ref_count;
            if (ref_count) {
                ++(*ref_count);
            }
        }
        return *this;
    }

    /**
     * @brief 赋值运算符 from pointer
     * @param ptr
     * @return any_ptr&
     */
    template <class T>
    any_ptr &operator=(T *ptr) {
        release();
        this->ptr = new any_ptr_storage<T>(*ptr);
        this->ref_count = new size_t(1);
        delete ptr; // Take ownership, so delete the original pointer
        return *this;
    }

    /**
     * @brief 获取该对象指向的值的引用
     * @note 若该对象指向的值不是 T 类型，则抛出异常 std::bad_cast
     * @tparam T
     * @return T&
     */
    template <class T>
    T &unwrap() {
        if (!ptr) {
            throw std::bad_cast();
        }
        if (ptr->type() != typeid(T)) {
            throw std::bad_cast();
        }
        return *static_cast<T*>(ptr->data());
    }

    /**
     * @brief 获取该对象指向的值的 const 参考
     * @note 若该对象指向的值不是 T 类型，则抛出异常 std::bad_cast
     * @tparam T
     * @return const T&
     */
    template <class T>
    const T &unwrap() const {
        if (!ptr) {
            throw std::bad_cast();
        }
        if (ptr->type() != typeid(T)) {
            throw std::bad_cast();
        }
        return *static_cast<const T*>(ptr->data());
    }
};

/**
 * @brief 由指定类型的值构造一个 any_ptr 对象
 * @tparam T
 * @param t
 * @return any_ptr
 */
template <class T>
any_ptr make_any_ptr(const T &t) {
    return any_ptr(new T(t));
}

/**
 * @brief 由 initializer_list construct any_ptr
 * @tparam T
 * @tparam U
 * @param il
 * @return any_ptr
 */
template <class T, class U>
any_ptr make_any_ptr(std::initializer_list<U> il) {
    return any_ptr(new T(il));
}

/**
 * @brief 由 specified arguments construct any_ptr
 * @tparam T
 * @tparam Args
 * @param args
 * @return any_ptr
 */
template <class T, class... Args>
any_ptr make_any_ptr(Args&&... args) {
    return any_ptr(new T(std::forward<Args>(args)...));
}

}  // namespace sjtu

#endif
