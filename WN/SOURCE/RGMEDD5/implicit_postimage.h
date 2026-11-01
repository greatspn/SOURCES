#ifndef IMPLICIT_POSTIMAGE__H
#define IMPLICIT_POSTIMAGE__H

// Include meddly only after having included <gmpxx.h>
#include <meddly/meddly.h>

//-----------------------------------------------------------------------------

namespace MEDDLY {

class saturation_operation;
class implicit_relation;

saturation_operation* createImplicitPostImage(implicit_relation* rel);

//-----------------------------------------------------------------------------

inline void initialize_implicit_postimage_opname() {}
inline void cleanup_implicit_postimage_opname() {}

}; // MEDDLY

//-----------------------------------------------------------------------------
#endif // IMPLICIT_POSTIMAGE__H
