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

typedef struct Lister Lister;
struct Lister{
	Node * pNode;
	struct Lister* next;
	struct Lister* prev;
};

void push(Lister** head_ref, Node * new_data);
void list_remove(struct Lister **list, struct Lister * del);
Lister * getNeighbours(Node grid[row][column], Node * pNode);
bool list_isIn(Lister * pitem, Lister * list);
int getDistance(Node nodeA, Node nodeB);
void findPath(Node grid[row][column], Node *startNode, Node *targetNode);


void push(Lister** head_ref, Node* new_pNode) 
{ 
    /* allocate node */
    Lister* new_element = (Lister*)malloc(sizeof(Lister)); 
  
    /* put in the data  */
    new_element->pNode = new_pNode; 
  
    /* since we are adding at the beginning, 
    prev is always NULL */
    new_element->prev = NULL; 
  
    /* link the old list off the new node */
    new_element->next = (*head_ref); 
  
    /* change prev of head node to new node */
    if ((*head_ref) != NULL) 
        (*head_ref)->prev = new_element; 
  
    /* move the head to point to the new node */
    (*head_ref) = new_element; 
}

void list_remove(Lister **head_ref, Lister * del)
{
	/* base case */
	if (*head_ref == NULL || del == NULL) 
		return; 
	/* If node to be deleted is head node */
	if (*head_ref == del) 
		*head_ref = del->next; 
	/* Change next only if node to be deleted is NOT the last node */
	if (del->next != NULL) 
		del->next->prev = del->prev; 
	/* Change prev only if node to be deleted is NOT the first node */
	if (del->prev != NULL) 
		del->prev->next = del->next; 
	/* Finally, free the memory occupied by del*/
	free(del); 
	return; 
}

Lister * getNeighbours(Node grid[row][column], Node * pNode)
{
	int x, y;
	Lister * list = NULL;
	for(x = -1; x <= 1; x++){
		for(y = -1; y <= 1; y++){
			if(x == y || x == -y)
				continue;

			//le voisin est-il sur la carte (Rappel pour pacman: a modifier car la topologie est cylindrique)
			//Considérons ici un plan bornée
			int checkX = pNode->pos.x + x;
			int checkY = pNode->pos.y + y;
			if (checkX >= 0 && checkX < column && checkY >= 0 && checkY < row){
				push(&list, &(grid[checkY][checkX]));
			}
		}		
	}
	return list;
}

bool list_isIn(Lister * pitem, Lister * list)
{
	Lister *tmp=list; /* opération en vert sur le diagramme */
	while (tmp)
	{
	    if(tmp->pNode == pitem->pNode) return true;
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
	Lister *openSet = NULL;
	
	Lister *closeSet = NULL;

	push(&openSet, startNode);

	Node *currentNode = NULL;

	while(openSet){
		currentNode = openSet->pNode;

		Lister * candidat = openSet;

		Lister *tmp = openSet->next; // part de 1 jusqu'au dernier element (où next = NULL)
		while (tmp)
		{
    		if (tmp->pNode->fCost < currentNode->fCost ||(tmp->pNode->fCost == currentNode->fCost && tmp->pNode->hCost < currentNode->hCost)){
				currentNode = tmp->pNode;
				candidat = tmp;
			}
    		tmp = tmp->next;
		}
		
		list_remove(&openSet, candidat);

		push(&closeSet, currentNode);
		
		if(currentNode == targetNode){
			while (currentNode != NULL) {
				currentNode->wayToGo = true;
				currentNode = currentNode->parent; 
			}
			return;
		}
		
		Lister * neighbours = getNeighbours(grid, currentNode);

        Lister *neighbour=neighbours;    
        while(neighbour)
        {
            if(list_isIn(neighbour, closeSet) || neighbour->pNode->walkable == false) {neighbour = neighbour->next; continue;}

            int newCostToNeighbour = currentNode->gCost + getDistance(*currentNode, *(neighbour->pNode));
            if (newCostToNeighbour < neighbour->pNode->gCost || list_isIn(neighbour, openSet) == false){
                neighbour->pNode->gCost = newCostToNeighbour;
                neighbour->pNode->hCost = getDistance(*(neighbour->pNode), *targetNode);
                neighbour->pNode->fCost = neighbour->pNode->gCost + neighbour->pNode->hCost;
                neighbour->pNode->parent = currentNode;

                if(list_isIn(neighbour, openSet) == false) {push(&openSet, neighbour->pNode);}
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
