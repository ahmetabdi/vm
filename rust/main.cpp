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
    return ((vectorLeft.x * vectorRight.x) + (vectorLeft.y * vectorRight.y) + (vectorLeft.z * vectorRight.z));
}

bool WorldToScreenv2(const FVector& point3D, Vector2& point2D)
{
	Vector3 _point3D = Vector3(point3D.x, point3D.z, point3D.y);

	auto& matrix = pViewMatrix;

	/*
	printf("\n\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n%f %f %f %f\n", 
		matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3],
		matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3],
		matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3],
		matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3]);
	*/

	Vector3 translationVector = Vector3(matrix.m[3][0], matrix.m[3][1], matrix.m[3][2]);
	Vector3 up = Vector3(matrix.m[1][0], matrix.m[1][1], matrix.m[1][2]);
	Vector3 right = Vector3(matrix.m[0][0], matrix.m[0][1], matrix.m[0][2]);

	float w = Vector3DotProduct(translationVector, _point3D) + matrix.m[3][3]; // ROLL

	if (w < 0.098f)
		return false;

	float y = Vector3DotProduct(up, _point3D) + matrix.m[1][3]; //PITCH
	float x = Vector3DotProduct(right, _point3D) + matrix.m[0][3]; //YAW

	point2D.x = (2560 / 2) * (1.f + x / w);
	point2D.y = (1440 / 2) * (1.f - y / w);

	return true;
}

float MaxDrawDistance = 100.f;
float distance;

void Run()
{
	while(true) {
		printf("loop\n");
		//if (!gameData->Read())
		gameData->InitOffsets();
		printf("end");
		Packet response = Packet();
		response.header.type = PacketType::packet_objects;
		int i = 0;

		for (auto& player : players)
		{
			printf("%i\n", i);
			if (i > 19)
				continue;

			distance = LocalPlayer.location.Distance(player.location);

			if (distance > MaxDrawDistance)
				continue;

			Vector2 location_screen_pos;

			WorldToScreenv2(player.location, location_screen_pos);

			PacketPlayer pPlayer;
			pPlayer.location = location_screen_pos;
			response.objects.players[i] = pPlayer;

			i++;
		}

		sendto(connection, &response, sizeof(Packet), 0, (struct sockaddr *)&remaddr, slen);

		
		//usleep(5);
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
			//printf("Process: %s\n", i.proc.name);
			if (!strcasecmp("RustClient.exe", i.proc.name)) {
				for (auto& o : i.modules) {
					//printf("Module: %s\n", o.info.name);
					if (!strcmp("UnityPlayer.dll", o.info.name)) {
						printf("Found %i\t%s\n", i.proc.pid, i.proc.name);
						process = &i;
						printf("Found name %s with base 0x%X and entry 0x%X and size 0x%X\n", o.info.name, o.info.baseAddress, o.info.entryPoint, o.info.sizeOfModule);
						unityPlayerBase = o.info.baseAddress;

						Run();

						printf("This should never get called\n");
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
