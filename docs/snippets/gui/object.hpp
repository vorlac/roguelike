#pragma once

#include <atomic>

#include "gui/common.hpp"

namespace rl::gui {

    /**
     * \class Object object.h nanogui/object.h
     *
     * \brief Object base class with intrusive reference counting
     *
     * The Object class provides a convenient foundation of a class hierarchy that
     * will ease lifetime and ownership-related issues whenever Python bindings are
     * involved.
     *
     * Internally, its constructor sets the `m_state` field to `1`, which indicates
     * that the instance is owned by C++. Bits 2..63 of this field are used to
     * store the actual reference count value. The `inc_ref()` and `dec_ref()`
     * functions can be used to increment or decrement this reference count. When
     * `dec_ref()` removes the last reference, the instance will be deallocated
     * using a `delete` expression handled using a polymorphic destructor.
     *
     * When a subclass of `Object` is constructed to Python or returned from C++ to
     * Python, nanobind will invoke `Object::set_self_py()`, which hands ownership
     * over to Python/nanobind. Any remaining references will be moved from the
     * `m_state` field to the Python reference count. In this mode, `inc_ref()` and
     * `dec_ref()` wrap Python reference counting primitives (`Py_INCREF()` /
     * `Py_DECREF()`) which must be made available by calling the function
     * `object_init_py` once during module initialization.
     *
     * Within C++, the RAII helper class `ref` (defined below) can be used to keep
     * instances alive. This removes the need to call the `inc_ref()` / `dec_ref()`
     * functions explicitly.
     *
     * ```
     * {
     *    ref<MyClass> inst = new MyClass();
     *    inst->my_function();
     *    ...
     * } // end of scope, 'inst' automatically deleted if no longer referenced
     * ```
     *
     * A separate optional file ``object_py.h`` provides a nanobind type caster
     * to bind functions taking/returning values of type `ref<T>`.
     */
    class Object
    {
    public:
        /// Default constructor
        Object() = default;

        // Polymorphic default destructor
        virtual ~Object();

        /* The following move/assignment constructors/operators are no-ops. They
           intentionally do not change the reference count field (m_state) that
           is associated with a fixed address in memory */
        Object(const Object&)
            : Object()
        {
        }

        Object(Object&&)
            : Object()
        {
        }

        Object& operator=(const Object&)
        {
            return *this;
        }

        Object& operator=(Object&&)
        {
            return *this;
        }

        /// Increase the object's reference count
        void inc_ref() const noexcept;

        /// Decrease the object's reference count and potentially deallocate it
        void dec_ref() const noexcept;

    private:
        mutable std::atomic<uintptr_t> m_state{ 1 };
    };

    /**
     * \class ref object.h nanogui/object.h
     *
     * \brief Reference counting helper.
     *
     * ``ref`` is a simple RAII wrapper class that stores a pointer to a subclass
     * of ``Object``. It takes care of increasing and decreasing the reference
     * count of the underlying instance. When the last reference goes out of scope,
     * the associated object will be deallocated.
     *
     * The advantage over C++ solutions such as ``std::shared_ptr`` is that
     * the reference count is very compactly integrated into the base object
     * itself.
     */
    template <typename T>
    class ref
    {
    public:
        /// Create a ``nullptr``-valued reference
        ref()
            : m_ptr(nullptr)
        {
        }

        /// Construct a reference from a pointer
        ref(T* ptr)
            : m_ptr(ptr)
        {
            if (m_ptr)
                ((Object*)m_ptr)->inc_ref();
        }

        /// Copy constructor
        ref(const ref& r)
            : m_ptr(r.m_ptr)
        {
            if (m_ptr)
                ((Object*)m_ptr)->inc_ref();
        }

        /// Move constructor
        ref(ref&& r) noexcept
            : m_ptr(r.m_ptr)
        {
            r.m_ptr = nullptr;
        }

        /// Destroy this reference
        ~ref()
        {
            if (m_ptr)
                ((Object*)m_ptr)->dec_ref();
        }

        /// Move another reference into the current one
        ref& operator=(ref&& r) noexcept
        {
            if (m_ptr)
                ((Object*)m_ptr)->dec_ref();
            m_ptr = r.m_ptr;
            r.m_ptr = nullptr;
            return *this;
        }

        /// Overwrite this reference with another reference
        ref& operator=(const ref& r)
        {
            if (r.m_ptr)
                ((Object*)r.m_ptr)->inc_ref();
            if (m_ptr)
                ((Object*)m_ptr)->dec_ref();
            m_ptr = r.m_ptr;
            return *this;
        }

        /// Overwrite this reference with a pointer to another object
        ref& operator=(T* ptr)
        {
            if (ptr)
                ((Object*)ptr)->inc_ref();
            if (m_ptr)
                ((Object*)m_ptr)->dec_ref();
            m_ptr = ptr;
            return *this;
        }

        /// Clear the reference
        void reset()
        {
            if (m_ptr)
            {
                ((Object*)m_ptr)->dec_ref();
                m_ptr = nullptr;
            }
        }

        /// Compare this reference with another reference
        bool operator==(const ref& r) const
        {
            return m_ptr == r.m_ptr;
        }

        /// Compare this reference with another reference
        bool operator!=(const ref& r) const
        {
            return m_ptr != r.m_ptr;
        }

        /// Compare this reference with a pointer
        bool operator==(const T* ptr) const
        {
            return m_ptr == ptr;
        }

        /// Compare this reference with a pointer
        bool operator!=(const T* ptr) const
        {
            return m_ptr != ptr;
        }

        /// Access the object referenced by this reference
        T* operator->()
        {
            return m_ptr;
        }

        /// Access the object referenced by this reference
        const T* operator->() const
        {
            return m_ptr;
        }

        /// Return a C++ reference to the referenced object
        T& operator*()
        {
            return *m_ptr;
        }

        /// Return a const C++ reference to the referenced object
        const T& operator*() const
        {
            return *m_ptr;
        }

        /// Return a pointer to the referenced object
        operator T*()
        {
            return m_ptr;
        }

        /// Return a const pointer to the referenced object
        T* get()
        {
            return m_ptr;
        }

        /// Return a pointer to the referenced object
        const T* get() const
        {
            return m_ptr;
        }

        /// Check if the object is defined
        operator bool() const
        {
            return m_ptr != nullptr;
        }

    private:
        T* m_ptr;
    };
}
