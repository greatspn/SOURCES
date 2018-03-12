
#define POLICY_AGE (unsigned)(1)
#define POLICY_REDRAW (unsigned)(128)
#define POLICY_DISABLE (unsigned)(14)
#define POLICY_DIS_RANDOM (unsigned)(0)
#define POLICY_DIS_FIRST_DRAWN (unsigned)(2)
#define POLICY_DIS_LAST_DRAWN (unsigned)(4)
#define POLICY_DIS_FIRST_SCHED (unsigned)(6)
#define POLICY_DIS_LAST_SCHED (unsigned)(8)
/* three more disabling policies can be encoded */
#define POLICY_RE_ENABLE (unsigned)(112)
#define POLICY_RE_EN_RANDOM (unsigned)(0)
#define POLICY_RE_EN_FIRST_DRAWN (unsigned)(16)
#define POLICY_RE_EN_LAST_DRAWN (unsigned)(32)
#define POLICY_RE_EN_FIRST_DESCHED (unsigned)(46)
#define POLICY_RE_EN_LAST_DESCHED (unsigned)(64)
/* three more re-enabling policies can be encoded */

#define TIMING_DISTRIBUTION (unsigned)(224)
#define TIMING_NOSTAGES (unsigned)(31)
#define TIMING_ERLANG (unsigned)(0) /* includes Exponential */
#define TIMING_DISCRETE (unsigned)(32) /* includes Deterministic */
#define TIMING_LINEAR (unsigned)(64)
#define TIMING_COX_normalized (unsigned)(96)
#define TIMING_IPEREXP (unsigned)(128)
/* three more distributions can be encoded */

