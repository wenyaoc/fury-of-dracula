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
#include <limits.h>
#include <string.h>

#define MAXCHAR 8
#define MAXLINE 40
// TODO: ADD YOUR OWN STRUCTS HERE
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
/*

typedef struct GraphRep *Graph;

struct GraphRep {
   int   nV;    // #vertices
   int   nE;    // #edges
   int **edges; // matrix of 0/1
};
*/
struct hunterView {
	// TODO: ADD FIELDS HERE
	GameView gv;
	//Graph graph;
};


bool canGo(HunterView hv, PlaceId place);
int getDoubleBackNum(HunterView hv, PlaceId place);
bool canDoubleBack(HunterView hv);
bool canHide(HunterView hv);

Queue newQueue (void);			// create new empty queue
void dropQueue (Queue Q);			// free memory used by queue
void showQueue (Queue Q);			// display as 3 > 5 > 4 > ...
void QueueJoin (Queue Q, int it);	// add int on queue
int QueueLeave (Queue Q);		// remove item from queue
int QueueIsEmpty (Queue Q);		// check for no items
/*
Graph newGraph(int nV);
void  insertE(Graph g, int v, int w);
void  removeE(Graph g, int v, int w);
void dropGraph(Graph g);
bool adjacent(Graph g, int v, int w);
int validV(Graph g, int v);
void show(Graph g);
*/
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
	//new->graph = newGraph(MAX_REAL_PLACE + 1);

	/*
	ConnList connection;

	for (int i = 0; i <= MAX_REAL_PLACE; i++) {
		connection = MapGetConnections(m, i);
		ConnList curr = connection;
		while (curr != NULL) {
			insertE(new->graph, i, curr->p);
			curr = curr->next;
		}
	}
	//show(new->graph);
	*/
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
	int number;
	bool canFree;
	PlaceId * place = GvGetLocationHistory(hv->gv, PLAYER_DRACULA, &number, &canFree);
	PlaceId LastKnownPlace;
	int known = 0;
	for (int i = number - 1; i >= 0; i--) {
		if(placeIsReal(place[i])) {
			//printf("%s\n", placeIdToName(place[i]));
			*round = i;
			LastKnownPlace = place[i];
			known = 1;
			//printf("%d %s\n",*round, placeIdToName(LastKnownPlace));
			break;
		}
	}
	if (canFree)
		free(place);
	if (!known) return NOWHERE;
                             
	return LastKnownPlace;
}

PlaceId *HvGetShortestPathTo(HunterView hv, Player hunter, PlaceId dest,
                             int *pathLength)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	PlaceId src = HvGetPlayerLocation(hv, hunter);

	PlaceId * path = NULL;
	*pathLength = 0;
	if (src == dest) {
		path = malloc((*pathLength + 1) * sizeof(PlaceId));
		path[0] = src;
		return path;
	}
	Round round = HvGetRound(hv);
	int dist[MAX_REAL_PLACE + 1];
	int pred[MAX_REAL_PLACE + 1];
	*pathLength = 0;

	for (int i = 0; i < MAX_REAL_PLACE + 1; i++) {
		dist[i] = INT_MAX;
		pred[i] = -1;
	}
	Queue q = newQueue();
	dist[src] = 0;
	pred[src] = src;
	QueueJoin(q, src);
	int w, v;
	while (!QueueIsEmpty(q)) {
		w = QueueLeave(q);
		//printf("%s  %d\n", placeIdToName(w), round + dist[w]);
		int numReturnedLocs;
		PlaceId * reachable = GvGetReachable(hv->gv, hunter, round + dist[w], w, &numReturnedLocs);
		for (int i = 0; i < numReturnedLocs; i++) {
			v = reachable[i];
			if (w != v && pred[v] == -1) {
				//printf("%d, %s\n", i, placeIdToName(v));
				pred[v] = w;
				dist[v] = dist[w] + 1;
				QueueJoin(q, v);
			}
		}
		free(reachable);
	}
	dropQueue(q);
	*pathLength = 0;

	*pathLength = dist[dest];
	path = malloc((*pathLength) * sizeof(PlaceId));
	w = dest;
	path[dist[dest] - 1] = dest;
	for (int i = dist[dest] - 2; i >= 0; i--) {
		path[i] = pred[w];
		w = pred[w];
	}

	return path;
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
	*numReturnedLocs = 0;
	Player player = GvGetPlayer(hv->gv);
	PlaceId from = GvGetPlayerLocation(hv->gv, player);
	if (from == NOWHERE) return NULL;
	return GvGetReachableByType(hv->gv, player, GvGetRound(hv->gv) + player, from, road, rail, boat, numReturnedLocs);
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
		*numReturnedLocs = 0;
		PlaceId from = GvGetPlayerLocation(hv->gv, player);
		if (from == NOWHERE) return NULL;
		
		Player currPlayer = GvGetPlayer(hv->gv);
		if (currPlayer < player)
			return GvGetReachableByType(hv->gv, player, GvGetRound(hv->gv), from, road, rail, boat, numReturnedLocs);
		else 
			return GvGetReachableByType(hv->gv, player, GvGetRound(hv->gv) + 1, from, road, rail, boat, numReturnedLocs);
	} else {
		PlaceId * place = NULL;
		PlaceId from = GvGetPlayerLocation(hv->gv, PLAYER_DRACULA);
		place = GvGetReachableByType(hv->gv, PLAYER_DRACULA, GvGetRound(hv->gv), from, road, rail, boat, numReturnedLocs);
		int newNum = 0;
		PlaceId * newPlace = calloc(*numReturnedLocs, sizeof(PlaceId));

		for (int i = 0; i < *numReturnedLocs; i++) {
			if (canGo(hv, place[i])) {
				newPlace[newNum] = place[i];
				newNum = newNum + 1;
			} else if (getDoubleBackNum(hv, place[i]) > 0) {
				newPlace[newNum] = place[i];
				newNum = newNum + 1;
			} else if (i == *numReturnedLocs - 1 && canHide(hv)) {
				newPlace[newNum] = place[i];
				newNum = newNum + 1;
			}
		}
		*numReturnedLocs = newNum;
		free(place);
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

// create new empty Queue
Queue newQueue (void)
{
	Queue new = malloc (sizeof *new);
	new->head = NULL;
	new->tail = NULL;
	return new;
}

void dropQueue (Queue Q)
{
	assert (Q != NULL);
	for (QueueNode *curr = Q->head, *next; curr != NULL; curr = next) {
		next = curr->next;
		free (curr);
	}
	free (Q);
}
/*
// display as 3 > 5 > 4 > ...
void showQueue (Queue Q)
{
	assert (Q != NULL);

	for (QueueNode *curr = Q->head; curr != NULL; curr = curr->next) {
		ItemShow (curr->value);
		if (curr->next != NULL)
			printf (">");
	}
	printf ("\n");
}
*/
// add item at end of Queue
void QueueJoin (Queue Q, int it)
{
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
int QueueLeave (Queue Q)
{
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
int QueueIsEmpty (Queue Q)
{
	return (Q->head == NULL);
}
/*
Graph newGraph(int nV)
{
	assert(nV > 0);
	int **e = malloc(nV * sizeof(int *));
	assert(e != NULL);
	for (int i = 0; i < nV; i++) {
		e[i] = calloc(nV, sizeof(int));
		assert(e[i] != NULL);
	}
	Graph g = malloc(sizeof(*g));
	assert(g != NULL);
	g->nV = nV;  g->nE = 0;  g->edges = e;
	return g;
}

void  insertE(Graph g, int v, int w)
{
	assert(g != NULL);
	assert(validV(g,v) && validV(g,w));
	if (g->edges[v][w]) return;
	g->edges[v][w] = 1;
	g->edges[w][v] = 1;
}

// delete an edge
void  removeE(Graph g, int v, int w)
{
	assert(g != NULL);
	assert(validV(g,v) && validV(g,w));
	if (!g->edges[v][w]) return;
	g->edges[v][w] = 0;
	g->edges[w][v] = 0;
}

void dropGraph(Graph g)
{
	assert(g != NULL);
	for (int i = 0; i < g->nV; i++)
		free(g->edges[i]);
	free(g->edges);
}

bool adjacent(Graph g, int v, int w)
{
	assert(validV(g,v) && validV(g,w));
	return (g->edges[v][w] != 0);
}

int validV(Graph g, int v)
{
	return (g != NULL && v >= 0 && v < g->nV);
}

void show(Graph g)
{
	assert(g != NULL);
	printf("Graph has V=%d and E=%d\n",g->nV,g->nE);
	printf("V    Connected to\n");
	printf("--   ------------\n");
	
	int v, w;
	for (v = 0; v < g->nV; v++) {
		printf("%-3s ",placeIdToName(v));
		for (w = 0; w < g->nV; w++) {
			if (adjacent(g,v,w)) printf(" %s",placeIdToName(w));
		}
		printf("\n");
	}
}

*/

