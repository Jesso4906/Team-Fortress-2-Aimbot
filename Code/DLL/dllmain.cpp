#include "dllmain.h"

DWORD WINAPI Thread(LPVOID param)
{
	AllocConsole(); // create console
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);
	if (f == 0) 
	{ 
		FreeLibraryAndExitThread((HMODULE)param, 0);
		return 0; 
	}

	uintptr_t clientDllBase = (uintptr_t)GetModuleHandle(L"client.dll");
	uintptr_t engineDllBase = (uintptr_t)GetModuleHandle(L"engine.dll");

	if (clientDllBase == 0 || engineDllBase == 0)
	{ 
		std::cout << "Failed to get handle to client.dll or engine.dll module";
		std::getchar();
		std::getchar();

		fclose(f);
		FreeConsole();
		FreeLibraryAndExitThread((HMODULE)param, 0);

		return 0; 
	}

	GetBoneCache = (_GetBoneCache)(clientDllBase + 0x1CA220);
	TraceRay = (_TraceRay)(engineDllBase + 0x18E140);

	PrintControls();

	MOUSEINPUT mouseInput = {};
	INPUT input = {};
	input.type = INPUT_MOUSE;

	TF2Class targetClass = Any;

	bool goForHeadShot = true;
	bool autoShoot = false;
	bool traceRays = true;

	void* engineTrace = (void*)GetInterface("engine.dll", "EngineTraceClient003");

	while (!GetAsyncKeyState(0x2D)) // exit when ins key is pressed
	{
		if (GetAsyncKeyState(0x09) & 1) // Tab
		{
			PrintControls();

			targetClass = Any;

			goForHeadShot = true;
			autoShoot = false;
			traceRays = true;
		}
		
		if (GetAsyncKeyState(0x42) & 1) // B
		{ 
			goForHeadShot = !goForHeadShot; 

			if (goForHeadShot) { std::cout << "Aiming for head shots\n"; }
			else { std::cout << "Aiming for body shots\n"; }
		}

		if (GetAsyncKeyState(0x54) & 1) // T
		{
			autoShoot = !autoShoot;

			if (autoShoot) { std::cout << "Auto shoot enabled\n"; }
			else { std::cout << "Auto shoot disabled\n"; }
		}

		if (GetAsyncKeyState(0x52) & 1) // R
		{
			traceRays = !traceRays;

			if (traceRays) { std::cout << "Ray tracing enabled\n"; }
			else { std::cout << "Ray tracing disabled\n"; }
		}
		
		if (GetAsyncKeyState(0x30) & 1) { targetClass = Any; std::cout << "Targetting closest enemy\n"; }
		if (GetAsyncKeyState(0x31) & 1) { targetClass = Scout; std::cout << "Targetting closest scout\n"; }
		if (GetAsyncKeyState(0x32) & 1) { targetClass = Soldier; std::cout << "Targetting closest soldier\n"; }
		if (GetAsyncKeyState(0x33) & 1) { targetClass = Pyro; std::cout << "Targetting closest pyro\n"; }
		if (GetAsyncKeyState(0x34) & 1) { targetClass = Demoman; std::cout << "Targetting closest demoman\n"; }
		if (GetAsyncKeyState(0x35) & 1) { targetClass = Heavy; std::cout << "Targetting closest heavy\n"; }
		if (GetAsyncKeyState(0x36) & 1) { targetClass = Engineer; std::cout << "Targetting closest engineer\n"; }
		if (GetAsyncKeyState(0x37) & 1) { targetClass = Medic; std::cout << "Targetting closest medic\n"; }
		if (GetAsyncKeyState(0x38) & 1) { targetClass = Sniper; std::cout << "Targetting closest sniper\n"; }
		if (GetAsyncKeyState(0x39) & 1) { targetClass = Spy; std::cout << "Targetting closest spy\n"; }

		if ((GetAsyncKeyState(0xA0) & 1)) // Left Shift
		{
			uintptr_t localPlayer = GetLocalPlayer(clientDllBase);
			if (!IsValidPlayer(localPlayer)) { continue; }

			uintptr_t targetPlayer = GetClosestPlayer(engineTrace, traceRays, goForHeadShot, clientDllBase, localPlayer, targetClass);
			if (!IsValidPlayer(targetPlayer)) { continue; }

			// nopping these to prevent moving the view angles with the mouse
			SetByte((void*)(engineDllBase + 0x70C02), 0x90, 8);
			SetByte((void*)(engineDllBase + 0x70C14), 0x90, 8);

			while (GetAsyncKeyState(0xA0)) { Sleep(1); } // clearing it

			int health = *(int*)(targetPlayer + 0xE4);

			int counter = 0;
			while (!GetAsyncKeyState(0xA0) && health > 1) // Left Shift
			{
				AimAngles angles = CalculateAimAngles(localPlayer, targetPlayer, goForHeadShot);

				if (!angles.valid) { break; }

				// set pitch and yaw
				*(float*)(engineDllBase + 0x53F354) = angles.pitch;
				*(float*)(engineDllBase + 0x53F358) = angles.yaw;

				if (autoShoot && counter > 2000)
				{
					SendLeftClick();
					counter = 0;
				}
				counter++;

				health = *(int*)(targetPlayer + 0xE4);
			}

			// undo the nop
			SetBytes((void*)(engineDllBase + 0x70C02), (BYTE*)"\xF3\x0F\x11\x05\x4A\xE7\x4C\x00", 8);
			SetBytes((void*)(engineDllBase + 0x70C14), (BYTE*)"\xF3\x0F\x11\x05\x3C\xE7\x4C\x00", 8);
		} 
	}

	fclose(f);
	FreeConsole();
	FreeLibraryAndExitThread((HMODULE)param, 0);
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD  dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		CreateThread(0, 0, Thread, hModule, 0, 0);
	}

	return TRUE;
}

void PrintControls()
{
	system("cls");
	std::cout << "Key binds: \n";
	std::cout << "Insert: exit\n";
	std::cout << "Tab: reset settings\n";
	std::cout << "Left Shift: aim bot\n";
	std::cout << "B: toggle between head shots and body shots\n";
	std::cout << "T: auto shoot valid target\n";
	std::cout << "R: toggle ray tracing\n";
	std::cout << "0: aim for the closest enemy\n";
	std::cout << "1-9: aim for specific class\n\n";
}

void SendLeftClick() 
{
	MOUSEINPUT mouseInput;
	mouseInput.dwFlags = MOUSEEVENTF_LEFTDOWN;

	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi = mouseInput;

	SendInput(1, &input, sizeof(INPUT)); // left click

	mouseInput.dwFlags = MOUSEEVENTF_LEFTUP;
	input.mi = mouseInput;

	SendInput(1, &input, sizeof(INPUT)); // stop left clicking
	Sleep(750);
}

bool IsValidPlayer(uintptr_t player)
{
	return player != 0 && (*(int*)(player + 0xE4)) > 1 && *(int*)(player + 0x1BB0) > 0 && *(int*)(player + 0x1BB0) < 10; // checking health and class too
}

uintptr_t GetLocalPlayer(uintptr_t clientDllBase) 
{
	uintptr_t playerList = (clientDllBase + 0x106CC18);

	Vector3 localPlayerPos = (*(Vector3*)(clientDllBase + 0xFE15AC));

	for(int i = 0; i < 64; i++)
	{
		uintptr_t player = *(uintptr_t*)playerList;

		if (!IsValidPlayer(player))
		{ 
			playerList += 0x20;
			continue; 
		}

		Vector3 playerPos = (*(Vector3*)(player + 0x338));

		Vector3 diff = localPlayerPos - playerPos;

		if ((diff.x < 5 && diff.x > -5) && (diff.z < 5 && diff.z > -5) && (diff.y < 5 && diff.y > -5))
		{
			return player;
		}

		playerList += 0x20;
	}

	return 0;
}

uintptr_t GetClosestPlayer(void* engineTrace, bool rayTrace, bool aimForHead, uintptr_t clientDllBase, uintptr_t localPlayer, TF2Class targetClass)
{
	uintptr_t playerList = (clientDllBase + 0x106CC18);

	int localPlayerTeam = (*(int*)(localPlayer + 0xEC));

	Vector3 localPlayerPos = (*(Vector3*)(localPlayer + 0x338));
	localPlayerPos.z += (*(float*)(localPlayer + 0x15C)); // add eye height

	float minDist = 999999999.0f;

	uintptr_t targetPlayer = 0;

	for(int i = 0; i < 64; i++)
	{
		uintptr_t player = *(uintptr_t*)playerList;

		if (!IsValidPlayer(player))
		{ 
			playerList += 0x20;
			continue; 
		}

		int health = *(int*)(player + 0xE4);
		int team = *(int*)(player + 0xEC);
		TF2Class playerClass = *(TF2Class*)(player + 0x1BB0);

		if (player == localPlayer || health < 2 || localPlayerTeam == team || (targetClass != Any && playerClass != targetClass)) // player found, but shouldn't be targeted
		{ 
			playerList += 0x20;
			continue; 
		}

		Vector3 playerPos = (*(Vector3*)(player + 0x338));

		float dist = sqrt(pow(localPlayerPos.x - playerPos.x, 2.0f) + pow(localPlayerPos.z - playerPos.z, 2.0f) + pow(localPlayerPos.y - playerPos.y, 2.0f));

		if (rayTrace)
		{
			CBoneCache* boneCache = GetBoneCache((void*)player, nullptr);

			if (boneCache == 0)
			{
				playerList += 0x20;
				continue;
			}

			Vector3 targetPos;
			if (aimForHead)
			{
				TF2Class playerClass = *(TF2Class*)(player + 0x1BB0);
				HeadInfo headInfo = GetHeadInfo(playerClass);
				targetPos = GetBonePosition(boneCache, headInfo.boneId);
				targetPos.z += headInfo.heightOffset;
			}
			else
			{
				targetPos = GetBonePosition(boneCache, 1);
			}

			Vector3 direction = targetPos - localPlayerPos;

			Ray_t ray;
			ray.start = localPlayerPos + (direction * 0.01); // bit infront to avoid hitting local player
			ray.delta = direction;
			ray.is_ray = true;

			trace_t trace;
			TraceRay(engineTrace, ray, CONTENTS_SOLID | CONTENTS_DEBRIS | CONTENTS_MOVEABLE | CONTENTS_HITBOX | CONTENTS_WINDOW, nullptr, &trace);

			if (trace.entity != (void*)player && trace.surface.flags != 1088) // 1088 seems to be invis wall at spawn
			{
				playerList += 0x20;
				continue;
			}
		}

		if (dist < minDist)
		{
			minDist = dist;

			targetPlayer = player;
		}

		playerList += 0x20;
	}

	return targetPlayer;
}

AimAngles CalculateAimAngles(uintptr_t localPlayer, uintptr_t targetPlayer, bool aimForHead)
{
	AimAngles result = {};
	
	if (!IsValidPlayer(localPlayer) || !IsValidPlayer(targetPlayer)) { return result; }
	
	Vector3 localPlayerPos = (*(Vector3*)(localPlayer + 0x338));
	localPlayerPos.z += (*(float*)(localPlayer + 0x15C)); // add eye height

	CBoneCache* boneCache = GetBoneCache((void*)targetPlayer, nullptr);

	if (boneCache == 0) { return result; }

	Vector3 targetPos;
	if (aimForHead)
	{
		TF2Class playerClass = *(TF2Class*)(targetPlayer + 0x1BB0);
		HeadInfo headInfo = GetHeadInfo(playerClass);
		targetPos = GetBonePosition(boneCache, headInfo.boneId);
		targetPos.z += headInfo.heightOffset;
	}
	else
	{
		targetPos = GetBonePosition(boneCache, 1);
	}

	PredictPosition(localPlayer, targetPlayer, targetPos);
	
	float distance = sqrt(pow(targetPos.x - localPlayerPos.x, 2.0f) + pow(targetPos.z - localPlayerPos.z, 2.0f) + pow(targetPos.y - localPlayerPos.y, 2.0f));

	result.pitch = -(asin((targetPos.z - localPlayerPos.z) / distance) * radiansToDegrees);
	result.yaw = (atan2(targetPos.y - localPlayerPos.y, targetPos.x - localPlayerPos.x) * radiansToDegrees);
	result.valid = true;

	return result;
}

void* GetInterface(const char* modName, const char* interfaceName) 
{
	HMODULE moduleHandle = GetModuleHandleA(modName);
	if (moduleHandle == 0) { return nullptr; }

	_CreateInterface CreateInterface = (_CreateInterface)GetProcAddress(moduleHandle, "CreateInterface");

	int returnCode;
	return CreateInterface(interfaceName, &returnCode);
}

HeadInfo GetHeadInfo(TF2Class playerClass)
{
	HeadInfo result = {};

	switch (playerClass) 
	{
	case Soldier:
		result.boneId = 6;
		result.heightOffset = 6;
		break;
	case Pyro:
		result.boneId = 6;
		result.heightOffset = 3;
		break;
	case Demoman:
		result.boneId = 16;
		result.heightOffset = 5;
		break;
	case Engineer:
		result.boneId = 8;
		result.heightOffset = 6;
		break;
	case Sniper:
		result.boneId = 6;
		result.heightOffset = 5;
		break;
	case Spy:
		result.boneId = 6;
		result.heightOffset = 5;
		break;
	default:
		result.boneId = 6;
		result.heightOffset = 4;
		break;
	}

	return result;
}

Vector3 GetBonePosition(CBoneCache* pcache, int iBone)
{
	matrix3x4_t bonetoworld = *pcache->GetCachedBone(iBone);

	Vector3 result;
	MatrixGetColumn(bonetoworld, 3, result);

	return result;
}

void PredictPosition(uintptr_t localPlayer, uintptr_t targetPlayer, Vector3& out)
{
	if (!IsValidPlayer(localPlayer) || !IsValidPlayer(targetPlayer)) { return; }
	
	Vector3 targetPlayerVelocity = (*(Vector3*)(targetPlayer + 0x178));
	Vector3 localPlayerVelocity = (*(Vector3*)(localPlayer + 0x178));
	Vector3 velocity = targetPlayerVelocity - localPlayerVelocity;

	out.x += velocity.x / 70;
	out.y += velocity.y / 70;
	out.z += velocity.z / 70;
}