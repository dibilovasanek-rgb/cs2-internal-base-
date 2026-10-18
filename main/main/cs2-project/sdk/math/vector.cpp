#include "vector.h"
#include "matrix.h"

vec3_t vec3_t::transform(matrix3x4_t trans)
{
	return { dot(trans[0]) + trans[0][3], dot(trans[1]) + trans[1][3], dot(trans[2]) + trans[2][3] };
}