        
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
#define SAVELANDPLACELENGTH 17
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
	ROME
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
DraculaView fakeHunterView(DraculaView dv, const char losc[3], int* player);
PlaceId goAway(DraculaView dv, PlaceId* nextmove, int size);
State distancefromhunter(DraculaView dv, PlaceId place);
PlaceId bestPlace(PlaceId* hunterPlace);
PlaceId* DvaddPlace(PlaceId* place, int* num, PlaceId newPlace);

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

	char newplace[3];
	strcpy(newplace, placeIdToAbbrev(currPlace));
	registerBestPlay(newplace, "come and catch me :)");
}

PlaceId predictLocation(DraculaView dv) {

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

	int numReturnedLocs = 0;
	PlaceId * places = DvGetValidMoves(dv, &numReturnedLocs);
	for (int i = 0; i < numReturnedLocs; i++)
		for (int j = 0; j < AVAILIABLEPLACE; j++)
			if (places[i] == AvailablePlace[j])
				return places[i];

	srand(time(0));
	int minLength = -1;
	int player = 0;
	PlaceId curr = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	PlaceId p = NOWHERE;
	for (int i = 0; i < AVAILIABLEPLACE; i++) {
		DvGetShortestPathTo(fakeHunterView(dv, placeIdToAbbrev(curr), &player), player, AvailablePlace[i], &numReturnedLocs);
		if (minLength == -1 || minLength > numReturnedLocs) {
			p = AvailablePlace[i];
			minLength = numReturnedLocs;
		}
	}

	int total = 0;
	places = DvGetValidMoves(dv, &total);
	PlaceId goodResult = NOWHERE;
	PlaceId badResult = NOWHERE;
	int badminLength = -1;
	minLength = -1;
	for (int i = 0; i < total; i++) {
		if (!placeIsReal(places[i]))
			continue;
		DvGetShortestPathTo(fakeHunterView(dv, placeIdToAbbrev(places[i]), &player), player, p, &numReturnedLocs);
		if ((minLength == -1 || minLength > numReturnedLocs) && placeIdToType(places[i]) == LAND) {
			goodResult = places[i];
			minLength = numReturnedLocs;
		}
		if ((badminLength == -1 || badminLength > numReturnedLocs)) {
			badResult = places[i];
			badminLength = numReturnedLocs;
		}
	}

	if (goodResult == NOWHERE)
		return badResult;
	return goodResult;
}

State getDraculaState(DraculaView dv)
{
	// need to know hunter thinking
	static int chacingCount = 0;
	static int unChacingCount = 0;

	int numReturnedLocs = 0;
	DvGetTrapLocations(dv, &numReturnedLocs);

	// Detect Hunter Finding Dracula
	if (numReturnedLocs <= 5) {

		// Detect Hunter Outflack Dracula
		int chacing = 0;
		for (int i = 0; i < HUNTERCOUNT; i++) {
			if (distancefromhunter(dv, DvGetPlayerLocation(dv, PLAYER_DRACULA)) == CHACING)
				chacing++;
		}

		if (chacing >= 2)
			return OUTFLANK;

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
	}
	
	return LOST;
}

DraculaView fakeHunterView(DraculaView dv, const char locs[3], int* player)
{
	Message msgs[] = xMsgs;
	int round = DvGetRound(dv);
	if (round % 4 == 0 || (round + 1) % 4 == 1 || (round + 2) % 4 == 2 || (round + 3) % 3 == 0) {
		char pastplay[32] = "GCD.... SCD.... HCD.... M......";
		pastplay[25] = locs[0];
		pastplay[26] = locs[1];
		*player = 3;
		return DvNew(pastplay, msgs);
	}
	if (round % 4 == 1 || (round + 1) % 4 == 2 || (round + 2) % 4 == 3 || (round + 3) % 4 == 0) {
		char pastplay[24] = "GCD.... SCD.... H......";
		pastplay[17] = locs[0];
		pastplay[18] = locs[1];
		*player = 2;
		return DvNew(pastplay, msgs);
	}
	if (round % 4 == 2 || (round + 1) % 4 == 3 || (round + 2) % 4 == 0 || (round + 3) % 4 == 1) {
		char pastplay[16] = "GCD.... S......";
		pastplay[9] = locs[0];
		pastplay[10] = locs[1];
		*player = 1;
		return DvNew(pastplay, msgs);
	}
	if (round % 4 == 3 || (round + 1) % 4 == 0 || (round + 2) % 4 == 1 || (round + 3) % 4 == 2) {
		char pastplay[8] = "G......";
		pastplay[1] = locs[0];
		pastplay[2] = locs[1];
		*player = 0;
		return DvNew(pastplay, msgs);
	}

	return NULL;
}

PlaceId goAway(DraculaView dv, PlaceId* nextmove, int size) {
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
	
	return bestPlace(hunterPlace);
}

State distancefromhunter(DraculaView dv, PlaceId place) {

	int numReturnLocs = 0;
	for (int i = 0; i < HUNTERCOUNT; i++) {
		DvGetShortestPathTo(dv, i, place, &numReturnLocs);
		if (numReturnLocs <= 2)
			return CHACING;
	}

	return LOST;
}

PlaceId bestPlace(PlaceId* hunterPlace) {
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