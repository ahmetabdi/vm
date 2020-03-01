#pragma once
#include <string>
#include "math.hpp"
#include <vector>
#include <list>
#include <xmmintrin.h>  
#include <emmintrin.h>
#include <fstream>
#include <locale>
#include <codecvt>
#include <thread>
#include <unistd.h>

struct EFTOffsets
{
	static constexpr uint64_t offs_gameObjectManager = 0x163D080;

	// uint64_t gameObjectManager = 0x0;
	// uint64_t gameWorld = 0x0;
	// uint64_t localGameWorld = 0x0;
	// uint64_t fpsCamera = 0x0;
	// uint64_t OpticCamera = 0x0;

	static constexpr uint64_t itemList = 0x48; //UPDATED 1/11/2020
	static constexpr uint64_t registeredPlayers = 0x68; //UPDATED 1/11/2020
	static constexpr uint64_t m_LocalPlayerID = 0x30;
	static constexpr uint64_t length = 0x10; //good
	static constexpr uint64_t stringBase = 0x14; //good
	static constexpr uint64_t information = 0x28; //updated
	static constexpr uint64_t id = 0x10; //updated
	static constexpr uint64_t playerName = 0x10; //updated
	static constexpr uint64_t angles_0 = 0x1D0;
	static constexpr uint64_t angles_1 = 0x1D8;
	static constexpr uint64_t position = 0x1E0;
	static constexpr uint64_t m_pPlayerProfile = 0x03A0;
	static constexpr uint64_t movementContext = 0x38;
	static constexpr uint64_t proceduralWeaponAnimation = 0x70; //updayed 1/11/2020
	static constexpr uint64_t playerBody = 0x88; //updated 1/11/2020
	static constexpr uint64_t m_pHealthController = 0x3D0; //updated 1/11/2020
	static constexpr uint64_t profile = 0x3A0; //updayed 1/11/2020
};

namespace EFTStructs
{
	struct BaseObject
	{
		uint64_t previousObjectLink; //0x0000
		uint64_t nextObjectLink; //0x0008
		uint64_t object; //0x0010
	};

	struct GameObjectManager
	{
		uint64_t lastTaggedObject; //0x0000
		uint64_t taggedObjects; //0x0008
		uint64_t lastActiveObject; //0x0010
		uint64_t activeObjects; //0x0018
	}; //Size: 0x0010

	class GameObject
	{
	public:
		char pad_0x0000[0x8]; //0x0000
		__int32_t m_instanceID; //0x0008 
		char pad_0x000C[0x24]; //0x000C
		__int32_t m_label; //0x0030 
		char pad_0x0034[0x4]; //0x0034
		__int32_t m_size; //0x0038 
		char pad_0x003C[0x4]; //0x003C
		__int32_t m_capacity; //0x0040 
		char pad_0x0044[0x4]; //0x0044
		__int32_t m_layer; //0x0048 
		__int16_t m_tag; //0x004C 
		unsigned char m_isActive; //0x004E 
		unsigned char m_isActiveCached; //0x004F 
		unsigned char m_isDestroying; //0x0050 
		unsigned char m_isActivating; //0x0051 
		char pad_0x0052[0x6]; //0x0052
		std::string* m_objectName; //0x0058
	};

	class ListInternal
	{
	public:
		char pad_0x0000[0x20]; //0x0000
		uintptr_t* firstEntry; //0x0020 
	}; //Size=0x0028

	class List
	{
	public:
		char pad_0x0000[0x10]; //0x0000
		ListInternal* listBase; //0x0010 
		__int32_t itemCount; //0x0018 
	}; //Size=0x001C
}

struct EFTPlayer
{
	uintptr_t	 instance;
	FVector Base;
	FVector Pelvis;
	FVector LThigh1;
	FVector LThigh2;
	FVector LCalf;
	FVector LFoot;
	FVector LToe;
	FVector RThigh1;
	FVector RThigh2;
	FVector RCalf;
	FVector RFoot;
	FVector RToe;
	FVector Spine1;
	FVector Spine2;
	FVector Spine3;
	FVector LCollarbone;
	FVector LUpperarm;
	FVector LForearm1;
	FVector LForearm2;
	FVector LForearm3;
	FVector LPalm;
	FVector RCollarbone;
	FVector RUpperarm;
	FVector RForearm1;
	FVector RForearm2;
	FVector RForearm3;
	FVector RPalm;
	FVector Neck;
	FVector Head;
	FVector	location;
};

struct EFTExtract
{
	uintptr_t	 instance;
	std::string  name;
	FVector		 location;
};

/*
struct EFTLoot
{
	uintptr_t	instance;
	std::string name;
	Vector3		origin;
};*/


enum BodyParts : uint64_t
{
	Head = 0x20,
	Thorax = 0x28,
	Stomach = 0x30,
	LeftArm = 0x38,
	RightArm = 0x40,
	LeftLeg = 0x48,
	RightLeg = 0x50,
	value_max
};

class EFTData
{
public:
	static EFTData* Instance();

	bool InitOffsets();

	FVector GetPosition(uint64_t transform);

	bool Read();

	std::vector<EFTExtract> extracts;

	bool IsAiming(uint64_t instance);

	uint64_t get_mpcamera(uint64_t instance);
	FMatrix getoptic_matrix(uint64_t instance);
	bool open_extract(uint64_t extract);

	EFTOffsets offsets;

	bool IsPlayer(uint64_t instance);
private:
	uint64_t GetObjectFromList(uint64_t list, uint64_t lastObject, const char* objectName);
	uint64_t getbone_matrix(uint64_t instance);
	//std::vector<std::wstring> names;
};

enum BoneList : int
{
	l_hip = 1,
	l_knee,
	l_foot,
	l_toe,
	l_ankle_scale,
	pelvis,
	penis,
	GenitalCensor,
	GenitalCensor_LOD0,
	Inner_LOD0,
	GenitalCensor_LOD1,
	GenitalCensor_LOD2,
	r_hip,
	r_knee,
	r_foot,
	r_toe,
	r_ankle_scale,
	spine1,
	spine1_scale,
	spine2,
	spine3,
	spine4,
	l_clavicle,
	l_upperarm,
	l_forearm,
	l_hand,
	l_index1,
	l_index2,
	l_index3,
	l_little1,
	l_little2,
	l_little3,
	l_middle1,
	l_middle2,
	l_middle3,
	l_prop,
	l_ring1,
	l_ring2,
	l_ring3,
	l_thumb1,
	l_thumb2,
	l_thumb3,
	IKtarget_righthand_min,
	IKtarget_righthand_max,
	l_ulna,
	neck,
	head,
	jaw,
	eyeTranform,
	l_eye,
	l_Eyelid,
	r_eye,
	r_Eyelid,
	r_clavicle,
	r_upperarm,
	r_forearm,
	r_hand,
	r_index1,
	r_index2,
	r_index3,
	r_little1,
	r_little2,
	r_little3,
	r_middle1,
	r_middle2,
	r_middle3,
	r_prop,
	r_ring1,
	r_ring2,
	r_ring3,
	r_thumb1,
	r_thumb2,
	r_thumb3,
	IKtarget_lefthand_min,
	IKtarget_lefthand_max,
	r_ulna,
	l_breast,
	r_breast,
	BoobCensor,
	BreastCensor_LOD0,
	BreastCensor_LOD1,
	BreastCensor_LOD2,
	collision,
	displacement
};

class matrix
{
public:
	inline float* operator[]( int i )
	{
		return m[ i ];
	}

	inline const float* operator[]( int i ) const
	{
		return m[ i ];
	}

	inline float* Base( )
	{
		return &m[ 0 ][ 0 ];
	}

	inline const float* Base( ) const
	{
		return &m[ 0 ][ 0 ];
	}
public:

	inline matrix( )
	{
	}

	inline matrix(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33 )
	{
		Init(
			m00, m01, m02, m03,
			m10, m11, m12, m13,
			m20, m21, m22, m23,
			m30, m31, m32, m33
		);
	}

	inline void Init(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33
	)
	{
		m[ 0 ][ 0 ] = m00;
		m[ 0 ][ 1 ] = m01;
		m[ 0 ][ 2 ] = m02;
		m[ 0 ][ 3 ] = m03;

		m[ 1 ][ 0 ] = m10;
		m[ 1 ][ 1 ] = m11;
		m[ 1 ][ 2 ] = m12;
		m[ 1 ][ 3 ] = m13;

		m[ 2 ][ 0 ] = m20;
		m[ 2 ][ 1 ] = m21;
		m[ 2 ][ 2 ] = m22;
		m[ 2 ][ 3 ] = m23;

		m[ 3 ][ 0 ] = m30;
		m[ 3 ][ 1 ] = m31;
		m[ 3 ][ 2 ] = m32;
		m[ 3 ][ 3 ] = m33;
	}

	matrix transpose( ) const
	{
		return matrix(
			m[ 0 ][ 0 ], m[ 1 ][ 0 ], m[ 2 ][ 0 ], m[ 3 ][ 0 ],
			m[ 0 ][ 1 ], m[ 1 ][ 1 ], m[ 2 ][ 1 ], m[ 3 ][ 1 ],
			m[ 0 ][ 2 ], m[ 1 ][ 2 ], m[ 2 ][ 2 ], m[ 3 ][ 2 ],
			m[ 0 ][ 3 ], m[ 1 ][ 3 ], m[ 2 ][ 3 ], m[ 3 ][ 3 ] );
	}

	float m[ 4 ][ 4 ];
};