#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <minigui/common.h>
#include <minigui/gdi.h>
#include <upf.h>

//#define VAL_DEBUG
//#define FUN_DEBUG
#include <my_debug.h>

#define APPEND_TO_LESS(parent_node, less_node) \
    do { \
        (parent_node)->less = less_node; \
        if (less_node) \
            (less_node)->parent = parent_node; \
    } while (0)

#define APPEND_TO_MORE(parent_node, more_node) \
    do { \
        (parent_node)->more = more_node; \
        if (more_node) \
            (more_node)->parent = parent_node; \
    } while (0)

#define LEAVE_PARENT(node) \
    do { \
        if (node && node->parent) \
        { \
            if (node->parent->less == node) \
                node->parent->less = NULL; \
            else if (node->parent->more == node) \
                node->parent->more = NULL; \
            else \
                assert (0); \
        } \
    } while (0)

void free_node (UPFGLYPHTREE** node)
{
    int n;
    int i;
    if (*node == NULL)
        return;

    n = (*node)->upf_node.max -(*node)->upf_node.min +1;
    for(i=0; i<n; i++){
        free((*node)->glyph[i].metrics);
        free((*node)->glyph[i].data);
    }
    free ((*node)->glyph);
    *node = NULL;
}


void free_glyph_tree(UPFGLYPHTREE** tree)
{
    if ((*tree)->less) {
        free_glyph_tree(&((*tree)->less));
    }
    if ((*tree)->more) {
        free_glyph_tree(&((*tree)->more));
    }
    free_node (tree);
}

static UPFGLYPHTREE* get_max_node (UPFGLYPHTREE* root)
{
    UPFGLYPHTREE* node;
    while (node->more)
        node = node->more;
    return node;
}

static UPFGLYPHTREE* get_min_node (UPFGLYPHTREE* root)
{
    UPFGLYPHTREE* node;
    while (node->less)
        node = node->less;
    return node;
}


/*add a node to the tree's node*/
static void add_node (UPFGLYPHTREE *root, UPFGLYPHTREE* node)
{
    if (root->upf_node.max < node->upf_node.min) {
        if (root->more)
            add_node (root->more, node);
        else 
            APPEND_TO_MORE (root, node);
    } 
    else if (node->upf_node.max < root->upf_node.min) {
        if (root->less)
            add_node (root->less, node);
        else 
            APPEND_TO_LESS (root, node);
    }
    else {
        assert (0);
    }
}

void print_tree (UPFGLYPHTREE* tree)
{
    fprintf(stderr, "{parent:0x%p--", tree->parent);
    fprintf(stderr, "less:0x%p--", tree->less);
    fprintf(stderr, "more:0x%p--}   ", tree->less);
    fprintf(stderr, "{cur-min:%d--", tree->upf_node.min);
    fprintf(stderr, "cur-max:%d}", tree->upf_node.max);
    fprintf(stderr, "\n");

    fprintf (stderr, "{\n");
    if (tree->less)
        print_tree (tree->less);
    if (tree->more)
        print_tree (tree->less);
    fprintf (stderr, "}\n");
}
#if 0
/*delete a node from the AVL tree*/
static void delete_node (UPFGLYPHTREE** root, UPFGLYPHTREE** node)
{
    UPFGLYPHTREE* parent;
    assert (*root);

    if (node == NULL)
        return;

    if (*node == *root) {
        if ((*root)->less) {
            UPFGLYPHTREE* max_node_in_less;
            max_node_in_less = get_max_node ((*root)->less);

            APPEND_TO_MORE (max_node_in_less, (*root)->more);
            APPEND_TO_LESS (max_node_in_less, (*root)->less);

            *root = max_node_in_less;
            free_node (node);

        }
        else if ((*root)->more) {
            *root = (*root)->more;
            free_node (&node);
        }
        else {
            free_node (*node);
            *node = NULL;
        }

    }
    else {
        parent = node->parent;
        /*node is parent's less*/
        if (parent->less == node) {
            APPEND_TO_MORE (parent, node->less);
            APPEND_TO_MORE (node->less, node->more);
        }
        /*node is parent's more*/
        else {
            APPEND_TO_LESS (parent, node->more);
            APPEND_TO_LESS (node->more, node->less);

        }
        free_node (node);
    }
}
#endif


/*append maybe at left or right on aim*/
void combine_node (UPFGLYPHTREE* ancestor, UPFGLYPHTREE* offspring)
{
    int left_num;
    int right_num;
    GLYPH* all_glyph;

    TEST_VAL (ancestor, %p);
    TEST_VAL (offspring, %p);
    TEST_VAL (ancestor->upf_node.min, %d);
    TEST_VAL (ancestor->upf_node.max, %d);

    TEST_VAL (offspring->upf_node.min, %d);
    TEST_VAL (offspring->upf_node.max, %d);

    /*  offspring, left; ancestor, right*/
    if (offspring->upf_node.max + 1 == ancestor->upf_node.min) {
        left_num = offspring->upf_node.max - offspring->upf_node.min + 1;
        right_num = ancestor->upf_node.max - ancestor->upf_node.min + 1;
        ancestor->upf_node.min = offspring->upf_node.min;

        all_glyph = (GLYPH*) malloc ((left_num + right_num) * sizeof(GLYPH));

        memcpy (all_glyph, offspring->glyph, 
                left_num * sizeof(GLYPH));
        memcpy (all_glyph+ left_num, ancestor->glyph, 
                right_num * sizeof(GLYPH));

        free (ancestor->glyph);
        ancestor->glyph = all_glyph;
        /*offspring is the most node of ascentor's less*/
        assert(!offspring->more);
        LEAVE_PARENT (offspring);
        APPEND_TO_MORE (offspring->parent, offspring->less);

        free (offspring);
    }
    /*ancestor, left;  offspring, right*/
    else if (ancestor->upf_node.max + 1 == offspring->upf_node.min) {
        left_num = ancestor->upf_node.max - ancestor->upf_node.min + 1;
        right_num = offspring->upf_node.max - offspring->upf_node.min + 1;
        ancestor->upf_node.max = offspring->upf_node.max;

        all_glyph = (GLYPH*) malloc ((left_num + right_num) * sizeof(GLYPH));

        memcpy (all_glyph, ancestor->glyph, 
                left_num * sizeof(GLYPH));
        memcpy (all_glyph + left_num, offspring->glyph,
                right_num * sizeof(GLYPH));

        ancestor->glyph = all_glyph;
        /*offspring is the least node of ascentor's more*/
        assert(!offspring->less);
        LEAVE_PARENT (offspring);
        APPEND_TO_LESS (offspring->parent, offspring->more);

        free (offspring->glyph);
        free (offspring);
    }
    /*error*/
    else
    {
        assert (0);
    }
}

/*delete glyph-dividing_line in node*/
void divide_node (UPFGLYPHTREE* node, int dividing_line)
{
    UPFGLYPHTREE* new_more;
    GLYPH* all_glyphs;
    int left_num;
    int right_num;

    assert (node->upf_node.min < dividing_line && dividing_line < node->upf_node.max);

    TEST_VAL (dividing_line, %d);

    all_glyphs = node->glyph;
    new_more = (UPFGLYPHTREE*)calloc(1, sizeof(UPFGLYPHTREE));

    new_more->upf_node.max = node->upf_node.max;
    new_more->upf_node.min = dividing_line + 1;
    node->upf_node.max = dividing_line - 1;

    left_num = node->upf_node.max - node->upf_node.min + 1;
    right_num = new_more->upf_node.max - new_more->upf_node.min + 1;

    node->glyph = (GLYPH*) malloc (left_num * sizeof(GLYPH));
    memcpy (node->glyph, all_glyphs, left_num*sizeof(GLYPH));

    new_more->glyph = (GLYPH*) malloc (right_num * sizeof(GLYPH));
    memcpy (new_more->glyph, all_glyphs + left_num + 1, 
            right_num*sizeof(GLYPH));

    APPEND_TO_MORE (new_more, node->more);
    APPEND_TO_MORE (node, new_more);

    TEST_VAL (node->upf_node.min, %d);
    TEST_VAL (node->upf_node.max, %d);

    TEST_VAL (node->more->upf_node.min, %d);
    TEST_VAL (node->more->upf_node.max, %d);

    free (all_glyphs);
}



/** =====================================================================
* Combines the continugous items.
* ===================================================================== */
void compress (UPFGLYPHTREE *root, UPFGLYPHTREE** min_offspring,
        UPFGLYPHTREE** max_offspring)
{
    UPFGLYPHTREE* l_min_offspring = NULL;
    UPFGLYPHTREE* l_max_offspring = NULL;
    UPFGLYPHTREE* m_min_offspring = NULL;
    UPFGLYPHTREE* m_max_offspring = NULL;

    if (!root)
        return;

    if (root->less) {
        compress (root->less, &l_min_offspring, &l_max_offspring);
        if (l_max_offspring->upf_node.max + 1 == root->upf_node.min)
            combine_node (root, l_max_offspring);
    }

    if (!root->less)
        *min_offspring = root;
    else
        *min_offspring = l_min_offspring;

    if (root->more) {
        compress (root->more, &m_min_offspring, &m_max_offspring);
        if (root->upf_node.max + 1 == m_min_offspring->upf_node.min)
            combine_node (root, m_min_offspring);
    }

    if (!root->more)
        *max_offspring = root;
    else
        *max_offspring = m_max_offspring;
}

/*return  root(new root node), l (node num in less_tree), m (more depth)
 * */
int balance (UPFGLYPHTREE **root, int* l_num, int* m_num)
{
    UPFGLYPHTREE* new_root;
    UPFGLYPHTREE* old_root = *root;
    if ( *root ) {
        int ll_num, lm_num, ml_num, mm_num;

        *l_num = balance (&(*root)->less, &ll_num, &lm_num);
        *m_num = balance (&(*root)->more, &ml_num, &mm_num);

        if ( (*root)->more ) {
            if ( *l_num + ml_num + 1 < mm_num ) {
                /* Shift less-ward */
                new_root = old_root->more;
                APPEND_TO_MORE (old_root, new_root->less);
                APPEND_TO_LESS (new_root, old_root);
                *root = new_root;
            }
        }
        if ( (*root)->less ) {
            if ( *m_num + lm_num + 1 < ll_num ) {
                /* Shift more-ward */
                new_root = old_root->less;
                APPEND_TO_LESS (old_root, new_root->more);
                APPEND_TO_MORE (new_root, old_root);
                *root = new_root;
            }
        }
        return 1 + *l_num + *m_num;
    }
    else {
        *l_num = *m_num = 0;
        return 0;
    }
}
