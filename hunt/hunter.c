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
PlaceId getVampire(HunterView hv, Player player, PlaceId place);
PlaceId getRandomLocation(HunterView hv, PlaceId * places, Player player, int numReturnedLocs);
PlaceId * removeSameLocations(HunterView hv, Player player, PlaceId * places, int * numReturnedLocs);
PlaceId * removeImpossibleLocations(HunterView hv, Player player, PlaceId * places, PlaceId currPlace, int * numReturnedLocs);
PlaceId * removeBoat(PlaceId * places, int * numReturnedLocs);
PlaceId * removeRoad(PlaceId * places, int * numReturnedLocs);
PlaceId moveIsValid(HunterView hv, Player player, PlaceId newPlace);


bool hasRailConnection(PlaceId place);
bool hasSeaMove(HunterView hv, int numMoves);
bool placeHasOtherHunter(HunterView hv, Player player, PlaceId place);
bool canGo(HunterView hv, PlaceId place);
bool planToTeleport(PlaceId * moves, int numMoves, PlaceId place);
bool placeCanTeleport(PlaceId place);

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
			place = "CD"; // simply prevent segmentation fault
		break;
    }

	registerBestPlay(place, "I like Dracula");
}

////////////////////////////////////////////////////////////////////////
// helper functions

const char * decideLordGodalmingMove(HunterView hv) {
	PlaceId currPlace = HvGetPlayerLocation(hv, PLAYER_LORD_GODALMING);
	Round currRound = HvGetRound(hv);
	if (currPlace == NOWHERE) return "ED";

	PlaceId newPlace = getMove(hv, PLAYER_LORD_GODALMING);
	printf("newplace = %s\n", placeIdToName(newPlace));
	// in the first 6 turns, make sure there is no vampires in England
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
	// deacula's location is unknown
	// Lord Godalming will move around liverpool if 1 <= round%26 < 6 
	// otherwise move around Frankfurt
	if (newPlace == NOWHERE) {
		int pathLength;
		PlaceId *shortestPath = NULL;
		if (currRound % 26 >= 1 && currRound % 26 < 6)
			shortestPath = HvGetShortestPathTo(hv, PLAYER_LORD_GODALMING, LIVERPOOL, &pathLength);
		else
			shortestPath = HvGetShortestPathTo(hv, PLAYER_LORD_GODALMING, FRANKFURT, &pathLength);
		
		printf("path = %d\n", pathLength);
		if (pathLength > 2 || placeIsSea(currPlace)) { // if Hunter is far away from the center
			newPlace = shortestPath[0];
		} else {
			int numReturnedLocs;
			bool sea = (hasSeaMove(hv, 6) || placeIsSea(currPlace));
			PlaceId * places = HvWhereCanIGoByType(hv, true, true, sea, &numReturnedLocs);
			for(int i = 0; i < numReturnedLocs; i++) {
				printf("%s\n", placeIdToName(places[i]));
			}	
			printf("\n");
			int numLocs = numReturnedLocs;
			PlaceId * removedPlaces = removeSameLocations(hv, PLAYER_LORD_GODALMING, places, &numLocs);
			for(int i = 0; i < numLocs; i++) {
				printf("%s\n", placeIdToName(removedPlaces[i]));
			}	
			printf("\n");
			if (numLocs > 0) {
				free(places);
				places = removedPlaces;
				numReturnedLocs = numLocs;
			}
			if (placeIsSea(currPlace))
				numReturnedLocs--;
			newPlace = getRandomLocation(hv, places, PLAYER_LORD_GODALMING, numReturnedLocs);
			
			free(places);
		}
		if (pathLength != 0)
			free(shortestPath);
		
		newPlace = getVampire(hv, PLAYER_LORD_GODALMING, newPlace);
	}
	newPlace = moveIsValid(hv, PLAYER_LORD_GODALMING, newPlace);
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
			shortestPath =  HvGetShortestPathTo(hv, PLAYER_DR_SEWARD, BUCHAREST, &pathLength);
		
		if (pathLength > 3) {
			newPlace = shortestPath[0];
		} else {
			int numReturnedLocs;
			bool sea = (hasSeaMove(hv, 6) || placeIsSea(currPlace));
			PlaceId * places = HvWhereCanIGoByType(hv, true, true, sea, &numReturnedLocs);
			int numLocs = numReturnedLocs;
			PlaceId * removedPlaces = removeSameLocations(hv, PLAYER_LORD_GODALMING, places, &numLocs);
			if (numLocs > 0) {
				free(places);
				places = removedPlaces;
				numReturnedLocs = numLocs;
			}
			if (placeIsSea(currPlace))
				numReturnedLocs--;
			newPlace = getRandomLocation(hv, places, PLAYER_DR_SEWARD, numReturnedLocs);
			free(places);
		}
		if (pathLength > 0)
			free(shortestPath);
		
		newPlace = getVampire(hv, PLAYER_DR_SEWARD, newPlace);
	}
	newPlace = moveIsValid(hv, PLAYER_DR_SEWARD, newPlace);
	return placeIdToAbbrev(newPlace);
}

const char * decideVanHelsingMove(HunterView hv) {
	PlaceId currPlace = HvGetPlayerLocation(hv, PLAYER_VAN_HELSING);
	//Round currRound = HvGetRound(hv);

	if (currPlace == NOWHERE) return "PA";

	PlaceId newPlace = getMove(hv, PLAYER_VAN_HELSING);
	printf("currPlace = %s newPlace = %s\n", placeIdToName(currPlace), placeIdToName(newPlace));
	if (newPlace == NOWHERE) {
		if (HvGetRound(hv) == 1) return "MA";

		if (currPlace == MEDITERRANEAN_SEA) {
			int numLocs = 0;
			bool canFree;
			PlaceId * HelsingLocs = HvGetLastLocations(hv, PLAYER_VAN_HELSING, 10, &numLocs, &canFree);
			for(int i = 0; i < numLocs; i++) {
				printf("locs = %s\n", placeIdToName(HelsingLocs[i]));
			}
			for (int i = 0; i < numLocs; i++) {
				if (HelsingLocs[i] == CAGLIARI)
					break;
				if ((i == numLocs - 1) && hasSeaMove(hv, 6))
					return "CG";
			}
			if (canFree && numLocs > 0)
				free(HelsingLocs);
		}

		int pathLength;
		PlaceId *shortestPath = HvGetShortestPathTo(hv, PLAYER_VAN_HELSING, MADRID, &pathLength);
	    printf("path = %d\n", pathLength);
		if (pathLength > 2 || placeIsSea(currPlace)) {
			newPlace = shortestPath[0];
		} else {
			int numReturnedLocs;
			bool sea = (hasSeaMove(hv, 6) || placeIsSea(currPlace));
			PlaceId * places = HvWhereCanIGoByType(hv, true, true, sea, &numReturnedLocs);
			int numLocs = numReturnedLocs;
			PlaceId * removedPlaces = removeSameLocations(hv, PLAYER_VAN_HELSING, places, &numLocs);
			if (numLocs > 0) {
				free(places);
				places = removedPlaces;
				numReturnedLocs = numLocs;
			}
			if (placeIsSea(currPlace))
				numReturnedLocs--;
			newPlace = getRandomLocation(hv, places, PLAYER_VAN_HELSING, numReturnedLocs);
			free(places);
		}
		if (pathLength > 0)
			free(shortestPath);
		
		newPlace = getVampire(hv, PLAYER_VAN_HELSING, newPlace);
	}
	newPlace = moveIsValid(hv, PLAYER_VAN_HELSING, newPlace);
	return placeIdToAbbrev(newPlace);
}

const char * decideMinaHarkerMove(HunterView hv) {
	PlaceId currPlace = HvGetPlayerLocation(hv, PLAYER_MINA_HARKER);
	if (currPlace == NOWHERE) 
		return "CD";
	
	Round round = HvGetRound(hv);
	Round knownDraculaRound = -1;
	HvGetLastKnownDraculaLocation(hv, &knownDraculaRound);
	PlaceId newPlace = CASTLE_DRACULA;
	if (knownDraculaRound != -1 && round - knownDraculaRound <= 6) {
		newPlace = getMove(hv, PLAYER_MINA_HARKER);
		printf("%s %s\n", placeIdToName(currPlace),placeIdToName(newPlace));
		int pathLength;
		PlaceId *shortestPath = HvGetShortestPathTo(hv, PLAYER_MINA_HARKER, newPlace, &pathLength);
		// if dracula is too far away, keep staying at castle
		if (pathLength > 4) {
			free(shortestPath);
			shortestPath = HvGetShortestPathTo(hv, PLAYER_MINA_HARKER,CASTLE_DRACULA, &pathLength);
		}
		if (pathLength > 0) {
			newPlace = shortestPath[0];
			free (shortestPath);
		} else if (currPlace == CASTLE_DRACULA)
			newPlace = GALATZ;
		else
			newPlace = currPlace;
	} else {
		int pathLength;
		PlaceId * shortestPath = HvGetShortestPathTo(hv, PLAYER_MINA_HARKER, CASTLE_DRACULA, &pathLength);
		if (pathLength > 1){
			newPlace = shortestPath[0];
			free(shortestPath);
		} else {
			if (pathLength != 0)
				free(shortestPath);
			int numReturnedLocs = 0;
			PlaceId * places = HvWhereCanIGoByType(hv, true, false, false, &numReturnedLocs);
			int numLocs = numReturnedLocs;
			PlaceId * removedPlaces = removeSameLocations(hv, PLAYER_MINA_HARKER, places, &numLocs);
			if (numLocs > 0) {
				free(places);
				places = removedPlaces;
				numReturnedLocs = numLocs;
			}
			newPlace = getRandomLocation(hv, places, PLAYER_MINA_HARKER, numReturnedLocs);
			free(places);
		}
		
	}
	newPlace = getVampire(hv, PLAYER_MINA_HARKER, newPlace);
	newPlace = moveIsValid(hv, PLAYER_MINA_HARKER, newPlace);
	return placeIdToAbbrev(newPlace);
}



PlaceId getMove(HunterView hv, Player player) {
	PlaceId currPlace = HvGetPlayerLocation(hv, player);
	Round round = HvGetRound(hv);
	Round knownDraculaRound = -1;
	PlaceId knownDraculaLocation = HvGetLastKnownDraculaLocation(hv, &knownDraculaRound);
	printf("%s %s\n", placeIdToName(currPlace), placeIdToName(knownDraculaLocation));
	printf("%d %d\n", round, knownDraculaRound);
	if (knownDraculaRound == -1 || round - knownDraculaRound > 6) {
		if(round - knownDraculaRound > 10) {
			// losing dracula for a long time -> research
			return currPlace;
		}
		PlaceId newPlace = NOWHERE;
		int numDracMoves = 0;
		bool canFreeMoves;
		PlaceId* DraculaMoves = HvGetLastMoves(hv, PLAYER_DRACULA, 4, &numDracMoves, &canFreeMoves);
		if (planToTeleport(DraculaMoves, numDracMoves, NOWHERE)) {
			int pathLength;
			PlaceId *shortestPath = HvGetShortestPathTo(hv, player, CASTLE_DRACULA, &pathLength);
			if (pathLength > 0) {
				newPlace = shortestPath[0];
				free(shortestPath);
			} else
				newPlace = currPlace;
		}
		if (canFreeMoves && DraculaMoves != NULL)
			free(DraculaMoves);
		// let hunters randomly pick a place
		return newPlace;
	} else if (round == knownDraculaRound + 1) { 
		// dracula's current place is known
		int pathLength;
		PlaceId *shortestPath = HvGetShortestPathTo(hv, player, knownDraculaLocation, &pathLength);
		if (pathLength == 0)  // hunter is in the same city with dracula
			return currPlace;
		else { // move to the city with dracula
			PlaceId newPlace = shortestPath[0];
			free(shortestPath);
			return newPlace;
		}
	} else if (round - knownDraculaRound >= 2 && round - knownDraculaRound <= 6) {
		//PlaceId DraculaCurrPlace = HvGetPlayerLocation(hv, PLAYER_DRACULA);
		int numMoves = round - knownDraculaRound;
		int numDracLocs = 0;
		bool canFreeLocs;
		PlaceId* DraculaLocs = HvGetLastLocations(hv, PLAYER_DRACULA, numMoves, &numDracLocs, &canFreeLocs);
		bool boat = false;
		bool road = true;
		PlaceId newPlace = knownDraculaLocation;
		for (int i = 0; i < numDracLocs - 1 && i < 3 ; i++) {
			//printf("move = %s\n", placeIdToName(DraculaMoves[i + 1]));
			boat = (DraculaLocs[i + 1] == SEA_UNKNOWN || DraculaLocs[i] == SEA_UNKNOWN);
			road = (DraculaLocs[i + 1] != SEA_UNKNOWN);
			//printf("%d %d\n", road, boat);
			int numLocs;
			PlaceId * places = HvGetReachableByType(hv, PLAYER_DRACULA, knownDraculaRound + i, newPlace, road, false, boat, &numLocs);
			int possibleNumLocs = numLocs;	
			PlaceId * possiblePlaces  = removeImpossibleLocations(hv, PLAYER_DRACULA, places, newPlace, &possibleNumLocs);	
			if (DraculaLocs[i] == SEA_UNKNOWN && road) {
				possiblePlaces = removeBoat(possiblePlaces, &possibleNumLocs);
			} else if (DraculaLocs[i + 1] == SEA_UNKNOWN) {
				possiblePlaces = removeRoad(possiblePlaces, &possibleNumLocs);
			}
			if (possibleNumLocs > 0) {
				free(places);
				places = possiblePlaces;
				numLocs = possibleNumLocs;
			} 
			for(int i = 0; i < numLocs; i++) {
				printf("%s\n", placeIdToName(places[i]));
			}	
			printf("\n");
			newPlace = getRandomLocation(hv, places, player, numLocs);
			//printf("newplace = %s %d\n\n", placeIdToName(newPlace), i);
			free (places);
		}
		if (canFreeLocs && DraculaLocs != NULL)	
			free (DraculaLocs);
		// if there is evidence that dracula want to teleport, set the destination to castle
		int numDracMoves = 0;
		bool canFreeMoves;
		PlaceId* DraculaMoves = HvGetLastMoves(hv, PLAYER_DRACULA, 4, &numDracMoves, &canFreeMoves);
		if (planToTeleport(DraculaMoves, numDracMoves, newPlace)) {
			int pathLength;
			PlaceId *shortestPath = HvGetShortestPathTo(hv, player, newPlace, &pathLength);
			if (pathLength == 1) {
				free(shortestPath);
				return newPlace;
			} else {
				newPlace = CASTLE_DRACULA;
				if (pathLength > 0)
					free(shortestPath);
			}
		}
			
		if (canFreeMoves && DraculaMoves != NULL)
			free(DraculaMoves);
		// if the player is Mina Harker, we need to consideration related the path length
		// return the predict dracula place directly for further decision
		if (player == PLAYER_MINA_HARKER)
			return newPlace;
		int pathLength;
		PlaceId *shortestPath = HvGetShortestPathTo(hv, player, newPlace, &pathLength);
		//for(int i = 0; i < pathLength; i++) {
		//	printf("%s\n", placeIdToName(shortestPath[i]));
		//}	
		//printf("\n");
		if (pathLength > 0) {
			newPlace = shortestPath[0];
			free(shortestPath);
			return newPlace;
		} else 
			return currPlace;	
		
	}
	return NOWHERE;
}

PlaceId getRandomLocation(HunterView hv, PlaceId * places, Player player, int numReturnedLocs) { 
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

PlaceId * removeSameLocations(HunterView hv, Player player, PlaceId * places, int * numReturnedLocs) {
	//printf("hello///////////////\n");
	PlaceId * newPlaces = malloc(*numReturnedLocs * sizeof(PlaceId));
	int num = 0;
	for(int i = 0; i < *numReturnedLocs; i++) {
		bool areSame = false;
		int numLocs = 0;
		bool canFree = true;
		PlaceId * lastLocs = NULL;
		for (int j = 0; j < 4; j++) {
			numLocs = 0;
			canFree = true;
			lastLocs = HvGetLastLocations(hv, j, 2, &numLocs, &canFree);
			if (numLocs == 2) {
				//printf("place = %s loc1 = %s loc2 = %s\n",placeIdToName(places[i]), placeIdToName(lastLocs[0]), placeIdToName(lastLocs[1]));
				if (player == j) {
					if (lastLocs[0] == places[i]) {
						areSame = true;
						break;
					}
				} else {
					if (lastLocs[0] == places[i] || lastLocs[1] == places[i]) {
						areSame = true;
						break;
					}
				}
			}
		}
		if (!areSame) {
			newPlaces[num] = places[i];
			num++;
		}
		if (canFree && numLocs > 0)
			free(lastLocs);
	}
	*numReturnedLocs = num;
	//free(places);
	return newPlaces;
}
PlaceId * removeImpossibleLocations(HunterView hv, Player player, PlaceId * places, PlaceId currPlace, int * numReturnedLocs) {

	PlaceId * availablePlace = malloc(*numReturnedLocs * sizeof(PlaceId));
	int num = 0;
	for(int i = 0; i < *numReturnedLocs; i++) {
		if (placeIsSea(currPlace) && places[i] == currPlace)
			continue;
		if(canGo(hv, places[i])) {
			int hasOtherHunter = 0;
			for(int j = 0; j < player; j++) {
				if(HvGetPlayerLocation(hv, j) == places[i] && placeIsLand(places[i]))
					hasOtherHunter = 1;
			}
			if(!hasOtherHunter && places[i] != ST_JOSEPH_AND_ST_MARY){
				availablePlace[num] = places[i];
				num++;
			}
		}
	}
	*numReturnedLocs = num;
	//free(places);
	return availablePlace;
}

PlaceId * removeBoat(PlaceId * places, int * numReturnedLocs) {
	
	PlaceId * newPlaces = malloc(*numReturnedLocs * sizeof(PlaceId));
	int num = 0;
	for(int i = 0; i < *numReturnedLocs; i++) {
		if(placeIsLand(places[i])) {
			newPlaces[num] = places[i];
			num++;
		}
	}
	*numReturnedLocs = num;
	free(places);

	return newPlaces;
}

PlaceId * removeRoad(PlaceId * places, int * numReturnedLocs) {
	PlaceId * newPlaces = malloc(*numReturnedLocs * sizeof(PlaceId));
	int num = 0;
	for(int i = 0; i < *numReturnedLocs; i++) {
		if(placeIsSea(places[i])) {
			newPlaces[num] = places[i];
			num++;
		}
	}
	*numReturnedLocs = num;
	free(places);
	return newPlaces;
}


PlaceId moveIsValid(HunterView hv, Player player, PlaceId newPlace) {
	int numLocs;
	PlaceId * places = HvWhereCanTheyGo(hv, player, &numLocs);
	bool isValid = false;
	for (int i = 0; i < numLocs; i++) {
		if (newPlace == places[i])
			isValid = true;
	}
	if (!isValid) {
		srand(time(0));
		newPlace = places[rand() % numLocs];
	}
	free(places);
	return newPlace;
}

bool placeHasOtherHunter(HunterView hv, Player player, PlaceId place) {
	for(int i = 0; i < player; i++) {
		if(HvGetPlayerLocation(hv, i) == place)
			return true;
	}
	if(HvGetPlayerLocation(hv, PLAYER_MINA_HARKER) == place)
		return true;
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
		if (placeIsSea(places[i]) || places[i] == SEA_UNKNOWN) {
			if (canFree)
				free(places);
			return true;
		}
			
	}
	if (canFree)
		free(places);
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
	}
	if (canFree)
		free(list); // free the location list
	return go;
}

bool planToTeleport(PlaceId * moves, int numMoves, PlaceId place) {

	int number = numMoves < 4 ? numMoves : 4;
	bool hide = false;
	bool doubleBack = false;
	for (int i = 0; i < number; i++) {
		if (moves[i] == HIDE)
			hide = true;
		else if (moves[i] >= DOUBLE_BACK_1 && moves[i] <= DOUBLE_BACK_3)
			doubleBack = true;
	}
	if (place == NOWHERE) {
		if ((moves[numMoves - 1]  == HIDE  || moves[numMoves - 1] == DOUBLE_BACK_1 
			|| moves[numMoves - 1] == DOUBLE_BACK_2) || (hide && doubleBack)) {
			
			return true;
		} else
			return false;
	} else {
		if (hide || doubleBack) {
			if (placeCanTeleport(place)) 
				return true;
			else
				return false;
		} else 
			return false;
	}
}

bool placeCanTeleport(PlaceId place) {
	if (place == EDINBURGH || place == MANCHESTER || place == NORTH_SEA ||
		place == LONDON || place == PLYMOUTH || place == SWANSEA ||
		place == IRISH_SEA || place == DUBLIN || place == GALWAY || 
		place == ATLANTIC_OCEAN || place == TYRRHENIAN_SEA ||
		place == MEDITERRANEAN_SEA || place == CAGLIARI || 
		place == ATHENS || place == VALONA || place == IONIAN_SEA) {

		return true;
	}
	return false;
}


PlaceId getVampire(HunterView hv, Player player, PlaceId place) {
	PlaceId newPlace = place;
	PlaceId vampirePlace = HvGetVampireLocation(hv);
	if (placeIsReal(vampirePlace)) {
		int pathLength;
		PlaceId * shortestPath = HvGetShortestPathTo(hv, player, vampirePlace, &pathLength);
		if (pathLength <= 3 && pathLength > 0) 
			newPlace = shortestPath[0];
		if (pathLength > 0)
			free(shortestPath);
	}
	return newPlace;
}
