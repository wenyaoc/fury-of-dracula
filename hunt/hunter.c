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


#include "Places.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
/*
typedef mode {
	CHASING,	
	RANDOM,	
	RESEARCH	
} Mode;
*/
const char * decideLordGodalmingMove(HunterView hv, Message message);
const char * decideDrSewardMove(HunterView hv, Message message);
const char * decideVanHelsingMove(HunterView hv, Message message);
const char * decideMinaHarkerMove(HunterView hv, Message message);
PlaceId predictLocation(HunterView hv, Message message);
int * readGodalmingmessage(HunterView hv, Player player);

void decideHunterMove(HunterView hv) {

	Player player = HvGetPlayer(hv);
	const char * place;
	Message message = "\0";
    switch (player) {
        case PLAYER_LORD_GODALMING:
            place = decideLordGodalmingMove(hv, message);
        break;
        case PLAYER_DR_SEWARD:
            place = decideDrSewardMove(hv, message);
        break;
        case PLAYER_VAN_HELSING:
            place = decideVanHelsingMove(hv, message);
        break;
        case PLAYER_MINA_HARKER:
            place = decideMinaHarkerMove(hv, message);
        break;
		default:
		break;
    }
	char * newPlace = malloc(2 * sizeof(char));
	strcpy(newPlace, place);
	registerBestPlay(newPlace, message);
}


const char * decideLordGodalmingMove(HunterView hv, Message message) {
	PlaceId currPlace = HvGetPlayerLocation(hv, PLAYER_LORD_GODALMING);
	if (currPlace == NOWHERE) return "ED";

	PlaceId newPlace = currPlace;
	PlaceId predictPlace = predictLocation(hv);
	if (predictPlace == NOWHERE) {
		int numReturnedLocs;
		PlaceId * places = HvWhereCanIGo(hv, &numReturnedLocs);
		srand(time(0));
		newPlace = places[rand() % numReturnedLocs];
		free(places);
	}
	strcpy(message, "I Like Dracula");
	return placeIdToAbbrev(newPlace);
}

const char * decideDrSewardMove(HunterView hv, Message message) {
	PlaceId currPlace = HvGetPlayerLocation(hv, PLAYER_DR_SEWARD);
	if (currPlace == NOWHERE) return "BD";

	PlaceId newPlace = currPlace;
	if (readGodalmingmessage(hv) == NOWHERE) {
		int numReturnedLocs;
		PlaceId * places = HvWhereCanIGo(hv, &numReturnedLocs);
		srand(time(0));
		newPlace = places[rand() % numReturnedLocs];
		free(places);
	}
	strcpy(message, "I Like Dracula");
	return placeIdToAbbrev(newPlace);
}

const char * decideVanHelsingMove(HunterView hv, Message message) {
	PlaceId currPlace = HvGetPlayerLocation(hv, PLAYER_VAN_HELSING);
	if (currPlace == NOWHERE) return "PA";

	PlaceId newPlace = currPlace;
	if (readGodalmingmessage(hv) == NOWHERE) {
		int numReturnedLocs;
		PlaceId * places = HvWhereCanIGo(hv, &numReturnedLocs);
		srand(time(0));
		newPlace = places[rand() % numReturnedLocs];
		free(places);
	}
	strcpy(message, "I Like Dracula");
	return placeIdToAbbrev(newPlace);
}

const char * decideMinaHarkerMove(HunterView hv, Message message){
	PlaceId currPlace = HvGetPlayerLocation(hv, PLAYER_MINA_HARKER);
	if (currPlace == NOWHERE) 
		return "CD";
	PlaceId newPlace = CASTLE_DRACULA;
	if (currPlace != CASTLE_DRACULA) {
		int pathLength;
		PlaceId * places = HvGetShortestPathTo(hv, PLAYER_MINA_HARKER, CASTLE_DRACULA, &pathLength);
		newPlace = places[0];
		free(places);
	}
	strcpy(message, "I Like Dracula");
	return placeIdToAbbrev(newPlace);

}

PlaceId predictLocation(HunterView hv, Message message) {
	strcpy(message, "2-2-2-")
	return NOWHERE;
}



int readGodalmingmessage(HunterView hv, Player player) {
	char place[2];
	int round = HvGetRound(hv);
    switch (player) {
        case PLAYER_DR_SEWARD:
            place[0] = hv->;
        break;
        case PLAYER_VAN_HELSING:
            place = decideVanHelsingMove(hv, message);
        break;
        case PLAYER_MINA_HARKER:
            place = decideMinaHarkerMove(hv, message);
        break;
		default:
		break;
    }	
}

