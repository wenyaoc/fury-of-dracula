////////////////////////////////////////////////////////////////////////
// Runs a player's game turn ...
//
// Can  produce  either a hunter player or a Dracula player depending on
// the setting of the I_AM_DRACULA #define
//
// This  is  a  dummy  version of the real player.c used when you submit
// your AIs. It is provided so that you can test whether  your  code  is
// likely to compile ...
//
// Note that this is used to drive both hunter and Dracula AIs. It first
// creates an appropriate view, and then invokes the relevant decideMove
// function,  which  should  use the registerBestPlay() function to send
// the move back.
//
// The real player.c applies a timeout, and will halt your  AI  after  a
// fixed  amount of time if it doesn 't finish first. The last move that
// your AI registers (using the registerBestPlay() function) will be the
// one used by the game engine. This version of player.c won't stop your
// decideMove function if it goes into an infinite loop. Sort  that  out
// before you submit.
//
// Based on the program by David Collien, written in 2012
//
// 2017-12-04	v1.1	Team Dracula <cs2521@cse.unsw.edu.au>
// 2018-12-31	v1.2	Team Dracula <cs2521@cse.unsw.edu.au>
// 2020-07-10	v1.3	Team Dracula <cs2521@cse.unsw.edu.au>
//
////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Game.h"
#ifdef I_AM_DRACULA
# include "dracula.h"
# include "DraculaView.h"
#else
# include "hunter.h"
# include "HunterView.h"
#endif

// Moves given by registerBestPlay are this long (including terminator)
#define MOVE_SIZE 3

// The minimum static globals I can get away with
static char latestPlay[MOVE_SIZE] = "";
static char latestMessage[MESSAGE_SIZE] = "";

// A pseudo-generic interface, which defines
// - a type `View',
// - functions `ViewNew', `decideMove', `ViewFree',
// - a trail `xtrail', and a message buffer `xmsgs'.
#ifdef I_AM_DRACULA

typedef DraculaView View;

# define ViewNew DvNew
# define decideMove decideDraculaMove
# define ViewFree DvFree

# define xPastPlays "GED.... SBD.... HPA.... MCD.... DZU.V.. GMN.... SVE.... HMA.... MCD.... DSTT... GLV.... SMU.... HSR.... MKL.... "
# define xMsgs { "", "", "", "" }

#else

typedef HunterView View;

# define ViewNew HvNew
# define decideMove decideHunterMove
# define ViewFree HvFree

# define xPastPlays "GED.... SBD.... HPA.... MCD.... DVA.V.. GMN.... SBE.... HMA.... MKL.... DATT... GLV.... SGA.... HGR.... MSZ.... DIO.... GSW.... SKL.... HAL.... MGA.... DTS.... GLO.... SBC.... HBA.... MCN.... DROT... GPL.... SBD.... HSN.... MGA.... DD2.... GEC.... SGA.... HBB.... MCD.... DGOT.V. GLE.... SCN.... HSN.... MKL.... DVET.M. GNA.... SVR.... HSR.... MBD.... DAS.... GCF.... SSA.... HPA.... MKL.... DBIT... GMR.... SBE.... HNA.... MGA.... DNPT.M. GGOT... SSZ.... HPA.... MSZ.... DROT... GMI.... SBD.... HMR.... MVET... DTS.... GVE.... SVE.... HMI.... MAS.... DMS.... GPR.... SVI.... HMR.... MVE.... DBAT... GLI.... SMU.... HMS.... MBD.... DTOT.M. GCO.... SST.... HCG.... MGA.... DMRT.M. GAM.... SMU.... HMS.... MBC.... DGOT.M. GBU.... SMI.... HAL.... MGA.... DTS.... GBO.... SVE.... HGR.... MCN.... DROT... GNA.... SFL.... HMA.... MGA.... DBIT.M. GBB.... SVE.... HBO.... MCD.... DNPT.M. GAO.... SBD.... HCF.... MKL.... DD4..M. GIR.... SSZ.... HMR.... MBE.... DIO..M. GLV.... SJM.... HGE.... MGA.... DBS.... GIR.... SZA.... HMR.... MCN...." 
# define xMsgs { "", "", "" }

#endif

int main(void)
{
	char *pastPlays = xPastPlays;
	Message msgs[] = xMsgs;

	View state = ViewNew(pastPlays, msgs);
	decideMove(state);
	ViewFree(state);

	printf("Move: %s, Message: %s\n", latestPlay, latestMessage);
	return EXIT_SUCCESS;
}

// Saves characters from play (and appends a terminator)
// and saves characters from message (and appends a terminator)
void registerBestPlay(char *play, Message message)
{
	strncpy(latestPlay, play, MOVE_SIZE - 1);
	latestPlay[MOVE_SIZE - 1] = '\0';

	strncpy(latestMessage, message, MESSAGE_SIZE - 1);
	latestMessage[MESSAGE_SIZE - 1] = '\0';
}
