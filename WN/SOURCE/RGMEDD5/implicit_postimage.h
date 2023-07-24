#ifndef IMPLICIT_POSTIMAGE__H
#define IMPLICIT_POSTIMAGE__H

// Include meddly only after having included <gmpxx.h>
#include <meddly/meddly.h>

//-----------------------------------------------------------------------------

namespace MEDDLY {

class postimage_impl_opname : public specialized_opname {
public:
    postimage_impl_opname(const char *n);
    virtual ~postimage_impl_opname();

    /// Arguments should have type "implicit_relation", below
    virtual specialized_operation *buildOperation(arguments *a) override;
};

postimage_impl_opname* initImplPostImage();

extern postimage_impl_opname* IMPLICIT_POSTIMAGE_OPNAME;

//-----------------------------------------------------------------------------

void initialize_implicit_postimage_opname();
void cleanup_implicit_postimage_opname();

}; // MEDDLY

//-----------------------------------------------------------------------------
#endif // IMPLICIT_POSTIMAGE__H
