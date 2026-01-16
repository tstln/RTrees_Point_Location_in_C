#ifndef _INDEX_
#define _INDEX_

/* PGSIZE is normally the natural page size of the machine */
#define PGSIZE	512
#define NUMDIMS	2	/* number of dimensions */
#define NDEBUG

typedef float RectReal;


/*-----------------------------------------------------------------------------
| Global definitions.
-----------------------------------------------------------------------------*/

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define NUMSIDES 2*NUMDIMS

/* Ed. 
Regardless of its dimension, a rectangle can be defined by its 
"lower left" (xmin ymin in 2D) and "upper right" (xmax, ymax in 2D)
That way, if we need to change dimension we can juste change NUMDIMS 
without updating anything else and that code will still work.
*/
struct Rect 
{
	RectReal boundary[NUMSIDES]; /* xmin,ymin,...,xmax,ymax,... */
};

struct Node; 

struct Branch
{
	struct Rect rect; //ed. the rectangle that englobes all things below (child node and everything next) 
	//ed. (called MBR : minimum bounding rectangle)
	struct Node *child; //ed. pointer to the next node
};

/* max branching factor of a node */
#define MAXCARD (int)((PGSIZE-(2*sizeof(int))) / sizeof(struct Branch))

/* Ed.
That line shows us that this whole implementation of R-Trees has been made to be memory efficient.
We want a node to fit inside a memory page of the machine (PGSIZE). 
So we take PGSIZE, we put aside the two line header of 'struct Node' 
(the two int, 'count' and 'level')
then we divide by the total size of the Branch. 
Conclusion : we have the max number of branches we can fit inside a node, 
such that this node still fits inside a page.
That is called the max branching factor. 
*/

struct Node
{
	int count;
	int level; /* 0 is leaf, others positive */
	struct Branch branch[MAXCARD]; //ed. array that store data
	//ed. if a leaf, then points to the real data (for example an index of a mesh triangle, it would be casted to do so)
	//ed. so if level > 0, just points to another node
};

struct ListNode
{
	struct ListNode *next;
	struct Node *node;
};
/* Ed. 
ListNode is not really in our R-Tree data structure, 
it is an auxiliary chained list used by routines such as insertion and deletion.
To be more precise, when a node grows "too big" (over MAXCARD), then the R-Tree must split the node into two,
then temporarily stores some nodes somewhere to reinsert them later. It does so with ListNode.
*/

/*
 * If passed to a tree search, this callback function will be called
 * with the ID of each data rect that overlaps the search rect
 * plus whatever user specific pointer was passed to the search.
 * It can terminate the search early by returning 0 in which case
 * the search will return the number of hits found up to that point.
 */
typedef int (*SearchHitCallback)(int id, void* arg);


extern int RTreeSearch(struct Node*, struct Rect*, SearchHitCallback, void*);
extern int RTreeInsertRect(struct Rect*, int, struct Node**, int depth);
extern int RTreeDeleteRect(struct Rect*, int, struct Node**);
extern struct Node * RTreeNewIndex();
extern struct Node * RTreeNewNode();
extern void RTreeInitNode(struct Node*);
extern void RTreeFreeNode(struct Node *);
extern void RTreePrintNode(struct Node *, int);
extern void RTreeTabIn(int);
extern struct Rect RTreeNodeCover(struct Node *);
extern void RTreeInitRect(struct Rect*);
extern struct Rect RTreeNullRect();
extern RectReal RTreeRectArea(struct Rect*);
extern RectReal RTreeRectSphericalVolume(struct Rect *R);
extern RectReal RTreeRectVolume(struct Rect *R);
extern struct Rect RTreeCombineRect(struct Rect*, struct Rect*);
extern int RTreeOverlap(struct Rect*, struct Rect*);
extern void RTreePrintRect(struct Rect*, int);
extern int RTreeAddBranch(struct Branch *, struct Node *, struct Node **);
extern int RTreePickBranch(struct Rect *, struct Node *);
extern void RTreeDisconnectBranch(struct Node *, int);
extern void RTreeSplitNode(struct Node*, struct Branch*, struct Node**);

extern int RTreeSetNodeMax(int);
extern int RTreeSetLeafMax(int);
extern int RTreeGetNodeMax();
extern int RTreeGetLeafMax();

#endif /* _INDEX_ */
