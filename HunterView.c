////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// HunterView.c: the HunterView ADT implementation
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10   v3.0    Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "Game.h"
#include "GameView.h"
#include "HunterView.h"
#include "Map.h"
#include "Places.h"
// add your own #includes here
#include <string.h>

#define MAXCHAR 8
#define MAXLINE 40
// TODO: ADD YOUR OWN STRUCTS HERE

struct hunterView {
	// TODO: ADD FIELDS HERE
	GameView gv;

};


bool canGo(HunterView hv, PlaceId place);
int getDoubleBackNum(HunterView hv, PlaceId place);
bool canDoubleBack(HunterView hv);
bool canHide(HunterView hv);

////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

HunterView HvNew(char *pastPlays, Message messages[])
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	HunterView new = malloc(sizeof(*new));
	if (new == NULL) {
		fprintf(stderr, "Couldn't allocate HunterView!\n");
		exit(EXIT_FAILURE);
	}

	new->gv = GvNew(pastPlays, messages);

	return new;
}

void HvFree(HunterView hv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	GvFree(hv->gv);
	free(hv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round HvGetRound(HunterView hv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return GvGetRound(hv->gv);
}

Player HvGetPlayer(HunterView hv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return GvGetPlayer(hv->gv);
}

int HvGetScore(HunterView hv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return GvGetScore(hv->gv);
}

int HvGetHealth(HunterView hv, Player player)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return GvGetHealth(hv->gv, player);
}

PlaceId HvGetPlayerLocation(HunterView hv, Player player)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return GvGetPlayerLocation(hv->gv, player);
}

PlaceId HvGetVampireLocation(HunterView hv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return GvGetVampireLocation(hv->gv);
}

////////////////////////////////////////////////////////////////////////
// Utility Functions

PlaceId HvGetLastKnownDraculaLocation(HunterView hv, Round *round)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*round = 0;
	return NOWHERE;
}

PlaceId *HvGetShortestPathTo(HunterView hv, Player hunter, PlaceId dest,
                             int *pathLength)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*pathLength = 0;
	return NULL;
}

////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId *HvWhereCanIGo(HunterView hv, int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return HvWhereCanIGoByType(hv, true, true, true, numReturnedLocs);
}

PlaceId *HvWhereCanIGoByType(HunterView hv, bool road, bool rail,
                             bool boat, int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	PlaceId * place = NULL;
	*numReturnedLocs = 0;
	Player player = GvGetPlayer(hv->gv);
	PlaceId from = GvGetPlayerLocation(hv->gv, player);
	if (from == NOWHERE) return NULL;
	place = GvGetReachableByType(hv->gv, player, GvGetRound(hv->gv), from, road, rail, boat, numReturnedLocs);
	return place;
}

PlaceId *HvWhereCanTheyGo(HunterView hv, Player player,
                          int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return HvWhereCanTheyGoByType(hv, player, true, true, true, numReturnedLocs);
}

PlaceId *HvWhereCanTheyGoByType(HunterView hv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	if (player != PLAYER_DRACULA) {
		PlaceId * place = NULL;
		*numReturnedLocs = 0;
		PlaceId from = GvGetPlayerLocation(hv->gv, player);
		if (from == NOWHERE) return NULL;
		place = GvGetReachableByType(hv->gv, player, GvGetRound(hv->gv), from, road, rail, boat, numReturnedLocs);
		return place;
	} else {
		PlaceId * place = NULL;
		PlaceId from = GvGetPlayerLocation(hv->gv, PLAYER_DRACULA);
		place = GvGetReachableByType(hv->gv, PLAYER_DRACULA, GvGetRound(hv->gv), from, road, rail, boat, numReturnedLocs);
		int *newNum = 0;
		PlaceId * newPlace = NULL;
		for (int i = 0; i < *numReturnedLocs; i++) {
			if (canGo(hv, place[i])) {
				place = realloc(newPlace, (*newNum + 1) * sizeof(PlaceId));
				newPlace[*newNum] = place[i];
				*newNum = *newNum + 1;
			} else if (getDoubleBackNum(hv, place[i]) > 0) {
				newPlace = realloc(newPlace, (*newNum + 1) * sizeof(PlaceId));
				newPlace[*numReturnedLocs] = place[i];
				*newNum = *newNum + 1;
			} else if (i == *numReturnedLocs - 1 && canHide(hv)) {
				newPlace = realloc(newPlace, (*newNum + 1) * sizeof(PlaceId));
				newPlace[*numReturnedLocs] = place[i];
				*newNum = *newNum + 1;
			}
		}
		*numReturnedLocs = *newNum;
		return newPlace;
	}
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// TODO

bool canGo(HunterView hv, PlaceId place) {

	//Begin the game
	if (place == NOWHERE) return true;

	//Cannot in hospital
	if (place == HOSPITAL_PLACE) return false;

	int num = -1;
	bool canFree = false;
	PlaceId* list = GvGetLocationHistory(hv->gv, PLAYER_DRACULA, &num, &canFree);

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

int getDoubleBackNum(HunterView hv, PlaceId place) {

	int num = -1;
	bool canFree = false;
	PlaceId* list = GvGetLocationHistory(hv->gv, PLAYER_DRACULA, &num, &canFree);

	if (!canDoubleBack(hv)) 
		return 0;

	for (int i = 0; i < num && i < 6; i++) {
		if (list[i] == place) return i + 1;
	}

	return 0;
}

bool canDoubleBack(HunterView hv) {
	
	int num = -1;
	bool canFree = false;
	PlaceId* list = GvGetMoveHistory(hv->gv, PLAYER_DRACULA, &num, &canFree);

	for (int i = 0; i < num && i < 6; i++) {
		if (list[i] >= DOUBLE_BACK_1 && list[i] <= DOUBLE_BACK_5)
			return false;
	}
	return true;
}

bool canHide(HunterView hv) {

	int num = -1;
	bool canFree = false;
	PlaceId* list = GvGetMoveHistory(hv->gv, PLAYER_DRACULA, &num, &canFree);

	for (int i = 0; i < num && i < 6; i++) {
		if (list[i] == HIDE)
			return false;
	}
	return true;
}