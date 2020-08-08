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
#include <time.h>
#include <string.h>
#include <stdio.h>
//#define DEBUG

#define PALAND 15
#define MULAND 12
#define MNLAND 10
#define TSLAND 7
#define MALAND 10
#define CDLAND 8
#define BSLAND 10
#define HUNTERCOUNT 4
#define XxMsgs { "come and catch me :-)", "I am here! Where are you? :-)", "lucky kids :-O", "See you :-)" }

//C
PlaceId PAland[] = {
	COLOGNE,
	AMSTERDAM,
	BRUSSELS,
	LE_HAVRE,
	NANTES,
	BORDEAUX,
	TOULOUSE,
	CLERMONT_FERRAND,
	CLERMONT_FERRAND,
	TOULOUSE,
	MARSEILLES,
	GENEVA,
	STRASBOURG,
	FRANKFURT,
	COLOGNE
};
//D
PlaceId MUland[] = {
	HAMBURG,
	HAMBURG,
	LEIPZIG,
	BERLIN,
	PRAGUE,
	VIENNA,
	MUNICH,
	MILAN,
	ZURICH,
	STRASBOURG,
	FRANKFURT,
	COLOGNE
};
//A
PlaceId MNland[] = {
	NORTH_SEA,
	EDINBURGH,
	MANCHESTER,
	LIVERPOOL,
	SWANSEA,
	LONDON,
	PLYMOUTH,
	LONDON,
	LONDON,
	ENGLISH_CHANNEL
};
//E
PlaceId TSland[] = {
	TYRRHENIAN_SEA,
	NAPLES,
	BARI,
	ROME,
	FLORENCE,
	GENOA,
	GENOA
};
//B
PlaceId MAland[] = {
	TOULOUSE,
	BARCELONA,
	BARCELONA,
	SARAGOSSA,
	SANTANDER,
	LISBON,
	CADIZ,
	GRANADA,
	ALICANTE,
	MEDITERRANEAN_SEA
};
//F
PlaceId CDland[] = {
	CONSTANTA,
	GALATZ,
	CASTLE_DRACULA,
	KLAUSENBURG,
	BUDAPEST,
	ZAGREB,
	SARAJEVO,
	SARAJEVO
};
//G
PlaceId BSland[] = {
	IONIAN_SEA,
	ATHENS,
	VALONA,
	SALONICA,
	SOFIA,
	VARNA,
	VARNA,
	CONSTANTA,
	VARNA,
	BLACK_SEA
};

PlaceId StartLocation[] = {
	TOULOUSE,
	ZURICH,
	EDINBURGH,
	GENOA,
	GRANADA,
	CASTLE_DRACULA,
	VALONA
};

typedef struct hunterPlace HunterPlace;

struct hunterPlace {
	PlaceId place;
	int total;
};

typedef enum state {
	LOST,                    // Hunter Lost the target
	CHACING,                 // Hunter Chacing dracula
	OUTFLANK,                // All Hunter Chacing dracula
	HIDDEN					 // If dracula in sea and the same location -> same with chacing
//	TPACTION,				 // Do the TELEPORT action when Hunter is not in CDLand
//	SEAANDSEA,               // When outflank, go to sea more than 2 times
}State;

typedef enum region {
	PALOOP,                  // After Hunter Lost the target
	MULOOP,					 // Dracula will follow the region
	MNLOOP,                  // loop the moves
	TSLOOP,
	MALOOP,
	CDLOOP,
	BSLOOP,
	UNKNOWN_LOOP = -1
}Region;

PlaceId predictLocation(DraculaView dv);

int theNearestHunter(DraculaView dv);
int getNumPathToRegion(DraculaView dv, Region r);
int getAllShorestPathToRegion(DraculaView dv, Region r, PlaceId* place, int num);
bool IsDraculaInRegion(DraculaView dv, Region r, PlaceId place, int* loopRound);
bool IsHunterInRegion(DraculaView dv, Region r, PlaceId* place);
bool IsHunterTogether(DraculaView dv);


PlaceId getNearestLocation(DraculaView dv);
PlaceId getShortestPathToRegion(DraculaView dv, Region r);
PlaceId getNextShortestPathToRegion(DraculaView dv, Region r, PlaceId* place, int num);
PlaceId getNextLoopLocation(DraculaView dv, Region r, PlaceId* placehunter, int* loopRound);
PlaceId goAway(DraculaView dv, PlaceId* nextmove, State state);
PlaceId draculaBorn(DraculaView dv, PlaceId* nextmove, PlaceId* placehunter);
PlaceId bestPlace(PlaceId* hunterPlace);
PlaceId* canGoAway(DraculaView dv, PlaceId* allplace, int allplacecount, State state, int* canGoNum);
PlaceId cancelChacing(DraculaView dv, PlaceId* canGo, State state, int canGoNum);
PlaceId cancelOutflank(DraculaView dv, PlaceId* canGo, State state, int canGoNum);
PlaceId noChoices(DraculaView dv);
PlaceId* DvaddPlace(PlaceId* place, int* num, PlaceId newPlace);
PlaceId ConvertToAction(DraculaView dv, PlaceId place);
PlaceId getLoopLocation(DraculaView dv, Region r,
	PlaceId* placehunter, PlaceId* places,
	int* loopRound, int maxRound);

Region getRegion(DraculaView dv);
Region getRegionWithoutSea(DraculaView dv);
Region decideRegion(DraculaView dv, bool region[7]);
Region checkRegion(DraculaView dv, PlaceId* placehunter, PlaceId* Place, Region r, int land);
Region checkDraculaRegion(PlaceId place, PlaceId* Place, Region r, int land, int* loopRound);

State getDraculaState(DraculaView dv);
State distancefromhunter(DraculaView dv, PlaceId place, int locsNums);

void decideDraculaMove(DraculaView dv)
{
#ifdef DEBUG
	printf("Do decideDraculaMove\n");
#endif
	// TODO: Replace this with something better!
	PlaceId currPlace = predictLocation(dv);

	bool checkplace = false;
	if (DvGetRound(dv) != 0) {
		int numReturnedLocs = 0;
		PlaceId* places = DvGetValidMoves(dv, &numReturnedLocs);

		for (int i = 0; i < numReturnedLocs; i++) {
			printf("Can go to = %s\n", placeIdToAbbrev(places[i]));
			if (ConvertToAction(dv, currPlace) == places[i]) {
				checkplace = true;
			}
		}
		//printf("Wrong Move %s \n", placeIdToAbbrev(places[i]));

		free(places);
	}
	else checkplace = true;

	if (currPlace == NOWHERE || currPlace == UNKNOWN_PLACE || checkplace == false)
		currPlace = noChoices(dv);

	char newplace[3];
	strcpy(newplace, placeIdToAbbrev(ConvertToAction(dv, currPlace)));
	Message msg[] = XxMsgs;
	registerBestPlay(newplace, msg[rand() % 4]);
}

PlaceId predictLocation(DraculaView dv) {
	PlaceId currPlace = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	int temp = 0;
	if (currPlace == NOWHERE)
		return goAway(dv, StartLocation, LOST);

	// How to move -> depend on state
	State s = getDraculaState(dv);
#ifdef DEBUG
	printf("Do predictLocation\n");
	printf("----------------------\n");
	printf("--- The State = %d ---\n", s);
	printf("----------------------\n");
#endif
	/*switch () {
		case AWAY: break;
	}*/
	switch (s) {
		case LOST:
			return getNearestLocation(dv);
		default:
			return goAway(dv, DvGetValidMoves(dv, &temp), s);
	}

	return currPlace;
}

PlaceId getNearestLocation(DraculaView dv) {
	int loopRound = 0;
	Region r = getRegion(dv);
#ifdef DEBUG
	printf("Do getNearestLocation\n");
	printf("-----------------------\n");
	printf("--- The Region = %d ---\n", r);
	printf("-----------------------\n");
#endif
	PlaceId* placehunter = NULL;
	int num = 0;
	for (int i = 0; i < HUNTERCOUNT; i++)
		placehunter = DvaddPlace(placehunter, &num, DvGetPlayerLocation(dv, i));
	
	if (DvGetHealth(dv, PLAYER_DRACULA) <= 14) {
		r = getRegionWithoutSea(dv);
	}
	else if (DvGetHealth(dv, PLAYER_DRACULA) <= 6) r = MULOOP;

	if (IsDraculaInRegion(dv, r, DvGetPlayerLocation(dv, PLAYER_DRACULA), &loopRound))
		return getNextLoopLocation(dv, r, placehunter, &loopRound);

	return getShortestPathToRegion(dv, r);
}

PlaceId getShortestPathToRegion(DraculaView dv, Region r) {
#ifdef DEBUG
	printf("Do getShortestPathToRegion\n");
#endif
	PlaceId p = UNKNOWN_PLACE;

	switch ((int)r) {
		case PALOOP: return getNextShortestPathToRegion(dv, r, PAland, PALAND);
		case MULOOP: return getNextShortestPathToRegion(dv, r, MUland, MULAND);
		case MNLOOP: return getNextShortestPathToRegion(dv, r, MNland, MNLAND);
		case TSLOOP: return getNextShortestPathToRegion(dv, r, TSland, TSLAND);
		case MALOOP: return getNextShortestPathToRegion(dv, r, MAland, MALAND);
		case CDLOOP: return getNextShortestPathToRegion(dv, r, CDland, CDLAND);
		case BSLOOP: return getNextShortestPathToRegion(dv, r, BSland, BSLAND);
	}

	return p;
}

PlaceId getNextShortestPathToRegion(DraculaView dv, Region r, PlaceId* place, int num) {
#ifdef DEBUG
	printf("Do getNextShorestPathToRegion\n");
#endif
	int minLength = -1;
	int numReturnedLocs = 0;
	PlaceId p = UNKNOWN_PLACE;
	for (int i = 0; i < num; i++) {
		PlaceId* list = DvGetShortestPathTo(dv, PLAYER_DRACULA, place[i], &numReturnedLocs);
		if ((minLength == -1 && numReturnedLocs > 0) || (minLength > numReturnedLocs && numReturnedLocs > 0)) {
			p = list[0];
			minLength = numReturnedLocs;
		}
		if (numReturnedLocs > 0)
			free(list);
	}
	return p;
}

int getNumPathToRegion(DraculaView dv, Region r) {
#ifdef DEBUG
	printf("Do getNumPathToRegion\n");
#endif
	int locs = 0;
	switch ((int)r) {
		case PALOOP: return getAllShorestPathToRegion(dv, r, PAland, PALAND);
		case MULOOP: return getAllShorestPathToRegion(dv, r, MUland, MULAND);
		case MNLOOP: return getAllShorestPathToRegion(dv, r, MNland, MNLAND);
		case TSLOOP: return getAllShorestPathToRegion(dv, r, TSland, TSLAND);
		case MALOOP: return getAllShorestPathToRegion(dv, r, MAland, MALAND);
		case CDLOOP: return getAllShorestPathToRegion(dv, r, CDland, CDLAND);
		case BSLOOP: return getAllShorestPathToRegion(dv, r, BSland, BSLAND);
	}

	return locs;
}

int getAllShorestPathToRegion(DraculaView dv, Region r, PlaceId* place, int num) {
#ifdef DEBUG
	printf("Do getAllShorestPathToRegion\n");
#endif
	int minLength = -1;
	int numReturnedLocs = 0;
	for (int i = 0; i < num; i++) {
		PlaceId* list = DvGetShortestPathTo(dv, PLAYER_DRACULA, place[i], &numReturnedLocs);
		if ((minLength == -1 && numReturnedLocs > 0) || (minLength > numReturnedLocs && numReturnedLocs > 0)) {
			minLength = numReturnedLocs;
		}
		if (numReturnedLocs > 0)
			free(list);
	}
	return minLength;
}

bool IsDraculaInRegion(DraculaView dv, Region r, PlaceId place, int* loopRound) {
#ifdef DEBUG
	printf("Do IsDraculaInRegion\n");
#endif
	switch ((int)r) {
		case PALOOP: return checkDraculaRegion(place, PAland, r, PALAND, loopRound) != UNKNOWN_LOOP;
		case MULOOP: return checkDraculaRegion(place, MUland, r, MULAND, loopRound) != UNKNOWN_LOOP;
		case MNLOOP: return checkDraculaRegion(place, MNland, r, MNLAND, loopRound) != UNKNOWN_LOOP;
		case TSLOOP: return checkDraculaRegion(place, TSland, r, TSLAND, loopRound) != UNKNOWN_LOOP;
		case MALOOP: return checkDraculaRegion(place, MAland, r, MALAND, loopRound) != UNKNOWN_LOOP;
		case CDLOOP: return checkDraculaRegion(place, CDland, r, CDLAND, loopRound) != UNKNOWN_LOOP;
		case BSLOOP: return checkDraculaRegion(place, BSland, r, BSLAND, loopRound) != UNKNOWN_LOOP;
	}
	return false;
}

bool IsHunterInRegion(DraculaView dv, Region r, PlaceId* place) {
#ifdef DEBUG
	printf("Do IsHunterInRegion\n");
#endif
	switch ((int)r) {
		case PALOOP: return checkRegion(dv, place, PAland, r, PALAND) == UNKNOWN_LOOP;
		case MULOOP: return checkRegion(dv, place, MUland, r, MULAND) == UNKNOWN_LOOP;
		case MNLOOP: return checkRegion(dv, place, MNland, r, MNLAND) == UNKNOWN_LOOP;
		case TSLOOP: return checkRegion(dv, place, TSland, r, TSLAND) == UNKNOWN_LOOP;
		case MALOOP: return checkRegion(dv, place, MAland, r, MALAND) == UNKNOWN_LOOP;
		case CDLOOP: return checkRegion(dv, place, CDland, r, CDLAND) == UNKNOWN_LOOP;
		case BSLOOP: return checkRegion(dv, place, BSland, r, BSLAND) == UNKNOWN_LOOP;
	}
	return false;
}

PlaceId getNextLoopLocation(DraculaView dv, Region r, PlaceId* placehunter, int* loopRound) {
#ifdef DEBUG
	printf("Do getNextLoopLocation\n");
#endif
	switch ((int)r) {
		case PALOOP: return getLoopLocation(dv, r, placehunter, PAland, loopRound, PALAND);
		case MULOOP: return getLoopLocation(dv, r, placehunter, MUland, loopRound, MULAND);
		case MNLOOP: return getLoopLocation(dv, r, placehunter, MNland, loopRound, MNLAND);
		case TSLOOP: return getLoopLocation(dv, r, placehunter, TSland, loopRound, TSLAND);
		case MALOOP: return getLoopLocation(dv, r, placehunter, MAland, loopRound, MALAND);
		case CDLOOP: return getLoopLocation(dv, r, placehunter, CDland, loopRound, CDLAND);
		case BSLOOP: return getLoopLocation(dv, r, placehunter, BSland, loopRound, BSLAND);
	}
	return UNKNOWN_PLACE;
}

PlaceId getLoopLocation(DraculaView dv, Region r, 
						PlaceId* placehunter, PlaceId* places,
						int* loopRound, int maxRound) {
#ifdef DEBUG
	printf("Do getLoopLocation\n");
#endif
	if (checkRegion(dv, placehunter, places, r, maxRound) == UNKNOWN_LOOP && getDraculaState(dv) != LOST)
		return getNearestLocation(dv);
	if (*loopRound < maxRound)
		return places[*loopRound++];

	*loopRound = 0;
	return places[*loopRound++];
}

Region getRegion(DraculaView dv) {
#ifdef DEBUG
	printf("Do getRegion\n");
#endif
	PlaceId* placehunter = NULL;
	int num = 0;
	for (int i = 0; i < HUNTERCOUNT; i++)
		placehunter = DvaddPlace(placehunter, &num, DvGetPlayerLocation(dv, i));

	bool regioncheck[7] = { false,false,false,false,false,false,false };

	for (int i = 0; i < 7; i++) {
		if (!IsHunterInRegion(dv, i, placehunter)) {
			regioncheck[i] = true;
		}
	}

	free(placehunter);
	return decideRegion(dv, regioncheck);
}

Region getRegionWithoutSea(DraculaView dv) {
#ifdef DEBUG
	printf("Do getRegionWithoutSea\n");
#endif
	PlaceId* placehunter = NULL;
	int num = 0;
	for (int i = 0; i < HUNTERCOUNT; i++)
		placehunter = DvaddPlace(placehunter, &num, DvGetPlayerLocation(dv, i));

	bool regioncheck[7] = { false,false,false,false,false,false,false };

	for (int i = 0; i < 7; i++) {
		if (!IsHunterInRegion(dv, i, placehunter) && getDraculaState(dv) == LOST)
			regioncheck[i] = true;
	}

	free(placehunter);
	return decideRegion(dv, regioncheck);
}

Region decideRegion(DraculaView dv, bool region[7]) {
	int shortestdistance[7] = { -1,-1,-1,-1,-1,-1,-1 };
	for (int i = 0; i < 7; i++)	
		if (region[i]) 
			shortestdistance[i] = getNumPathToRegion(dv, i);

	int countCanntGo = 0;
	for (int i = 0; i < 7; i++) {
		if (shortestdistance[i] == -1)
			countCanntGo++;
	}

	if (countCanntGo == 7)
		shortestdistance[MULOOP] = getNumPathToRegion(dv, MULOOP);

#ifdef DEBUG
	printf("Do decideRegion\n");
	for (int i = 0; i < 7; i++) {
		printf("%d  ", shortestdistance[i]);
	}
	printf("\n");
#endif

	PlaceId* placehunter = NULL;
	int num = 0;
	for (int i = 0; i < HUNTERCOUNT; i++)
		placehunter = DvaddPlace(placehunter, &num, DvGetPlayerLocation(dv, i));

	int temp = 0;
	State s = getDraculaState(dv);
	if (s == LOST) {
		for (int i = 0; i < 7; i++)
			if (IsDraculaInRegion(dv, i, DvGetPlayerLocation(dv, PLAYER_DRACULA), &temp) &&
				!IsHunterInRegion(dv, i, placehunter))
				return i;
	}

	if (s == CHACING || s == HIDDEN) {
		// Decide the shortest path to the region
		int minLength = 100;
		for (int i = 0; i < 7; i++)
			if (minLength > shortestdistance[i] && shortestdistance[i] != -1)
				minLength = shortestdistance[i];

		// Decide the place that dracula can go 
		for (int i = 0; i < 7; i++)
			if (minLength != shortestdistance[i] && shortestdistance[i] != -1)
				shortestdistance[i] = -1;

		// Decide the place to go
		// First is LAND
		// Second is SEA

		// SEA
		for (int i = 0; i < 7; i++) {
			if (shortestdistance[i] != -1) {
				PlaceId p = getShortestPathToRegion(dv, i);
				if (placeIdToType(p) == SEA)
					return i;
			}
		}

		// LAND
		for (int i = 0; i < 7; i++) {
			if (shortestdistance[i] != -1)
				return i;
		}
	}

	if (s == OUTFLANK) {
		// Decide the shortest path to the region
		int maxLength = -1;
		for (int i = 0; i < 7; i++)
			if (maxLength < shortestdistance[i] && shortestdistance[i] != -1)
				maxLength = shortestdistance[i];

		// Decide the place that dracula can go 
		for (int i = 0; i < 7; i++)
			if (maxLength != shortestdistance[i] && shortestdistance[i] != -1)
				shortestdistance[i] = -1;

		// Decide the place to go
		// First is SEA
		// Second is LAND

		// SEA
		for (int i = 0; i < 7; i++) {
			PlaceId p = getShortestPathToRegion(dv, i);
			if (shortestdistance[i] != -1 && placeIdToType(p) == SEA)
				return i;
		}

		// LAND
		for (int i = 0; i < 7; i++) {
			if (shortestdistance[i] != -1)
				return i;
		}
	}

	return UNKNOWN_LOOP;
}

Region checkRegion(DraculaView dv, PlaceId* placehunter, PlaceId* Place, Region r, int land) {
#ifdef DEBUG
	printf("Do checkRegion\n");
#endif
	for (int i = 0; i < HUNTERCOUNT; i++) {
		for (int j = 0; j < land; j++) {
			if (placehunter[i] == Place[j]) {
				return UNKNOWN_LOOP;
			}
		}
	}

	return r;
}

Region checkDraculaRegion(PlaceId place, PlaceId* Place, Region r, int land, int* loopRound) {
#ifdef DEBUG
	printf("Do checkDraculaRegion\n");
#endif
	for (int i = 0; i < land; i++) {
		if (place == Place[i]) {
			*loopRound = i + 1;
			return r;
		}
	}
	return UNKNOWN_LOOP;
}

State getDraculaState(DraculaView dv) {
#ifdef DEBUG
	printf("Do getDraculaState\n");
#endif
	// need to know hunter thinking
	int numReturnedLocs = 0;
	PlaceId *p = DvGetTrapLocations(dv, &numReturnedLocs);
	if (numReturnedLocs > 0)
		free(p);

	bool canFree = false;
	int trapsNums = 0;
	p = GvGetLastLocations(getGameView(dv), PLAYER_DRACULA, 6, &trapsNums, &canFree);

	int temp = trapsNums;
	for (int i = 0; i < temp; i++) {
		if (placeIdToType(p[i]) == SEA) {
			trapsNums--;
		}
	}

	if (placeIsReal(DvGetVampireLocation(dv)))
		trapsNums--;

	if (canFree)
		free(p);

	State s = distancefromhunter(dv, DvGetPlayerLocation(dv, PLAYER_DRACULA), 1);

	// Detect Hunter Finding Dracula
	if (numReturnedLocs < trapsNums) {
		// Detect Hunter Chacing Dracula
		if (s == CHACING)
			s = distancefromhunter(dv, DvGetPlayerLocation(dv, PLAYER_DRACULA), 3);
		// Detect Hunter Outflack Dracula
		if (s == OUTFLANK)
			return OUTFLANK;
	}
	
	return s;
}

PlaceId goAway(DraculaView dv, PlaceId* nextmove, State state) {
#ifdef DEBUG
	printf("Do goAway\n");
#endif

	PlaceId* allplace = NULL;
	int allplacecount = 0;

	for (int i = 0; i < HUNTERCOUNT; i++)
		allplace = DvaddPlace(allplace, &allplacecount, DvGetPlayerLocation(dv, i));

	// Go away for safety
	if (DvGetPlayerLocation(dv, PLAYER_DRACULA) == NOWHERE)
		return draculaBorn(dv, nextmove, allplace);

	int canGoNum = 0;
	PlaceId* canGo = canGoAway(dv, allplace, allplacecount, state, &canGoNum);

	PlaceId p = NOWHERE;
	// Go Away From Hunter
	if (state == CHACING || state == HIDDEN)
		p = cancelChacing(dv, canGo, state, canGoNum);

	if (state == OUTFLANK)
		p = cancelOutflank(dv, canGo, state, canGoNum);

	if (canGoNum > 0)
		free(canGo);

	if (p != NOWHERE)
		return p;

	// If no choises, Random the location
	return noChoices(dv);
}

PlaceId draculaBorn(DraculaView dv, PlaceId* nextmove, PlaceId* placehunter) {
#ifdef DEBUG
	printf("Do draculaBorn\n");
#endif
	// Decide The Starting Land
	bool regioncheck[7] = { false,false,false,false,false,false,false };
	for (int i = 0; i < 7; i++) {
		if (!IsHunterInRegion(dv, i, placehunter)) {
			regioncheck[i] = true;
		}
	}

	for (int i = 0; i < 7; i++)
		if (regioncheck[i])
			return nextmove[i];

	srand(time(0));
	return MUland[rand() % MULAND];
}

State distancefromhunter(DraculaView dv, PlaceId place, int locsNums) {
#ifdef DEBUG
	printf("Do distancefromhunter\n");
#endif
	int numReturnLocs = 0;
	int numOfHunter = 0;

	// If in land, how far away from hunter
	for (int i = 0; i < HUNTERCOUNT; i++) {
		PlaceId* list = DvGetShortestPathTo(dv, i, place, &numReturnLocs);
		if (numReturnLocs <= locsNums) {
			numOfHunter++;
		}
		if (numReturnLocs > 0)
			free(list);
	}

	// If in sea, hunter get lost
	if (numOfHunter >= 1 && placeIdToType(place) == SEA)
		return HIDDEN;

	if (numOfHunter == 1)
		return CHACING;

	if (numOfHunter >= 2)
		return OUTFLANK;

	return LOST;
}

PlaceId bestPlace(PlaceId* hunterPlace) {
#ifdef DEBUG
	printf("Do bestPlace\n");
#endif
	HunterPlace hunterBucket[HUNTERCOUNT];
	PlaceId best = NOWHERE;

	for (int i = 0; i < HUNTERCOUNT; i++) {
		//printf("%dangerState\n", placeIdToName(hunterPlace[i]));
		hunterBucket[i].place = hunterPlace[i];
		hunterBucket[i].total = 1;
	}

	for (int i = 0; i < HUNTERCOUNT; i++)
		for (int j = 0; j < HUNTERCOUNT; j++) {
			if (hunterBucket[i].place == hunterBucket[j].place && i != j) {
				hunterBucket[j].place = NOWHERE;
				hunterBucket[i].total++;
			}
		}

	int max = 0;
	for (int i = 0; i < HUNTERCOUNT; i++)
		if (max < hunterBucket[i].total && hunterBucket[i].total != NOWHERE) {
			max = hunterBucket[i].total;
			best = hunterBucket[i].place;
		}

	return best;
}

PlaceId* canGoAway(DraculaView dv, PlaceId* allplace, int allplacecount, State state, int* canGoNum) {
#ifdef DEBUG
	printf("Do canGoAway\n");
#endif
	int currCount = 0;
	PlaceId* currPlace = NULL;

	// If Outflank dracula , dracula can decide moving to sea
	// If health is lower or equal 2, then dont go to sea
	if (DvGetHealth(dv, PLAYER_DRACULA) <= 2)
		currPlace = DvWhereCanIGoByType(dv, true, false, &currCount);
	else
		currPlace = DvWhereCanIGo(dv, &currCount);

	PlaceId* canGo = NULL;
	bool sameplace = false;

	PlaceId draculaplace = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	for (int j = 0; j < currCount; j++) {
		sameplace = false;
		for (int k = 0; k < allplacecount; k++)
			if (currPlace[j] == allplace[k] || currPlace[j] == draculaplace)
				sameplace = true;
		if (sameplace) continue;

		if (!placeIsReal(currPlace[j]))
			continue;

		canGo = DvaddPlace(canGo, canGoNum, currPlace[j]);
	}

	if (allplace != NULL)
		free(allplace);

#ifdef DEBUG
	for (int i = 0; i < *canGoNum; i++) {
		printf("Availible place =  %s\n", placeIdToAbbrev(canGo[i]));
	}
#endif
	return canGo;
}

PlaceId cancelChacing(DraculaView dv, PlaceId* canGo, State state, int canGoNum) {

	// Selete the longest place to the hunter and selete the shortest path to the region
	Region r = UNKNOWN_LOOP;
	if (DvGetHealth(dv, PLAYER_DRACULA) >= 6)
		r = getRegion(dv);
	else r = getRegionWithoutSea(dv);
#ifdef DEBUG
	printf("Do cancelChacing\n");
	printf("-----------------------\n");
	printf("--- New Region = %d ---\n", r);
	printf("-----------------------\n");
#endif

	PlaceId p = getShortestPathToRegion(dv, r);
	for (int i = 0; i < canGoNum; i++) {
		if (p == canGo[i])
			return p;
	}

	// Selete the longest place to the hunter and can be go to sea
	PlaceId pp = NOWHERE;
	int canGoSeaNum = 0;
	PlaceId draculaplace = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	PlaceId* currplace = DvWhereCanIGo(dv, &canGoSeaNum);
	for (int i = 0; i < canGoSeaNum; i++) {
		State s = distancefromhunter(dv, currplace[i], 1);
		if (s == HIDDEN && currplace[i] != draculaplace) {
			pp = currplace[i];
		}
	}

	if (canGoSeaNum > 0 && pp != NOWHERE) {
		free(currplace);
		return pp;
	}

	int player = theNearestHunter(dv);
	int numReturnLocs = 0;

	// Selete the longest place
	int maxLength = 0;
	for (int i = 0; i < canGoSeaNum; i++) {
		PlaceId* list = DvGetShortestPathTo(dv, player, currplace[i], &numReturnLocs);

		if (maxLength < numReturnLocs && numReturnLocs > 0) {
			maxLength = numReturnLocs;
			pp = currplace[i];
		}

		if (numReturnLocs > 0)
			free(list);
	}

	if (canGoSeaNum > 0)
		free(currplace);

	if (pp != NOWHERE)
		return pp;

	return noChoices(dv);
}

PlaceId cancelOutflank(DraculaView dv, PlaceId* canGo, State state, int canGoNum) {

	// Selete the longest place to the hunter and can be go to sea
	for (int i = 0; i < canGoNum; i++) {
		State s = distancefromhunter(dv, canGo[i], 1);
		if (s == LOST) {
			return canGo[i];
		}
	}

	// Selete the longest place to the hunter and selete the shortest path to the region
	Region r = UNKNOWN_LOOP;
	if (DvGetHealth(dv, PLAYER_DRACULA) >= 4)
		r = getRegion(dv);
	else r = getRegionWithoutSea(dv);
#ifdef DEBUG
	printf("Do cancelOutflank\n");
	printf("-----------------------\n");
	printf("--- New Region = %d ---\n", r);
	printf("-----------------------\n");
#endif
	// if get region is unknow loop -> try to go to MNland and try to teleport
	// TODO
	// if get the best region
	PlaceId p = getShortestPathToRegion(dv, r);
	for (int i = 0; i < canGoNum; i++) {
		if (p == canGo[i])
			return p;
	}

	//////////////////////// Unuseful decide the next path ////////////////////////

	PlaceId hunterPlace[HUNTERCOUNT];
	for (int i = 0; i < HUNTERCOUNT; i++)
		hunterPlace[i] = UNKNOWN_PLACE;

	// Get all hunter to dracula distance
	p = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	int numReturnLocs = 0;
	int distance[HUNTERCOUNT] = { -1,-1,-1,-1 };
	for (int i = 0; i < HUNTERCOUNT; i++) {
		PlaceId* list = DvGetShortestPathTo(dv, i, p, &numReturnLocs);
		if (numReturnLocs >= 0) {
			distance[i] = numReturnLocs;
		}
		if (numReturnLocs > 0)
			free(list);
	}

	// Get the hunter who is chacing dracula
	bool chacingHunter[HUNTERCOUNT] = { false,false,false,false };
	for (int i = 0; i < HUNTERCOUNT - 1; i++) {
		if (distance[i] < 2) {
			chacingHunter[i] = true;
		}
	}

	numReturnLocs = 0;
	int maxLength = 0;
	for (int i = 0; i < HUNTERCOUNT; i++) {
		if(chacingHunter[i])
			for (int j = 0; j < canGoNum; j++) {
				PlaceId* list = DvGetShortestPathTo(dv, i, canGo[j], &numReturnLocs);

				if (numReturnLocs > maxLength && numReturnLocs > 0) {
					hunterPlace[i] = canGo[j];
					maxLength = numReturnLocs;
				}
				if (numReturnLocs > 0)
					free(list);
			}
		maxLength = 0;
	}

	p = bestPlace(hunterPlace);
	if (placeIsReal(p))
		return p;

	return noChoices(dv);
}

PlaceId noChoices(DraculaView dv) {
#ifdef DEBUG
	printf("Do noChoices\n");
#endif

	// If no choises, Random the location
	int locs = 0;
	srand(time(0));
	PlaceId* pp = DvGetValidMoves(dv, &locs);

	// If no more ValidMoves -> Teleport
	if (locs <= 0)
		return TELEPORT;

	int numReturnedLocs = 0;
	PlaceId* places = DvWhereCanIGoByType(dv, true, false, &numReturnedLocs);
	PlaceId* landPlace = NULL;
	int numLandLocs = 0;
	for (int i = 0; i < numReturnedLocs; i++) {
		if (placeIdToType(places[i]) != SEA) {
			landPlace = DvaddPlace(landPlace, &numLandLocs, places[i]);
			printf("place = %s\n", placeIdToAbbrev(places[i]));
		}
	}

	PlaceId* FinalPlaces = NULL;
	for (int i = 0; i < locs; i++) {
		for (int j = 0; j < numLandLocs; j++)
			if (ConvertToAction(dv, pp[i]) == landPlace[j]) {
				FinalPlaces = DvaddPlace(landPlace, &numReturnedLocs, landPlace[j]);
				printf("place = %s\n", placeIdToAbbrev(landPlace[j]));
			}
	}

	if (numReturnedLocs > 0 && FinalPlaces != NULL) {
		PlaceId place = FinalPlaces[rand() % numReturnedLocs];
		free(FinalPlaces);
		return place;
	}


	// Get Random
	PlaceId p = pp[rand() % locs];
	free(pp);
	return p;
}

// add a place to a given PlaceId array if that place haven't been added before
// input: PlaceId array, total number in array, new place
// output: the new PlaceId array
PlaceId* DvaddPlace(PlaceId* place, int* num, PlaceId newPlace) {
	for (int i = 0; i < *num; i++) {
		if (place[i] == newPlace) return place;
	}
	place = realloc(place, (*num + 1) * sizeof(PlaceId));
	place[*num] = newPlace;
	*num = *num + 1;
	return place;
}

PlaceId ConvertToAction(DraculaView dv, PlaceId place) {
#ifdef DEBUG
	printf("Do ConvertToAction\n");
#endif
	// dracula can directly move to that location
	if (DvCanGo(dv, place))
		return place;

	if (getDoubleBackNum(dv, place) > 0) {
		// dracula can double back to that location
		int DBnumber = getDoubleBackNum(dv, place);
		// if there is no DOUBLE_BACK_# in the trail and the last move was HIDE
		// Dracula can go either DOUBLE_BACK_1 or DOUBLE_BACK_2
		if (DBnumber == 2) {
			bool canFree;
			int numReturned = 0;
			// get the last move in the history
			if (numReturned > 0) {
				PlaceId* list = GvGetLastMoves(getGameView(dv), PLAYER_DRACULA, 1, &numReturned, &canFree);
				if (list[0] == HIDE) {
					// if the last move is HIDE, add DOUBLE_BACK_1 to moves
					place = DOUBLE_BACK_1;
				}
				if (canFree)
					free(list);
			}


		}
		// add DOUBLE_BACK_# to the history
		return DOUBLE_BACK_1 + DBnumber - 1;
	}

	// dracula can hide in the current location
	if (canHide(dv))
		return HIDE;

	return NOWHERE;
}

int theNearestHunter(DraculaView dv) {
#ifdef DEBUG
	printf("Do theNearestHunter\n");
#endif
	// Get all hunter to dracula distance
	PlaceId p = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	int numReturnLocs = 0;
	int distance[HUNTERCOUNT] = { -1,-1,-1,-1 };
	int minLength = 100;
	for (int i = 0; i < HUNTERCOUNT; i++) {
		PlaceId* list = DvGetShortestPathTo(dv, i, p, &numReturnLocs);
		if (numReturnLocs >= 0) {
			distance[i] = numReturnLocs;
			minLength = numReturnLocs;
		}
		if (numReturnLocs > 0)
			free(list);
	}

	// Get the hunter who is chacing dracula
	int player = 0;
	minLength = distance[0];
	for (int i = 1; i < HUNTERCOUNT; i++) {
		if (distance[i] < minLength) {
			player = i;
			minLength = distance[i];
		}
	}

	return player;
}

bool IsHunterTogether(DraculaView dv) {
#ifdef DEBUG
	printf("Do IsHunterTogether\n");
#endif
	int player = theNearestHunter(dv);
	PlaceId hunterplace = DvGetPlayerLocation(dv, player);
	int numReturnLocs = 0;
	int numOfHunter = 0;
	for (int i = 0; i < HUNTERCOUNT; i++) {
		if (i != player) {
			PlaceId* list = DvGetShortestPathTo(dv, i, hunterplace, &numReturnLocs);
			if (numReturnLocs > 0 && numReturnLocs <= 2) {
				numOfHunter++;
			}
			if (numReturnLocs > 0)
				free(list);
		}
	}
	if (numOfHunter >= 2)
		return true;
	return false;
}
