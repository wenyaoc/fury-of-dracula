  
#include <stdbool.h>
#include <stdlib.h>

#include "Game.h"
#include "Places.h"
#include "Map.h"

typedef struct historyNode *HistoryNode;

struct historyNode {
	PlaceId place; //the real location
	bool vampire;
	int trapNumber;
    	bool revealed;
	HistoryNode next;
};

typedef struct _playerData {
	Player player;
	int health;
	int totalNumber; // total munber in history
	HistoryNode first; // the first node in the history list, 别忘了初始化的时候赋值为NULL
} playerData;


// free the whole history list
void ListFree(HistoryNode node);

// creat a new node 
HistoryNode creatNode(PlaceId place, bool vampire, int trap, bool revealed);

// creat a copy of an exist noode
HistoryNode copyNode(HistoryNode prevNode);


// add the node to the head of the history  
//  每走新的一步的时候， 把node加在history的最前面
void addToHistory(playerData data, HistoryNode newNode);

// add the node to the end of the list
// 单纯link list操作， 与history无瓜
HistoryNode addToTail(HistoryNode list, HistoryNode newNode);

// get a copy of the last N Node in the history list
// returns: the head of the new list
// 别忘了 free！！！！//
HistoryNode getLastN(playerData data, int n);


// find out whether dracula can go to that city is in the trail(does not include HIDE and BOUBLE_BACK)
// 不包括判断Dracula是否能坐火车， 请自行判断
// return false if already in the trail
// return true if not in the trail
bool canGo(HistoryNode list, PlaceId place);

// find out whether dracula can double back to a city
// return #  for doubleback if can double back
// return 0 if cannot double back or do not need to use double back
int canDoubleBack(HistoryNode list, PlaceId place);


// find out whether dracula can hide in a city
// return the number in the trail if already in the trail
// return 0 if not in the trail
bool canHide(HistoryNode list);
