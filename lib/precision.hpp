#pragma once

#ifdef IS_USE_DOUBLE_PRECISION
using decimal = double;
#else
using decimal = float;
#endif
