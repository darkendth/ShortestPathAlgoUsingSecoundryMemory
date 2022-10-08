#include <stdio.h>
#include <stdlib.h>

struct Node {
    int id;
    double x_cordinate;
    double y_cordinate;
};
typedef struct Node Node;

struct Edge
{
    int n1;
    int n2;
    double weight;
};
typedef struct Edge Edge;


void main()
{
    FILE *dt,*cp;
    Node nd;
    Edge ed;
    char buffer[10000];
    dt = fopen("nodes.txt","r");
    if(dt==NULL)
    {
        printf("Nodes data not found!");
        exit(1);
    }
    cp = fopen("idata_nodes.txt","wb");
    if(cp==NULL)
    {
        printf("Not able to create a file(Nodes).");
        exit(1);
    }
    while (1)
    {
        if(!fgets(buffer,sizeof(buffer),dt))
            break;
        sscanf(buffer,"%d %lf %lf",&nd.id,&nd.x_cordinate,&nd.y_cordinate);
        // printf("%d %lf %lf\n",nd.id,nd.x_cordinate,nd.y_cordinate);
        fwrite(&nd,sizeof(nd),1,cp);
    }
    fclose(dt);
    fclose(cp);

    printf("Now create compatible files for edges.");
    dt = fopen("edges.txt","r");
    if(dt==NULL)
    {
        printf("edges data not found!");
        exit(1);
    }
    cp = fopen("idata_edges.txt","wb");
    if(cp==NULL)
    {
        printf("Not able to create a file(edges).");
        exit(1);
    }
    while (1)
    {
        if(!fgets(buffer,sizeof(buffer),dt))
            break;
        sscanf(buffer,"%d %d %lf",&ed.n1,&ed.n2,&ed.weight);
        // printf("%d %d %lf\n",ed.n1,ed.n2,ed.weight);
        fwrite(&ed,sizeof(ed),1,cp);
    }
    fclose(dt);
    fclose(cp);
}