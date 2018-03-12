/*
 *  private_integers.h
 *
 *  Type-safe private typedef of an integer type
 *
 *  Created by Elvio Amparore.
 *
 */

//=============================================================================
#ifndef __PRIVATE_INTEGER_H__
#define __PRIVATE_INTEGER_H__
//=============================================================================

#include <iostream>
#include <vector>
#include <initializer_list>

#ifdef GSOL
#error
#endif

template<class T, class U>
class private_integer {
    T val;
public:
    typedef private_integer<T, U> ThisType;
    typedef T contained_type;

    private_integer() {}
    private_integer(T v) : val(v) {}
    private_integer(const private_integer<T, U> &pi) : val(pi.val) {}

    inline ThisType operator + () const { return ThisType(val); }
    inline ThisType operator - () const { return ThisType(-val); }

    inline ThisType &operator ++ () { ++val;  return *this; }
    inline ThisType &operator -- () { --val;  return *this; }
    inline ThisType operator ++ (int) { ThisType t(*this); val++; return t; }
    inline ThisType operator -- (int) { ThisType t(*this); val--; return t; }

    inline ThisType operator + (const ThisType &op2) const { return ThisType(val + op2.val); }
    inline ThisType operator - (const ThisType &op2) const { return ThisType(val - op2.val); }
    inline ThisType operator * (const ThisType &op2) const { return ThisType(val * op2.val); }
    inline ThisType operator / (const ThisType &op2) const { return ThisType(val / op2.val); }
    inline ThisType operator % (const ThisType &op2) const { return ThisType(val % op2.val); }

    inline ThisType &operator += (const ThisType &op2) { val += op2.val; return *this; }
    inline ThisType &operator -= (const ThisType &op2) { val -= op2.val; return *this; }
    inline ThisType &operator *= (const ThisType &op2) { val *= op2.val; return *this; }
    inline ThisType &operator /= (const ThisType &op2) { val /= op2.val; return *this; }
    inline ThisType &operator %= (const ThisType &op2) { val %= op2.val; return *this; }

    inline bool operator < (const ThisType &op2) const { return val < op2.val; }
    inline bool operator > (const ThisType &op2) const { return val > op2.val; }
    inline bool operator <= (const ThisType &op2) const { return val <= op2.val; }
    inline bool operator >= (const ThisType &op2) const { return val >= op2.val; }
    inline bool operator == (const ThisType &op2) const { return val == op2.val; }
    inline bool operator != (const ThisType &op2) const { return val != op2.val; }

    inline ThisType &operator = (const ThisType &op2) { val = op2.val; return *this; }

    inline operator T() const { return val; }


    inline T get() const { return val; }
    inline void set(T v) { val = v; }
};


template<class ostream_t, class T, class U>
ostream_t &operator << (ostream_t &os, const private_integer<T, U> &pi) {
    os << T(pi);
    return os;
}

template<class istream_t, class T, class U>
istream_t &operator >> (istream_t &is, private_integer<T, U> &pi) {
    T v;
    is >> v;
    pi.set(v);
    return is;
}

/*template<class T, class U>
inline size_t hash_value(const private_integer<T,U>& pi) {
	// TODO: return hash_value(pi.get());
	return pi.get();
}*/

template<class T, class U>
inline T get_value(const private_integer<T, U> &pi)  {  return pi.get();  }

template<class T, class U>
inline void set_value(private_integer<T, U> &pi, T val)  {  pi.set(val);  }

inline size_t get_value(const size_t n) noexcept {  return n;  }
inline void set_value(size_t &n, size_t val)  {  n = val;  }

inline int get_value(const int n) noexcept {  return n;  }
inline void set_value(int &n, int val)  {  n = val;  }





//=============================================================================
//   An opaque typedef of an int type.
//   Behaves exactly like an int, but is not exchangable with int's.
//=============================================================================

template<class T>
class opaque_int {
public:
    typedef opaque_int<T>           this_type;
    typedef typename T::value_type  value_type;
private:
    value_type val;
public:

    inline opaque_int() {}
    inline explicit opaque_int(value_type v) : val(v) {}
    inline opaque_int(const this_type &pi) = default;
    inline opaque_int(this_type &&pi) = default;

    inline this_type operator + () const { return this_type(val); }
    inline this_type operator - () const { return this_type(-val); }

    inline this_type &operator ++ () { ++val;  return *this; }
    inline this_type &operator -- () { --val;  return *this; }
    inline this_type operator ++ (int) { this_type t(*this); val++; return t; }
    inline this_type operator -- (int) { this_type t(*this); val--; return t; }

    inline this_type operator + (const this_type &op2) const { return this_type(val + op2.val); }
    inline this_type operator - (const this_type &op2) const { return this_type(val - op2.val); }
    inline this_type operator * (const this_type &op2) const { return this_type(val * op2.val); }
    inline this_type operator / (const this_type &op2) const { return this_type(val / op2.val); }
    inline this_type operator % (const this_type &op2) const { return this_type(val % op2.val); }

    inline this_type &operator += (const this_type &op2) { val += op2.val; return *this; }
    inline this_type &operator -= (const this_type &op2) { val -= op2.val; return *this; }
    inline this_type &operator *= (const this_type &op2) { val *= op2.val; return *this; }
    inline this_type &operator /= (const this_type &op2) { val /= op2.val; return *this; }
    inline this_type &operator %= (const this_type &op2) { val %= op2.val; return *this; }

    inline bool operator < (const this_type &op2) const { return val < op2.val; }
    inline bool operator > (const this_type &op2) const { return val > op2.val; }
    inline bool operator <= (const this_type &op2) const { return val <= op2.val; }
    inline bool operator >= (const this_type &op2) const { return val >= op2.val; }
    inline bool operator == (const this_type &op2) const { return val == op2.val; }
    inline bool operator != (const this_type &op2) const { return val != op2.val; }

    inline this_type &operator = (const this_type &op2) = default;
    inline this_type &operator = (this_type && op2) = default;

    inline explicit operator value_type() const { return val; }


    // inline value_type get() const { return val; }
    inline void set(value_type v) { val = v; }
};

/*template<class T, class U>
inline size_t hash_value(const opaque_int<T,U>& pi) {
	// TODO: return hash_value(pi.get());
	return pi.get();
}*/

template<class T>
inline typename opaque_int<T>::value_type
get_value(const opaque_int<T> &pi)  {  return (typename opaque_int<T>::value_type)pi;  }

template<class T>
inline void set_value(opaque_int<T> &pi, typename opaque_int<T>::value_type val)
{  pi.set(val);  }

// inline size_t get_value(const size_t n)  {  return n;  }
// inline void set_value(size_t& n, size_t val)  {  n = val;  }

// inline int get_value(const int n)  {  return n;  }
// inline void set_value(int& n, int val)  {  n = val;  }


template<class T>
std::ostream &operator << (std::ostream &os, const opaque_int<T> &pi) {
    os << get_value(pi);
    return os;
}

template<class T>
std::istream &operator >> (std::istream &is, opaque_int<T> &pi) {
    typename T::value_type v;
    is >> v;
    set_value(pi, v);
    return is;
}

//=============================================================================
//   Modified vector where the index type is a template parameter
//=============================================================================

template<typename T, typename I, typename Alloc = std::allocator<T> >
class ivector2 {
public:
    typedef std::vector<T, Alloc>                 base_type;
    typedef typename base_type::value_type        value_type;
    typedef typename base_type::pointer           pointer;
    typedef typename base_type::const_pointer     const_pointer;
    typedef typename base_type::reference         reference;
    typedef typename base_type::const_reference   const_reference;
    typedef I		  							  size_type;
    typedef typename base_type::difference_type	  difference_type;
    typedef typename base_type::allocator_type    allocator_type;

    typedef typename base_type::iterator iterator;
    typedef typename base_type::const_iterator const_iterator;
    typedef typename base_type::reverse_iterator reverse_iterator;
    typedef typename base_type::const_reverse_iterator const_reverse_iterator;


    explicit inline
    ivector2(const allocator_type &a = allocator_type()) : _vector(a) { }

    explicit inline
    ivector2(size_type n, const value_type &v = value_type(),
             const allocator_type &a = allocator_type())
        : _vector(get_value(n), v, a) { }

    inline ivector2(const ivector2 &) = default;
    inline ivector2(ivector2 &&) = default;

    template<typename InputIterator> inline
    ivector2(InputIterator first, InputIterator last,
             const allocator_type &a = allocator_type())
        : _vector(first, last, a) { }

    inline ivector2(std::initializer_list<value_type> il, const allocator_type &a = allocator_type())
        : _vector(il, a) { }

    inline ~ivector2() { }

    inline ivector2 &operator = (const ivector2 &) = default;
    inline ivector2 &operator = (ivector2 &&) = default;
    ivector2 &operator= (std::initializer_list<value_type> il) { _vector = il; return *this; }

    inline void assign(size_type n, const value_type &v)
    { _vector.assign(get_value(n), v); }

    template<typename InputIterator>
    inline void assign(InputIterator first, InputIterator last)
    { _vector.assign(first, last); }

    inline void assign(std::initializer_list<value_type> il) { _vector.assign(il); }

    inline iterator begin() { return iterator(_vector.begin()); }
    inline const_iterator begin() const { return const_iterator(_vector.begin()); }
    inline iterator end() { return iterator(_vector.end()); }
    inline const_iterator end() const { return const_iterator(_vector.end()); }

    inline const_iterator cbegin() const { return const_iterator(_vector.begin()); }
    inline const_iterator cend() const { return const_iterator(_vector.end()); }

    inline reverse_iterator rbegin() { return reverse_iterator(_vector.rbegin()); }
    inline const_reverse_iterator rbegin() const { return const_reverse_iterator(_vector.rbegin()); }
    inline reverse_iterator rend() { return reverse_iterator(_vector.rend()); }
    inline const_reverse_iterator rend() const { return const_reverse_iterator(_vector.rend()); }

    inline const_reverse_iterator crbegin() const { return const_reverse_iterator(_vector.rbegin()); }
    inline const_reverse_iterator crend() const { return const_reverse_iterator(_vector.rend()); }

    inline size_type size() const noexcept { return size_type(_vector.size()); }
    inline size_type max_size() const noexcept { return size_type(_vector.max_size()); }
    inline size_type capacity() const noexcept { return size_type(_vector.capacity()); }
    inline bool empty() const noexcept { return _vector.empty(); }

    inline void resize(size_type ns, const value_type &v = value_type())
    { _vector.resize(get_value(ns), v); }
    inline void reserve(size_type ns) { _vector.reserve(get_value(ns)); }

    inline void shrink_to_fit() { _vector.shrink_to_fit(); }

    inline reference operator[](size_type n) { return _vector[get_value(n)]; }
    inline const_reference operator[](size_type n) const { return _vector[get_value(n)]; }
    inline reference at(size_type n) { return _vector.at(get_value(n)); }
    inline const_reference at(size_type n) const { return _vector.at(get_value(n)); }

    inline reference front() { return _vector.front(); }
    inline const_reference front() const { return _vector.front(); }
    inline reference back() { return _vector.back(); }
    inline const_reference back() const { return _vector.back(); }

    inline pointer data() noexcept { return _vector.data(); }
    inline const_pointer data() const noexcept { return _vector.data(); }

    inline void push_back(const value_type &v) { _vector.push_back(v); }
    inline void push_back(value_type &&v) { _vector.push_back(std::forward<value_type>(v)); }
    inline void pop_back() { _vector.pop_back(); }

    template <class... Args>
    iterator emplace(const_iterator position, Args &&... args)
    { _vector.emplace(position, std::forward<Args>(args)...); }

    template <class... Args>
    void emplace_back(Args &&... args) { _vector.emplace_back(std::forward<Args>(args)...); }

    inline iterator insert(iterator it, const value_type &v)
    { return iterator(_vector.insert(it, v)); }
    inline iterator insert(iterator it, size_type n, const value_type &v)
    { return iterator(_vector.insert(it, get_value(n), v)); }
    template<typename InputIterator>
    inline void insert(iterator pos, InputIterator first, InputIterator last)
    { _vector.insert(pos, first, last); }
    inline iterator insert(const_iterator position, value_type &&val)
    { return _vector.insert(position, val); }
    inline iterator insert(const_iterator position, std::initializer_list<value_type> il)
    { return _vector.insert(position, il); }

    inline iterator erase(const_iterator pos) { return iterator(_vector.erase(pos)); }
    inline iterator erase(const_iterator first, const_iterator last)
    { return iterator(_vector.erase(first, last)); }

    inline void swap(ivector2 &iv) { _vector.swap(iv._vector); }
    inline void clear() noexcept { _vector.clear(); }

    inline allocator_type get_allocator() const noexcept { return _vector.get_allocator(); }

// public:
    inline base_type &vec() noexcept             { return _vector; }
    inline const base_type &vec() const noexcept { return _vector; }

private:
    std::vector<T, Alloc> _vector;
};

template<typename T, typename I, typename Alloc>
inline bool operator==(const ivector2<T, I, Alloc> &x, const ivector2<T, I, Alloc> &y)
{ return x.vec() == y.vec(); }

template<typename T, typename I, typename Alloc>
inline bool operator!=(const ivector2<T, I, Alloc> &x, const ivector2<T, I, Alloc> &y)
{ return x.vec() != y.vec(); }

template<typename T, typename I, typename Alloc>
inline bool operator>(const ivector2<T, I, Alloc> &x, const ivector2<T, I, Alloc> &y)
{ return x.vec() > y.vec(); }

template<typename T, typename I, typename Alloc>
inline bool operator<(const ivector2<T, I, Alloc> &x, const ivector2<T, I, Alloc> &y)
{ return x.vec() < y.vec(); }

template<typename T, typename I, typename Alloc>
inline bool operator>=(const ivector2<T, I, Alloc> &x, const ivector2<T, I, Alloc> &y)
{ return x.vec() >= y.vec(); }

template<typename T, typename I, typename Alloc>
inline bool operator<=(const ivector2<T, I, Alloc> &x, const ivector2<T, I, Alloc> &y)
{ return x.vec() <= y.vec(); }

template<typename T, typename I, typename Alloc>
inline void swap(ivector2<T, I, Alloc> &x, ivector2<T, I, Alloc> &y)
{ x.vec().swap(y.vec()); }




//=============================================================================
#endif   // __PRIVATE_INTEGER_H__
