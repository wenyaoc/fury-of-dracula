#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "DraculaView.h"
#include "Game.h"
#include "GameView.h"
#include "Map.h"
// add your own #includes here
#include "PlayerDataList.h"
#include <string.h>
// TODO: ADD YOUR OWN STRUCTS HERE

#define MAXCHAR 8
#define MAXLINE 40

struct draculaView {
	// TODO: ADD FIELDS HERE
	Round round;
	int score;
	playerData data[5];
};

Player getPlayer(char c);
void split(char* src, const char* separator, char** dest, int* num);
void trailLoad(DraculaView dv, char * play);
PlaceId SetLocation(DraculaView dv, char* location);
void PastEvent(DraculaView dv, char* line, int player);
int countTraps(DraculaView dv);
int getHistoryTraps(DraculaView dv, int previous);
////////////////////////////////////////////////////////////////////////
// Constructor/Destructor

DraculaView DvNew(char *pastPlays, Message messages[])
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	DraculaView new_dv = malloc(sizeof(*new_dv));
	if (new_dv == NULL) {
		fprintf(stderr, "Couldn't allocate DraculaView\n");
		exit(EXIT_FAILURE);
	}
	int totalLength = strlen(pastPlays); 
	new_dv->round = (totalLength + 1) / MAXLINE;
	new_dv->score = GAME_START_SCORE;

	//Set Player
	for (int i = 0; i < 5; i++) {
		new_dv->data[i].player = i;
		new_dv->data[i].health = GAME_START_HUNTER_LIFE_POINTS;
		new_dv->data[i].turn = 0;
		new_dv->data[i].first = NULL;
	}
	new_dv->data[PLAYER_DRACULA].health = GAME_START_BLOOD_POINTS;

	if (totalLength + 1 < MAXCHAR) return new_dv;

	char * play = calloc(MAXCHAR, sizeof(char));
	for (int i = 0; i <= totalLength; i++) {
		play[i%8] = pastPlays[i];
		if ((i%8) == 7) {
			trailLoad(new_dv, play);
		}
	}

	free(play);
	return new_dv;
}

void DvFree(DraculaView dv)
{
    // TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION

    for (int i = 0; i < 5; i++) {
        ListFree(dv->data[i].first);
    }
    free(dv);
}

////////////////////////////////////////////////////////////////////////
// Game State Information

Round DvGetRound(DraculaView dv)
{
    // TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return dv->round;
}

int DvGetScore(DraculaView dv)
{
    // TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return dv->score;
}

int DvGetHealth(DraculaView dv, Player player)
{
    // TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    return dv->data[player].health;
}

PlaceId DvGetPlayerLocation(DraculaView dv, Player player)
{
    // TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    if(dv->data[player].turn == 0)
        return NOWHERE;
    else 
        return dv->data[player].first->place;
}

PlaceId DvGetVampireLocation(DraculaView dv)
{
    // TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    if(dv->data[PLAYER_DRACULA].turn == 0)
        return NOWHERE;
    HistoryNode curr = dv->data[PLAYER_DRACULA].first;
    int counter = 0;
    while (curr != NULL && counter < 5) {
        if (curr->vampire == true) {
            return curr->place;
        }
        counter++;
        curr = curr->next;
    }
    return NOWHERE;
}

PlaceId *DvGetTrapLocations(DraculaView dv, int *numTraps)
{
    // TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
    *numTraps = 0;
    if(dv->data[PLAYER_DRACULA].turn == 0) return NULL;
    HistoryNode curr = dv->data[PLAYER_DRACULA].first;
	
    int counter = 0;
    PlaceId * place = NULL;
    while (curr != NULL && counter < 5 && counter < dv->round) {
        if (curr->trapNumber > 0) {
            place = realloc(place, (*numTraps + 1) * sizeof(PlaceId));
			if (curr->place == HIDE) {
				if (curr->next->place >= DOUBLE_BACK_1 && curr->next->place <= DOUBLE_BACK_5)
					place[*numTraps] = findDBCity(curr->next)->place;
				else
					place[*numTraps] = curr->next->place;

			}
			else if (curr->place >= DOUBLE_BACK_1 && curr->place <= DOUBLE_BACK_5) {
				place[*numTraps] = findDBCity(curr)->place;
			} else
				place[*numTraps] = curr->place;
            *numTraps = *numTraps + 1;			
        }
        counter++;
        curr = curr->next;
    }
    return place;
}


////////////////////////////////////////////////////////////////////////
// Making a Move

PlaceId* DvGetValidMoves(DraculaView dv, int* numReturnedMoves)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedMoves = 0;
	if (dv->data[PLAYER_DRACULA].turn == 0) return NULL;
	int DBnumber = 0;
	Map m = MapNew();
	ConnList connection = MapGetConnections(m, dv->data[PLAYER_DRACULA].first->place);
	ConnList curr = connection;
	PlaceId * move = NULL;
	while (curr != NULL) {
		if (curr->type == ROAD || curr->type == BOAT) {
			if (canGo(dv->data[PLAYER_DRACULA].first, curr->p)) {
				move = realloc(move, (*numReturnedMoves + 1) * sizeof(PlaceId));
				move[*numReturnedMoves] = curr->p;
				*numReturnedMoves = *numReturnedMoves + 1;
			}
			else if ((DBnumber = getDoubleBackNum(dv->data[PLAYER_DRACULA].first, curr->p)) > 0) {
				move = realloc(move, (*numReturnedMoves + 1) * sizeof(PlaceId));
				move[*numReturnedMoves] = DOUBLE_BACK_1 + DBnumber - 1;
				*numReturnedMoves = *numReturnedMoves + 1;
			}
		}
		curr = curr->next;
	}
	if (canDoubleBack(dv->data[PLAYER_DRACULA].first)) {
		move = realloc(move, (*numReturnedMoves + 1) * sizeof(PlaceId));
		move[*numReturnedMoves] = DOUBLE_BACK_1;
		*numReturnedMoves = *numReturnedMoves + 1;
	}
	if (canHide(dv->data[PLAYER_DRACULA].first)) {
		move = realloc(move, (*numReturnedMoves + 1) * sizeof(PlaceId));
		move[*numReturnedMoves] = HIDE;
		*numReturnedMoves = *numReturnedMoves + 1;
	}

	for(int i = 0; i < *numReturnedMoves; i++) {
		printf("%s ", placeIdToName(move[i]));
	}
	return move;
}

PlaceId* DvWhereCanIGo(DraculaView dv, int* numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return DvWhereCanIGoByType(dv, true, true, numReturnedLocs);
}

PlaceId* DvWhereCanIGoByType(DraculaView dv, bool road, bool boat,
	int* numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	if (dv->data[4].turn == 0) return NULL;

	Map m = MapNew();
	ConnList connection = MapGetConnections(m, dv->data[4].first->place);
	ConnList curr = connection;
	PlaceId * place = NULL;
	while (curr != NULL) {
		if ((curr->type == ROAD && road == true) || (curr->type == BOAT && boat == true)) {
			if (canGo(dv->data[4].first, curr->p)) {
				place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
				place[*numReturnedLocs] = curr->p;
				*numReturnedLocs = *numReturnedLocs + 1;
			}
			else if (getDoubleBackNum(dv->data[4].first, curr->p) > 0) {
				place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
				place[*numReturnedLocs] = curr->p;
				*numReturnedLocs = *numReturnedLocs + 1;
			}
		}
		curr = curr->next;
	}
	if (canHide(dv->data[4].first)) {
		place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
		place[*numReturnedLocs] = dv->data[4].first->place;
		*numReturnedLocs = *numReturnedLocs + 1;
	}
	return place;
}

PlaceId* DvWhereCanTheyGo(DraculaView dv, Player player,
	int* numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	return DvWhereCanTheyGoByType(dv, player, true, true, true, numReturnedLocs);
}

PlaceId* DvWhereCanTheyGoByType(DraculaView dv, Player player,
	bool road, bool rail, bool boat,
	int* numReturnedLocs)
{
	// TODO: REPLACE THIS WITH YOUR OWN IMPLEMENTATION
	*numReturnedLocs = 0;
	if (player == PLAYER_DRACULA) return DvWhereCanIGoByType(dv, road, boat, numReturnedLocs);

	int playernum = player;
	if (dv->data[playernum].turn == 0) return NULL;
	Map m = MapNew();
	ConnList connection = MapGetConnections(m, dv->data[4].first->place);
	ConnList curr = connection;
	PlaceId * place = NULL;
	while (curr != NULL) {
		if ((curr->type == ROAD && road == true) || (curr->type == BOAT && boat == true)) {
			place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
			place[*numReturnedLocs] = dv->data[4].first->place;
			*numReturnedLocs = *numReturnedLocs + 1;
			continue;
		}
		if (curr->type == RAIL && rail == true && dv->round % 4 != 0) {
			if (dv->round % 4 == 1) {
				place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
				place[*numReturnedLocs] = dv->data[4].first->place;
				*numReturnedLocs = *numReturnedLocs + 1;
				continue;
			}

			ConnList connection2 = MapGetConnections(m, curr->p);
			ConnList curr2 = connection2;
			while (curr2 != NULL) {
				if (curr2->type == RAIL) {
					place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
					place[*numReturnedLocs] = dv->data[4].first->place;
					*numReturnedLocs = *numReturnedLocs + 1;
					if (dv->round % 4 == 3) {
						ConnList connection3 = MapGetConnections(m, curr->p);
						ConnList curr3 = connection3;
						while (curr3 != NULL) {
							if (curr3->type == RAIL) {
								place = realloc(place, (*numReturnedLocs + 1) * sizeof(PlaceId));
								place[*numReturnedLocs] = dv->data[4].first->place;
								*numReturnedLocs = *numReturnedLocs + 1;
							}
							curr3 = curr3->next;
						}
					}
				}
				curr2 = curr2->next;
			}
		}
		curr = curr->next;
	}
	return place;
}

////////////////////////////////////////////////////////////////////////
// Your own interface functions

// TODO

//backup function
/*
void split(char* src, const char* separator, char** dest, int* num) {
	char* pNext;
	int count = 0;
	if (src == NULL || strlen(src) == 0) 
		return;
	if (separator == NULL || strlen(separator) == 0)
		return;
	pNext = (char*)strtok(src, separator);
	while (pNext != NULL) {
		*dest++ = pNext;
		++count;
		pNext = (char*)strtok(NULL, separator);
	}
	*num = count;
}
*/

void trailLoad(DraculaView dv, char * play) {

	Player player = getPlayer(play[0]);
	char abbrev[3];
	abbrev[0] = play[1];
	abbrev[1] = play[2];
	abbrev[2] = '\0';
	PlaceId place = placeAbbrevToId(abbrev);
	if(player == PLAYER_DRACULA) {
		bool vampire = false;
		int trap = 0;
		if(play[3] == 'T')
			trap++;
		if(play[4] == 'V')
			vampire = true;
		HistoryNode new = creatNode(place, vampire, trap, true);
		dv->data[player] = addToHistory(dv->data[player], new);

		if (placeIsSea(place))
			dv->data[player].health -= LIFE_LOSS_SEA;
		/*
		handle M
		*/
		if (play[6] == 'V') {
			dv->score -= SCORE_LOSS_VAMPIRE_MATURES;
		}

		if (place == CASTLE_DRACULA)
			dv->data[player].health += LIFE_GAIN_CASTLE_DRACULA;
		/*
			handle hunters' health
		*/
		dv->score -= SCORE_LOSS_DRACULA_TURN;
	} else {
		HistoryNode new = creatNode(place, false, 0, true);
		dv->data[player] = addToHistory(dv->data[player], new);
		/*
			if teleport to hospital
		*/
		if (play[3] == 'T') {
			/*
				delete that trap
			*/
			dv->data[player].health -= LIFE_LOSS_TRAP_ENCOUNTER ; 

		} 
		if (play[3] == 'V' || play[4] == 'V') {
			/*
				delete that V
			*/
		}
		if (play[3] == 'D' || play[4] == 'D' || play[5] == 'D') {
			dv->data[player].health -= LIFE_LOSS_DRACULA_ENCOUNTER;
			dv->data[PLAYER_DRACULA].health -= LIFE_LOSS_HUNTER_ENCOUNTER;
		}
	}
}


Player getPlayer(char c){
	if (c == 'G') return PLAYER_LORD_GODALMING;
	if (c == 'S') return PLAYER_DR_SEWARD;
	if (c == 'H') return PLAYER_VAN_HELSING;
	if (c == 'M') return PLAYER_MINA_HARKER;
	else return PLAYER_DRACULA;		
}

/*
PlaceId SetLocation(DraculaView dv, char* location)
{
	char place[3];
	for (int m = 1; m < 3; m++) {
		place[m - 1] = location[m];
	}
	place[2] = '\0';

	return placeAbbrevToId(place);
}

void PastEvent(DraculaView dv, char* line, int player)
{
	char pevent[5];
	for (int m = 3; m < 7; m++) {
		pevent[m - 3] = line[m];
	}
	pevent[4] = '\0';

	if (player != PLAYER_DRACULA) {
		for (int i = 0; i < 4; i++) {
			switch (pevent[i]) {
				case 'T':
					break;
				case 'V':break;
				case 'D':
					dv->data[player].health -= 4;
					dv->data[PLAYER_DRACULA].health -= 10;
				case '.':break;
			}
		}
		return;
	}

	for (int i = 0; i < 4; i++) {
		switch (pevent[i]) {
			case 'T':
				dv->data->last->trapNumber += countTraps(dv);
			case 'V':
				if (i == 1);
				if (i == 2);
			case 'M':break;
			case '.':break;
		}
	}
}

int countTraps(DraculaView dv) {
	PlaceId place = dv->data[PLAYER_DRACULA].last->place;
	if (place == HIDE)
		return getHistoryTraps(dv, 1);
	if (place >= DOUBLE_BACK_1 && place <= DOUBLE_BACK_5)
		return getHistoryTraps(dv, DOUBLE_BACK_1 - 102);
	if (place == TELEPORT)
		return getHistoryTraps(dv, 1);

	return 0;
}

int getHistoryTraps(DraculaView dv, int previous) {
	HistoryNode history_node = dv->data[PLAYER_DRACULA].first;
	for (int i = 0; i < dv->data[i].turn; i++) {

		//Find the previous history node
		if (i < dv->data[i].turn - previous) {
			history_node = history_node->next;
			continue;
		}

		return history_node->trapNumber;
	}
	return 0;
}*/