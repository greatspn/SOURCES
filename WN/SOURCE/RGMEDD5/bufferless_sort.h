//---------------------------------------------------------------------------------------
#ifndef __BUFFERLESS_MERGESORT_H__
#define __BUFFERLESS_MERGESORT_H__
//---------------------------------------------------------------------------------------

namespace bufferless_mergesort_details {

template<typename It>
static void reverse_seq(It a, It b) {
    for ( --b; a < b; a++, b-- )
        swap(*a, *b);
}

// swap the sequence [a,b) with [b,c).
template<typename It>
static It rotate_seq(It a, It b, It c) {
    if (a != b && b != c) {
       reverse_seq(a, b);
       reverse_seq(b, c);
       reverse_seq(a, c);
    }
    return a + (c - b);
}

// find first element not less than @p key in sorted sequence or end of
// sequence (@p b) if not found.
template<typename It>
static It lower_bound(It a, It b, const typename It::value_type key) {
    int i;
    for ( i = b-a; i != 0; i /= 2 ) {
        It mid = a + i/2;
        if (*mid < key)
            a = mid + 1, i--;
    }
    return a;
}

// find first element greater than @p key in sorted sequence or end of
// sequence (@p b) if not found. 
template<typename It>
static It upper_bound(It a, It b, const typename It::value_type key) {
    int i;
    for ( i = b-a; i != 0; i /= 2 ) {
        It mid = a + i/2;
        if (*mid <= key)
            a = mid + 1, i--;
    }
    return a;
}

// inplace merge. 
template<typename It>
static void ip_merge(It a, It b, It c) {
    int n1 = b - a;
    int n2 = c - b;

    if (n1 == 0 || n2 == 0)
       return;
    if (n1 == 1 && n2 == 1) {
       if (*b < *a)
          swap(*a, *b);
    }
    else {
       It p, q;

       if (n1 <= n2)
          p = upper_bound(a, b, *(q = b+n2/2));
       else
          q = lower_bound(b, c, *(p = a+n1/2));
       b = rotate_seq(p, b, q);

       ip_merge(a, p, b);
       ip_merge(b, q, c);
    }
}

} // namespace bufferless_mergesort_details

template<typename It>
static void bufferless_mergesort(It v, size_t n) {
    if (n > 1) {
       int h = n/2;
       bufferless_mergesort(v, h); 
       bufferless_mergesort(v+h, n-h);
       
       bufferless_mergesort_details::ip_merge(v, v+h, v+n);
    }
}

//---------------------------------------------------------------------------------------
#endif // __BUFFERLESS_MERGESORT_H__
