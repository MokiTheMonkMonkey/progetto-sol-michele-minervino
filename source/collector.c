#include <collector.h>
#include "./../includes/collector.h"


void collectorExitFun(){

    freeTree(tree);

}

int sock_connect(){

    int fd_sock , cfd;

    struct sockaddr_un sa;


    IS_MENO1(fd_sock = socket(AF_UNIX , SOCK_STREAM , 0 ) , "errore creazione socket :" , return -1 )


    memset(&sa, '\0' , sizeof(sa));

    strncpy( sa.sun_path , SOCK_NAME , SOCK_NAME_LEN );
    sa.sun_family = AF_UNIX;

    IS_MENO1 (bind(fd_sock , (struct sockaddr *) &sa , sizeof(sa)) , "errore bind :" , return -1 )

    IS_MENO1 (listen (fd_sock , SOMAXCONN) , "errore listen :", return -1 )

    IS_MENO1 (cfd = accept (fd_sock , 0 , NULL ) , "errore accept :" , return -1)

    return cfd;

}
void insTree(Mes nodoIns,TreeNode ** cTree) {

    if(*cTree == NULL){

        *cTree = s_malloc(sizeof(TreeNode));
        size_t nameSize = strnlen(nodoIns .nome ,MAX_NAME) + 1;
        (*cTree) -> fileName = s_malloc(nameSize);
        (*cTree) -> val = nodoIns . val;
        strncpy((*cTree) -> fileName, nodoIns . nome, nameSize);
        (*cTree) -> right = NULL;
        (*cTree) -> left = NULL;
        return;

    }
    if((*cTree) -> val < nodoIns . val){

        insTree(nodoIns , &((*cTree) -> right));

    }
    else{

        insTree(nodoIns,&((*cTree) -> left));

    }


}

void freeTree(TreeNode * cTree){

    if(!cTree){

        return;

    }
    if(cTree -> left){

        freeTree(cTree -> left);

    }
    if(cTree -> right){

        freeTree(cTree -> right);

    }
    free(cTree -> fileName);
    free(cTree);

}


void printTree (TreeNode * cTree){

    if(!cTree){

        return;

    }
    printTree(cTree -> left);
    fprintf(stdout , "%ld %s\n" , cTree -> val , cTree -> fileName );
    printTree(cTree -> right);

}
