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

typedef struct _placeNode* PlaceNode;

struct draculaView {
	// TODO: ADD FIELDS HERE
	Round round;
	int score;
	playerData data[5];
};

struct _placeNode {
	PlaceId place;
	PlaceNode next;
};

char* split(char* pastPlays, char* str);
////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

DraculaView DvNew(char *pastPlays, Message messages[])
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	DraculaView new_dv = malloc(sizeof(*new_dv));
	new_dv->round = 0;
	new_dv->score = GAME_START_SCORE;
	
	char* gdata = split(pastPlays, " ");

	//Set Player
	for (int i = 0; i < 3; i++) {
		new_dv->data[i].player = i;
		new_dv->data[i].health = GAME_START_HUNTER_LIFE_POINTS;
		new_dv->data[i].totalNumber = 0;
	}
	new_dv->data[PLAYER_DRACULA].health = GAME_START_BLOOD_POINTS;

	//Set Location
	for (int i = 0; i < 5; i++) {
		new_dv->data[i].first = new_dv->data[i].last = malloc(sizeof(HistoryNode));
		new_dv->data[i].first->place = gdata[i];
	}
	
	//Set Vampire and Trap
	new_dv->data[PLAYER_DRACULA].first->vampire = false;
	new_dv->data[PLAYER_DRACULA].first->trapNumber = 0;

	if (new_dv == NULL) {
		fprintf(stderr, "Couldn't allocate DraculaView\n");
		exit(EXIT_FAILURE);
	}

	return new_dv;
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
	return dv->round;
}

int DvGetScore(DraculaView dv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return dv->score;
}

int DvGetHealth(DraculaView dv, Player player)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	switch (player) {
		case PLAYER_LORD_GODALMING:
			return dv->data[PLAYER_LORD_GODALMING].health;
		case PLAYER_DR_SEWARD:
			return dv->data[PLAYER_DR_SEWARD].health;
		case PLAYER_VAN_HELSING:
			return dv->data[PLAYER_VAN_HELSING].health;
		case PLAYER_MINA_HARKER:
			return dv->data[PLAYER_MINA_HARKER].health;
		case PLAYER_DRACULA:
			return dv->data[PLAYER_DRACULA].health;
	}

	return 0;
}

PlaceId DvGetPlayerLocation(DraculaView dv, Player player)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	switch (player) {
		case PLAYER_LORD_GODALMING:
			return dv->data[PLAYER_LORD_GODALMING].last->place;
		case PLAYER_DR_SEWARD:
			return dv->data[PLAYER_DR_SEWARD].last->place;
		case PLAYER_VAN_HELSING:
			return dv->data[PLAYER_VAN_HELSING].last->place;
		case PLAYER_MINA_HARKER:
			return dv->data[PLAYER_MINA_HARKER].last->place;
		case PLAYER_DRACULA:
			return dv->data[PLAYER_DRACULA].last->place;
	}

	return NOWHERE;
}

PlaceId DvGetVampireLocation(DraculaView dv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	HistoryNode node = dv->data[PLAYER_DRACULA].first;
	while (node != dv->data[PLAYER_DRACULA].last) {

		if (node->vampire)
			return node->place;

		node = node->next;
	}

	return NOWHERE;
}

PlaceId* DvGetTrapLocations(DraculaView dv, int* numTraps)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numTraps = 0;
	PlaceNode place_head = NULL;
	PlaceNode place_node = NULL;
	HistoryNode history_node = dv->data[PLAYER_DRACULA].first;
	while (history_node != dv->data[PLAYER_DRACULA].last) {

		if (place_head == NULL) {
			if (history_node->trapNumber >= 1) {
				place_head = malloc(sizeof(PlaceNode));
				place_head->place = history_node->place;
				place_head->next = NULL;
				place_node = place_head;
				*numTraps += history_node->trapNumber;
			}
		}
		else if (history_node->trapNumber >= 1) {
			place_node->next = malloc(sizeof(PlaceNode));
			place_node->next->place = history_node->place;
			place_node = place_node->next;
			place_node = NULL;
			*numTraps += history_node->trapNumber;
		}

		history_node = history_node->next;
	}

	return place_head;
}

////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId *DvGetValidMoves(DraculaView dv, int *numReturnedMoves)
{
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

char* firstSplit(char* pastPlays, char* str)
{
 int i = 0;
 char temp[7][10] = { 0 };
 char* p1 = (char*)malloc(128);

 while ((p1 = strchr(pastPlays, ' ')) != NULL && i <= 5)
 {
  strncpy(temp[i], pastPlays, strlen(pastPlays) - strlen(p1));
  pastPlays = p1 + 1;
  i++;
 }
 strncpy(temp[i], pastPlays, strlen(pastPlays));

 char place[3];
 for (int j = 0; j <= i; j++) {
  for (int m = 1; m < 3; m++) {
   place[m - 1] = temp[j][m];
   
  }
  printf("tmp[%d] = %s\n", j, place);
 }

 for (int j = 0; j <= i; j++) 
  printf("tmp[%d] = %s\n", j, temp[j]);

 system("pause");
 return temp;
 
}


