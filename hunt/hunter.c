////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// hunter.c: your "Fury of Dracula" hunter AI.
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-01	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include "Game.h"
#include "hunter.h"
#include "HunterView.h"


#include "Places.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
/*
typedef mode {
	CHASING,	
	RANDOM,	
	RESEARCH	
} Mode;
*/
const char * decideLordGodalmingMove(HunterView hv);
const char * decideDrSewardMove(HunterView hv);
const char * decideVanHelsingMove(HunterView hv);
const char * decideMinaHarkerMove(HunterView hv);
PlaceId predictLocation(HunterView hv);


void decideHunterMove(HunterView hv) {

	Player player = HvGetPlayer(hv);
	const char * place;
    switch (player) {
        case PLAYER_LORD_GODALMING:
            place = decideLordGodalmingMove(hv);
        break;
        case PLAYER_DR_SEWARD:
            place = decideDrSewardMove(hv);
        break;
        case PLAYER_VAN_HELSING:
            place = decideVanHelsingMove(hv);
        break;
        case PLAYER_MINA_HARKER:
            place = decideMinaHarkerMove(hv);
        break;
		default:
		break;
    }
	char * newPlace = malloc(2 * sizeof(char));
	strcpy(newPlace, place);
	registerBestPlay(newPlace, "I like Dracula");
}


const char * decideLordGodalmingMove(HunterView hv) {
	PlaceId currPlace = HvGetPlayerLocation(hv, PLAYER_LORD_GODALMING);
	if (currPlace == NOWHERE) return "ED";

	PlaceId newPlace = currPlace;
	PlaceId predictPlace = predictLocation(hv);
	if (predictPlace == NOWHERE) {
		int numReturnedLocs;
		PlaceId * places = HvWhereCanIGo(hv, &numReturnedLocs);
		srand(time(0));
		newPlace = places[rand() % numReturnedLocs];
		free(places);
	}
	return placeIdToAbbrev(newPlace);
}

const char * decideDrSewardMove(HunterView hv) {
	PlaceId currPlace = HvGetPlayerLocation(hv, PLAYER_DR_SEWARD);
	if (currPlace == NOWHERE) return "BD";

	PlaceId newPlace = currPlace;
	if (predictLocation(hv) == NOWHERE) {
		int numReturnedLocs;
		PlaceId * places = HvWhereCanIGo(hv, &numReturnedLocs);
		srand(time(0));
		newPlace = places[rand() % numReturnedLocs];
		free(places);
	}
	return placeIdToAbbrev(newPlace);
}

const char * decideVanHelsingMove(HunterView hv) {
	PlaceId currPlace = HvGetPlayerLocation(hv, PLAYER_VAN_HELSING);
	if (currPlace == NOWHERE) return "PA";

	PlaceId newPlace = currPlace;
	if (predictLocation(hv) == NOWHERE) {
		int numReturnedLocs;
		PlaceId * places = HvWhereCanIGo(hv, &numReturnedLocs);
		srand(time(0));
		newPlace = places[rand() % numReturnedLocs];
		free(places);
	}
	return placeIdToAbbrev(newPlace);
}

const char * decideMinaHarkerMove(HunterView hv){
	assert(hv != NULL);
	PlaceId currPlace = HvGetPlayerLocation(hv, PLAYER_Mina_Harker);
	
	if (currPlace != CD) {
		PlaceId *shortestPath = HvGetShortestPathTo(hv, PLAYER_Mina_Harker, CD, &pathLength);
		return shortestPath;
	}

	reuturn "CD";

}

PlaceId predictLocation(HunterView hv) {
	return NOWHERE;
}


