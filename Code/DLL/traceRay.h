#pragma once
#include "mathStructs.h"

// from TF2 source code

// an eye is never valid in a solid
constexpr auto CONTENTS_SOLID = 0x1;
// translucent, but not watery (glass)
constexpr auto CONTENTS_WINDOW = 0x2;
// alpha-tested "grate" textures.  Bullets/sight pass through, but solids don't
constexpr auto CONTENTS_GRATE = 0x8;

// hits entities which are MOVETYPE_PUSH (doors, plats, etc.)
constexpr auto CONTENTS_MOVEABLE = 0x4000;

// should never be on a brush, only in game
constexpr auto CONTENTS_MONSTER = 0x2000000;
constexpr auto CONTENTS_DEBRIS = 0x4000000;
// use accurate hitboxes on trace
constexpr auto CONTENTS_HITBOX = 0x40000000;

struct Ray_t
{
	__declspec(align(16)) Vector3 start{};
	__declspec(align(16)) Vector3 delta{};
	__declspec(align(16)) Vector3 start_offset{};
	__declspec(align(16)) Vector3 extents{};
	bool is_ray{};
	bool is_swept{};
};

struct csurface_t
{
	const char* name;
	short		surfaceProps;
	unsigned short	flags;		// BUGBUG: These are declared per surface, not per material, but this database is per-material now
};

struct cplane_t
{
	Vector3	normal;
	float	dist;
	char	type;			// for fast side tests
	char	signbits;		// signx + (signy<<1) + (signz<<1)
	char	pad[2];
};

struct trace_t
{
public:
	Vector3			startpos;				// start position
	Vector3			endpos;					// final position
	cplane_t		plane;					// surface normal at impact

	float			fraction;				// time completed, 1.0 = didn't hit anything

	int				contents;				// contents on other side of surface hit
	unsigned short	dispFlags;				// displacement flags for marking surfaces with data

	bool			allsolid;				// if true, plane is not valid
	bool			startsolid;				// if true, the initial point was in a solid area


	float		fractionleftsolid;		// time we left a solid, only valid if we started in solid
	csurface_t	surface;				// surface hit (impact surface)

	int			hitgroup;				// 0 == generic, non-zero is specific body part
	short		physicsbone;			// physics bone hit by trace in studio
	
	void*		entity;

	// NOTE: this member is overloaded.
	// If hEnt points at the world entity, then this is the static prop index.
	// Otherwise, this is the hitbox index.
	int			hitbox;					// box hit by trace in studio
};