#include "../hlapi/hlapi.h"
#include "eftstructs.h"

inline WinProcess *process;
inline uintptr_t unityPlayerBase;

inline uint64_t gameObjectManager = 0x0;
inline uint64_t gameWorld = 0x0;
inline uint64_t localGameWorld = 0x0;
inline uint64_t fpsCamera = 0x0;
inline uint64_t OpticCamera = 0x0;

inline	int playercount;
//inline	EFTPlayer localPlayer;
//inline	FMatrix viewMatrix;

inline	uint64_t matrix_list_base = 0;
inline	uint64_t dependency_index_table_base = 0;


inline  std::vector<EFTPlayer> players;
inline EFTPlayer LocalPlayer;
inline	matrix pViewMatrix;