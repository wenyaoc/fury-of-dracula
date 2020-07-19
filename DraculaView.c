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
#include "PlayerDataList.h"
// TODO: ADD YOUR OWN STRUCTS HERE

struct draculaView {
	// TODO: ADD FIELDS HERE
	Round round;
	int score;
	playerData data[5];
};


////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

DraculaView DvNew(char *pastPlays, Message messages[])
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	DraculaView new = malloc(sizeof(*new));
	if (new == NULL) {
		fprintf(stderr, "Couldn't allocate DraculaView\n");
		exit(EXIT_FAILURE);
	}
	return new;
}

void DvFree(DraculaView dv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	free(dv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round DvGetRound(DraculaView dv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return 0;
}

int DvGetScore(DraculaView dv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return 0;
}

int DvGetHealth(DraculaView dv, Player player)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return 0;
}

PlaceId DvGetPlayerLocation(DraculaView dv, Player player)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return NOWHERE;
}

PlaceId DvGetVampireLocation(DraculaView dv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return NOWHERE;
}

PlaceId *DvGetTrapLocations(DraculaView dv, int *numTraps)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numTraps = 0;
	return NULL;
}

////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId *DvGetValidMoves(DraculaView dv, int *numReturnedMoves)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedMoves = 0;
	if(dv->data[4].totalNumber == 0) return NULL;
	int DBnumber = 0;
	Map m = MapNew();
	ConnList connection = MapGetConnections(m, dv->data[4].first->place);
	ConnList curr = connection;
	PlaceId * place = NULL;
	while (curr != NULL) {
		if (curr->type == ROAD || curr->type == BOAT) {
			if (canGo(dv->data[4].first, curr->p)) {
				place = realloc(place, (*numReturnedMoves + 1) * sizeof(PlaceId));
				place[*numReturnedMoves] =  curr->p;
				*numReturnedMoves = *numReturnedMoves + 1;
			} else if ((DBnumber = canDoubleBack(dv->data[4].first, curr->p)) > 0) {
				place = realloc(place, (*numReturnedMoves + 1) * sizeof(PlaceId));
				place[*numReturnedMoves] = DOUBLE_BACK_1 + DBnumber - 1;
				*numReturnedMoves = *numReturnedMoves + 1;
			}	
		}
		curr = curr -> next;
	}
	if (canHide(dv->data[4].first)) {
		place = realloc(place, (*numReturnedMoves + 1) * sizeof(PlaceId));
		place[*numReturnedMoves] = HIDE;
		*numReturnedMoves = *numReturnedMoves + 1;
	}
	return place;
}

PlaceId *DvWhereCanIGo(DraculaView dv, int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return DvWhereCanIGoByType(dv, true, true, numReturnedLocs);
}

PlaceId *DvWhereCanIGoByType(DraculaView dv, bool road, bool boat,
                             int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	if(dv->data[4].totalNumber == 0) return NULL;

	Map m = MapNew();
	ConnList connection = MapGetConnections(m, dv->data[4].first->place);
	ConnList curr = connection;
	PlaceId * place = NULL;
	while (curr != NULL) {
		if ((curr->type == ROAD && road == true) || (curr->type == BOAT && boat == true)) {
			if (canGo(dv->data[4].first, curr->p)) {
				place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
				place[*numReturnedLocs] =  curr->p;
				*numReturnedLocs = *numReturnedLocs + 1;
			} else if (canDoubleBack(dv->data[4].first, curr->p) > 0) {
				place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
				place[*numReturnedLocs] = curr->p;
				*numReturnedLocs = *numReturnedLocs + 1;
			}	
		}
		curr = curr -> next;
	}
	if (canHide(dv->data[4].first)) {
		place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
		place[*numReturnedLocs] = dv->data[4].first->place;
		*numReturnedLocs = *numReturnedLocs + 1;
	}
	return place;
}

PlaceId *DvWhereCanTheyGo(DraculaView dv, Player player,
                          int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return DvWhereCanTheyGoByType(dv, player, true, true, true, numReturnedLocs);
}

PlaceId *DvWhereCanTheyGoByType(DraculaView dv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	if (player == PLAYER_DRACULA) return DvWhereCanIGoByType(dv, road, boat, numReturnedLocs);

	int playernum = player;
	if(dv->data[playernum].totalNumber == 0) return NULL;
	Map m = MapNew();
	ConnList connection = MapGetConnections(m, dv->data[4].first->place);
	ConnList curr = connection;
	PlaceId * place = NULL;
	while (curr != NULL) {
		if ((curr->type == ROAD && road == true) || (curr->type == BOAT && boat == true)) {
			place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
			place[*numReturnedLocs] = dv->data[4].first->place;
			*numReturnedLocs = *numReturnedLocs + 1;
		} else if (curr->type == RAIL && rail == true && dv->round % 4 != 0) {
			if (dv->round % 4 == 1) {
				place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
				place[*numReturnedLocs] = dv->data[4].first->place;
				*numReturnedLocs = *numReturnedLocs + 1;
			} else {
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
		}
		curr = curr -> next;
	}
	return place;
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions
// TODO



