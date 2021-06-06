#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
struct tree
{

    struct tree** branch;
    int size;
    int actsize;
    char ppid[10];
    char id[10];
    char console[2048];
    char state;
};
struct tree* createTree()
{
    struct tree* nTree;
    nTree=(struct tree*) malloc(sizeof(struct tree));
    memcpy(nTree->id,"0",sizeof("0"));
    memcpy(nTree->ppid,"0",sizeof("0"));
    nTree->branch=(struct tree**) malloc(sizeof(struct tree*)*5);
    for(int i=0;i<5;i++)
        nTree->branch[i]=NULL;
    nTree->size=5;
    memcpy(nTree->console,"",sizeof(""));
    nTree->state='\0';
    nTree->actsize=0;
    return nTree;
}
void clean(struct tree* root)
{
    for(int i=0;i<root->actsize;i++)
    {
        clean(root->branch[i]);
        free(root->branch[i]);
    }
    free(root->branch);
}
void addList(struct tree* root,struct tree* list)
{
    if(root->branch==NULL)
    {
        root->branch=(struct tree**) malloc(sizeof(struct tree*)*5);
        root->size=5;
        root->actsize=0;
    }
    else if(root->size-1==root->actsize)
    {
        struct tree** buff;
        buff = (struct tree**) malloc(sizeof(struct tree)*(root->size+3));
        for(int i=0;i<root->size+3;i++)
        {
            buff[i]=NULL;
        }
        for(int i=0;i<root->size;i++)
        {
            buff[i]=root->branch[i];
        }
        free(root->branch);
        root->branch=buff;
        root->size+=3;
    }
    root->branch[root->actsize]=list;
    root->actsize++;
}
void findList(struct tree* root,struct tree* list,int* res)
{
    if(!strcmp(root->id,list->ppid))
    {
        *res=1;
        addList(root,list);
    }
    else
    {
        for(int i=0;i<root->actsize;i++)
        {
            findList(root->branch[i],list,res);
        }
    }
}
void printList(struct tree* head,int div)
{
    //printf("%s %s %s %s\n", head->id, head->ppid, head->state, head->console);
    for(int i=0;i<head->actsize;i++)
    {
        printList( head->branch[i], div++);
    } 
}
struct tree* createList(const char* id, const char* ppid, const char* cons, const char* state)
{
    struct tree* list;
    list=(struct tree*) malloc(sizeof(struct tree));
    memcpy(list->id,id,10);
    memcpy(list->ppid,ppid,10);
    memcpy(list->console,cons,2048);
    list->state=state[0];
    list->size=0;
    list->actsize=0;
    list->branch=NULL;
    return list;
}
int isNum(const char* name)
{
    int i=0;
    while(name[i]!='\0')
    {
        if(name[i]>'0' && name[i]<'9')
            i++;
        else return 0;
    }
    return 1;
}
int getDirdata(const char *name,char* procc)
{
    char path[1024];
    char rbuffer[2048];
    DIR *dir;
    struct dirent *entry;
    if (!(dir = opendir(name)))
        return -1;
    snprintf(path, sizeof(path), "%s/%s", name, "cmdline");
    FILE* cmdData;
    cmdData = fopen(path,"r");
    if (cmdData==NULL)
    {
        printf("\n Error in opening the file");
        return -1;
    }
    int res=fread(rbuffer,1,sizeof(rbuffer),cmdData);
    fclose(cmdData);
    closedir(dir);
    if(res>0)
    {
        memcpy(procc,rbuffer,2048);
    }
    else return -1;
    return 0;
}
void getStat(const char* name, char* ppid, char* state)
{
    char path[1024];
    char rbuffer[256];
    DIR *dir;
    struct dirent *entry;
    if (!(dir = opendir(name)))
        return;
    snprintf(path, sizeof(path), "%s/%s", name, "stat");
    FILE* statData;
    statData = fopen(path,"r");
    if (statData==NULL)
    {
        printf("\n Error in opening the file");
        return;
    }
    int res=fread(rbuffer,1,sizeof(rbuffer),statData);
    if (res==0) 
    {
        fclose(statData);
        closedir(dir);
        return;
    }
    int i=0;
    for(;i<sizeof(rbuffer);i++)
    {
        if(rbuffer[i]==')')
        {
            //printf(" %c ",rbuffer[i+2]);
            break;
        }
    }
    state[0]=rbuffer[i+2];
    i=i+4;
    int k=0;
    for(;i<sizeof(rbuffer);i++,k++)
    {
        if(rbuffer[i]==' ') break;
        ppid[k]=rbuffer[i];
    }
    ppid[k]='\0';
    fclose(statData);
    closedir(dir);
    return;
}
void listdir(const char *name, int indent, struct tree* head)
{
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            char path[1024];
            char ppid[10];
            char procName[2048];
            char state[10];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            if(!isNum(entry->d_name)) continue;
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            int res=getDirdata(path,procName);
            if(res<0) continue;
            getStat(path,ppid,state);
            struct tree* list=createList(entry->d_name,ppid, procName,state);
            int add=0;
            findList(head,list,&add);
            if(add==0)
                addList(head,list);
        }
    }
    closedir(dir);
}
int main()
{
    DIR *dir;
    struct dirent *entry;
    char buffer[50];
    struct tree* head;
    head=createTree();
    listdir("/proc",0,head);
    printf("\n\n\n");
    printList(head,0);
    clean(head);
    free(head);
}
