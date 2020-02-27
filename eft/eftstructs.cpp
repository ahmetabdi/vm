#include "eftstructs.h"
#include "xorstr.hpp"
#include <xmmintrin.h>  
#include <emmintrin.h>
#include <fstream>
#include <locale>
#include <codecvt>
#include "globals.h"

std::list<uint64_t> bodypart = { BodyParts::Head, BodyParts::Thorax, BodyParts::Stomach, BodyParts::LeftArm, BodyParts::RightArm, BodyParts::LeftLeg, BodyParts::RightLeg };

FMatrix FMatrixTranspose(FMatrix matrix)
{
    FMatrix newMatrix = FMatrix();

    newMatrix.m[0][0] = matrix.m[0][0];// 0
    newMatrix.m[0][1] = matrix.m[1][0];// 1
    newMatrix.m[0][2] = matrix.m[2][0];// 2
    newMatrix.m[0][3] = matrix.m[3][0];// 3

    newMatrix.m[1][0] = matrix.m[0][1];// 4
    newMatrix.m[1][1] = matrix.m[1][1];// 5
    newMatrix.m[1][2] = matrix.m[2][1];// 6
    newMatrix.m[1][3] = matrix.m[3][1];// 7

    newMatrix.m[2][0] = matrix.m[0][2];// 8
    newMatrix.m[2][1] = matrix.m[1][2];// 9
    newMatrix.m[2][2] = matrix.m[2][2];// 10
    newMatrix.m[2][3] = matrix.m[3][2];// 11

    newMatrix.m[3][0] = matrix.m[0][3];// 12
    newMatrix.m[3][1] = matrix.m[1][3];// 13
    newMatrix.m[3][2] = matrix.m[2][3];// 14
    newMatrix.m[3][3] = matrix.m[3][3];// 15

    /*
    FMatrix m = { matrix.m[0], matrix.m[4], matrix.m[8], matrix.m[12],
        matrix.m[1], matrix.m[5], matrix.m[9], matrix.m[13],
        matrix.m[2], matrix.m[6], matrix.m[10], matrix.m[14],
        matrix.m[3], matrix.m[7], matrix.m[11], matrix.m[15] };
    return m;
    */
   return newMatrix;
}

EFTData* EFTData::Instance()
{
	static EFTData instance;
	return &instance;
}

/* All one time initialization in here*/
bool EFTData::InitOffsets()
{
	gameObjectManager = process->Read<uint64_t>(unityPlayerBase + this->offsets.offs_gameObjectManager);

	//printf("GOM: 0x%X\n", gameObjectManager);

	// Read pointer to activeObjects and lastActiveObject with 1 read, then find game world and local game world.
	auto active_objects = process->Read<std::array<uint64_t, 2>>(gameObjectManager + offsetof(EFTStructs::GameObjectManager, lastActiveObject));
	if (!active_objects[0] || !active_objects[1])
		return false;

	//printf("ActiveObjects: 0x%X\n", active_objects);

	if (!(gameWorld = GetObjectFromList(active_objects[1], active_objects[0], _xor_("GameWorld"))))
		return false;

	//printf("gameWorld: 0x%X\n", gameWorld);

	// Find fps camera.
	localGameWorld = process->ReadChain(gameWorld, { 0x30, 0x18, 0x28 });

	//printf("localgameWorld: 0x%X\n", localGameWorld);

	// Get tagged objects and find fps camera.
	auto tagged_objects = process->Read<std::array<uint64_t, 2>>(gameObjectManager + offsetof(EFTStructs::GameObjectManager, lastTaggedObject));
	if (!tagged_objects[0] || !tagged_objects[1])
		return false;

	if (!(fpsCamera = GetObjectFromList(tagged_objects[1], tagged_objects[0], _xor_("FPS Camera"))))
		return false;

	//printf("fpsCamera: 0x%X\n", fpsCamera);

	return true;
}

FVector EFTData::GetPosition(uint64_t transform)
{
	auto transform_internal = process->Read<uint64_t>(transform + 0x10);

	auto matrices = process->Read<uint64_t>(transform_internal + 0x38);
	auto index = process->Read<int>(transform_internal + 0x40);

	process->Read((uintptr_t)(matrices + 0x18), &matrix_list_base, sizeof(matrix_list_base));

	process->Read((uintptr_t)(matrices + 0x20), &dependency_index_table_base, sizeof(dependency_index_table_base));

	static auto get_dependency_index = [this](uint64_t base, int32_t index)
	{
		process->Read((uintptr_t)(base + index * 4), &index, sizeof(index));
		return index;
	};

	static auto get_matrix_blob = [this](uint64_t base, uint64_t offs, float* blob, uint32_t size) {
		process->Read((uintptr_t)(base + offs), blob, size);
	};

	int32_t index_relation = get_dependency_index(dependency_index_table_base, index);

	FVector ret_value;
	{
		float* base_matrix3x4 = (float*)malloc(64),
			* matrix3x4_buffer0 = (float*)((uint64_t)base_matrix3x4 + 16),
			* matrix3x4_buffer1 = (float*)((uint64_t)base_matrix3x4 + 32),
			* matrix3x4_buffer2 = (float*)((uint64_t)base_matrix3x4 + 48);

		get_matrix_blob(matrix_list_base, index * 48, base_matrix3x4, 16);

		__m128 xmmword_1410D1340 = { -2.f, 2.f, -2.f, 0.f };
		__m128 xmmword_1410D1350 = { 2.f, -2.f, -2.f, 0.f };
		__m128 xmmword_1410D1360 = { -2.f, -2.f, 2.f, 0.f };

		while (index_relation >= 0)
		{
			uint32_t matrix_relation_index = 6 * index_relation;

			// paziuret kur tik 3 nureadina, ten translationas, kur 4 = quatas ir yra rotationas.
			get_matrix_blob(matrix_list_base, 8 * matrix_relation_index, matrix3x4_buffer2, 16);
			__m128 v_0 = *(__m128*)matrix3x4_buffer2;

			get_matrix_blob(matrix_list_base, 8 * matrix_relation_index + 32, matrix3x4_buffer0, 16);
			__m128 v_1 = *(__m128*)matrix3x4_buffer0;

			get_matrix_blob(matrix_list_base, 8 * matrix_relation_index + 16, matrix3x4_buffer1, 16);
			__m128i v9 = *(__m128i*)matrix3x4_buffer1;

			__m128* v3 = (__m128*)base_matrix3x4; // r10@1
			__m128 v10; // xmm9@2
			__m128 v11; // xmm3@2
			__m128 v12; // xmm8@2
			__m128 v13; // xmm4@2
			__m128 v14; // xmm2@2
			__m128 v15; // xmm5@2
			__m128 v16; // xmm6@2
			__m128 v17; // xmm7@2

			v10 = _mm_mul_ps(v_1, *v3);
			v11 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, 0));
			v12 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, 85));
			v13 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, -114));
			v14 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, -37));
			v15 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, -86));
			v16 = _mm_castsi128_ps(_mm_shuffle_epi32(v9, 113));

			v17 = _mm_add_ps(
				_mm_add_ps(
					_mm_add_ps(
						_mm_mul_ps(
							_mm_sub_ps(
								_mm_mul_ps(_mm_mul_ps(v11, (__m128)xmmword_1410D1350), v13),
								_mm_mul_ps(_mm_mul_ps(v12, (__m128)xmmword_1410D1360), v14)),
							_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v10), -86))),
						_mm_mul_ps(
							_mm_sub_ps(
								_mm_mul_ps(_mm_mul_ps(v15, (__m128)xmmword_1410D1360), v14),
								_mm_mul_ps(_mm_mul_ps(v11, (__m128)xmmword_1410D1340), v16)),
							_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v10), 85)))),
					_mm_add_ps(
						_mm_mul_ps(
							_mm_sub_ps(
								_mm_mul_ps(_mm_mul_ps(v12, (__m128)xmmword_1410D1340), v16),
								_mm_mul_ps(_mm_mul_ps(v15, (__m128)xmmword_1410D1350), v13)),
							_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(v10), 0))),
						v10)),
				v_0);

			*v3 = v17;

			index_relation = get_dependency_index(dependency_index_table_base, index_relation);
		}

		ret_value = *(FVector*)base_matrix3x4;
		
		free(base_matrix3x4);
		//delete[] base_matrix3x4;
	}

	return ret_value;
}


bool EFTData::Read()
{
	players.clear();

	// Accumulate players.
	{
		uint64_t onlineusers = process->Read<uint64_t>(localGameWorld + 0x68);

		if (!onlineusers)
			return false;

		uint64_t list_base = process->Read<uint64_t>(onlineusers + offsetof(EFTStructs::List, listBase));
		int player_count = process->Read<int>(onlineusers + offsetof(EFTStructs::List, itemCount));

		if (player_count <= 0 || !list_base)
			return false;

		constexpr auto BUFFER_SIZE = 128;

		uint64_t player_buffer[BUFFER_SIZE];
		process->Read(list_base + offsetof(EFTStructs::ListInternal, firstEntry), player_buffer, sizeof(uint64_t) * player_count);

		EFTPlayer player;

		for (int i = 0; i < player_count; i++)
		{
			player.instance = player_buffer[i];
			playercount = player_count;

			uint64_t bone_matrix = this->getbone_matrix(player.instance);

			if (bone_matrix)
			{
				uint64_t bone = process->ReadChain(bone_matrix, { 0x20, 0x10, 0x38 });
				player.location = process->Read<FVector>(bone + 0xB0);

				player.Base = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanBase * 8))));
				player.Pelvis = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanPelvis * 8))));
				player.LThigh1 = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanLThigh1 * 8))));
				player.LThigh2 = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanLThigh2 * 8))));
				player.LCalf = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanLCalf * 8))));
				player.LFoot = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanLFoot * 8))));
				player.LToe = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanLToe * 8))));
				player.RThigh1 = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanRThigh1 * 8))));
				player.RThigh2 = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanRThigh2 * 8))));
				player.RCalf = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanRCalf * 8))));
				player.RFoot = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanRFoot * 8))));
				player.RToe = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanRToe * 8))));
				player.Spine1 = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanSpine1 * 8))));
				player.Spine2 = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanSpine2 * 8))));
				player.Spine3 = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanSpine3 * 8))));
				player.LCollarbone = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanLCollarbone * 8))));
				player.LUpperarm = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanLUpperarm * 8))));
				player.LForearm1 = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanLForearm1 * 8))));
				player.LForearm2 = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanLForearm2 * 8))));
				player.LForearm3 = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanLForearm3 * 8))));
				player.LPalm = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanLPalm * 8))));
				player.RCollarbone = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanRCollarbone * 8))));
				player.RUpperarm = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanRUpperarm * 8))));
				player.RForearm1 = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanRForearm1 * 8))));
				player.RForearm2 = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanRForearm2 * 8))));
				player.RForearm3 = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanRForearm3 * 8))));
				player.RPalm = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanRPalm * 8))));
				player.Neck = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanNeck * 8))));
				player.Head = GetPosition(process->Read<uint64_t>(bone_matrix + (0x20 + ((int)Bones::HumanHead * 8))));
			}

			// Leave this at the end to have all the data.
			if (process->Read<int>(player.instance + 0x18))
			{
				localPlayer = player;
				localPlayer.location = player.location;
			}

			players.emplace_back(player);
		}
	}

	/*
	this->extracts.clear();
	// Accumulate Extractions.
	{
		uint64_t exit_controller = process->Read<uint64_t>(this->offsets.localGameWorld + 0x18);

		if (!exit_controller)
			return false;

		uint64_t exit_point = process->Read<uint64_t>(exit_controller + 0x20);
		int exit_count = process->Read<int>(exit_point + 0x18);

		if (exit_count <= 0 || !exit_point)
			return false;

		constexpr auto BUFFER_SIZE = 24;

		uint64_t extract_buffer[BUFFER_SIZE];
		process->Read(exit_point + offsetof(EFTStructs::ListInternal, firstEntry), extract_buffer, sizeof(uint64_t) * exit_count);

		EFTExtract extract;

		for (int i = 0; i < exit_count; i++)
		{
			extract.instance = extract_buffer[i];

			if (!EFTData::Instance()->open_extract(extract.instance))
				continue;

			uint64_t transform = process->ReadChain(extract.instance, { 0x10, 0x30, 0x30, 0x8, 0x28 });
			extract.location = GetPosition(transform);

			uint64_t extract_name = process->ReadChain(extract.instance, { 0x58, 0x10 });

			if (extract_name)
			{
				int32_t nameLength = process->Read<int32_t>(extract_name + this->offsets.unicodeString.length);
				extract.name = process->GetUnicodeString(extract_name + this->offsets.unicodeString.stringBase, nameLength);

			}

			this->extracts.emplace_back(extract);

		}

	}
	*/

	// Get view matrix.
	{
		uint64_t temp = fpsCamera;
		if (!(temp = process->Read<uint64_t>(temp + 0x30)) || !(temp = process->Read<uint64_t>(temp + 0x18)))
			return false;

		//	printf("temp: 0x%X\n", temp);
		FMatrix temp_matrix;
		process->Read(temp + 0x00D8, &temp_matrix, sizeof(temp_matrix));
		viewMatrix = FMatrixTranspose(temp_matrix);
	}

	return true;
}


uint64_t EFTData::GetObjectFromList(uint64_t listPtr, uint64_t lastObjectPtr, const char* objectName)
{
	using EFTStructs::BaseObject;
	char name[256];
	uint64_t classNamePtr = 0x0;

	BaseObject activeObject = process->Read<BaseObject>(listPtr);
	BaseObject lastObject = process->Read<BaseObject>(lastObjectPtr);

	if (activeObject.object != 0x0)
	{
		while (activeObject.object != 0 && activeObject.object != lastObject.object)
		{
			classNamePtr = process->Read<uint64_t>(activeObject.object + 0x60);
			process->Read(classNamePtr + 0x0, &name, sizeof(name));
			if (strcmp(name, objectName) == 0)
			{
				return activeObject.object;
			}

			activeObject = process->Read<BaseObject>(activeObject.nextObjectLink);
		}
	}
	if (lastObject.object != 0x0)
	{
		classNamePtr = process->Read<uint64_t>(lastObject.object + 0x60);
		process->Read(classNamePtr + 0x0, &name, 256);
		if (strcmp(name, objectName) == 0)
		{
			return lastObject.object;
		}
	}

	return uint64_t();
}

bool EFTData::IsAiming(uint64_t	 instance)
{
	uint64_t m_pbreath = process->ReadChain(instance, { this->offsets.proceduralWeaponAnimation, 0x28 });
	return process->Read<bool>(m_pbreath + 0x88);
}

uint64_t EFTData::get_mpcamera(uint64_t instance)
{
	static std::vector<uint64_t> temp{ this->offsets.proceduralWeaponAnimation, 0x88, 0x20 };

	return process->ReadChain(instance, temp);
}

uint64_t EFTData::getbone_matrix(uint64_t instance)
{
	static std::vector<uint64_t> temp{ this->offsets.playerBody, 0x28, 0x28, 0x10 };

	return process->ReadChain(instance, temp);
}

bool EFTData::IsPlayer(uint64_t instance)
{
	static std::vector<uint64_t> tempchain{ this->offsets.profile, this->offsets.information };

	uint64_t information = process->ReadChain(instance, tempchain);

	if (process->Read<int32_t>(information + 0x54) != 0)
		return true;

	return false;
}

FMatrix EFTData::getoptic_matrix(uint64_t instance)
{
	FMatrix temp_matrix;
	FMatrix outmatrix;

	static std::vector<uint64_t> tempchain{ this->offsets.proceduralWeaponAnimation,0x88, 0x20, 0x28, 0x30 };

	uint64_t temp = process->ReadChain(instance, tempchain);

	//printf(_xor_("temp : 0x%X\n"), temp);
	process->Read(temp + 0x00D8, &temp_matrix, sizeof(temp_matrix));
	outmatrix = FMatrixTranspose(temp_matrix);

	return outmatrix;
}


bool EFTData::open_extract(uint64_t extract)
{
	if (!extract)
		return false;

	int32_t status = process->Read<int32_t>(extract + 0xA8); /* NotOpen = 1// Imcpmpleterequirement = 2 // Countdown = 3 // Open = 4 // Pending = 5 // Awaot ,amial Activation = 6*/

	if (status == 4)
		return true;


	return false;
}

