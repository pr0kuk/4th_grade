#pragma once
#include "nucleusDetail.hpp"
#define MAXINT 1 << 31
#define CALLOCSIZE 1024 ///< nmemb parameter in calloc()
#define OWNSIZE 1024 ///< maximum number of callocs
#define STACKSIZE 1024
#ifdef DEBUG
#define STEP 2 ///< distance between two cells in size of stored data
#endif
#ifndef DEBUG
#define STEP 1 ///< distance between two cells in size of stored data
#endif

extern ErrCode errorCode; ///< Global error variable

/*! struct index stores 3 components of index of data */
struct index
{
    short code1; ///< number of calloc block
    int code2; ///< number of cell in calloc block
    short code3; ///< geneartion
};

/*! struct cell stores pointer */
struct cell
{
    void* pt; ///< pointer to data
    short gen; ///< generation of index
    bool free; ///< status of pointer
};

/*! struct table is used for work with data with one size written to tb.cellsize*/
struct table
{
    struct cell** owner; ///< 2-dimensial array of structs cell (OWNSIZE x CALLOCSIZE)
    short ownersize; ///< current size of
    int cellsize; ///< size of storing data
    const char* const type;
    int freesize; ///< number of free cells
    struct index freestack[STACKSIZE];
};

/*! 
    allocates new cell
    \param[in,out] tb Table with data
    \return index to allocated cell
*/
struct index allocate(struct table* tb)
{
    /* searches for free cell*/
    if (tb->freesize > 0) { // free cell found
        tb->owner[tb->freestack[tb->freesize-1].code1][tb->freestack[tb->freesize-1].code2].free = false;
        return tb->freestack[--tb->freesize];
    }
    /* free cell wasn't found */
    /* all cells are busy -> makes new calloc */
    tb->ownersize += 1;
    char * tt = (char*)(calloc(CALLOCSIZE, tb->cellsize*STEP));
    for (int i = 0; i < CALLOCSIZE; i+=1) {
        tb->owner[tb->ownersize-1][i] = {tt+i*tb->cellsize*STEP,0,true};
        tb->freestack[(tb->freesize)++] = {static_cast<short>(tb->ownersize-1), i, ++tb->owner[tb->ownersize-1][i].gen};
    }
    /* returns first cell of new calloc */
    tb->owner[tb->ownersize-1][0].free = false;
    errorCode = ErrCode::Non;
    return {static_cast<short>(tb->ownersize-1), 0, tb->owner[tb->ownersize-1][0].gen};
}

/*! 
    gets pointer from index
    \param[in,out] tb Table with data
    \param[in] in Index of data looked for
    \return pointer to data
*/
void* get(struct table* tb, struct index* in, const char* const type)
{
    if (type != tb->type){
        errorCode = ErrCode::WrongType;
        return nullptr;
    }
    if (in->code1 >=0 && in->code1 < tb->ownersize && in->code2 >=0 && in->code2 < CALLOCSIZE) { // if index correct
        if (tb->owner[in->code1][in->code2].gen == in->code3 && tb->owner[in->code1][in->code2].free == false) { // if generation is correct and memory is unfreed
            errorCode = ErrCode::Non;
            return tb->owner[in->code1][in->code2].pt;
        }
    }
    errorCode = ErrCode::GetZero;
    return nullptr;
}

/*! 
    frees cell by index
    \param[in,out] tb Table with data
    \param[in] index
    \return error code
*/
ErrCode Free(struct table* tb, struct index* in)
{
    short code1 = in->code1, code3 = in->code3;
    int code2 = in->code2; 
    struct cell it = tb->owner[code1][code2];
    if (code1 >= tb->ownersize || code2 >= CALLOCSIZE) // if index is incorrect
        return errorCode = ErrCode::FreeZero;
    if (it.gen != code3) // if generation is incorrect
        return errorCode = ErrCode::WrongGen;
    if (it.free == true) // if cell was freed
        return errorCode = ErrCode::FreeZero;
    tb->owner[code1][code2].free = true; // frees the cell
    tb->freestack[tb->freesize++] = {code1, code2, ++code3}; // adding cell to free stack
    return errorCode = ErrCode::Non;
}

#ifdef DEBUG
/*! 
    checks if bounds were violated
    \param[in,out] tb Table with data
    \return error code
*/
ErrCode check_bounds (struct table* tb)
{
    /* 
        checks intercellular space
        if there is a byte between cells that is not 0 ->
        it means some data was written out of bounds
    */
    for (int i = 0; i < tb->ownersize; i++) {
        for (int j = tb->cellsize; j < CALLOCSIZE*tb->cellsize; j++) {
            if (*((char*)tb->owner[i][0].pt + j) != (int)(0)) // if byte != 0
                return ErrCode::OutOfBounds;
            if (j % tb->cellsize == tb->cellsize - 1) // we don't want to check the data cells, we looking between them
                j += tb->cellsize;
        }
    }
    return ErrCode::Non;
}
#endif

/*! 
    checks for memory leaks
    \param[in] tb Table with data
    \return index to unfreed memory or {-1, -1, -1}
*/
struct index check_leaks(struct table* tb)
{
    for (short i = 0; i < tb->ownersize; i++) {
        for (int j = 0; j < CALLOCSIZE; j++) {
            if (tb->owner[i][j].free == false) { // if any cell wasn't free -> it means leak
                errorCode = ErrCode::Leaks;
            return {i,j,tb->owner[i][j].gen};}
        }
    }
    errorCode = ErrCode::Non;
    return {-1, -1, -1};
}

/*! 
    frees all pointers in the table
    \param[in,out] tb Table with data
    \return error code
*/
ErrCode destroy(struct table* tb)
{
    for (int i = 0; i < tb->ownersize; i++)
        free(tb->owner[i][0].pt);
    free(tb->owner);
    return ErrCode::Non;
}

/*! 
    initializes owner for table
    \return pointer to owner
*/
struct cell** init_owner()
{
    struct cell** owner = (struct cell**)malloc(OWNSIZE * sizeof(struct cell));
    for (int i = 0; i < OWNSIZE; i++)
    owner[i] = (struct cell*)calloc(CALLOCSIZE, sizeof(struct cell));
    return owner;
}

// struct index[STACKSIZE] init_freestack()
// {
//     struct cell** owner = (struct cell**)malloc(OWNSIZE * sizeof(struct cell));
//     for (int i = 0; i < OWNSIZE; i++)
//     owner[i] = (struct cell*)calloc(CALLOCSIZE, sizeof(struct cell));
//     return owner;
// }

/*! 
    initializes table
    \param[in] cellsize size of data stored
    \return initialized table
*/
struct table init_table(int cellsize, const char* const type)
{
    return {init_owner(), 0, cellsize, type, 0};
}
