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
	bool trackAim = true;
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
			trackAim = true;
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
			trackAim = !trackAim;

			if (trackAim) { std::cout << "Track aiming\n"; }
			else { std::cout << "Snap aiming\n"; }
		}

		if (GetAsyncKeyState(0x48) & 1) // H
		{
			autoShoot = !autoShoot;

			if (autoShoot)
			{
				std::cout << "Auto shoot enabled\n";

				trackAim = false;
				std::cout << "Snap aiming\n";
			}
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

		if ((GetAsyncKeyState(0xA0) & 1) || autoShoot) // Left Shift
		{
			uintptr_t localPlayer = GetLocalPlayer(clientDllBase);
			if (!IsValidPlayer(localPlayer)) { continue; }

			uintptr_t targetPlayer = GetClosestPlayer(engineTrace, traceRays, goForHeadShot, clientDllBase, localPlayer, targetClass);
			if (!IsValidPlayer(targetPlayer)) { continue; }

			if (trackAim) // constantly update the angles without shooting
			{
				while (GetAsyncKeyState(0xA0)) { Sleep(1); } // clearing it

				int health = *(int*)(targetPlayer + 0xE4);

				while (!GetAsyncKeyState(0xA0) && health > 1) // Left Shift
				{
					AimAngles angles = CalculateAimAngles(localPlayer, targetPlayer, goForHeadShot);

					if (!angles.valid) { break; }

					// set pitch and yaw
					*(float*)(engineDllBase + 0x53F354) = angles.pitch;
					*(float*)(engineDllBase + 0x53F358) = angles.yaw;

					health = *(int*)(targetPlayer + 0xE4);
				}
			}
			else // snap to target and send click input
			{
				AimAngles angles = CalculateAimAngles(localPlayer, targetPlayer, goForHeadShot);

				if (!angles.valid) { continue; }

				// set pitch and yaw
				*(float*)(engineDllBase + 0x53F354) = angles.pitch;
				*(float*)(engineDllBase + 0x53F358) = angles.yaw;

				MOUSEINPUT mouseInput;
				mouseInput.dwFlags = MOUSEEVENTF_LEFTDOWN;

				INPUT input;
				input.type = INPUT_MOUSE;
				input.mi = mouseInput;

				SendInput(1, &input, sizeof(INPUT)); // left click
				Sleep(50);

				mouseInput.dwFlags = MOUSEEVENTF_LEFTUP;
				input.mi = mouseInput;

				SendInput(1, &input, sizeof(INPUT)); // stop left clicking
				Sleep(1000);
			}
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
	std::cout << "T: toggle between track and snap aiming\n";
	std::cout << "H: auto shoot valid target\n";
	std::cout << "R: toggle ray tracing\n";
	std::cout << "0: aim for the closest enemy\n";
	std::cout << "1-9: aim for specific class\n\n";
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
	localPlayerPos.z += (*(float*)(localPlayer + 0x15C)) + 10; // add eye height

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
				targetPos = GetBonePosition(boneCache, GetHeadBoneIndex(playerClass));
				targetPos.z -= 5;
			}
			else
			{
				targetPos = GetBonePosition(boneCache, 1);
			}

			Ray_t ray;
			ray.start = localPlayerPos;
			ray.delta = targetPos - localPlayerPos;
			ray.is_ray = true;

			trace_t trace;
			TraceRay(engineTrace, ray, CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTER | CONTENTS_WINDOW | CONTENTS_HITBOX, nullptr, &trace);

			if (trace.entity != (void*)player)
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
		targetPos = GetBonePosition(boneCache, GetHeadBoneIndex(playerClass));
		targetPos.z += 4;
	}
	else
	{
		targetPos = GetBonePosition(boneCache, 1);
	}
	
	float distance = sqrt(pow(targetPos.x - localPlayerPos.x, 2.0f) + pow(targetPos.z - localPlayerPos.z, 2.0f) + pow(targetPos.y - localPlayerPos.y, 2.0f));

	PredictPosition(localPlayer, targetPlayer, targetPos);

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

int GetHeadBoneIndex(TF2Class playerClass) 
{
	switch (playerClass) 
	{
	case Demoman:
		return 16;
	case Engineer:
		return 8;
	default:
		return 6;
	}
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

	out.x += velocity.x / 60;
	out.y += velocity.y / 60;
	out.z += velocity.z / 60;
}