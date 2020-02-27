#include "hlapi/hlapi.h"
#include "globals.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <random>
#include <chrono>
#include <unistd.h> //getpid
#include <thread>
#include <atomic>
#include <csignal>
#include <numeric>
#include <thread>
#include <chrono>
#include <iostream>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include "server_shared.h"
#include "eftstructs.h"

auto gameData = EFTData::Instance();
int connection;
struct sockaddr_in myaddr, remaddr;
int i, slen=sizeof(remaddr);
char *server = "192.168.122.49";	/* change this to use a different server */

float Vector3DotProduct(Vector3 vectorLeft, Vector3 vectorRight)
{
    return vectorLeft.x * vectorRight.x + vectorLeft.y * vectorRight.y + vectorLeft.z * vectorRight.z;
}

bool WorldToScreenv2(const FVector& point3D, Vector2& point2D)
{
	Vector3 _point3D = Vector3(point3D.x, point3D.z, point3D.y);

	auto& matrix = viewMatrix;

	
	if (EFTData::Instance()->IsAiming(localPlayer.instance) && EFTData::Instance()->get_mpcamera(localPlayer.instance))
	{
    	 matrix = EFTData::Instance()->getoptic_matrix(localPlayer.instance);
	}
	
/*
D3DXMATRIX( FLOAT _11, FLOAT _12, FLOAT _13, FLOAT _14,
            FLOAT _21, FLOAT _22, FLOAT _23, FLOAT _24,
            FLOAT _31, FLOAT _32, FLOAT _33, FLOAT _34,
            FLOAT _41, FLOAT _42, FLOAT _43, FLOAT _44 );
*/

	Vector3 translationVector = Vector3(matrix.m[3][0], matrix.m[3][1], matrix.m[3][2]);
	Vector3 up = Vector3(matrix.m[1][0], matrix.m[1][1], matrix.m[1][2]);
	Vector3 right = Vector3(matrix.m[0][0], matrix.m[0][1], matrix.m[0][2]);

	float w = Vector3DotProduct(translationVector, _point3D) + matrix.m[3][3];

	if (w < 0.098f)
		return false;

	float y = Vector3DotProduct(up, _point3D) + matrix.m[1][3];
	float x = Vector3DotProduct(right, _point3D) + matrix.m[0][3];

	if (EFTData::Instance()->IsAiming(localPlayer.instance) && EFTData::Instance()->get_mpcamera(localPlayer.instance))
	{
		uint64_t chain = process->ReadChain(fpsCamera, { 0x30, 0x18 });
			x /= process->Read<float>(chain + 0x12C);
			if(x < 2.f)
			x /= process->Read<float>(chain + 0xAC);
			y /= process->Read<float>(chain + 0x118);
			if(y < 2.f)
			y /= process->Read<float>(chain + 0x98);
	}

	point2D.x = (2560 / 2) * (1.f + x / w);
	point2D.y = (1440 / 2) * (1.f - y / w);

	return true;
}

float MaxDrawDistance = 300.f;

void Run()
{
	while(true) {
		if (!gameData->Read())
			gameData->InitOffsets();
		
		float distance;
		//float distance2;
	
		Packet response = Packet();
		//response.header.type = PacketType::packet_player;
		response.header.type = PacketType::packet_objects;
		int i = 0;

		for (auto& player : players)
		{
			if (!player.instance)
				continue;

			if (player.instance == localPlayer.instance)
				continue;

			distance = localPlayer.location.Distance(player.location);

			if (distance > MaxDrawDistance)
				continue;

			Vector2 location_screen_pos;

			WorldToScreenv2(player.location, location_screen_pos);

			if (location_screen_pos.x <= 1 || location_screen_pos.x >= 2560 || location_screen_pos.y <= 1 || location_screen_pos.y >= 1440)
				continue;

			Vector2 pBase, pPelvis, pLThigh1, pLThigh2, pLCalf, pLFoot, pLToe, pRThigh1, pRThigh2, pRCalf, pRFoot, pRToe, pSpine1, pSpine2, pSpine3, pLCollarbone, pLUpperarm, pLForearm1, pLForearm2, pLForearm3, pLPalm, pRCollarbone, pRUpperarm, pRForearm1, pRForearm2, pRForearm3, pRPalm, pNeck, pHead; 
			
			WorldToScreenv2(player.Base, pBase);
			WorldToScreenv2(player.Pelvis, pPelvis);
			WorldToScreenv2(player.LThigh1, pLThigh1);
			WorldToScreenv2(player.LThigh2, pLThigh2);
			WorldToScreenv2(player.LCalf, pLCalf);
			WorldToScreenv2(player.LFoot, pLFoot);
			WorldToScreenv2(player.LToe, pLToe);
			WorldToScreenv2(player.RThigh1, pRThigh1);
			WorldToScreenv2(player.RThigh2, pRThigh2);
			WorldToScreenv2(player.RCalf, pRCalf);
			WorldToScreenv2(player.RFoot, pRFoot);
			WorldToScreenv2(player.RToe, pRToe);
			WorldToScreenv2(player.Spine1, pSpine1);
			WorldToScreenv2(player.Spine2, pSpine2);
			WorldToScreenv2(player.Spine3, pSpine3);
			WorldToScreenv2(player.LCollarbone, pLCollarbone);
			WorldToScreenv2(player.LUpperarm, pLUpperarm);
			WorldToScreenv2(player.LForearm1, pLForearm1);
			WorldToScreenv2(player.LForearm2, pLForearm2);
			WorldToScreenv2(player.LForearm3, pLForearm3);
			WorldToScreenv2(player.LPalm, pLPalm);
			WorldToScreenv2(player.RCollarbone, pRCollarbone);
			WorldToScreenv2(player.RUpperarm, pRUpperarm);
			WorldToScreenv2(player.RForearm1, pRForearm1);
			WorldToScreenv2(player.RForearm2, pRForearm2);
			WorldToScreenv2(player.RForearm3, pRForearm3);
			WorldToScreenv2(player.RPalm, pRPalm);
			WorldToScreenv2(player.Neck, pNeck);
			WorldToScreenv2(player.Head, pHead);

			PacketPlayer pPlayer;

			pPlayer.Base = pBase;
			pPlayer.Pelvis = pPelvis;
			pPlayer.LThigh1 = pLThigh1;
			pPlayer.LThigh2 = pLThigh2;
			pPlayer.LCalf = pLCalf;
			pPlayer.LFoot = pLFoot;
			pPlayer.LToe = pLToe;
			pPlayer.RThigh1 = pRThigh1;
			pPlayer.RThigh2 = pRThigh2;
			pPlayer.RCalf = pRCalf;
			pPlayer.RFoot = pRFoot;
			pPlayer.RToe = pRToe;
			pPlayer.Spine1 = pSpine1;
			pPlayer.Spine2 = pSpine2;
			pPlayer.Spine3 = pSpine3;
			pPlayer.LCollarbone = pLCollarbone;
			pPlayer.LUpperarm = pLUpperarm;
			pPlayer.LForearm1 = pLForearm1;
			pPlayer.LForearm2 = pLForearm2;
			pPlayer.LForearm3 = pLForearm3;
			pPlayer.LPalm = pLPalm;
			pPlayer.RCollarbone = pRCollarbone;
			pPlayer.RUpperarm = pRUpperarm;
			pPlayer.RForearm1 = pRForearm1;
			pPlayer.RForearm2 = pRForearm2;
			pPlayer.RForearm3 = pRForearm3;
			pPlayer.RPalm = pRPalm;
			pPlayer.Neck = pNeck;
			pPlayer.Head = pHead;

			pPlayer.location = location_screen_pos;
			pPlayer.index = i;
			pPlayer.total = players.size();
			pPlayer.distance = distance;
			pPlayer.isPlayer = EFTData::Instance()->IsPlayer(player.instance);

			// response.player = pPlayer;
			// sendto(connection, &response, sizeof(Packet), 0, (struct sockaddr *)&remaddr, slen);
			
			response.objects.players[i] = pPlayer;

			i++;
		}

		sendto(connection, &response, sizeof(Packet), 0, (struct sockaddr *)&remaddr, slen);
	}
}

#define PORT 24925
#define BUFLEN 2048
#define MSGS 5	/* number of messages to send */

void Connect()
{
	// Create a UDP socket
	// SOCK_STREAM for TCP / SOCK_DGRAM for UDP
	if ((connection=socket(AF_INET, SOCK_DGRAM, 0))==-1)
		printf("socket created\n");

	/* bind it to all local addresses and pick any port number */

	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(0);

	if (bind(connection, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
	}

	/* now define remaddr, the address to whom we want to send messages */
	/* For convenience, the host address is expressed as a numeric IP address */
	/* that we will convert to a binary format via inet_aton */

	memset((char *) &remaddr, 0, sizeof(remaddr));
	remaddr.sin_family = AF_INET;
	remaddr.sin_port = htons(PORT);
	if (inet_aton(server, &remaddr.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}

	/* now let's send the messages */
}

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

FILE* dfile;

__attribute__((constructor))
static void init()
{
	FILE* out = stdout;
	pid_t pid;
#if (LMODE() == MODE_EXTERNAL())
	FILE* pipe = popen("pidof qemu-system-x86_64", "r");
	fscanf(pipe, "%d", &pid);
	pclose(pipe);
#else
	out = fopen("/tmp/testr.txt", "w");
	pid = getpid();
#endif
	fprintf(out, "Using Mode: %s\n", TOSTRING(LMODE));
	fprintf(out, "Using KMOD_MEMMAP: %s\n", TOSTRING(KMOD_MEMMAP));

	dfile = out;

	Connect();

	try {
		WinContext ctx(pid);
		ctx.processList.Refresh();

		for (auto& i : ctx.processList) {
			if (!strcasecmp("EscapeFromTark", i.proc.name) || !strcasecmp("FortniteClient", i.proc.name)) {
				for (auto& o : i.modules) {
					//printf("Module: %s\n", o.info.name);
					if (!strcmp("UnityPlayer.dll", o.info.name) || !strcmp("FortniteClient-Win64-Shipping.exe", o.info.name)) {
						printf("Found %i\t%s\n", i.proc.pid, i.proc.name);
						process = &i;
						printf("Found name %s with base 0x%X and entry 0x%X and size 0x%X\n", o.info.name, o.info.baseAddress, o.info.entryPoint, o.info.sizeOfModule);
						unityPlayerBase = o.info.baseAddress;

						Run();
					}
				}
			}
		}
	} catch (VMException& e) {
		fprintf(out, "Initialization error: %d\n", e.value);
	}

	fclose(out);
}

int main()
{
	return 0;
}
