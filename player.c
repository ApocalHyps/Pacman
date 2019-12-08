 // add the needed C libraries below
#include <stdbool.h> // bool, true, false
#include <stdlib.h> // malloc, free
#include <math.h> // exp

#include <stdio.h>

// look at the file below for the definition of the direction type
// pacman.h must not be modified!
#include "pacman.h"

#define max(a,b) (a>=b?a:b)
#define min(a,b) (a<=b?a:b)

// ascii characters used for drawing levels
extern const char PACMAN; // ascii used for pacman
extern const char WALL; // ascii used for the walkable_map
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
const char * binome="BRUSAU";

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
  bool pacgomme;
  bool superpacgomme;
} Node;

typedef struct NodeList NodeList;
struct NodeList { 
  Node * pNode; 
  struct NodeList* next; 
  struct NodeList* prev; 
}; 

typedef struct fantome fantome;
struct fantome
{
  Coord pos;
  int id;
};

direction decision_maker(char ** map, Coord *pacman_pos, Coord grid_shape, bool energy, int remainingenergymoderounds, fantome fantomes[4]);
void refresh_ghost_pos(char ** map, Coord grid_shape, fantome fantomes[4]);

void push(NodeList** head_ref, Node * new_pNode);
void list_remove(struct NodeList **list, struct NodeList * del);
NodeList * getNeighbours(Node ** grid, Coord grid_shape, Node * pNode);
bool list_isIn(NodeList * pitem, NodeList * list);

int getDistance(Node nodeA, Node nodeB, Coord grid_shape);
int getDistanceFromClosestPacgomme(Node position, Node ** grid, Coord grid_shape);
int getDistanceFromClosestSuperpacgomme(Node position, Node ** grid, Coord grid_shape);

void findPath(Node ** grid, Coord grid_shape, Node *startNode, Node *targetNode);
void findPacmanPath(Node ** grid, Coord grid_shape, Node *startNode, bool ((*isTarget)(Node *)), int ((*distanceTo)(Node, Node **, Coord)));

bool isPacgomme(Node * pNode);
bool isSuperpacgomme(Node * pNode);

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

  direction d;
  Coord grid_shape = (Coord){xsize, ysize};

  //Création des instances des fantomes
  int k;
  Coord c = {-1, -1};
  fantome fantomes[4];
  for(k = 0; k < 4; k++){
    fantomes[k].pos = c;
    fantomes[k].id = k+1;
  }
  refresh_ghost_pos(map, grid_shape, fantomes);
  
  Coord pacman_pos = {x, y};

  d = decision_maker(map, &pacman_pos, grid_shape, energy, remainingenergymoderounds, fantomes);
  // answer to the game engine 
  return d;
}


void refresh_ghost_pos(char ** map, Coord grid_shape, fantome fantomes[4]){
  int i, j;
  char test;
  fantomes[0].pos = (Coord){-1, -1};
  fantomes[1].pos = (Coord){-1, -1};
  fantomes[2].pos = (Coord){-1, -1};
  fantomes[3].pos = (Coord){-1, -1};
  for(i = 0; i < grid_shape.y; i++){
    for(j = 0; j < grid_shape.x; j++){
      test = map[i][j];
      if(test == GHOST1){fantomes[0].pos = (Coord){j, i};}
      if(test == GHOST2){fantomes[1].pos = (Coord){j, i};}
      if(test == GHOST3){fantomes[2].pos = (Coord){j, i};}
      if(test == GHOST4){fantomes[3].pos = (Coord){j, i};}
    }
  }
}

bool isPacgomme(Node * pNode){return pNode->pacgomme;}
bool isSuperpacgomme(Node * pNode){return pNode->superpacgomme;}


direction decision_maker(char ** map, Coord *pacman_pos, Coord grid_shape, bool energy, int remainingenergymoderounds, fantome fantomes[4])/*Fonction d'évaluation du meilleur coup a faire*/{
  /*
  Actualiser la position des fantomes
  Convertir la carte en node (walkable or not)
  Pour les 4 fantomes :
    Chercher le chemin vers pacman
    Au passage des nodes vers pacman adapter le ghostCost en fonction de la profondeur, de manière exponentielle
  Pour pacman:
    Effectuer un pathfinding multitarget en prenant compte du coup modifier des case avec les fantomes
    Trouver donc le meilleur chemin vers un point en evitant les fantomes
    Ou si energy mode et qu'il reste queslques round chercher se diriger vers les fantomes
  Renvoyer donc la direction necessaire pour y arriver
  */
  int i;
  bool walkable_map[grid_shape.y][grid_shape.x]; // false if there is a wall

  //initialisation des murs
  int x, y;
  for(x = 0; x<grid_shape.x; x++){
    for(y = 0; y<grid_shape.y; y++){
      if(map[y][x] == WALL){
        //printf("wall pos : %i, %i\n", x, y);
        walkable_map[y][x] = false;
      }else{
        walkable_map[y][x] = true;
      }
    }
  }

  //Création du tableau des nodes
  Node ** grid = NULL;
  grid = malloc(grid_shape.y*sizeof(Node *));
  if(!grid){printf("Can't correctly allocate memory."); exit(0);}
  for (int i = 0; i < grid_shape.y; ++i)
  {
    grid[i] = NULL;
    grid[i] = malloc(grid_shape.x*sizeof(Node));
    if(!grid[i]){printf("Can't correctly allocate memory."); exit(0);}
  }

  bool stillpacgomme = false;
  bool stillsuperpacgomme = false;

  for(x = 0; x<grid_shape.x; x++){
    for(y = 0; y<grid_shape.y; y++){
      grid[y][x] = (Node){walkable_map[y][x], false, (Coord){x, y}, 0, 0, 0, 0, NULL, false, false};
      if(map[y][x] == VIRGIN_PATH) {
        grid[y][x].pacgomme = true;
        stillpacgomme = true;
      }else if(map[y][x] == ENERGY){
        grid[y][x].superpacgomme = true;
        stillsuperpacgomme = true;
      }
    }
  }

  Node * pacman_pNode = &(grid[pacman_pos->y][pacman_pos->x]);
  Node * currentGhost_pNode = NULL;
  Node * tmp = NULL;
  int count;
  for (i = 0; i < 4; ++i)
  {
    if(fantomes[i].pos.y == -1 && fantomes[i].pos.x == -1) continue; // Le fantome est mort pour ce tour
    currentGhost_pNode = &(grid[fantomes[i].pos.y][fantomes[i].pos.x]);
    // find path to pacman
    findPath(grid, grid_shape, currentGhost_pNode, pacman_pNode);

    // Décommenter la suite pour faire apparaitre en debug les chemin calculé par les fantomes

    //   for(y = 0; y < grid_shape.y; y++){
    //   for(x = 0; x < grid_shape.x; x++){
    //     if(grid[y][x].pos.x == currentGhost_pNode->pos.x && grid[y][x].pos.y == currentGhost_pNode->pos.y){
    //       printf("è");
    //     }else if(grid[y][x].pos.x == pacman_pNode->pos.x && grid[y][x].pos.y == pacman_pNode->pos.y){
    //       printf("@");  
    //     }else if(grid[y][x].pacgomme){
    //       printf(":");
    //     }else if(grid[y][x].wayToGo){
    //       printf("°");        
    //     }else if(grid[y][x].walkable){
    //       printf(".");  
    //     }else{
    //       printf("#");
    //     } 
    //   }
    //   printf("\n");   
    // }

    // Updating ghostCost des nodes sur le chemin
    //first getting the lenght of the path
    count = 0;
    tmp = pacman_pNode;
    while(tmp){
      count++;
      tmp = tmp->parent;
    }
    // updating ghostCost
    tmp = pacman_pNode;
    while(tmp){
      //tmp->ghostCost += floor(10000*exp(count-lenght-3)); // coefficient a ajuster
      if(energy && remainingenergymoderounds > 10){
        tmp->ghostCost -= floor(1000/pow(count, 2));
      }else {
        tmp->ghostCost += floor(1000/pow(count, 2));
      }
      tmp = tmp->parent;
      count--;
    }
    


    //reset grid for next pathfind
    for(x = 0; x<grid_shape.x; x++){
      for(y = 0; y<grid_shape.y; y++){
        grid[y][x].fCost = 0;
        grid[y][x].hCost = 0;
        grid[y][x].gCost = 0;
        grid[y][x].wayToGo = false;
        grid[y][x].parent = NULL;
      }
    } 
  }

  for(x = 0; x<grid_shape.x; x++){
    for(y = 0; y<grid_shape.y; y++){
      if(map[y][x] == DOOR) grid[y][x].walkable = false; // pour pacman maintenant il ne peut pas traverser la DOOR
    }
  } 

  Coord nextpos;
  bool ((*isTarget)(Node *));
  int ((*distanceTo)(Node, Node**, Coord));
  if(stillpacgomme){
    isTarget = isPacgomme;
    distanceTo = getDistanceFromClosestPacgomme;
  }else if(stillsuperpacgomme){
    isTarget = isSuperpacgomme;
    distanceTo = getDistanceFromClosestSuperpacgomme;
  }

  NodeList *neighbours = getNeighbours(grid, grid_shape, pacman_pNode);
  NodeList *temp2 = neighbours;

  if((stillpacgomme || stillsuperpacgomme) && ((!energy) || (energy && remainingenergymoderounds < 10))){
    findPacmanPath(grid, grid_shape, pacman_pNode, isTarget, distanceTo);
    //printf("323\n");
    
    while(temp2){ 
      if(temp2->pNode->wayToGo){
        nextpos = temp2->pNode->pos;
        break;
      }
      temp2 = temp2->next;
    }
  }else{
    // on regarde quelle node autour de pacman a le plus petit ghostCost pour finir ou si une pacgomme est caché pour temporiser
    int score = 0;
    while(temp2){
      score = max(score, temp2->pNode->ghostCost);
      temp2 = temp2->next;
    }
    //we now got score at least the minimal around
    while(neighbours){
      if(neighbours->pNode->ghostCost <= score && neighbours->pNode->walkable){
        nextpos = neighbours->pNode->pos;
        score = neighbours->pNode->ghostCost;
      }
      neighbours = neighbours->next;
    }
  }

  // Décommenter la suite pour faire apparaitre en debug les chemin calculé pour pacman

  // for(y = 0; y < grid_shape.y; y++){
  //     for(x = 0; x < grid_shape.x; x++){
  //       if(grid[y][x].pos.x == pacman_pNode->pos.x && grid[y][x].pos.y == pacman_pNode->pos.y){
  //         printf("@");  
  //       }else if(grid[y][x].wayToGo){
  //         printf("°");        
  //       }else if(grid[y][x].pacgomme){
  //         printf(":");
  //       }else if(grid[y][x].walkable){
  //         printf(".");  
  //       }else{
  //         printf("#");
  //       } 
  //     }
  //     printf("\n"); 
  //   }
  
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

  if(nextpos.x == pacman_pos->x - 1) d = WEST;//North
  if(nextpos.x == pacman_pos->x + 1) d = EAST;//South
  if(nextpos.y == pacman_pos->y - 1) d = NORTH;//West
  if(nextpos.y == pacman_pos->y + 1) d = SOUTH;//East

  if (pacman_pos->x == 0 && nextpos.x == grid_shape.x-1) return WEST;
  if (pacman_pos->x == grid_shape.x-1 && nextpos.x == 0) return EAST;
  if (pacman_pos->y == 0 && nextpos.y == grid_shape.y-1) return NORTH;
  if (pacman_pos->y == grid_shape.y-1 && nextpos.y == 0) return SOUTH;
  return d;
}

//next code is for the pathFinding algorithm

// Gestion des listes

void push(NodeList** head_ref, Node* new_pNode) 
{ 
  /* allocate node */
  NodeList* new_element = (NodeList*)malloc(sizeof(NodeList)); 

  /* put in the pNode  */
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

void list_remove(NodeList **head_ref, NodeList * del)
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

bool list_isIn(NodeList * pitem, NodeList * list)
{
  NodeList *tmp=list; /* opération en vert sur le diagramme */
  while (tmp)
  {
      if(tmp->pNode == pitem->pNode) return true;
      tmp = tmp->next; /* opération en bleu sur le diagramme */
  }
  return false;
}

//recherche des case voisines (exclus les case hors de la carte, pas très necessaire pour pacman grace aux murs exterieurs mais bon ...)

NodeList * getNeighbours(Node ** grid, Coord grid_shape, Node * pNode)
{
  int row = grid_shape.y;
  int column = grid_shape.x;
  int x, y;
  NodeList * list = NULL;
  for(x = -1; x <= 1; x++){
    for(y = -1; y <= 1; y++){
      if(x == y || x == -y)
        continue;

      //le voisin est-il sur la carte (Rappel pour pacman: a modifier car la topologie est torique)
      int checkX = pNode->pos.x + x;
      int checkY = pNode->pos.y + y;
      if(checkX < 0) checkX += column;
      if(checkX > column-1) checkX -= column;
      if(checkY < 0) checkY += row;
      if(checkY > row-1) checkY -= row;
      push(&list, &(grid[checkY][checkX]));
    }   
  }
  return list;
}

 // Distance  pour exspace a topologie torique
int getDistance(Node nodeA, Node nodeB, Coord grid_shape)
{
  int row = grid_shape.y;
  int column = grid_shape.x;
  int dstX = abs(nodeB.pos.x-nodeA.pos.x);
  int dstY = abs(nodeB.pos.y-nodeA.pos.y);
  int dist = min(min(dstX+dstY, row-dstX + dstY), min(dstX + column-dstY, row+column - dstX-dstY));
  return dist;
}

int getDistanceFromClosestSuperpacgomme(Node position, Node ** grid, Coord grid_shape){/* Problème lors de la fin de partie si un fantome est sur une des dernière pacgomme : ajouter de la temporisation*/
  int x, y;
  int row = grid_shape.y;
  int column = grid_shape.x;
  int distance = row*column;
  int newdistance;
  for(x = 0; x < column; x++){
    for(y = 0; y < row; y++){
      if(grid[y][x].superpacgomme == false) continue;
      newdistance = getDistance(grid[y][x], position, grid_shape); // Altération du cout si un fantomes ferai bien de pacer par là
      if(newdistance <= distance) distance = newdistance;
    }
  }
  return distance;
}


int getDistanceFromClosestPacgomme(Node position, Node ** grid, Coord grid_shape){/* Problème lors de la fin de partie si un fantome est sur une des dernière pacgomme : ajouter de la temporisation*/
  int x, y;
  int row = grid_shape.y;
  int column = grid_shape.x;
  int distance = row*column;
  int newdistance;
  for(x = 0; x < column; x++){
    for(y = 0; y < row; y++){
      if(grid[y][x].pacgomme == false) continue;
      newdistance = getDistance(grid[y][x], position, grid_shape); // Altération du cout si un fantomes ferai bien de pacer par là
      if(newdistance <= distance) distance = newdistance;
    }
  }
  return distance;
}


// findpath

void findPath(Node ** grid, Coord grid_shape, Node *startNode, Node *targetNode){
  //Création openSet
  NodeList *openSet = NULL;
  
  NodeList *closeSet = NULL;

  push(&openSet, startNode);

  Node *currentNode = NULL;

  while(openSet){
    currentNode = openSet->pNode;

    NodeList * candidat = openSet;

    NodeList *tmp = openSet->next; // part de 1 jusqu'au dernier element (où next = NULL)
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
      free(openSet);
      free(closeSet);

      while(currentNode){
        currentNode->wayToGo = true;
        currentNode = currentNode->parent;
      }
      return;
    }
    
    NodeList * neighbours = getNeighbours(grid, grid_shape, currentNode);

    NodeList *neighbour=neighbours;    
    while(neighbour)
    {
      if(list_isIn(neighbour, closeSet) || neighbour->pNode->walkable == false) {neighbour = neighbour->next; continue;}

      int newCostToNeighbour = currentNode->gCost + 1;
      if (newCostToNeighbour < neighbour->pNode->gCost || list_isIn(neighbour, openSet) == false){
        neighbour->pNode->gCost = newCostToNeighbour;
        neighbour->pNode->hCost = getDistance(*(neighbour->pNode), *targetNode, grid_shape);
        neighbour->pNode->fCost = neighbour->pNode->gCost + neighbour->pNode->hCost;
        neighbour->pNode->parent = currentNode;

        if(list_isIn(neighbour, openSet) == false) {push(&openSet, neighbour->pNode);}
      }
      neighbour = neighbour->next;
    }
    free(neighbours);
  }
  printf("Didn't find a path\n");
  free(openSet);
  free(closeSet);

}

void findPacmanPath(Node ** grid, Coord grid_shape, Node *startNode, bool ((*isTarget)(Node *)), int ((*distanceTo)(Node, Node** , Coord))){
  //Création openSet
  NodeList *openSet = NULL;
  
  NodeList *closeSet = NULL;

  push(&openSet, startNode);

  Node *currentNode = NULL;

  while(openSet){
    currentNode = openSet->pNode;

    NodeList * candidat = openSet;

    NodeList *tmp = openSet->next; // part de 1 jusqu'au dernier element (où next = NULL)
    while (tmp){
      if (tmp->pNode->fCost < currentNode->fCost ||(tmp->pNode->fCost == currentNode->fCost && tmp->pNode->hCost < currentNode->hCost)){
        currentNode = tmp->pNode;
        candidat = tmp;
      }
      tmp = tmp->next;
    }

    list_remove(&openSet, candidat);

    push(&closeSet, currentNode);
    
    if((*isTarget)(currentNode)){
      free(openSet);
      free(closeSet);
      while(currentNode){
        currentNode->wayToGo = true;
        currentNode = currentNode->parent;
      }
      return;
    }
    
    NodeList * neighbours = getNeighbours(grid, grid_shape, currentNode);

    NodeList *neighbour=neighbours;    
    while(neighbour){
      if(list_isIn(neighbour, closeSet) || neighbour->pNode->walkable == false) {neighbour = neighbour->next; continue;}

      int newCostToNeighbour = currentNode->gCost + 1;
      if (newCostToNeighbour < neighbour->pNode->gCost || list_isIn(neighbour, openSet) == false){
        neighbour->pNode->gCost = newCostToNeighbour;
        neighbour->pNode->hCost = (*distanceTo)(*(neighbour->pNode), grid, grid_shape);
        neighbour->pNode->fCost = neighbour->pNode->gCost + neighbour->pNode->hCost + neighbour->pNode->ghostCost;
        neighbour->pNode->parent = currentNode;

        if(list_isIn(neighbour, openSet) == false) {push(&openSet, neighbour->pNode);}
      }
      neighbour = neighbour->next;
    }
    free(neighbours);
  }
  printf("Didn't find a path\n");
  free(openSet);
  free(closeSet);

}
