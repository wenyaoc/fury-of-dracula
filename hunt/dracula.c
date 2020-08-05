        
        
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

#include "Places.h"
#include "Map.h"
#include <time.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>

#define AVAILIABLEPLACE 14
#define SAVELANDPLACELENGTH 17
#define LANDPLACELENGTH 15
#define RAILPLACELENGTH 30
#define SEAPLACELENGTH 10
#define HUNTERCOUNT 4
#define xMsgs { "", "", "", "" }

PlaceId England[] = {
	GALWAY,
	DUBLIN,
	SWANSEA,
	LONDON,
	PLYMOUTH,
	MANCHESTER,
	LIVERPOOL,
	EDINBURGH,
	ADRIATIC_SEA,
	IRISH_SEA, 
	ENGLISH_CHANNEL, 
	NORTH_SEA
};

PlaceId Spanish[] = {
	BAY_OF_BISCAY,
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

PlaceId SpanishToItaly[] = {
	TOULOUSE,
	MARSEILLES
};

PlaceId EnglandToItaly[] = {
	LE_HAVRE,
	PARIS,
	STRASBOURG,
	MUNICH,
	VENICE
};


int numberOfHunters(DraculaView dv, PlaceId * places);
PlaceId doDecideDraculaMove(DraculaView dv);
PlaceId findStartingPoint(DraculaView dv);
PlaceId getNextLocation(DraculaView dv, PlaceId currPlace);
PlaceId dracMoveIsValid(DraculaView dv, PlaceId place);

void decideDraculaMove(DraculaView dv)
{
	PlaceId place = doDecideDraculaMove(dv);

	char * newPlace = malloc(2 * sizeof(char));
	strcpy(newPlace, placeIdToAbbrev(place));
	registerBestPlay("CD", "come and catch me :)");
}

PlaceId doDecideDraculaMove(DraculaView dv) {
	PlaceId currPlace = DvGetPlayerLocation(dv, PLAYER_DRACULA);
	if (currPlace == 0)
		return findStartingPoint(dv);
	return getNextLocation(dv, currPlace);
}


int numberOfHunters(DraculaView dv, PlaceId * places) {
	PlaceId hunter0 = DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING);
	PlaceId hunter1 = DvGetPlayerLocation(dv, PLAYER_DR_SEWARD);
	PlaceId hunter2 = DvGetPlayerLocation(dv, PLAYER_VAN_HELSING);
	PlaceId hunter3 = DvGetPlayerLocation(dv, PLAYER_MINA_HARKER);

	int num = sizeof(places)/4;
	int hunterNum = 0;
	for (int i = 0; i < num; i ++) {
		if (hunter0 == places[i] || hunter1 == places[i] || hunter2 == places[i] || hunter3 == places[i]) {
			hunterNum++;
		}
	}
	return hunterNum;
}

PlaceId findStartingPoint(DraculaView dv) {
	int england = numberOfHunters(dv, England);
	int spanish = numberOfHunters(dv, Spanish);
	int italy = numberOfHunters(dv, Italy);

	int fewer = spanish;
	if (spanish <= fewer) 
		fewer = england;
	if (italy <= fewer)
		fewer = italy;

    if (fewer == spanish)
        return ALICANTE;
    else if (fewer == england)
        return GALWAY;
    else
        return VENICE;
}

PlaceId getNextLocation(DraculaView dv, PlaceId currPlace) {
	// spanish
	if (currPlace == ALICANTE) {
		if (numberOfHunters(dv, Spanish) == 0)
			return BARCELONA;
		else 
			return MEDITERRANEAN_SEA;
	} 
	if (currPlace == BARCELONA) {
		if (numberOfHunters(dv, Spanish) != 0) {
			if (numberOfHunters(dv, SpanishToItaly) == 0 && numberOfHunters(dv, Italy) == 0)
				return TOULOUSE;
			else
				return MEDITERRANEAN_SEA;
		} else {
			return SARAGOSSA;
		}
	}
	if (currPlace == SARAGOSSA) 
		return SANTANDER;
	if (currPlace == SANTANDER)
		return MADRID;
	if (currPlace == MADRID)
		return LISBON;
	if (currPlace == LISBON) {
		if (numberOfHunters(dv, Spanish) != 0)
			return ATLANTIC_OCEAN;
		else
			return CADIZ;
	}		
	if (currPlace == CADIZ) {
		if (numberOfHunters(dv, Spanish) != 0)
			return ATLANTIC_OCEAN;
		else
			return GRANADA;
	}
	if (currPlace == GRANADA)
		return ALICANTE;
	// spanish to italy
	if (currPlace == TOULOUSE)
		return MARSEILLES;
	if (currPlace == MARSEILLES)
		return GENOA;
	if (currPlace == GENOA) {
		//if (numberOfHunters(dv, Italy) != 0)
		return VENICE;
	}
	if (currPlace == VENICE)
		return FLORENCE;
	if (currPlace == FLORENCE)
		return ROME;
	if (currPlace == ROME) {
		if (numberOfHunters(dv, Italy) != 0) 
			return TYRRHENIAN_SEA;
		else 
			return BARI;
	}
	if (currPlace == BARI)
		return NAPLES;
	if (currPlace == NAPLES)
		return TYRRHENIAN_SEA;
	
	if (currPlace == TYRRHENIAN_SEA)
		return MEDITERRANEAN_SEA;

	if (currPlace == MEDITERRANEAN_SEA) {
		if (numberOfHunters(dv, SpanishToItaly) == 0 && numberOfHunters(dv, Italy) == 0) 
			return MARSEILLES;
		else if (numberOfHunters(dv, Spanish) == 0)
			return ALICANTE;
		else if (numberOfHunters(dv, England) == 0)
			return ATLANTIC_OCEAN;
		else 
			return MARSEILLES;
	} 
	if (currPlace == ATLANTIC_OCEAN) {	
		if (numberOfHunters(dv, England) == 0)
			return CADIZ;
		else if (numberOfHunters(dv, Spanish) == 0)
			return LISBON;
		else if (numberOfHunters(dv, SpanishToItaly) == 0 && numberOfHunters(dv, Italy) == 0)
			return MEDITERRANEAN_SEA;
		else 
			return LISBON;
	}

	//England
	if (currPlace == GALWAY)
		return DUBLIN;
	if (currPlace == DUBLIN)
		return SWANSEA;
	if (currPlace == SWANSEA)
		return LIVERPOOL;
	if (currPlace == LIVERPOOL) {
		return MANCHESTER;
	} 
	if (currPlace == MANCHESTER)
		return PLYMOUTH;
	if (currPlace == PLYMOUTH)
		return ENGLISH_CHANNEL;
	if (currPlace == ENGLISH_CHANNEL) {
		if (numberOfHunters(dv, EnglandToItaly) == 0 && numberOfHunters(dv, Italy) == 0)
			return LE_HAVRE;
		else 
			return ATLANTIC_OCEAN;
	}


	//England To Itlay
	if (currPlace == LE_HAVRE)
		return PARIS;
	if (currPlace == PARIS)
		return STRASBOURG;
	if (currPlace == STRASBOURG)
		return MUNICH;
	if (currPlace == MUNICH)
		return VENICE;

	return NOWHERE;
}


PlaceId dracMoveIsValid(DraculaView dv, PlaceId place) {
	bool isValid = false;
	int numLocs;
	PlaceId * places;
	if (place != NOWHERE) {
		places = DvWhereCanIGo(dv, &numLocs);
		for (int i = 0; i < numLocs; i++) {
			if (place == places[i])
			isValid = true;
		}
	} else
		isValid = false;
	
	if (!isValid) {
		srand(time(0));
		place = places[rand() % numLocs];
	}

	if (numLocs > 0)
		free(places);
	else 
		return TELEPORT;
	return place;
}
