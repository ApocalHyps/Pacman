 // add the needed C libraries below
#include <stdbool.h> // bool, true, false
#include <stdlib.h> // malloc, free
#include <math.h> // exp

#include <stdio.h>

// look at the file below for the definition of the direction type
// pacman.h must not be modified!
#include "pacman.h"

#define max(a,b) (a>=b?a:b)

// ascii characters used for drawing levels
extern const char PACMAN; // ascii used for pacman
extern const char WALL; // ascii used for the walls
extern const char PATH; // ascii used for the explored paths
extern const char DOOR; // ascii used for the ghosts' door
extern const char VIRGIN_PATH; // ascii used for the unexplored paths
extern const char ENERGY; // ascii used for the energizers
extern const char GHOST1; // ascii used for the ghost 1
extern const char GHOST2; // ascii used for the ghost 2
extern const char GHOST3; // ascii used for the ghost 3
extern const char GHOST4; // ascii used for the ghost 4

// reward (in points) when eating dots/energizers 
extern const int VIRGIN_PATH_SCORE; // reward for eating a dot
extern const int ENERGY_SCORE; // reward for eating an energizer

// put the student names below (mandatory)
const char * binome="Random";

// put the prototypes of your additional functions/procedures below
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
  int ghostCost;
  struct Node *parent;
} Node;

typedef struct Lister Lister;
struct Lister { 
  Node * data; 
  struct Lister* next; 
  struct Lister* prev; 
}; 

typedef struct fantome fantome;
struct fantome
{
  Coord pos;
  int id;
};

int row = 23;
int column = 27;

direction decision_maker(char ** map, Coord *pacman_pos, Coord *grid_shape, bool energy, int remainingenergymoderounds, fantome fantomes[4]);
void refresh_ghost_pos(char ** map, Coord *grid_shape, fantome fantomes[4]);

void push(Lister** head_ref, Node * new_data);
void list_remove(struct Lister **list, struct Lister * del);
Lister * getNeighbours(Node grid[row][column], Node * data);
bool list_isIn(Lister * pitem, Lister * list);
int getDistance(Node nodeA, Node nodeB);
void findPath(Node grid[row][column], Node *startNode, Node *targetNode);

// change the pacman function below to build your own player
// your new pacman function can use as many additional functions/procedures as needed; put the code of these functions/procedures *AFTER* the pacman function
direction pacman(
		 char * * map, // the map as a dynamic array of strings, ie of arrays of chars
		 int xsize, // number of columns of the map
		 int ysize, // number of lines of the map
		 int x, // x-position of pacman in the map 
		 int y, // y-position of pacman in the map
		 direction lastdirection, // last move made by pacman (see pacman.h for the direction type; lastdirection value is -1 at the beginning of the game
		 bool energy, // is pacman in energy mode? 
		 int remainingenergymoderounds // number of remaining rounds in energy mode, if energy mode is true
		 ) {
  // row = ysize;
  // column = xsize;
  // printf("%i, %i \n", row, column);
  direction d;
  Coord grid_shape = (Coord){xsize, ysize};
  // guess a direction among the allowed four, until a valid choice is made

  //Création des instances des fantomes
  int k;
  Coord c = {-1, -1};
  fantome fantomes[4];
  for(k = 0; k < 4; k++){
    fantomes[k].pos = c;
    fantomes[k].id = k+1;
    
    //printf("fantome %i, pos = %i, %i, forme : %c\n", fantomes[k].id, fantomes[k].pos.x, fantomes[k].pos.y, fantomes[k].forme);
  }
  refresh_ghost_pos(map, &grid_shape, fantomes);
  
  Coord pacman_pos = {x, y};

  d = decision_maker(map, &pacman_pos, &grid_shape, energy, remainingenergymoderounds, fantomes);
  
  printf("d = %i\n", d);
  // answer to the game engine 
  return d;
}


void refresh_ghost_pos(char ** map, Coord *grid_shape, fantome fantomes[4]){
  int i, j;
  char test;
  fantomes[0].pos = (Coord){-1, -1};
  fantomes[1].pos = (Coord){-1, -1};
  fantomes[2].pos = (Coord){-1, -1};
  fantomes[3].pos = (Coord){-1, -1};
  for(i = 0; i < grid_shape->y; i++){
    for(j = 0; j < grid_shape->x; j++){
      test = map[i][j];
      //printf("%c\n", map[i][j+1]);
      if(test == GHOST1){fantomes[0].pos = (Coord){j, i};}
      if(test == GHOST2){fantomes[1].pos = (Coord){j, i};}
      if(test == GHOST3){fantomes[2].pos = (Coord){j, i};}
      if(test == GHOST4){fantomes[3].pos = (Coord){j, i};}
    }
  }
}


direction decision_maker(char ** map,Coord *pacman_pos, Coord *grid_shape, bool energy, int remainingenergymoderounds, fantome fantomes[4])/*Fonction d'évaluation du meilleur coup a faire*/{
  /*
  actualiser la position des fantomes - check
  convertir la carte en node (walkable or not) - check
  Pour les 4 fantomes :
    Chercher le chemin vers pacman - check
    au passage des nodes vers pacman adapter le ghostCost en fonction de la profondeur, de manière exponentielle (genre a trois case c'est ok. mais a 1 case c'est pas acceptable) - check
  Pour pacman:
    Effectuer un pathfinding multitarget en prenant compte du coup modifier des case avec les fantomes
    trouver donc le meilleur chemin vers un point en evitant les fantomes
  renvoyer donc la première direction necessaire pour y arriver
  recommencer a chaque étape pour actualiser le chemin a prendre en fction des déplacement des fantomes.

  */
  int i;

  refresh_ghost_pos(map, grid_shape, fantomes);

  bool walls[row][column]; // false if there is a wall

  //initialisation des murs
  int x, y;
  for(x = 0; x<column; x++){
    for(y = 0; y<row; y++){
      if(map[y][x] == WALL){
        //printf("wall pos : %i, %i\n", x, y);
        walls[y][x] = false;
      }else{
        walls[y][x] = true;
      }
    }
  }

  //Création du tableau des nodes
  Node grid[row][column];
  //int dstXsup = 2*abs(goal.x-start.x);
  //int dstYsup = 2*abs(goal.y-start.y);
  for(x = 0; x<column; x++){
    for(y = 0; y<row; y++){
      grid[y][x] = (Node){walls[y][x], false, (Coord){x, y}, 0, 0, 0, 0, NULL};
    }
  }

  Node * pacman_data = &(grid[pacman_pos->y][pacman_pos->x]);
  Node * currentGhost_data = NULL;
  Node * tmp = NULL;
  int count;
  int lenght;
  for (i = 0; i < 4; ++i)
  {
    currentGhost_data = &(grid[fantomes[i].pos.y][fantomes[i].pos.x]);
    // find path to pacman
    findPath(grid, currentGhost_data, pacman_data);


    //checking if it is found
    //test a supprimer

    for(y = 0; y<row; y++){
    for(x = 0; x<column; x++){
      if(grid[y][x].pos.x == currentGhost_data->pos.x && grid[y][x].pos.y == currentGhost_data->pos.y){
        printf("è");
      }else if(grid[y][x].pos.x == pacman_data->pos.x && grid[y][x].pos.y == pacman_data->pos.y){
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

    if(pacman_data->parent == NULL){

      for(x = 0; x<column; x++){
      for(y = 0; y<row; y++){
        grid[y][x].fCost = 0;
        grid[y][x].hCost = 0;
        grid[y][x].gCost = 0;
        grid[y][x].wayToGo = false;
        grid[y][x].parent = NULL;
      }
    } 
      continue;
    }
      printf("Hey\n");

    // Updating ghostCost des nodes sur le chemin
    //first getting the lenght of the path
    count = 0;
    tmp = pacman_data;
    while(tmp){
      count++;
      tmp = tmp->parent;
    }
    lenght = count;
    // updating ghostCost
    tmp = pacman_data;
    while(tmp){
      //tmp->ghostCost += floor(10000*exp(count-lenght-3)); // coefficient a ajuster
      if(energy){
        tmp->ghostCost -= floor(10000/pow(lenght, 3)) * remainingenergymoderounds;
      }else {
        tmp->ghostCost += floor(10000/pow(lenght, 3));
      }
      tmp = tmp->parent;
      count--;
    }
    


    //reset grid for next pathfind
    for(x = 0; x<column; x++){
      for(y = 0; y<row; y++){
        grid[y][x].fCost = 0;
        grid[y][x].hCost = 0;
        grid[y][x].gCost = 0;
        grid[y][x].wayToGo = false;
        grid[y][x].parent = NULL;
      }
    } 
  }
  
  // pour le moment pour test on va simplement faire en sorte que pacman échappe aux fantomes donc pas encore de path finding
  // on regarde quelle node autour de pacman a le plus petit ghostCost
  Coord nextpos;
  Lister *neighbours = getNeighbours(grid, pacman_data);
  Lister *temp2 = neighbours;
  // while(temp2){
  //   printf("%p : x = %i, y = %i\n", temp2->data, temp2->data->pos.x, temp2->data->pos.y);
  //   temp2 = temp2->next;
  // }
  int score = 0;
  while(temp2){
    score = max(score, temp2->data->ghostCost);
    temp2 = temp2->next;
    printf("score : %i\n", score);
  }
  printf("final score : %i\n", score);
  //we now got score at least the minimal around
  while(neighbours){
    printf("ghost cost of neighbour : %i \n", neighbours->data->ghostCost);
    printf("%p : x = %i, y = %i, ghostcost = %i \n", neighbours->data, neighbours->data->pos.x, neighbours->data->pos.y, neighbours->data->ghostCost);
    if(neighbours->data->ghostCost <= score && neighbours->data->walkable){
      nextpos = neighbours->data->pos;
      printf("new selection : iswalkable : %s\n", neighbours->data->walkable?"yes":"no");
      score = neighbours->data->ghostCost;
    }
    neighbours = neighbours->next;
  }
  printf("score : %i\n", score);
  //now looking for the direction to head for based on the nextpos
  /*
  #-------------------> x
  |
  |
  |   Modèle utilisé
  |
  |
  |
  |
  \/
  y

  */
  direction d;
  printf("pacman pos : %i, %i\n", pacman_pos->x, pacman_pos->y);
  printf("next pos evaluated : %i, %i\n", nextpos.x, nextpos.y);

  if(nextpos.x == pacman_pos->x - 1) d = WEST;//North
  if(nextpos.x == pacman_pos->x + 1) d = EAST;//South
  if(nextpos.y == pacman_pos->y - 1) d = NORTH;//West
  if(nextpos.y == pacman_pos->y + 1) d = SOUTH;//East

  printf(" final d = %i\n", d);

  return d; //0,1,2,3
}

//next code is for the pathFinding algorithm

// Gestion des listes

void push(Lister** head_ref, Node* new_data) 
{ 
    /* allocate node */
    Lister* new_element = (Lister*)malloc(sizeof(Lister)); 
  
    /* put in the data  */
    new_element->data = new_data; 
  
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

//lecture sur les listes

bool list_isIn(Lister * pitem, Lister * list)
{
  Lister *tmp=list; /* opération en vert sur le diagramme */
  while (tmp)
  {
      if(tmp->data == pitem->data) return true;
      tmp = tmp->next; /* opération en bleu sur le diagramme */
  }
  return false;
}

//recherche des case voisines (exclus les case hors de la carte, pas très necessaire pour pacman grace aux murs exterieurs mais bon ...)

Lister * getNeighbours(Node grid[row][column], Node * data)
{
  int x, y;
  Lister * list = NULL;
  for(x = -1; x <= 1; x++){
    for(y = -1; y <= 1; y++){
      if(x == y || x == -y)
        continue;

      //le voisin est-il sur la carte (Rappel pour pacman: a modifier car la topologie est cylindrique)
      //Considérons ici un plan bornée
      int checkX = data->pos.x + x;
      int checkY = data->pos.y + y;
      if (checkX >= 0 && checkX < column && checkY >= 0 && checkY < row){
        push(&list, &(grid[checkY][checkX]));
      }
    }   
  }
  return list;
}

//fonction qui determine le coup absolu d'un déplacement

int getDistance(Node nodeA, Node nodeB)
{/* A modifier pour pacman avec sa topologie*/
  int dstX = abs(nodeB.pos.x-nodeA.pos.x);
  int dstY = abs(nodeB.pos.y-nodeA.pos.y);
  // int dist;
  int dist = dstX+dstY; // Si il n'y a pas de déplacement diagonal
  // if (dstX>dstY) dist = 14*dstY + 10*(dstX-dstY);
  // if (dstX<=dstY) dist = 14*dstX + 10*(dstY-dstX);
  return dist;
}

// findpath

void findPath(Node grid[row][column], Node *startNode, Node *targetNode){
  //Création openSet
  Lister *openSet = NULL;
  
  Lister *closeSet = NULL;

  push(&openSet, startNode);

  Node *currentNode = NULL;

  while(openSet){
    currentNode = openSet->data;

    Lister * candidat = openSet;

    Lister *tmp = openSet->next; // part de 1 jusqu'au dernier element (où next = NULL)
    while (tmp)
    {
        if (tmp->data->fCost < currentNode->fCost ||(tmp->data->fCost == currentNode->fCost && tmp->data->hCost < currentNode->hCost)){
        currentNode = tmp->data;
        candidat = tmp;
      }
        tmp = tmp->next;
    }
    
    list_remove(&openSet, candidat);

    push(&closeSet, currentNode);
    
    if(currentNode == targetNode){
      free(openSet);
      free(closeSet);
      //temporaire
      while(currentNode){
        currentNode->wayToGo = true;
        currentNode = currentNode->parent;
      }
      return; // retrace to do outside
    }
    
    Lister * neighbours = getNeighbours(grid, currentNode);

        Lister *neighbour=neighbours;    
        while(neighbour)
        {
            if(list_isIn(neighbour, closeSet) || neighbour->data->walkable == false) {neighbour = neighbour->next; continue;}

            int newCostToNeighbour = currentNode->gCost + getDistance(*currentNode, *(neighbour->data));
            if (newCostToNeighbour < neighbour->data->gCost || list_isIn(neighbour, openSet) == false){
                neighbour->data->gCost = newCostToNeighbour;
                neighbour->data->hCost = getDistance(*(neighbour->data), *targetNode);
                neighbour->data->fCost = neighbour->data->gCost + neighbour->data->hCost;
                neighbour->data->parent = currentNode;

                if(list_isIn(neighbour, openSet) == false) {push(&openSet, neighbour->data);}
            }
            neighbour = neighbour->next;
        }
        free(neighbours);
  }
  printf("Didn't find a path\n");
  free(openSet);
  free(closeSet);

}
