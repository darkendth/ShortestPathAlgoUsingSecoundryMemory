#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>

#define MAXSIZEARRAY 65000

/*   Define Graph.   */
struct graph_node
{
    int vertex;
    double weight;
    struct graph_node *next;
};
typedef struct graph_node Graph_Node;

struct graph
{
    int vertex;
    Graph_Node *ptr;
};
typedef struct graph Graph;
Graph gp[MAXSIZEARRAY];

/*  Define Min Heap.  */
struct  heap_node
{
    int vertex;
    double d;
    // int pi;
};
typedef struct heap_node HeapNode;

struct heap
{
    HeapNode *arPtr;
    int heap_size;
};
typedef struct heap Heap;
Heap minheap;

/* Meta data of file. */
struct Meta_data 
{
    char name[50];
    int num_of_record; //Total number of record present in a cell;
    int num_overflow;  //number of overflow cell linket to main cell;
    char last_of_cell[20]; //Name of last overflow cell;
};
typedef struct Meta_data CellMetaData;

/*   Define List(SET).   */
struct set_node
{
    int cv;     // current vertex.
    int pi;     // predecessor vertex.
    double dw;   // shortest distance to cv.
    struct set_node *next;
};
typedef struct set_node SetNode;
SetNode *openList,*closedList,*lastptr;

/* list of cell that are brought into main memory */
struct listNode
{
    int id;
    struct listNode *next;
};
typedef struct listNode ListNode;
ListNode *cellList;

/*  Global Variable Declaration.    */
int source_vertex,destination_vertex;
int nodesInGraph;
int pathLength = 0;


/*  Function Declaration    */
int getCellIdFromNodeId(int v);
char* getCellName(int cell);
char* getNameOfOverflowCell(int p,char* ck);
void addEdgeInGraph(int ,int ,double );
int isNodeInGraph(int);
void loadCellInBuffer(int );
void showGraph();

void updateCellList(int );
int numberOfCellInBuffer();
int isCellInMemory(int );
void showCellList();

/* min Heap operation function */
void heapDecreaseKey(int );
void MinHeapify(int );
void insertInMinHeap(int ,double );
HeapNode ExtractMin();
void displayMinHeap();
void destroyMinHeap();

/* Set of node in open list and closed list. */
void addInOpenList(int ,double ,int );
SetNode* getListNodeFromOpenList(int );
int isNodeInOpenList(int );
void updateOpenList(int ,double ,int);
void addInClosedList(int );
int isNodeInClosedList(int );
void showList(SetNode *);

void Relax(HeapNode ,int ,double );
int isDestinationClosed();
void printShortestPath(SetNode *,int );

/* Deallocation */
void destroyGraph();
void destroySetList(SetNode *);
void destroyList();

void main()
{
    /* Local Variable Declaration */
    FILE *fp;
    int cellId,count=0;
    HeapNode u;
    Graph_Node *v;
    char *cellName;
    /* -------------------------- */

    printf("Enter source and destination vertex : ");
    scanf("%d%d",&source_vertex,&destination_vertex);

    cellId = getCellIdFromNodeId(source_vertex);
    printf("cell id : %d\n",cellId);
    if (cellId == 0)
    {
        printf("unable to find source vertex in partitioned graph.\n");
        exit(1);
    }
    loadCellInBuffer(cellId);

    /* min heap initialization */
    minheap.arPtr = (HeapNode *)malloc(sizeof(HeapNode)*MAXSIZEARRAY);
    if (minheap.arPtr == NULL)
    {
        printf("Error: %d (%s)\n", errno, strerror(errno));
        exit(2);
    }
    minheap.heap_size = 0;

    /* dijkstra's algo */
    insertInMinHeap(source_vertex,0);
    addInOpenList(source_vertex,0,-1);
    while ((minheap.heap_size!=0)&&(!isDestinationClosed()))
    {
        u = ExtractMin();
        printf("minimum %d %.2f\n",u.vertex,u.d);
        addInClosedList(u.vertex);
        for (int i=0;i<nodesInGraph;i++)
        {
            if (gp[i].vertex == u.vertex)
            {
                v = gp[i].ptr;
                break;
            }
        }
        while (v != NULL)
        {
            printf("v : %d\n",v->vertex);
            cellId = getCellIdFromNodeId(v->vertex);
            if (cellId != 0)
            {
                printf("cellid : %d\n",cellId);
                if (!isCellInMemory(cellId))
                {
                    printf("Not in buffer\n");
                    loadCellInBuffer(cellId);
                }
                if ((!isNodeInOpenList(v->vertex))&&(!isNodeInClosedList(v->vertex)))
                {
                    addInOpenList(v->vertex,(u.d+v->weight),u.vertex);
                    insertInMinHeap(v->vertex,(u.d+v->weight));
                }
                else if (isNodeInOpenList(v->vertex))
                    Relax(u,v->vertex,v->weight);
            }
            v = v->next;
        }
        printf("open List : ");
        showList(openList);
    }
    // showList(closedList);
    printf("Shortest Path: \n");
    printShortestPath(closedList,lastptr->cv);
    printf("\nPath Length : %d\n",pathLength);
    count = numberOfCellInBuffer();
    printf("Number of cells in memory : %d\n",count);
    showCellList();
    
    /* Deallocate memory */
    destroyGraph();
    destroyMinHeap();
    destroySetList(openList);
    destroySetList(closedList);
    destroyList();
    
}

int getCellIdFromNodeId(int v)
{
    int node,cell;
    FILE *fp;
    fp = fopen("nodeidToCellid.txt","r");   
    if (fp == NULL)
    {
        printf("unable to map source node to cell.\n");
        printf("Error: %d (%s)\n", errno, strerror(errno));
        exit(1);
    }
    while (fscanf(fp,"%d %d",&node,&cell)!=EOF)
    {
        if (node == v)
        {
            fclose(fp);
            return cell;
        }
    }
    return 0;
}

char* getCellName(int cell)
{
    char *name = (char *)malloc(sizeof(char)*10);
    sprintf(name,"%d",cell);
    strcat(name,".txt");
    return name;
}

char* getNameOfOverflowCell(int p,char* ck)
{
    char *bk = (char *)malloc(sizeof(char)*20);
    sprintf(bk,"%d",p);
    strcat(bk,"_OV_");
    strcat(bk,ck);
    return bk;
}

void addEdgeInGraph(int u,int v,double w)
{
    Graph_Node *p,*temp;
    if(temp==NULL)
    {
        printf("memory overflow!");
        exit(4);
    }
    for (int i=0;i<nodesInGraph;i++)
    {
        if (gp[i].vertex == u)
        {
            p = gp[i].ptr;
            while (p!=NULL)
            {
                if (p->vertex == v)
                    return ;
                p = p->next;
            }
            temp = (Graph_Node *)malloc(sizeof(Graph_Node));
            temp->vertex = v;
            temp->weight = w;
            temp->next = gp[i].ptr;
            gp[i].ptr = temp;
            break;
        }
    }
}

int isNodeInGraph(int n)
{
    int node,cell;
    FILE *fp;
    fp = fopen("nodeidToCellid.txt","r");   
    if (fp == NULL)
    {
        printf("unable to map source node to cell.\n");
        printf("Error: %d (%s)\n", errno, strerror(errno));
        exit(1);
    }
    while (fscanf(fp,"%d %d",&node,&cell)!=EOF)
    {
        if (node == n)
        {
            fclose(fp);
            return 1;
        }
    }
    return 0;
}

void loadCellInBuffer(int cellId)
{
    /* store cell in main memory by creating graph. */
    CellMetaData md,mdo;
    char buffer[100];
    int nodeId,flag=0,flagb=0;
    int overflowNumber=0;
    int u,v;
    double w;
    char *overflowCellName,*cellName;
    cellName = getCellName(cellId);
    FILE *cell = fopen(cellName,"r");
    if (cell == NULL)
    {
        printf("%s file not found(Unable to load in buffer.)!",cellName);
        printf("Error: %d (%s)\n", errno, strerror(errno));
        exit(2);
    }
    fread(&md,sizeof(md),1,cell);
    while (fgets(buffer,sizeof(buffer),cell)!=NULL)
    {
        if(strcmp(buffer,"###\n")==0)
        {
            flag = 1;
            break;
        }
        if(strcmp(buffer,"\n")!=0)
        {
            sscanf(buffer,"%d",&nodeId);
            if (isNodeInGraph(nodeId))
            {
                gp[nodesInGraph].vertex = nodeId;
                nodesInGraph++;
            }
        }
    }
    /* if flag is 0 means overflow file is there. */
    if (flag != 1)
    {
        fclose(cell);
        flag = 0;
        for (int p=1;p<=md.num_overflow;p++)
        {
            overflowCellName = getNameOfOverflowCell(p,cellName);
            cell = fopen(overflowCellName,"r");
            if (cell != NULL)
            {
                fread(&mdo,sizeof(mdo),1,cell);
                while (fgets(buffer,sizeof(buffer),cell)!=NULL)
                {
                    if(strcmp(buffer,"###\n")==0)
                    {
                        flag = 1;
                        overflowNumber = p;
                        break;
                    }
                    if(strcmp(buffer,"\n")!=0)
                    {
                        sscanf(buffer,"%d",&nodeId);
                        if (isNodeInGraph(nodeId))
                        {
                            gp[nodesInGraph].vertex = nodeId;
                            nodesInGraph++;
                        }
                    }
                }
                free(overflowCellName);
                if (flag != 0)
                    break;
                fclose(cell);
            }
        }
    }
    flag=0;
    while (fgets(buffer,sizeof(buffer),cell)!=NULL)
    {
        if(strcmp(buffer,"***\n")==0)
        {
            flag = 1;
            break;
        }
        if(strcmp(buffer,"\n")!=0)
        {
            sscanf(buffer,"%d %d %lf",&u,&v,&w);
            addEdgeInGraph(u,v,w);
        }
    }
    if (flag != 1)
    {
        fclose(cell);
        flag = 0;
        for (int p=overflowNumber+1;p<=md.num_overflow;p++)
        {
            overflowCellName = getNameOfOverflowCell(p,cellName);
            cell = fopen(overflowCellName,"r");
            if (cell != NULL)
            {
                fread(&mdo,sizeof(mdo),1,cell);
                while (fgets(buffer,sizeof(buffer),cell)!=NULL)
                {
                    if(strcmp(buffer,"***\n")==0)
                    {
                        flag = 1;
                        overflowNumber = p;
                        break;
                    }
                    if(strcmp(buffer,"\n")!=0)
                    {
                        sscanf(buffer,"%d %d %lf",&u,&v,&w);
                        addEdgeInGraph(u,v,w);
                    }
                }
                free(overflowCellName);
                if (flag != 0)
                    break;
                fclose(cell);
            }
        }
    }
    flag = 0;
    while (fgets(buffer,sizeof(buffer),cell)!=NULL)
    {
        if(strcmp(buffer,"???\n")==0)
        {
            flag = 1;
            break;
        }
    }
    if (flag != 1)
    {
        fclose(cell);
        flag = 0;
        for (int p=overflowNumber+1;p<=md.num_overflow;p++)
        {
            overflowCellName = getNameOfOverflowCell(p,cellName);
            cell = fopen(overflowCellName,"r");
            if (cell != NULL)
            {
                fread(&mdo,sizeof(mdo),1,cell);
                while (fgets(buffer,sizeof(buffer),cell)!=NULL)
                {
                    if(strcmp(buffer,"???\n")==0)
                    {
                        flag = 1;
                        overflowNumber = p;
                        break;
                    }
                }
                free(overflowCellName);
                if (flag != 0)
                    break;
                fclose(cell);
            }
        }
    }

    while (fgets(buffer,sizeof(buffer),cell)!=NULL)
    {
        if(strcmp(buffer,"\n")!=0)
        {
            sscanf(buffer,"%d %d %lf",&u,&v,&w);
            addEdgeInGraph(u,v,w);
        }
    }
    fclose(cell);
    if (overflowNumber<md.num_overflow)
    {
        for (int p=overflowNumber+1;p<=md.num_overflow;p++)
        {
            overflowCellName = getNameOfOverflowCell(p,cellName);
            cell = fopen(overflowCellName,"r");
            if (cell != NULL)
            {
                fread(&mdo,sizeof(mdo),1,cell);
                while (fgets(buffer,sizeof(buffer),cell)!=NULL)
                {
                    if(strcmp(buffer,"\n")!=0)
                    {
                        sscanf(buffer,"%d %d %lf",&u,&v,&w);
                        addEdgeInGraph(u,v,w);
                    }
                }
            }  
            free(overflowCellName);
            fclose(cell);     
        }
    }

    free(cellName);
    showGraph();
    updateCellList(cellId);
}

void showGraph()
{
    Graph_Node *temp;
    printf("nodes in graph : %d\n",nodesInGraph);
    for(int i=0;i<nodesInGraph;i++)
    {
        printf(" %d :-> ",gp[i].vertex);
        temp = gp[i].ptr;
        while (temp!=NULL)
        {
            printf("|%d| -> ",temp->vertex);
            temp = temp->next;
        }
        printf("\n");
    }
}

void updateCellList(int cellId)
{
    ListNode *ln = (ListNode *)malloc(sizeof(ListNode));
    if (ln == NULL)
    {
        printf("Memory overflow!");
        exit(4);
    }
    ln->id = cellId;
    ln->next = cellList;
    cellList = ln;
}

int numberOfCellInBuffer()
{
    int count = 0;
    ListNode *p = cellList;
    while (p != NULL)
    {
        count++;
        p = p->next;
    }
    return count;
}

int isCellInMemory(int cellId)
{
    ListNode *p=cellList;
    while (p!=NULL)
    {
        if (p->id == cellId)
        {
            return 1;
        }
        p = p->next;
    }
    return 0;
}

void showCellList()
{
    ListNode *p = cellList;
    while (p != NULL)
    {
        printf("%d, ",p->id);
        p = p->next;
    }
    printf("\n");
}

void heapDecreaseKey(int i)
{
    int parent = (i-1)/2;
    HeapNode temp;
    while (i>0 && minheap.arPtr[parent].d > minheap.arPtr[i].d)
    {
        /* Exchange i with parent. */
        temp.vertex = minheap.arPtr[i].vertex;
        temp.d = minheap.arPtr[i].d;
        minheap.arPtr[i].vertex = minheap.arPtr[parent].vertex;
        minheap.arPtr[i].d = minheap.arPtr[parent].d;
        minheap.arPtr[parent].vertex = temp.vertex;
        minheap.arPtr[parent].d = temp.d;
        i = parent;
        parent = (i-1)/2;
    }
}

void MinHeapify(int i)
{
    int smallest;
    HeapNode temp;
    int l = 2*i + 1;
    int r = 2*i + 2;
    if (l<minheap.heap_size && minheap.arPtr[l].d < minheap.arPtr[i].d )
        smallest = l;
    else
        smallest = i;
    if (r<minheap.heap_size && minheap.arPtr[r].d < minheap.arPtr[smallest].d)
        smallest = r;
    if (smallest!=i)
    {

        temp.vertex = minheap.arPtr[i].vertex;
        temp.d = minheap.arPtr[i].d;
        // temp.pi = minheap.arPtr[i].pi;
        minheap.arPtr[i].vertex = minheap.arPtr[smallest].vertex;
        minheap.arPtr[i].d = minheap.arPtr[smallest].d;
        // minheap.arPtr[i].pi = minheap.arPtr[smallest].pi;
        minheap.arPtr[smallest].vertex = temp.vertex;
        minheap.arPtr[smallest].d = temp.d;
        // minheap.arPtr[smallest].pi = temp.pi;
        MinHeapify(smallest);
    }
}

void insertInMinHeap(int u,double d)
{
    minheap.arPtr[minheap.heap_size].vertex = u;
    minheap.arPtr[minheap.heap_size].d = d;
    minheap.heap_size +=1;
    heapDecreaseKey(minheap.heap_size-1);
}

HeapNode ExtractMin()
{
    HeapNode temp;
    temp.vertex = minheap.arPtr[0].vertex;
    temp.d = minheap.arPtr[0].d;
    // temp.pi = minheap.arPtr[0].pi;
    
    minheap.arPtr[0].vertex = minheap.arPtr[minheap.heap_size-1].vertex;
    minheap.arPtr[0].d = minheap.arPtr[minheap.heap_size-1].d;
    // minheap.arPtr[0].pi = minheap.arPtr[minheap.heap_size-1].pi;

    minheap.heap_size -= 1;
    MinHeapify(0);

    return temp;
}

void displayMinHeap()
{
    for(int i=0;i<minheap.heap_size;i++)
    {
        printf("|%d,_,%.2lf| -> ",minheap.arPtr[i].vertex,minheap.arPtr[i].d);
    }
    printf("\n");
}

void destroyMinHeap()
{
    free(minheap.arPtr);
    minheap.arPtr = NULL;
}

void addInOpenList(int vt,double wt,int predecessor)
{
    SetNode *cl = (SetNode *)malloc(sizeof(SetNode));
    if (cl == NULL)
    {
        printf("memory overflow");
        exit(2);
    }
    cl->cv = vt;
    cl->dw = wt;
    cl->pi = predecessor;
    cl->next = openList;
    openList = cl;
}

SetNode* getListNodeFromOpenList(int k)
{
    SetNode *p,*q;
    p = NULL;
    q = openList;
    if (q == NULL)
        return NULL;
    if (q->cv == k)
    {
        openList = openList->next;
        q->next = NULL;
        return q;
    }
    while ((q->cv != k)&&(q->next != NULL))
    {
        p = q;
        q = q->next;
    }
    if (q->cv == k)
    {
        p->next = q->next;
        q->next = NULL;
        return q;
    }
    return NULL;
}

int isNodeInOpenList(int vt)
{
    SetNode *p=openList;
    while (p != NULL)
    {
        if (p->cv == vt)
        {
            return 1;
        }
        p = p->next;
    }
    return 0;
}

void updateOpenList(int v,double w,int predecessor)
{
    SetNode *p=openList;
    while (p != NULL)
    {
        if (p->cv == v)
        {
            p->dw = w;
            p->pi = predecessor;
            break;
        }
        p = p->next;
    }
}

void addInClosedList(int vertex)
{
    SetNode *q,*p;
    q = getListNodeFromOpenList(vertex);
    if (q == NULL)
    {
        printf("Node not found in open list!");
        exit(3);
    }
    if (closedList == NULL)
    {
        closedList = q;
        lastptr = q;
    }
    else
    {
        p = closedList;
        while (p->next != NULL)
            p = p->next;
        p->next = q;
        lastptr = q;
    }
}

int isNodeInClosedList(int vt)
{
    SetNode *p=closedList;
    while (p != NULL)
    {
        if (p->cv == vt)
            return 1;
        p = p->next;
    }
    return 0;
}

void showList(SetNode *q)
{
    while (q != NULL)
    {
        printf("%d,%d,%.2lf - ",q->cv,q->pi,q->dw);
        q = q->next;
    }
    printf("\n");
}

void Relax(HeapNode u,int v,double w)
{
    int i,flag=0;
    for (i=0;i<minheap.heap_size;i++)
    {
        if (minheap.arPtr[i].vertex == v)
        {
            flag = 1;
            break;
        }
    }
    if (flag == 1)
    {
        if (minheap.arPtr[i].d > u.d + w)
        {
            minheap.arPtr[i].d = u.d + w;
            heapDecreaseKey(i);
            updateOpenList(v,u.d+w,u.vertex);
        }
    }
}

int isDestinationClosed()
{
    if (lastptr == NULL)
        return 0;
    else
    {
        if (lastptr->cv == destination_vertex)
            return 1;
    }
    return 0;
}

void printShortestPath(SetNode *st,int v)
{
    SetNode *temp;
    if (st->cv == v)
    {
        printf("|%d,%.2lf| - ",st->cv,st->dw);
        pathLength++;
    }
    else
    {
        temp = closedList;
        while (temp->cv != v)
        {
            temp = temp->next;
        }
        printShortestPath(st,temp->pi);
        printf("|%d,%.2lf| - ",temp->cv,temp->dw);
        pathLength++;
    }
}

void destroyGraph()
{
    Graph_Node *temp1,*temp2;
    for (int i=0;i<nodesInGraph;i++)
    {
        temp1 = gp[i].ptr;
        while (temp1 != NULL)
        {
            temp2 = temp1;
            temp1 = temp1->next;
            free(temp2);;
        }
    }
}

void destroySetList(SetNode *list)
{
    SetNode *temp = list;
    while (list != NULL)
    {
        temp = list;
        list = list->next;
        free(temp);
    }
}

void destroyList()
{
    ListNode *temp;
    while (cellList != NULL)
    {
        temp = cellList;
        cellList = cellList->next;
        free(temp);
    }
}