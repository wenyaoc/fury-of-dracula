        
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

#define LANDPLACELENGTH 16
#define RAILPLACELENGTH 30
#define SEAPLACELENGTH 10
#define HUNTERCOUNT 4

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
	ST_JOSEPH_AND_ST_MARY,
	SARAJEVO,
	VALONA,
	KLAUSENBURG,
	CASTLE_DRACULA,
	ATHENS
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
/*
typedef struct hunterPlace HunterPlace;

struct hunterPlace {
	PlaceId place;
	int total;
};

static PlaceId predictLocation(DraculaView dv);
PlaceId bestPlace(PlaceId* hunterPlace);
*/
void decideDraculaMove(DraculaView dv)
{
	// TODO: Replace this with something better!
	PlaceId currPlace = DvGetPlayerLocation(dv, PLAYER_DRACULA);

	if (currPlace == NOWHERE) {
		srand(time(0));
		currPlace = LandPlace[rand() % LANDPLACELENGTH];
	}
	else if (currPlace != NOWHERE) {
		int numReturnedLocs;
		PlaceId* places = DvGetValidMoves(dv, &numReturnedLocs);
		srand(time(0));
		currPlace = places[rand() % numReturnedLocs];
		free(places);
	}
	
	char* newPlace = malloc(2 * sizeof(char));
	strcpy(newPlace, placeIdToAbbrev(currPlace));
	registerBestPlay(newPlace, "hello :)");
}
/*
PlaceId predictLocation(DraculaView dv) {

	PlaceId currPlace = DvGetPlayerLocation(dv, PLAYER_DRACULA);

	int numReturnLocs = 0;
	int maxLength = 0;
	if (currPlace == NOWHERE) {
		
		PlaceId hunterPlace[HUNTERCOUNT];
		for (int i = 0; i < HUNTERCOUNT; i++)
			for (int j = 0; j < LANDPLACELENGTH; j++) {
				DvGetShortestPathTo(dv, i, LandPlace[j], &numReturnLocs);
				if (numReturnLocs > maxLength) { hunterPlace[i] = LandPlace[j]; maxLength = numReturnLocs; }
			}

		return bestPlace(hunterPlace);
	}


	// How to move
	// need to know hunter thinking
	// 
	

	return currPlace;
}

PlaceId bestPlace(PlaceId* hunterPlace) {
	HunterPlace hunterBucket[HUNTERCOUNT];
	PlaceId best = NOWHERE;

	for (int i = 0; i < HUNTERCOUNT; i++) {
		hunterBucket[i].place = hunterPlace[i];
		hunterBucket[i].total = 1;
	}

	for (int i = 0; i < HUNTERCOUNT; i++)
		for (int j = 0; j < HUNTERCOUNT; j++)
			if (hunterBucket[i].place == hunterBucket[j].place) {
				hunterBucket[j].place = NOWHERE;
				hunterBucket[i].total++;
			}

	int max = 0;
	for (int i = 0; i < HUNTERCOUNT; i++)
		if (max < hunterBucket[i].total && hunterBucket[i].total != NOWHERE) {
			max = hunterBucket[i].total;
			best = hunterBucket[i].place;
		}

	return best;
}*/

    
