//-----------------------------------------------------------------------------
/// \file selectors.h
/// Element selectors for the numerical methods. A selector is a filter that
/// tells on which elements a given operation should be applied.
///
/// \author Amparore Elvio
///
//-----------------------------------------------------------------------------

#ifndef __SELECTORS_H__
#define __SELECTORS_H__

/** \addtogroup Numerical */ /* @{ */

//-----------------------------------------------------------------------------


#if defined USE_PRIVATE_TYPES && !defined NDEBUG
struct SubsetIndexStructTag;
typedef private_integer<int, SubsetIndexStructTag>  subsetindex_t;
#else
typedef int  subsetindex_t;
#endif


//-----------------------------------------------------------------------------
// Element Selectors: used to select partial set of elements from a vector
//  or a partial set of rows/columns from a matrix.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
/// Selector for a contiguous range [0, N). \n
/// This class is essentially a dummy class used for template algorithm
/// that works with an un-needed row/column selector.
class full_element_selector : boost::noncopyable {
    const size_t N;	  ///< Selected range size.
public:
    /// Initialize a full_element_selector instance.
    ///   \param _N  number of elements in range [0, N).
    inline full_element_selector(size_t _N);

    /// Test if a value \a k is in the selected range.
    ///   \param k  Tested value.
    ///   \return True if the element k is in range [0, N), with 0<=k<N.
    inline bool isSelected(size_t k) const;

    /// Returns the selected elements count.
    ///   \return The range value N.
    inline size_t count() const;

    /// Return the n-th selected element.
    ///   \param n  Selected element index.
    ///   \return Simply returns n.
    inline size_t operator[](size_t n) const;
};

inline full_element_selector::full_element_selector(size_t _N) : N(_N) { }
inline bool full_element_selector::isSelected(size_t k) const { return true; }
inline size_t full_element_selector::count() const { return N; }
inline size_t full_element_selector::operator[](size_t n) const { return n; }



//-----------------------------------------------------------------------------
/// Selector for a subset of elements, specified as a vector. \n
/// The subset is encoded in this way: a \a classes vector and a comparison
/// value \a selClass are specified. An element \e k is selected if
/// \a classes[\a k] == \a selClass.
template<class T, class comparator = equal_to<T> >
class subset_element_selector : boost::noncopyable {
    T                selClass; ///< Comparison element of the selected class.
    const vector<T> *classes;  ///< Element classes.
    vector<size_t>   nthSel;   ///< Indexes of the selected elements.
    comparator		 comp;	   ///< Element comparator (default is ==).
public:
    /// Initialize a subset_element_selector instance.
    ///   \param _classes   Classes vector.
    ///   \param _selClass  Selected class on which the selector will be built.
    ///   \param _comp      Comparator for the selected element in _classes
    inline subset_element_selector(const vector<T> *_classes, T _selClass,
                                   comparator &_comp)
        : selClass(_selClass), classes(_classes), comp(_comp) {
        _init();
    }

    /// Initialize a subset_element_selector instance.
    ///   \param _classes   Classes vector.
    ///   \param _selClass  Selected class on which the selector will be built.
    inline subset_element_selector(const vector<T> *_classes, T _selClass)
        : selClass(_selClass), classes(_classes) {
        comp = comparator();
        _init();
    }

    /// Test if a value \a k is in the selected range.
    ///   \param k  Tested value.
    ///   \return True if the element k is selected, with 0<=k<N.
    inline bool isSelected(size_t k) const
    {   return comp((*classes)[k], selClass);   }

    /// Returns the selected elements count.
    ///   \return The number of selected elements
    inline size_t count() const
    {   return nthSel.size();   }

    /// Return the n-th selected element.
    ///   \param n  Selected element index.
    ///   \return The n-th selected element
    inline size_t operator[](size_t n) const
    {   return nthSel[n];   }

private:
    inline void _init() {
        nthSel.resize((size_t)std::count_if(classes->begin(), classes->end(),
                                            bind2nd(comp, selClass)));
        size_t nth = 0;
        for (size_t k = 0; k < classes->size(); k++)
            if (comp((*classes)[k], selClass))
                nthSel[nth++] = k;
        assert(nth == nthSel.size());
    }
};

/*template<class comparator>
inline subset_element_selector
::subset_element_selector(const vector<int> *_classes, int _selClass,
						  comparator& _comp)
: classes(_classes), selClass(_selClass), comp(_comp)
{
	nthSel.resize((size_t)std::count(classes->begin(), classes->end(), selClass));
	size_t nth=0;
	for (size_t k=0; k<classes->size(); k++)
		if (comp((*classes)[k], selClass))
			nthSel[nth++] = k;
	assert(nth == nthSel.size());
}

template<class comparator>
inline bool subset_element_selector::isSelected(size_t k) const
{   return comp((*classes)[k], selClass);   }

template<class comparator>
inline size_t subset_element_selector::count() const
{   return nthSel.size();   }

template<class comparator>
inline size_t subset_element_selector::operator[] (size_t n) const
{   return nthSel[n];   }*/



//-----------------------------------------------------------------------------
/// Container for a list of subsets. For each subset the list of states is
/// stored. Also, an inverse map (marking -> subset) is kept. Subset indices
/// are stored in an internal zero-based way: however, external indices may be
/// different.
/// The function CONV.externalSubsetTo0BasedIndex() does the conversion.
template<class CONV, class SUBSET, class STATE>
class BaseSubsetList : boost::noncopyable {
public:
    typedef SUBSET		extsubset_t;
    typedef STATE		extstate_t;
    typedef typename vector<extstate_t>::const_iterator  const_iterator;
private:
    /// Number of state subsets
    extsubset_t				subsetNum;
    /// Subset spans in the nthSel array
    vector<size_t>			subsetStart;
    /// State indices of each subset i are in [subsetStart[i], subsetStart[i+1])
    vector<extstate_t>		nthSubset;
    /// Inverse subset map: given a state it tells which subset it belongs
    vector<extsubset_t>		stateToSubset;
    // Internal <-> external subset index conversion functor
    CONV					conv;
    // Inverse state map: for a given state it tells which cardinal position
    //  it has in its subset
    vector<size_t>			stateToSubsetPos;
public:

    /// Initialize an empty subset list.
    inline BaseSubsetList() { }

    /// Construct the subset list from the state->subset map \a _stateToSubset .
    ///   \param _subsetNum       Number of state subset in \a _stateToSubset
    ///   \param _stateToSubset   Table of partition indices, one for each state.
    /// \note The given vector is destroyed on exit.
    inline void ConstructSubsetList(extsubset_t _subsetNum,
                                    vector<extsubset_t> &_stateToSubset) {
        subsetNum = _subsetNum;
        stateToSubset.swap(_stateToSubset);
        stateToSubsetPos.resize(numStates());

        // Count the number of states in each subset
        vector<size_t> stCounts(subsetNum);
        for (size_t i = 0; i < stateToSubset.size(); i++) {
            const subsetindex_t sid(conv.externalSubsetTo0BasedIndex(stateToSubset[i]));
            assert(sid >= subsetindex_t(0) && sid < subsetindex_t(subsetCount()));
            stateToSubsetPos[i] = stCounts[sid];
            stCounts[sid]++;
        }
        subsetStart.resize(size_t(subsetNum) + 1);
        subsetStart[0] = 0;
        for (subsetindex_t s = subsetindex_t(0); s < subsetindex_t(subsetCount()); s++)
            subsetStart[s + subsetindex_t(1)] = subsetStart[s] + stCounts[s];

        // Build up the subset->state list nthSubset
        nthSubset.resize(stateToSubset.size());
        fill(stCounts.begin(), stCounts.end(), 0);
        for (extstate_t i = 0; i < extstate_t(stateToSubset.size()); i++) {
            const subsetindex_t sid(conv.externalSubsetTo0BasedIndex(stateToSubset[i]));
            assert(sid >= subsetindex_t(0) && sid < subsetindex_t(subsetCount()));
            nthSubset[extstate_t(subsetStart[sid] + stCounts[sid])] = i;
            ++stCounts[sid];
        }
    }

    /// Return the number of state subsets
    inline extsubset_t subsetCount() const   {   return subsetNum;   }

    /// Return the number of states
    inline extstate_t numStates() const   {   return extstate_t(stateToSubset.size());   }

    /// Return the number of states in a given subset
    ///   \param subset  Subset index
    inline extstate_t numStatesIn(const extsubset_t subset) const {
        const subsetindex_t sid(conv.externalSubsetTo0BasedIndex(subset));
        assert(sid >= subsetindex_t(0) && sid < subsetindex_t(subsetCount()));
        return (subsetStart[sid + subsetindex_t(1)] - subsetStart[sid]);
    }

    /// Return the stateToSubset map given to ConstructSubsetList method
    inline const vector<extsubset_t> &getStateToSubsetMap() const {
        return stateToSubset;
    }

    /// Return the position of state \a st inside its subset
    inline size_t positionInsideSubset(const extstate_t st) const {
        size_t pos = stateToSubsetPos[st];
        assert(beginSubset(subsetOf(st))[pos] == st);
        return pos;
    }


    /// Return the subset of a given states
    ///   \param st  State index
    inline extsubset_t subsetOf(const extstate_t st) const {
        assert(/*st >= extstate_t(0) &&*/ st < numStates());
        return stateToSubset[st];
    }

    /// Begin state iterator of a given subset.
    ///   \param subset  Subset index
    inline const_iterator beginSubset(const extsubset_t subset) const {
        const subsetindex_t sid(conv.externalSubsetTo0BasedIndex(subset));
        assert(sid >= subsetindex_t(0) && sid < subsetindex_t(subsetCount()));
        return nthSubset.begin() + subsetStart[sid];
    }

    /// End state iterator of a given subset.
    ///   \param subset  Subset index
    inline const_iterator endSubset(const extsubset_t subset) const {
        const subsetindex_t sid(conv.externalSubsetTo0BasedIndex(subset));
        assert(sid >= subsetindex_t(0) && sid < subsetindex_t(subsetCount()));
        return nthSubset.begin() + subsetStart[sid + subsetindex_t(1)];
    }

    /// Element selector for a given subset
    class selector {
        const extsubset_t *pStateToSubset;
        const extstate_t *pStates;
        size_t sCount;
        extsubset_t selSubset;
    public:
        /// Initialize a selector instance for a specified subset
        ///   \param p1   state->subset vector pointer
        ///   \param p2   subset states vector
        ///   \param cnt  number of states in the selected subset
        ///   \param sel  Selected subset id
        inline selector(const extsubset_t *p1, const extstate_t *p2,
                        size_t cnt, extsubset_t sel)
            : pStateToSubset(p1), pStates(p2), sCount(cnt), selSubset(sel) { }

        /// Test if a value \a k is in the selected range.
        ///   \param k  Tested value.
        ///   \return True if the element k is selected, with 0<=k<N.
        inline bool isSelected(size_t k) const {
            return pStateToSubset[k] == selSubset;
        }

        /// Returns the selected elements count.
        ///   \return The number of selected elements
        inline size_t count() const {
            return sCount;
        }

        /// Return the n-th selected element.
        ///   \param n  Selected element index.
        ///   \return The n-th selected element
        inline size_t operator[](size_t n) const {
            assert(n < sCount);
            return size_t(pStates[n]);
        }
    };

    /// Create a subset selector for a given \a subset of states.
    ///   \param subset   Index of the selected subset
    inline selector selectorOf(const extsubset_t subset) const {
        const subsetindex_t sid(conv.externalSubsetTo0BasedIndex(subset));
        assert(sid >= subsetindex_t(0) && sid < subsetindex_t(subsetCount()));
        return selector(&stateToSubset[0], &nthSubset[subsetStart[sid]],
                        numStatesIn(subset), subset);
    }
};

//-----------------------------------------------------------------------------

template<class Vector, class Predicate>
class subset_selector : boost::noncopyable {
    const Vector &vec;			/// vector of T
    std::vector<int> revInd;	/// reverse index
    Predicate pred;				/// unary_function<T,bool>
public:
    /// Initialize a subset_selector instance.
    subset_selector(const Vector &_vec, Predicate _pred = Predicate()) : vec(_vec), pred(_pred) {
        size_t cnt = count_if(vec.begin(), vec.end(), pred), k = 0;
        revInd.resize(cnt);
        for (size_t i = 0; i < vec.size(); i++)
            if (pred(vec[i]))
                revInd[k++] = i;
        assert(k == cnt);
    }

    /// Return if the k-th element is in the selected subset.
    inline bool isSelected(size_t k) const
    {	return pred(vec[k]);   }

    /// Returns the selected elements count.
    inline size_t count() const
    {   return revInd.size();   }

    /// Return the n-th selected element.
    inline size_t operator[](size_t n) const
    {   return revInd[n];   }
};

//-----------------------------------------------------------------------------
/* @}  Numerical documentiation group. */
#endif  // __SELECTORS_H__
