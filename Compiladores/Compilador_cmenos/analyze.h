
#ifndef _ANALYZE_H_
#define _ANALYZE_H_

void atualizaEscopo(TreeNode * t);

/* Function buildSymtab constructs the symbol
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode *);

/* Procedure typeCheck performs type checking
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode *);

void checkNode(TreeNode * t);

#endif
