#include <iostream>

#include "memoryTools.h"
#include "boneCache.h"
#include "traceRay.h"

const float radiansToDegrees = (180 / 3.14159);

const float headOffset = 3;

enum TF2Class
{
	Any = 0,
	Scout = 1,
	Soldier = 3,
	Pyro = 7,
	Demoman = 4,
	Heavy = 6,
	Engineer = 9,
	Medic = 5,
	Sniper = 2,
	Spy = 8
};

void PrintControls();

void SendLeftClick();

typedef CBoneCache* (__thiscall* _GetBoneCache)(void* thisPtr, void* pStudioHdr);
_GetBoneCache GetBoneCache;

typedef void(__thiscall* _TraceRay)(void* thisPtr, const Ray_t& ray, unsigned int mask, void* filter, trace_t* trace);
_TraceRay TraceRay;

bool IsValidPlayer(uintptr_t player);

uintptr_t GetLocalPlayer(uintptr_t clientDllBase);
uintptr_t GetClosestPlayer(void* engineTrace, bool rayTrace, bool aimForHead, uintptr_t clientDllBase, uintptr_t localPlayer, TF2Class targetClass);

struct AimAngles
{
	float pitch, yaw;
	bool valid;
};

AimAngles CalculateAimAngles(uintptr_t localPlayer, uintptr_t targetPlayer, bool aimForHead);

typedef void* (__cdecl* _CreateInterface)(const char* name, int* returnCode);
void* GetInterface(const char* modName, const char* interfaceName);

int GetHeadBoneIndex(TF2Class playerClass);

Vector3 GetBonePosition(CBoneCache* pcache, int iBone);

void PredictPosition(uintptr_t localPlayer, uintptr_t targetPlayer, Vector3& out);