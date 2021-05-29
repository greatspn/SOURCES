//----------------------------------------------------------------------------
#ifndef REFCOUNTER_H_
#define REFCOUNTER_H_
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

#define safe_removeOwner(p) {   if (p != nullptr) { p->removeOwner(); p = nullptr; }   }
#define safe_addOwner(p)    {   if (p != nullptr) { p->addOwner(); }   }

//----------------------------------------------------------------------------

class refcounted_base {
    int countOwner;
protected:
    refcounted_base();
    virtual ~refcounted_base();
    virtual void before_delete();

    refcounted_base(const refcounted_base&) = delete;
    refcounted_base(refcounted_base&&) = delete;
    refcounted_base& operator=(const refcounted_base&) = delete;
public:
    void addOwner();
    void removeOwner();
    // void printRef() { cout <<(this)<<" owners="<<countOwner<<endl; }
};

//----------------------------------------------------------------------------

template<typename T> 
class ref_ptr {
    T*    ptr;       // ref-counted pointer to a subclass of BaseFormula
public:
    typedef T value_type;
    inline ref_ptr() : ptr(nullptr) {
        static_assert(std::is_base_of<refcounted_base, T>::value, 
                      "type parameter is not a refcounted_base");
    }
    inline ref_ptr(std::nullptr_t) : ptr(nullptr) { }
    // inline ref_ptr(T* _ptr) : ptr(_ptr) { }
    inline ref_ptr(const ref_ptr<T>& ref) : ptr(ref.ptr) { safe_addOwner(ptr); }
    inline ref_ptr(ref_ptr<T>&& ref) { ptr=ref.ptr; ref.ptr = nullptr; }

    inline ~ref_ptr() { safe_removeOwner(ptr); }

    inline T& operator* () const { return *ptr; } 
    inline T* operator-> () const { return ptr; }
    
    inline ref_ptr<T>& operator=(const ref_ptr<T>& ref) {
        if (this != &ref) { // no self assignments
            safe_removeOwner(ptr); 
            ptr = ref.ptr;
            safe_addOwner(ptr);
        }
        return *this;
    }
    inline ref_ptr<T>& operator=(ref_ptr<T>&& ref) {
        ptr = ref.ptr; 
        ref.ptr = nullptr; 
        return *this;
    }
    // inline ref_ptr<T>& operator=(T* _ptr) {
    //     if (this->ptr != _ptr) {
    //         safe_removeOwner(ptr); 
    //         ptr = _ptr;
    //         // NO safe_addOwner(ptr);
    //     }
    //     return *this;
    // }

    template<typename T2> 
    inline ref_ptr<T>& operator=(const ref_ptr<T2>& ref);
    template<typename T2> 
    inline ref_ptr<T>& operator=(ref_ptr<T2>&& ref);
    // template<typename T2> 
    // inline ref_ptr<T>& operator=(T2* _ptr);

    template<typename T2> operator ref_ptr<T2>();
    template<typename T2> operator const ref_ptr<T2>() const;

    inline operator bool() const { return (ptr != nullptr); }

    inline bool operator!=(std::nullptr_t) const { return (ptr != nullptr); }
    inline bool operator==(std::nullptr_t) const { return (ptr == nullptr); }

    inline T* get() const { return ptr; }
    // inline T* get() { return ptr; }
    inline void set(T* p) { ptr = p; }

    inline void reset() { safe_removeOwner(ptr); }
};

//----------------------------------------------------------------------------

// Already owns the pointer, so no addOwner() call is needed
template<typename T>
inline ref_ptr<T> make_new_ref_ptr(T* p) {
    ref_ptr<T> ref;
    ref.set(p);
    return ref;
}

template<typename T>
inline ref_ptr<T> make_ref_ptr(T* p) {
    ref_ptr<T> ref;
    ref.set(p);
    safe_addOwner(p);
    return ref;
}

//----------------------------------------------------------------------------

template <class Base, class Derived>
inline ref_ptr<Base> dynamic_ptr_cast(const ref_ptr<Derived>& sp) {
    ref_ptr<Base> rb = make_ref_ptr<Base>(dynamic_cast<Base*>(sp.get()));
    // cout << "(dynamic_ptr_cast(ref_ptr<Derived>& sp)) "<<sp.get()<<endl;
    // if (rb.get() != nullptr) {
    //     cout << "dynamic_ptr_cast - addOwner" << endl;
    //     rb.get()->addOwner();
    // }
    return rb;
}

template <class Base, class Derived>
inline ref_ptr<Base> dynamic_ptr_cast(ref_ptr<Derived>&& sp) {
    ref_ptr<Base> rb = make_new_ref_ptr<Base>(dynamic_cast<Base*>(sp.get()));
    // cout << "(dynamic_ptr_cast(ref_ptr<Derived>&& sp)) null=="<<sp.get()<<endl;
    // // if (rb.get() != nullptr)
    // //     rb.get()->addOwner();
    sp.set(nullptr);
    return rb;
}


template<typename T>
template<typename T2>
inline ref_ptr<T>& ref_ptr<T>::operator=(const ref_ptr<T2>& ref) {
    return (*this) = dynamic_ptr_cast<T>(ref);
}

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