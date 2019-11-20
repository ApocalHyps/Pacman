#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

const int column = 10;
const int row = 10;

typedef struct Coord Coord;
struct Coord{
  int x;
  int y;
};

typedef struct Node{
	bool walkable;
	bool wayToGo;
	Coord pos;
	int gCost;
	int hCost;
	int fCost; // fCost = gCost + hCost
	struct Node *parent;
} Node;

typedef struct NodeList{
	Node * pNode;
	struct NodeList * next;
	struct NodeList * previous;
} NodeList;

NodeList * list_prepend(NodeList *old, Node *pNode);
NodeList * list_create (Node *pNode);
NodeList * list_remove(NodeList *list, int index);
NodeList * getNeighbours(Node grid[row][column], Node * pNode);
bool list_isIn(NodeList * pitem, NodeList * list);
int getDistance(Node nodeA, Node nodeB);
void findPath(Node grid[row][column], Node *startNode, Node *targetNode);


NodeList * list_create (Node *pNode)
{
   NodeList *list = malloc(sizeof(NodeList)); /* allocation (en vert sur le diagramme) et affectation à la variable list (en bleu) */
   if (list)                           /* si l'allocation a réussi */
   {
       list->pNode = pNode;              /* affectation du champ pNode (en rouge) */
       list->next = NULL;  /* affectation du champ next à la liste vide */
       list->previous = NULL;            /*Node parent dans la liste*/
   }
   return list;                        /* retour de la liste (correctement allouée et affectée ou NULL) */
 }

NodeList * list_remove(NodeList *list, int index)
{
	NodeList **plist = &list;
	for (int i = 0; i < index; ++i)
	{
		plist = &(*plist)->next;
	}
	if((*plist)->next)
	((*plist)->next)->previous = (*plist)->previous;
	if((*plist)->previous)
	((*plist)->previous)->next = (*plist)->next;

	free((*plist));

	return list;
}

NodeList * list_prepend(NodeList *old, Node *pNode)
{

    NodeList *list = list_create(pNode); /* création et affectation d'une liste d'un élément (en vert sur le diagramme) */
    if (list){                     /* si l'allocation mémoire a réussi */
       list->next = old;             /*     accrochage de l'ancienne liste à la nouvelle  (en bleu sur le diagramme) */
       (*old).previous = list;
    }
    return list;                    /* retour de la nouvelle liste (ou NULL si l'allocation a échoué) */
}

NodeList * getNeighbours(Node grid[row][column], Node * pNode)
{
	int x, y;
	NodeList * list = NULL;
	for(x = -1; x <= 1; x++){
		for(y = -1; y <= 1; y++){
			if(x == y || x == -y)
				continue;

			//le voisin est-il sur la carte (Rappel pour pacman: a modifier car la topologie est cylindrique)
			//Considérons ici un plan bornée
			int checkX = pNode->pos.x + x;
			int checkY = pNode->pos.y + y;
			if (checkX >= 0 && checkX < column && checkY >= 0 && checkY < row){
				list = list_prepend(list, &(grid[checkY][checkX]));
			}
		}		
	}
	return list;
}

bool list_isIn(NodeList * pitem, NodeList * list)
{
	NodeList *tmp=list; /* opération en vert sur le diagramme */
	while (tmp)
	{
	    if(tmp == pitem) return true;
	    tmp = tmp->next; /* opération en bleu sur le diagramme */
	}
	return false;
}

int getDistance(Node nodeA, Node nodeB)
{/* A modifier pour pacman avec sa topologie*/
	int dstX = abs(nodeB.pos.x-nodeA.pos.x);
	int dstY = abs(nodeB.pos.y-nodeA.pos.y);
	int dist;
	//int dst = dstX+dstY; // Si il n'y a pas de déplacement diagonal
	if (dstX>dstY) dist = 14*dstY + 10*(dstX-dstY);
	if (dstX<=dstY) dist = 14*dstX + 10*(dstY-dstX);
	return dist;
}

void findPath(Node grid[row][column], Node *startNode, Node *targetNode){
	//Création openSet
	NodeList *openSet = NULL;

	//Création closeSet
	NodeList *closeSet = NULL;
	
	// définition des nodes elementaires (implicite)

	openSet = list_prepend(openSet, startNode);
	//On loop
	while(openSet->pNode != NULL){
		Node *currentNode = openSet->pNode;

		//Recherche meilleur candidat, a ameliorer...
		NodeList *candidat = openSet->next;
		NodeList *tmp = openSet->next; // part de 1 jusqu'au dernier element (où next = NULL)
		while (tmp)
		{
    		if (tmp->pNode->fCost < currentNode->fCost ||(tmp->pNode->fCost == currentNode->fCost && tmp->pNode->hCost < currentNode->hCost)){
				currentNode = tmp->pNode;
				candidat = tmp;
			}
    		tmp = tmp->next;
		}
		//retirer currentNode de openSet grace a candidat
		if((candidat)->next)
		((candidat)->next)->previous = (candidat)->previous;
		if((candidat)->previous)
		((candidat)->previous)->next = (candidat)->next;
		//openSet = list_remove(openSet, index, &openSetCount);

		closeSet = list_prepend(closeSet, currentNode);

		/*if (currentNode.pos.x == targetNode.pos.x && currentNode.pos.y == targetNode.pos.y){
			return; // chemin trouvé
		}*/
		if(currentNode == targetNode){
			Node * node = currentNode;
			while(currentNode != startNode){
				node->wayToGo = true;
				node = node->parent;
			}
			return;
		}
		
		NodeList * neighbours = NULL;
		neighbours = getNeighbours(grid, currentNode);

		NodeList *neighbour=neighbours;
		while(neighbour)
		{
			if(list_isIn(neighbour, closeSet) || neighbour->pNode->walkable == false) {/*printf("in closedSet\n");*/ continue;}

			int newCostToNeighbour = currentNode->gCost + getDistance(*currentNode, *(neighbour->pNode));
			if (newCostToNeighbour < neighbour->pNode->gCost || list_isIn(neighbour, openSet) == false){
				neighbour->pNode->gCost = newCostToNeighbour;
				neighbour->pNode->hCost = getDistance(*(neighbour->pNode), *targetNode);
				neighbour->pNode->fCost = neighbour->pNode->gCost + neighbour->pNode->hCost;
				neighbour->pNode->parent = currentNode;

				if(list_isIn(neighbour, openSet) == false) openSet = list_prepend(openSet, neighbour->pNode);
			}
			neighbour = neighbour->next;
		}
		free(neighbours);
	}

	free(openSet);
	free(closeSet);
}





int main()
{
	Coord goal = {8,8};
	Coord start  = {2, 2};
	//Coord *mur = {{4, 2}, {4, 3}, {4, 4}, {4, 5}, {4, 6}, {4, 7}, {4, 8}}
	//création de la map d'essai
	bool map[row][column];



	//initialisation des murs
	int x, y;
	for(x = 0; x<column; x++){
		for(y = 0; y<row; y++){
			if(y == 4 && x<9 && x>1){
				map[y][x] = false;
			}else{
				map[y][x] = true;
			}
		}
	}

	//Création du tableau des nodes
	Node grid[row][column];
	//int dstXsup = 2*abs(goal.x-start.x);
	//int dstYsup = 2*abs(goal.y-start.y);
	for(x = 0; x<column; x++){
		for(y = 0; y<row; y++){
				grid[y][x] = (Node){map[y][x], false, (Coord){x, y}, 0, 0, 0, NULL};
		}
	}	
	// initialisation de start et goal
	int dist = getDistance(grid[goal.y][goal.x], grid[start.y][start.x]);
	grid[start.y][start.x].gCost = dist;
	grid[goal.y][goal.x].hCost = dist;
	grid[start.y][start.x].fCost = dist;
	grid[goal.y][goal.x].fCost = dist;
	
	findPath(grid, &(grid[start.y][start.x]), &(grid[goal.y][goal.x]));


	//Display
	for(x = 0; x<column; x++){
		for(y = 0; y<row; y++){
			if(grid[y][x].pos.x == start.x && grid[y][x].pos.y == start.y){
				printf("è");
			}else if(grid[y][x].pos.x == goal.x && grid[y][x].pos.y == goal.y){
				printf("@");	
			}else if(grid[y][x].wayToGo){
				printf("°");				
			}else if(grid[y][x].walkable){
				printf(".");	
			}else{
				printf("#");
			}	
		}
		printf("\n");
			
	}	

	return 0;
}
