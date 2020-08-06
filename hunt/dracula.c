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

#define AVAILIABLEPLACE 14
#define SAVELANDPLACELENGTH 20
#define LANDPLACELENGTH 15
#define RAILPLACELENGTH 30
#define SEAPLACELENGTH 10
#define HUNTERCOUNT 4
#define xMsgs { "", "", "", "" }

PlaceId AvailablePlace[] = {
	GENOA,
	MARSEILLES,
	TOULOUSE,
	BORDEAUX,
	NANTES,
	LE_HAVRE,
	BRUSSELS,
	COLOGNE,
	HAMBURG,
	BERLIN,
	PRAGUE,
	VIENNA,
	MUNICH,
	VENICE
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

PlaceId LandPlace[] = {
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
};

PlaceId RailPlace[] = {
	ALICANTE,
	BARCELONA,
	BARI,
	BELGRADE,
	BERLIN,
	BORDEAUX,
	BRUSSELS,
	BUCHAREST,
	BUDAPEST,
	COLOGNE,
	EDINBURGH,
	FLORENCE,
	FRANKFURT,
	GENEVA,
	GENOA,
	LEIPZIG,
	LE_HAVRE,
	LISBON,
	LIVERPOOL,
	LONDON,
	MADRID,
	MARSEILLES,
	MILAN,
	MUNICH,
	NAPLES,
	PRAGUE,
	SALONICA,
	SOFIA,
	STRASBOURG,
	VENICE,
};

PlaceId SeaPlace[] = {
	ADRIATIC_SEA,
	ATLANTIC_OCEAN,
	BAY_OF_BISCAY,
	BLACK_SEA,
	ENGLISH_CHANNEL,
	IONIAN_SEA,
	IRISH_SEA,
	MEDITERRANEAN_SEA,
	NORTH_SEA,
	TYRRHENIAN_SEA,
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

static PlaceId predictLocation(DraculaView dv);
PlaceId getNearestLocation(DraculaView dv);
State getDraculaState(DraculaView dv);
PlaceId goAway(DraculaView dv, PlaceId* nextmove, int size);
State distancefromhunter(DraculaView dv, PlaceId place);
PlaceId bestPlace(PlaceId* hunterPlace);
PlaceId* DvaddPlace(PlaceId* place, int* num, PlaceId newPlace);
PlaceId ConvertToAction(DraculaView dv, PlaceId place);

void decideDraculaMove(DraculaView dv)
{
	//printf("Do decideDraculaMove\n");
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
	registerBestPlay(newplace, "come and catch me :)");
}

PlaceId predictLocation(DraculaView dv) {
	//printf("Do predictLocation\n");
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
	//printf("Do getNearestLocation\n");
	int numReturnedLocs = 0;
	PlaceId * places = DvWhereCanIGo(dv, &numReturnedLocs);
	for (int i = 0; i < numReturnedLocs; i++)
		for (int j = 0; j < AVAILIABLEPLACE; j++)
			if (places[i] == AvailablePlace[j])
				return places[i];

	int minLength = -1;
	//PlaceId curr = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	PlaceId p = NOWHERE;
	for (int i = 0; i < AVAILIABLEPLACE; i++) {
		DvGetShortestPathTo(dv, PLAYER_DRACULA, AvailablePlace[i], &numReturnedLocs);
		if (minLength == -1 || minLength > numReturnedLocs) {
			p = AvailablePlace[i];
			minLength = numReturnedLocs;
		}
	}

	int numPP = 0;
	int numcurr = 0;
	PlaceId* pp = DvGetShortestPathTo(dv, PLAYER_DRACULA, p, &numPP);
	PlaceId* curr = DvGetValidMoves(dv, &numcurr);
	places = DvWhereCanIGo(dv, &numReturnedLocs);

	for (int i = 0; i < numReturnedLocs && numPP > 0; i++) {
		if (places[i] == pp[0])
			for (int j = 0; j < numcurr; j++)
				if (ConvertToAction(dv, places[i]) == curr[j])
					return pp[0];
	}

	srand(time(0));

	if (numcurr == 0)
		return TELEPORT;

	return curr[rand() % numcurr];
}

State getDraculaState(DraculaView dv)
{
	//printf("Do getDraculaState\n");
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
	//printf("Do goAway\n");
	PlaceId* place = NULL;
	PlaceId* allplace = NULL;
	int numReturnLocs = 0;
	int allplacecount = 0;

	for (int i = 0; i < HUNTERCOUNT; i++) {
		allplace = DvaddPlace(allplace, &allplacecount, DvGetPlayerLocation(dv, i));
	}

	for (int i = 0; i < HUNTERCOUNT; i++) {
		place = DvWhereCanTheyGo(dv, i, &numReturnLocs);
		for (int j = 0; j < numReturnLocs; j++) {
			allplace = DvaddPlace(allplace, &allplacecount, place[j]);
		}
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

			DvGetShortestPathTo(dv, i, nextmove[j], &numReturnLocs);
			if (numReturnLocs > maxLength)
			{ hunterPlace[i] = nextmove[j]; maxLength = numReturnLocs; }

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

	return pp[rand() % locs];
}

State distancefromhunter(DraculaView dv, PlaceId place) {
	//printf("Do distancefromhunter\n");
	int numReturnLocs = 0;
	for (int i = 0; i < HUNTERCOUNT; i++) {
		DvGetShortestPathTo(dv, i, place, &numReturnLocs);
		if (numReturnLocs <= 2)
			return CHACING;
	}

	return LOST;
}

PlaceId bestPlace(PlaceId* hunterPlace) {
	//printf("Do bestPlace\n");
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
	//printf("Do ConvertToAction\n");
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
				if (GvGetLastMoves(getGameView(dv), PLAYER_DRACULA, 1, &numReturned, &canFree)[0] == HIDE) {
					// if the last move is HIDE, add DOUBLE_BACK_1 to moves
					return DOUBLE_BACK_1;
				}
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
