#include "pch.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <time.h>
#include <queue>

//using namespace std;
typedef unsigned char BYTE;

enum roomType
{
	eNone = 0,
	eBasic, //#
	eStart,
	eEnd,
	eTJunc, //_|_
	eCorner, //L
	eCorridor, //|
	eDeadEnd,
	eUnassigned
};

enum orientation
{
	eNorth = 0,
	eEast,
	eSouth,
	eWest
};

struct Room
{
	roomType type;
	orientation facing;

	Room() : type(eUnassigned), facing(eNorth) {}
	Room(roomType t, orientation f = eNorth) : type(t), facing(f) {}
};

orientation rotateDeadEnd(int doors[4])
{
	orientation facing;
	if (doors[0] == 1)
		facing = eNorth;
	else if (doors[1] == 1)
		facing = eEast;
	else if (doors[2] == 1)
		facing = eSouth;
	else
		facing = eWest;

	return facing;
}

orientation rotateCorner(int doors[4])
{
	orientation facing;
	if (doors[0] == 1 && doors[1] == 1)
		facing = eNorth;
	else if (doors[1] == 1 && doors[2] == 1)
		facing = eEast;
	else if (doors[2] == 1 && doors[3] == 1)
		facing = eSouth;
	else
		facing = eWest;

	return facing;
}

orientation rotateT(int doors[4])
{
	orientation facing;
	if (doors[0] == 0)
		facing = eSouth;
	else if (doors[1] == 0)
		facing = eWest;
	else if (doors[2] == 0)
		facing = eNorth;
	else
		facing = eEast;

	return facing;
}

bool roomInBounds(orientation dir, int location, int width)
{
	bool inBounds = false;
	switch (dir)
	{
	case eNorth:
		inBounds = (location - width) >= 0;
		break;
	case eEast:
		inBounds = (location + 1) % width != 0;
		break;
	case eSouth:
		inBounds = (location + width) < (width * width);
		break;
	case eWest:
		inBounds = (location - 1) >= 0 && (location - 1) % width != (width - 1);
		break;
	}
	return inBounds;
}

Room getAcceptableRoom(int location, int width, const std::vector<Room> & map, int doorChance = 34)
{
	int doors[4]{ 0, 0, 0, 0 }; //NESW
	//North
	if (roomInBounds(eNorth, location, width))
	{
		Room north = map[location - width];
		switch (north.type)
		{
		case eBasic:
			doors[0] = 1;
			break;
		case eStart:
			doors[0] = 1;
			break;
		case eEnd:
			doors[0] = 1;
			break;
		case eTJunc:
			if (north.facing != eNorth)
				doors[0] = 1;
			break;
		case eCorner:
			if (north.facing != eNorth && north.facing != eWest)
				doors[0] = 1;
			break;
		case eCorridor:
			if (north.facing != eEast && north.facing != eWest)
				doors[0] = 1;
			break;
		case eDeadEnd:
			if (north.facing == eSouth)
				doors[0] = 1;
			break;
		case eUnassigned:
			doors[0] = 2;
			break;
		}
	}
	//East
	if (roomInBounds(eEast, location, width))
	{
		Room east = map[location + 1];
		switch (east.type)
		{
		case eBasic:
			doors[1] = 1;
			break;
		case eStart:
			doors[1] = 1;
			break;
		case eEnd:
			doors[1] = 1;
			break;
		case eTJunc:
			if (east.facing != eEast)
				doors[1] = 1;
			break;
		case eCorner:
			if (east.facing != eEast && east.facing != eNorth)
				doors[1] = 1;
			break;
		case eCorridor:
			if (east.facing != eNorth && east.facing != eSouth)
				doors[1] = 1;
			break;
		case eDeadEnd:
			if (east.facing == eWest)
				doors[1] = 1;
			break;
		case eUnassigned:
			doors[1] = 2;
			break;
		}
	}
	//South
	if (roomInBounds(eSouth, location, width))
	{
		Room south = map[location + width];
		switch (south.type)
		{
		case eBasic:
			doors[2] = 1;
			break;
		case eStart:
			doors[2] = 1;
			break;
		case eEnd:
			doors[2] = 1;
			break;
		case eTJunc:
			if (south.facing != eSouth)
				doors[2] = 1;
			break;
		case eCorner:
			if (south.facing != eSouth && south.facing != eEast)
				doors[2] = 1;
			break;
		case eCorridor:
			if (south.facing != eEast && south.facing != eWest)
				doors[2] = 1;
			break;
		case eDeadEnd:
			if (south.facing == eNorth)
				doors[2] = 1;
			break;
		case eUnassigned:
			doors[2] = 2;
			break;
		}
	}
	//West
	if (roomInBounds(eWest, location, width))
	{
		Room west = map[location - 1];
		switch (west.type)
		{
		case eBasic:
			doors[3] = 1;
			break;
		case eStart:
			doors[3] = 1;
			break;
		case eEnd:
			doors[3] = 1;
			break;
		case eTJunc:
			if (west.facing != eWest)
				doors[3] = 1;
			break;
		case eCorner:
			if (west.facing != eSouth && west.facing != eWest)
				doors[3] = 1;
			break;
		case eCorridor:
			if (west.facing != eNorth && west.facing != eSouth)
				doors[3] = 1;
			break;
		case eDeadEnd:
			if (west.facing == eEast)
				doors[3] = 1;
			break;
		case eUnassigned:
			doors[3] = 2;
			break;
		}
	}
	int adjacentRooms = 0;
	//Randomise the empty adjacency and count
	for (int i = 0; i < 4; i++)
	{
		if (doors[i] == 2)
		{
			if (rand() % 100 < doorChance) //Insert random element
				doors[i] = 1;
			else
				doors[i] = 0;
		}
		if (doors[i] == 1)
			adjacentRooms++;
	}
	//Pick a room
	Room returnRoom = Room(eNone);
	switch (adjacentRooms)
	{
	case 0:
		break;
	case 1:
		returnRoom = Room(eDeadEnd, rotateDeadEnd(doors));
		break;
	case 2:
		if (doors[0] == 1 && doors[2] == 1)
			returnRoom = Room(eCorridor, eNorth);
		else if (doors[1] == 1 && doors[3] == 1)
			returnRoom = Room(eCorridor, eEast);
		else
			returnRoom = Room(eCorner, rotateCorner(doors));
		break;
	case 3:
		returnRoom = Room(eTJunc, rotateT(doors));
		break;
	case 4:
		returnRoom.type = eBasic;
		break;
	}

	return returnRoom;
}

orientation endDirection(int currentLoc, int endLoc, int mapSize)
{
	orientation returnDir;
	int dir[2]{ 0, 0 }; //X, Y
	dir[0] = (endLoc % mapSize) - (currentLoc % mapSize);
	dir[1] = (endLoc / mapSize) - (currentLoc / mapSize);

	if (abs(dir[0]) > abs(dir[1])) //X is biggest
	{
		if (dir[0] >= 0)
			returnDir = eEast;
		else
			returnDir = eWest;
	}
	else //Y is biggest
	{
		if (dir[1] >= 0)
			returnDir = eSouth;
		else
			returnDir = eNorth;
	}
	return returnDir;
}

void generatePath(int start, int end, int width, std::vector<Room> & map)
{
	int currentLoc = start;
	while (currentLoc != end)
	{
		if (map[currentLoc].type == eUnassigned)
			map[currentLoc] = Room(eBasic);

		int options[4]{ 1, 1, 1, 1 };

		if (!roomInBounds(eNorth, currentLoc, width))
			options[0] = 0;
		if (!roomInBounds(eEast, currentLoc, width))
			options[1] = 0;
		if (!roomInBounds(eSouth, currentLoc, width))
			options[2] = 0;
		if (!roomInBounds(eWest, currentLoc, width))
			options[3] = 0;
		//Find direction of end
		//Remove the option that points away from the end
		switch (endDirection(currentLoc, end, width))
		{
		case eNorth:
			options[2] = 0;
			options[0] = 2;
			break;
		case eEast:
			options[3] = 0;
			options[1] = 2;
			break;
		case eSouth:
			options[0] = 0;
			options[2] = 2;
			break;
		case eWest:
			options[1] = 0;
			options[3] = 2;
			break;
		}
		//Pick a random option from what's left and move in it
		int count = 1 + (rand() % 12);
		for (int i = 0; i < 100; i++)
		{
			if (options[i % 4] >= 1)
			{
				count--;
				if (count <= 0)
				{
					switch (i % 4)
					{
					case 0: //North
						currentLoc -= width;
						break;
					case 1: //East
						currentLoc++;
						break;
					case 2: //South
						currentLoc += width;
						break;
					case 3: //West
						currentLoc--;
						break;
					}
					break;
				}
			}
		}
	}
}

void basicRoomExplorer(Room currentRoom, int mapWidth, std::queue<int> & nextRoom, std::vector<bool> & explored)
{
	int location = nextRoom.front();
	nextRoom.pop();
	explored[location] = true;
	//Find available door directions
	bool doors[4]{ 0, 0, 0, 0 };
	switch (currentRoom.type)
	{
	case eBasic:
	case eStart:
	case eEnd:
		doors[0] = true;
		doors[1] = true;
		doors[2] = true;
		doors[3] = true;
		break;
	case eTJunc:
		switch (currentRoom.facing)
		{
		case eNorth:
			doors[0] = true;
			doors[1] = true;
			doors[3] = true;
			break;
		case eEast:
			doors[0] = true;
			doors[1] = true;
			doors[2] = true;
			break;
		case eSouth:
			doors[1] = true;
			doors[2] = true;
			doors[3] = true;
			break;
		case eWest:
			doors[0] = true;
			doors[2] = true;
			doors[3] = true;
			break;
		}
		break;
	case eCorner:
		switch (currentRoom.facing)
		{
		case eNorth:
			doors[0] = true;
			doors[1] = true;
			break;
		case eEast:
			doors[1] = true;
			doors[2] = true;
			break;
		case eSouth:
			doors[2] = true;
			doors[3] = true;
			break;
		case eWest:
			doors[0] = true;
			doors[3] = true;
			break;
		}
		break;
	case eCorridor:
		switch (currentRoom.facing)
		{
		case eNorth:
		case eSouth:
			doors[0] = true;
			doors[2] = true;
			break;
		case eEast:
		case eWest:
			doors[1] = true;
			doors[3] = true;
			break;
		}
		break;
	case eDeadEnd:
		switch (currentRoom.facing)
		{
		case eNorth:
			doors[0] = true;
			break;
		case eEast:
			doors[1] = true;
			break;
		case eSouth:
			doors[2] = true;
			break;
		case eWest:
			doors[3] = true;
			break;
		}
		break;
	case eNone:
	case eUnassigned:
		break;
	}
	//Set a task to explore each door
	if (doors[0] && !explored[location - mapWidth])
		nextRoom.push(location - mapWidth);

	if (doors[1] && !explored[location + 1])
		nextRoom.push(location + 1);

	if (doors[2] && !explored[location + mapWidth])
		nextRoom.push(location + mapWidth);

	if (doors[3] && !explored[location - 1])
		nextRoom.push(location - 1);
}

std::vector<bool> mapExplorer(int start, int mapWidth, const std::vector<Room> & map)
{
	std::vector<bool> explored(map.size(), false);
	std::queue<int> nextRoom;
	nextRoom.push(start);
	while (!nextRoom.empty())
	{
		basicRoomExplorer(map[nextRoom.front()], mapWidth, nextRoom, explored);
	}
	return explored;
}

int quadPos(int mapWidth)
{
	return (rand() % ((mapWidth / 2) - 2)) + 1;
}

//This will be a node
std::vector<int> generateRooms(int seed, int mapWidth, int doorChance)
{
	if (mapWidth < 8)
		mapWidth = 8;

	srand(seed);
	std::vector<Room> map(mapWidth * mapWidth, Room());

	//Place essential rooms (start and end)
	const int start = 2 * mapWidth + 2;

	int x = quadPos(mapWidth) + (mapWidth / 2);
	int y = quadPos(mapWidth) + (mapWidth / 2);
	const int end = x + y * mapWidth;

	map[start].type = eStart;
	map[end].type = eEnd;

	//Generate a random path to get there
	x = quadPos(mapWidth) + (mapWidth / 2);
	y = quadPos(mapWidth);
	const int quad2 = x + y * mapWidth;

	x = quadPos(mapWidth);
	y = quadPos(mapWidth) + (mapWidth / 2);
	const int quad3 = x + y * mapWidth;

	generatePath(start, end, mapWidth, map);
	for (int i = 0; i < map.size(); i++)
	{
		if (map[i].type == eBasic)
			map[i] = getAcceptableRoom(i, mapWidth, map, doorChance);
	}

	generatePath(start, quad2, mapWidth, map);
	generatePath(start, quad3, mapWidth, map);
	generatePath(quad2, end, mapWidth, map);
	generatePath(quad3, end, mapWidth, map);

	//Fill in path rooms
	for (int i = 0; i < map.size(); i++)
	{
		if (map[i].type == eBasic)
			map[i] = getAcceptableRoom(i, mapWidth, map, doorChance);
	}

	//Fill rest of rooms
	for (int i = 0; i < map.size(); i++)
	{

		if (map[i].type == eUnassigned)
		{
			map[i] = getAcceptableRoom(i, mapWidth, map, doorChance);
		}
	}

	//Cull inaccessible rooms
	std::vector<bool> roomsToKeep = mapExplorer(start, mapWidth, map);
	for (int i = 0; i < roomsToKeep.size(); i++)
	{
		if (!roomsToKeep[i])
			map[i] = Room(eNone);
	}

	//Convert to unreal array
	std::vector<int> output;
	output.resize(2 * map.size());
	for (int i = 0; i < map.size(); i++)
	{
		output[i * 2] = static_cast<int>(map[i].type);
		output[(i * 2) + 1] = static_cast<int>(map[i].facing);
	}
	return output;
}

struct MapNode
{
	//Address
	BYTE pos;

	//Adjacency
	BYTE adjacents; //Bitmask
	BYTE adj[4];

	//Costing
	BYTE baseCost;
	BYTE deathCost; //Will need resetting gradually
	BYTE barricades; //Bitmask

	MapNode(BYTE position) :
		pos(position), adjacents(0), baseCost(10), deathCost(0), barricades(0)
	{
		adj[0] = 0;
		adj[1] = 0;
		adj[2] = 0;
		adj[3] = 0;
	}
};

BYTE adjacentMasker(const roomType type, const orientation dir)
{
	//N, E, S, W
	//1, 2, 4, 8
	BYTE output = 0;
	switch (type)
	{
	case eBasic:
	case eStart:
	case eEnd:
		output = 1 + 2 + 4 + 8;
		break;

	case eTJunc:
		switch (dir)
		{
		case eNorth:
			output = 1 + 2 + 8;
			break;
		case eEast:
			output = 1 + 2 + 4;
			break;
		case eSouth:
			output = 2 + 4 + 8;
			break;
		case eWest:
			output = 1 + 4 + 8;
			break;
		}
		break;

	case eCorner:
		switch (dir)
		{
		case eNorth:
			output = 1 + 2;
			break;
		case eEast:
			output = 2 + 4;
			break;
		case eSouth:
			output = 4 + 8;
			break;
		case eWest:
			output = 1 + 8;
			break;
		}
		break;

	case eCorridor:
		switch (dir)
		{
		case eNorth:
		case eSouth:
			output = 1 + 4;
			break;
		case eEast:
		case eWest:
			output = 2 + 8;
			break;
		}
		break;

	case eDeadEnd:
		switch (dir)
		{
		case eNorth:
			output = 1;
			break;
		case eEast:
			output = 2;
			break;
		case eSouth:
			output = 4;
			break;
		case eWest:
			output = 8;
			break;
		}
		break;
	}
	return output;
}

void roomConnector(const int index, const int width, std::vector<MapNode> & nodes)
{
	const BYTE adjacents = nodes[index].adjacents;
	//North
	if ((adjacents & 1) != 0)
	{
		bool found = false;
		int adjAddress = nodes[index].pos - width;
		for (int i = 0; i < nodes.size(); i++)
		{
			if (nodes[i].pos == adjAddress)
			{
				nodes[index].adj[0] = i;
				found = true;
				break;
			}
		}
		if (!found)
		{
			nodes.emplace_back(MapNode((BYTE)adjAddress));
			nodes[index].adj[0] = nodes.size() - 1;
		}
	}
	//East
	if ((adjacents & 2) != 0)
	{
		bool found = false;
		int adjAddress = nodes[index].pos + 1;
		for (int i = 0; i < nodes.size(); i++)
		{
			if (nodes[i].pos == adjAddress)
			{
				nodes[index].adj[1] = i;
				found = true;
				break;
			}
		}
		if (!found)
		{
			nodes.emplace_back(MapNode((BYTE)adjAddress));
			nodes[index].adj[1] = nodes.size() - 1;
		}
	}
	//South
	if ((adjacents & 4) != 0)
	{
		bool found = false;
		int adjAddress = nodes[index].pos + width;
		for (int i = 0; i < nodes.size(); i++)
		{
			if (nodes[i].pos == adjAddress)
			{
				nodes[index].adj[2] = i;
				found = true;
				break;
			}
		}
		if (!found)
		{
			nodes.emplace_back(MapNode((BYTE)adjAddress));
			nodes[index].adj[2] = nodes.size() - 1;
		}
	}
	//West
	if ((adjacents & 8) != 0)
	{
		bool found = false;
		int adjAddress = nodes[index].pos - 1;
		for (int i = 0; i < nodes.size(); i++)
		{
			if (nodes[i].pos == adjAddress)
			{
				nodes[index].adj[3] = i;
				found = true;
				break;
			}
		}
		if (!found)
		{
			nodes.emplace_back(MapNode((BYTE)adjAddress));
			nodes[index].adj[3] = nodes.size() - 1;
		}
	}
}

//This is also a node
std::vector<int> generateNodes(const std::vector<int> & rooms)
{
	const int mapWidth = std::sqrt(rooms.size() / 2);
	std::vector<MapNode> nodeNetwork;
	nodeNetwork.reserve(256);
	//Find the start room
	for (int i = 0; i < rooms.size(); i += 2)
	{
		if (rooms[i] == eStart)
		{
			nodeNetwork.emplace_back(i / 2);
			break;
		}
	}
	//Add all adjacent rooms to each room in the list iteratively
	for (int currentNode = 0; currentNode < 256; currentNode++)
	{
		if (currentNode >= nodeNetwork.size())
			break;

		roomType type = (roomType)rooms[nodeNetwork[currentNode].pos * 2];
		orientation direction = (orientation)rooms[nodeNetwork[currentNode].pos * 2 + 1];

		nodeNetwork[currentNode].adjacents = adjacentMasker(type, direction);
		roomConnector(currentNode, mapWidth, nodeNetwork);
	}
	//Convert into TArray
	std::vector<int> output;
	output.resize(nodeNetwork.size() * 9);
	for (int node = 0; node < nodeNetwork.size(); node++)
	{
		int it = node * 9;
		output[it] = nodeNetwork[node].pos;
		output[it + 1] = nodeNetwork[node].adjacents;
		output[it + 2] = nodeNetwork[node].adj[0];
		output[it + 3] = nodeNetwork[node].adj[1];
		output[it + 4] = nodeNetwork[node].adj[2];
		output[it + 5] = nodeNetwork[node].adj[3];
		output[it + 6] = nodeNetwork[node].baseCost;
		output[it + 7] = nodeNetwork[node].deathCost;
		output[it + 8] = nodeNetwork[node].barricades;
	}
	return output;
}

int main()
{
	std::vector<int> rooms = generateRooms(0, 8, 34);
	std::vector<int> nodes = generateNodes(rooms);
	for (int i = 0; i < nodes.size(); i++)
	{
		if (i % 9 == 0)
			std::cout << "\n";
		std::cout << nodes[i] << ", ";
	}
	return 0;
}