#include "eftstructs.h"
#include "xorstr.hpp"
#include <xmmintrin.h>  
#include <emmintrin.h>
#include <fstream>
#include <locale>
#include <codecvt>
#include "globals.h"
#include <thread>
#include <unistd.h>

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

	players.clear();

	for (uint64_t Object = process->Read<uint64_t>(gameObjectManager + 0x8); 
				Object != process->Read<uint64_t>(gameObjectManager); 
				Object = process->Read<uint64_t>(Object + 0x8))
	{
		uint64_t GameObject = process->Read<uint64_t>(Object + 0x10);
		ushort Tag = process->Read<ushort>(GameObject + 0x54);

		if (Tag == 5) {
			uint64_t ObjectClass = process->Read<uint64_t>(GameObject + 0x30);
			uint64_t Entity = process->Read<uint64_t>(ObjectClass + 0x18);

			matrix temp_matrix;
			process->Read(Entity + 0xDC, &temp_matrix, sizeof(temp_matrix));
			pViewMatrix = temp_matrix.transpose();

			continue;
		}

		if (Tag == 6) // Player Tag
		{
			uint64_t ObjectClass = process->Read<uint64_t>(GameObject + 0x30);
			uint64_t Entity = process->Read<uint64_t>(ObjectClass + 0x18);

			uint64_t Player = process->Read<uint64_t>(Entity + 0x28);

			if (Player == NULL)
				continue;

			EFTPlayer cPlayer = process->Read<EFTPlayer>(Entity + 0x28);

			//uint64_t objectNamePtr = process->read<uint64_t>(GameObject + 0x68);
			//readCString(objectNamePtr, 50, entity[c].objectName);

			char name[256];
			uint64_t classNamePtr = process->Read<uint64_t>(GameObject + 0x60);
			process->Read(classNamePtr + 0x0, &name, sizeof(name));

			//printf("%s\n", name);

			if (strcmp(name, "LocalPlayer") == 0) {
				uint64_t b1 = process->Read<uint64_t>(Player + 0xB0);
				uint64_t b2 = process->Read<uint64_t>(b1 + 0x48);
				uint64_t entity_bone = process->Read<uint64_t>(b2 + (0x20 + (BoneList::head * 0x8)));

				LocalPlayer.location = GetPosition(entity_bone);
				/*
				uint64_t Inventory = process->Read<uint64_t>(Player + 0x490);
				uint64_t Belt = process->Read<uint64_t>(Inventory + 0x28);
				uint64_t ItemList = process->Read<uint64_t>(Belt + 0x38);
				uint64_t Items = process->Read<uint64_t>(ItemList + 0x10);

				for (int ItemsOnBelt = 0; ItemsOnBelt <= 6; ItemsOnBelt++)
				{
					try
					{
						if (Items != 0)
						{
							uint64_t Item = process->Read<uint64_t>(Items + 0x20 + (ItemsOnBelt * 0x8));
							uint64_t Held = process->Read<uint64_t>(Item + 0x90);

							process->Write<float>(Held + 0x2D4, -1.0F); //No Aimcone
							process->Write<float>(Held + 0x2D8, -1.0F); //No Aimcone

							uint64_t RecoilPropert = process->Read<uint64_t>(Held + 0x2C0);

							process->Write<float>(RecoilPropert + 0x18, 0.0f);
							process->Write<float>(RecoilPropert + 0x1C, 0.0f);
							process->Write<float>(RecoilPropert + 0x20, 0.0f);
							process->Write<float>(RecoilPropert + 0x24, 0.0f);
						}
					}
					catch (std::exception)
					{

					}
				}
				*/
			} else {
				//BaseEntity + 0xB0 ] + 0x48 ] + 0x20 + BoneID * 8 ] + 0x10
				
				//if (process->Read<bool>(Player + 0x1E4)) // if dead skip
				//	continue;

				//printf("hp %f\n", process->Read<float>(Player + 0x1EC));
				//if (process->Read<float>(Entity + 0x1EC) < 0.8f)
				//	continue;
				//printf("health %f\n", process->Read<float>(Entity + 0x1EC));

				uint64_t b1 = process->Read<uint64_t>(Player + 0xB0);
				uint64_t b2 = process->Read<uint64_t>(b1 + 0x48);
				uint64_t head_bone = process->Read<uint64_t>(b2 + (0x20 + (BoneList::head * 0x8)));

				cPlayer.location = GetPosition(head_bone);

				//printf("%f %f %f\n", cPlayer.location.x, cPlayer.location.y, cPlayer.location.z);

				players.emplace_back(cPlayer);
			}
		}
	}

	printf("done");
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