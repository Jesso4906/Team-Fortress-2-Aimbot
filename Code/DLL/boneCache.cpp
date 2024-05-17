#include "boneCache.h"

void MatrixGetColumn(const matrix3x4_t& in, int column, Vector3& out)
{
	out.x = in[0][column];
	out.y = in[1][column];
	out.z = in[2][column];
}

matrix3x4_t* CBoneCache::BoneArray()
{
	return (matrix3x4_t*)((char*)(this + 1) + m_matrixOffset);
}

short* CBoneCache::StudioToCached()
{
	return (short*)((char*)(this + 1));
}

matrix3x4_t* CBoneCache::GetCachedBone(int studioIndex)
{
	int cachedIndex = StudioToCached()[studioIndex];
	if (cachedIndex >= 0)
	{
		return BoneArray() + cachedIndex;
	}
	return nullptr;
}