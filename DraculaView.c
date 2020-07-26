////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// DraculaView.c: the DraculaView ADT implementation
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "DraculaView.h"
#include "Game.h"
#include "GameView.h"
#include "Map.h"
// add your own #includes here
#include <string.h>
// TODO: ADD YOUR OWN STRUCTS HERE

#define MAXCHAR 8
#define MAXLINE 40

struct draculaView {
	// TODO: ADD FIELDS HERE
	GameView gv;
};

bool canGo(DraculaView dv, PlaceId place);
int getDoubleBackNum(DraculaView dv, PlaceId place);
bool canDoubleBack(DraculaView dv);
bool canHide(DraculaView dv);
//bool UniqueLocation(PlaceId* list, PlaceId place, int num);
//PlaceId* MapGetLocation(Map m, ConnList connection, PlaceId p, PlaceId* pastLocation, int pastNum, int num, int* count);
////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

DraculaView DvNew(char* pastPlays, Message messages[])
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	DraculaView new_dv = malloc(sizeof(DraculaView));
	if (new_dv == NULL) {
		fprintf(stderr, "Couldn't allocate DraculaView\n");
		exit(EXIT_FAILURE);
	}

	new_dv->gv = GvNew(pastPlays, messages);

	return new_dv;
}

void DvFree(DraculaView dv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	GvFree(dv->gv);
	free(dv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round DvGetRound(DraculaView dv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return GvGetRound(dv->gv);
}

int DvGetScore(DraculaView dv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return GvGetScore(dv->gv);
}

int DvGetHealth(DraculaView dv, Player player)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return GvGetHealth(dv->gv, player);
}

PlaceId DvGetPlayerLocation(DraculaView dv, Player player)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return GvGetPlayerLocation(dv->gv, player);
}

PlaceId DvGetVampireLocation(DraculaView dv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return GvGetVampireLocation(dv->gv);
}

PlaceId* DvGetTrapLocations(DraculaView dv, int* numTraps)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numTraps = 0;
	return GvGetTrapLocations(dv->gv, numTraps);
}

////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId* DvGetValidMoves(DraculaView dv, int* numReturnedMoves)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedMoves = 0;

	PlaceId lastLocation = GvGetPlayerLocation(dv->gv, PLAYER_DRACULA);
	if (lastLocation == NOWHERE) 
		return NULL;

	int DBnumber = 0;
	Map m = MapNew();
	ConnList connection = MapGetConnections(m, lastLocation);
	ConnList curr = connection;
	PlaceId* move = NULL;
	while (curr != NULL) {
		if (curr->type == ROAD || curr->type == BOAT) {
			if (canGo(dv, curr->p)) {
				move = realloc(move, (*numReturnedMoves + 1) * sizeof(PlaceId));
				move[*numReturnedMoves] = curr->p;
				*numReturnedMoves = *numReturnedMoves + 1;
			}
			else if ((DBnumber = getDoubleBackNum(dv, curr->p)) > 0) {
				move = realloc(move, (*numReturnedMoves + 1) * sizeof(PlaceId));
				move[*numReturnedMoves] = DOUBLE_BACK_1 + DBnumber - 1;
				*numReturnedMoves = *numReturnedMoves + 1;
			}
		}
		curr = curr->next;
	}

	//Same with Hide, Map cannot get the current location, cannot do double back
	if (canDoubleBack(dv)) {
		move = realloc(move, (*numReturnedMoves + 1) * sizeof(PlaceId));
		move[*numReturnedMoves] = DOUBLE_BACK_1;
		*numReturnedMoves = *numReturnedMoves + 1;
	}
	if (canHide(dv)) {
		move = realloc(move, (*numReturnedMoves + 1) * sizeof(PlaceId));
		move[*numReturnedMoves] = HIDE;
		*numReturnedMoves = *numReturnedMoves + 1;
	}

	//TP

	/*
	for (int j = 0; j < *numReturnedMoves; j++) {
		printf("%s ", placeIdToName(move[j]));
	}
	*/
	return move;
}

PlaceId* DvWhereCanIGo(DraculaView dv, int* numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return DvWhereCanIGoByType(dv, true, true, numReturnedLocs);
}

PlaceId* DvWhereCanIGoByType(DraculaView dv, bool road, bool boat,
	int* numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	PlaceId lastLocation = GvGetPlayerLocation(dv->gv, PLAYER_DRACULA);
	if (lastLocation == NOWHERE) 
		return NULL;

	Map m = MapNew();
	ConnList connection = MapGetConnections(m, lastLocation);
	ConnList curr = connection;
	PlaceId* place = NULL;
	while (curr != NULL) {
		if ((curr->type == ROAD && road) || (curr->type == BOAT && boat)) {
			if (canGo(dv, curr->p)) {
				place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
				place[*numReturnedLocs] = curr->p;
				*numReturnedLocs = *numReturnedLocs + 1;
			}
			else if (getDoubleBackNum(dv, curr->p) > 0) {
				place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
				place[*numReturnedLocs] = curr->p;
				*numReturnedLocs = *numReturnedLocs + 1;
			}
		}
		curr = curr->next;
	}
	if (canHide(dv)) {
		place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
		place[*numReturnedLocs] = lastLocation;
		*numReturnedLocs = *numReturnedLocs + 1;
	}
	return place;
}

PlaceId* DvWhereCanTheyGo(DraculaView dv, Player player,
	int* numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return DvWhereCanTheyGoByType(dv, player, true, true, true, numReturnedLocs);
}

PlaceId* DvWhereCanTheyGoByType(DraculaView dv, Player player,
	bool road, bool rail, bool boat,
	int* numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	if (player == PLAYER_DRACULA)
		return DvWhereCanIGoByType(dv, road, boat, numReturnedLocs);

	PlaceId lastLocation = GvGetPlayerLocation(dv->gv, player);
	if (lastLocation == NOWHERE) return NULL;

	Player currPlayer = GvGetPlayer(dv->gv);
	
	if (currPlayer < player)
		return GvGetReachableByType(dv->gv, player, GvGetRound(dv->gv), lastLocation, road, rail, boat, numReturnedLocs);
	else
		return GvGetReachableByType(dv->gv, player, GvGetRound(dv->gv) + 1, lastLocation, road, rail, boat, numReturnedLocs);
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// TODO

bool canGo(DraculaView dv, PlaceId place) {

	//Begin the game
	if (place == NOWHERE) return true;

	//Cannot in hospital
	if (place == HOSPITAL_PLACE) return false;

	int num = -1;
	bool canFree = false;
	PlaceId* list = GvGetLocationHistory(dv->gv, PLAYER_DRACULA, &num, &canFree);

	//Cannot be the same location
	for (int i = 0; i < num && i < 6; i++) {

		if (list[i] == place)
			return false;

		if (list[i] >= DOUBLE_BACK_1 && list[i] <= DOUBLE_BACK_5 && list[i] - DOUBLE_BACK_1 < num) {
			if (list[list[i] - DOUBLE_BACK_1] == place)
				return false;
		}

		if (i + 1 == 6 && num >= 6) {
			if (list[i] == HIDE && list[i + 1] == place)
				return false;
		}
	}

	return true;
}

int getDoubleBackNum(DraculaView dv, PlaceId place) {

	int num = -1;
	bool canFree = false;
	PlaceId* list = GvGetLocationHistory(dv->gv, PLAYER_DRACULA, &num, &canFree);

	if (!canDoubleBack(dv)) 
		return 0;

	for (int i = 0; i < num && i < 6; i++) {
		if (list[i] == place) return i + 1;
	}

	return 0;
}

bool canDoubleBack(DraculaView dv) {
	
	int num = -1;
	bool canFree = false;
	PlaceId* list = GvGetMoveHistory(dv->gv, PLAYER_DRACULA, &num, &canFree);

	for (int i = 0; i < num && i < 6; i++) {
		if (list[i] >= DOUBLE_BACK_1 && list[i] <= DOUBLE_BACK_5)
			return false;
	}
	return true;
}

bool canHide(DraculaView dv) {

	int num = -1;
	bool canFree = false;
	PlaceId* list = GvGetMoveHistory(dv->gv, PLAYER_DRACULA, &num, &canFree);

	for (int i = 0; i < num && i < 6; i++) {
		if (list[i] == HIDE)
			return false;
	}
	return true;
}
/*
bool UniqueLocation(PlaceId* list, PlaceId place, int num)
{
	if (list == NULL) return true;
	if (num == 0) return true;
	for (int i = 0; i < num; i++) {
		if (list[i] == place)
			return false;
	}
	return true;
}

PlaceId* MapGetLocation(Map m, ConnList connection, PlaceId p, PlaceId* pastLocation, int pastNum, int num, int* count)
{
	PlaceId* place = NULL;
	PlaceId* temp = NULL;

	*count = 0;
	int localcount = 0;
	ConnList curr = MapGetConnections(m, p);
	//printf("Num of Rail Can Move: %d, in : %s\n", num, placeIdToName(p));
	while (curr != NULL) {
		//printf("   Rail curr is : %s, The type is : %s\n", placeIdToName(curr->p), transportTypeToString(curr->type));
		if (curr->type == RAIL && num > 0 && curr->p != p && UniqueLocation(pastLocation, curr->p, pastNum)) {
			//printf("<Go> next Location :【 %s 】\n", placeIdToName(curr->p));
			temp = MapGetLocation(m, curr, curr->p, pastLocation, pastNum, num - 1, &localcount);

			//need to unique and combine together
			place = realloc(place, (*count + localcount) * sizeof(PlaceId));
			int total = *count + localcount;
			for (int j = *count; j < total; j++) {
				if (UniqueLocation(place, temp[j - *count], j)) {
					place[j] = temp[j - *count];
					//printf("<-Finall Push->Add Location: %s\n", placeIdToName(place[j]));
				}
				else {
					j = j - 1;
					*count = *count - 1;
				}
				
			}
			*count = *count + localcount;
		}
		else if (curr->type == RAIL && curr->p != p && UniqueLocation(pastLocation, curr->p, pastNum)) {
			place = realloc(place, (*count + 1) * sizeof(PlaceId));
			place[*count] = curr->p;
			//printf("<-Push-> %s\n", placeIdToName(curr->p));
			*count = *count + 1;
		}
		curr = curr->next;
	}
	
	//for (int i = 0; i < *count; i++) {
	//	printf("<-Check Push-> %s\n", placeIdToName(place[i]));
	//}

	return place;
}
*/