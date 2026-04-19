# My Simple Malloc

This is a personal project to understand how does the implementation of gnu's dynamic memory allocation (`malloc`) work.
The implementation uses at its core an special type of self-balancing binary search tree: Red-Black tree. It's something a bit
different cause the data managed by this tree are actually memory segments so while the insertion logic is the same, the deletion
pipeline changes a little. Instead of swapping values with the inorder-succesor node (in the case of wanting to eliminate a node
with two children), we swap the nodes, basically re-ordering the tree's hierarchy.

The current state of this project is unfinished. I mean, is still in progress. I will provide a brief description of every function
in every module of this project as soon as I finish each module. By now, I will only add the `rb_tree` and `header` modules functions descriptions.
