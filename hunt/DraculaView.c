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
// our own #includes
#include <string.h>
#include <limits.h>

#define MAXCHAR 8
#define MAXLINE 40

// OUR OWN STRUCTS
typedef struct QueueRep* Queue;

typedef struct QueueNode {
	int value;
	int round;
	struct QueueNode* next;
} QueueNode;

struct QueueRep {
	QueueNode* head; // ptr to first node
	QueueNode* tail; // ptr to last node
};

typedef struct ShortestPath {
	PlaceId src;
	int* dist;
	int* pred;
} ShortestPath;


struct draculaView {
	GameView gv;
	ShortestPath path[4];
};

static Queue newQueue(void); // create new empty queue, from lab07
static void dropQueue(Queue Q); // free memory used by queue, from lab07
static void QueueJoin(Queue Q, int it); // add int on queue, from lab07
static int QueueLeave(Queue Q); // remove item from queue, from lab07
static int QueueIsEmpty(Queue Q); // check for no items, from lab07


bool canGo(DraculaView dv, PlaceId place);
int getDoubleBackNum(DraculaView dv, PlaceId place);
bool canDoubleBack(DraculaView dv);
bool canHide(DraculaView dv);

////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

DraculaView DvNew(char* pastPlays, Message messages[]) {
	DraculaView new_dv = malloc(sizeof(DraculaView));
	if (new_dv == NULL) {
		fprintf(stderr, "Couldn't allocate DraculaView\n");
		exit(EXIT_FAILURE);
	}

	new_dv->gv = GvNew(pastPlays, messages);

	return new_dv;
}

void DvFree(DraculaView dv) {
	GvFree(dv->gv);
	free(dv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round DvGetRound(DraculaView dv) {
	return GvGetRound(dv->gv);
}

int DvGetScore(DraculaView dv) {
	return GvGetScore(dv->gv);
}

int DvGetHealth(DraculaView dv, Player player) {
	return GvGetHealth(dv->gv, player);
}

PlaceId DvGetPlayerLocation(DraculaView dv, Player player) {
	return GvGetPlayerLocation(dv->gv, player);
}

PlaceId DvGetVampireLocation(DraculaView dv) {
	return GvGetVampireLocation(dv->gv);
}

PlaceId* DvGetTrapLocations(DraculaView dv, int* numTraps) {
	*numTraps = 0;
	return GvGetTrapLocations(dv->gv, numTraps);
}

////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId* DvGetValidMoves(DraculaView dv, int* numReturnedMoves) {	
	*numReturnedMoves = 0;
	// get all the locations in the history
	PlaceId lastLocation = GvGetPlayerLocation(dv->gv, PLAYER_DRACULA);
	if (lastLocation == NOWHERE) // Dracula has no movement yet
		return NULL;
	
	int numReachable;
	// get all the reachable places for Dracula
    PlaceId * reachable = GvGetReachable(dv->gv, PLAYER_DRACULA, 
                                         GvGetRound(dv->gv), lastLocation, 
                                         &numReachable);
	PlaceId * move = NULL;
	// search through all the reachable locations
	for (int i = 0; i < numReachable; i++) {
		if (canGo(dv, reachable[i])) { // dracula can directly move to that location
			move = realloc(move, (*numReturnedMoves + 1) * sizeof(PlaceId));
			move[*numReturnedMoves] = reachable[i];
			*numReturnedMoves = *numReturnedMoves + 1;
		} else if (getDoubleBackNum(dv, reachable[i]) > 0) { 
			// dracula can double back to that location
			int DBnumber = getDoubleBackNum(dv, reachable[i]);
			// if there is no DOUBLE_BACK_# in the trail and the last move was HIDE
			// Dracula can go either DOUBLE_BACK_1 or DOUBLE_BACK_2
			if (DBnumber == 2) { 
				bool canFree;
				int numReturned = 0;
				// get the last move in the history
				PlaceId * lastMove = GvGetLastMoves(dv->gv, PLAYER_DRACULA, 1, 
				                                    &numReturned, &canFree);
				if (numReturned > 0){
					if (lastMove[0] == HIDE) { 
						// if the last move is HIDE, add DOUBLE_BACK_1 to moves
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

PlaceId* DvWhereCanIGo(DraculaView dv, int* numReturnedLocs) {
	return DvWhereCanIGoByType(dv, true, true, numReturnedLocs);
}

PlaceId* DvWhereCanIGoByType(DraculaView dv, bool road, bool boat,
	int* numReturnedLocs) {
	*numReturnedLocs = 0;
	// get current location
	PlaceId lastLocation = GvGetPlayerLocation(dv->gv, PLAYER_DRACULA); 
	if (lastLocation == NOWHERE) // Dracula has no movement yet
		return NULL;
    
	int numReachable;
	// get all the reachable places for Dracula
	PlaceId * reachable = GvGetReachableByType(dv->gv, PLAYER_DRACULA,
	                                           GvGetRound(dv->gv), lastLocation, 
	                                           road, false, boat, &numReachable);
	PlaceId * place = NULL;
	// search through all the reachable locations
	for (int i = 0; i < numReachable; i++) {
		if (canGo(dv, reachable[i])) { 
			// dracula can directly move to that location
			place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
			place[*numReturnedLocs] = reachable[i];
			*numReturnedLocs = *numReturnedLocs + 1;
		} else if (getDoubleBackNum(dv, reachable[i]) > 0) { 
			// dracula can double back to that location
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
	int* numReturnedLocs) {	
	return DvWhereCanTheyGoByType(dv, player, true, true, true, numReturnedLocs);
}

PlaceId* DvWhereCanTheyGoByType(DraculaView dv, Player player,
	bool road, bool rail, bool boat,
	int* numReturnedLocs) {	
	*numReturnedLocs = 0;
	if (player == PLAYER_DRACULA) // player is dracula, call whereCanIMove
		return DvWhereCanIGoByType(dv, road, boat, numReturnedLocs);

	PlaceId lastLocation = GvGetPlayerLocation(dv->gv, player);
	if (lastLocation == NOWHERE) // the player has no movement yet
		return NULL;

	Player currPlayer = GvGetPlayer(dv->gv); // get the current player
	
	if (currPlayer < player) // the player haven't made a move in the current turn
		return GvGetReachableByType(dv->gv, player, GvGetRound(dv->gv), 
		                            lastLocation, road, rail, 
		                            boat, numReturnedLocs);
	else // the player alreade made a move in the current turn
		return GvGetReachableByType(dv->gv, player, GvGetRound(dv->gv) + 1, 
		                            lastLocation, road, rail, 
		                            boat, numReturnedLocs);
}

////////////////////////////////////////////////////////////////////////
// our own interface functions

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
		else if (list[i] >= DOUBLE_BACK_1 && list[i] <= DOUBLE_BACK_5 
		         && list[i] - DOUBLE_BACK_1 < num) {
			if (list[list[i] - DOUBLE_BACK_1] == place) // already double back before
				go = false;
		} else if (i + 1 == 6 && num >= 6) {
			if (list[i] == HIDE && list[i + 1] == place) // already hide before
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

// Queue operations
// create new empty Queue
Queue newQueue(void) {
	Queue new = malloc(sizeof * new);
	new->head = NULL;
	new->tail = NULL;
	return new;
}

// free the Queue
void dropQueue(Queue Q) {
	assert(Q != NULL);
	for (QueueNode* curr = Q->head, *next; curr != NULL; curr = next) {
		next = curr->next;
		free(curr);
	}
	free(Q);
}

// add item at end of Queue
void QueueJoin(Queue Q, int it) {
	assert(Q != NULL);

	QueueNode * new = malloc(sizeof * new);
	assert(new != NULL);
	new->value = it;
	new->next = NULL;

	if (Q->head == NULL)
		Q->head = new;
	if (Q->tail != NULL)
		Q->tail->next = new;
	Q->tail = new;
}

// remove item from front of Queue
int QueueLeave(Queue Q) {
	assert(Q != NULL);
	assert(Q->head != NULL);
	int it = Q->head->value;
	QueueNode * old = Q->head;
	Q->head = old->next;
	if (Q->head == NULL)
		Q->tail = NULL;
	free(old);
	return it;
}

// check for no items
int QueueIsEmpty(Queue Q) {
	return (Q->head == NULL);
}

PlaceId* DvGetShortestPathTo(DraculaView dv, Player hunter, PlaceId dest, int* pathLength) {
	PlaceId src = DvGetPlayerLocation(dv, hunter);

	PlaceId* path = NULL;
	*pathLength = 0;
	if (src == dest) { // if the hunter is at that location
		path = malloc((*pathLength + 1) * sizeof(PlaceId));
		path[0] = src;
		return path;
	}
	int w;

	if (src != dv->path[hunter].src) { // if HvGetShortestPathTo haven't been called brfore
		// initialise values
		dv->path[hunter].src = src;
		Round round = DvGetRound(dv);
		int* dist = malloc((MAX_REAL_PLACE + 1) * sizeof(int));
		int* pred = malloc((MAX_REAL_PLACE + 1) * sizeof(int));
		*pathLength = 0;
		for (int i = 0; i < MAX_REAL_PLACE + 1; i++) {
			dist[i] = INT_MAX;  //infinity
			pred[i] = -1;
		}
		// bfs to the map
		Queue q = newQueue();
		dist[src] = 0;
		pred[src] = src;
		QueueJoin(q, src);
		int v;
		while (!QueueIsEmpty(q)) {
			w = QueueLeave(q);
			int numReturnedLocs;
			PlaceId* reachable = GvGetReachable(dv->gv, hunter,
				round + dist[w],
				w, &numReturnedLocs);
			for (int i = 0; i < numReturnedLocs; i++) {
				v = reachable[i];
				if (w != v && pred[v] == -1) {
					pred[v] = w;
					dist[v] = dist[w] + 1;
					QueueJoin(q, v);
				}
			}
			free(reachable);
		}
		dropQueue(q);
		// store the array into HunterView
		dv->path[hunter].dist = dist;
		dv->path[hunter].pred = pred;
	}
	// store the paths into array
	*pathLength = dv->path[hunter].dist[dest];
	path = malloc((*pathLength) * sizeof(PlaceId));
	w = dest;
	path[*pathLength - 1] = dest;
	for (int i = *pathLength - 2; i >= 0; i--) {
		path[i] = dv->path[hunter].pred[w];
		w = dv->path[hunter].pred[w];
	}
	return path;
}