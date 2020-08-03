        
////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// dracula.c: your "Fury of Dracula" Dracula AI
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include "dracula.h"
#include "DraculaView.h"
#include "Game.h"
#include "GameView.h"

#include "Map.h"
#include "Places.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define LANDPLACELENGTH 16
#define RAILPLACELENGTH 30
#define SEAPLACELENGTH 10
#define HUNTERCOUNT 4


typedef struct hunterPlace HunterPlace;

struct hunterPlace {
	PlaceId place;
	int total;
};

static PlaceId predictLocation(DraculaView dv);
PlaceId bestPlace(PlaceId* hunterPlace);

void decideDraculaMove(DraculaView dv)
{
	// TODO: Replace this with something better!
	PlaceId currPlace = predictLocation(dv);

	if (currPlace == NOWHERE) {
		int numReturnedLocs;
		PlaceId* places = DvGetValidMoves(dv, &numReturnedLocs);
		srand(time(0));
		currPlace = places[rand() % numReturnedLocs];
		free(places);
	}
	registerBestPlay(placeIdToAbbrev(currPlace), "小弟弟来追我啊，我已经放了陷阱了哦 :)");
}

PlaceId predictLocation(DraculaView dv) {

	PlaceId currPlace = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	int maxLength = 0;
	if (currPlace == NOWHERE) {
		PlaceId hunterPlace[HUNTERCOUNT];
		for (int i = 0; i < HUNTERCOUNT; i++) {
			printf("%d\n", i);
			int * dest = DvGetShortestPath(dv, i);
			maxLength = dest[0];
			for (int j = 0; j <= MAX_REAL_PLACE; j++) {
				if (dest[j] > maxLength && placeIsLand(i)) { 
					hunterPlace[i] = j; 
					maxLength = dest[j]; 
				}
			}
			printf("%d %s\n", i, placeIdToName(hunterPlace[i]));
		}
		return bestPlace(hunterPlace);
	}
	return NOWHERE;
}

PlaceId bestPlace(PlaceId* hunterPlace) {
	HunterPlace hunterBucket[HUNTERCOUNT];
	PlaceId best = NOWHERE;

	for (int i = 0; i < HUNTERCOUNT; i++) {
		hunterBucket[i].place = hunterPlace[i];
		hunterBucket[i].total = 1;
	}

	for (int i = 0; i < HUNTERCOUNT; i++)
		for (int j = 0; j < HUNTERCOUNT; j++)
			if (hunterBucket[i].place == hunterBucket[j].place) {
				hunterBucket[j].place = NOWHERE;
				hunterBucket[i].total++;
			}

	int max = 0;
	for (int i = 0; i < HUNTERCOUNT; i++)
		if (max < hunterBucket[i].total && hunterBucket[i].total != NOWHERE) {
			max = hunterBucket[i].total;
			best = hunterBucket[i].place;
		}

	return best;
}

    