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

#include "Map.h"
#include "Places.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


const char * decideLordGodalmingMove(HunterView hv);
const char * decideDrSewardMove(HunterView hv);
const char * decideVanHelsingMove(HunterView hv);
const char * decideMinaHarkerMove(HunterView hv);
PlaceId getMove(HunterView hv, Player player);
PlaceId randomLocation(HunterView hv, PlaceId * places, Player player, int numReturnedLocs);

bool hasRailConnection(PlaceId place);


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
	Round round = HvGetRound(hv);
	if (currPlace == NOWHERE) return "ED";

	PlaceId newPlace = getMove(hv, PLAYER_LORD_GODALMING);
	if (newPlace == NOWHERE && round <= 6) {
		switch (currPlace) {
			case EDINBURGH:
				newPlace = MANCHESTER;
			break;
			case MANCHESTER:
				newPlace = LIVERPOOL;
			break;
			case LIVERPOOL:
				newPlace = SWANSEA;
			break;
			case SWANSEA:
				newPlace = LONDON;
			break;
			case LONDON:
				newPlace = PLYMOUTH;
			break;
			default:
			break;
    	}
	} 
	if (newPlace == NOWHERE) {
		int pathLength;
		PlaceId *shortestPath = HvGetShortestPathTo(hv, PLAYER_LORD_GODALMING, FRANKFURT, &pathLength);
		//for(int i = 0; i < pathLength; i ++) {
		//	printf("%s\n", placeIdToName(shortestPath[i]));
		//}
		if (pathLength > 3) {
			newPlace = shortestPath[0];
			free(shortestPath);
		} else {
			int numReturnedLocs;
			PlaceId * places = HvWhereCanIGo(hv, &numReturnedLocs);
			newPlace = randomLocation(hv, places, PLAYER_LORD_GODALMING, numReturnedLocs);
			free(places);
		}
	}
	return placeIdToAbbrev(newPlace);
}

const char * decideDrSewardMove(HunterView hv) {
	PlaceId currPlace = HvGetPlayerLocation(hv, PLAYER_DR_SEWARD);
	if (currPlace == NOWHERE) return "BD";

	PlaceId newPlace = getMove(hv, PLAYER_DR_SEWARD);
	if (newPlace == NOWHERE) {
		int pathLength;
		PlaceId *shortestPath = HvGetShortestPathTo(hv, PLAYER_DR_SEWARD, BELGRADE, &pathLength);
		if (pathLength > 3) {
			newPlace = shortestPath[0];
			free(shortestPath);
		} else {
			int numReturnedLocs;
			PlaceId * places = HvWhereCanIGo(hv, &numReturnedLocs);
			newPlace = randomLocation(hv, places, PLAYER_DR_SEWARD, numReturnedLocs);
			free(places);
		}
	}
	return placeIdToAbbrev(newPlace);
}

const char * decideVanHelsingMove(HunterView hv) {
	PlaceId currPlace = HvGetPlayerLocation(hv, PLAYER_VAN_HELSING);
	if (currPlace == NOWHERE) return "PA";

	PlaceId newPlace = getMove(hv, PLAYER_VAN_HELSING);
	if (newPlace == NOWHERE) {
		if (HvGetRound(hv) == 2) return "MA";
		int pathLength;
		PlaceId *shortestPath = HvGetShortestPathTo(hv, PLAYER_VAN_HELSING, MADRID, &pathLength);
		if (pathLength > 3) {
			newPlace = shortestPath[0];
			free(shortestPath);
		} else {
			int numReturnedLocs;
			PlaceId * places = HvWhereCanIGo(hv, &numReturnedLocs);
			newPlace = randomLocation(hv, places, PLAYER_VAN_HELSING, numReturnedLocs);
			free(places);
		}
	}
	return placeIdToAbbrev(newPlace);
}

const char * decideMinaHarkerMove(HunterView hv) {
	return "CD";
}

PlaceId getMove(HunterView hv, Player player) {
	PlaceId currPlace = HvGetPlayerLocation(hv, PLAYER_DR_SEWARD);
	Round round = HvGetRound(hv);
	Round knownDraculaRound = -1;
	Place knownDraculaLocation = HvGetLastKnownDraculaLocation(hv, &knownDraculaRound);

	if (knownDraculaRound == -1) { 
		return NOWHERE;
	} else if (round == knownDraculaRound + 1) {
		int pathLength;
		PlaceId *shortestPath = HvGetShortestPathTo(hv, player, knownDraculaLocation, &pathLength);
		if (pathLength == 0) 
			return currPlace;
		else {
			PlaceId newPlace = shortestPath[0];
			free(shortestPath);
			return newPlace;
		}
	}

}

PlaceId randomLocation(HunterView hv, PlaceId * places, Player player, int numReturnedLocs) {
	Round round = HvGetRound(hv);
	int num = round + player;
	PlaceId newPlace;

	srand(time(0));
	newPlace = places[rand() % numReturnedLocs];
	// if hunter can travel long distance via rail next ture
	// prefer a city with rail 
	if ((num % 4 == 1 || num % 4 == 2) && !hasRailConnection(newPlace)) {
		for (int i = 0; i < numReturnedLocs; i++) {
			if (hasRailConnection(places[i])) {
				newPlace = places[i];
				break;
			}		
		}
	}
	return newPlace;
}


bool hasRailConnection(PlaceId place) {
	printf("%s\n", placeIdToName(place));
	Map m = MapNew();
	ConnList list = MapGetConnections(m, place);
	while (list != NULL) {
		if (list->type == RAIL) 
			return true;
		list = list->next;
	}
	return false;
}
