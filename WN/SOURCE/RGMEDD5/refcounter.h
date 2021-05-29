//----------------------------------------------------------------------------
#ifndef REFCOUNTER_H_
#define REFCOUNTER_H_
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#define safe_remove_ref(p) {   if (p != nullptr) { p->remove_ref(); p = nullptr; }   }
#define safe_add_ref(p)    {   if (p != nullptr) { p->add_ref(); }   }

//----------------------------------------------------------------------------
// base class of non-copyable reference counted objects
//----------------------------------------------------------------------------
class refcounted_base {
    // reference counter. starts at 1.
    int ref_count;
protected:
    refcounted_base();
    virtual ~refcounted_base();
    virtual void before_delete();

    refcounted_base(const refcounted_base&) = delete;
    refcounted_base(refcounted_base&&) = delete;
    refcounted_base& operator=(const refcounted_base&) = delete;
public:
    // increase the reference counter
    void add_ref();
    // decrease the reference counter. 
    // if it is 0, call before_delete() and then delete the object.
    void remove_ref();
};

//----------------------------------------------------------------------------
// smart pointer to refcounted_base* objects
//----------------------------------------------------------------------------
template<typename T> 
class ref_ptr {
    T*    ptr;       // ref-counted pointer to a subclass of BaseFormula
public:
    typedef T    value_type;
    typedef T   *pointer_type;

    // default constructor starts with nullptr
    inline ref_ptr() : ptr(nullptr) {
        static_assert(std::is_base_of<refcounted_base, T>::value, 
                      "type parameter is not a refcounted_base");
    }
    // explicit nullptr constructor
    inline explicit ref_ptr(std::nullptr_t) : ptr(nullptr) { }
    // ref_ptr cannot be initialized by raw pointers. 
    // Use make_ref_ptr or move_to_ref_ptr
    // DISABLED: inline ref_ptr(T* _ptr) : ptr(_ptr) { }
    
    // copy ctor
    inline ref_ptr(const ref_ptr<T>& ref) : ptr(ref.ptr) { safe_add_ref(ptr); }
    // move ctor
    inline ref_ptr(ref_ptr<T>&& ref) { ptr=ref.ptr; ref.ptr = nullptr; }
    // dtor
    inline ~ref_ptr() { safe_remove_ref(ptr); }

    // indirect acces
    inline T& operator* () const { return *ptr; } 
    inline T* operator-> () const { return ptr; }
    
    // copy assignment of the same T type
    inline ref_ptr<T>& operator=(const ref_ptr<T>& ref) {
        if (this != &ref) { // no self assignments
            safe_remove_ref(ptr); 
            ptr = ref.ptr;
            safe_add_ref(ptr);
        }
        return *this;
    }
    // move assignment of the same T type
    inline ref_ptr<T>& operator=(ref_ptr<T>&& ref) {
        safe_remove_ref(ptr); 
        ptr = ref.ptr; 
        ref.ptr = nullptr; 
        return *this;
    }
    // DISABLED
    // inline ref_ptr<T>& operator=(T* _ptr) {
    //     if (this->ptr != _ptr) {
    //         safe_remove_ref(ptr); 
    //         ptr = _ptr;
    //         // NO safe_add_ref(ptr);
    //     }
    //     return *this;
    // }

    // covariant copy assignment with dynamic pointer cast
    template<typename T2> 
    inline ref_ptr<T>& operator=(const ref_ptr<T2>& ref);
    // covariant move assignment with dynamic pointer cast
    template<typename T2> 
    inline ref_ptr<T>& operator=(ref_ptr<T2>&& ref);
    // template<typename T2> 
    // inline ref_ptr<T>& operator=(T2* _ptr);

    // dynamic cast to ref_ptr<T2>
    template<typename T2> operator ref_ptr<T2>();
    template<typename T2> operator const ref_ptr<T2>() const;

    // check nullity
    inline operator bool() const { return (ptr != nullptr); }

    // check nullity
    inline bool operator!=(std::nullptr_t) const { return (ptr != nullptr); }
    inline bool operator==(std::nullptr_t) const { return (ptr == nullptr); }

    // get the pointer without changing the ref counts
    inline T* get() const { return ptr; }
    
    // reset the pointer to nullptr
    inline void reset() { safe_remove_ref(ptr); }

    // return the pointer, and reset to nullptr

private:
    // low-level manipulatrion of the pointer without changing the ref counts
    inline void set(T* p) { ptr = p; }

    // methods that can use set()
    template<typename T2> friend ref_ptr<T2> move_to_ref_ptr(T2*&& p);

    template<typename T2> friend ref_ptr<T2> make_ref_ptr(T2* p);

    template <class Base, class Derived>
    friend inline ref_ptr<Base> dynamic_ptr_cast(ref_ptr<Derived>&& sp);
};

//----------------------------------------------------------------------------

// make a ref_ptr<T> moving pointer without increasing the ref counts
template<typename T>
inline ref_ptr<T> move_to_ref_ptr(T*&& p) {
    ref_ptr<T> ref;
    ref.set(p);
    return ref;
}

// return a new ref_ptr<T>, increasing the ref count
template<typename T>
inline ref_ptr<T> make_ref_ptr(T* p) {
    ref_ptr<T> ref;
    ref.set(p);
    safe_add_ref(p);
    return ref;
}

//----------------------------------------------------------------------------

// covariant cast: make a new ref_ptr<Base> of the dynamically casted pointer
template <class Base, class Derived>
inline ref_ptr<Base> dynamic_ptr_cast(const ref_ptr<Derived>& sp) {
    ref_ptr<Base> rb = make_ref_ptr<Base>(dynamic_cast<Base*>(sp.get()));
    // cout << "(dynamic_ptr_cast(ref_ptr<Derived>& sp)) "<<sp.get()<<endl;
    // if (rb.get() != nullptr) {
    //     cout << "dynamic_ptr_cast - add_ref" << endl;
    //     rb.get()->add_ref();
    // }
    return rb;
}

// covariant cast: move a new ref_ptr<Base> of the dynamically casted pointer
template <class Base, class Derived>
inline ref_ptr<Base> dynamic_ptr_cast(ref_ptr<Derived>&& sp) {
    ref_ptr<Base> rb = move_to_ref_ptr<Base>(dynamic_cast<Base*>(sp.get()));
    // cout << "(dynamic_ptr_cast(ref_ptr<Derived>&& sp)) null=="<<sp.get()<<endl;
    // // if (rb.get() != nullptr)
    // //     rb.get()->add_ref();
    sp.set(nullptr);
    return rb;
}

// covariant copy assignment
template<typename T>
template<typename T2>
inline ref_ptr<T>& ref_ptr<T>::operator=(const ref_ptr<T2>& ref) {
    return (*this) = dynamic_ptr_cast<T>(ref);
}

// covariant move assignment
template<typename T>
template<typename T2>
inline ref_ptr<T>& ref_ptr<T>::operator=(ref_ptr<T2>&& ref) {
    return (*this) = std::move(dynamic_ptr_cast<T>(ref));
}

// template<typename T>
// template<typename T2>
// inline ref_ptr<T>& ref_ptr<T>::operator=(T2* _ptr) {
//     return (*this) = dynamic_cast<T*>(_ptr);
// }

// dynamic cast to ref_ptr<T2>
template<typename T>
template<typename T2> 
ref_ptr<T>::operator const ref_ptr<T2>() const {
    return dynamic_ptr_cast<T2>(*this);
}

template<typename T>
template<typename T2> 
ref_ptr<T>::operator ref_ptr<T2>() {
    return dynamic_ptr_cast<T2>(*this);
}


//----------------------------------------------------------------------------
#endif // REFCOUNTER_H_