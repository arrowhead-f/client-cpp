
#include "binaryTree.h"

node *newNode(char *_providerID){
    node *n = malloc(sizeof(node));
    strcpy(n->providerID,_providerID);
    n->left = NULL;
    n->right = NULL;
    return n;
}

int cmpID(char *_l, char *_r){
    if(strcmp(_l, _r) < 0)
	return 1;
    else
	return 0;
}

void insertNode(node **_root, node *_child){
    if(!*_root) *_root = _child;
    else insertNode( cmpID(_child->providerID,(*_root)->providerID) ? &(*_root)->left : &(*_root)->right, _child );
}

node *searchNode(node *root, char *_value){
    //return !root ? NULL : strcmp(root->providerID, _value)==0 ? root : searchNode( cmpID(_value, root->providerID) ? root->left : root  , _value);

    if(root == NULL){
	printf("root is NULL\n");
	return NULL;
    }
    else if( strcmp(root->providerID, _value) == 0 ){
	printf("ID is equal\n");
	return root;
    }
    else if( cmpID(_value, root->providerID) == 1 ){
	printf("searchNode(root->left)\n");
	searchNode(root->left, _value);
    }
    else{
	printf("searchNOde(root->right)\n");
	searchNode(root->right, _value);
    }
}

void printPreorder(node *_tree){
    if(_tree == NULL) return;

    printf("%s\n",_tree->providerID);
    printPreorder(_tree->left);
    printPreorder(_tree->right);
}

node *minValueNode(node * _node){
    node *current = _node;

    while(current->left != NULL)
	current = current->left;

    return current;
}

node *deleteID(node *_root, char *_id){
    if( _root == NULL ) return _root;

    if( strcmp(_id, _root->providerID) < 0 )
	_root->left = deleteID( _root->left, _id );
    else if( strcmp( _id, _root->providerID) > 0)
	_root->right = deleteID( _root->right, _id );
    else{
	    if(_root->left == NULL){
		node *temp = _root->right;
		free(_root);
		return temp;
	    }
	    else if(_root->right == NULL){
		node *temp = _root->left;
		free(_root);
		return temp;
	    }

	    node *temp = minValueNode(_root->right);
	    strcpy( _root->providerID, temp->providerID );

	    _root->right = deleteID(_root->right, temp->providerID);
    }
    return _root;
}
