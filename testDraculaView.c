////////////////////////////////////////////////////////////////////////
// COMP2521 20T2 ... the Fury of Dracula
// testDraculaView.c: test the DraculaView ADT
//
// As supplied, these are very simple tests.  You should write more!
// Don't forget to be rigorous and thorough while writing tests.
//
// 2014-07-01	v1.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2017-12-02	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v2.0	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v3.0	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DraculaView.h"
#include "Game.h"
#include "Places.h"
#include "testUtils.h"

int main(void)
{
	{///////////////////////////////////////////////////////////////////

		printf("Test for basic functions, "
			"just before Dracula's first move\n");

		char* trail =
			"GST.... SAO.... HZU.... MBB....";

		Message messages[] = {
			"Hello", "Goodbye", "Stuff", "..."
		};
		DraculaView dv = DvNew(trail, messages);
		assert(DvGetRound(dv) == 0);
		assert(DvGetScore(dv) == GAME_START_SCORE);
		assert(DvGetHealth(dv, PLAYER_DRACULA) == GAME_START_BLOOD_POINTS);
		assert(DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) == STRASBOURG);
		assert(DvGetPlayerLocation(dv, PLAYER_DR_SEWARD) == ATLANTIC_OCEAN);
		assert(DvGetPlayerLocation(dv, PLAYER_VAN_HELSING) == ZURICH);
		assert(DvGetPlayerLocation(dv, PLAYER_MINA_HARKER) == BAY_OF_BISCAY);
		assert(DvGetPlayerLocation(dv, PLAYER_DRACULA) == NOWHERE);
		assert(DvGetVampireLocation(dv) == NOWHERE);
		int numTraps = -1;
		PlaceId * traps = DvGetTrapLocations(dv, &numTraps);
		assert(numTraps == 0);
		free(traps);

		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////

		printf("Test for encountering Dracula\n");

		char* trail =
			"GST.... SAO.... HCD.... MAO.... DGE.V.. "
			"GGEVD.. SAO.... HCD.... MAO....";

		Message messages[] = {
			"Hello", "Goodbye", "Stuff", "...", "Mwahahah",
			"Aha!", "", "", ""
		};

		DraculaView dv = DvNew(trail, messages);

		assert(DvGetRound(dv) == 1);
		assert(DvGetScore(dv) == GAME_START_SCORE - SCORE_LOSS_DRACULA_TURN);
		assert(DvGetHealth(dv, PLAYER_LORD_GODALMING) == 5);
		assert(DvGetHealth(dv, PLAYER_DRACULA) == 30);
		assert(DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) == GENEVA);
		assert(DvGetPlayerLocation(dv, PLAYER_DRACULA) == GENEVA);
		//assert(DvGetVampireLocation(dv) == NOWHERE);
		printf("Test passed!\n");
		DvFree(dv);
	}

	{///////////////////////////////////////////////////////////////////

		printf("Test for Dracula leaving minions 1\n");

		char* trail =
			"GGE.... SGE.... HGE.... MGE.... DED.V.. "
			"GST.... SST.... HST.... MST.... DMNT... "
			"GST.... SST.... HST.... MST.... DLOT... "
			"GST.... SST.... HST.... MST.... DHIT... "
			"GST.... SST.... HST.... MST....";

		Message messages[24];
		DraculaView dv = DvNew(trail, messages);

		assert(DvGetRound(dv) == 4);
		assert(DvGetVampireLocation(dv) == EDINBURGH);
		int numTraps = -1;
		PlaceId * traps = DvGetTrapLocations(dv, &numTraps);
		assert(numTraps == 3);
		sortPlaces(traps, numTraps);
		assert(traps[0] == LONDON);
		assert(traps[1] == LONDON);
		assert(traps[2] == MANCHESTER);
		free(traps);
		printf("Test passed!\n");
		DvFree(dv);
	}

	/*
	{///////////////////////////////////////////////////////////////////

		printf("Test for Dracula's valid moves 1\n");

		char* trail =
			"GGE.... SGE.... HGE.... MGE.... DCD.V.. "
			"GGE.... SGE.... HGE.... MGE....";

		Message messages[9];
		DraculaView dv = DvNew(trail, messages);

		int numMoves = -1;
		PlaceId* moves = DvGetValidMoves(dv, &numMoves);
		assert(numMoves == 4);
		sortPlaces(moves, numMoves);
		assert(moves[0] == GALATZ);
		assert(moves[1] == KLAUSENBURG);
		assert(moves[2] == HIDE);
		assert(moves[3] == DOUBLE_BACK_1);
		free(moves);

		printf("Test passed!\n");
		DvFree(dv);
	}
	system("pause");
	{///////////////////////////////////////////////////////////////////

		printf("Test for DvWhereCanIGo 1\n");

		char* trail =
			"GGE.... SGE.... HGE.... MGE.... DKL.V.. "
			"GGE.... SGE.... HGE.... MGE.... DD1T... "
			"GGE.... SGE.... HGE.... MGE.... DBCT... "
			"GGE.... SGE.... HGE.... MGE.... DHIT... "
			"GGE.... SGE.... HGE.... MGE....";

		Message messages[24];
		DraculaView dv = DvNew(trail, messages);

		int numLocs = -1;
		PlaceId* locs = DvWhereCanIGo(dv, &numLocs);
		assert(numLocs == 4);
		sortPlaces(locs, numLocs);
		assert(locs[0] == BELGRADE);
		assert(locs[1] == CONSTANTA);
		assert(locs[2] == GALATZ);
		assert(locs[3] == SOFIA);
		free(locs);

		printf("Test passed!\n");
		DvFree(dv);
	}
	system("pause");
	*/
	// extra test 1 : hunter encounters a trap 1
	{///////////////////////////////////////////////////////////////////

		printf("Test hunter encounters a trap\n");

		char* trail =
			"GGE.... SGE.... HGE.... MGE.... DSTT... "
			"GGE.... SGE.... HGE.... MGE.... DCOT... "
			"GSTT... SGE.... HGE.... MGE....";

		Message messages[24];
		DraculaView dv = DvNew(trail, messages);


		assert(DvGetScore(dv) == GAME_START_SCORE - 2 * SCORE_LOSS_DRACULA_TURN);
		assert(DvGetHealth(dv, PLAYER_LORD_GODALMING) == GAME_START_HUNTER_LIFE_POINTS - LIFE_LOSS_TRAP_ENCOUNTER);
		int numTraps = -1;
		PlaceId * traps = DvGetTrapLocations(dv, &numTraps);
		assert(numTraps == 1);
		sortPlaces(traps, numTraps);
		assert(traps[0] == COLOGNE);
		free(traps);

		printf("Test passed!\n");
		DvFree(dv);
	}

	// extra test 2 : hunter encounters a trap 2(with HIDE)
	{///////////////////////////////////////////////////////////////////

		printf("Test hunter encounters a trap\n");

		char* trail =
			"GGE.... SGE.... HGE.... MGE.... DSTT... "
			"GGE.... SGE.... HGE.... MGE.... DHIT... "
			"GGE.... SGE.... HGE.... MGE.... DCOT... "
			"GSTT... SGE.... HGE.... MGE....";

		Message messages[24];
		DraculaView dv = DvNew(trail, messages);

		assert(DvGetScore(dv) == GAME_START_SCORE - 3 * SCORE_LOSS_DRACULA_TURN);
		assert(DvGetHealth(dv, PLAYER_LORD_GODALMING) == GAME_START_HUNTER_LIFE_POINTS - LIFE_LOSS_TRAP_ENCOUNTER);
		int numTraps = -1;
		PlaceId * traps = DvGetTrapLocations(dv, &numTraps);
		assert(numTraps == 2);
		sortPlaces(traps, numTraps);
		assert(traps[0] == COLOGNE);
		assert(traps[1] == STRASBOURG);
		free(traps);

		printf("Test passed!\n");
		DvFree(dv);
	}

	// extra test 3 : Hunter Die
	{///////////////////////////////////////////////////////////////////

		printf("Test 3\n");

		char* trail =
			"GGE.... SGE.... HGE.... MGE.... DST.V.. "
			"GGE.... SGE.... HGE.... MGE.... DHIT... "
			"GGE.... SGE.... HGE.... MGE.... DPAT... "
			"GPATD.. SGE.... HGE.... MGE.... DBUT... "
			"GBUTD.. SGE.... HGE.... MGE....";


		Message messages[24];
		DraculaView dv = DvNew(trail, messages);

		//return EXIT_SUCCESS;
		assert(DvGetScore(dv) == GAME_START_SCORE - 4 * SCORE_LOSS_DRACULA_TURN - SCORE_LOSS_HUNTER_HOSPITAL);
		assert(DvGetHealth(dv, PLAYER_LORD_GODALMING) == 0);
		assert(DvGetHealth(dv, PLAYER_DRACULA) == GAME_START_BLOOD_POINTS - 2 * LIFE_LOSS_HUNTER_ENCOUNTER);
		assert(DvGetPlayerLocation(dv, PLAYER_DRACULA) == BRUSSELS);
		assert(DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) == ST_JOSEPH_AND_ST_MARY);

		int numTraps = -1;
		PlaceId * traps = DvGetTrapLocations(dv, &numTraps);
		assert(numTraps == 1);
		sortPlaces(traps, numTraps);
		assert(traps[0] == STRASBOURG);
		free(traps);

		printf("Test passed!\n");
		DvFree(dv);
	}

	// extra test 4 : Dracula Doubleback to CD
	{///////////////////////////////////////////////////////////////////

		printf("Test 4\n");

		char* trail =
			"GGE.... SGE.... HGE.... MGE.... DGA.V.. "
			"GGE.... SGE.... HGE.... MGE.... DCDT... "
			"GGE.... SGE.... HGE.... MGE.... DKLT... "
			"GGE.... SGE.... HGE.... MGE.... DD2T... "
			"GGE.... SGE.... HGE.... MGE....";


		Message messages[24];
		DraculaView dv = DvNew(trail, messages);

		//return EXIT_SUCCESS;
		assert(DvGetScore(dv) == GAME_START_SCORE - 4 * SCORE_LOSS_DRACULA_TURN); 
		/*printf("Health = %d", DvGetHealth(dv,PLAYER_DRACULA));
		return EXIT_SUCCESS;*/
		assert(DvGetHealth(dv, PLAYER_DRACULA) == GAME_START_BLOOD_POINTS + 2 * LIFE_GAIN_CASTLE_DRACULA);
		assert(DvGetPlayerLocation(dv, PLAYER_DRACULA) == CASTLE_DRACULA);

		int numTraps = -1;
		PlaceId * traps = DvGetTrapLocations(dv, &numTraps);
		assert(numTraps == 3);
		sortPlaces(traps, numTraps);
		assert(traps[0] == CASTLE_DRACULA);
		assert(traps[1] == CASTLE_DRACULA);
		assert(traps[2] == KLAUSENBURG);
		assert(DvGetVampireLocation(dv) == GALATZ);
		free(traps);

		printf("Test passed!\n");
		DvFree(dv);
	}

	// extra test 5 : Dracula Doubleback to CD and HIDE
	{///////////////////////////////////////////////////////////////////

		printf("Test 5\n");

		char* trail =
			"GGE.... SGE.... HGE.... MGE.... DGA.V.. "
			"GGE.... SGE.... HGE.... MGE.... DCDT... "
			"GGE.... SGE.... HGE.... MGE.... DKLT... "
			"GGE.... SGE.... HGE.... MGE.... DD2T... "
			"GGE.... SGE.... HGE.... MGE.... DHIT... "
			"GGE.... SGE.... HGE.... MGE....";


		Message messages[24];
		DraculaView dv = DvNew(trail, messages);

		//return EXIT_SUCCESS;
		assert(DvGetScore(dv) == GAME_START_SCORE - 5 * SCORE_LOSS_DRACULA_TURN);
		assert(DvGetHealth(dv, PLAYER_DRACULA) == GAME_START_BLOOD_POINTS + 3 * LIFE_GAIN_CASTLE_DRACULA);
		assert(DvGetPlayerLocation(dv, PLAYER_DRACULA) == CASTLE_DRACULA);

		int numTraps = -1;
		PlaceId * traps = DvGetTrapLocations(dv, &numTraps);
		assert(numTraps == 4);
		sortPlaces(traps, numTraps);
		assert(traps[0] == CASTLE_DRACULA);
		assert(traps[1] == CASTLE_DRACULA);
		assert(traps[2] == CASTLE_DRACULA);
		assert(traps[3] == KLAUSENBURG);
		assert(DvGetVampireLocation(dv) == GALATZ);
		free(traps);

		printf("Test passed!\n");
		DvFree(dv);
	}

	// extra test 6 : Dracula from sea to land then doubleback to sea
	{///////////////////////////////////////////////////////////////////

		printf("Test 6\n");

		char* trail =
			"GGE.... SGE.... HGE.... MGE.... DAM.V.. "
			"GGE.... SGE.... HGE.... MGE.... DNS.... "
			"GGE.... SGE.... HGE.... MGE.... DEDT... "
			"GGE.... SGE.... HGE.... MGE.... DD2.... "
			"GGE.... SGE.... HGE.... MGE....";


		Message messages[24];
		DraculaView dv = DvNew(trail, messages);

		//return EXIT_SUCCESS;
		assert(DvGetScore(dv) == GAME_START_SCORE - 4 * SCORE_LOSS_DRACULA_TURN);
		assert(DvGetHealth(dv, PLAYER_DRACULA) == GAME_START_BLOOD_POINTS - 2 * LIFE_LOSS_SEA);
		assert(DvGetPlayerLocation(dv, PLAYER_DRACULA) == NORTH_SEA);

		int numTraps = -1;
		PlaceId * traps = DvGetTrapLocations(dv, &numTraps);
		assert(numTraps == 1);
		sortPlaces(traps, numTraps);
		assert(traps[0] == EDINBURGH);
		assert(DvGetVampireLocation(dv) == AMSTERDAM);
		free(traps);

		printf("Test passed!\n");
		DvFree(dv);
	}

	// extra test 7 : Dracula TP CD
	{///////////////////////////////////////////////////////////////////

		printf("Test 7\n");

		char* trail =
			"GGE.... SGE.... HGE.... MGE.... DSW.V.. "
			"GGE.... SGE.... HGE.... MGE.... DIR.... "
			"GGE.... SGE.... HGE.... MGE.... DDUT... "
			"GGE.... SGE.... HGE.... MGE.... DGWT... "
			"GGE.... SGE.... HGE.... MGE.... DD2T... "
			"GGE.... SGE.... HGE.... MGE.... DHIT... "
			"GGE.... SGE.... HGE.... MGE.... DTPT.V. "
			"GGE.... SGE.... HGE.... MGE....";


		Message messages[24];
		DraculaView dv = DvNew(trail, messages);

		//return EXIT_SUCCESS;

		assert(DvGetScore(dv) == GAME_START_SCORE - 7 * SCORE_LOSS_DRACULA_TURN - SCORE_LOSS_VAMPIRE_MATURES);
		assert(DvGetHealth(dv, PLAYER_DRACULA) == GAME_START_BLOOD_POINTS - LIFE_LOSS_SEA + LIFE_GAIN_CASTLE_DRACULA);
		assert(DvGetPlayerLocation(dv, PLAYER_DRACULA) == CASTLE_DRACULA);

		int numTraps = -1;
		PlaceId * traps = DvGetTrapLocations(dv, &numTraps);
		assert(numTraps == 5);
		sortPlaces(traps, numTraps);
		assert(traps[0] == CASTLE_DRACULA);
		assert(traps[1] == DUBLIN);
		assert(traps[2] == DUBLIN);
		assert(traps[3] == DUBLIN);
		assert(traps[4] == GALWAY);

		assert(DvGetVampireLocation(dv) == NOWHERE);
		free(traps);

		printf("Test passed!\n");
		DvFree(dv);
	}

	// extra test 8 : Dracula trap vanishes
	{///////////////////////////////////////////////////////////////////

		printf("Test 8\n");

		char* trail =
			"GGE.... SGE.... HGE.... MGE.... DED.V.. "
			"GST.... SST.... HST.... MST.... DMNT... "
			"GGE.... SGE.... HGE.... MGE.... DLOT... "
			"GST.... SST.... HST.... MST.... DSWT... "
			"GGE.... SGE.... HGE.... MGE.... DLVT... "
			"GST.... SST.... HST.... MST.... DIR.... "
			"GGE.... SGE.... HGE.... MGE.... DDUT.V. "
			"GST.... SST.... HST.... MST.... DGWT.M. "
			"GGE.... SGE.... HGE.... MGE.... DHIT.M. "
			"GST.... SST.... HST.... MST....";


		Message messages[24];
		DraculaView dv = DvNew(trail, messages);

		//return EXIT_SUCCESS;

		assert(DvGetScore(dv) == GAME_START_SCORE - 9 * SCORE_LOSS_DRACULA_TURN - SCORE_LOSS_VAMPIRE_MATURES);
		assert(DvGetHealth(dv, PLAYER_DRACULA) == GAME_START_BLOOD_POINTS - LIFE_LOSS_SEA);
		assert(DvGetPlayerLocation(dv, PLAYER_DRACULA) == GALWAY);

		int numTraps = -1;
		PlaceId * traps = DvGetTrapLocations(dv, &numTraps);
		assert(numTraps == 5);
		sortPlaces(traps, numTraps);
		assert(traps[0] == DUBLIN);
		assert(traps[1] == GALWAY);
		assert(traps[2] == GALWAY);
		assert(traps[3] == LIVERPOOL);
		assert(traps[4] == SWANSEA);


		assert(DvGetVampireLocation(dv) == NOWHERE);
		free(traps);

		printf("Test passed!\n");
		DvFree(dv);
	}

	// extra test 9 : a complete game
	{///////////////////////////////////////////////////////////////////

		printf("Test complete game\n");

		char* trail =
			"GSW.... SLS.... HMR.... MHA.... DSJ.V.. "
			"GLO.... SAL.... HCO.... MBR.... DBET... "
			"GED.... SBO.... HLI.... MPR.... DKLT... "
			"GLV.... SNA.... HNU.... MBD.... DCDT... "
			"GIR.... SPA.... HPR.... MKLT... DHIT... "
			"GAO.... SST.... HSZ.... MCDTTD. DGAT... "
			"GMS.... SFL.... HKL.... MSZ.... DCNT.V. "
			"GTS.... SRO.... HBC.... MCNTD.. DBS..M. "
			"GIO.... SBI.... HCN.... MCN.... DIO.... "
			"GIO.... SAS.... HBS.... MCN.... DTS.... "
			"GTS.... SAS.... HIO.... MBS.... DMS.... "
			"GMS.... SIO.... HTS.... MIO.... DAO..M. "
			"GAO.... STS.... HMS.... MTS.... DNS.... "
			"GBB.... SMS.... HAO.... MMS.... DED.V.. "
			"GNA.... SAO.... HEC.... MAO.... DMNT... "
			"GBO.... SIR.... HLE.... MEC.... DD2T... "
			"GSR.... SDU.... HBU.... MPL.... DHIT... "
			"GSN.... SIR.... HAM.... MLO.... DTPT... "
			"GAL.... SAO.... HCO.... MEC.... DCDT... "
			"GMS.... SMS.... HFR.... MLE.... DKLT.V. "
			"GTS.... STS.... HBR.... MCO.... DGAT.M. "
			"GIO.... SIO.... HBD.... MLI.... DD3T.M. "
			"GBS.... SBS.... HKLT... MBR.... DHI..M. "
			"GCN.... SCN.... HCDTTTD MVI.... DTPT... "
			"GGAT... SGA.... HSZ.... MBC.... DCDT... "
			"GCDTTD. SCDD... HKL.... MGA.... DKLT... "
			"GSZ.... SKLTD.. HKLD... MKLD... DBC.V.. "
			"GBD.... SBE.... HGA.... MBCVD.. DSOT... "
			"GSZ.... SSOTD.. HBC.... MSOD...";


		Message messages[24];
		DraculaView dv = DvNew(trail, messages);

		//return EXIT_SUCCESS;

		assert(DvGetRound(dv) == 28);
		assert(DvGetScore(dv) == 282);
		assert(DvGetHealth(dv, PLAYER_LORD_GODALMING) == 9);
		assert(DvGetHealth(dv, PLAYER_DR_SEWARD) == 3);
		assert(DvGetHealth(dv, PLAYER_VAN_HELSING) == 8);
		assert(DvGetHealth(dv, PLAYER_MINA_HARKER) == 0);
		assert(DvGetHealth(dv, PLAYER_DRACULA) == 0);
		assert(DvGetPlayerLocation(dv, PLAYER_LORD_GODALMING) == SZEGED);
		assert(DvGetPlayerLocation(dv, PLAYER_DR_SEWARD) == SOFIA);
		assert(DvGetPlayerLocation(dv, PLAYER_VAN_HELSING) == BUCHAREST);
		assert(DvGetPlayerLocation(dv, PLAYER_MINA_HARKER) == ST_JOSEPH_AND_ST_MARY);
		assert(DvGetPlayerLocation(dv, PLAYER_DRACULA) == SOFIA);

		int numTraps = -1;
		PlaceId * traps = DvGetTrapLocations(dv, &numTraps);
		assert(numTraps == 0);
		assert(DvGetVampireLocation(dv) == NOWHERE);
		free(traps);

		printf("Test passed!\n");
		DvFree(dv);
	}

	return EXIT_SUCCESS;
}