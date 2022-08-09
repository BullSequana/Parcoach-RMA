/* AVL tree for notifications */

#include <stdio.h>
#include <stdlib.h>
#include "core/notif_tree.h"
#include "core/interval.h"

/* Calculate height of the tree */
static int height(const Notif_tree *N) {
  if (N == NULL)
    return 0;
  return N->height;
}

static inline int max(int A, int B)
{
    return (A > B ? A : B);
}

/* Create a new node in the notification tree */
static Notif_tree *new_notif_tree(Interval *itv) {
  Notif_tree *node = (Notif_tree *)
    malloc(sizeof(Notif_tree));
  node->notif_id = get_notif_id(itv);
  node->itv = itv;
  node->left = NULL;
  node->right = NULL;
  node->height = 1;
  node->tree_size = 1;
  return node;
}

/* Right rotation of the tree */
static Notif_tree *notif_tree_rightRotate(Notif_tree *y) {
  if (y == NULL || y->left == NULL)
    return y;

  Notif_tree *x = y->left;
  Notif_tree *T2 = x->right;

  x->right = y;
  y->left = T2;

  y->height = max(height(y->left), height(y->right)) + 1;
  x->height = max(height(x->left), height(x->right)) + 1;

  return x;
}

/* Left rotation of the tree */
static Notif_tree *notif_tree_leftRotate(Notif_tree *x) {
  if (x == NULL || x->right == NULL)
    return x;

  Notif_tree *y = x->right;
  Notif_tree *T2 = y->left;

  y->left = x;
  x->right = T2;

  x->height = max(height(x->left), height(x->right)) + 1;
  y->height = max(height(y->left), height(y->right)) + 1;

  return y;
}

/* Get the balance factor */
static int getBalance(const Notif_tree *N) {
  if (N == NULL)
    return 0;
  return height(N->left) - height(N->right);
}

/* Inserts a new node with the notification ID associated to the interval
 * passed in parameter. */
Notif_tree *insert_notif_tree(Notif_tree *node, Interval *itv) {
  if (node == NULL)
    return (new_notif_tree(itv));

  int notif_id = itv->notification_id;

  if (notif_id < node->notif_id)
    node->left = insert_notif_tree(node->left, itv);
  else
    node->right = insert_notif_tree(node->right, itv);

  node->tree_size = 1 + node->tree_size;

  /* Update the balance factor of each node and
   * balance the tree */
  node->height = 1 + max(height(node->left),
               height(node->right));

  int balance = getBalance(node);
  int notif_id_left = notif_id + 1;
  if (NULL != node->left)
      notif_id_left = node->left->notif_id;
  int notif_id_right = notif_id - 1;
  if (NULL != node->right)
      notif_id_right = node->right->notif_id;

  if (balance > 1 && notif_id < notif_id_left)
    return notif_tree_rightRotate(node);

  if (balance < -1 && notif_id > notif_id_right)
    return notif_tree_leftRotate(node);

  if (balance > 1 && notif_id > notif_id_left) {
    node->left = notif_tree_leftRotate(node->left);
    return notif_tree_rightRotate(node);
  }

  if (balance < -1 && notif_id < notif_id_right) {
    node->right = notif_tree_rightRotate(node->right);
    return notif_tree_leftRotate(node);
  }

  return node;
}

/* Find the leftmost node of the tree */
static Notif_tree *minValueNode(Notif_tree *node) {
  Notif_tree *current = node;

  while (current->left != NULL)
    current = current->left;

  return current;
}

/* Free the whole tree from memory. Used for full cleanup when synchronizing
 * MPI-RMA epochs. The expected return is NULL. */
Notif_tree *free_notif_tree(Notif_tree *root) {
    Notif_tree *temp = root;
    while (NULL != temp) {
        temp = delete_notif_tree(temp, temp->notif_id);
    }
    return temp;
}

/* Remove a specific notification ID from the tree. */
Notif_tree *delete_notif_tree(Notif_tree *root, int notif_id) {
  if (root == NULL)
    return root;

  int notif_id_root = root->notif_id;

  if (notif_id < notif_id_root) {
    root->left = delete_notif_tree(root->left, notif_id);
  } else if (notif_id > notif_id_root) {
    root->right = delete_notif_tree(root->right, notif_id);
  } else {
    /* If part of the subtree of the node to delete is empty, just bring back
     * the other subtree up, or remove the root if it is a leaf */
    if ((root->left == NULL) || (root->right == NULL)) {
      /* Try finding the side of the tree that is not to bring up to replace root */
      Notif_tree *temp = root->left ? root->left : root->right;

      /* If the value is NULL, it means that root is a leaf; just delete it. */
      if (temp == NULL) {
        free(root);
        root = NULL;
      } else {
        /* Else, put the subtree that is not empty as new root */
        Notif_tree *to_free = root;
        root = temp;
        free(to_free);
      }
    } else {
      /* If both of the subtrees are populated, try finding the leftmost value
       * of the right tree, and substitute it to the root. To do so, we will
       * fill the current root with the values of this leftmost node of the
       * right tree, and remove this leaf instead.*/
      Notif_tree *temp = minValueNode(root->right);
      root->notif_id = temp->notif_id;
      root->itv = temp->itv;
      root->right = delete_notif_tree(root->right, temp->notif_id);
    }
  }

  if (root == NULL)
    return root;

  /* Update the balance factor of each node and
   * balance the tree */
  root->height = 1 + max(height(root->left),
               height(root->right));

  int balance = getBalance(root);
  if (balance > 1 && getBalance(root->left) >= 0)
    return notif_tree_rightRotate(root);

  if (balance > 1 && getBalance(root->left) < 0) {
    root->left = notif_tree_leftRotate(root->left);
    return notif_tree_rightRotate(root);
  }

  if (balance < -1 && getBalance(root->right) <= 0)
    return notif_tree_leftRotate(root);

  if (balance < -1 && getBalance(root->right) > 0) {
    root->right = notif_tree_rightRotate(root->right);
    return notif_tree_leftRotate(root);
  }

  root->tree_size = root->tree_size - 1;

  return root;
}

/* Print the tree in order. */
void in_order_print_notif_tree(Notif_tree *root) {
  if (root != NULL) {
    print_interval(*(root->itv));
    in_order_print_notif_tree(root->left);
    in_order_print_notif_tree(root->right);
  }
}

/* Prints interval tree statistics:
 * - Number of nodes in the tree
 * - Memory size of the tree
 * - Depth of the longest branch in the tree
 */
void print_notif_tree_stats(const Notif_tree *root)
{
  if (root != NULL)
  {
    printf("Tree statistics:\n");
    printf("Number of nodes in the tree: %lu \n", root->tree_size);
    printf("Size of the tree: %lu \n", root->tree_size * sizeof(Notif_tree));
    printf("Depth of the longest branch of the tree: %d \n", root->height);
  }
}

Interval *find_interval_from_notif(Notif_tree *root, int notif_id)
{
  Interval *itv = NULL;
  if (root == NULL)
    return itv;

  int notif_id_root = root->notif_id;

  if (notif_id == notif_id_root) {
      return root->itv;
  }
  
  if (notif_id < notif_id_root) {
    itv = find_interval_from_notif(root->left, notif_id);
  } else if (notif_id > notif_id_root) {
    itv = find_interval_from_notif(root->right, notif_id);
  }

  return itv;
}

/* Kept for testing purposes */
//int main() {
//  Notif_tree *root = NULL;
//  Interval *itv1 = create_interval(0, 2, RMA_READ, 1, 0, NULL);
//  Interval *itv2 = create_interval(2, 4, RMA_WRITE, 2,  0, NULL);
//  Interval *itv3 = create_interval(4, 6, RMA_WRITE, 3,  0, NULL);
//  Interval *itv4 = create_interval(6, 8, RMA_WRITE, 4,  0, NULL);
//
//  root = insert_notif_tree(root, itv1);
//  root = insert_notif_tree(root, itv2);
//  root = insert_notif_tree(root, itv3);
//  root = insert_notif_tree(root, itv4);
//
//  in_order_print_notif_tree(root);
//
//  Interval *itv = find_interval_from_notif(root, 5);
//  if (itv)
//    print_interval(*itv);
//
//  root = delete_notif_tree(root, 3);
//
//  printf("\nAfter deletion: ");
//  in_order_print_notif_tree(root);
//
//  root = free_notif_tree(root);
//
//  printf("\nAfter full deletion: ");
//  in_order_print_notif_tree(root);
//
//  return 0;
//}
