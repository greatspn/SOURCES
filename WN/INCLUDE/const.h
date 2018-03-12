//=============================================================================
#ifndef  __GSPN_BASE_CONST_H__
#define  __GSPN_BASE_CONST_H__
//=============================================================================

#if defined TOOL_GSPNRG
  #include "const.gsrg"
#elif defined TOOL_GSPNSIM
  #include "const.gssim"
#elif defined TOOL_SWN_NTRS  
  #include "const.esrg"
#elif defined TOOL_SWN_GGSC  
  #include "const.esrg"
#elif defined TOOL_SWN_STNDRD  
  #include "const.esrg"
#elif defined TOOL_SWN_GST_STNDRD  
  #include "const.esrg"
#elif defined TOOL_SWN_GST_PREP  
  #include "const.esrg"
#elif defined TOOL_WNSIM  
  #include "const.cs"
#elif defined TOOL_WNSYMB  
  #include "const.ss"
#elif defined TOOL_WNRG  
  #include "const.rg"
#elif defined TOOL_WNSRG  
  #include "const.srg"
#elif defined TOOL_MDWNRG  
  #include "const.MDWNrg"
#elif defined TOOL_MDWNSRG  
  #include "const.MDWNsrg"
#elif defined TOOL_WNESRG  
  #include "const.esrg"
// #elif defined TOOL_RGMEDD  
//   #include "const.MDDrg"
#elif defined TOOL_RGMEDD2  
  #include "const.MDDrg"
#elif defined TOOL_RGMEDD3  
  #include "const.MDDrg"
#elif defined TOOL_PN2ODE  
  #include "const.PN2ODE"
#elif defined TOOL_LIBGSPNSSP
  #include "const.libssp"
#elif defined TOOL_LIBGSPNRG
  #include "const.librg"
#elif defined TOOL_LIBGSPNSRG
  #include "const.libsrg"
#elif defined TOOL_LIBGSPNMCESRG
  #include "const.libmcesrg"
#elif defined TOOL_LIBGSPNMCDSRG
  #include "const.libmcdsrg"
#elif defined TOOL_ESRG_CTMC  
  #include "const.esrg"
#elif defined TOOL_MDP  
  #include "const.libmcesrg"
#elif defined TOOL_WNDSRG  
  #include "const.libmcdsrg"
#elif defined TOOL_WNRDSRG  
  #include "const.libmcdsrg"
#elif defined TOOL_WNDSRGSOLVER  
  #include "const.libmcdsrg"
#elif defined TOOL_WNESRGSOLVER  
  #include "const.libmcesrg"
#else
  #error "Missing TOOL specifier."
#endif

//=============================================================================
#endif   // __GSPN_BASE_CONST_H__ 



