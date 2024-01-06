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