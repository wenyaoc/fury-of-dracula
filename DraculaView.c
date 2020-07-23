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
/*
PlaceId* DvGetValidMoves(DraculaView dv, int* numReturnedMoves)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedMoves = 0;
	if (dv->data[PLAYER_DRACULA].turn == 0) return NULL;
	int DBnumber = 0;
	Map m = MapNew();
	ConnList connection = MapGetConnections(m, dv->data[PLAYER_DRACULA].first->place);
	ConnList curr = connection;
	PlaceId * move = NULL;
	while (curr != NULL) {
		if (curr->type == ROAD || curr->type == BOAT) {
			if (canGo(dv->data[PLAYER_DRACULA].first, curr->p)) {
				move = realloc(move, (*numReturnedMoves + 1) * sizeof(PlaceId));
				move[*numReturnedMoves] = curr->p;
				*numReturnedMoves = *numReturnedMoves + 1;
			}
			else if ((DBnumber = getDoubleBackNum(dv->data[PLAYER_DRACULA].first, curr->p)) > 0) {
				move = realloc(move, (*numReturnedMoves + 1) * sizeof(PlaceId));
				move[*numReturnedMoves] = DOUBLE_BACK_1 + DBnumber - 1;
				*numReturnedMoves = *numReturnedMoves + 1;
			}
		}
		curr = curr->next;
	}
	if (canDoubleBack(dv->data[PLAYER_DRACULA].first)) {
		move = realloc(move, (*numReturnedMoves + 1) * sizeof(PlaceId));
		move[*numReturnedMoves] = DOUBLE_BACK_1;
		*numReturnedMoves = *numReturnedMoves + 1;
	}
	if (canHide(dv->data[PLAYER_DRACULA].first)) {
		move = realloc(move, (*numReturnedMoves + 1) * sizeof(PlaceId));
		move[*numReturnedMoves] = HIDE;
		*numReturnedMoves = *numReturnedMoves + 1;
	}

	for (int i = 0; i < *numReturnedMoves; i++) {
		printf("%s ", placeIdToName(move[i]));
	}
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
	if (dv->data[4].turn == 0) return NULL;
	Map m = MapNew();
	ConnList connection;
	if (placeIsReal(dv->data[4].first->place) == 0) {
		connection = MapGetConnections(m, dv->data[4].first->next->place);
	}
	else {
		connection = MapGetConnections(m, dv->data[4].first->place);
	}
	ConnList curr = connection;
	PlaceId* place = NULL;
	while (curr != NULL) {
		if ((curr->type == ROAD && road == true) || (curr->type == BOAT && boat == true)) {
			if (canGo(dv->data[4].first, curr->p)) {
				place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
				place[*numReturnedLocs] = curr->p;
				*numReturnedLocs = *numReturnedLocs + 1;
			}
			else if (getDoubleBackNum(dv->data[4].first, curr->p) > 0) {
				place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
				place[*numReturnedLocs] = curr->p;
				*numReturnedLocs = *numReturnedLocs + 1;
			}
		}
		curr = curr->next;
	}
	if (canHide(dv->data[4].first)) {
		place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
		place[*numReturnedLocs] = dv->data[4].first->place;
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
	if (player == PLAYER_DRACULA) return DvWhereCanIGoByType(dv, road, boat, numReturnedLocs);

	int playernum = player;
	if (dv->data[playernum].turn == 0) return NULL;
	Map m = MapNew();
	ConnList connection;

	if (placeIsReal(dv->data[4].first->place) == 0) {
		connection = MapGetConnections(m, dv->data[4].first->next->place);
	}
	else {
		connection = MapGetConnections(m, dv->data[4].first->place);
	}
	ConnList curr = connection;
	PlaceId* place = NULL;
	while (curr != NULL) {
		if ((curr->type == ROAD && road == true) || (curr->type == BOAT && boat == true)) {
			place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
			place[*numReturnedLocs] = dv->data[4].first->place;
			*numReturnedLocs = *numReturnedLocs + 1;
			continue;
		}
		if (curr->type == RAIL && rail == true && dv->round % 4 != 0) {
			if (dv->round % 4 == 1) {
				place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
				place[*numReturnedLocs] = dv->data[4].first->place;
				*numReturnedLocs = *numReturnedLocs + 1;
				continue;
			}

			ConnList connection2 = MapGetConnections(m, curr->p);
			ConnList curr2 = connection2;
			while (curr2 != NULL) {
				if (curr2->type == RAIL) {
					place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
					place[*numReturnedLocs] = dv->data[4].first->place;
					*numReturnedLocs = *numReturnedLocs + 1;
					if (dv->round % 4 == 3) {
						ConnList connection3 = MapGetConnections(m, curr->p);
						ConnList curr3 = connection3;
						while (curr3 != NULL) {
							if (curr3->type == RAIL) {
								place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
								place[*numReturnedLocs] = dv->data[4].first->place;
								*numReturnedLocs = *numReturnedLocs + 1;
							}
							curr3 = curr3->next;
						}
					}
				}
				curr2 = curr2->next;
			}
		}
		curr = curr->next;
	}
	return place;
}*/

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// TODO