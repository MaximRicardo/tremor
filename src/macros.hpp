#pragma once

#ifdef __GNUC__
#define m_attribute(x) __attribute__(x)
#else
#define m_attrm_attribute(x)
#endif
