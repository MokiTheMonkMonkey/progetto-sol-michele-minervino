#include <bst.h>
#include "./../includes/bst.h"


void insTree(Mes nodoIns,TreeNode ** tree) {

    if(*tree == NULL){

        *tree = _malloc(sizeof(TreeNode));
        size_t nameSize = strnlen(nodoIns .nome ,MAX_NAME) + 1;
        (*tree) -> fileName = _malloc(nameSize);
        (*tree) -> val = nodoIns . val;
        strncpy((*tree) -> fileName,nodoIns . nome,nameSize);
        (*tree) -> right = NULL;
        (*tree) -> left = NULL;
        return;

    }
    if((*tree) -> val < nodoIns . val){

        insTree(nodoIns , &((*tree) -> right));

    }
    else{

        insTree(nodoIns,&((*tree) -> left));

    }


}

void freeTree(TreeNode * tree){

    if(!tree){

        return;

    }
    if(tree -> left){

        freeTree(tree -> left);

    }
    if(tree -> right){

        freeTree(tree -> right);

    }
    free(tree -> fileName);
    free(tree);

}


void printTree (TreeNode * tree){

    if(!tree){

        return;

    }
    printTree(tree -> left);
    fprintf(stderr,"%ld %s\n",tree -> val,tree -> fileName);
    printTree(tree -> right);

}