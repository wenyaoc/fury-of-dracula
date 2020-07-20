#include <stdbool.h>
#include <stdlib.h>

#include "Game.h"
#include "Places.h"
#include "Map.h"

typedef struct _historyNode *HistoryNode;


typedef struct _historyNode {
	PlaceId place; //the real location
	bool vampire;
	int trapNumber;
    bool revealed;
	HistoryNode next;
} historyNode;

typedef struct _playerData {
	Player player;
	int health;
	int totalNumber; // total munber in history
	HistoryNode first; // the first node in the history list
	HistoryNode last;  // the last node in the history list
} playerData;


// free the whole history list
void ListFree(historyNode node);

// free the given node in the list
void NodeFree(historyNode node);

// creat a new node 
HistoryNode newNode(PlaceId place, bool vampire, bool trap, bool revealed);

// add the node to the head of the list  
//  每走新的一步的时候， 把node加在linklist的最前面
playerData addToHead(playerData data, HistoryNode newNode);

// add the node to the end of the list
playerData addToTail(playerData data, HistoryNode newNode);

// get a copy of the last N Node in the history list
// returns: the head of the new list
// !!!!!! make sure you free it after you use it ， 别忘了!!!!!!
HistoryNode getLastN(playerData data, int n);
