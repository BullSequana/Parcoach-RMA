#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "core/interval_tree.h"
#include <inttypes.h>

static inline unsigned long max(unsigned long A, unsigned long B)
{
    return (A > B ? A : B);
}

/* Create a new node in the interval tree */
static Interval_tree *new_interval_tree(Interval *i)
{
  Interval_tree *temp;
  temp = malloc(sizeof(Interval_tree));
  temp->itv = i;
  temp->left = temp->right = NULL;
  temp->tree_size = 1;
  temp->height = 1;
  return temp;
}

/* Inserts a new node with the interval asked in the interval tree. The low
 * value of interval is used to do comparisons */
Interval_tree *insert_interval_tree(Interval_tree *root, Interval *i)
{
  /* If the tree is empty, return a new node */
  if(root == NULL)
    return new_interval_tree(i);

  uint64_t low_bound = get_low_bound(i);
  uint64_t low_bound_root = get_low_bound(root->itv);

  /* If root's low value is smaller than new interval's low value -> go to left
   * subtree */
  if(low_bound < low_bound_root)
  {
    if(root->left == NULL)
      root->left = new_interval_tree(i);
    else
      root->left = insert_interval_tree(root->left, i);
    /* Update max depth if needed */
    root->height = max(root->height, root->left->height + 1);
  }
  /* Else go to right subtree */
  else {
    if(root->right == NULL)
      root->right = new_interval_tree(i);
    else
      root->right = insert_interval_tree(root->right, i);
    /* Update max depth if needed */
    root->height = max(root->height, root->right->height + 1);
  }

  /* Increment size of the tree */
  root->tree_size++;

  return root;
}

/* Checks if the given interval overlaps with another interval in the interval
 * tree. If yes, returns the guilty interval. */
Interval *overlap_search(Interval_tree *root, Interval *i)
{
  /* Empty tree */
  if(root == NULL)
    return NULL;

  uint64_t low_bound = get_low_bound(i);
  uint64_t low_bound_root = get_low_bound(root->itv);

  /* Check if the given interval overlaps with root */
  if(if_intersects(*root->itv,*i))
    return root->itv;

  /* Check if the left subtree exists, and check overlap conditions */
  if(low_bound < low_bound_root) 
    return overlap_search(root->left, i);

  /* Check if the right subtree exists and check overlap conditions */
  if(low_bound > low_bound_root)
    return overlap_search(root->right, i); 

  return NULL;
}

/* Print the tree in order. */
void in_order_print_interval_tree(Interval_tree *root)
{
  if(root == NULL)
    return; 

  uint64_t low_bound_root = get_low_bound(root->itv);
  uint64_t up_bound_root = get_up_bound(root->itv);

  if(root->left != NULL) { 
    in_order_print_interval_tree(root->left);
  }

  printf("[%"PRIu64 ", %"PRIu64 "]\n",low_bound_root, up_bound_root);

  if(root->right != NULL) {
    in_order_print_interval_tree(root->right);
  }
}

/* Free the whole tree from memory. Used for full cleanup when synchronizing
 * MPI-RMA epochs. The expected return is NULL. */
Interval_tree *free_interval_tree(Interval_tree *root)
{
  if(root == NULL)
    return root;

  root->left = free_interval_tree(root->left);
  root->right = free_interval_tree(root->right);
  free(root->itv);
  free(root);

  return NULL;
}

/* Prints interval tree statistics:
 * - Number of nodes in the tree
 * - Memory size of the tree
 * - Depth of the longest branch in the tree
 */
void print_interval_tree_stats(Interval_tree *root)
{
  if (root != NULL)
  {
    printf("Tree statistics:\n");
    printf("Number of nodes in the tree: %lu \n", root->tree_size);
    printf("Size of the tree: %lu \n", root->tree_size * sizeof(Interval_tree));
    printf("Depth of the longest branch of the tree: %d \n", root->height);
  }
}

/*
int main()
{
  Interval *A = create_interval(16, 21, -1, READ);
  Interval *B = create_interval(8, 9, -1, READ);
  Interval *C = create_interval(25, 30, -1, READ);
  Interval *D = create_interval(15, 23, -1, READ);
  Interval *E = create_interval(17, 19, -1, READ);
  Interval *F = create_interval(5, 8, -1, READ);
  Interval *G = create_interval(26, 28, -1, READ);
  Interval *H = create_interval(0, 3, -1, READ);
  Interval *I = create_interval(20, 21, -1, READ);
  Interval *J = create_interval(6, 10, -1, READ);
  Interval_tree *my_tree = NULL;
  my_tree = new_interval_tree(A);
  printf("The root of this tree is : [%"PRIu64 ", %"PRIu64 "]\n", A->low_bound, A->up_bound);
  if(!overlap_search(my_tree, B))
    insert_interval_tree(my_tree, B);
  if(!overlap_search(my_tree, C))
    insert_interval_tree(my_tree, C);
  if(!overlap_search(my_tree, D))
    insert_interval_tree(my_tree, D);
  if(!overlap_search(my_tree, E))
    insert_interval_tree(my_tree, E);
  if(!overlap_search(my_tree, F))
    insert_interval_tree(my_tree, F);
  if(!overlap_search(my_tree, G))
    insert_interval_tree(my_tree, G); 
  if(!overlap_search(my_tree, H))
    insert_interval_tree(my_tree, H);
  if(!overlap_search(my_tree, I))
    insert_interval_tree(my_tree, I);
  if(!overlap_search(my_tree, J))
    insert_interval_tree(my_tree, J);
  printf("Inorder traversal of the constructed Interval Tree is :\n");
  in_order_print_interval_tree(my_tree);
  free(my_tree);
}
*/
