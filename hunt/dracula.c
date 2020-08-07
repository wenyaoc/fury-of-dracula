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
#define SAVELANDPLACELENGTH 20
#define PALAND 15
#define MULAND 12
#define MNLAND 10
#define TSLAND 7
#define MALAND 10
#define CDLAND 8
#define BSLAND 10
#define HUNTERCOUNT 4
#define xMsgs { "come and catch me :-)", "I am here! Where are you? :-)", "lucky kids :-O", "See you :-)" }

//C
PlaceId PAland[] = {
	HIDE,
	AMSTERDAM,
	BRUSSELS,
	LE_HAVRE,
	NANTES,
	BORDEAUX,
	TOULOUSE,
	CLERMONT_FERRAND,
	HIDE,
	TOULOUSE,
	MARSEILLES,
	GENEVA,
	STRASBOURG,
	FRANKFURT,
	COLOGNE
};
//D
PlaceId MUland[] = {
	HIDE,
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
	HIDE,
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
	HIDE
};
//B
PlaceId MAland[] = {
	TOULOUSE,
	BARCELONA,
	HIDE,
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
	HIDE
};
//G
PlaceId BSland[] = {
	IONIAN_SEA,
	ATHENS,
	VALONA,
	SALONICA,
	SOFIA,
	VARNA,
	HIDE,
	CONSTANTA,
	VARNA,
	BLACK_SEA
};

PlaceId StartLocation[] = {
	CLERMONT_FERRAND,
	ZURICH,
	MANCHESTER,
	GENOA,
	GRANADA,
	CASTLE_DRACULA,
	SARAJEVO
};

typedef struct hunterPlace HunterPlace;

struct hunterPlace {
	PlaceId place;
	int total;
};

typedef enum state {
	LOST,                    // Hunter Lost the target
	CHACING,                 // Hunter Chacing dracula
	OUTFLANK                 // All Hunter Chacing dracula
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

static PlaceId predictLocation(DraculaView dv);
PlaceId getNearestLocation(DraculaView dv);
PlaceId getShortestPathToRegion(DraculaView dv, Region r);
PlaceId getNextShorestPathToRegion(DraculaView dv, Region r, PlaceId* place, int num);
int getNumPathToRegion(DraculaView dv, Region r);
int getAllShorestPathToRegion(DraculaView dv, Region r, PlaceId* place, int num);
bool IsDraculaInRegion(DraculaView dv, Region r, PlaceId place, int* loopRound);
PlaceId getNextLoopLocation(DraculaView dv, Region r, PlaceId* placehunter, int* loopRound);
PlaceId getLoopLocation(DraculaView dv, Region r, 
						PlaceId* placehunter, PlaceId* places,
						int* loopRound, int maxRound);
Region getRegion(DraculaView dv);
Region decideRegion(DraculaView dv, bool region[7]);
Region checkRegion(PlaceId* placehunter, PlaceId* Place, Region r, int land);
Region checkDraculaRegion(PlaceId place, PlaceId* Place, Region r, int land, int* loopRound);
State getDraculaState(DraculaView dv);
PlaceId goAway(DraculaView dv, PlaceId* nextmove, State state);
State distancefromhunter(DraculaView dv, PlaceId place);
PlaceId bestPlace(PlaceId* hunterPlace);
PlaceId* DvaddPlace(PlaceId* place, int* num, PlaceId newPlace);
PlaceId ConvertToAction(DraculaView dv, PlaceId place);

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
			//printf("Can go to = %s\n", placeIdToAbbrev(places[i]));
			if (ConvertToAction(dv, currPlace) == places[i])
				checkplace = true;
		}

		free(places);
	}
	else checkplace = true;

	if (currPlace == NOWHERE || currPlace == UNKNOWN_PLACE || checkplace == false) {
		int numReturnedLocs = 0;
		PlaceId* places = DvGetValidMoves(dv, &numReturnedLocs);
		if (numReturnedLocs == 0)
			currPlace = TELEPORT;
		else {
			srand(time(0));
			currPlace = places[rand() % numReturnedLocs];
		}
		free(places);
	}

	char newplace[3];
	strcpy(newplace, placeIdToAbbrev(ConvertToAction(dv, currPlace)));
	Message msg[] = xMsgs;
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
	switch (s) {
		case LOST:
			return getNearestLocation(dv);
		case CHACING:
			return goAway(dv, DvGetValidMoves(dv, &temp), CHACING);
		case OUTFLANK:
			return goAway(dv, DvGetValidMoves(dv, &temp), OUTFLANK);
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
		while (r == TSLOOP || r == BSLOOP) {
			r = rand() % 7;
		}
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
		case PALOOP: return getNextShorestPathToRegion(dv, r, PAland, PALAND);
		case MULOOP: return getNextShorestPathToRegion(dv, r, MUland, MULAND);
		case MNLOOP: return getNextShorestPathToRegion(dv, r, MNland, MNLAND);
		case TSLOOP: return getNextShorestPathToRegion(dv, r, TSland, TSLAND);
		case MALOOP: return getNextShorestPathToRegion(dv, r, MAland, MALAND);
		case CDLOOP: return getNextShorestPathToRegion(dv, r, CDland, CDLAND);
		case BSLOOP: return getNextShorestPathToRegion(dv, r, BSland, BSLAND);
	}

	return p;
}

PlaceId getNextShorestPathToRegion(DraculaView dv, Region r, PlaceId* place, int num) {
#ifdef DEBUG
	printf("Do getNextShorestPathToRegion\n");
#endif
	int minLength = -1;
	int numReturnedLocs = 0;
	PlaceId p = UNKNOWN_PLACE;
	for (int i = 0; i < num; i++) {
		if (!placeIsReal(place[i]))
			continue;
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
		if (!placeIsReal(place[i]))
			continue;
		PlaceId* list = DvGetShortestPathTo(dv, PLAYER_DRACULA, place[i], &numReturnedLocs);
		if ((minLength == -1 && numReturnedLocs > 0) || (minLength > numReturnedLocs && numReturnedLocs > 0))
			minLength = numReturnedLocs;
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
		case PALOOP: return checkRegion(place, PAland, r, PALAND) == UNKNOWN_LOOP;
		case MULOOP: return checkRegion(place, MUland, r, MULAND) == UNKNOWN_LOOP;
		case MNLOOP: return checkRegion(place, MNland, r, MNLAND) == UNKNOWN_LOOP;
		case TSLOOP: return checkRegion(place, TSland, r, TSLAND) == UNKNOWN_LOOP;
		case MALOOP: return checkRegion(place, MAland, r, MALAND) == UNKNOWN_LOOP;
		case CDLOOP: return checkRegion(place, CDland, r, CDLAND) == UNKNOWN_LOOP;
		case BSLOOP: return checkRegion(place, BSland, r, BSLAND) == UNKNOWN_LOOP;
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
	if (checkRegion(placehunter, places, r, maxRound) == UNKNOWN_LOOP && getDraculaState(dv) != LOST)
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
		if (IsHunterInRegion(dv, i, placehunter) || getDraculaState(dv) == LOST)
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

#ifdef DEBUG
	printf("Do decideRegion\n");
	for (int i = 0; i < 7; i++) {
		printf("%d  ", shortestdistance[i]);
	}
	printf("\n");
#endif


	int temp = 0;
	for (int i = 0; i < 7; i++)
		if (IsDraculaInRegion(dv, i, DvGetPlayerLocation(dv, PLAYER_DRACULA), &temp) &&
			getDraculaState(dv) == LOST)
			return i;

	for (int i = 0; i < 7; i++)
		if (shortestdistance[i] != -1) {
			return i;
		}

	return UNKNOWN_LOOP;
}

Region checkRegion(PlaceId* placehunter, PlaceId* Place, Region r, int land) {
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
	p = GvGetLastLocations(getGameView(dv), PLAYER_DRACULA, 5, &trapsNums, &canFree);

	int temp = trapsNums;
	for (int i = 0; i < temp; i++)
		if (placeIdToType(p[i]) == SEA)
			trapsNums--;

	if (canFree)
		free(p);

	// Detect Hunter Finding Dracula
	if (numReturnedLocs < trapsNums) {
		// Detect Hunter Chacing Dracula
		State s = distancefromhunter(dv, DvGetPlayerLocation(dv, PLAYER_DRACULA));
		if (s == CHACING)
			return CHACING;
		// Detect Hunter Outflack Dracula
		if (s == OUTFLANK)
			return OUTFLANK;
	}
	
	return LOST;
}

PlaceId goAway(DraculaView dv, PlaceId* nextmove, State state) {
#ifdef DEBUG
	printf("Do goAway\n");
#endif

	PlaceId* placehunter = NULL;
	int num = 0;
	for (int i = 0; i < HUNTERCOUNT; i++)
		placehunter = DvaddPlace(placehunter, &num, DvGetPlayerLocation(dv, i));

	// Decide The Starting Land
	if (DvGetPlayerLocation(dv, PLAYER_DRACULA) == NOWHERE) {

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

	// Go Away From Hunter
	PlaceId* allplace = NULL;
	int numReturnLocs = 0;
	int allplacecount = 0;

	for (int i = 0; i < HUNTERCOUNT; i++) {
		allplace = DvaddPlace(allplace, &allplacecount, DvGetPlayerLocation(dv, i));
	}

	for (int i = 0; i < HUNTERCOUNT; i++) {
		PlaceId* place = DvWhereCanTheyGo(dv, i, &numReturnLocs);
		for (int j = 0; j < numReturnLocs; j++) {
			allplace = DvaddPlace(allplace, &allplacecount, place[j]);
		}
		if (numReturnLocs > 0)
			free(place);
	}


	int currCount = 0;
	PlaceId* currPlace = NULL;

	// If Outflank dracula , dracula can decide moving to sea
	if (DvGetHealth(dv, PLAYER_DRACULA) <= 4 && state != OUTFLANK)
		currPlace = DvWhereCanIGoByType(dv, true, false, &currCount);
	else
		currPlace = DvWhereCanIGo(dv, &currCount);

	int canGoNum = 0;
	PlaceId * canGo = NULL;
	numReturnLocs = 0;
	int maxLength = 0;
	bool sameplace = false;
	PlaceId hunterPlace[HUNTERCOUNT];
	for (int i = 0; i < HUNTERCOUNT; i++)
		hunterPlace[i] = UNKNOWN_PLACE;

	for (int j = 0; j < currCount; j++) {
		sameplace = false;
		for (int k = 0; k < allplacecount; k++)
			if (currPlace[j] == allplace[k])
				sameplace = true;
		if (sameplace) continue;

		if (!placeIsReal(currPlace[j]))
			continue;

		canGo = DvaddPlace(canGo, &canGoNum, currPlace[j]);
	}

	if (allplace != NULL)
		free(allplace);

	for (int i = 0; i < HUNTERCOUNT; i++) {
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

	PlaceId p = bestPlace(hunterPlace);
	if (placeIsReal(p))
		return p;

	int locs = 0;
	srand(time(0));
	PlaceId* pp = DvGetValidMoves(dv, &locs);
	if (locs <= 0)
		return TELEPORT;

	p = pp[rand() % locs];
	free(pp);
	return p;
}

State distancefromhunter(DraculaView dv, PlaceId place) {
#ifdef DEBUG
	printf("Do distancefromhunter\n");
#endif
	int numReturnLocs = 0;
	int numOfHunter = 0;

	for (int i = 0; i < HUNTERCOUNT; i++) {
		PlaceId* list = DvGetShortestPathTo(dv, i, place, &numReturnLocs);
		if (numReturnLocs < 2)
			numOfHunter++;
		if (numReturnLocs > 0)
			free(list);
	}

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
		//printf("%s\n", placeIdToName(hunterPlace[i]));
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
