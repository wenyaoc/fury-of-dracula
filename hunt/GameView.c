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
// our own #includes
#include <string.h>

#define MAXPLAY 8
#define MAXLINE 40

// OUR OWN STRUCTS
typedef struct historyNode *HistoryNode;

struct historyNode {
	PlaceId place; // move
	bool vampire; 
	bool trap;
	HistoryNode next;
};

typedef struct _playerData {
	Player player; // name of the player
	int health; // current health for the player
	int turn; // total number in history
	HistoryNode first; // the first node in the history list
} playerData;


struct gameView {
	Round round; // current round number
	int score; // current score
	playerData data[5]; // data[0-3]:hunter  data[4]:dracula
};

Player getPlayer(char c);
void trailLoad(GameView gv, char * play);
void draculaEvent(GameView gv, char* play, PlaceId place, int player);
void hunterEvent(GameView gv, char* play, PlaceId place, int player);
void deleteTraps(GameView gv, PlaceId place);
void deleteVampire(GameView gv);
bool deleteLastTraps(HistoryNode node, PlaceId place, int num);

void ListFree(HistoryNode node);
HistoryNode creatNode(PlaceId place, bool vampire, bool trap);
playerData addToHistory(playerData data, HistoryNode newNode);
HistoryNode findDBCity(HistoryNode DBnode);
PlaceId * addPlace(PlaceId * place, int * num, PlaceId newPlace);
////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

GameView GvNew(char* pastPlays, Message messages[]) {
	GameView new = malloc(sizeof(*new));
	if (new == NULL) {
		fprintf(stderr, "Couldn't allocate GameView!\n");
		exit(EXIT_FAILURE);
	}
	// calculate the total turns for all the player
	int totalLength = strlen(pastPlays);
	// calculate the current round number
	new->round = (totalLength + 1) / MAXLINE; 
	new->score = GAME_START_SCORE; // initialise score

	// initialise playerData
	for (int i = 0; i < 5; i++) {
		new->data[i].player = i;
		new->data[i].health = GAME_START_HUNTER_LIFE_POINTS;
		new->data[i].turn = 0;
		new->data[i].first = NULL;
	}
	new->data[PLAYER_DRACULA].health = GAME_START_BLOOD_POINTS;

	if (totalLength + 1 < MAXPLAY) // no movement in pastPlays
		return new;
	
	// tokenise each play, and store into history
	char* play = calloc(MAXPLAY, sizeof(char));
	for (int i = 0; i <= totalLength; i++) {
		play[i % 8] = pastPlays[i];
		if ((i % 8) == 7) {
			trailLoad(new, play);
		}
	}

	//if(GvGetPlayer(new) == PLAYER_LORD_GODALMING || GvGetHealth(new, PLAYER_DRACULA) <= 0) {
	//	printf("round = %d\nscore = %d\nhunter = %d %d %d %d, Dracula = %d\n", GvGetRound(new), GvGetScore(new), GvGetHealth(new, 0), GvGetHealth(new, 1),GvGetHealth(new, 2),GvGetHealth(new, 3),GvGetHealth(new, PLAYER_DRACULA));
	//}
	free(play);
	return new;
}

void GvFree(GameView gv) {	
	for (int i = 0; i < 5; i++) {
		ListFree(gv->data[i].first);
	}
	free(gv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round GvGetRound(GameView gv) {
	return gv->round;
}

Player GvGetPlayer(GameView gv) {	
	for (int i = 0; i < 5; i++) {
		if(gv->data[i].turn < gv->round + 1) 
			return i;
	}
	return 0;
}

int GvGetScore(GameView gv) {	
	return gv->score;
}

int GvGetHealth(GameView gv, Player player) {	
	return gv->data[player].health < 0 ? 0 : gv->data[player].health;
}

PlaceId GvGetPlayerLocation(GameView gv, Player player) {	
	if (gv->data[player].turn == 0) // no movement has been made
		return NOWHERE;
	else if (player != PLAYER_DRACULA) // player is hunter, return the place
		return gv->data[player].first->place;
	else {
		HistoryNode node = gv->data[player].first;

		if (node->place == TELEPORT) // the move is teleport
			return CASTLE_DRACULA;
		else if (placeIsLand(node->place)) 
			return node->place;
		else if (placeIsSea(node->place)) 
			return node->place;
		else if (node->place == HIDE) { // the move is HIDE
			HistoryNode real = node->next;
			if (!placeIsReal(real->place)) // the move is DOUBLE_BACK# + HIDE
				real = findDBCity(real);
			if (placeIsLand(real->place)) 
				return real->place;
			else if (placeIsSea(node->next->place)) 
				return real->place;	
		} else {  // the move is DOUBLE_BACK#
			HistoryNode real = findDBCity(node);
			while (!placeIsReal(real->place)) {
				if (real->place == HIDE) // the move is HIDE + DOUBLE_BACK# 
					real = real->next;
				if (real->place == TELEPORT)
					return CASTLE_DRACULA;
				if (!placeIsReal(real->place))
					real = findDBCity(real);
			}
			if (real->place == TELEPORT)
				return CASTLE_DRACULA;
			if (placeIsLand(real->place)) 
				return real->place;
			else if (placeIsSea(real->place)) 
				return real->place;
		}
	}
	return UNKNOWN_PLACE;
}

PlaceId GvGetVampireLocation(GameView gv) {	
	if (gv->data[PLAYER_DRACULA].turn == 0)
		return NOWHERE;
	HistoryNode curr = gv->data[PLAYER_DRACULA].first;
	int counter = 0;
	while (curr != NULL && counter < 6) {
		if (curr->vampire == true) 
			return curr->place;
		counter++;
		curr = curr->next;
	}
	return NOWHERE; // there is no vampire
}

PlaceId* GvGetTrapLocations(GameView gv, int* numTraps) {	
	*numTraps = 0;
	if (gv->data[PLAYER_DRACULA].turn == 0) return NULL;
	HistoryNode curr = gv->data[PLAYER_DRACULA].first;

	int counter = 0;
	PlaceId * place = NULL;
	while (curr != NULL && counter < 6 && counter < gv->round) {
		if (curr->trap > 0) {
			place = realloc(place, (*numTraps + 1) * sizeof(PlaceId));
			// if place is not real, find the rel place name
			if (curr->place == HIDE) { // the move is HIDE
				// the move is DOUBLE_BACK# + HIDE
				if (curr->next->place >= DOUBLE_BACK_1 
				    && curr->next->place <= DOUBLE_BACK_5)
					place[*numTraps] = findDBCity(curr->next)->place;
				else
					place[*numTraps] = curr->next->place;

			} else if (curr->place >= DOUBLE_BACK_1 
			           && curr->place <= DOUBLE_BACK_5) { 
				// the move is DOUBLE_BACK#
				HistoryNode realPlace = findDBCity(curr);
				if (realPlace->place == HIDE) // the move is HIDE + DOUBLE_BACK#
					place[*numTraps] = realPlace->next->place;
				else
					place[*numTraps] = realPlace->place;
			}
			else if (curr->place == TELEPORT) // the move is TELEPORT
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
                          int *numReturnedMoves, bool *canFree) {	
	int numMoves = gv->data[player].turn; // get the total turn
	return GvGetLastMoves(gv, player, numMoves, numReturnedMoves, canFree);
}

PlaceId* GvGetLastMoves(GameView gv, Player player, int numMoves,
                        int* numReturnedMoves, bool* canFree) {	
	*canFree = true;
	int totalNum = gv->data[player].turn;
	if (totalNum == 0) return NULL; // no move has been done yet
	if (totalNum < numMoves) // there dont have enough moves in history
		*numReturnedMoves = totalNum;
	else
		*numReturnedMoves = numMoves;

	PlaceId * move = malloc(*numReturnedMoves * sizeof(PlaceId));

	// stort the move into the array
	HistoryNode curr = gv->data[player].first;
	for (int i = *numReturnedMoves - 1; i >= 0; i--) {
		move[i] = curr->place;
		curr = curr->next;
	}
	return move;

}

PlaceId *GvGetLocationHistory(GameView gv, Player player,
                              int *numReturnedLocs, bool *canFree) {	
	int numLocs = gv->data[player].turn;
	return GvGetLastLocations(gv, player, numLocs, numReturnedLocs, canFree);
}

PlaceId* GvGetLastLocations(GameView gv, Player player, int numLocs,
                            int* numReturnedLocs, bool* canFree) {
	PlaceId* place = GvGetLastMoves(gv, player, numLocs, numReturnedLocs, canFree);
	
	HistoryNode curr = gv->data[player].first;
	for (int i = (*numReturnedLocs - 1); i >= 0; i--) {
		if (place[i] == HIDE) { // if the move is HIDE
			// if the move is DOUBLE_BACK# + HIDE
			if (curr->next->place >= DOUBLE_BACK_1 
			    && curr->next->place <= DOUBLE_BACK_5)
				place[i] = findDBCity(curr->next)->place;
			else
				place[i] = curr->next->place;
		} else if (curr->place >= DOUBLE_BACK_1 && curr->place <= DOUBLE_BACK_5) {
			HistoryNode newCurr = findDBCity(curr);
			// if the move is HIDE + DOUBLE_BACK#
			if (newCurr->place == HIDE) 
				place[i] = newCurr->next->place;
			else
				place[i] = newCurr->place;
		}
		// if the move is TELEPORT
		if (place[i] == TELEPORT)
			place[i] = CASTLE_DRACULA;

		curr = curr->next;
	}
	return place;
}

////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId *GvGetReachable(GameView gv, Player player, Round round,
                        PlaceId from, int *numReturnedLocs) {
	return GvGetReachableByType(gv, player, round, from, 
	                            true, true, true, numReturnedLocs);
}

PlaceId *GvGetReachableByType(GameView gv, Player player, Round round,
                              PlaceId from, bool road, bool rail,
                              bool boat, int *numReturnedLocs) {	
	*numReturnedLocs = 0;
	Map m = MapNew(); // creat a map
	ConnList connection;
	
	connection = MapGetConnections(m, from);
	ConnList curr = connection;
	PlaceId * place = NULL;
	if (curr == NULL) return NULL;
	if (player == PLAYER_DRACULA) {  // If  the  given player is Dracula
		while (curr != NULL) {
			if ((curr->type == ROAD && road == true) 
			     || (curr->type == BOAT && boat == true)) {
				// make sure the place is not St Joseph and St Mary.
				if (curr->p != HOSPITAL_PLACE) {  
					place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
					place[*numReturnedLocs] = curr->p;
					*numReturnedLocs = *numReturnedLocs + 1;
				}
			}
			curr = curr->next;
		}
		// add the current place
		place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId)); 
		place[*numReturnedLocs] = from;
		*numReturnedLocs = *numReturnedLocs + 1;

	} else { // If  the  given  player is a hunter
		int num = round + player; // calculate the num
		while (curr != NULL) {
			if ((curr->type == ROAD && road) || (curr->type == BOAT && boat)) {
				place = addPlace(place, numReturnedLocs, curr->p);
			} else if (curr->type == RAIL && rail && num % 4 != 0) {
				place = addPlace(place, numReturnedLocs, curr->p);
				if (num % 4 > 1) {  
					// if num % 4 == 2 or num % 4 == 3, find more connections
					ConnList connection2 = MapGetConnections(m, curr->p);
					ConnList curr2 = connection2;
					while (curr2 != NULL) {
						// find all the RAIL conntctions
						if (curr2->type == RAIL) { 
							place = addPlace(place, numReturnedLocs, curr2->p);
							if (num % 4 == 3) { 
								// if num % 4 == 3, find more connections
								ConnList connection3 = MapGetConnections(m, curr2->p);
								ConnList curr3 = connection3;
								while (curr3 != NULL) {
									// find all the RAIL conntctions
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
	// add the current place to the array
	place = addPlace(place, numReturnedLocs, from); 
	return place;
}

////////////////////////////////////////////////////////////////////////
// our own interface functions

// add a play to GameView
// input: GameView, a hunter play or a dracula play
void trailLoad(GameView gv, char* play) {

	Player player = getPlayer(play[0]); // get the player name
	char abbrev[3];
	abbrev[0] = play[1];
	abbrev[1] = play[2];
	abbrev[2] = '\0';
	PlaceId place = placeAbbrevToId(abbrev); // get the location number
	if (player == PLAYER_DRACULA)
		draculaEvent(gv, play, place, player);
	else
		hunterEvent(gv, play, place, player);
}

// add a dracula's play to GameView
// input: GameView, a dracula's play, place, PLAYER_DRACULA
void draculaEvent(GameView gv, char* play, PlaceId place, int player) {
	bool vampire = false;
	int trap = false;
	if (play[3] == 'T')
		trap = true;
	if (play[4] == 'V')
		vampire = true;

	// create a new node and add to history
	HistoryNode new = creatNode(place, vampire, trap);
	gv->data[player] = addToHistory(gv->data[player], new);

	if (play[5] == 'M') { // trap leave the trail
		HistoryNode curr = gv->data[PLAYER_DRACULA].first;
		for (int counter = 0; curr != NULL && counter < 6; counter++) {
			curr = curr->next;
		}
		curr->trap = false; // delete the trap
	}
	else if (play[5] == 'V') { //vampire matured
		gv->score -= SCORE_LOSS_VAMPIRE_MATURES;
	}

	if (placeIsSea(place)) // dracula finish this turn in sea
		gv->data[player].health -= LIFE_LOSS_SEA;
	else if (place >= DOUBLE_BACK_1 && place <= DOUBLE_BACK_5) {
		HistoryNode node = findDBCity(gv->data[PLAYER_DRACULA].first); 
		if (placeIsSea(node->place)) // dracula double back to sea
			gv->data[player].health -= LIFE_LOSS_SEA;
	}
	if (place == CASTLE_DRACULA || place == TELEPORT) { // dracula is at castle
		gv->data[player].health += LIFE_GAIN_CASTLE_DRACULA ;
	} else if (place == HIDE) { // dracula hide in this turn
		HistoryNode node = gv->data[PLAYER_DRACULA].first->next;
        // hide at castle
		if (node->place == CASTLE_DRACULA || node->place == TELEPORT) 
			gv->data[player].health += LIFE_GAIN_CASTLE_DRACULA;
		else if (node->place >= DOUBLE_BACK_1 && node->place <= DOUBLE_BACK_5) {
			// hide and double back at castle
			if (findDBCity(node)->place == CASTLE_DRACULA 
			    || findDBCity(node)->place == TELEPORT)
				gv->data[player].health += LIFE_GAIN_CASTLE_DRACULA;
		}
	} else if (place >= DOUBLE_BACK_1 && place <= DOUBLE_BACK_5) { 
	    // dracula double back in this turn
		HistoryNode node = findDBCity(gv->data[PLAYER_DRACULA].first);
        // double back to castle
		if (node->place == CASTLE_DRACULA || node->place == TELEPORT) 
			gv->data[player].health += LIFE_GAIN_CASTLE_DRACULA;
		else if (node->place == HIDE) { //double back and hide at castle
			if (node->next->place == CASTLE_DRACULA || node->next->place == TELEPORT) 
				gv->data[player].health += LIFE_GAIN_CASTLE_DRACULA;
		}
	}
	gv->score -= SCORE_LOSS_DRACULA_TURN;
}

// add a hunter's play to GameView
// input: GameView, a hunter play, place, hunter's name
void hunterEvent(GameView gv, char* play, PlaceId place, int player) {
    // if the hunter die last turn, set the health back
	if (gv->data[player].health == 0) 
		gv->data[player].health = GAME_START_HUNTER_LIFE_POINTS;

	// create a new node and add to history
	HistoryNode new = creatNode(place, false, false);
	gv->data[player] = addToHistory(gv->data[player], new);

	int i = 3;
	for(i = 3; play[i] == 'T'; i++) { //encounters traps
		deleteTraps(gv, place);
		gv->data[player].health -= LIFE_LOSS_TRAP_ENCOUNTER;
	}
	if (play[i] == 'V') { //encounters vampire
		deleteVampire(gv);
		i++;
	}
	if (i <= 6) { // encounters dracula
		if (play[i] == 'D') {
			gv->data[player].health -= LIFE_LOSS_DRACULA_ENCOUNTER;
			gv->data[PLAYER_DRACULA].health -= LIFE_LOSS_HUNTER_ENCOUNTER;
		}
	}
	if (gv->data[player].health <= 0) { // the hunter die, teleport to hospital
		gv->data[player].health = 0;
		gv->score -= SCORE_LOSS_HUNTER_HOSPITAL;
		gv->data[player].first->place = HOSPITAL_PLACE;
	} else if (gv->data[player].turn > 1 
	    && gv->data[player].first->place == gv->data[player].first->next->place) {
		gv->data[player].health += LIFE_GAIN_REST;
		// if health > 9, set back to 9
		if (gv->data[player].health > GAME_START_HUNTER_LIFE_POINTS) 
			gv->data[player].health = GAME_START_HUNTER_LIFE_POINTS;
		
	}
}

// get the player id
// input: the character representing the player
// output: the player id
Player getPlayer(char c) {
	if (c == 'G') return PLAYER_LORD_GODALMING;
	if (c == 'S') return PLAYER_DR_SEWARD;
	if (c == 'H') return PLAYER_VAN_HELSING;
	if (c == 'M') return PLAYER_MINA_HARKER;
	else return PLAYER_DRACULA;		
}

// delete a trap after hunter encountered that trap
// input: GameView, placeid
void deleteTraps(GameView gv, PlaceId place) {

	HistoryNode curr = gv->data[PLAYER_DRACULA].first;
	for (int counter = 0; curr != NULL && counter < 6; counter++) {
		// exist a real place maches the encountered place
		if (curr->place == place && curr->trap == true) { 
			curr->trap = false;
			return; 
		}
		curr = curr->next;
	}
	// the place is not real
	curr = gv->data[PLAYER_DRACULA].first;
	for (int counter = 0; curr != NULL && counter < 6; counter++) {
		if (curr->place == HIDE) { // the move is HIDE
			if (curr->next->place == place){ 
			    // delete the trap in the real place
				if (!deleteLastTraps(curr->next, place, counter)) 
				    // delete the trap in the current place
					curr->next->trap = false; 
				return; 
			} else if (curr->next->place >= DOUBLE_BACK_1 
			           && curr->next->place <= DOUBLE_BACK_5) {
				// the move is double back + hide
				HistoryNode node = findDBCity(curr->next);
				if (node->place == place) {
				    // delete the trap in the real place
					if (!deleteLastTraps(node, place, counter)) 
					    // delete the trap in the current place
						curr->next->trap = false; 
					return; 
				}
			}
		} else if (curr->place >= DOUBLE_BACK_1 && curr->place <= DOUBLE_BACK_5) { 
			// the move is Double Back
			HistoryNode node = findDBCity(curr);
			if (node->place == place) {
			    // delete the trap in the real place
				if (!deleteLastTraps(node, place, counter)) 
					curr->trap = false;  // delete the trap in the current place
				return;
			} 
			else if (node->place == HIDE) { // the move is hide + double back
				if (node->next->place == place) {
				    // delete the trap in the real place
					if (!deleteLastTraps(node, place, counter)) 
						curr->trap = false; // delete the trap in the current place
					return;
				}
			}
		} else if (curr->place == TELEPORT) // the move is teleport
		    // delete the trap in the real place
			if (!deleteLastTraps(curr, place, counter)) 
				curr->trap = false; // delete the trap in the current place
		curr = curr->next;
	}
}

// delete the trap for the original node that have been double back or hide,
// input: the character representing the player
// output: true if the trap is successgully deleted, false if the trap is already vanish
bool deleteLastTraps(HistoryNode node, PlaceId place, int num) {
	HistoryNode curr = node;
	HistoryNode result = NULL;
	for (int counter = num; counter < 6 && curr != NULL; counter++) {
		if (curr->place == place)
			result = curr;
		curr = curr->next;
	}

	if (result == NULL) // the place alreade leave the trail
		return false;

	result->trap = false; // set the trap to false (delete the trap)
	return true;
}


// delete the vampire
// input: GameView
void deleteVampire(GameView gv) {
	HistoryNode curr = gv->data[PLAYER_DRACULA].first;
	for (int counter = 0; curr != NULL && counter < 6 
	     && counter < gv->round; counter++) {
		if (curr->vampire){ 
			curr->vampire = false;
			return;
		}
		curr = curr->next;
	}
}


// finde the original place that dracula double back to
// input: the HistoryNode that the move is double back
// output: the HistoryNode for the original place
HistoryNode findDBCity(HistoryNode DBnode) {
    int DBNumber = DBnode->place - DOUBLE_BACK_1;
    int i = 0;
    HistoryNode curr = DBnode;
    while (curr != NULL && i <= DBNumber) {
        i++;
        curr = curr->next;
    }

    return curr;
}

// add a place to a given PlaceId array if that place haven't been added before
// input: PlaceId array, total number in array, new place
// output: the new PlaceId array
PlaceId * addPlace(PlaceId * place, int * num, PlaceId newPlace) {
	for(int i = 0; i < *num; i++) {
		if(place[i] == newPlace) return place;
	}
	place = realloc(place, (*num + 1) * sizeof(PlaceId));
	place[*num] = newPlace;
	*num = *num + 1;
	return place;
}


////////////////////////////////////////////////////////////////////////
// link list operations

// free the list
void ListFree(HistoryNode node) {
    HistoryNode curr = node;
    HistoryNode next ;
    while (curr != NULL) {
        next = curr->next;
        free(curr);
        curr = next;
    }
}

// create a new node
HistoryNode creatNode(PlaceId place, bool vampire, bool trap) {
    HistoryNode new = malloc(sizeof(* new));
    new->place = place;
    new->vampire = vampire;
    new->trap = trap;
    new->next = NULL;
    return new;
}


// add the node to history 
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
