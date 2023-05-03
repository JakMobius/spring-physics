
#include "buffer-range.hpp"

BufferRange BufferRange::global{INT_MIN, INT_MAX};
BufferRange BufferRange::empty{INT_MAX, INT_MIN};