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
	// get all the locations in the history
	PlaceId lastLocation = GvGetPlayerLocation(dv->gv, PLAYER_DRACULA);
	if (lastLocation == NOWHERE) // Dracula has no movement yet
		return NULL;
	
	int numReachable;
	// get all the reachable places for Dracula
    PlaceId * reachable = GvGetReachable(dv->gv, PLAYER_DRACULA, GvGetRound(dv->gv), lastLocation,&numReachable);
	PlaceId * move = NULL;
	// search through all the reachable locations
	for (int i = 0; i < numReachable; i++) {
		if (canGo(dv, reachable[i])) { // dracula can directly move to that location
			move = realloc(move, (*numReturnedMoves + 1) * sizeof(PlaceId));
			move[*numReturnedMoves] = reachable[i];
			*numReturnedMoves = *numReturnedMoves + 1;
		} else if (getDoubleBackNum(dv, reachable[i]) > 0) { // dracula can double back to that location
			int DBnumber = getDoubleBackNum(dv, reachable[i]);
			// if there is no double back in the trail and the last move was HIDE
			// Dracula can go either DOUBLE_BACK_1 or DOUBLE_BACK_2
			if (DBnumber == 2) { 
				bool canFree;
				int numReturned = 0;
				// get the last move in the history
				PlaceId * lastMove = GvGetLastMoves(dv->gv, PLAYER_DRACULA, 1, &numReturned, &canFree);
				if (numReturned > 0){
					if (lastMove[0] == HIDE) { // if the last move is HIDE, add DOUBLE_BACK_1 to valid moves
						move= realloc(move, (*numReturnedMoves + 1) * sizeof(PlaceId));
						move[*numReturnedMoves] = DOUBLE_BACK_1;
						*numReturnedMoves = *numReturnedMoves + 1;
					}
					free(lastMove);
				}
			}
			// add DOUBLE_BACK_# to the history
			move= realloc(move, (*numReturnedMoves + 1) * sizeof(PlaceId));
			move[*numReturnedMoves] = DOUBLE_BACK_1 + DBnumber - 1;
			*numReturnedMoves = *numReturnedMoves + 1;
		}
	}
	if (canHide(dv)) { // dracula can hide in the current location
		move = realloc(move, (*numReturnedMoves + 1) * sizeof(PlaceId));
		move[*numReturnedMoves] = HIDE;
		*numReturnedMoves = *numReturnedMoves + 1;
	}
	free(reachable);
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
	PlaceId lastLocation = GvGetPlayerLocation(dv->gv, PLAYER_DRACULA); // get current location
	if (lastLocation == NOWHERE) // Dracula has no movement yet
		return NULL;
    
	int numReachable;
	// get all the reachable places for Dracula
    PlaceId * reachable = GvGetReachableByType(dv->gv, PLAYER_DRACULA, GvGetRound(dv->gv), lastLocation, road, false, boat, &numReachable);
	PlaceId * place = NULL;
	// search through all the reachable locations
	for (int i = 0; i < numReachable; i++) {
		if (canGo(dv, reachable[i])) { // dracula can directly move to that location
			place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
			place[*numReturnedLocs] = reachable[i];
			*numReturnedLocs = *numReturnedLocs + 1;
		} else if (getDoubleBackNum(dv, reachable[i]) > 0) { // dracula can double back to that location
			place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
			place[*numReturnedLocs] = reachable[i];
			*numReturnedLocs = *numReturnedLocs + 1;
		}
	}
	// dracula can hide in the current location & the location haven't been added
	if (canHide(dv) && !getDoubleBackNum(dv, lastLocation)) { 
		place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
		place[*numReturnedLocs] = lastLocation;
		*numReturnedLocs = *numReturnedLocs + 1;
	} 
	free (reachable);
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
	if (player == PLAYER_DRACULA) // player is dracula, call whereCanIMove
		return DvWhereCanIGoByType(dv, road, boat, numReturnedLocs);

	PlaceId lastLocation = GvGetPlayerLocation(dv->gv, player);
	if (lastLocation == NOWHERE) // the player has no movement yet
		return NULL;

	Player currPlayer = GvGetPlayer(dv->gv); // get the current player
	
	if (currPlayer < player) // the player haven't made a move in the current turn
		return GvGetReachableByType(dv->gv, player, GvGetRound(dv->gv), lastLocation, road, rail, boat, numReturnedLocs);
	else // the player alreade made a move in the current turn
		return GvGetReachableByType(dv->gv, player, GvGetRound(dv->gv) + 1, lastLocation, road, rail, boat, numReturnedLocs);
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// TODO

// determine whether the player can directly go to a place
// input: DraculaView, place
// output: ture if the place is allowed to go, false otherwise
bool canGo(DraculaView dv, PlaceId place) {

	if (place == NOWHERE) // Dracula has no movement yet
		return true;
	if (place == HOSPITAL_PLACE) // Dracula cannot goto hospital
		return false;

	int num = -1;
	bool canFree = false;
	bool go = true;
	PlaceId* list = GvGetLastLocations(dv->gv, PLAYER_DRACULA, 5, &num, &canFree);

	for (int i = 0; i < num; i++) {	
		if (list[i] == place) // the place is already in the trail
			go = false;
		else if (list[i] >= DOUBLE_BACK_1 && list[i] <= DOUBLE_BACK_5 && list[i] - DOUBLE_BACK_1 < num) {
			if (list[list[i] - DOUBLE_BACK_1] == place) // Dracula already double back to that place before
				go = false;
		} else if (i + 1 == 6 && num >= 6) {
			if (list[i] == HIDE && list[i + 1] == place) // Dracula already hide in that place before
				go = false;
		}
	}
	if (canFree)
		free(list); // free the location list
	return go;
}

// calculate the number of the DOUBLE_BACK move
// input: DraculaView, place
// output: 0: cannot double back, #1-5: # for DOUBLE_BACK
int getDoubleBackNum(DraculaView dv, PlaceId place) {

	if (!canDoubleBack(dv)) // there exist a double back move in history
		return 0;

	int num = -1;
	bool canFree = false;
	int doubleBackNum = 0;
	
	PlaceId* list = GvGetLastLocations(dv->gv, PLAYER_DRACULA, 5, &num, &canFree);

	for (int i = num - 1; i >= 0; i--) { // calculate the number
		if (list[i] == place) 
			doubleBackNum = num - i;
	}
	if (canFree)
		free(list);
	return doubleBackNum;
}

// determine whether Dracula can double back in the curretnt turn
// input: DraculaView
// output: true if dracula can double back, false otherwise
bool canDoubleBack(DraculaView dv) {
	
	int num = -1;
	bool canFree = false;
	bool doubleBack = true;
	PlaceId* list = GvGetLastMoves(dv->gv, PLAYER_DRACULA, 5, &num, &canFree);

	for (int i = 0; i < num; i++) {
		if (list[i] >= DOUBLE_BACK_1 && list[i] <= DOUBLE_BACK_5)
			doubleBack = false; // there exist a double back move in the trail
	}
	if (canFree)
		free(list);
	return doubleBack;
}

// determine whether Dracula can hide in the curretnt turn
// input: HunterView
// output: true if dracula can HIDE, false otherwise
bool canHide(DraculaView dv) {
	int num = -1;
	bool canFree = false;
	bool hide = true;
	PlaceId* list = GvGetLastMoves(dv->gv, PLAYER_DRACULA, 5, &num, &canFree);

	for (int i = 0; i < num; i++) {
		if (list[i] == HIDE) // there exist a HIDE move in the trail
			hide = false;
	}
	if (canFree)
		free(list);
	return hide;
}
