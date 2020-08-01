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
PlaceId * removeImpossibleLocations(HunterView hv, Player player, PlaceId * places, int * numReturnedLocs);

bool hasRailConnection(PlaceId place);
bool hasSeaMove(HunterView hv, int numMoves);
bool placeHasOtherHunter(HunterView hv, Player player, PlaceId place);
bool canGo(HunterView hv, PlaceId place);
bool canDoubleBack(HunterView hv, PlaceId place);

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

////////////////////////////////////////////////////////////////////////
// helper functions

const char * decideLordGodalmingMove(HunterView hv) {
	PlaceId currPlace = HvGetPlayerLocation(hv, PLAYER_LORD_GODALMING);
	Round currRound = HvGetRound(hv);
	if (currPlace == NOWHERE) return "ED";

	PlaceId newPlace = getMove(hv, PLAYER_LORD_GODALMING);
	if (newPlace == NOWHERE && currRound < 6) {
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
		PlaceId *shortestPath = NULL;
		if (currRound % 26 >= 1 && currRound % 26 < 6)
			shortestPath = HvGetShortestPathTo(hv, PLAYER_LORD_GODALMING, LIVERPOOL, &pathLength);
		else
			shortestPath = HvGetShortestPathTo(hv, PLAYER_LORD_GODALMING, FRANKFURT, &pathLength);

		if (pathLength > 3) {
			newPlace = shortestPath[0];
		} else {
			int numReturnedLocs;
			bool sea = (hasSeaMove(hv, 6) || placeIsSea(currPlace));
			PlaceId * places = HvWhereCanIGoByType(hv, true, true, sea, &numReturnedLocs);
			if (placeIsSea(currPlace))
				numReturnedLocs--;
			newPlace = randomLocation(hv, places, PLAYER_LORD_GODALMING, numReturnedLocs);
			free(places);
		}

		free(shortestPath);
		PlaceId vampirePlace = HvGetVampireLocation(hv);
		if (placeIsReal(vampirePlace)) {
			shortestPath = HvGetShortestPathTo(hv, PLAYER_LORD_GODALMING, vampirePlace, &pathLength);
			if(pathLength <= 3 && pathLength > 0) 
				newPlace = shortestPath[0];
			free(shortestPath);
		}
	}
	return placeIdToAbbrev(newPlace);
}

const char * decideDrSewardMove(HunterView hv) {
	PlaceId currPlace = HvGetPlayerLocation(hv, PLAYER_DR_SEWARD);
	Round currRound = HvGetRound(hv);
	if (currPlace == NOWHERE) return "BD";

	PlaceId newPlace = getMove(hv, PLAYER_DR_SEWARD);
	if (newPlace == NOWHERE) {
		int pathLength;
		PlaceId *shortestPath = NULL;
		if ((currRound % 26 >= 1 && currRound % 26 <= 6) && hasSeaMove(hv, 6))
			shortestPath =  HvGetShortestPathTo(hv, PLAYER_DR_SEWARD, MUNICH, &pathLength);
		else
			shortestPath =  HvGetShortestPathTo(hv, PLAYER_DR_SEWARD, SZEGED, &pathLength);
		
		if (pathLength > 3) {
			newPlace = shortestPath[0];
		} else {
			int numReturnedLocs;
			bool sea = (hasSeaMove(hv, 6) || placeIsSea(currPlace));
			PlaceId * places = HvWhereCanIGoByType(hv, true, true, sea, &numReturnedLocs);
			if (placeIsSea(currPlace))
				numReturnedLocs--;
			newPlace = randomLocation(hv, places, PLAYER_DR_SEWARD, numReturnedLocs);
			free(places);
		}

		free(shortestPath);
		PlaceId vampirePlace = HvGetVampireLocation(hv);
		if (placeIsReal(vampirePlace)) {
			shortestPath = HvGetShortestPathTo(hv, PLAYER_DR_SEWARD, vampirePlace, &pathLength);
			if(pathLength <= 3 && pathLength > 0) 
				newPlace = shortestPath[0];
			free(shortestPath);
		}
	}
	return placeIdToAbbrev(newPlace);
}

const char * decideVanHelsingMove(HunterView hv) {
	PlaceId currPlace = HvGetPlayerLocation(hv, PLAYER_VAN_HELSING);
	//Round currRound = HvGetRound(hv);

	if (currPlace == NOWHERE) return "PA";

	PlaceId newPlace = getMove(hv, PLAYER_VAN_HELSING);
	if (newPlace == NOWHERE) {
		if (HvGetRound(hv) == 1) return "MA";
		int pathLength;

		PlaceId *shortestPath = HvGetShortestPathTo(hv, PLAYER_VAN_HELSING, MADRID, &pathLength);
				
		if (pathLength > 2) {
			newPlace = shortestPath[0];
		} else {
			int numReturnedLocs;
			bool sea = (hasSeaMove(hv, 6) || placeIsSea(currPlace));
			PlaceId * places = HvWhereCanIGoByType(hv, true, true, sea, &numReturnedLocs);
			if (placeIsSea(currPlace))
				numReturnedLocs--;
			newPlace = randomLocation(hv, places, PLAYER_VAN_HELSING, numReturnedLocs);
			free(places);
		}

		free(shortestPath);
		PlaceId vampirePlace = HvGetVampireLocation(hv);
		if (placeIsReal(vampirePlace)) {
			shortestPath = HvGetShortestPathTo(hv, PLAYER_VAN_HELSING, vampirePlace, &pathLength);
			if(pathLength <= 3 && pathLength > 0) 
				newPlace = shortestPath[0];
			free(shortestPath);
		}
	}
	return placeIdToAbbrev(newPlace);
}

const char * decideMinaHarkerMove(HunterView hv) {
	PlaceId currPlace = HvGetPlayerLocation(hv, PLAYER_MINA_HARKER);
	if (currPlace == NOWHERE) 
		return "CD";
	
	Round round = HvGetRound(hv);
	Round knownDraculaRound = -1;
	PlaceId knownDraculaLocation = HvGetLastKnownDraculaLocation(hv, &knownDraculaRound);
	PlaceId newPlace = CASTLE_DRACULA;
	printf("%s %s\n", placeIdToName(currPlace),placeIdToName(knownDraculaLocation));
	if (knownDraculaRound != -1 && round - knownDraculaRound < 6) {
		if ((knownDraculaLocation == currPlace) && (round - knownDraculaRound == 1)) 
			newPlace = currPlace;
		else {
			int numLocs;
			bool boat = placeIsSea(currPlace);
			PlaceId * places = HvGetReachableByType(hv, PLAYER_DRACULA, knownDraculaRound, knownDraculaLocation, true, false, boat, &numLocs);
			places = removeImpossibleLocations(hv, PLAYER_DRACULA, places, &numLocs);
			if (placeIsSea(currPlace))
				numLocs--;
			newPlace = randomLocation(hv, places, PLAYER_MINA_HARKER, numLocs - 1);
			int pathLength;
			free(places);
			places = HvGetShortestPathTo(hv, PLAYER_MINA_HARKER, knownDraculaLocation, &pathLength);
			if(pathLength > 0) {
				newPlace = places[0];
				free(places);
			}
		}
	} else if (currPlace != CASTLE_DRACULA) {
		int pathLength;
		PlaceId * places = HvGetShortestPathTo(hv, PLAYER_MINA_HARKER, CASTLE_DRACULA, &pathLength);
		newPlace = places[0];
		free(places);
	}
	return placeIdToAbbrev(newPlace);
}



PlaceId getMove(HunterView hv, Player player) {
	PlaceId currPlace = HvGetPlayerLocation(hv, player);
	Round round = HvGetRound(hv);
	Round knownDraculaRound = -1;
	PlaceId knownDraculaLocation = HvGetLastKnownDraculaLocation(hv, &knownDraculaRound);
	//printf("%s %s\n", placeIdToName(currPlace), placeIdToName(knownDraculaLocation));
	//printf("%d %d\n", round, knownDraculaRound);
	if (round - knownDraculaRound > 7 || knownDraculaLocation == NOWHERE) { 
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
	} else if (round - knownDraculaRound >= 2 && round - knownDraculaRound <= 4 ) {
		PlaceId DraculaCurrPlace = HvGetPlayerLocation(hv, PLAYER_DRACULA);
		bool boat = false;
		bool road = true;
		if (DraculaCurrPlace == SEA_UNKNOWN) {
			boat = true;
			road = false;
		}				
		PlaceId newPlace;
		if (currPlace == knownDraculaLocation) {
			int numLocs;
			PlaceId * places = HvWhereCanIGoByType(hv, road, false, boat, &numLocs);
			places = removeImpossibleLocations(hv, PLAYER_DRACULA, places, &numLocs);
			if (numLocs != 0) {
				newPlace = randomLocation(hv, places, player, numLocs - 1);
				free(places);
				return newPlace;
			}
		} else {
			int numLocs;
			PlaceId * places = HvGetReachableByType(hv, PLAYER_DRACULA, knownDraculaRound, knownDraculaLocation, true, false, boat, &numLocs);
			places = removeImpossibleLocations(hv, PLAYER_DRACULA, places, &numLocs);
			newPlace = randomLocation(hv, places, player, numLocs - 1);
			int pathLength;
			PlaceId *shortestPath = HvGetShortestPathTo(hv, player, newPlace, &pathLength);
			free(places);
			if (pathLength > 0) {
				newPlace = shortestPath[0];
				free(shortestPath);
				return newPlace;
			}
		}
	}
	return NOWHERE;
}

PlaceId randomLocation(HunterView hv, PlaceId * places, Player player, int numReturnedLocs) { 
	if (numReturnedLocs == 0)
		return places[0];
	Round round = HvGetRound(hv);
	int num = round + player;
	PlaceId newPlace;

	srand(time(0));
	newPlace = places[rand() % numReturnedLocs];
	// if hunter can travel long distance via rail next turn
	// prefer a city with rail 
	if ((num % 4 == 2) && !hasRailConnection(newPlace)) {
		for (int i = 0; i < numReturnedLocs; i++) {
			if (hasRailConnection(places[i])) {
				newPlace = places[i];
				break;
			}		
		}
	}
	if (placeHasOtherHunter(hv, player, newPlace)) {
		srand(time(0));
		newPlace = places[rand() % numReturnedLocs];
	}
	return newPlace;
}

PlaceId * removeImpossibleLocations(HunterView hv, Player player, PlaceId * places, int * numReturnedLocs) {

	PlaceId * availablePlace = malloc(*numReturnedLocs * sizeof(PlaceId));
	int num = 0;
	for(int i = 0; i < *numReturnedLocs; i++) {
		if(canGo(hv, places[i]) || canDoubleBack(hv, places[i])) {
			int hasOtherHunter = 0;
			for(int j = 0; j < player; j++) {
				if(HvGetPlayerLocation(hv, j) == places[i])
					hasOtherHunter = 1;
			}
			if(!hasOtherHunter){
				availablePlace[num] = places[i];
				num++;
			}
		}
	}
	*numReturnedLocs = num;
	free(places);
	for(int i = 0; i < num; i++) {
		printf("%s\n", placeIdToName(availablePlace[i]));
	}
	return availablePlace;
}




bool placeHasOtherHunter(HunterView hv, Player player, PlaceId place) {
	for(int i = 0; i < player; i++) {
		if(HvGetPlayerLocation(hv, i) == place)
			return true;
	}
	return false;
}

bool hasRailConnection(PlaceId place) {
	Map m = MapNew();
	ConnList list = MapGetConnections(m, place);
	while (list != NULL) {
		if (list->type == RAIL) 
			return true;
		list = list->next;
	}
	return false;
}

bool hasSeaMove(HunterView hv, int numMoves) {
	int numReturnedMoves;
	bool canFree;
	PlaceId * places = HvGetLastMoves(hv, PLAYER_DRACULA, numMoves, &numReturnedMoves, &canFree);
	if (numReturnedMoves == 0)
		return false;	
	for(int i = 0; i < numReturnedMoves; i++) {
		if (placeIsSea(places[i]) || places[i] == SEA_UNKNOWN) 
			return true;
	}
	return false;
}


bool canGo(HunterView hv, PlaceId place) {

	if (place == NOWHERE) // Dracula has no movement yet
		return true;
	if (place == HOSPITAL_PLACE) // Dracula cannot goto hospital
		return false;

	int num = -1;
	bool canFree = false;
	bool go = true;
	PlaceId* list = HvGetLastLocations(hv, PLAYER_DRACULA, 5, &num, &canFree);

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

// determine whether Dracula can double back in the curretnt turn
// input: DraculaView
// output: true if dracula can double back, false otherwise
bool canDoubleBack(HunterView hv, PlaceId place) {
	
	int num = -1;
	bool canFree = false;
	bool doubleBack = true;
	PlaceId* list = HvGetLastMoves(hv, PLAYER_DRACULA, 5, &num, &canFree);

	for (int i = 0; i < num; i++) {
		if ((list[i] >= DOUBLE_BACK_1 && list[i] <= DOUBLE_BACK_5) || (list[i] == place))
			doubleBack = false; // there exist a double back move in the trail
	}
	if (canFree)
		free(list);
	return doubleBack;
}

