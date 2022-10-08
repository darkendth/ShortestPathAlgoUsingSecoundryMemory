#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>

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
Graph *gp;

/*  Define Min Heap.  */
struct  heap_node
{
    int vertex;
    double d;
    int pi;
};
typedef struct heap_node HeapNode;

struct heap
{
    struct heap_node *arPtr;
    int heap_size;
};
typedef struct heap Heap;
Heap minheap;

/*   Define List(SET).   */
struct set_node
{
    int cv;     // current vertex.
    int pi;     // predecessor vertex.
    double dw;   // shortest distance to cv.
    struct set_node *next;
};
typedef struct set_node Set_Node;
Set_Node *set,*lastSet;

/*  Global Variable Declaration.    */
int source_vertex,destination_vertex;

/*  Function Declaration    */
void initialize_graph(FILE *,int);
void isMemoryOverflow(Graph_Node *);
void Make_Graph(FILE *,int);
void show_graph(Graph *,int );

void Initialize(int ,int);
void MinHeapify(int );
void heapDecreaseKey(int );
HeapNode ExtractMin();
void addInSet(HeapNode );
void Relax(HeapNode ,int ,double );
void displayMinHeap();
int isDestinationClosed();
void showSet();
void printShortestPath(Set_Node *,int );

void destroyGraph(int );
void destroyMinHeap();
void destroySet();

void main()
{
    FILE *fp;
    int node_count=0,flag;
    HeapNode u;
    Graph_Node *v;
    char buffer[100];

    /* *************************** */
    fp = fopen("nodes.txt","r");
    if(fp==NULL)
    {
        printf("Error: %d (%s)\n", errno, strerror(errno));
        exit(1);
    }
    while (1)
    {
        if(!fgets(buffer,sizeof(buffer),fp))
            break;
        node_count++;
    }
    printf("number of nodes %d \n",node_count);
    rewind(fp);
    initialize_graph(fp,node_count);
    fclose(fp);

    fp = fopen("edges.txt","r");
    if(fp==NULL)
    {
        printf("Error: %d (%s)\n", errno, strerror(errno));
        exit(1);
    }
    Make_Graph(fp,node_count);
    /* ++++++++++++++++++++++++++++ */

    printf("Enter source and destination vertex : ");
    scanf("%d%d",&source_vertex,&destination_vertex);

    /*  Dijkstra's Algorithm.    */
    Initialize(source_vertex,node_count);
    while ((minheap.heap_size!=0)&&(!isDestinationClosed()))
    {
        u = ExtractMin();
        printf("minimum %d %.2f %d\n",u.vertex,u.d,u.pi);
        addInSet(u);
        for (int i = 0; i < node_count; i++)
        {
            if(gp[i].vertex == u.vertex)
            {
                v = gp[i].ptr;
                break;
            }
        }
        while (v != NULL)
        {
            Relax(u,v->vertex,v->weight);
            v = v->next;
        }
    }
    showSet();
    printShortestPath(set,lastSet->cv);

    destroyGraph(node_count);
    destroyMinHeap();
    destroySet();
}

void initialize_graph(FILE *fp,int n)
{
    char buffer[100];
    double tx,ty;
    int i=0;
    gp = (Graph *)malloc(sizeof(Graph)*n);
    if(gp==NULL)
    {
        printf("Memory overflow (unable to initialize graph) :");
        exit(2);
    }
    while (fgets(buffer,sizeof(buffer),fp))
    {
        sscanf(buffer,"%d %lf %lf",&(gp[i].vertex),&tx,&ty);
        gp[i].ptr = NULL;
        i++;
    }
}

void isMemoryOverflow(Graph_Node *g)
{
    if(g == NULL)
    {
        printf("Error: %d (%s)\n", errno, strerror(errno));
        exit(3);
    }
}

void Make_Graph(FILE *fp,int n)
{
    int u,v;
    double w;
    char buffer[100];
    Graph_Node *gn;
    while (fgets(buffer,sizeof(buffer),fp))
    {
        gn = (Graph_Node *)malloc(sizeof(Graph_Node));
        isMemoryOverflow(gn);
        sscanf(buffer,"%d %d %lf",&u,&v,&w);
        for(int i=0;i<n;i++)
        {
            if(gp[i].vertex == u)
            {
                gn->vertex = v;
                gn->weight = w;
                gn->next = gp[i].ptr;
                gp[i].ptr = gn;
                break;
            }
        }
    }
    show_graph(gp,n);
}

void show_graph(Graph *gp,int n)
{
    Graph_Node *temp;
    for(int i=0;i<n;i++)
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

void Initialize(int sv,int nc)
{
    /*  Create min heap with d set to infinity for all.
        and set precedecessor to null for all.          */
    
    minheap.arPtr = (HeapNode *)malloc(sizeof(HeapNode)*nc);
    if(minheap.arPtr == NULL)
    {
        printf("Error: %d (%s)\n", errno, strerror(errno));
        exit(2);
    }
    minheap.heap_size = nc;
    for(int i=0;i<nc;i++)
    {
        minheap.arPtr[i].vertex = gp[i].vertex;
        minheap.arPtr[i].d = DBL_MAX;
        minheap.arPtr[i].pi = -1;
        if(gp[i].vertex == sv)
            minheap.arPtr[i].d = 0;
    }
    // displayMinHeap();
    for (int i=(nc+1)/2-1;i>=0;i--)
    {
        MinHeapify(i);
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
        temp.pi = minheap.arPtr[i].pi;
        minheap.arPtr[i].vertex = minheap.arPtr[smallest].vertex;
        minheap.arPtr[i].d = minheap.arPtr[smallest].d;
        minheap.arPtr[i].pi = minheap.arPtr[smallest].pi;
        minheap.arPtr[smallest].vertex = temp.vertex;
        minheap.arPtr[smallest].d = temp.d;
        minheap.arPtr[smallest].pi = temp.pi;
        MinHeapify(smallest);
    }
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
        temp.pi = minheap.arPtr[i].pi;
        minheap.arPtr[i].vertex = minheap.arPtr[parent].vertex;
        minheap.arPtr[i].d = minheap.arPtr[parent].d;
        minheap.arPtr[i].pi = minheap.arPtr[parent].pi;
        minheap.arPtr[parent].vertex = temp.vertex;
        minheap.arPtr[parent].d = temp.d;
        minheap.arPtr[parent].pi = temp.pi;
        i = parent;
        parent = (i-1)/2;
    }
}

HeapNode ExtractMin()
{
    HeapNode temp;
    temp.vertex = minheap.arPtr[0].vertex;
    temp.d = minheap.arPtr[0].d;
    temp.pi = minheap.arPtr[0].pi;
    
    minheap.arPtr[0].vertex = minheap.arPtr[minheap.heap_size-1].vertex;
    minheap.arPtr[0].d = minheap.arPtr[minheap.heap_size-1].d;
    minheap.arPtr[0].pi = minheap.arPtr[minheap.heap_size-1].pi;

    minheap.heap_size -= 1;
    MinHeapify(0);

    return temp;
}

void addInSet(HeapNode u)
{
    Set_Node *temp = (Set_Node *)malloc(sizeof(Set_Node));
    Set_Node *q;
    temp->cv = u.vertex;
    temp->pi = u.pi;
    temp->dw = u.d;
    temp->next = NULL;
    if(set == NULL)
    {
        set = temp;
        lastSet = temp;
    }
    else
    {
        q = set;
        while (q->next != NULL)
            q = q->next;
        q->next = temp;
        lastSet = temp;
    }
}   

void Relax(HeapNode u,int v,double w)
{
    int i,flag=0;
    for (i = 0; i < minheap.heap_size; i++)
    {
        if (minheap.arPtr[i].vertex == v)
        {
            flag = 1;
            break;
        }
    }
    if (flag==1)
    {
        if (minheap.arPtr[i].d > u.d + w)
        {
            minheap.arPtr[i].d = u.d + w;
            minheap.arPtr[i].pi = u.vertex;
            heapDecreaseKey(i);
        }
    }   
}

int isDestinationClosed()
{
    if(lastSet==NULL)
        return 0;
    else
    {
        if (lastSet->cv==destination_vertex)
            return 1;
    }
    return 0;
}

void showSet()
{
    Set_Node *temp;
    temp = set;
    while (temp != NULL)
    {
        printf("|%d,%d,%.2lf| -> ",temp->cv,temp->pi,temp->dw);
        temp = temp->next;
    }
    printf("\n");
}

void printShortestPath(Set_Node *st,int v)
{
    Set_Node *temp;
    if (st->cv == v)
        printf("|%d,%.2lf| - ",st->cv,st->dw);
    else
    {
        temp = set;
        while (temp->cv != v)
        {
            temp = temp->next;
        }
        printShortestPath(st,temp->pi);
        printf("|%d,%.2lf| - ",temp->cv,temp->dw);
    }
}

void displayMinHeap()
{
    for(int i=0;i<minheap.heap_size;i++)
    {
        printf("|%d,_,%d| -> ",minheap.arPtr[i].vertex,minheap.arPtr[i].pi);
    }
    printf("\n");
}

void destroyGraph(int n)
{
    Graph_Node *temp1,*temp2;
    for (int i=0;i<n;i++)
    {
        temp1 = gp[i].ptr;
        while (temp1 != NULL)
        {
            temp2 = temp1;
            temp1 = temp1->next;
            free(temp2);
        }
    }
    free(gp);
    gp = NULL;
}

void destroyMinHeap()
{
    free(minheap.arPtr);
    minheap.arPtr = NULL;
}

void destroySet()
{
    Set_Node *temp;
    while (set != NULL)
    {
        temp = set;
        set = set->next;
        free(temp);
    }
}