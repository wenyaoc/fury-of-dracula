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
#include <string.h>
// TODO: ADD YOUR OWN STRUCTS HERE

#define MAXCHAR 8
#define MAXLINE 40

struct draculaView {
	// TODO: ADD FIELDS HERE
	Round round;
	int score;
	playerData data[5];
};

Player getPlayer(char c);
void trailLoad(DraculaView dv, char * play);
void DvEvent(DraculaView dv, char* play, PlaceId place, int player);
void HvEvent(DraculaView dv, char* play, PlaceId place, int player);
void deleteTraps(DraculaView dv, PlaceId place);
void deleteVampire(DraculaView dv, PlaceId place);
////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

DraculaView DvNew(char *pastPlays, Message messages[])
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	DraculaView new_dv = malloc(sizeof(*new_dv));
	if (new_dv == NULL) {
		fprintf(stderr, "Couldn't allocate DraculaView\n");
		exit(EXIT_FAILURE);
	}
	int totalLength = strlen(pastPlays); 
	new_dv->round = (totalLength + 1) / MAXLINE;
	new_dv->score = GAME_START_SCORE;

	//Set Player
	for (int i = 0; i < 5; i++) {
		new_dv->data[i].player = i;
		new_dv->data[i].health = GAME_START_HUNTER_LIFE_POINTS;
		new_dv->data[i].turn = 0;
		new_dv->data[i].first = NULL;
	}
	new_dv->data[PLAYER_DRACULA].health = GAME_START_BLOOD_POINTS;

	if (totalLength + 1 < MAXCHAR) return new_dv;

	char * play = calloc(MAXCHAR, sizeof(char));
	for (int i = 0; i <= totalLength; i++) {
		play[i%8] = pastPlays[i];
		if ((i%8) == 7) {
			trailLoad(new_dv, play);
		}
	}

	free(play);
	return new_dv;
}

void DvFree(DraculaView dv)
{
    // TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION

    for (int i = 0; i < 5; i++) {
        ListFree(dv->data[i].first);
    }
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
    return dv->data[player].health;
}

PlaceId DvGetPlayerLocation(DraculaView dv, Player player)
{
    // TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    if(dv->data[player].turn == 0)
        return NOWHERE;
    else 
        return dv->data[player].first->place;
}

PlaceId DvGetVampireLocation(DraculaView dv)
{
    // TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    if(dv->data[PLAYER_DRACULA].turn == 0)
        return NOWHERE;
    HistoryNode curr = dv->data[PLAYER_DRACULA].first;
    int counter = 0;
    while (curr != NULL && counter < 5) {
        if (curr->vampire == true) {
            return curr->place;
        }
        counter++;
        curr = curr->next;
    }
    return NOWHERE;
}

PlaceId *DvGetTrapLocations(DraculaView dv, int *numTraps)
{
    // TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    *numTraps = 0;
    if(dv->data[PLAYER_DRACULA].turn == 0) return NULL;
    HistoryNode curr = dv->data[PLAYER_DRACULA].first;
	
    int counter = 0;
    PlaceId * place = NULL;
    while (curr != NULL && counter < 5 && counter < dv->round) {
        if (curr->trapNumber > 0) {
            place = realloc(place, (*numTraps + 1) * sizeof(PlaceId));
			if (curr->place == HIDE) {
				if (curr->next->place >= DOUBLE_BACK_1 && curr->next->place <= DOUBLE_BACK_5)
					place[*numTraps] = findDBCity(curr->next)->place;
				else
					place[*numTraps] = curr->next->place;

			}
			else if (curr->place >= DOUBLE_BACK_1 && curr->place <= DOUBLE_BACK_5) {
				place[*numTraps] = findDBCity(curr)->place;
			} else
				place[*numTraps] = curr->place;
            *numTraps = *numTraps + 1;			
        }
        counter++;
        curr = curr->next;
    }
    return place;
}


////////////////////////////////////////////////////////////////////////
// Making a Move

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

	for(int i = 0; i < *numReturnedMoves; i++) {
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
	ConnList connection ;
	if (placeIsReal(dv->data[4].first->place) == 0) {
		connection = MapGetConnections(m, dv->data[4].first->next->place);
	}
	else {
		connection = MapGetConnections(m, dv->data[4].first->place);
	}
	ConnList curr = connection;
	PlaceId * place = NULL;
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
	PlaceId * place = NULL;
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
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// TODO

//backup function

void trailLoad(DraculaView dv, char* play) {

	Player player = getPlayer(play[0]);
	char abbrev[3];
	abbrev[0] = play[1];
	abbrev[1] = play[2];
	abbrev[2] = '\0';
	PlaceId place = placeAbbrevToId(abbrev);
	if (player == PLAYER_DRACULA)
		DvEvent(dv, play, place, player);
	else
		HvEvent(dv, play, place, player);

}

void DvEvent(DraculaView dv, char* play, PlaceId place, int player)
{
	bool vampire = false;
	int trap = 0;
	if (play[3] == 'T')
		trap++;
	if (play[4] == 'V')
		vampire = true;

	HistoryNode new = creatNode(place, vampire, trap, true);
	dv->data[player] = addToHistory(dv->data[player], new);

	if (placeIsSea(place))
		dv->data[player].health -= LIFE_LOSS_SEA;
	/*
	handle M
	*/
	if (play[6] == 'V') {
		dv->score -= SCORE_LOSS_VAMPIRE_MATURES;
	}

	if (place == CASTLE_DRACULA)
		dv->data[player].health += LIFE_GAIN_CASTLE_DRACULA;

	dv->score -= SCORE_LOSS_DRACULA_TURN;
}

void HvEvent(DraculaView dv, char* play, PlaceId place, int player)
{
	if (dv->data[player].health <= 0) 
		dv->data[player].health = GAME_START_HUNTER_LIFE_POINTS;

	HistoryNode new = creatNode(place, false, 0, true);
	dv->data[player] = addToHistory(dv->data[player], new);
	if (play[3] == 'T') {
		deleteTraps(dv, place);
		dv->data[player].health -= LIFE_LOSS_TRAP_ENCOUNTER;

	}
	if (play[3] == 'V' || play[4] == 'V') {
		deleteVampire(dv, place);
	}
	if (play[3] == 'D' || play[4] == 'D' || play[5] == 'D') {
		dv->data[player].health -= LIFE_LOSS_DRACULA_ENCOUNTER;
		dv->data[PLAYER_DRACULA].health -= LIFE_LOSS_HUNTER_ENCOUNTER;
	}

	if (dv->data[player].turn > 1 && dv->data[player].first->place == dv->data[player].first->next->place) {
		dv->data[player].health += LIFE_GAIN_REST;
		if (dv->data[player].health > GAME_START_HUNTER_LIFE_POINTS)
			dv->data[player].health = GAME_START_HUNTER_LIFE_POINTS;
	}
}


Player getPlayer(char c){
	if (c == 'G') return PLAYER_LORD_GODALMING;
	if (c == 'S') return PLAYER_DR_SEWARD;
	if (c == 'H') return PLAYER_VAN_HELSING;
	if (c == 'M') return PLAYER_MINA_HARKER;
	else return PLAYER_DRACULA;		
}

void deleteTraps(DraculaView dv, PlaceId place) {

	HistoryNode curr = dv->data[PLAYER_DRACULA].first;
	HistoryNode result = NULL;
	for (int counter = 0; curr != NULL && counter < 5; counter++) {
		if (curr->place == place && curr->trapNumber == 1) {
			curr->trapNumber = 0;
			return; 
		}	
		curr = curr->next;
	}
	curr = dv->data[PLAYER_DRACULA].first;
	for (int counter = 0; curr != NULL && counter < 5; counter++) {
		if (curr->place == HIDE) {
			if (curr->next->place == place){
				if(curr->next->trapNumber == 1)
					curr->next->trapNumber = 0;
				else
					curr->trapNumber = 0;
				return; 
			} else if (curr->next->place >= DOUBLE_BACK_1 && curr->next->place <= DOUBLE_BACK_5) {
				HistoryNode node = findDBCity(curr->next);
				if (node->place == place) {
					if (node->trapNumber == 1)
						node->trapNumber = 0;
					else if (curr->next->trapNumber == 1)
						curr->trapNumber = 0;
					else
						curr->trapNumber = 0;
					return; 
				}
			}
		} else if (curr->place >= DOUBLE_BACK_1 && curr->place <= DOUBLE_BACK_5) {
			HistoryNode node = findDBCity(curr->next);
			if (node->place == place) {
				if (node->trapNumber == 1)
					node->trapNumber = 0;
				else
					curr->trapNumber = 0;
				return;
			} else if (node->place == HIDE) {
				if (node->next->place == place) {
					if (node->next->trapNumber == 1)
						node->next->trapNumber = 0;
					else if (node->trapNumber == 1)
						node->trapNumber = 0;
					else
						curr->trapNumber = 0;
					return;
				}
				
			}
		}
		curr = curr->next;
	}

	if (result != NULL)
		result->trapNumber = 0;
}

void deleteVampire(DraculaView dv, PlaceId place) {

	HistoryNode curr = dv->data[PLAYER_DRACULA].first;
	HistoryNode result = NULL;
	for (int counter = 0; curr != NULL && counter < 5; counter++) {
		if (curr->vampire && curr->place == place)
			result = curr;
		curr = curr->next;
	}
	if (result != NULL)
		result->vampire = false;
}
