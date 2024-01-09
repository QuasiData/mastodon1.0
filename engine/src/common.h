#pragma once
#include "id.h"

#define DISABLE_COPY(x)									\
				x(const x##&) = delete;					\
				x##& operator=(const x##&) = delete;

#define DISABLE_MOVE(x)									\
				x(x##&&) = delete;						\
				x##& operator=(x##&&) = delete;

#define DISABLE_COPY_AND_MOVE(x)						\
			    x(const x##&) = delete;					\
				x##& operator=(const x##&) = delete;	\
				x(x##&&) = delete;						\
				x##& operator=(x##&&) = delete;

#ifdef _DEBUG
#define DEBUG_OP(x) x
#else
#define DEBUG_OP(x) (void(0))
#endif

#define IMPLEMENT_TYPED_BITFLAG_OPERATORS(name)																											\
inline name operator~(name a)																															\
{																																						\
	return static_cast<name>(~static_cast<std::underlying_type_t<name>>(a));																			\
}																																						\
                                                                                                                                                        \
inline name operator&(name a, name b)																													\
{																																						\
	return static_cast<name>(static_cast<std::underlying_type_t<name>>(a) & static_cast<std::underlying_type_t<name>>(b));								\
}																																						\
																																						\
inline name& operator&=(name& a, const name b)																											\
{																																						\
	return a = a & b;																																	\
}																																						\
																																						\
inline name operator|(name a, name b)																													\
{																																						\
	return static_cast<name>(static_cast<std::underlying_type_t<name>>(a) | static_cast<std::underlying_type_t<name>>(b));								\
}																																						\
																																						\
inline name& operator|=(name& a, const name b)																											\
{																																						\
	return a = a | b;																																	\
}																																						\
																																						\
inline name operator-(name a, name b)																													\
{																																						\
	return static_cast<name>(static_cast<std::underlying_type_t<name>>(a) - static_cast<std::underlying_type_t<name>>(b));								\
}																																						\
																																						\
inline name& operator-=(name& a, const name b)																											\
{																																						\
	return a = a - b;																																	\
}