#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <float.h>

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

struct Meta_data 
{
    char name[50];
    int num_of_record; //Total number of record present in a cell;
    int num_overflow;  //number of overflow cell linket to main cell;
    char last_of_cell[20]; //Name of last overflow cell;
};
typedef struct Meta_data Cell_meta_data;

struct hash_map_node         // it map node id to its cordinate and cell id.
{
    int node_id;
    double x;
    double y;
    int cell_id;
    struct hash_map_node *next;
};
typedef struct hash_map_node Hash_map_node;

struct hash_map
{
    Hash_map_node *ptr;
};
typedef struct hash_map Hash_map;
Hash_map map[100];
int m = 100;

/* store the created file name. */
struct storeCreatedFileList
{
    int id;
    struct storeCreatedFileList *next;
};
typedef struct storeCreatedFileList storeCreatedFileList;
storeCreatedFileList *fileList;


double x_min,y_min,x_max,y_max;
int num_nodes=0;        // number of nodes.
int k,B;                // B contain the number of entries in a file.

void get_min_max(FILE *);
int get_cell_number(double,double);
char* get_name_of_overflow_cell(int ,char* );
char* get_cell_name(int );
char* get_overflow_cell(char *);
int record_in_cell(char *);
void store_node_in_cell(Node ,char *);
void store_edge_in_cell(Edge ,char *);
void insert_in_hash_map(Node, int);
void delete_hash_map();
int map_node_to_cell(int );
Node map_node_to_cordinate(int );
void add_special_symbol_in_file(char *);
int search_for_node_in_cell(char *,int );

int findIdInListFile(int p);
void insertInListFile(int );
void destroyListFile();

void storeNodeToCellData();

void main()
{
    FILE *nfp;         // node file pointer and edge file pointer.
    Node nd;
    Edge eg;
    int cell_no,cell_1,cell_2;
    char *cell_name;
    double countCell;
    int number_of_cell;
    nfp = fopen("idata_nodes.txt","rb");
    if(nfp == NULL)
    {
        printf("Nodes file not found!");
        exit(1);
    }
    get_min_max(nfp);

    printf("Enter Size of cell (k*K) : ");
    scanf("%d",&k);

    printf("x: min %lf max %lf\n",x_min,x_max);
    printf("y: min %lf max %lf\n",y_min,y_max);
    x_max = x_max + (k - fmod((x_max - x_min),k));
    y_max = y_max + (k - fmod((y_max - y_min),k));
    printf("x: min %lf max %lf\n",x_min,x_max);
    printf("y: min %lf max %lf\n",y_min,y_max);
    countCell = ((x_max - x_min)/k)*((y_max - y_min)/k);
    number_of_cell = round(countCell);
    printf("Cell Count : %.2lf\n",countCell);
    printf("Number of cells %d\n",number_of_cell);
    printf("Enter the number of entry in a file : ");
    scanf("%d",&B);

    rewind(nfp);
    while (fread(&nd,sizeof(nd),1,nfp)==1)
    {
        printf("data : %d\n",nd.id);
        cell_no = get_cell_number(nd.x_cordinate,nd.y_cordinate);
        printf("cell no : %d\n",cell_no);
        cell_name = get_cell_name(cell_no);
        printf("cell name :%s\n",cell_name);
        // printf("record in file : %d\n",record_in_cell(cell_name));
        if(record_in_cell(cell_name)>=B)
        {
            cell_name = get_overflow_cell(cell_name);
            store_node_in_cell(nd,cell_name);
        }
        else
        {
            store_node_in_cell(nd,cell_name);
            if (!findIdInListFile(cell_no))
                insertInListFile(cell_no);
        }
        insert_in_hash_map(nd,cell_no);
        free(cell_name);
    }
    fclose(nfp);

    /*   Edges   */
    /* add ### at the end of file for edges. */
    add_special_symbol_in_file("###");

    nfp = fopen("idata_edges.txt","rb");
    if(nfp==NULL)
    {
        printf("edges file not found!");
        exit(4);
    }
    printf("Edges\n");
    while (fread(&eg,sizeof(eg),1,nfp)==1)
    {
        cell_1 = map_node_to_cell(eg.n1);
        cell_2 = map_node_to_cell(eg.n2);
        if((cell_1==cell_2)&&(cell_1!=0)&&(cell_2!=0))
        {
            cell_name = get_cell_name(cell_1);
            if(record_in_cell(cell_name)>=B)
            {
                cell_name = get_overflow_cell(cell_name);
                store_edge_in_cell(eg,cell_name);
            }
            else
            {
                store_edge_in_cell(eg,cell_name);
            }
            free(cell_name);
        }
    }
    rewind(nfp);
    
    /* Store nodes in different cell. */
    /* add ***  for starting boundary nodes. */
    add_special_symbol_in_file("***");

    printf("boundary nodes...\n");
    while (fread(&eg,sizeof(eg),1,nfp)==1)
    {
        cell_1 = map_node_to_cell(eg.n1);
        cell_2 = map_node_to_cell(eg.n2);
        // printf("cell_1 %d, cell_2 %d\n",cell_1,cell_2);
        if((cell_1 != cell_2)&&(cell_1!=0)&&(cell_2!=0))
        {
            // store n2 in cell1.
            cell_name = get_cell_name(cell_1);
            if(!search_for_node_in_cell(cell_name,eg.n2))
            {
                nd = map_node_to_cordinate(eg.n2);
                if(record_in_cell(cell_name)>=B)
                {
                    cell_name = get_overflow_cell(cell_name);
                    store_node_in_cell(nd,cell_name);
                }
                else
                {
                    store_node_in_cell(nd,cell_name);
                }
            }
            free(cell_name);

            // store n1 in cell2.
            cell_name = get_cell_name(cell_2);
            if(!search_for_node_in_cell(cell_name,eg.n1))
            {
                nd = map_node_to_cordinate(eg.n1);
                if(record_in_cell(cell_name)>=B)
                {
                    cell_name = get_overflow_cell(cell_name);
                    store_node_in_cell(nd,cell_name);
                }
                else
                {
                    store_node_in_cell(nd,cell_name);
                }
            }
            free(cell_name);
        }
    }
    rewind(nfp);

    /* store boundary edges. */
    /* add ??? for boundary edges. */
    add_special_symbol_in_file("???");
    printf("boundary edges\n");

    while (fread(&eg,sizeof(eg),1,nfp)==1)
    {
        cell_1 = map_node_to_cell(eg.n1);
        cell_2 = map_node_to_cell(eg.n2);
        if((cell_1!=cell_2)&&(cell_1!=0)&&(cell_2!=0))
        {
            cell_name = get_cell_name(cell_1);
            if(record_in_cell(cell_name)>=B)
            {
                cell_name = get_overflow_cell(cell_name);
                store_edge_in_cell(eg,cell_name);
            }
            else
            {
                store_edge_in_cell(eg,cell_name);
            }
            free(cell_name);

            cell_name = get_cell_name(cell_2);
            if(record_in_cell(cell_name)>=B)
            {
                cell_name = get_overflow_cell(cell_name);
                store_edge_in_cell(eg,cell_name);
            }
            else
            {
                store_edge_in_cell(eg,cell_name);
            }
            free(cell_name);
        }
    }
    printf("End!\n");

    /* store nodeId to cellId in a file for use of partition dijkstra's algo */
    storeNodeToCellData();


    delete_hash_map();
    fclose(nfp);
}

void get_min_max(FILE *nfp)
{
    Node nd1,nd2;
    while (fread(&nd1,sizeof(nd1),1,nfp)==1)
    {
        num_nodes++;
    }
    printf("numver of Nodes : %d\n",num_nodes);
    rewind(nfp);
    if(num_nodes%2==0)
    {
        fread(&nd1,sizeof(nd1),1,nfp);
        fread(&nd2,sizeof(nd2),1,nfp);
        if(nd1.x_cordinate<nd2.x_cordinate)
        {
            x_min = nd1.x_cordinate;
            x_max = nd2.x_cordinate;
        }
        else
        {
            x_min = nd2.x_cordinate;
            x_max = nd1.x_cordinate;
        }
        if(nd1.y_cordinate<nd2.y_cordinate)
        {
            y_min = nd1.y_cordinate;
            y_max = nd2.y_cordinate;
        }
        else
        {
            y_min = nd2.y_cordinate;
            y_max = nd1.y_cordinate;
        }
    }
    else
    {
        fread(&nd1,sizeof(nd1),1,nfp);
        x_min = nd1.x_cordinate;
        x_max = nd1.x_cordinate;
        y_min = nd1.y_cordinate;
        y_max = nd1.y_cordinate;
    }
    while ((fread(&nd1,sizeof(nd1),1,nfp)==1)&&(fread(&nd2,sizeof(nd2),1,nfp)==1))
    {
        if(nd1.x_cordinate<nd2.x_cordinate)
        {
            if(nd1.x_cordinate<x_min)
                x_min = nd1.x_cordinate;
            if(nd2.x_cordinate>x_max)
                x_max = nd2.x_cordinate;
        }
        else
        {
            if(nd2.x_cordinate<x_min)
                x_min = nd2.x_cordinate;
            if(nd1.x_cordinate>x_max)
                x_max = nd1.x_cordinate;
        }

        if(nd1.y_cordinate<nd2.y_cordinate)
        {
            if(nd1.y_cordinate<y_min)
                y_min = nd1.y_cordinate;
            if(nd2.y_cordinate>y_max)
                y_max = nd2.y_cordinate;
        }
        else
        {
            if(nd2.y_cordinate<y_min)
                y_min = nd2.y_cordinate;
            if(nd1.y_cordinate>y_max)
                y_max = nd1.y_cordinate;
        }
    }
}

int get_cell_number(double x,double y)
{
    int x1,y1,cn;
    double cell_no;
    x1 = floor((x-x_min)/k);
    y1 = floor((y-y_min)/k);
    cell_no = y1*((x_max - x_min)/k) + x1 + 1;
    cn = round(cell_no);
    return cn;
}

char* get_cell_name(int cell)
{
    char *name = (char *)malloc(sizeof(char)*10);
    sprintf(name,"%d",cell);
    strcat(name,".txt");
    return name;
}

char* get_name_of_overflow_cell(int p,char* ck)
{
    char *bk = (char *)malloc(sizeof(char)*20);
    sprintf(bk,"%d",p);
    strcat(bk,"_OV_");
    strcat(bk,ck);
    return bk;
}

char* get_overflow_cell(char *cell_name)
{   
    FILE *cell,*overflow_cell;
    char *ov_cell;
    Cell_meta_data md1,md2;
    cell = fopen(cell_name,"r+");
    if(cell == NULL)
    {
        printf("unable to open file(overflow)!");
        printf("Error: %d (%s)\n", errno, strerror(errno));
        exit(3);
    }
    fread(&md1,sizeof(md1),1,cell);
    // fscanf(cell,"%s %d %d %s",md1.name,&md1.num_of_record,&md1.num_overflow,md1.last_of_cell);
    if(md1.num_overflow == 0)
    {
        // first time overflow.
        md1.num_overflow = 1;
        ov_cell = get_name_of_overflow_cell(md1.num_overflow,cell_name);
        strcpy(md1.last_of_cell,ov_cell);
        rewind(cell);
        // printf("first overflow cell %s\n",md1.last_of_cell);
        fwrite(&md1,sizeof(md1),1,cell);
        // fprintf(cell,"%s %d %d %s\n",md1.name,md1.num_of_record,md1.num_overflow,md1.last_of_cell);
        fclose(cell);
    }
    else
    {
        //find previously overflow cell and return it.
        overflow_cell = fopen(md1.last_of_cell,"r");
        if(overflow_cell == NULL)
        {
            printf("overflow cell not found\n");
            printf("Error: %d (%s)\n", errno, strerror(errno));
            exit(4);
        }
        fread(&md2,sizeof(md2),1,overflow_cell);
        // fscanf(overflow_cell,"%s %d %d %s",md2.name,&md2.num_of_record,&md2.num_overflow,md2.last_of_cell);
        if(md2.num_of_record >= B)
        {
            md1.num_overflow += 1;
            ov_cell = get_name_of_overflow_cell(md1.num_overflow,cell_name);
            strcpy(md1.last_of_cell,ov_cell);
            rewind(cell);
            fwrite(&md1,sizeof(md1),1,cell);
            // fprintf(cell,"%s %d %d %s\n",md1.name,md1.num_of_record,md1.num_overflow,md1.last_of_cell);        
        }
        else
        {   
            ov_cell = (char *)malloc(sizeof(char)*20);
            strcpy(ov_cell,md1.last_of_cell);
        }
        fclose(overflow_cell);
        fclose(cell);
    }
    free(cell_name);
    return ov_cell;
}

int record_in_cell(char *cell_name)
{
    Cell_meta_data md;
    char buffer[100];
    int MAX_PATH_LENGTH = 80;
    FILE *cellfl,*cell;
    cellfl = fopen(cell_name,"r");
    if(cellfl == NULL)
    {
        // printf("yahoo\n");
        // printf("Error: %d (%s)\n", errno, strerror(errno));
        return 0;
    }
    // if (fread(&md,sizeof(md),1,cellfl)!=1)
    // {
    //     printf("why\n");
    //     cellfl = fopen(cell_name,"rb");
    //     if (cellfl == NULL)
    //     {
    //         printf("Error: %d (%s)\n", errno, strerror(errno));
    //     }
    //     fread(&md,sizeof(md),1,cellfl);
    //     printf("num of record : %d\n",md.num_of_record);
    //     exit(5);
    // }
    if (fread(&md,sizeof(md),1,cellfl)!=1)
    {
        printf("yahoooo\n");
        printf("clell name %s\n",cell_name);
        printf("Error: %d (%s)\n", errno, strerror(errno));
        
        while (fgets(buffer,sizeof(buffer),cellfl))
        {
            printf("%s\n",buffer);
        }
        fgets(buffer, sizeof(buffer), stdin);
        printf("%s\n",buffer);
        // printf("%s ",cellfl->_bufsiz);
        // printf("%s ",cellfl->_charbuf);
        // printf("%s ",cellfl->_flag);
        // printf("%s ",cellfl->_cnt);
        // printf("%s ",cellfl->_file);
        // printf("%s ",cellfl->_ptr);
        // printf("%s ",cellfl->_tmpfna2me);
        // printf("%s ",cellfl->_base);
        // printf("\n");
        cell = fopen(".\45.txt","r");
        printf("delhdlsafj\n");
        fread(&md,sizeof(md),1,cell);
        printf("num of record : %d\n",md.num_of_record);
    }
    fclose(cellfl);
    printf("num of record : %d\n",md.num_of_record);
    return md.num_of_record;
}

void store_node_in_cell(Node nd,char *cell_name)
{
    FILE *cell;
    Cell_meta_data md;
    cell = fopen(cell_name,"r+");
    if(cell==NULL)
    {
        // create cell file first time.
        cell = fopen(cell_name,"w+");
        if(cell == NULL)
        {
            printf("unable to create cell file!");
            exit(2);
        }
        strcpy(md.name,cell_name);
        md.num_of_record = 1;
        md.num_overflow = 0;
        strcpy(md.last_of_cell,"NULL");
        fwrite(&md,sizeof(md),1,cell);
        fprintf(cell,"\n\n");
    }
    else
    {   
        fread(&md,sizeof(md),1,cell);
        rewind(cell);
        md.num_of_record += 1;
        fwrite(&md,sizeof(md),1,cell);
    }
    fseek(cell,0,SEEK_END);
    fprintf(cell,"%d %lf %lf\n",nd.id,nd.x_cordinate,nd.y_cordinate);
    // rewind(cell);
    // fread(&md,sizeof(md),1,cell);
    // printf("%s %d %d %s\n",md.name,md.num_of_record,md.num_overflow,md.last_of_cell);
    fclose(cell);
}   

void store_edge_in_cell(Edge eg,char *cell_name)
{
    FILE *cell;
    Cell_meta_data md;
    cell = fopen(cell_name,"r+");
    if(cell == NULL)
    {
        cell = fopen(cell_name,"w+");
        if(cell == NULL)
        {
            printf("unable to create cell file!");
            printf("Error: %d (%s)\n", errno, strerror(errno));
            exit(2);
        }   
        strcpy(md.name,cell_name);
        md.num_of_record = 1;
        md.num_overflow = 0;
        strcpy(md.last_of_cell,"NULL");
        fwrite(&md,sizeof(md),1,cell);
        fprintf(cell,"\n\n");
    }
    else
    {
        fread(&md,sizeof(md),1,cell);
        // fscanf(cell,"%s %d %d",md.name,&md.num_of_record,&md.num_overflow);
        rewind(cell);
        md.num_of_record += 1;
        fwrite(&md,sizeof(md),1,cell);
        // fprintf(cell,"%s %d %d\n",md.name,md.num_of_record,md.num_overflow);
    }
    fseek(cell,0,SEEK_END);
    fprintf(cell,"%d %d %lf\n",eg.n1,eg.n2,eg.weight);
    fclose(cell);
}

void insert_in_hash_map(Node nd, int cell_no)
{
    int i = nd.id % m;
    Hash_map_node *map_node;
    map_node = (Hash_map_node *)malloc(sizeof(Hash_map_node));
    map_node->node_id = nd.id;
    map_node->cell_id = cell_no;
    map_node->x = nd.x_cordinate;
    map_node->y = nd.y_cordinate;
    map_node->next = NULL;

    if(map[i].ptr == NULL)
    {
        map[i].ptr = map_node;
    }
    else
    {
        map_node->next = map[i].ptr;
        map[i].ptr = map_node;
    }
    map_node = NULL;
}

void delete_hash_map()
{
    Hash_map_node *temp;
    for(int i=0;i<m;i++)
    {
        while(map[i].ptr!=NULL)
        {
            temp = map[i].ptr;
            map[i].ptr = temp->next;
            free(temp);
        }
    }
}

int map_node_to_cell(int k)
{
    int i = k%m;
    Hash_map_node *temp = map[i].ptr;
    while (temp!=NULL)
    {
        if(temp->node_id == k)
            return temp->cell_id;
        temp = temp->next;
    }
    return 0;
}

Node map_node_to_cordinate(int k)
{
    int i = k%m;
    Node nd;
    Hash_map_node *temp = map[i].ptr;
    while (temp!=NULL)
    {
        if(temp->node_id == k)
        {
            nd.id = temp->node_id;
            nd.x_cordinate = temp->x;
            nd.y_cordinate = temp->y;
            return nd;
        }
        temp = temp->next;
    }
    printf("node id not found in hash map.\n");
    return nd;
}

void add_special_symbol_in_file(char *symbol)
{
    FILE *fp;
    Cell_meta_data md;
    char *name;
    storeCreatedFileList *q=fileList;
    while (q!=NULL)
    {
        name = get_cell_name(q->id);
        fp = fopen(name,"r+");
        if(fp==NULL)
            return ;   
        fread(&md,sizeof(md),1,fp);
        if (md.num_overflow == 0)
        {
            fseek(fp,0,SEEK_END);
            fprintf(fp,"\n%s\n",symbol);
            fclose(fp);
        } 
        else
        {
            fclose(fp);
            fp = fopen(md.last_of_cell,"r+");
            if(fp==NULL)
                return ;
            fseek(fp,0,SEEK_END);
            fprintf(fp,"\n%s\n",symbol);
            fclose(fp);
        }
        q = q->next;
    }
}

int search_for_node_in_cell(char *cell_name,int n_id)
{
    char ch[100];
    char *overflow_cell_name;
    int f_id,k=0;
    Cell_meta_data md;
    FILE *fp = fopen(cell_name,"r");
    if(fp == NULL)
    {
        return 0;
    }
    fread(&md,sizeof(md),1,fp);
    while (fgets(ch,sizeof(ch),fp)!=NULL)
    {
        if(strcmp(ch,"***\n")==0)
        {
            k = 1;
            break;
        }
    }
    if(k==1)
    {
        while(fgets(ch,sizeof(ch),fp)!=NULL)
        {
            if(strcmp(ch,"???\n")==0)
            {
                fclose(fp);
                return 0;
            }
            sscanf(ch,"%d",&f_id);
            if(f_id == n_id)
            {
                fclose(fp);
                return 1;
            } 
        }
    }
    k = 0;
    fclose(fp);
    for(int i=1;i<=md.num_overflow;i++)
    {
        k = 0;
        overflow_cell_name = get_name_of_overflow_cell(i,cell_name);
        fp =  fopen(overflow_cell_name,"r");
        if(fp != NULL)
        {
            while (fgets(ch,sizeof(ch),fp)!=NULL)
            {
                if(strcmp(ch,"***\n")==0)
                {
                    k = 1;
                    break;
                }
            }            
            if(k==1)
            {
                while(fgets(ch,sizeof(ch),fp)!=NULL)
                {
                    sscanf(ch,"%d",&f_id);
                    if(f_id == n_id)
                    {
                        fclose(fp);
                        return 1;
                    } 
                }
            }
            fclose(fp);
        }
        free(overflow_cell_name);
    }
    return 0;
}

int findIdInListFile(int p)
{
    storeCreatedFileList *q=fileList;
    while (q!=NULL)
    {
        if (q->id==p)
            return 1;
        q = q->next;
    }
    return 0;
}

void insertInListFile(int p)
{
    storeCreatedFileList *temp = (storeCreatedFileList *)malloc(sizeof(storeCreatedFileList));
    temp->id = p;
    temp->next = NULL;
    if (fileList == NULL)
        fileList = temp;
    else
    {
        temp->next = fileList;
        fileList = temp;
    }
}

void destroyListFile()
{
    storeCreatedFileList *temp;
    while (fileList != NULL)
    {
        temp = fileList;
        fileList = fileList->next;
        free(temp);
    }
}

void storeNodeToCellData()
{
    Hash_map_node *temp;
    FILE *fd = fopen("nodeidToCellid.txt","w");
    if (fd == NULL)
    {
        printf("Error: %d (%s)\n", errno, strerror(errno));
        return ;
    }
    for (int i=0;i<100;i++)
    {
        temp = map[i].ptr;
        while (temp != NULL)
        {
            fprintf(fd,"%d %d\n",temp->node_id,temp->cell_id);
            temp = temp->next;
        }
    }
    fclose(fd);
}