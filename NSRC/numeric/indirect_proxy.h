
#define BOOST_UBLAS_MOVE_SEMANTICS  1
#define BOOST_UBLAS_USE_ITERATING   1
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_sparse.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/matrix_expression.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/detail/vector_assign.hpp>
#include <boost/numeric/ublas/detail/matrix_assign.hpp>
#include <boost/numeric/ublas/detail/temporary.hpp>

//namespace ublas = boost::numeric::ublas;


namespace boost {
namespace numeric {
namespace ublas {

template<class M, class IA>
class matrix_projected: public matrix_expression<matrix_projected<M, IA> > {

    typedef matrix_projected<M, IA> self_type;
public:
#ifdef BOOST_UBLAS_ENABLE_PROXY_SHORTCUTS
    using matrix_expression<self_type>::operator();
#endif
    typedef M matrix_type;
    typedef IA indirect_array_type;
    typedef typename M::size_type size_type;
    typedef typename M::difference_type difference_type;
    typedef typename M::value_type value_type;
    typedef typename M::const_reference const_reference;
    typedef typename boost::mpl::if_<boost::is_const<M>,
            typename M::const_reference,
            typename M::reference>::type reference;
    typedef typename boost::mpl::if_<boost::is_const<M>,
            typename M::const_closure_type,
            typename M::closure_type>::type matrix_closure_type;
    typedef basic_range<size_type, difference_type> range_type;
    typedef basic_slice<size_type, difference_type> slice_type;
    typedef const self_type const_closure_type;
    typedef self_type closure_type;
    typedef typename storage_restrict_traits<typename M::storage_category,
            sparse_proxy_tag>::storage_category storage_category;
    typedef typename M::orientation_category orientation_category;

    // Construction and destruction
    BOOST_UBLAS_INLINE matrix_projected(matrix_type &data, size_type size1,
                                        size_type size2) :
        data_(data), ia1_(size1), ia2_(size2) {
    }
    BOOST_UBLAS_INLINE matrix_projected(matrix_type &data,
                                        const indirect_array_type &ia1, const indirect_array_type &ia2) :
        data_(data), ia1_(ia1.preprocess(data.size1())), ia2_(ia2.preprocess(
                    data.size2())) {
    }
    BOOST_UBLAS_INLINE matrix_projected(const matrix_closure_type &data,
                                        const indirect_array_type &ia1, const indirect_array_type &ia2, int) :
        data_(data), ia1_(ia1.preprocess(data.size1())), ia2_(ia2.preprocess(
                    data.size2())) {
    }

    // Accessors
    BOOST_UBLAS_INLINE
    size_type size1() const {
        return ia1_.count();
    }
    BOOST_UBLAS_INLINE
    size_type size2() const {
        return ia2_.count();
    }
    BOOST_UBLAS_INLINE
    const indirect_array_type &indirect1() const {
        return ia1_;
    }
    BOOST_UBLAS_INLINE
    indirect_array_type &indirect1() {
        return ia1_;
    }
    BOOST_UBLAS_INLINE
    const indirect_array_type &indirect2() const {
        return ia2_;
    }
    BOOST_UBLAS_INLINE
    indirect_array_type &indirect2() {
        return ia2_;
    }

    // Storage accessors
    BOOST_UBLAS_INLINE
    const matrix_closure_type &data() const {
        return data_;
    }
    BOOST_UBLAS_INLINE
    matrix_closure_type &data() {
        return data_;
    }

    // Element access
#ifndef BOOST_UBLAS_PROXY_CONST_MEMBER
    BOOST_UBLAS_INLINE
    const_reference operator()(size_type i, size_type j) const {
        return data_(ia1_[i], ia2_[j]);
    }
    BOOST_UBLAS_INLINE
    reference operator()(size_type i, size_type j) {
        return data_(ia1_[i], ia2_[j]);
    }
#else
    BOOST_UBLAS_INLINE
    reference operator()(size_type i, size_type j) const {
        return data_(ia1_[i], ia2_[j]);
    }
#endif

    // ISSUE can this be done in free project function?
    // Although a const function can create a non-const proxy to a non-const object
    // Critical is that matrix_type and data_ (vector_closure_type) are const correct
    /*BOOST_UBLAS_INLINE
    matrix_projected<matrix_type, indirect_array_type> project(
    		const range_type &r1, const range_type &r2) const
    {
    	return matrix_projected<matrix_type, indirect_array_type> (data_,
    			ia1_.compose(r1.preprocess(data_.size1())), ia2_.compose(
    					r2.preprocess(data_.size2())), 0);
    }
    BOOST_UBLAS_INLINE
    matrix_projected<matrix_type, indirect_array_type> project(
    		const slice_type &s1, const slice_type &s2) const
    {
    	return matrix_projected<matrix_type, indirect_array_type> (data_,
    			ia1_.compose(s1.preprocess(data_.size1())), ia2_.compose(
    					s2.preprocess(data_.size2())), 0);
    }
    BOOST_UBLAS_INLINE
    matrix_projected<matrix_type, indirect_array_type> project(
    		const indirect_array_type &ia1, const indirect_array_type &ia2) const
    {
    	return matrix_projected<matrix_type, indirect_array_type> (data_,
    			ia1_.compose(ia1.preprocess(data_.size1())), ia2_.compose(
    					ia2.preprocess(data_.size2())), 0);
    }*/

    // Assignment
    BOOST_UBLAS_INLINE
    matrix_projected &operator =(const matrix_projected &mi) {
        matrix_assign<scalar_assign> (*this, mi);
        return *this;
    }
    BOOST_UBLAS_INLINE
    matrix_projected &assign_temporary(matrix_projected &mi) {
        return *this = mi;
    }
    template<class AE>
    BOOST_UBLAS_INLINE
    matrix_projected &operator = (const matrix_expression<AE> &ae) {
        matrix_assign<scalar_assign> (*this, typename matrix_temporary_traits<M>::type(ae));
        return *this;
    }
    template<class AE>
    BOOST_UBLAS_INLINE
    matrix_projected &assign(const matrix_expression<AE> &ae) {
        matrix_assign<scalar_assign> (*this, ae);
        return *this;
    }
    template<class AE>
    BOOST_UBLAS_INLINE
    matrix_projected &operator += (const matrix_expression<AE> &ae) {
        matrix_assign<scalar_assign> (*this, typename matrix_temporary_traits<M>::type(*this + ae));
        return *this;
    }
    template<class AE>
    BOOST_UBLAS_INLINE
    matrix_projected &plus_assign(const matrix_expression<AE> &ae) {
        matrix_assign<scalar_plus_assign> (*this, ae);
        return *this;
    }
    template<class AE>
    BOOST_UBLAS_INLINE
    matrix_projected &operator -= (const matrix_expression<AE> &ae) {
        matrix_assign<scalar_assign> (*this, typename matrix_temporary_traits<M>::type(*this - ae));
        return *this;
    }
    template<class AE>
    BOOST_UBLAS_INLINE
    matrix_projected &minus_assign(const matrix_expression<AE> &ae) {
        matrix_assign<scalar_minus_assign> (*this, ae);
        return *this;
    }
    template<class AT>
    BOOST_UBLAS_INLINE
    matrix_projected &operator *= (const AT &at) {
        matrix_assign_scalar<scalar_multiplies_assign> (*this, at);
        return *this;
    }
    template<class AT>
    BOOST_UBLAS_INLINE
    matrix_projected &operator /= (const AT &at) {
        matrix_assign_scalar<scalar_divides_assign> (*this, at);
        return *this;
    }

    // Closure comparison
    BOOST_UBLAS_INLINE
    bool same_closure(const matrix_projected &mi) const {
        return (*this).data_.same_closure(mi.data_);
    }

    // Comparison
    BOOST_UBLAS_INLINE
    bool operator ==(const matrix_projected &mi) const {
        return (*this).data_ == mi.data_ && ia1_ == mi.ia1_ && ia2_ == mi.ia2_;
    }

    // Swapping
    BOOST_UBLAS_INLINE
    void swap(matrix_projected mi) {
        if (this != &mi) {
            BOOST_UBLAS_CHECK(size1() == mi.size1(), bad_size());
            BOOST_UBLAS_CHECK(size2() == mi.size2(), bad_size());
            matrix_swap<scalar_swap> (*this, mi);
        }
    }
    BOOST_UBLAS_INLINE
    friend void swap(matrix_projected mi1, matrix_projected mi2) {
        mi1.swap(mi2);
    }

    // Iterator types
private:
    typedef typename M::const_iterator1 const_subiterator1_type;
    typedef typename M::iterator1 subiterator1_type;
    typedef typename M::const_iterator2 const_subiterator2_type;
    typedef typename M::iterator2 subiterator2_type;
    /*typedef typename IA::const_iterator const_subiterator1_type;
    typedef typename IA::const_iterator subiterator1_type;
    typedef typename IA::const_iterator const_subiterator2_type;
    typedef typename IA::const_iterator subiterator2_type;*/

public:
    /*#ifdef BOOST_UBLAS_USE_INDEXED_ITERATOR
    	typedef indexed_iterator1<matrix_projected<matrix_type, indirect_array_type>,
    	typename matrix_type::iterator1::iterator_category> iterator1;
    	typedef indexed_iterator2<matrix_projected<matrix_type, indirect_array_type>,
    	typename matrix_type::iterator2::iterator_category> iterator2;
    	typedef indexed_const_iterator1<matrix_projected<matrix_type, indirect_array_type>,
    	typename matrix_type::const_iterator1::iterator_category> const_iterator1;
    	typedef indexed_const_iterator2<matrix_projected<matrix_type, indirect_array_type>,
    	typename matrix_type::const_iterator2::iterator_category> const_iterator2;
    #else*/
    class const_iterator1;
    class iterator1;
    class const_iterator2;
    class iterator2;
//#endif
    typedef reverse_iterator_base1<const_iterator1> const_reverse_iterator1;
    typedef reverse_iterator_base1<iterator1> reverse_iterator1;
    typedef reverse_iterator_base2<const_iterator2> const_reverse_iterator2;
    typedef reverse_iterator_base2<iterator2> reverse_iterator2;

    // Element lookup
    BOOST_UBLAS_INLINE
    const_iterator1 find1(int /* rank */, size_type i, size_type j) const {
        std::assert(j == 0);
        const_subiterator1_type i1 = data_.find1(0, ia1_[i], 0);
        while (i1 != data_.end1() && !ia1_.isSelected(i1.index1()))
            ++i1;
        //const_subiterator2_type i2 = data_.find2(0, ia1_[i], ia2_[j]);
        return const_iterator1(*this, i1);
        //const_subiterator2_type i2 = data_.begin2();
        //return const_iterator1(*this, ia1_.begin() + i, ia2_.begin() + j);
    }
    BOOST_UBLAS_INLINE
    iterator1 find1(int /* rank */, size_type i, size_type j) {
        std::assert(j == 0);
        subiterator1_type i1 = data_.find1(0, ia1_[i], 0);
        while (i1 != data_.end1() && !ia1_.isSelected(i1.index1()))
            ++i1;
        return iterator1(*this, i1);
        //return iterator1(*this, ia1_.begin() + i, ia2_.begin() + j);
    }
    BOOST_UBLAS_INLINE
    const_iterator2 find2(int /* rank */, size_type i, size_type j) const {
        std::assert(i == 0);
        const_subiterator2_type i2 = data_.find1(0, 0, ia2_[j]);
        while (i2 != data_.end2() && !ia2_.isSelected(i2.index2()))
            ++i2;
        return const_iterator2(*this, i2);
        //return const_iterator2(*this, ia1_.begin() + i, ia2_.begin() + j);
    }
    BOOST_UBLAS_INLINE
    iterator2 find2(int /* rank */, size_type i, size_type j) {
        std::assert(i == 0);
        subiterator2_type i2 = data_.find1(0, 0, ia2_[j]);
        while (i2 != data_.end2() && !ia2_.isSelected(i2.index2()))
            ++i2;
        return iterator2(*this, i2);
        //return iterator2(*this, ia1_.begin() + i, ia2_.begin() + j);
    }

    // TODO: Iterators simply are indices.

    class const_iterator1: public container_const_reference<matrix_projected> ,
        public iterator_base_traits <
        typename M::const_iterator1::iterator_category >::template
        iterator_base<const_iterator1, value_type>::type {
        public:
        typedef typename M::const_iterator1::value_type value_type;
        typedef typename M::const_iterator1::difference_type difference_type;
        typedef typename M::const_reference reference; //FIXME due to indexing access
        typedef typename M::const_iterator1::pointer pointer;
        typedef const_iterator2 dual_iterator_type;
        typedef const_reverse_iterator2 dual_reverse_iterator_type;

        // Construction and destruction
        BOOST_UBLAS_INLINE const_iterator1() :
    container_const_reference<self_type> (), it1_() { /*, it2_()*/
    }
    BOOST_UBLAS_INLINE const_iterator1(const self_type &mi,
                                       const const_subiterator1_type &it1/*,
				const const_subiterator2_type &it2*/) :
    container_const_reference<self_type> (mi), it1_(it1) { /*, it2_(it2)*/
    }
    BOOST_UBLAS_INLINE const_iterator1(const iterator1 &it) :
    container_const_reference<self_type> (it()), it1_(it.it1_)/*, it2_(
					it.it2_)*/
    {
    }

    // Arithmetic
    BOOST_UBLAS_INLINE
    const_iterator1 &operator ++() {
        ++it1_;
        while (it1_.index1() < c_->size1() && !c_->ia1_.isSelected(it1_.index1()))
            ++it1_;
        return *this;
    }
    BOOST_UBLAS_INLINE
    const_iterator1 &operator --() {
        std::assert(0);
        //--it1_;
        return *this;
    }
    BOOST_UBLAS_INLINE
    const_iterator1 &operator +=(difference_type n) {
        std::assert(0);
        //it1_ += n;
        return *this;
    }
    BOOST_UBLAS_INLINE
    const_iterator1 &operator -=(difference_type n) {
        std::assert(0);
        //it1_ -= n;
        return *this;
    }
    BOOST_UBLAS_INLINE
    difference_type operator -(const const_iterator1 &it) const {
        std::assert(0);
        return 0;
        /*BOOST_UBLAS_CHECK((*this)().same_closure(it()), external_logic());
        BOOST_UBLAS_CHECK(it2_ == it.it2_, external_logic());
        return it1_ - it.it1_;*/
    }

    // Dereference
    BOOST_UBLAS_INLINE
    const_reference operator *() const {
        //std::cout << " ci1 (" << *it1_ << ", " << *it2_ << ")" << std::endl;
        // FIXME replace find with at_element
        //return (*this)().data_(*it1_, *it2_);
        return *it1_;
    }
    BOOST_UBLAS_INLINE
    const_reference operator [](difference_type n) const {
        std::assert(0);
        return 0;
        //return *(*this + n);
    }

#ifndef BOOST_UBLAS_NO_NESTED_CLASS_RELATION
    BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
    typename self_type::
#endif
    const_iterator2 begin() const {
        return c_->find2(0,);
        return const_iterator2((*this)(), it1_, it2_().begin());
    }
    BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
    typename self_type::
#endif
    const_iterator2 end() const {
        return const_iterator2((*this)(), it1_, it2_().end());
    }
    BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
    typename self_type::
#endif
    const_reverse_iterator2 rbegin() const {
        return const_reverse_iterator2(end());
    }
    BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
    typename self_type::
#endif
    const_reverse_iterator2 rend() const {
        return const_reverse_iterator2(begin());
    }
#endif

    // Indices
    BOOST_UBLAS_INLINE
    size_type index1() const {
        return it1_.index();
    }
    BOOST_UBLAS_INLINE
    size_type index2() const {
        return it2_.index();
    }

    // Assignment
    BOOST_UBLAS_INLINE
    const_iterator1 &operator =(const const_iterator1 &it) {
        container_const_reference<self_type>::assign(&it());
        it1_ = it.it1_;
        it2_ = it.it2_;
        return *this;
    }

    // Comparison
    BOOST_UBLAS_INLINE
    bool operator ==(const const_iterator1 &it) const {
        BOOST_UBLAS_CHECK((*this)().same_closure(it()), external_logic());
        BOOST_UBLAS_CHECK(it2_ == it.it2_, external_logic());
        return it1_ == it.it1_;
    }
    BOOST_UBLAS_INLINE
    bool operator <(const const_iterator1 &it) const {
        BOOST_UBLAS_CHECK((*this)().same_closure(it()), external_logic());
        BOOST_UBLAS_CHECK(it2_ == it.it2_, external_logic());
        return it1_ < it.it1_;
    }

    private:
    const_subiterator1_type it1_;
    const_subiterator2_type it2_;
    };

    BOOST_UBLAS_INLINE
    const_iterator1 begin1() const {
        return find1(0, 0, 0);
    }
    BOOST_UBLAS_INLINE
    const_iterator1 end1() const {
        return find1(0, size1(), 0);
    }

    class iterator1: public container_reference<matrix_projected> ,
        public iterator_base_traits <
        typename M::iterator1::iterator_category >::template
        iterator_base<iterator1, value_type>::type {
        public:
        typedef typename M::iterator1::value_type value_type;
        typedef typename M::iterator1::difference_type difference_type;
        typedef typename M::reference reference; //FIXME due to indexing access
        typedef typename M::iterator1::pointer pointer;
        typedef iterator2 dual_iterator_type;
        typedef reverse_iterator2 dual_reverse_iterator_type;

        // Construction and destruction
        BOOST_UBLAS_INLINE iterator1() :
    container_reference<self_type> (), it1_(), it2_() {
    }
    BOOST_UBLAS_INLINE iterator1(self_type &mi,
                                 const subiterator1_type &it1, const subiterator2_type &it2) :
    container_reference<self_type> (mi), it1_(it1), it2_(it2) {
    }

    // Arithmetic
    BOOST_UBLAS_INLINE
    iterator1 &operator ++() {
        ++it1_;
        return *this;
    }
    BOOST_UBLAS_INLINE
    iterator1 &operator --() {
        --it1_;
        return *this;
    }
    BOOST_UBLAS_INLINE
    iterator1 &operator +=(difference_type n) {
        it1_ += n;
        return *this;
    }
    BOOST_UBLAS_INLINE
    iterator1 &operator -=(difference_type n) {
        it1_ -= n;
        return *this;
    }
    BOOST_UBLAS_INLINE
    difference_type operator -(const iterator1 &it) const {
        BOOST_UBLAS_CHECK((*this)().same_closure(it()), external_logic());
        BOOST_UBLAS_CHECK(it2_ == it.it2_, external_logic());
        return it1_ - it.it1_;
    }

    // Dereference
    BOOST_UBLAS_INLINE
    reference operator *() const {
        // FIXME replace find with at_element
        std::cout << " i1 (" << *it1_ << ", " << *it2_ << ")" << std::endl;
        return (*this)().data_(*it1_, *it2_);
    }
    BOOST_UBLAS_INLINE
    reference operator [](difference_type n) const {
        return *(*this + n);
    }

#ifndef BOOST_UBLAS_NO_NESTED_CLASS_RELATION
    BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
    typename self_type::
#endif
    iterator2 begin() const {
        return iterator2((*this)(), it1_, it2_().begin());
    }
    BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
    typename self_type::
#endif
    iterator2 end() const {
        return iterator2((*this)(), it1_, it2_().end());
    }
    BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
    typename self_type::
#endif
    reverse_iterator2 rbegin() const {
        return reverse_iterator2(end());
    }
    BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
    typename self_type::
#endif
    reverse_iterator2 rend() const {
        return reverse_iterator2(begin());
    }
#endif

    // Indices
    BOOST_UBLAS_INLINE
    size_type index1() const {
        return it1_.index();
    }
    BOOST_UBLAS_INLINE
    size_type index2() const {
        return it2_.index();
    }

    // Assignment
    BOOST_UBLAS_INLINE
    iterator1 &operator =(const iterator1 &it) {
        container_reference<self_type>::assign(&it());
        it1_ = it.it1_;
        it2_ = it.it2_;
        return *this;
    }

    // Comparison
    BOOST_UBLAS_INLINE
    bool operator ==(const iterator1 &it) const {
        BOOST_UBLAS_CHECK((*this)().same_closure(it()), external_logic());
        BOOST_UBLAS_CHECK(it2_ == it.it2_, external_logic());
        return it1_ == it.it1_;
    }
    BOOST_UBLAS_INLINE
    bool operator <(const iterator1 &it) const {
        BOOST_UBLAS_CHECK((*this)().same_closure(it()), external_logic());
        BOOST_UBLAS_CHECK(it2_ == it.it2_, external_logic());
        return it1_ < it.it1_;
    }

    private:
    subiterator1_type it1_;
    subiterator2_type it2_;

    friend class const_iterator1;
    };

    BOOST_UBLAS_INLINE
    iterator1 begin1() {
        return find1(0, 0, 0);
    }
    BOOST_UBLAS_INLINE
    iterator1 end1() {
        return find1(0, size1(), 0);
    }

    class const_iterator2: public container_const_reference<matrix_projected> ,
        public iterator_base_traits <
        typename M::const_iterator2::iterator_category >::template
        iterator_base<const_iterator2, value_type>::type {
        public:
        typedef typename M::const_iterator2::value_type value_type;
        typedef typename M::const_iterator2::difference_type difference_type;
        typedef typename M::const_reference reference; //FIXME due to indexing access
        typedef typename M::const_iterator2::pointer pointer;
        typedef const_iterator1 dual_iterator_type;
        typedef const_reverse_iterator1 dual_reverse_iterator_type;

        // Construction and destruction
        BOOST_UBLAS_INLINE const_iterator2() :
    container_const_reference<self_type> (), it1_(), it2_() {
    }
    BOOST_UBLAS_INLINE const_iterator2(const self_type &mi,
                                       const const_subiterator1_type &it1,
                                       const const_subiterator2_type &it2) :
    container_const_reference<self_type> (mi), it1_(it1), it2_(it2) {
    }
    BOOST_UBLAS_INLINE const_iterator2(const iterator2 &it) :
    container_const_reference<self_type> (it()), it1_(it.it1_), it2_(
        it.it2_) {
    }

    // Arithmetic
    BOOST_UBLAS_INLINE
    const_iterator2 &operator ++() {
        ++it2_;
        return *this;
    }
    BOOST_UBLAS_INLINE
    const_iterator2 &operator --() {
        --it2_;
        return *this;
    }
    BOOST_UBLAS_INLINE
    const_iterator2 &operator +=(difference_type n) {
        it2_ += n;
        return *this;
    }
    BOOST_UBLAS_INLINE
    const_iterator2 &operator -=(difference_type n) {
        it2_ -= n;
        return *this;
    }
    BOOST_UBLAS_INLINE
    difference_type operator -(const const_iterator2 &it) const {
        BOOST_UBLAS_CHECK((*this)().same_closure(it()), external_logic());
        BOOST_UBLAS_CHECK(it1_ == it.it1_, external_logic());
        return it2_ - it.it2_;
    }

    // Dereference
    BOOST_UBLAS_INLINE
    const_reference operator *() const {
        // FIXME replace find with at_element
        std::cout << " ci2 (" << *it1_ << ", " << *it2_ << ")" << std::endl;
        return (*this)().data_(*it1_, *it2_);
    }
    BOOST_UBLAS_INLINE
    const_reference operator [](difference_type n) const {
        return *(*this + n);
    }

#ifndef BOOST_UBLAS_NO_NESTED_CLASS_RELATION
    BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
    typename self_type::
#endif
    const_iterator1 begin() const {
        return const_iterator1((*this)(), it1_().begin(), it2_);
    }
    BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
    typename self_type::
#endif
    const_iterator1 end() const {
        return const_iterator1((*this)(), it1_().end(), it2_);
    }
    BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
    typename self_type::
#endif
    const_reverse_iterator1 rbegin() const {
        return const_reverse_iterator1(end());
    }
    BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
    typename self_type::
#endif
    const_reverse_iterator1 rend() const {
        return const_reverse_iterator1(begin());
    }
#endif

    // Indices
    BOOST_UBLAS_INLINE
    size_type index1() const {
        return it1_.index();
    }
    BOOST_UBLAS_INLINE
    size_type index2() const {
        return it2_.index();
    }

    // Assignment
    BOOST_UBLAS_INLINE
    const_iterator2 &operator =(const const_iterator2 &it) {
        container_const_reference<self_type>::assign(&it());
        it1_ = it.it1_;
        it2_ = it.it2_;
        return *this;
    }

    // Comparison
    BOOST_UBLAS_INLINE
    bool operator ==(const const_iterator2 &it) const {
        BOOST_UBLAS_CHECK((*this)().same_closure(it()), external_logic());
        BOOST_UBLAS_CHECK(it1_ == it.it1_, external_logic());
        return it2_ == it.it2_;
    }
    BOOST_UBLAS_INLINE
    bool operator <(const const_iterator2 &it) const {
        BOOST_UBLAS_CHECK((*this)().same_closure(it()), external_logic());
        BOOST_UBLAS_CHECK(it1_ == it.it1_, external_logic());
        return it2_ < it.it2_;
    }

    private:
    const_subiterator1_type it1_;
    const_subiterator2_type it2_;
    };

    BOOST_UBLAS_INLINE
    const_iterator2 begin2() const {
        return find2(0, 0, 0);
    }
    BOOST_UBLAS_INLINE
    const_iterator2 end2() const {
        return find2(0, 0, size2());
    }

    class iterator2: public container_reference<matrix_projected> ,
        public iterator_base_traits <
        typename M::iterator2::iterator_category >::template
        iterator_base<iterator2, value_type>::type {
        public:
        typedef typename M::iterator2::value_type value_type;
        typedef typename M::iterator2::difference_type difference_type;
        typedef typename M::reference reference; //FIXME due to indexing access
        typedef typename M::iterator2::pointer pointer;
        typedef iterator1 dual_iterator_type;
        typedef reverse_iterator1 dual_reverse_iterator_type;

        // Construction and destruction
        BOOST_UBLAS_INLINE iterator2() :
    container_reference<self_type> (), it1_(), it2_() {
    }
    BOOST_UBLAS_INLINE iterator2(self_type &mi,
                                 const subiterator1_type &it1, const subiterator2_type &it2) :
    container_reference<self_type> (mi), it1_(it1), it2_(it2) {
    }

    // Arithmetic
    BOOST_UBLAS_INLINE
    iterator2 &operator ++() {
        ++it2_;
        return *this;
    }
    BOOST_UBLAS_INLINE
    iterator2 &operator --() {
        --it2_;
        return *this;
    }
    BOOST_UBLAS_INLINE
    iterator2 &operator +=(difference_type n) {
        it2_ += n;
        return *this;
    }
    BOOST_UBLAS_INLINE
    iterator2 &operator -=(difference_type n) {
        it2_ -= n;
        return *this;
    }
    BOOST_UBLAS_INLINE
    difference_type operator -(const iterator2 &it) const {
        BOOST_UBLAS_CHECK((*this)().same_closure(it()), external_logic());
        BOOST_UBLAS_CHECK(it1_ == it.it1_, external_logic());
        return it2_ - it.it2_;
    }

    // Dereference
    BOOST_UBLAS_INLINE
    reference operator *() const {
        // FIXME replace find with at_element
        std::cout << " i2 (" << *it1_ << ", " << *it2_ << ")" << std::endl;
        return (*this)().data_(*it1_, *it2_);
    }
    BOOST_UBLAS_INLINE
    reference operator [](difference_type n) const {
        return *(*this + n);
    }

#ifndef BOOST_UBLAS_NO_NESTED_CLASS_RELATION
    BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
    typename self_type::
#endif
    iterator1 begin() const {
        return iterator1((*this)(), it1_().begin(), it2_);
    }
    BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
    typename self_type::
#endif
    iterator1 end() const {
        return iterator1((*this)(), it1_().end(), it2_);
    }
    BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
    typename self_type::
#endif
    reverse_iterator1 rbegin() const {
        return reverse_iterator1(end());
    }
    BOOST_UBLAS_INLINE
#ifdef BOOST_UBLAS_MSVC_NESTED_CLASS_RELATION
    typename self_type::
#endif
    reverse_iterator1 rend() const {
        return reverse_iterator1(begin());
    }
#endif

    // Indices
    BOOST_UBLAS_INLINE
    size_type index1() const {
        return it1_.index();
    }
    BOOST_UBLAS_INLINE
    size_type index2() const {
        return it2_.index();
    }

    // Assignment
    BOOST_UBLAS_INLINE
    iterator2 &operator =(const iterator2 &it) {
        container_reference<self_type>::assign(&it());
        it1_ = it.it1_;
        it2_ = it.it2_;
        return *this;
    }

    // Comparison
    BOOST_UBLAS_INLINE
    bool operator ==(const iterator2 &it) const {
        BOOST_UBLAS_CHECK((*this)().same_closure(it()), external_logic());
        BOOST_UBLAS_CHECK(it1_ == it.it1_, external_logic());
        return it2_ == it.it2_;
    }
    BOOST_UBLAS_INLINE
    bool operator <(const iterator2 &it) const {
        BOOST_UBLAS_CHECK((*this)().same_closure(it()), external_logic());
        BOOST_UBLAS_CHECK(it1_ == it.it1_, external_logic());
        return it2_ < it.it2_;
    }

    private:
    subiterator1_type it1_;
    subiterator2_type it2_;

    friend class const_iterator2;
    };

    BOOST_UBLAS_INLINE
    iterator2 begin2() {
        return find2(0, 0, 0);
    }
    BOOST_UBLAS_INLINE
    iterator2 end2() {
        return find2(0, 0, size2());
    }

    // Reverse iterators

    BOOST_UBLAS_INLINE
    const_reverse_iterator1 rbegin1() const {
        return const_reverse_iterator1(end1());
    }
    BOOST_UBLAS_INLINE
    const_reverse_iterator1 rend1() const {
        return const_reverse_iterator1(begin1());
    }

    BOOST_UBLAS_INLINE
    reverse_iterator1 rbegin1() {
        return reverse_iterator1(end1());
    }
    BOOST_UBLAS_INLINE
    reverse_iterator1 rend1() {
        return reverse_iterator1(begin1());
    }

    BOOST_UBLAS_INLINE
    const_reverse_iterator2 rbegin2() const {
        return const_reverse_iterator2(end2());
    }
    BOOST_UBLAS_INLINE
    const_reverse_iterator2 rend2() const {
        return const_reverse_iterator2(begin2());
    }

    BOOST_UBLAS_INLINE
    reverse_iterator2 rbegin2() {
        return reverse_iterator2(end2());
    }
    BOOST_UBLAS_INLINE
    reverse_iterator2 rend2() {
        return reverse_iterator2(begin2());
    }

private:
    matrix_closure_type data_;
    indirect_array_type &ia1_;
    indirect_array_type &ia2_;
};







/// Specialization of temporary_traits
/*template <class M>
struct matrix_temporary_traits< matrix_projected<M> >
: matrix_temporary_traits< M > {};
template <class M>
struct matrix_temporary_traits< const matrix_projected<M> >
: matrix_temporary_traits< M > {};

template <class M>
struct vector_temporary_traits< matrix_projected<M> >
: vector_temporary_traits< M > {};
template <class M>
struct vector_temporary_traits< const matrix_projected<M> >
: vector_temporary_traits< M > {};*/

}
}
}
