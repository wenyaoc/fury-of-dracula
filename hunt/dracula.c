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
//#define DEBUG
#include "dracula.h"
#include "DraculaView.h"
#include "Game.h"
#include "GameView.h"
#include <time.h>
#include <string.h>
#include <stdio.h>

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
	DOUBLE_BACK_2,
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
	DOUBLE_BACK_1,
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
	DOUBLE_BACK_2,
	BLACK_SEA
};

PlaceId SaveLandPlace[] = {
	GALWAY,
	DUBLIN,
	AMSTERDAM,
	NANTES,
	CLERMONT_FERRAND,
	TOULOUSE,
	GRANADA,
	CADIZ,
	CAGLIARI,
	ZAGREB,
	SARAJEVO,
	VALONA,
	KLAUSENBURG,
	CASTLE_DRACULA,
	ATHENS,
	BARI,
	NAPLES,
	GENOA,
	ROME,
	MANCHESTER
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
	CDLOOP,
	MALOOP,
	BSLOOP,
	UNKNOWN_LOOP = -1
}Region;

static PlaceId predictLocation(DraculaView dv);
PlaceId getNearestLocation(DraculaView dv);
PlaceId getShortestPathToRegion(DraculaView dv, Region r);
PlaceId getNextShorestPathToRegion(DraculaView dv, Region r, PlaceId* place, int num);
bool IsInRegion(DraculaView dv, Region r, PlaceId place);
PlaceId getNextLoopLocation(DraculaView dv, Region r, PlaceId* placehunter, int* continueRound, int* loopRound);
PlaceId getLoopLocation(DraculaView dv, Region r, 
						PlaceId* placehunter, PlaceId* places,
						int* continueRound, int* loopRound, int maxRound);
Region getRegion(DraculaView dv);
Region decideRegion(DraculaView dv, bool region[7]);
Region checkRegion(PlaceId* placehunter, PlaceId* Place, Region r, int land);
Region checkDraculaRegion(PlaceId place, PlaceId* Place, Region r, int land);
State getDraculaState(DraculaView dv);
PlaceId goAway(DraculaView dv, PlaceId* nextmove, int size);
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
#ifdef DEBUG
	printf("Do predictLocation\n");
#endif
	PlaceId currPlace = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	int temp = 0;
	if (currPlace == NOWHERE)
		return goAway(dv, SaveLandPlace, SAVELANDPLACELENGTH);

	// How to move -> depend on state
	State s = getDraculaState(dv);
	PlaceId* p = NULL;
	switch (s) {
		case LOST:
			return getNearestLocation(dv);
		case CHACING:
			p = DvGetValidMoves(dv, &temp);
			return goAway(dv, p, temp);
		case OUTFLANK:
			p = DvGetValidMoves(dv, &temp);
			return goAway(dv, p, temp);
	}

	return currPlace;
}

PlaceId getNearestLocation(DraculaView dv) {
#ifdef DEBUG
	printf("Do getNearestLocation\n");
#endif
	static int loopRound = 0;
	static int continueRound = 0;
	static Region r = UNKNOWN_LOOP;
	if (continueRound == 0) {
		r = getRegion(dv);
		continueRound = 1;
	}
#ifdef DEBUG
	printf("-----------------------\n");
	printf("--- The Region = %d ---\n", r);
	printf("-----------------------\n");
#endif
	PlaceId* placehunter = NULL;
	int num = 0;
	for (int i = 0; i < HUNTERCOUNT; i++)
		placehunter = DvaddPlace(placehunter, &num, DvGetPlayerLocation(dv, i));

	if (IsInRegion(dv, r, DvGetPlayerLocation(dv, PLAYER_DRACULA))) {
		PlaceId p = getNextLoopLocation(dv, r, placehunter, &continueRound, &loopRound);
		return p;
	}

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
		//printf("Place = %s\n", placeIdToName(place[i]));
		if (!placeIsReal(place[i]))
			continue;
		if (DvGetPlayerLocation(dv, PLAYER_DRACULA) == place[i])
			return place[i];
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

bool IsInRegion(DraculaView dv, Region r, PlaceId place) {
#ifdef DEBUG
	printf("Do IsInRegion\n");
#endif
	switch ((int)r) {
		case PALOOP: return checkDraculaRegion(place, PAland, r, PALAND) != UNKNOWN_LOOP;
		case MULOOP: return checkDraculaRegion(place, MUland, r, MULAND) != UNKNOWN_LOOP;
		case MNLOOP: return checkDraculaRegion(place, MNland, r, MNLAND) != UNKNOWN_LOOP;
		case TSLOOP: return checkDraculaRegion(place, TSland, r, TSLAND) != UNKNOWN_LOOP;
		case MALOOP: return checkDraculaRegion(place, MAland, r, MALAND) != UNKNOWN_LOOP;
		case CDLOOP: return checkDraculaRegion(place, CDland, r, CDLAND) != UNKNOWN_LOOP;
		case BSLOOP: return checkDraculaRegion(place, BSland, r, BSLAND) != UNKNOWN_LOOP;
	}
	return false;
}

PlaceId getNextLoopLocation(DraculaView dv, Region r, PlaceId* placehunter, int* continueRound, int* loopRound) {
#ifdef DEBUG
	printf("Do getNextLoopLocation\n");
#endif
	switch ((int)r) {
		case PALOOP: return getLoopLocation(dv, r, placehunter, PAland, continueRound, loopRound, PALAND);
		case MULOOP: return getLoopLocation(dv, r, placehunter, MUland, continueRound, loopRound, MULAND);
		case MNLOOP: return getLoopLocation(dv, r, placehunter, MNland, continueRound, loopRound, MNLAND);
		case TSLOOP: return getLoopLocation(dv, r, placehunter, TSland, continueRound, loopRound, TSLAND);
		case MALOOP: return getLoopLocation(dv, r, placehunter, MAland, continueRound, loopRound, MALAND);
		case CDLOOP: return getLoopLocation(dv, r, placehunter, CDland, continueRound, loopRound, CDLAND);
		case BSLOOP: return getLoopLocation(dv, r, placehunter, BSland, continueRound, loopRound, BSLAND);
	}
	return UNKNOWN_PLACE;
}

PlaceId getLoopLocation(DraculaView dv, Region r, 
						PlaceId* placehunter, PlaceId* places,
						int* continueRound, int* loopRound, int maxRound) {
#ifdef DEBUG
	printf("Do getLoopLocation\n");
#endif
	if (checkRegion(placehunter, places, r, maxRound) == UNKNOWN_LOOP && getDraculaState(dv) != LOST) {
		*continueRound = 0;
		return getNearestLocation(dv);
	}

	if (*loopRound <= maxRound)
		return places[*loopRound++];

	*loopRound = 0;
	return places[*loopRound++];
}

Region getRegion(DraculaView dv) {
#ifdef DEBUG
	printf("Do getRegion\n");
#endif
	PlaceId* placehunter = NULL;
	Region r = UNKNOWN_LOOP;
	int num = 0;
	for (int i = 0; i < HUNTERCOUNT; i++)
		placehunter = DvaddPlace(placehunter, &num, DvGetPlayerLocation(dv, i));

	bool regioncheck[7] = { false,false,false,false,false,false,false };

	r = checkRegion(placehunter, PAland, PALOOP, PALAND);
	if (r == PALOOP) {
		regioncheck[0] = true;
	}
	r = checkRegion(placehunter, MUland, MULOOP, MULAND);
	if (r == MULOOP) {
		regioncheck[1] = true;
	}
	r = checkRegion(placehunter, MNland, MNLOOP, MNLAND);
	if (r == MNLOOP) {
		regioncheck[2] = true;
	}
	r = checkRegion(placehunter, TSland, TSLOOP, TSLAND);
	if (r == TSLOOP) {
		regioncheck[3] = true;
	}
	r = checkRegion(placehunter, MAland, MALOOP, MALAND);
	if (r == MALOOP) {
		regioncheck[4] = true;
	}
	r = checkRegion(placehunter, CDland, CDLOOP, CDLAND);
	if (r == CDLOOP) {
		regioncheck[5] = true;
	}
	r = checkRegion(placehunter, BSland, BSLOOP, BSLAND);
	if (r == BSLOOP) {
		regioncheck[6] = true;
	}
	free(placehunter);
	return decideRegion(dv, regioncheck);
}

Region decideRegion(DraculaView dv, bool region[7]) {
#ifdef DEBUG
	printf("Do decideRegion\n");
#endif
	int shortestdistance[7] = { -1,-1,-1,-1,-1,-1,-1 };
	if (region[0]) {

		PlaceId p = getNextShorestPathToRegion(dv, PALOOP, PAland, PALAND);
		PlaceId* pp = DvGetShortestPathTo(dv, PLAYER_DRACULA, p, &shortestdistance[0]);
		if (shortestdistance[0] > 0)
			free(pp);
	}
	if (region[1]) {
	
		PlaceId p = getNextShorestPathToRegion(dv, PALOOP, MUland, MULAND);
		PlaceId* pp = DvGetShortestPathTo(dv, PLAYER_DRACULA, p, &shortestdistance[1]);
		if (shortestdistance[1] > 0)
			free(pp);
	}
	if (region[2]) {
	
		PlaceId p = getNextShorestPathToRegion(dv, PALOOP, MNland, MNLAND);
		PlaceId* pp = DvGetShortestPathTo(dv, PLAYER_DRACULA, p, &shortestdistance[2]);
		if (shortestdistance[2] > 0)
			free(pp);
	}
	if (region[3]) {
		
		PlaceId p = getNextShorestPathToRegion(dv, PALOOP, TSland, TSLAND);
		PlaceId* pp = DvGetShortestPathTo(dv, PLAYER_DRACULA, p, &shortestdistance[3]);
		if (shortestdistance[3] > 0)
			free(pp);
	}
	if (region[4]) {
		
		PlaceId p = getNextShorestPathToRegion(dv, PALOOP, MAland, MALAND);
		PlaceId* pp = DvGetShortestPathTo(dv, PLAYER_DRACULA, p, &shortestdistance[4]);
		if (shortestdistance[4] > 0)
			free(pp);
	}
	if (region[5]) {
		
		PlaceId p = getNextShorestPathToRegion(dv, PALOOP, CDland, CDLAND);
		PlaceId* pp = DvGetShortestPathTo(dv, PLAYER_DRACULA, p, &shortestdistance[5]);
		if (shortestdistance[5] > 0)
			free(pp);
	}
	if (region[6]) {
		
		PlaceId p = getNextShorestPathToRegion(dv, PALOOP, BSland, BSLAND);
		PlaceId* pp = DvGetShortestPathTo(dv, PLAYER_DRACULA, p, &shortestdistance[6]);
		if (shortestdistance[6] > 0)
			free(pp);
	}

	int min = 100;
	Region r = UNKNOWN_LOOP;
	for (int i = 0; i < 7; i++)
		if (min > shortestdistance[i] && shortestdistance[i] != -1) {
			min = shortestdistance[i];
			r = i;
		}
	return r;
}

Region checkRegion(PlaceId* placehunter, PlaceId* Place, Region r, int land) {
#ifdef DEBUG
	printf("Do checkRegion\n");
#endif
	for (int i = 0; i < land; i++) {
		for (int j = 0; j < HUNTERCOUNT; j++)
			if (placehunter[j] == Place[i]) {
				return UNKNOWN_LOOP;
			}
	}
	return r;
}

Region checkDraculaRegion(PlaceId place, PlaceId* Place, Region r, int land) {
#ifdef DEBUG
	printf("Do checkDraculaRegion\n");
#endif
	for (int i = 0; i < land; i++) {
		if (place == Place[i]) {
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
	static int chacingCount = 0;
	static int unChacingCount = 0;

	int numReturnedLocs = 0;
	DvGetTrapLocations(dv, &numReturnedLocs);

	// Detect Hunter Finding Dracula
	if (numReturnedLocs <= 5) {
		// Detect Hunter Chacing Dracula
		State s = distancefromhunter(dv, DvGetPlayerLocation(dv, PLAYER_DRACULA));
		if (s == CHACING) {
			chacingCount++;
			return CHACING;
		}
		if (unChacingCount >= 2) {
			chacingCount = unChacingCount = 0;
			return LOST;
		}
		if (s == LOST && chacingCount >= 2) {
			unChacingCount++;
			return CHACING;
		}
		// Detect Hunter Outflack Dracula
		if (chacingCount >= 4)
			return OUTFLANK;
	}
	
	return LOST;
}

PlaceId goAway(DraculaView dv, PlaceId* nextmove, int size) {
#ifdef DEBUG
	printf("Do goAway\n");
#endif
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

	numReturnLocs = 0;
	int maxLength = 0;
	bool sameplace = false;
	PlaceId hunterPlace[HUNTERCOUNT];
	for (int i = 0; i < HUNTERCOUNT; i++)
		hunterPlace[i] = UNKNOWN_PLACE;
	for (int i = 0; i < HUNTERCOUNT; i++) {
		for (int j = 0; j < size; j++) {
			sameplace = false;
			for (int k = 0; k < allplacecount; k++)
				if (nextmove[j] == allplace[k])
					sameplace = true;
			if (sameplace) continue;

			if (!placeIsReal(nextmove[j]))
				continue;

			PlaceId * list = DvGetShortestPathTo(dv, i, nextmove[j], &numReturnLocs);
			if (numReturnLocs > maxLength && numReturnLocs > 0) {
				hunterPlace[i] = nextmove[j]; 
				maxLength = numReturnLocs;
			}
			if (numReturnLocs > 0)
				free(list);

		}
		maxLength = 0;
	}

	if (allplace != NULL)
		free(allplace);

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
		if (numReturnLocs <= 2)
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
