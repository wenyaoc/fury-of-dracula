////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula 6/8
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

PlaceId England[] = {
	GALWAY,
	DUBLIN,
	SWANSEA,
	LONDON,
	PLYMOUTH,
	MANCHESTER,
	LIVERPOOL,
	EDINBURGH,
	IRISH_SEA
};

PlaceId Spanish[] = {
	MADRID,
	LISBON,
	CADIZ,
	GRANADA,
	ALICANTE,
	SARAGOSSA,
	SANTANDER,
	BARCELONA
};

PlaceId Italy[] = {
	GENOA,
	VENICE,
	FLORENCE,
	ROME,
	NAPLES,
	BARI
};

PlaceId Castle[] = {
	CASTLE_DRACULA,
	KLAUSENBURG,
	GALATZ,
	BUCHAREST,
	CONSTANTA,
	VARNA,
	SOFIA,
	SARAJEVO,
	BELGRADE,
	ZAGREB, 
	SZEGED, 
	HOSPITAL_PLACE,
	BUDAPEST,
	SALONICA,
	VALONA,
	ATHENS,
	BLACK_SEA,
	IONIAN_SEA
};

PlaceId WestEurope[] = {
	LE_HAVRE,
	NANTES,
	BRUSSELS,
	AMSTERDAM,
	HAMBURG,
	COLOGNE,
	BERLIN,
	PARIS,
	BORDEAUX,
	NORTH_SEA,
	ENGLISH_CHANNEL,
};

PlaceId SouthCastle[] = {
	VARNA,
	SOFIA,
	SARAJEVO,
	SALONICA,
	VALONA,
	ATHENS
};

const char * decideLordGodalmingMove(HunterView hv);
const char * decideDrSewardMove(HunterView hv);
const char * decideVanHelsingMove(HunterView hv);
const char * decideMinaHarkerMove(HunterView hv);
PlaceId getMove(HunterView hv, Player player);
PlaceId getVampire(HunterView hv, Player player, PlaceId place);
PlaceId getRandomLocation(HunterView hv, PlaceId * places, Player player, int numReturnedLocs);
PlaceId getCloestLocation(HunterView hv, PlaceId * places, Player player, int numReturnedLocs);
PlaceId * removeSameLocations(HunterView hv, Player player, PlaceId * places, int * numReturnedLocs);
PlaceId * removeImpossibleLocations(HunterView hv, Player player, PlaceId * places, PlaceId currPlace, int * numReturnedLocs);
PlaceId * removeBoat(PlaceId * places, int * numReturnedLocs);
PlaceId * removeRoad(PlaceId * places, int * numReturnedLocs);
PlaceId moveIsValid(HunterView hv, Player player, PlaceId newPlace);
PlaceId searchingAround(HunterView hv, Player player, PlaceId centerPlace, int dist);

PlaceId findDracFromSpanish(HunterView hv, Player player, PlaceId knownDraculaLocation, Round knownDraculaRound);
PlaceId findDracFromItaly(HunterView hv, Player player, PlaceId knownDraculaLocation, Round knownDraculaRound);
PlaceId findDracFromEngland(HunterView hv, Player player, PlaceId knownDraculaLocation, Round knownDraculaRound);
PlaceId findDracFromCastle(HunterView hv, Player player, PlaceId knownDraculaLocation, Round knownDraculaRound);
PlaceId findDracFromWestEurope(HunterView hv, Player player, PlaceId knownDraculaLocation, Round knownDraculaRound);
PlaceId findDracFromMiddleEurope(HunterView hv, Player player, PlaceId knownDraculaLocation, Round knownDraculaRound);
//PlaceId findDracFromSea(HunterView hv, Player player, PlaceId knownDraculaLocation, Round knownDraculaRound);
int getValidSeaMove(HunterView hv, int numRound);
int getValidMove(HunterView hv, int numRound);

bool hasRailConnection(PlaceId place);
bool hasSeaMove(HunterView hv, int numMoves);
bool placeHasOtherHunter(HunterView hv, PlaceId place);
bool countryHasOtherHunter(HunterView hv, Player player, PlaceId *places, int num);
bool canGo(HunterView hv, PlaceId place);
bool planToTeleport(PlaceId * moves, int numMoves, PlaceId place);
bool placeCanTeleport(PlaceId place);
bool hasSpecialMove(HunterView hv, int numMoves);
bool hasLotOfSeaMove(HunterView hv);
bool inIsland(PlaceId newPlace);
bool inCountry(PlaceId * places, PlaceId newPlace, int num);
bool wentBefore(HunterView hv, PlaceId place);


void decideHunterMove(HunterView hv) {
	Player player = HvGetPlayer(hv);
	//printf("%d\n", player);
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
	//printf("%d currPlace = %s newPlace = %s\n", currRound, placeIdToName(currPlace), placeIdToName(newPlace));
	//printf("newplace = %s\n", placeIdToName(newPlace));
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
		//PlaceId currPlace = HvGetPlayerLocation(hv, PLAYER_LORD_GODALMING);
		Round knownDraculaRound = -1;
		HvGetLastKnownDraculaLocation(hv, &knownDraculaRound);
		PlaceId centerPlace;

		if (currRound > 20 && ((currRound - knownDraculaRound) > 6 && (currRound - knownDraculaRound) < 16))
			centerPlace = LIVERPOOL;
		else
			centerPlace = COLOGNE;

		newPlace = searchingAround(hv, PLAYER_LORD_GODALMING, centerPlace, 2);	
		newPlace = getVampire(hv, PLAYER_LORD_GODALMING, newPlace);

		if (HvGetHealth(hv, PLAYER_LORD_GODALMING) <= 2) newPlace = currPlace;
	}
	newPlace = moveIsValid(hv, PLAYER_LORD_GODALMING, newPlace);	
	return placeIdToAbbrev(newPlace);
}

const char * decideDrSewardMove(HunterView hv) {
	PlaceId currPlace = HvGetPlayerLocation(hv, PLAYER_DR_SEWARD);
	Round currRound = HvGetRound(hv);
	if (currPlace == NOWHERE) return "BD";


	PlaceId newPlace = getMove(hv, PLAYER_DR_SEWARD);
	//printf("currPlace = %s newPlace = %s\n", placeIdToName(currPlace), placeIdToName(newPlace));
	if (newPlace == NOWHERE) {
		if (HvGetRound(hv) == 1) return "VE";

		Round knownDraculaRound = -1;
		HvGetLastKnownDraculaLocation(hv, &knownDraculaRound);
		PlaceId centerPlace;
		//printf("%s %s\n", placeIdToName(currPlace), placeIdToName(knownDraculaLocation));
		//printf("%d %d %d\n", currRound, knownDraculaRound, currRound - knownDraculaRound);

		if (currRound % 26 >= 1 && currRound % 26 <= 5)
			centerPlace = MUNICH;
		else if (currRound % 26 > 5 && currRound % 26 <= 15)
			centerPlace = NAPLES;
		else
			centerPlace = GENEVA;
		
		newPlace = searchingAround(hv, PLAYER_DR_SEWARD, centerPlace, 1);
		
		newPlace = getVampire(hv, PLAYER_DR_SEWARD, newPlace);
		if (HvGetHealth(hv, PLAYER_DR_SEWARD) <= 2) newPlace = currPlace;
	}
	newPlace = moveIsValid(hv, PLAYER_DR_SEWARD, newPlace);
	return placeIdToAbbrev(newPlace);
}

const char * decideVanHelsingMove(HunterView hv) {
	PlaceId currPlace = HvGetPlayerLocation(hv, PLAYER_VAN_HELSING);
	//Round currRound = HvGetRound(hv);

	if (currPlace == NOWHERE) return "PA";

	PlaceId newPlace = getMove(hv, PLAYER_VAN_HELSING);
	//printf("currPlace = %s newPlace = %s\n", placeIdToName(currPlace), placeIdToName(newPlace));
	if (newPlace == NOWHERE) {
		if (HvGetRound(hv) == 1) return "MA";
		//if (HvGetRound(hv) == 2 && currPlace == MADRID) return "GR";

		if (currPlace == MEDITERRANEAN_SEA) {
			int numLocs = 0;
			bool canFree;
			PlaceId * HelsingLocs = HvGetLastLocations(hv, PLAYER_VAN_HELSING, 10, &numLocs, &canFree);
			//for(int i = 0; i < numLocs; i++) {
			//	printf("locs = %s\n", placeIdToName(HelsingLocs[i]));
			//}
			for (int i = 0; i < numLocs; i++) {
				if (HelsingLocs[i] == CAGLIARI)
					break;
				if ((i == numLocs - 1) && hasSeaMove(hv, 6))
					return "CG";
			}
			if (canFree && numLocs > 0)
				free(HelsingLocs);
		}
		newPlace = searchingAround(hv, PLAYER_VAN_HELSING, CADIZ, 1);
		if (HvGetHealth(hv, PLAYER_VAN_HELSING) <= 2) newPlace = currPlace;
	}
	newPlace = moveIsValid(hv, PLAYER_VAN_HELSING, newPlace);
	return placeIdToAbbrev(newPlace);
}

const char * decideMinaHarkerMove(HunterView hv) {
	PlaceId currPlace = HvGetPlayerLocation(hv, PLAYER_MINA_HARKER);
	if (currPlace == NOWHERE) 
		return "CD";

	PlaceId newPlace = getMove(hv, PLAYER_MINA_HARKER);
	//printf("currPlace = %s newPlace = %s\n", placeIdToName(currPlace), placeIdToName(newPlace));
	if (newPlace == NOWHERE) {
		int pathLength;
		PlaceId * shortestPath = HvGetShortestPathTo(hv, PLAYER_MINA_HARKER, CASTLE_DRACULA, &pathLength);
		//for(int i = 0; i < pathLength; i++) {
		//	printf("->%s\n", placeIdToName(shortestPath[i]));
		//}	
		//printf("\n");
		if (pathLength > 0){
			newPlace = shortestPath[0];
			free(shortestPath);
		} else {
			if (pathLength != 0)
				free(shortestPath);
			int numReturnedLocs = 0;
			bool sea = placeIsSea(currPlace);
			PlaceId * places = HvWhereCanIGoByType(hv, true, false, sea, &numReturnedLocs);
			int numLocs = numReturnedLocs;
			PlaceId * removedPlaces = removeSameLocations(hv, PLAYER_MINA_HARKER, places, &numLocs);
			if (numLocs > 0) {
				free(places);
				places = removedPlaces;
				numReturnedLocs = numLocs;
			}
			newPlace = getCloestLocation(hv, places, PLAYER_MINA_HARKER, numReturnedLocs);
			free(places);
		}
		newPlace = getVampire(hv, PLAYER_MINA_HARKER, newPlace);
		if (HvGetHealth(hv, PLAYER_MINA_HARKER) <= 2) newPlace = currPlace;
	}
	//printf("currPlace = %s newPlace = %s\n", placeIdToName(currPlace), placeIdToName(newPlace));
	newPlace = moveIsValid(hv, PLAYER_MINA_HARKER, newPlace);
	//printf("currPlace = %s newPlace = %s\n", placeIdToName(currPlace), placeIdToName(newPlace));
	return placeIdToAbbrev(newPlace);
}



PlaceId getMove(HunterView hv, Player player) {
	PlaceId currPlace = HvGetPlayerLocation(hv, player);
	Round round = HvGetRound(hv);
	Round knownDraculaRound = -1;
	PlaceId knownDraculaLocation = HvGetLastKnownDraculaLocation(hv, &knownDraculaRound);
	int validRound = getValidMove(hv, round - knownDraculaRound);
	printf("curr = %s knownDrac = %s\n", placeIdToName(currPlace), placeIdToName(knownDraculaLocation));
	printf("%d %d %d %d\n", round, knownDraculaRound, round - knownDraculaRound, validRound);
	if (round - knownDraculaRound > 10) {
		return currPlace;
	}	
	else if (knownDraculaRound == -1) 
		return NOWHERE;
	else if (round == knownDraculaRound + 1) { 
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
	} else if (validRound >= 1 && validRound < 3) {
		if (currPlace == NORTH_SEA && knownDraculaLocation == AMSTERDAM) {
			return EDINBURGH;
		} else if (currPlace == IONIAN_SEA && knownDraculaLocation == CONSTANTA) {
			if (getValidSeaMove(hv, round - knownDraculaRound) == 2) {
				if (player == 0 || player == 3) return VALONA;
				else if (player == 1) return ATHENS;
				else return SALONICA;
			}
		}
		//PlaceId DraculaCurrPlace = HvGetPlayerLocation(hv, PLAYER_DRACULA);
		int numMoves = round - knownDraculaRound;
		int numDracLocs = 0;
		bool canFreeLocs;
		PlaceId* DraculaLocs = HvGetLastLocations(hv, PLAYER_DRACULA, numMoves, &numDracLocs, &canFreeLocs);
		bool boat = false;
		bool road = true;
		PlaceId newPlace = knownDraculaLocation;
		for (int i = 0; i < numDracLocs - 1 && i < 2 && i < validRound; i++) {
			//printf("move = %s\n", placeIdToName(DraculaLocs[i + 1]));
			boat = (DraculaLocs[i + 1] == SEA_UNKNOWN || DraculaLocs[i] == SEA_UNKNOWN);
			road = (DraculaLocs[i + 1] != SEA_UNKNOWN);
			//printf("%d %d\n", road, boat);
			int numLocs;
			PlaceId * places = HvGetReachableByType(hv, PLAYER_DRACULA, knownDraculaRound + i, newPlace, road, false, boat, &numLocs);
			//for(int j = 0; j < numLocs; j++) {
			//	printf("possible = %s\n", placeIdToName(places[j]));
			//}	
			//printf("\n");
			int possibleNumLocs = numLocs;	
			PlaceId * possiblePlaces  = removeImpossibleLocations(hv, PLAYER_DRACULA, places, newPlace, &possibleNumLocs);
			//for(int j = 0; j < possibleNumLocs; j++) {
			//	printf("possible = %s\n", placeIdToName(possiblePlaces[j]));
			//}	
			//printf("\n");
			if (possibleNumLocs == 1 && i != 1 && i != validRound - 1) {
				//printf("hello\n");
				int pathLength;
				PlaceId *shortestPath = HvGetShortestPathTo(hv, player, possiblePlaces[0], &pathLength);
				if (pathLength > 0) {
					int numHunterLocs = 0;
					bool canFreeLocs;
					//printf("place = %s\n", placeIdToName(possiblePlaces[0]));
					PlaceId* locs = HvGetLastLocations(hv, player, 2, &numHunterLocs, &canFreeLocs);
					if (numHunterLocs == 2) {
						//printf("%s %s\n\n", placeIdToName(locs[0]), placeIdToName(locs[1]));
						if(possiblePlaces[0] != locs[0] && possiblePlaces[0] != locs[1]) {
							newPlace = possiblePlaces[0];
							//printf("new = %s\n", placeIdToName(newPlace));
							if (canFreeLocs)
								free(locs);
							free(shortestPath);
							break;
						}
					}
					if (numHunterLocs > 0 && canFreeLocs)
						free(locs);	
				}
				if (pathLength > 0)
					free(shortestPath);
				newPlace = possiblePlaces[i];
				continue;
			}	
			
			//printf("%s\n", placeIdToName(DraculaLocs[i + 1]));
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

			newPlace = getCloestLocation(hv, places, player, numLocs);
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
		//if (player == PLAYER_MINA_HARKER)
		//	return newPlace;

		int pathLength;
		PlaceId *shortestPath = HvGetShortestPathTo(hv, player, newPlace, &pathLength);
		//for(int i = 0; i < pathLength; i++) {
		//	printf("->%s\n", placeIdToName(shortestPath[i]));
		//}	
		//printf("\n");
		if (pathLength > 0) {
			newPlace = shortestPath[0];
			free(shortestPath);
			return newPlace;
		} else 
			return currPlace;	
		
	} else {
		if (inCountry(Spanish, knownDraculaLocation, 8)) 
			return findDracFromSpanish(hv, player, knownDraculaLocation, knownDraculaRound);
		else if (inCountry(England, knownDraculaLocation, 9))
			return findDracFromEngland(hv, player, knownDraculaLocation, knownDraculaRound);
		else if (inCountry(Italy, knownDraculaLocation, 6))
			return findDracFromItaly(hv, player, knownDraculaLocation, knownDraculaRound);
		else if (inCountry(Castle, knownDraculaLocation, 18))
			return findDracFromCastle(hv, player, knownDraculaLocation, knownDraculaRound);
		else if (inCountry(WestEurope, knownDraculaLocation, 11))
			return findDracFromWestEurope(hv, player, knownDraculaLocation, knownDraculaRound);
		else if (placeIsLand(knownDraculaLocation))
			return findDracFromMiddleEurope(hv, player, knownDraculaLocation, knownDraculaRound);
		
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
	if (placeHasOtherHunter(hv, newPlace)) {
		srand(time(0));
		newPlace = places[rand() % numReturnedLocs];
	}
	return newPlace;
}

PlaceId getCloestLocation(HunterView hv, PlaceId * places, Player player, int numReturnedLocs) {
	if (numReturnedLocs == 0 || numReturnedLocs == 1)
		return places[0];

	PlaceId src;
	if (player == PLAYER_LORD_GODALMING) src = GALWAY;
	else if (player == PLAYER_DR_SEWARD) {
		if (inCountry(Castle, HvGetPlayerLocation(hv, player), 17)) {
			src = BLACK_SEA;
		} else
			src = BERLIN;
	}
	else if (player == PLAYER_VAN_HELSING) {
		if (inCountry(Castle, HvGetPlayerLocation(hv, player), 17)) {
			src = ATHENS;
		} else
			src = CADIZ;
	}
	else {
		if (inCountry(Castle, HvGetPlayerLocation(hv, player), 17)) {
			src = CASTLE_DRACULA;
		} else 
			src = BLACK_SEA;
	}
	int * dest = HvGetShortestPathToAll(hv, src);

	int cloest = dest[places[0]];
	PlaceId cloestPlace;
	int totalNumber = 1;
	for (int i = 0; i < numReturnedLocs; i++) {
		printf("%s %d\n", placeIdToName(places[i]), dest[places[i]]);
		if (dest[places[i]] < cloest) {
			
			cloest = dest[places[i]];
			cloestPlace = places[i];
			totalNumber = 1;
		}
		if (dest[places[i]] == cloest)
			totalNumber++;
	}
	if (totalNumber == 1) {
		free(dest);
		return cloestPlace;
	} else {
		int newNum = 0;
		PlaceId * newPlaces = malloc(totalNumber * sizeof(PlaceId));
		for (int i = 0; i < numReturnedLocs; i++) {
			if (dest[places[i]] == cloest) {
				newPlaces[newNum] = places[i];
				newNum++;
			}
		}
		//printf("%d\n", newNum);
		free(dest);
		return getRandomLocation(hv, newPlaces, player, newNum);
	}
}


PlaceId * removeSameLocations(HunterView hv, Player player, PlaceId * places, int * numReturnedLocs) {
	//printf("hello///////////////\n")
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
			lastLocs = HvGetLastLocations(hv, j, 4, &numLocs, &canFree);
			//for(int i = 0; i < numLocs; i++) {
			//	printf(" place = %s\n", placeIdToName(lastLocs[i]));
		//	}	
		//	printf("\n");
			if (numLocs >= 2) {
				if (player != j) {
					if (lastLocs[numLocs - 1] == places[i] || lastLocs[numLocs - 2] == places[i]) {
						areSame = true;
						break;
					}
				} else {
					for (int k = 0; k < numLocs; k++) {
						//printf("known = %s\n", placeIdToName(lastLocs[k]));
						if (lastLocs[k] == places[i]) {
							
							areSame = true;
							break;
						}
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
	//printf("curr = %s\n", placeIdToName(currPlace));	
	PlaceId * availablePlace = malloc(*numReturnedLocs * sizeof(PlaceId));
	int num = 0;
	//printf("num1 = %d\n", *numReturnedLocs);
	for(int i = 0; i < *numReturnedLocs; i++) {
		if (places[i] == currPlace)
			continue;
		if(canGo(hv, places[i])) {
			int hasOtherHunter = 0;
			for(int j = 0; j < 4; j++) {
				int numLocs = 0;
				bool canFree = true;
				PlaceId * lastLocs = HvGetLastLocations(hv, j, 2, &numLocs, &canFree);
				if (numLocs == 2) {
					//printf("%s %s %s\n", placeIdToName(lastLocs[0]), placeIdToName(lastLocs[1]), placeIdToName(places[i]));
					if ((lastLocs[0] == places[i] || lastLocs[1] == places[i]) && placeIsLand(places[i]))
						hasOtherHunter = true;
				}
				if (numLocs > 0 && canFree)
					free(lastLocs);
			}
			//printf("%d\n", hasOtherHunter);
			if(!hasOtherHunter && places[i] != ST_JOSEPH_AND_ST_MARY && places[i] != CASTLE_DRACULA){
				//printf("hello\n");
				availablePlace[num] = places[i];
				num++;
			}
		}
	}
	*numReturnedLocs = num;
	//printf("num2 = %d\n", *numReturnedLocs);
	return availablePlace;
}

PlaceId * removeBoat(PlaceId * places, int * numReturnedLocs) {
	
	PlaceId * newPlaces = malloc(*numReturnedLocs * sizeof(PlaceId));
	int num = 0;
	for(int i = 0; i < *numReturnedLocs; i++) {
		if(placeIsLand(places[i])) {
			//printf("land = %s\n", placeIdToName(places[i]));
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
	if (*numReturnedLocs > 0)
		free(places);
	*numReturnedLocs = num;
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

PlaceId searchingAround(HunterView hv, Player player, PlaceId centerPlace, int dist) {
	PlaceId currPlace = HvGetPlayerLocation(hv, player);
	//Round currRound = HvGetRound(hv);
	int pathLength;
	PlaceId *shortestPath;
	PlaceId newPlace = NOWHERE;
	if (placeIsLand(currPlace) && !inIsland(currPlace) && !inIsland(centerPlace))
		shortestPath = HvGetShortestPathWithoutBoatTo(hv, player, centerPlace, &pathLength);
	else 
		shortestPath = HvGetShortestPathTo(hv, player, centerPlace, &pathLength);

	//for(int i = 0; i < pathLength; i++) {
	//	printf(" place = %s\n", placeIdToName(shortestPath[i]));
	//}	
	//printf("\n");
	
	if (pathLength > dist || (placeIsSea(currPlace) && pathLength > 0)) {
		newPlace = shortestPath[0];
	} else {
		int numReturnedLocs;
		bool sea = (placeIsSea(currPlace));
		PlaceId * places = HvWhereCanIGoByType(hv, true, true, sea, &numReturnedLocs);
		int numLocs = numReturnedLocs;
		PlaceId * removedPlaces = removeSameLocations(hv, player, places, &numLocs);
		if (numLocs > 0) {
			free(places);
			places = removedPlaces;
			numReturnedLocs = numLocs;
		}
		if (placeIsSea(currPlace))
			numReturnedLocs--;
		newPlace = getRandomLocation(hv, places, player, numReturnedLocs);
		free(places);
	}
	if (pathLength > 0)
		free(shortestPath);
	return newPlace;
}



bool placeHasOtherHunter(HunterView hv, PlaceId place) {
	for(int i = 0; i < 4; i++) {
		if(HvGetPlayerLocation(hv, i) == place)
			return true;
	}
	return false;
}


bool countryHasOtherHunter(HunterView hv, Player player, PlaceId *places, int num) {
	for (int i = 0; i < num; i++) {
		for (int j = 0; j < 4; j++) {
			if (i != player && placeHasOtherHunter(hv, places[i])) {
				return true;
			}
		}
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
	//printf("%s\n", placeIdToName(place));
	//for (int i = 0; i < num; i ++) {
	//	printf("cango = %s\n", placeIdToName(list[i]));
	//}
	//printf("\n");
	for (int i = 0; i < num; i++) {	
		if (list[i] == place) // the place is already in the trail
			go = false;
	}
	if (canFree)
		free(list); // free the location list
	return go;
}

bool planToTeleport(PlaceId * moves, int numMoves, PlaceId place) {

	//for (int i = 0; i < numMoves; i++) {
	//	printf("%s\n", placeIdToName(moves[i]));
	//}

	int number = numMoves < 4 ? numMoves : 4;
	bool hide = false;
	bool doubleBack = false;
	for (int i = 0; i < number; i++) {
		if (moves[i] == HIDE)
			hide = true;
		else if (moves[i] >= DOUBLE_BACK_1 && moves[i] <= DOUBLE_BACK_3)
			doubleBack = true;
	}
	//printf("%d %d\n", hide, doubleBack);
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

bool hasSpecialMove(HunterView hv, int numMoves) {
	int numReturnedMoves;
	bool canFree;
	PlaceId * moves = HvGetLastMoves(hv, PLAYER_DRACULA, numMoves, &numReturnedMoves, &canFree);
	if (numReturnedMoves == 0)
		return false;	
	for(int i = 0; i < numReturnedMoves; i++) {
		if (moves[i] == HIDE || (moves[i] >= DOUBLE_BACK_1 && moves[i] <= DOUBLE_BACK_5)) {
			if (canFree)
				free(moves);
			return true;
		}
			
	}
	if (canFree)
		free(moves);
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

bool hasLotOfSeaMove(HunterView hv) {
	int number = 0;
	int numReturnedMoves = 0;
	bool canFree = true;
	PlaceId * places = HvGetLastMoves(hv, PLAYER_DRACULA, 5, &numReturnedMoves, &canFree);
	if (numReturnedMoves == 0)
		return false;	
	for(int i = 0; i < numReturnedMoves; i++) {
		if (placeIsSea(places[i]) || places[i] == SEA_UNKNOWN) {
			number++;
		}		
	}
	if (canFree)
		free(places);

	if (number >= 3)
		return true;
	else
		return false;
}


bool inIsland(PlaceId newPlace) {
	int flag = false;
	for(int i = 0; i < 8; i++) {
		//printf("%s %s\n",placeIdToName(England[i]), placeIdToName(newPlace));
		//printf("%s\n", placeIdToName(newPlace));
		if (England[i] == newPlace) {
			flag = true;
		}
		//printf("%s\n", placeIdToName(newPlace));
	}
	if (newPlace == CAGLIARI)
		flag = true;
	return flag;
}

bool inCountry(PlaceId * places, PlaceId newPlace , int num) {
	int flag = false;
	for(int i = 0; i < num; i++) {
		if (places[i] == newPlace) {
			flag = true;
		}
	}
	return flag;
}

PlaceId findDracFromSpanish(HunterView hv, Player player, PlaceId knownDraculaLocation, Round knownDraculaRound) {
	//printf("hello");
	Round currRound = HvGetRound(hv);
	int numSeaMove = getValidSeaMove(hv, currRound - knownDraculaRound);
	int centerPlace;
	if (numSeaMove == 0) {
		if (currRound - knownDraculaRound < 7) {
			if (player == PLAYER_LORD_GODALMING) {
				if (countryHasOtherHunter(hv, player, Spanish, 8)) centerPlace = NANTES;
				else centerPlace = MADRID;
			} else if (player == PLAYER_DR_SEWARD) {
				if (countryHasOtherHunter(hv, player, Spanish, 8)) centerPlace = MARSEILLES;
				else centerPlace = MADRID;
			} else if (player == PLAYER_VAN_HELSING) centerPlace = MADRID;
			else centerPlace = ZAGREB;
		} else {
			if (player == PLAYER_LORD_GODALMING) {
				if (countryHasOtherHunter(hv, player, Spanish, 8))  centerPlace = COLOGNE;
				else centerPlace = MADRID;
			} else if (player == PLAYER_DR_SEWARD) {
				if (countryHasOtherHunter(hv, player, Spanish, 8)) centerPlace = FLORENCE;
				else centerPlace = MADRID;
			} else if (player == PLAYER_VAN_HELSING) centerPlace = MADRID;
			else centerPlace = BUDAPEST;
		}
		int dist = 1;
		return searchingAround(hv, player, centerPlace, dist);
	} else {
		if(numSeaMove <= 2) {
			//printf("hello");
			if ((knownDraculaLocation == CADIZ && wentBefore(hv, LISBON)) || 
				(knownDraculaLocation == LISBON && wentBefore(hv, CADIZ))) {
				PlaceId currPlace = HvGetPlayerLocation(hv, player);
				if (currPlace == ATLANTIC_OCEAN) {
					if (player == PLAYER_LORD_GODALMING) centerPlace = LIVERPOOL;
					else if (player == PLAYER_DR_SEWARD) centerPlace = PARIS;
					else if (player == PLAYER_VAN_HELSING) centerPlace = SANTANDER;
					else centerPlace = CASTLE_DRACULA;
					//printf("center = %s\n", placeIdToName(centerPlace));
					return searchingAround(hv, player, centerPlace, 1);
				} else if (inCountry(England, currPlace, 9)) {
					if (player == PLAYER_LORD_GODALMING) centerPlace = LIVERPOOL;
					else if (player == PLAYER_DR_SEWARD ) {
						if (countryHasOtherHunter(hv, player, England, 9))
							centerPlace = HAMBURG;
						else centerPlace = LIVERPOOL;
					} 
					else if (player == PLAYER_VAN_HELSING)  {
						if (countryHasOtherHunter(hv, player, England, 9))
							centerPlace = LIVERPOOL;
						else centerPlace = SANTANDER;
					} 
					else centerPlace = CASTLE_DRACULA;
					return searchingAround(hv, player, centerPlace, 1);
				}
			} else if (player == PLAYER_LORD_GODALMING) {
				return searchingAround(hv, player, LIVERPOOL, 1);
			}
		}
	}
	return NOWHERE;
}


PlaceId findDracFromItaly(HunterView hv, Player player, PlaceId knownDraculaLocation, Round knownDraculaRound) {
	Round currRound = HvGetRound(hv);
	int numSeaMove = getValidSeaMove(hv, currRound - knownDraculaRound);
	if (numSeaMove == 0) {
		int centerPlace;
		if (player == PLAYER_LORD_GODALMING) centerPlace = STRASBOURG;
		else if (player == PLAYER_DR_SEWARD) centerPlace = NAPLES;
		else if (player == PLAYER_VAN_HELSING) centerPlace = GRANADA;
		else centerPlace = SZEGED;
		int dist = 2;
		return searchingAround(hv, player, centerPlace, dist);
	} else 
		return NOWHERE;
}


PlaceId findDracFromEngland(HunterView hv, Player player, PlaceId knownDraculaLocation, Round knownDraculaRound) {
	Round currRound = HvGetRound(hv);
	PlaceId currPlace = HvGetPlayerLocation(hv, player);
	int numSeaMove = getValidSeaMove(hv, currRound - knownDraculaRound);
	//printf("%d %s\n", numSeaMove, placeIdToName(knownDraculaLocation));
	if (numSeaMove == 0) {
		int centerPlace;
		if (player == PLAYER_LORD_GODALMING) centerPlace = LIVERPOOL;
		else if (player == PLAYER_DR_SEWARD) {
			if (countryHasOtherHunter(hv, player, England, 9)) centerPlace = COLOGNE;
			else centerPlace = LIVERPOOL;
		}else if (player == PLAYER_VAN_HELSING) centerPlace = SANTANDER;
		else centerPlace = SZEGED;
		int dist = 2;
		if (player == 2 || player == 3) dist = 1;
		//printf("%s\n", placeIdToName(centerPlace));
		return searchingAround(hv, player, centerPlace, dist);
	} else {
		if(numSeaMove == 1) {
			if(knownDraculaLocation == EDINBURGH) {
				if (currPlace == NORTH_SEA) {
					if (placeHasOtherHunter(hv, HAMBURG)) return AMSTERDAM;
					else return HAMBURG;
				} else if (currPlace == ENGLISH_CHANNEL) {
					if (placeHasOtherHunter(hv, LE_HAVRE)) return LONDON;
					else return LE_HAVRE;
				}
			}
			if (knownDraculaLocation == LIVERPOOL) {
				int validMove = getValidMove(hv, currRound - knownDraculaRound);
				//printf("%d %d", numSeaMove, validMove);
				if (validMove < 3) {
					if (currPlace == ATLANTIC_OCEAN) {
						if (placeHasOtherHunter(hv, CADIZ)) return LISBON;
						else return CADIZ;
					} else if (player == PLAYER_MINA_HARKER) 
						return searchingAround(hv, player, MADRID, 1);
				}
			}
		}
		int validMove = getValidMove(hv, currRound - knownDraculaRound);
		if (numSeaMove == 2 && (knownDraculaLocation == SWANSEA || knownDraculaLocation == DUBLIN) && validMove < 4) {
			
			//printf("Sea = %d all =  %d\n", numSeaMove, validMove);
			if (currPlace == ATLANTIC_OCEAN) {
				if (placeHasOtherHunter(hv, CADIZ)) return LISBON;
				else return CADIZ;
			} else if (player == PLAYER_VAN_HELSING) {
				
				return searchingAround(hv, player, MADRID, 1);
			}
			 
		}
		if (numSeaMove == 2 && validMove < 8) {
			int centerPlace;
			if (player == PLAYER_LORD_GODALMING) centerPlace = LIVERPOOL;
			else if (player == PLAYER_DR_SEWARD) {
				if (countryHasOtherHunter(hv, player, Spanish, 8)) centerPlace = MADRID;
				else centerPlace = LE_HAVRE;
			} else if (player == PLAYER_VAN_HELSING) {
				if (countryHasOtherHunter(hv, player, England, 8)) centerPlace = LIVERPOOL;
				centerPlace = MADRID;
			}
			else centerPlace = BUCHAREST;

			return searchingAround(hv, player, centerPlace, 1);
		}

	}
	//printf("hello");
	return NOWHERE;
}

PlaceId findDracFromCastle(HunterView hv, Player player, PlaceId knownDraculaLocation, Round knownDraculaRound) {
	Round currRound = HvGetRound(hv);
	PlaceId currPlace = HvGetPlayerLocation(hv, player);
	int numSeaMove = getValidSeaMove(hv, currRound - knownDraculaRound);
	if (numSeaMove == 0) {
		int centerPlace;
		if (currRound - knownDraculaRound < 7) {
			if (player == PLAYER_LORD_GODALMING) centerPlace = STRASBOURG;
			else if (player == PLAYER_DR_SEWARD) centerPlace = SOFIA;
			else if (player == PLAYER_VAN_HELSING) centerPlace = MILAN;
			else centerPlace = CASTLE_DRACULA;
		} else {
			if (player == PLAYER_LORD_GODALMING) centerPlace = STRASBOURG;
			else if (player == PLAYER_DR_SEWARD) centerPlace = SARAJEVO;
			else if (player == PLAYER_VAN_HELSING) centerPlace = LISBON;
			else centerPlace = CASTLE_DRACULA;
		}
		int dist = 2;
		if (player == 2 || player == 3) dist = 1;
		return searchingAround(hv, player, centerPlace, dist);
	} else {
		printf("numSea = %d\n", numSeaMove);
		if(numSeaMove == 3 || numSeaMove == 4) {
			if(currPlace == TYRRHENIAN_SEA) {
				if (player == 0 || player == 1) return ROME;
				else return NAPLES;
			} else if (currPlace == ADRIATIC_SEA) {
				if (player == 0 || player == 1) return BARI;
				else return VENICE;
			}
		}
		if (numSeaMove == 2) {
			//printf("%s\n", placeIdToName(currPlace));
			if (knownDraculaLocation == CONSTANTA || knownDraculaLocation == VARNA) {
				if (currPlace == CONSTANTA) return BLACK_SEA;
				if (currPlace == VARNA) return BLACK_SEA;
				if (currPlace == BLACK_SEA) return IONIAN_SEA;
				if (currPlace == IONIAN_SEA) {
					if (!placeHasOtherHunter(hv, VALONA)) return VALONA;
					else if (!placeHasOtherHunter(hv, ADRIATIC_SEA)) return ADRIATIC_SEA;
					else return SALONICA;
				}
				else {
					//printf("hello\n");
					return searchingAround(hv, player, VALONA, 1);
				}
			} else if (knownDraculaLocation == SOFIA || knownDraculaLocation == BUCHAREST || knownDraculaLocation == GALATZ) {
				if (!placeHasOtherHunter(hv, VALONA) && !placeHasOtherHunter(hv, ATHENS) && !placeHasOtherHunter(hv, SALONICA)) {
					if (player == PLAYER_DR_SEWARD) return searchingAround(hv, player, VALONA, 1);
					else if (player == PLAYER_VAN_HELSING) {
						if (countryHasOtherHunter(hv, player, Spanish, 8)) return searchingAround(hv, player, MADRID, 1);
						else return searchingAround(hv, player, NAPLES, 1);
					} else if (player == PLAYER_LORD_GODALMING) {
						if (countryHasOtherHunter(hv, player, Castle, 17)) return searchingAround(hv, player, ZAGREB, 1);
						return searchingAround(hv, player, VALONA, 1);
					}
				}
			}		
		}
		if (numSeaMove == 1) {
			 if (knownDraculaLocation == VALONA || knownDraculaLocation == ATHENS || knownDraculaLocation == SALONICA) {
				if (!placeHasOtherHunter(hv, VALONA) && !placeHasOtherHunter(hv, ATHENS) && !placeHasOtherHunter(hv, SALONICA)) {
					if (player == PLAYER_DR_SEWARD) return searchingAround(hv, player, VALONA, 1);
					else if (player == PLAYER_VAN_HELSING) {
						if (countryHasOtherHunter(hv, player, Spanish, 8)) return searchingAround(hv, player, MADRID, 1);
						else return searchingAround(hv, player, NAPLES, 1);
					} else if (player == PLAYER_LORD_GODALMING) {
						if (countryHasOtherHunter(hv, player, Castle, 17)) return searchingAround(hv, player, ZAGREB, 1);
						return searchingAround(hv, player, VALONA, 1);
					}
				}
			 }
		}
	}
	return NOWHERE;
}

PlaceId findDracFromWestEurope(HunterView hv, Player player, PlaceId knownDraculaLocation, Round knownDraculaRound) {
	//printf("%d known = %s ",player, placeIdToName(knownDraculaLocation));
	Round currRound = HvGetRound(hv);
	PlaceId currPlace = HvGetPlayerLocation(hv, player);
	int numSeaMove = getValidSeaMove(hv, currRound - knownDraculaRound);
	if (numSeaMove == 0) {
		int centerPlace;
		if (currRound - knownDraculaRound < 7) {
			if (player == PLAYER_LORD_GODALMING) centerPlace = LEIPZIG;
			else if (player == PLAYER_DR_SEWARD) centerPlace = ZURICH;
			else if (player == PLAYER_VAN_HELSING) centerPlace = SARAGOSSA;
			else centerPlace = KLAUSENBURG;
		} else {
			if (player == PLAYER_LORD_GODALMING) centerPlace = STRASBOURG;
			else if (player == PLAYER_DR_SEWARD) centerPlace = ROME;
			else if (player == PLAYER_VAN_HELSING) centerPlace = LISBON;
			else centerPlace = GALATZ;
		}
		int dist = 2;
		if (player == 3 || player == 2)
			dist = 1;
		return searchingAround(hv, player, centerPlace, dist);
	} else {
		int validMove = getValidMove(hv, currRound - knownDraculaRound);
		if (numSeaMove == 1 && validMove <= 4) {
			if (knownDraculaLocation == HAMBURG && currPlace == NORTH_SEA) {
				if (placeHasOtherHunter(hv, EDINBURGH)) return AMSTERDAM;
				else return EDINBURGH;
			} else if (knownDraculaLocation == AMSTERDAM && currPlace == NORTH_SEA) {
				if (placeHasOtherHunter(hv, EDINBURGH)) return HAMBURG;
				else return EDINBURGH;
			} else if (knownDraculaLocation == LE_HAVRE && currPlace == ENGLISH_CHANNEL) {
				if (placeHasOtherHunter(hv, LONDON)) return PLYMOUTH;
				else return LONDON;
			}
		}
		if((numSeaMove == 1 || numSeaMove == 2) && player == PLAYER_LORD_GODALMING)
			return searchingAround(hv, player, LIVERPOOL, 1);
			
	}
	return NOWHERE;
}

PlaceId findDracFromMiddleEurope(HunterView hv, Player player, PlaceId knownDraculaLocation, Round knownDraculaRound) {
	
	Round currRound = HvGetRound(hv);
	int numSeaMove = getValidSeaMove(hv, currRound - knownDraculaRound);
	int centerPlace;
	if (numSeaMove == 0) {
		if (currRound - knownDraculaRound < 7) {
			if (player == PLAYER_LORD_GODALMING) centerPlace = HAMBURG;
			else if (player == PLAYER_DR_SEWARD) centerPlace = VENICE;
			else if (player == PLAYER_VAN_HELSING) centerPlace = MADRID;
			else centerPlace = CASTLE_DRACULA;
		} else {
			if (player == PLAYER_LORD_GODALMING) centerPlace = BUCHAREST;
			else if (player == PLAYER_DR_SEWARD) centerPlace = ROME;
			else if (player == PLAYER_VAN_HELSING) centerPlace = LISBON;
			else centerPlace = CASTLE_DRACULA;
		}
		int dist = 2;
		//printf("%d center = %s\n",player, placeIdToName(centerPlace));
		if (player == 2 && player == 3) dist = 1;
		return searchingAround(hv, player, centerPlace, dist);
	} else {
		if (numSeaMove == 1) {
			if (player == PLAYER_LORD_GODALMING) centerPlace = LIVERPOOL;
			else if (player == PLAYER_DR_SEWARD) {
				if (!countryHasOtherHunter(hv, player, England, 9)) centerPlace = LIVERPOOL;
				else return NOWHERE;
			} else if (player == PLAYER_VAN_HELSING) {
				if (!countryHasOtherHunter(hv, player, England, 9)) centerPlace = LIVERPOOL;
				else return NOWHERE;
			} else return NOWHERE;

			int dist = 1;
			return searchingAround(hv, player, centerPlace, dist);
		}
	}
	return NOWHERE;
}
//PlaceId findDracFromSea(HunterView hv, Player player, PlaceId knownDraculaLocation, Round knownDraculaRound);



int getValidSeaMove(HunterView hv, int numRound) {

	if (numRound == 1) return 0;

	int numReturnedMoves = 0;
	bool canFreeMoves = true;
	PlaceId * moves = HvGetLastMoves(hv, PLAYER_DRACULA, numRound, &numReturnedMoves, &canFreeMoves);
	int numReturnedLocs = 0;
	bool canFreeLocs = true;
	PlaceId * locs = HvGetLastLocations(hv, PLAYER_DRACULA, numRound, &numReturnedLocs, &canFreeLocs);
	
	//printf("%d %d\n", numReturnedLocs, numReturnedMoves);
	if (numReturnedLocs != numReturnedMoves) {
		if (canFreeLocs && numReturnedLocs > 0)
			free(locs);
		if (canFreeMoves && numReturnedMoves > 0)
			free(moves);
		return 0;
	}
	int doubleBackNum = 0;
	int roundInSea = 0;
	int * round = calloc(numReturnedMoves, sizeof(int));
	for (int i = 1; i < numReturnedMoves; i++) {
		//printf("%s %s\n", placeIdToName(locs[i]),placeIdToName(moves[i]));
		if (locs[i] == SEA_UNKNOWN && moves[i] == SEA_UNKNOWN) {
			roundInSea = roundInSea + 1;
		} else if (moves[i] >= DOUBLE_BACK_1 && moves[i] <= DOUBLE_BACK_5) { 
			// double back
			doubleBackNum = moves[i] - DOUBLE_BACK_1;
			if (doubleBackNum > i && locs[i] == SEA_UNKNOWN)
				roundInSea = roundInSea + 1;
			else 
				roundInSea = round[i - doubleBackNum - 1];
		}
		round[i] = roundInSea;
	}
	if (canFreeLocs && numReturnedLocs > 0)
			free(locs);
	if (canFreeMoves && numReturnedMoves > 0)
			free(moves);
	return roundInSea;
}



int getValidMove(HunterView hv, int numRound) {

	int numReturnedMoves = 0;
	bool canFreeMoves = true;
	PlaceId * moves = HvGetLastMoves(hv, PLAYER_DRACULA, numRound, &numReturnedMoves, &canFreeMoves);
	int numReturnedLocs = 0;
	bool canFreeLocs = true;
	PlaceId * locs = HvGetLastLocations(hv, PLAYER_DRACULA, numRound, &numReturnedLocs, &canFreeLocs);
	
	//printf("%d %d\n", numReturnedLocs, numReturnedMoves);
	if (numReturnedLocs != numReturnedMoves) {
		if (canFreeLocs && numReturnedLocs > 0)
			free(locs);
		if (canFreeMoves && numReturnedMoves > 0)
			free(moves);
		return 0;
	}
	int doubleBackNum = 0;
	int validRound = 0;
	int * round = calloc(numReturnedMoves, sizeof(int));
	for (int i = 0; i < numReturnedMoves; i++) {
		if ((locs[i] == SEA_UNKNOWN && moves[i] == SEA_UNKNOWN) || (locs[i] == CITY_UNKNOWN && moves[i] == CITY_UNKNOWN)) {
			validRound = validRound + 1;
		} else if (moves[i] >= DOUBLE_BACK_1 && moves[i] <= DOUBLE_BACK_5) { 
			// double back
			doubleBackNum = moves[i] - DOUBLE_BACK_1;
			if (doubleBackNum > i)
				validRound = validRound + 1;
			else 
				validRound = round[i - doubleBackNum - 1];
		}
		round[i] = validRound;
	}
	if (canFreeLocs && numReturnedLocs > 0)
			free(locs);
	if (canFreeMoves && numReturnedMoves > 0)
			free(moves);
	return validRound;
}


bool wentBefore(HunterView hv, PlaceId place) {
	bool hasOtherHunter = 0;
	for(int j = 0; j < 4; j++) {
		int numLocs = 0;
		bool canFree = true;
		PlaceId * lastLocs = HvGetLastLocations(hv, j, 3, &numLocs, &canFree);
		if (numLocs == 3) {
			//printf("%s %s %s\n", placeIdToName(lastLocs[0]), placeIdToName(lastLocs[1]), pl)
			if (lastLocs[0] == place || lastLocs[1] == place || lastLocs[2] == place)
				hasOtherHunter = true;
		}
		if (numLocs > 0 && canFree)
			free(lastLocs);
	}
	return hasOtherHunter;
}