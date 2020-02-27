#pragma once
#include "stdint.h"
#include <vector>

constexpr auto server_ip = 0xC0A87A01; // 192.168.122.1
constexpr auto server_port = 27925;

enum PacketType : uint16_t
{
	packet_player = 0,
	packet_objects = 1,
	packet_get_peb = 2,
	packet_clear = 3,
};

struct PacketPlayer
{
	int index;
	int total;
	Vector2 location;
	float distance;
	bool isPlayer;

	Vector2 Base;
	Vector2 Pelvis;
	Vector2 LThigh1;
	Vector2 LThigh2;
	Vector2 LCalf;
	Vector2 LFoot;
	Vector2 LToe;
	Vector2 RThigh1;
	Vector2 RThigh2;
	Vector2 RCalf;
	Vector2 RFoot;
	Vector2 RToe;
	Vector2 Spine1;
	Vector2 Spine2;
	Vector2 Spine3;
	Vector2 LCollarbone;
	Vector2 LUpperarm;
	Vector2 LForearm1;
	Vector2 LForearm2;
	Vector2 LForearm3;
	Vector2 LPalm;
	Vector2 RCollarbone;
	Vector2 RUpperarm;
	Vector2 RForearm1;
	Vector2 RForearm2;
	Vector2 RForearm3;
	Vector2 RPalm;
	Vector2 Neck;
	Vector2 Head;
};

struct PacketObjects
{
	PacketPlayer players[20];
};

struct PacketGetBaseAddress
{
	uint32_t process_id;
};

struct PacketGetBasePeb
{
	uint32_t process_id;
};

struct PackedCompleted
{
	uint64_t result;
};

struct PacketHeader
{
	PacketType type;
};

struct Packet
{
	PacketHeader header;
	PacketPlayer player;
	PacketObjects objects;
	/*
	union
	{
		PacketPlayer	 player;
		PacketGetBaseAddress get_base_address;
		PacketGetBasePeb get_base_peb;
		PackedCompleted		 completed;
	} data;
	*/
};
