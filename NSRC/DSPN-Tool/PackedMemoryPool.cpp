/*
 *  PackedMemoryPool.cpp
 *
 *  Implementation of the fast memory pool and of the packing procedures
 *
 *  Created by Elvio Amparore
 *
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <stack>
#include <queue>
#include <set>
#include <exception>
#include <algorithm>
#include <ctime>
#include <cfloat>
#include <cstring>
#include <climits>
#include <limits>
#include <memory>
#include <unordered_map>
using namespace std;

#include "../numeric/numeric.h"
using namespace numerical;

#include "utils.h"
#include "sparsevector.h"
#include "PetriNet.h" // To instantiate the sparse_seq_store<SparseMarking>
#include "PackedMemoryPool.h"


//=============================================================================
//  Standard allocator encapsulated in the raw_pool_alloc interface
//=============================================================================

class std_alloc : public raw_pool_alloc {
public:
    inline std_alloc() : cnt(0), bytesCnt(0) { }
    virtual ~std_alloc() {
        if (cnt != 0 || bytesCnt != 0)
            throw bad_alloc();
    }
    virtual uint8_t *allocate(size_t sz) {
        cnt++; bytesCnt += sz;
        return new uint8_t[sz];
    }
    virtual void deallocate(uint8_t *ptr, size_t sz) {
        cnt--; bytesCnt -= sz;
        delete[] ptr;
    }
    virtual size_t max_size() const    { return numeric_limits<size_t>::max(); }
    virtual size_t get_count() const   { return cnt; }
    virtual size_t get_allocated_bytes() const { return bytesCnt; }

protected:
    size_t	cnt;
    size_t  bytesCnt;
};

// Create a general purpose memory allocator from ::new and ::delete
raw_pool_alloc *CreateStdAllocator() {
    return new std_alloc();
}

//=============================================================================
//  Fixed-size raw memory allocator
//=============================================================================

//#define ALLOC_DBG(code)  cout << code << endl
#define ALLOC_DBG(code)

#ifndef NDEBUG

#define MAX_SIZEOF_BARRIER  8

const uint8_t s_barr[4][MAX_SIZEOF_BARRIER] = {
    { 0x93, 0xa6, 0x90, 0xf4, 0xdd, 0x78, 0xe1, 0xca },
    { 0x07, 0xe7, 0x05, 0x28, 0xe1, 0x0a, 0xdf, 0xa9 },
    { 0xeb, 0xb5, 0x76, 0xb6, 0x3c, 0x45, 0xb3, 0x99 },
    { 0xb4, 0x77, 0xe8, 0x10, 0x70, 0xa7, 0x88, 0xa1 }
};

//#define PB(b)  cout << #b << "=["<<(int)b[0]<<"-"<<(int)b[1]<<"-"<<(int)b[2]<<"-"<<(int)b[3]<<"]"<<endl
#define SETBARR(b, k) assert(sizeof(b) <= MAX_SIZEOF_BARRIER); \
	std::copy(&s_barr[k][0], &s_barr[k][sizeof(b)], b);
#define CHECKBARR(b, k) assert(sizeof(b) <= MAX_SIZEOF_BARRIER); \
	assert(std::equal(&s_barr[k][0], &s_barr[k][sizeof(b)], b))

#endif

// Used to track the alloc_sz field of freed elements.
#define NOT_ALLOC_SZ   		size_t(-200)

// SZ = allocation unit size
template<int SZ>
class fixed_raw_alloc : public raw_pool_alloc {
    struct elem {
#ifndef NDEBUG
        size_t   alloc_sz;			///< Allocation size
        uint8_t  b1[sizeof(int *)];	///< Debug barrier 1
        union {
            elem    *pNext;			///< Next elem* in the free list
            uint8_t  buf[SZ];		///< Raw bytes
        };
        uint8_t  b2[sizeof(int *)];	///< Debug barrier 2

        inline void set_alloc_barriers() { SETBARR(b1, 0); SETBARR(b2, 1); }
        inline void set_free_barriers()  { SETBARR(b1, 2); SETBARR(b2, 3); }
        inline void check_alloc_barriers() const { CHECKBARR(b1, 0); CHECKBARR(b2, 1); }
        inline void check_free_barriers() const  { CHECKBARR(b1, 2); CHECKBARR(b2, 3); }
        inline void set_alloc_sz(size_t sz) { alloc_sz = sz; }
        inline void check_alloc_sz(size_t sz) const { assert(alloc_sz == sz); }
#else
        union {
            elem    *pNext;			///< Next elem* in the free list
            uint8_t  buf[SZ];		///< Raw bytes
        };
        inline void set_alloc_barriers() {}
        inline void set_free_barriers() {}
        inline void check_alloc_barriers() const {}
        inline void check_free_barriers() const {}
        inline void set_alloc_sz(size_t sz) { }
        inline void check_alloc_sz(size_t sz) const { }
#endif
    };
    struct chunk {
        chunk *pNext;			///< Next chunk in the chunk list, or nullptr
        size_t num_elems;		///< Number of elem objects in e[]
        elem   e[1];			///< Array of allocation units
    };
    elem    *m_pEmpty;	 		///< List of free elem* objects
    chunk   *m_pChunks;	 		///< Chunks where all the elem* objects belong
    ssize_t  m_cnt;				///< Allocation count
    size_t   m_lastChunkElems;  ///< Last # of elems allocated in a single chunk
public:
    inline fixed_raw_alloc(size_t startChunkElems = 16)
        : m_pEmpty(nullptr), m_pChunks(nullptr), m_cnt(0), m_lastChunkElems(startChunkElems) {}

    virtual ~fixed_raw_alloc() {
        while (m_pChunks) {
            chunk *pNext = m_pChunks->pNext;
            delete[] reinterpret_cast<uint8_t *>(m_pChunks);
            m_pChunks = pNext;
        }
        m_pEmpty = nullptr;
    }

    virtual uint8_t *allocate(size_t sz) {
        assert(sz <= SZ);
        elem *pObj = allocate_elem(sz);
        return reinterpret_cast<uint8_t *>(&pObj->buf);
    }

    virtual void deallocate(uint8_t *ptr, size_t sz) {
        assert(sz <= SZ);
        elem *pObj = reinterpret_cast<elem *>(ptr - offsetof(elem, buf));
        deallocate_elem(pObj, sz);
    }

    virtual size_t max_size() const		{	return SZ;   }

    virtual size_t get_count() const		{   return m_cnt;   }

    virtual size_t get_allocated_bytes() const { return get_count() * max_size(); }

protected:
    elem *allocate_elem(size_t sz) {
        ALLOC_DBG("allocate(): start");
        if (m_pEmpty == nullptr) {
            size_t num_elems = m_lastChunkElems;
            m_lastChunkElems *= 2;
            size_t num_bytes = size_t(&((chunk *)nullptr)->e[num_elems] /*- nullptr*/);
            assert(num_bytes >= sizeof(chunk) + sizeof(elem) * (num_elems - 1));
            chunk *pNewCh = reinterpret_cast<chunk *>(new uint8_t[num_bytes]);
            pNewCh->pNext = m_pChunks;
            pNewCh->num_elems = num_elems;
            m_pChunks = pNewCh;
            for (ssize_t i = num_elems - 1; i >= 0; i--) {
                pNewCh->e[i].set_free_barriers();
                pNewCh->e[i].set_alloc_sz(NOT_ALLOC_SZ);
                pNewCh->e[i].pNext = m_pEmpty;
                m_pEmpty = &pNewCh->e[i];
            }
        }
        elem *pObj = m_pEmpty;
        pObj->check_free_barriers();
        pObj->check_alloc_sz(NOT_ALLOC_SZ);
        pObj->set_alloc_barriers();
        pObj->set_alloc_sz(sz);
        m_pEmpty = m_pEmpty->pNext;
        m_cnt++;
        ALLOC_DBG("allocate(): pObj=" << pObj << "  cnt=" << m_cnt);
        return pObj;
    }

    void deallocate_elem(elem *pObj, size_t sz) {
        pObj->check_alloc_barriers();
        pObj->check_alloc_sz(sz);
#ifndef NDEBUG
        assert(m_cnt > 0);
        // Avoid double deallocations
        elem *pEl = m_pEmpty;
        while (pEl) {
            assert(pEl != pObj);
            pEl = pEl->pNext;
        }
        // Ensure that p is a valid pointer, allocated from this pool
        chunk *pCh = m_pChunks;
        bool ok = false;
        while (pCh) {
            ptrdiff_t elemIndex = pObj - pCh->e;
            if (elemIndex >= 0 && elemIndex < ptrdiff_t(pCh->num_elems) &&
                    &pCh->e[elemIndex] == pObj) {
                ALLOC_DBG(" elemIndex=" << elemIndex << "  &pCh->e[elemIndex]="
                          << &pCh->e[elemIndex] << "  pObj=" << pObj);
                ok = true;
                break;
            }
            pCh = pCh->pNext;
        }
        assert(ok);
#endif
        pObj->set_free_barriers();
        pObj->set_alloc_sz(NOT_ALLOC_SZ);
        pObj->pNext = m_pEmpty;
        m_pEmpty = pObj;
        m_cnt--;
        ALLOC_DBG("deallocate(): cnt=" << m_cnt);
    }
};

//=============================================================================
//  General variable-length raw memory allocator with pooling
//=============================================================================

class raw_pool_alloc_impl : public raw_pool_alloc {
public:
    typedef std::list<raw_pool_alloc *>  allocator_list;

    raw_pool_alloc_impl() { }
    virtual ~raw_pool_alloc_impl() {
        allocator_list::iterator it = m_allocs.begin(), it2;
        while (it != m_allocs.end()) {
            delete *it;
            it2 = it;
            ++it;
            m_allocs.erase(it2);
        }
    }

    virtual uint8_t *allocate(size_t sz) {
        allocator_list::const_iterator it = m_allocs.begin();
        while (it != m_allocs.end()) {
            if (sz <= (*it)->max_size())
                return (*it)->allocate(sz);
            ++it;
        }
        // If this exception is thrown, is because the number of fixed_raw_alloc
        // inside this raw_allocator are not enough. Add more of them in the
        // CreateRawPoolAllocator() function
        throw bad_alloc();
    }

    virtual void deallocate(uint8_t *ptr, size_t sz) {
        allocator_list::const_iterator it = m_allocs.begin();
        while (it != m_allocs.end()) {
            if (sz <= (*it)->max_size()) {
                (*it)->deallocate(ptr, sz);
                return;
            }
            ++it;
        }
        throw bad_alloc();
    }

    virtual size_t max_size() const {
        if (m_allocs.empty())
            return 0;
        else return m_allocs.back()->max_size();
    }

    virtual size_t get_count() const {
        size_t cnt = 0;
        allocator_list::const_iterator it = m_allocs.begin();
        while (it != m_allocs.end()) {
            cnt += (*it)->get_count();
            ++it;
        }
        return cnt;
    }

    virtual size_t get_allocated_bytes() const {
        size_t cnt = 0;
        allocator_list::const_iterator it = m_allocs.begin();
        while (it != m_allocs.end()) {
            cnt += (*it)->get_allocated_bytes();
            ++it;
        }
        return cnt;
    }

    inline void append_allocator(raw_pool_alloc *pAlloc) {
        assert(pAlloc->max_size() > max_size());
        m_allocs.push_back(pAlloc);
    }

protected:
    /// Fixed-size allocator list
    allocator_list  m_allocs;
};

//=============================================================================

raw_pool_alloc *CreateRawPoolAllocator() {
    raw_pool_alloc_impl *p = new raw_pool_alloc_impl();

    // Add the fixed-size memory allocators, in ascending order.
    p->append_allocator(new fixed_raw_alloc<8>(256));
    p->append_allocator(new fixed_raw_alloc<16>(256));
    p->append_allocator(new fixed_raw_alloc<24>(128));
    p->append_allocator(new fixed_raw_alloc<32>(128));
    p->append_allocator(new fixed_raw_alloc<48>(128));
    p->append_allocator(new fixed_raw_alloc<64>(64));
    p->append_allocator(new fixed_raw_alloc<128>(32));
    p->append_allocator(new fixed_raw_alloc<256>(32));
    p->append_allocator(new fixed_raw_alloc<512>(16));
    p->append_allocator(new fixed_raw_alloc<1024>(16));

    return p;
}

//=============================================================================




/* TEST CODE FOR THE sgm_vector<T> CLASS

const char* FMT[] = { "[", "", ",", "]" };

int main (int argc, char **argv)
{
	sgm_vector<int, 3> v1;
	vector<int> v2;

	v1.insert(v1.begin(), 2);
	v1.insert(v1.end(), 3);
	v2.insert(v2.begin(), v1.begin(), v1.end());

	cout << print_vec(v1,FMT) << "  " << print_vec(v2,FMT) << endl;

	for (size_t i=0; i<10; i++) {
		v1.insert(v1.begin()+i, i+10);
		v2.insert(v2.begin()+i, i+10);
	}
	cout << print_vec(v1,FMT) << "  " << print_vec(v2,FMT) << endl;
	for (size_t i=0; i<3; i++) {
		v1.insert(v1.begin()+5, 1000);
		v2.insert(v2.begin()+5, 1000);
	}
	cout << print_vec(v1,FMT) << "  " << print_vec(v2,FMT) << endl;

	v1.erase(v1.begin());
	v2.erase(v2.begin());
	v1.erase(find(v1.begin(), v1.end(), 16), find(v1.begin(), v1.end(), 18));
	v2.erase(find(v2.begin(), v2.end(), 16), find(v2.begin(), v2.end(), 18));
	cout << print_vec(v1,FMT) << "  " << print_vec(v2,FMT) << endl;

	v1.pop_back(); v1.pop_back();
	v2.pop_back(); v2.pop_back();
	v1.push_back(0); v1.push_back(-30);
	v2.push_back(0); v2.push_back(-30);
	cout << print_vec(v1,FMT) << "  " << print_vec(v2,FMT) << endl;

	while (v1.size() > 0)
		v1.pop_back();
	while (v2.size() > 0)
		v2.pop_back();
	cout << print_vec(v1,FMT) << "  " << print_vec(v2,FMT) << endl;
}
*/






















