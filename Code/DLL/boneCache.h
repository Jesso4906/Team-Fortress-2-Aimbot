#include "mathStructs.h"

// from TF2 source code

void MatrixGetColumn(const matrix3x4_t& in, int column, Vector3& out);

class CBoneCache
{
public:
	CBoneCache* GetData() { return this; }
	unsigned int Size() { return m_size; }

	matrix3x4_t* GetCachedBone(int studioIndex);

public:
	float m_timeValid;
	int m_boneMask;

private:
	matrix3x4_t* BoneArray();
	short* StudioToCached();
	
	unsigned int m_size;
	unsigned short m_cachedBoneCount;
	unsigned short m_matrixOffset;
	unsigned short m_cachedToStudioOffset;
	unsigned short m_boneOutOffset;
};