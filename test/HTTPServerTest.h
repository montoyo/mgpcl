#pragma once
#ifdef M_HTST_C
#define M_HTST_PREFIX
#else
#define M_HTST_PREFIX extern
#endif

M_HTST_PREFIX void mTestHTTPServer();
