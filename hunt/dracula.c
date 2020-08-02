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

#include "Places.h"


void decideDraculaMove(DraculaView dv)
{
	// TODO: Replace this with something better!  
	PlaceId currPlace = DvGetPlayerLocation (dv,PlayerPlaceId currPlace = HvGetPlayerLocation(hv, PLAYER_LORD_GODALMING);
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
	return placeIdToAbbrev(newPlace);
	
	registerBestPlay("CD", "Mwahahahaha");
}

