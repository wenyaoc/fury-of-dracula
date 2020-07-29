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
// our own #includes 
#include <limits.h>
#include <string.h>

#define MAXCHAR 8
#define MAXLINE 40
// OUR OWN STRUCTS
typedef struct QueueRep *Queue;

typedef struct QueueNode {
	int value;
	int round;
	struct QueueNode *next;
} QueueNode;

struct QueueRep {
	QueueNode *head; // ptr to first node
	QueueNode *tail; // ptr to last node
};

typedef struct ShortestPath {
	PlaceId src;
	int * dist;
	int * pred;
} ShortestPath;


struct hunterView {
	GameView gv;
	ShortestPath path[4];
};


Queue newQueue (void); // create new empty queue, from lab07
void dropQueue (Queue Q); // free memory used by queue, from lab07
void QueueJoin (Queue Q, int it); // add int on queue, from lab07
int QueueLeave (Queue Q); // remove item from queue, from lab07
int QueueIsEmpty (Queue Q); // check for no items, from lab07

////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

HunterView HvNew(char *pastPlays, Message messages[]) {
	HunterView new = malloc(sizeof(*new));
	if (new == NULL) {
		fprintf(stderr, "Couldn't allocate HunterView!\n");
		exit(EXIT_FAILURE);
	}
	new->gv = GvNew(pastPlays, messages);
	for (int i = 0; i < 4; i++) {
		// set src to NOWHERE if shortest haven't been called
		new->path[i].src = NOWHERE; 
		new->path[i].dist = NULL;
		new->path[i].pred = NULL;
	}
	return new;
}

void HvFree(HunterView hv) {
	
	GvFree(hv->gv);
	for (int i = 0; i < 4; i++) {
		if (hv->path[i].src != NOWHERE) {
			free(hv->path[i].pred);
			free(hv->path[i].dist);
		}
	}
	free(hv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round HvGetRound(HunterView hv) {	
	return GvGetRound(hv->gv);
}

Player HvGetPlayer(HunterView hv) {
	return GvGetPlayer(hv->gv);
}

int HvGetScore(HunterView hv) {
	return GvGetScore(hv->gv);
}

int HvGetHealth(HunterView hv, Player player) {	
	return GvGetHealth(hv->gv, player);
}

PlaceId HvGetPlayerLocation(HunterView hv, Player player) {
	return GvGetPlayerLocation(hv->gv, player);
}

PlaceId HvGetVampireLocation(HunterView hv) {
	return GvGetVampireLocation(hv->gv);
}

////////////////////////////////////////////////////////////////////////
// Utility Functions

PlaceId HvGetLastKnownDraculaLocation(HunterView hv, Round *round) {
	int number;
	bool canFree;
	PlaceId * place = GvGetLocationHistory(hv->gv, PLAYER_DRACULA, 
                                           &number, &canFree);
	PlaceId LastKnownPlace;
	int known = 0;
	for (int i = number - 1; i >= 0; i--) {
		if(placeIsReal(place[i])) { // find a revealed location
			*round = i;
			LastKnownPlace = place[i];
			known = 1;
			break;
		}
	}
	if (canFree)
		free(place);
	if (!known) return NOWHERE; // Dracula's location has never been reveled
                             
	return LastKnownPlace;
}

PlaceId *HvGetShortestPathTo(HunterView hv, Player hunter, PlaceId dest, int *pathLength) {
	PlaceId src = HvGetPlayerLocation(hv, hunter);

	PlaceId * path = NULL;
	*pathLength = 0;
	if (src == dest) { // if the hunter is at that location
		path = malloc((*pathLength + 1) * sizeof(PlaceId));
		path[0] = src;
		return path;
	}
	int w;

	if (src != hv->path[hunter].src) { // if HvGetShortestPathTo haven't been called brfore
		// initialise values
		hv->path[hunter].src = src;
		Round round = HvGetRound(hv);
		int * dist = malloc((MAX_REAL_PLACE + 1) * sizeof(int));
		int * pred = malloc((MAX_REAL_PLACE + 1) * sizeof(int));
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
			PlaceId * reachable = GvGetReachable(hv->gv, hunter, round + dist[w], 
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
		hv->path[hunter].dist = dist;
		hv->path[hunter].pred = pred;
	} 
	// store the paths into array
	*pathLength = hv->path[hunter].dist[dest];
	path = malloc((*pathLength) * sizeof(PlaceId));
	w = dest;
	path[*pathLength - 1] = dest;
	for (int i = *pathLength - 2; i >= 0; i--) {
		path[i] = hv->path[hunter].pred[w];
		w = hv->path[hunter].pred[w];
	}
	return path;
}

////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId *HvWhereCanIGo(HunterView hv, int *numReturnedLocs) {
	return HvWhereCanIGoByType(hv, true, true, true, numReturnedLocs);
}

PlaceId *HvWhereCanIGoByType(HunterView hv, bool road, bool rail,
                             bool boat, int *numReturnedLocs) {
	*numReturnedLocs = 0;
	Player player = GvGetPlayer(hv->gv); // get current player
	PlaceId from = GvGetPlayerLocation(hv->gv, player);
	if (from == NOWHERE) return NULL; // hunter has no movement yet
	return GvGetReachableByType(hv->gv, player, GvGetRound(hv->gv) + player, from, 
                                road, rail, boat, numReturnedLocs);
}

PlaceId *HvWhereCanTheyGo(HunterView hv, Player player,
                          int *numReturnedLocs) {	
	return HvWhereCanTheyGoByType(hv, player, true, true, true, numReturnedLocs);
}

PlaceId *HvWhereCanTheyGoByType(HunterView hv, Player player,
                                bool road, bool rail, bool boat,
                                int *numReturnedLocs) {	
	if (player != PLAYER_DRACULA) { // player is a hunter
		*numReturnedLocs = 0;
		PlaceId from = GvGetPlayerLocation(hv->gv, player);
		if (from == NOWHERE) return NULL; // hunter has no movement yet
		
		Player currPlayer = GvGetPlayer(hv->gv);
		if (currPlayer <= player) { // haven't made a movement in the current turn
			return GvGetReachableByType(hv->gv, player, GvGetRound(hv->gv), from, 
                                        road, rail, boat, numReturnedLocs);
		} else { // the player already made a movement in the current turn
			return GvGetReachableByType(hv->gv, player, GvGetRound(hv->gv) + 1, 
			                            from, road, rail, boat, numReturnedLocs);
		}
	} else { // player is Dracula
		PlaceId from = GvGetPlayerLocation(hv->gv, PLAYER_DRACULA);
		if (!placeIsReal(from)) {  // Dracula's current location is not revealed
			*numReturnedLocs = 0;
			return NULL;
		}
		return GvGetReachableByType(hv->gv, PLAYER_DRACULA, GvGetRound(hv->gv), 
		                            from, road, false, boat, numReturnedLocs);
	}
}

////////////////////////////////////////////////////////////////////////
// Our own interface functions

// Queue operations
// create new empty Queue
Queue newQueue (void) {
	Queue new = malloc (sizeof *new);
	new->head = NULL;
	new->tail = NULL;
	return new;
}

// free the Queue
void dropQueue (Queue Q) {
	assert (Q != NULL);
	for (QueueNode *curr = Q->head, *next; curr != NULL; curr = next) {
		next = curr->next;
		free (curr);
	}
	free (Q);
}

// add item at end of Queue
void QueueJoin (Queue Q, int it) {
	assert (Q != NULL);

	QueueNode *new = malloc (sizeof *new);
	assert (new != NULL);
	new->value = it;
	new->next = NULL;

	if (Q->head == NULL)
		Q->head = new;
	if (Q->tail != NULL)
		Q->tail->next = new;
	Q->tail = new;
}

// remove item from front of Queue
int QueueLeave (Queue Q) {
	assert (Q != NULL);
	assert (Q->head != NULL);
	int it = Q->head->value;
	QueueNode *old = Q->head;
	Q->head = old->next;
	if (Q->head == NULL)
		Q->tail = NULL;
	free (old);
	return it;
}

// check for no items
int QueueIsEmpty (Queue Q) {
	return (Q->head == NULL);
}
