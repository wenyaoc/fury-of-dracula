////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// GameView.c: GameView ADT implementation
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
#include "Map.h"
#include "Places.h"
// add your own #includes here
#include <string.h>

#define MAXCHAR 8
#define MAXLINE 40

// TODO: ADD YOUR OWN STRUCTS HERE
typedef struct historyNode *HistoryNode;

struct historyNode {
	PlaceId place; //the real location
	bool vampire;
	int trapNumber;
    bool revealed;
	HistoryNode next;
};

typedef struct _playerData {
	Player player;
	int health;
	int turn; // total munber in history
	HistoryNode first; // the first node in the history list
} playerData;


struct gameView {
	// TODO: ADD FIELDS HERE
	Round round;
	int score;
	playerData data[5]; // data[0-3]:hunter  data[4]:dracula
};

Player getPlayer(char c);
void trailLoad(GameView gv, char * play);
void DvEvent(GameView gv, char* play, PlaceId place, int player);
void HvEvent(GameView gv, char* play, PlaceId place, int player);
void deleteTraps(GameView gv, PlaceId place);
void deleteVampire(GameView gv);
bool deleteLastTraps(HistoryNode node, PlaceId place, int num);
//bool deleteHideTraps(HistoryNode node, PlaceId place, int num);
//bool deleteDBTraps(HistoryNode node, PlaceId place, int num);
//void revealLocation(HistoryNode list, PlaceId place);

void ListFree(HistoryNode node);
HistoryNode creatNode(PlaceId place, bool vampire, int trap, bool revealed);
HistoryNode copyNode(HistoryNode prevNode);
playerData addToHistory(playerData data, HistoryNode newNode);
HistoryNode addToTail(HistoryNode list, HistoryNode newNode);
HistoryNode getLastN(playerData data, int n);
HistoryNode findDBCity(HistoryNode DBnode);
PlaceId * addPlace(PlaceId * place, int * num, PlaceId newPlace);
////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

GameView GvNew(char* pastPlays, Message messages[])
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	GameView new = malloc(sizeof(*new));
	if (new == NULL) {
		fprintf(stderr, "Couldn't allocate GameView!\n");
		exit(EXIT_FAILURE);
	}

	int totalLength = strlen(pastPlays);
	new->round = (totalLength + 1) / MAXLINE;
	new->score = GAME_START_SCORE;

	//Set Player
	for (int i = 0; i < 5; i++) {
		new->data[i].player = i;
		new->data[i].health = GAME_START_HUNTER_LIFE_POINTS;
		new->data[i].turn = 0;
		new->data[i].first = NULL;
	}
	new->data[PLAYER_DRACULA].health = GAME_START_BLOOD_POINTS;

	if (totalLength + 1 < MAXCHAR) return new;

	char* play = calloc(MAXCHAR, sizeof(char));
	for (int i = 0; i <= totalLength; i++) {
		play[i % 8] = pastPlays[i];
		if ((i % 8) == 7) {
			trailLoad(new, play);
		}
	}
	free(play);
	return new;
}

void GvFree(GameView gv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	for (int i = 0; i < 5; i++) {
		ListFree(gv->data[i].first);
	}
	free(gv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round GvGetRound(GameView gv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return gv->round;
}

Player GvGetPlayer(GameView gv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	for (int i = 0; i < 5; i++) {
		if(gv->data[i].turn < gv->round + 1) return i;
	}
	return 0;
}

int GvGetScore(GameView gv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return gv->score;
}

int GvGetHealth(GameView gv, Player player)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return gv->data[player].health < 0 ? 0 : gv->data[player].health;
}

PlaceId GvGetPlayerLocation(GameView gv, Player player)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	if (gv->data[player].turn == 0) return NOWHERE;
	else if (player != PLAYER_DRACULA) return gv->data[player].first->place;
	else {
		HistoryNode node = gv->data[player].first;

		if (node->place == TELEPORT) return CASTLE_DRACULA;
		if (placeIsLand(node->place)) {
			//if (node->revealed == true) 
			return node->place;
			//else return CITY_UNKNOWN;
		}
		else if (placeIsSea(node->place)) {
			//if (node->revealed == true) 
			return node->place;
			//else return SEA_UNKNOWN;
		}
		else if (node->place == HIDE) {
			HistoryNode real = node->next;
			if (!placeIsReal(real->place)) {
				real = findDBCity(real);
			}
			if (placeIsLand(real->place)) {
				//if (node->revealed == true) 
				return real->place;
				//else return CITY_UNKNOWN;
			}
			else if (placeIsSea(node->next->place)) {
				//if (node->revealed == true) 
				return real->place;
				//else return SEA_UNKNOWN;
			}
		}
		else {
			HistoryNode real = findDBCity(node);

			if (real->place == HIDE) {
				real = real->next;
			}
			if (placeIsLand(real->place)) {
				//if (node->revealed == true) 
				return real->place;
				//else return CITY_UNKNOWN;
			}
			else if (placeIsSea(real->place)) {
				//if (node->revealed == true) 
				return real->place;
				//else return SEA_UNKNOWN;
			}
		}
	}
	return UNKNOWN_PLACE;
}

PlaceId GvGetVampireLocation(GameView gv)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	if (gv->data[PLAYER_DRACULA].turn == 0)
		return NOWHERE;
	HistoryNode curr = gv->data[PLAYER_DRACULA].first;
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

PlaceId* GvGetTrapLocations(GameView gv, int* numTraps)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numTraps = 0;
	if (gv->data[PLAYER_DRACULA].turn == 0) return NULL;
	HistoryNode curr = gv->data[PLAYER_DRACULA].first;

	int counter = 0;
	PlaceId * place = NULL;
	while (curr != NULL && counter < 6 && counter < gv->round) {
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
			}
			else if (curr->place == TELEPORT)
				place[*numTraps] = CASTLE_DRACULA;
			else
				place[*numTraps] = curr->place;
			*numTraps = *numTraps + 1;
		}
		counter++;
		curr = curr->next;
	}
	return place;
}

////////////////////////////////////////////////////////////////////////
// Game History

PlaceId *GvGetMoveHistory(GameView gv, Player player,
                          int *numReturnedMoves, bool *canFree)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	int numMoves = gv->data[player].turn;
	return GvGetLastMoves(gv, player, numMoves, numReturnedMoves, canFree);
}

PlaceId* GvGetLastMoves(GameView gv, Player player, int numMoves,
						  int* numReturnedMoves, bool* canFree)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*canFree = true;
	int totalNum = gv->data[player].turn;
	if (totalNum == 0) return NULL;
	if (totalNum < numMoves)
		*numReturnedMoves = totalNum;
	else
		*numReturnedMoves = numMoves;

	PlaceId * move = malloc(*numReturnedMoves * sizeof(PlaceId));

	//if (player != PLAYER_DRACULA) {
	HistoryNode curr = gv->data[player].first;
	for (int i = *numReturnedMoves - 1; i >= 0; i--) {
		move[i] = curr->place;
		curr = curr->next;
	}
	return move;
	/*	} else {
			HistoryNode curr = gv->data[player].first;
			for (int counter = *numReturnedMoves - 1; counter >= 0; counter--) {
				if (curr->revealed == true || curr->place == HIDE || (curr->place >= DOUBLE_BACK_1 && curr->place <= DOUBLE_BACK_5)) {
					move[counter] = curr->place;
				} else {
					if (placeIsLand(curr->place))
						move[counter] = CITY_UNKNOWN;
					else if (placeIsSea(curr->place))
						move[counter] = SEA_UNKNOWN;

					else if (curr->place == HIDE) {

						if (placeIsLand(curr->place))
							move[counter] = CITY_UNKNOWN;
						else if (placeIsSea(curr->place))
							move[counter] = SEA_UNKNOWN;
						else if (curr->next->place >= DOUBLE_BACK_1 && curr->next->place <= DOUBLE_BACK_5) {
							HistoryNode newCurr = findDBCity(curr->next);
							if (placeIsLand(newCurr->place))
								move[counter] = CITY_UNKNOWN;
							else if (placeIsSea(newCurr->place))
								move[counter] = SEA_UNKNOWN;
						}
					} else {
						HistoryNode newCurr = findDBCity(curr->next) {

						}
					}*/
					//	}
					//	printf("%d ", counter);
					//	curr = curr->next;
					//}
				//}
				//return move;
}

PlaceId *GvGetLocationHistory(GameView gv, Player player,
                              int *numReturnedLocs, bool *canFree)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	int numLocs = gv->data[player].turn;
	return GvGetLastLocations(gv, player, numLocs, numReturnedLocs, canFree);
}

PlaceId* GvGetLastLocations(GameView gv, Player player, int numLocs,
							  int* numReturnedLocs, bool* canFree)
{
	PlaceId* place = GvGetLastMoves(gv, player, numLocs, numReturnedLocs, canFree);
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	HistoryNode curr = gv->data[player].first;
	for (int i = (*numReturnedLocs - 1); i >= 0; i--) {
		if (place[i] == HIDE) {
			if (curr->next->place >= DOUBLE_BACK_1 && curr->next->place <= DOUBLE_BACK_5)
				place[i] = findDBCity(curr->next)->place;
			else
				place[i] = curr->next->place;
		}
		else if (curr->place >= DOUBLE_BACK_1 && curr->place <= DOUBLE_BACK_5) {
			HistoryNode newCurr = findDBCity(curr);
			if (newCurr->place == HIDE)
				place[i] = newCurr->next->place;
			else
				place[i] = newCurr->place;
		}

		if (place[i] == TELEPORT)
			place[i] = CASTLE_DRACULA;

		curr = curr->next;
	}
	return place;
}

////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId *GvGetReachable(GameView gv, Player player, Round round,
                        PlaceId from, int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return GvGetReachableByType(gv, player, round, from, true, true, true, numReturnedLocs);
}

PlaceId *GvGetReachableByType(GameView gv, Player player, Round round,
                              PlaceId from, bool road, bool rail,
                              bool boat, int *numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	Map m = MapNew();
	ConnList connection;
	
	connection = MapGetConnections(m, from);
	ConnList curr = connection;
	PlaceId * place = NULL;
	if (curr == NULL) return NULL;

	if (player == PLAYER_DRACULA) {
		while (curr != NULL) {
			if ((curr->type == ROAD && road == true) || (curr->type == BOAT && boat == true)) {
				if (curr->p != HOSPITAL_PLACE) {
					place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
					place[*numReturnedLocs] = curr->p;
					*numReturnedLocs = *numReturnedLocs + 1;
				}
			}
			curr = curr->next;
		}
		place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
		place[*numReturnedLocs] = from;
		*numReturnedLocs = *numReturnedLocs + 1;

	} else {
		while (curr != NULL) {
			if ((curr->type == ROAD && road == true) || (curr->type == BOAT && boat == true)) {
				place = addPlace(place, numReturnedLocs, curr->p);
			} else if (curr->type == RAIL && rail == true && round % 4 != 0) {
				place = addPlace(place, numReturnedLocs, curr->p);
				if (round % 4 > 1) {
					ConnList connection2 = MapGetConnections(m, curr->p);
					ConnList curr2 = connection2;
					while (curr2 != NULL) {
						if (curr2->type == RAIL) {
							place = addPlace(place, numReturnedLocs, curr2->p);
							if (round % 4 == 3) {
								ConnList connection3 = MapGetConnections(m, curr->p);
								ConnList curr3 = connection3;
								while (curr3 != NULL) {
									if (curr3->type == RAIL) {
										place = addPlace(place, numReturnedLocs, curr3->p);
									}
									curr3 = curr3->next;
								}
							}
						}
						curr2 = curr2->next;
					}
				}
			}
			curr = curr->next;
		}
	}
	place = addPlace(place, numReturnedLocs, from);
	return place;
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// TODO
void trailLoad(GameView gv, char* play) {

	Player player = getPlayer(play[0]);
	char abbrev[3];
	abbrev[0] = play[1];
	abbrev[1] = play[2];
	abbrev[2] = '\0';
	PlaceId place = placeAbbrevToId(abbrev);
	if (player == PLAYER_DRACULA)
		DvEvent(gv, play, place, player);
	else
		HvEvent(gv, play, place, player);
}

void DvEvent(GameView gv, char* play, PlaceId place, int player)
{
	bool vampire = false;
	int trap = 0;
	if (play[3] == 'T')
		trap++;
	if (play[4] == 'V')
		vampire = true;

	HistoryNode new = creatNode(place, vampire, trap, true);
	gv->data[player] = addToHistory(gv->data[player], new);

	if (place < 100)
		gv->data[player].first->revealed = true;

	if (play[5] == 'M') {
		HistoryNode curr = gv->data[PLAYER_DRACULA].first;
		for (int counter = 0; curr != NULL && counter < 6; counter++) {
			curr = curr->next;
		}
		curr->trapNumber = 0;
	}
	else if (play[5] == 'V') {
		gv->score -= SCORE_LOSS_VAMPIRE_MATURES;
	}

	if (placeIsSea(place))
		gv->data[player].health -= LIFE_LOSS_SEA;
	else if (place >= DOUBLE_BACK_1 && place <= DOUBLE_BACK_5) {
		HistoryNode node = findDBCity(gv->data[PLAYER_DRACULA].first);
		if (placeIsSea(node->place))
			gv->data[player].health -= LIFE_LOSS_SEA;
	}

	if (place == CASTLE_DRACULA || place == TELEPORT) {
		gv->data[player].health += LIFE_GAIN_CASTLE_DRACULA ;
		//gv->data[player].first->revealed = true;
	}
	else if (place == HIDE) {
		HistoryNode node = gv->data[PLAYER_DRACULA].first->next;

		if (node->place == CASTLE_DRACULA || node->place == TELEPORT) {
			gv->data[player].health += LIFE_GAIN_CASTLE_DRACULA;
			//gv->data[player].first->revealed = true;
		}
		else if (node->place >= DOUBLE_BACK_1 && node->place <= DOUBLE_BACK_5) {
			if (findDBCity(node)->place == CASTLE_DRACULA || findDBCity(node)->place == TELEPORT) {
				gv->data[player].health += LIFE_GAIN_CASTLE_DRACULA;
				//gv->data[player].first->revealed = true;
			}

		}
	}
	else if (place >= DOUBLE_BACK_1 && place <= DOUBLE_BACK_5) {
		HistoryNode node = findDBCity(gv->data[PLAYER_DRACULA].first);

		if (node->place == CASTLE_DRACULA || node->place == TELEPORT) {
			gv->data[player].health += LIFE_GAIN_CASTLE_DRACULA;
			//gv->data[player].first->revealed = true;
		}
		else if (node->place == HIDE) {
			if (node->next->place == CASTLE_DRACULA || node->next->place == TELEPORT) {
				//gv->data[player].first->revealed = true;
				gv->data[player].health += LIFE_GAIN_CASTLE_DRACULA;
			}
		}
	}

	gv->score -= SCORE_LOSS_DRACULA_TURN;
}

void HvEvent(GameView gv, char* play, PlaceId place, int player)
{
	if (gv->data[player].health == 0) 
		gv->data[player].health = GAME_START_HUNTER_LIFE_POINTS;

	HistoryNode new = creatNode(place, false, 0, true);
	gv->data[player] = addToHistory(gv->data[player], new);

	int i = 3;
	for(i = 3; play[i] == 'T'; i++){
		deleteTraps(gv, place);
		gv->data[player].health -= LIFE_LOSS_TRAP_ENCOUNTER;
	}
	if (play[i] == 'V') {
		deleteVampire(gv);
		i++;
	}
	if (i <= 6) {
		if (play[i] == 'D') {
			gv->data[player].health -= LIFE_LOSS_DRACULA_ENCOUNTER;
			gv->data[PLAYER_DRACULA].health -= LIFE_LOSS_HUNTER_ENCOUNTER;
		}
	}
	if (gv->data[player].turn > 1 && gv->data[player].first->place == gv->data[player].first->next->place) {
		gv->data[player].health += LIFE_GAIN_REST;
		if (gv->data[player].health > GAME_START_HUNTER_LIFE_POINTS)
			gv->data[player].health = GAME_START_HUNTER_LIFE_POINTS;
	}
	if (gv->data[player].health <= 0) {
		gv->data[player].health = 0;
		gv->score -= SCORE_LOSS_HUNTER_HOSPITAL;
		gv->data[player].first->place = HOSPITAL_PLACE;
	}
		
	
}

Player getPlayer(char c){
	if (c == 'G') return PLAYER_LORD_GODALMING;
	if (c == 'S') return PLAYER_DR_SEWARD;
	if (c == 'H') return PLAYER_VAN_HELSING;
	if (c == 'M') return PLAYER_MINA_HARKER;
	else return PLAYER_DRACULA;		
}

void deleteTraps(GameView gv, PlaceId place) {

	HistoryNode curr = gv->data[PLAYER_DRACULA].first;
	for (int counter = 0; curr != NULL && counter < 6; counter++) {
		if (curr->place == place && curr->trapNumber == 1) {
			curr->trapNumber = 0;
			return; 
		}	
		curr = curr->next;
	}

	curr = gv->data[PLAYER_DRACULA].first;
	for (int counter = 0; curr != NULL && counter < 6; counter++) {
		if (curr->place == HIDE) {
			if (curr->next->place == place){
				if (!deleteLastTraps(curr->next, place, counter))
					curr->next->trapNumber = 0;
				/*
				if(curr->next->trapNumber == 1)
					curr->next->trapNumber = 0;
				else
					curr->trapNumber = 0;
				*/
				return; 
			} else if (curr->next->place >= DOUBLE_BACK_1 && curr->next->place <= DOUBLE_BACK_5) {
				HistoryNode node = findDBCity(curr->next);
				if (node->place == place) {
					if (!deleteLastTraps(node, place, counter))
						curr->next->trapNumber = 0;
					/*
					if (node->trapNumber == 1)
						node->trapNumber = 0;
					else if (curr->next->trapNumber == 1)
						curr->next->trapNumber = 0;
					else
						curr->trapNumber = 0;
					*/
					return; 
				}
			}
		}
		else if (curr->place >= DOUBLE_BACK_1 && curr->place <= DOUBLE_BACK_5) {
			HistoryNode node = findDBCity(curr);
			if (node->place == place) {
				if (!deleteLastTraps(node, place, counter))
					curr->trapNumber = 0;
				/*
				if (node->trapNumber == 1)
					node->trapNumber = 0;
				else
					curr->trapNumber = 0;
				*/
				return;
			} 
			else if (node->place == HIDE) {
				if (node->next->place == place) {
					if (!deleteLastTraps(node, place, counter))
						curr->trapNumber = 0;
					/*
					if (node->next->trapNumber == 1)
						node->next->trapNumber = 0;
					else if (node->trapNumber == 1)
						node->trapNumber = 0;
					else
						curr->trapNumber = 0;
					*/
					return;
				}
			}
		}
		else if (curr->place == TELEPORT)
			if (!deleteLastTraps(curr, place, counter))
				curr->trapNumber = 0;
		curr = curr->next;
	}
}

bool deleteLastTraps(HistoryNode node, PlaceId place, int num) {
	HistoryNode curr = node;
	HistoryNode result = NULL;
	for (int counter = num; counter < 6 && curr != NULL; counter++) {
		if (curr->place == place)
			result = curr;
		curr = curr->next;
	}

	if (result == NULL)
		return false;

	result->trapNumber = 0;
	return true;
}
/*
bool deleteHideTraps(HistoryNode node, PlaceId place, int num) {
	if (node->place == HIDE) {
		if (node->next->place == place) {
			if (!deleteLastTraps(node->next, place, num))
				node->next->trapNumber = 0;
			return true;
		}
	}
	return false;
}

bool deleteDBTraps(HistoryNode node, PlaceId place, int num) {
	if (node->place >= DOUBLE_BACK_1 && node->place <= DOUBLE_BACK_5) {
		HistoryNode curr = findDBCity(node);
		if (curr->place == place) {
			if (!deleteLastTraps(curr, place, num))
				node->trapNumber = 0;
			return true;
		}
	}
	return false;
}
*/
void deleteVampire(GameView gv) {

	HistoryNode curr = gv->data[PLAYER_DRACULA].first;
	for (int counter = 0; curr != NULL && counter < 5 && counter < gv->round; counter++) {
		if (curr->vampire){
			curr->vampire = false;
			return;
		}
		curr = curr->next;
	}
}
/*
void revealLocation(HistoryNode list, PlaceId place) {
	HistoryNode curr = list;
    int counter = 0;
    while((curr != NULL) && counter < 5) {
        if (curr->place == place) 
			curr->revealed = true;
		else if (curr->place == HIDE) {
			if (curr->next->place == place) {
				curr->revealed = true;
				curr->next->revealed = true;
			} else if (curr->next->place >= DOUBLE_BACK_1 && curr->next->place <= DOUBLE_BACK_5) {
				HistoryNode newCurr = findDBCity(curr->next);
				if(newCurr->place == place) {
					newCurr->revealed = true;
					curr->next->revealed = true;
					curr->revealed = true;
				}
			}
		} else if (curr->place >= DOUBLE_BACK_1 && curr->place <= DOUBLE_BACK_5) {
			HistoryNode newCurr = findDBCity(curr);
			if (newCurr->place == place) {
				newCurr->revealed = true;
				curr->revealed = true;
			} else if (newCurr->place == HIDE) {
				if (newCurr->next->place == place) {
					newCurr->next->revealed = true;
					newCurr->revealed = true;
					curr->revealed = true;
				}
			}
		}
        counter++;
        curr = curr->next;
    }
}
*/

void ListFree(HistoryNode node) {
    HistoryNode curr = node;
    HistoryNode next ;
    while (curr != NULL) {
        next = curr->next;
        free(curr);
        curr = next;
    }
}

HistoryNode creatNode(PlaceId place, bool vampire, int trap, bool revealed){
    HistoryNode new = malloc(sizeof(* new));
    new->place = place;
    new->vampire = vampire;
    new->trapNumber = trap;
    new->revealed = revealed; 
    new->next = NULL;
    return new;
}

HistoryNode copyNode(HistoryNode prevNode) { 
    HistoryNode new = malloc(sizeof(* new));
    new->place = prevNode->place;
    new->vampire = prevNode->vampire;
    new->trapNumber = prevNode->trapNumber;
    new->revealed = prevNode->revealed;
    new->next = NULL;
    return new;
}


// add the node to the head of the list  
playerData addToHistory(playerData data, HistoryNode newNode) {
    if(data.first == NULL){
        data.first = newNode;
        data.turn++;
        return data;
    } 
    newNode->next = data.first;
    data.first = newNode;
    data.turn++;
    return data;
}

// add the node to the end of the list
HistoryNode addToTail(HistoryNode list, HistoryNode newNode) {
    if (list == NULL) return newNode;

    HistoryNode curr = list;
    while(curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = newNode;
    return list;
}

HistoryNode getLastN(playerData data, int n){
    HistoryNode curr = data.first;
    if(curr == NULL) return NULL;
    HistoryNode newHead = copyNode(curr);
    HistoryNode newCurr = newHead;
    curr = curr->next;
    int i = 1;
    while((curr != NULL) && i < n){
        newCurr->next = copyNode(curr);
        newCurr = newCurr->next;
        curr = curr->next;
        i++;
    }
    return newHead;
}

HistoryNode findDBCity(HistoryNode DBnode) {
    int DBNumber = DBnode->place - DOUBLE_BACK_1;
    int i = 0;
    HistoryNode curr = DBnode;
		//printf("%d %d", node->place, node->next->place);
    while (curr != NULL && i <= DBNumber) {
        i++;
		//printf("%s\n",placeIdToName(curr->place));
        curr = curr->next;
    }

    return curr;
}

PlaceId * addPlace(PlaceId * place, int * num, PlaceId newPlace) {
	for(int i = 0; i < *num; i++) {
		if(place[i] == newPlace) return place;
	}
	place = realloc(place, (*num + 1) * sizeof(PlaceId));
	place[*num] = newPlace;
	*num = *num + 1;
	return place;
}