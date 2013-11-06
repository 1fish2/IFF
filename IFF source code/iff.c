/* @(#) iff.c 96/04/29 1.21 */

#include <kernel/types.h>
#include <kernel/mem.h>
#include <kernel/tags.h>
#include <misc/iff.h>
#include "io.h"


/*****************************************************************************/


#ifdef NO_64_BIT_SCALARS
#define int64  int32
#define uint64 uint32
#endif


/*****************************************************************************/


#define IsIFFParser(iff) ((iff && (iff->iff_Cookie == iff)) ? TRUE : FALSE)
#define IsGenericID(id)  ((id) == ID_FORM || (id) == ID_CAT || (id) == ID_LIST || (id) == ID_PROP)
#define IsContainerID(id) ((id & '\0\0\0{') == '\0\0\0{')
#define	ID_PAD	          MAKE_ID('!', 'P','A','D')


/*****************************************************************************/


/* Data attached to a ContextInfo structure which contains a handler function
 * to be applied when a chunk is pushed/popped.
 */
typedef struct ChunkHandler
{
    IFFCallBack	 ch_HandlerCB;
    const void  *ch_UserData;
} ChunkHandler;


/*****************************************************************************/


/* Data attached to a ContextInfo structure which contains part of
 * the linked list of collection properties - the ones for this
 * context. Only the ones for this context get purged when this
 * item gets purged. First is the first element for this context
 * level, LastPtr is the value of the Next pointer for the last
 * element at this level.
 */
typedef struct CollectionList
{
    CollectionChunk  *cl_First;
    CollectionChunk  *cl_LastPtr;
    ContextNode      *cl_Context;
} CollectionList;


/*****************************************************************************/


typedef struct Chunk
{
    PackedID ID;
    uint32   Filler;
    uint64   Size;
} Chunk;

typedef struct SmallChunk
{
    PackedID ID;
    uint32   Size;
} SmallChunk;


/*****************************************************************************/


static int32 internalWriteChunk(IFFParser *iff, ContextNode *top,
                                const void *buf, uint32 numBytes);


/*****************************************************************************/


/* Characters in [0x20 - 0x7e], no leading spaces (except for ID_NULL) */
static bool GoodID(PackedID id)
{
char  *ptr;
uint32 i;

    ptr = (char *)&id;
    if (ptr[0] == ' ')
    {
        if (id == ID_NULL)
            return TRUE;

        /* No leading spaces, except for NULL identifier */
        return FALSE;
    }

    for (i = 0; i < sizeof(id); i++)
    {
        if ((ptr[i] < ' ') || (ptr[i] > '~'))
            return FALSE;
    }

    return TRUE;
}


/*****************************************************************************/


/* No lower case or punctuation allowed */
static bool GoodType(PackedID type)
{
char   *ptr;
uint32  i;

    if (!GoodID(type))
        return (FALSE);

    ptr = (char *)&type;
    for (i = 0; i < sizeof(type); i++)
    {
        if (((ptr[i] < 'A') || (ptr[i] > 'Z'))
         && ((ptr[i] < '0') || (ptr[i] > '9'))
         && (ptr[i] != ' '))
        {
            return FALSE;
        }
    }

    return TRUE;
}


/*****************************************************************************/


/* Function reads id and size and pushes a new context for this chunk
 * on to the context stack, initializing byte count to zero.
 * Assumes that file is positioned at the start of a chunk.
 */
static Err PushChunkR(IFFParser *iff, bool firstChunk)
{
ContextNode *top, *cn;
PackedID     type;
Err          err;
Chunk        chunk;
SmallChunk   smallChunk;

    top = GetCurrentContext(iff);
    if (!top && !firstChunk)
        return IFF_PARSE_EOF;

    type = (top ? top->cn_Type : 0);

    if (firstChunk)
    {
        err = IFFRead(iff, &smallChunk, sizeof(smallChunk));
        if (err != sizeof(smallChunk))
        {
            if (err >= 0)
                err = IFF_ERR_NOTIFF;

            return err;
        }

        chunk.ID   = smallChunk.ID;
        chunk.Size = smallChunk.Size;

        if (chunk.Size == IFF_SIZE_64BIT)
        {
            err = IFFRead(iff, &chunk.Size, sizeof(chunk.Size));
            if (err != sizeof(chunk.Size))
            {
                if (err >= 0)
                    err = IFF_ERR_NOTIFF;

                return err;
            }
        }
        else if (chunk.Size >= IFF_SIZE_RESERVED)
        {
            return IFF_ERR_MANGLED;
        }
    }
    else
    {
        /* Using ReadChunk() causes these bytes to go into the
         * parent's scan count
         */
        err = ReadChunk(iff, &smallChunk, sizeof(smallChunk));
        if (err != sizeof(smallChunk))
        {
            if (err >= 0)
                err = IFF_ERR_PREMATURE_EOF;

            return err;
        }

        chunk.ID   = smallChunk.ID;
        chunk.Size = smallChunk.Size;

        if (chunk.Size == IFF_SIZE_64BIT)
        {
            err = ReadChunk(iff, &chunk.Size, sizeof(chunk.Size));
            if (err != sizeof(chunk.Size))
            {
                if (err >= 0)
                    err = IFF_ERR_PREMATURE_EOF;

                return err;
            }
        }
        else if (chunk.Size >= IFF_SIZE_RESERVED)
        {
            return IFF_ERR_MANGLED;
        }

        if (chunk.Size > top->cn_Size - top->cn_Offset)
            return IFF_ERR_MANGLED;
    }

    if (GoodID(chunk.ID))
    {
        if (cn = AllocMem(sizeof(ContextNode), MEMTYPE_ANY))
        {
            cn->cn_Type        = type;
            cn->cn_ID          = chunk.ID;
            cn->cn_Size        = chunk.Size;
            cn->cn_Offset      = 0;
            cn->cn_CurrentSize = chunk.Size;
            PrepList(&cn->cn_ContextInfo);
            AddHead(&iff->iff_Stack, (Node *)cn);

            return 0;
        }
        return IFF_ERR_NOMEM;
    }

    if (iff->iff_NewIO)
        return IFF_ERR_NOTIFF;

    return IFF_ERR_SYNTAX;
}


/*****************************************************************************/


static Err PurgeContextInfo(IFFParser *iff, ContextInfo *ci)
{
IFFCallBack cb;

    if (cb = ci->ci_PurgeCB)
        return (* cb)(iff, ci);

    FreeContextInfo(ci);
    return 0;
}


/*****************************************************************************/


/* Free memory associated with a ContextNode, calling its optional free vector
 * for any ContextInfo structures it contains.
 */
static void FreeContextNode(IFFParser *iff, ContextNode *cn)
{
ContextInfo *ci;

    RemNode((Node *)cn);

    /* Dispose of all local context items for this context node.
     * error return value is ignored for now.
     */
    while (ci = (ContextInfo *)RemHead(&cn->cn_ContextInfo))
    {
        ci->ci.n_Next = NULL;
        PurgeContextInfo(iff, ci);
    }

    FreeMem(cn, sizeof(ContextNode));
}


/*****************************************************************************/


/* Pops the top chunk off the stack and skips past any unread bytes in the
 * chunk. Updates the new top chunk with bytes read and purges local context.
 */
static Err PopChunkR(IFFParser *iff)
{
ContextNode *top;
Err          err;
uint64       rsize;

    /* Get top chunk and seek past anything un-scanned including
     * the optional pad byte.
     */
    if (!(top = GetCurrentContext(iff)))
        return IFF_PARSE_EOF;

    rsize = (top->cn_Size + 1) & ~1;

    if (top->cn_Offset < rsize)
    {
        err = IFFSeek(iff, rsize - top->cn_Offset);
        if (err < 0)
            return err;
    }

    /* Remove and deallocate this chunk and any stored properties */
    FreeContextNode(iff, top);

    /* Update the new top chunk, if any. */
    if (top = GetCurrentContext(iff))
        top->cn_Offset += rsize;

    return 0;
}


/*****************************************************************************/


int32 ReadChunk(IFFParser *iff, void *buf, uint32 numBytes)
{
ContextNode *top;
uint32       maxBytes;
int32        result;

    if (!(top = GetCurrentContext(iff)))
        return IFF_PARSE_EOF;

    maxBytes = top->cn_Size - top->cn_Offset;

    if (numBytes > maxBytes)
        numBytes = maxBytes;

    result = IFFRead(iff, buf, numBytes);
    if (result != numBytes)
    {
        if (result >= 0)
        {
            /* less data in the file than the header says there is */
            result = IFF_ERR_PREMATURE_EOF;
        }
    }
    else
    {
        top->cn_Offset += (uint32)result;
    }

    return result;
}


/*****************************************************************************/


int32 ReadChunkCompressed(IFFParser *iff, void *buf, uint32 numBytes)
{
    TOUCH(iff);
    TOUCH(buf);
    TOUCH(numBytes);
    return IFF_ERR_NOTSUPPORTED;
}


/*****************************************************************************/


Err SeekChunk(IFFParser *iff, int32 position, IFFSeekModes mode)
{
ContextNode *top;
uint64       base;
uint64       newOffset;
uint64       pos;

    if (!(top = GetCurrentContext(iff)))
        return IFF_PARSE_EOF;

    /* pick offset base depending on mode */
    switch (mode)
    {
        case IFF_SEEK_START:   base = 0;              break;
        case IFF_SEEK_CURRENT: base = top->cn_Offset; break;
        case IFF_SEEK_END:     base = top->cn_Size;   break;
        default:               return IFF_ERR_BADMODE;
    }

    /* Clip position in range of -base..cn_Size-base. Result in newOffset
     * is in range of 0..cn_Size
     */
    if (position >= 0)
    {
        pos = position;
        if (pos < top->cn_Size - base)
            newOffset = base + pos;
        else
            newOffset = top->cn_Size;
    }
    else
    {
        pos = -position;
        if (pos < base)
            newOffset = base - pos;
        else
            newOffset = 0;
    }

    /* seek on stream */
    {
        const int64 seekOffset = (int32)(newOffset - top->cn_Offset);
        int32 oldStreamPos, newStreamPos;

        /* seek relative to current file position, and see if we got there */
        if ((oldStreamPos = IFFSeek(iff,seekOffset)) < 0) return oldStreamPos;
        if ((newStreamPos = IFFSeek(iff,0)) < 0) return newStreamPos;
        if (newStreamPos - oldStreamPos != seekOffset) return IFF_ERR_PREMATURE_EOF;
    }

    top->cn_Offset = newOffset;
    return 0;
}


/*****************************************************************************/


int64 GetIFFOffset(IFFParser *iff)
{
    return (int64)IFFSeek(iff,0);
}


/*****************************************************************************/


/* Push a chunk onto the IFF context stack for write.
 * Type must be specified for generic chunks.
 */
static Err PushChunkW(IFFParser *iff, PackedID type, PackedID id, uint64 size)
{
ContextNode *cn;
Err          err;
PackedID     parentType;
Chunk        chunk;
SmallChunk   smallChunk;
bool         firstChunk;

    if (cn = GetCurrentContext(iff))
    {
        parentType = cn->cn_Type;
        firstChunk = FALSE;
    }
    else if (iff->iff_NewIO)
    {
        parentType = 0;
	firstChunk = TRUE;
	iff->iff_NewIO = FALSE;

        /* first chunk must be FORM, CAT, or LIST */
        if ((id != ID_FORM)  && (id != ID_CAT) && (id != ID_LIST))
            return IFF_ERR_NOTIFF;
    }
    else
    {
        return IFF_PARSE_EOF;
    }

    if (!GoodID(id))
        return IFF_ERR_SYNTAX;

    if (IsGenericID(id))
    {
        if (id == ID_PROP)
        {
            /* the containing context for PROP must be a LIST */
            if (cn->cn_ID != ID_LIST)
                return IFF_ERR_SYNTAX;
        }

        /* Generic ID.  Check the validity of its subtype. */
        if (!GoodType(type))
            return IFF_ERR_NOTIFF;
    }
    else
    {
        /* Non-generic ID.  Containing context must be PROP or FORM or container */
        if ((cn->cn_ID != ID_FORM)
         && (cn->cn_ID != ID_PROP)
         && (!IsContainerID(cn->cn_ID)))
            return IFF_ERR_SYNTAX;
    }

    /* Write the chunk header: ID and size (if IFF_SIZE_UNKNOWN, it will
     * be patched later by PopChunkW()).
     */
    if ((size >= IFF_SIZE_RESERVED) && (size != IFF_SIZE_UNKNOWN_32))
    {
        chunk.ID     = id;
        chunk.Filler = IFF_SIZE_64BIT;
        chunk.Size   = size;
        if (firstChunk)
        {
            err = IFFWrite(iff, &chunk, sizeof(Chunk));
        }
        else
        {
            /* Update parent's count of bytes written. */
            err = internalWriteChunk(iff, cn, &chunk, sizeof(Chunk));
        }
    }
    else
    {
        smallChunk.ID   = id;
        smallChunk.Size = size;
        if (firstChunk)
        {
            err = IFFWrite(iff, &smallChunk, sizeof(smallChunk));
        }
        else
        {
            /* Update parent's count of bytes written. */
            err = internalWriteChunk(iff, cn, &smallChunk, sizeof(smallChunk));
        }
    }

    if (err < 0)
        return err;

    /* Allocate and fill in a ContextNode for the new chunk */
    if (!(cn = AllocMem(sizeof(ContextNode), MEMTYPE_ANY)))
        return IFF_ERR_NOMEM;

    cn->cn_ID          = id;
    cn->cn_Size        = size;
    cn->cn_Offset      = 0;
    cn->cn_CurrentSize = 0;
    PrepList(&cn->cn_ContextInfo);
    AddHead(&iff->iff_Stack, (Node *)cn);

    /* For generic chunks, write the type out now that
     * the chunk context node is initialized.
     */
    if (IsGenericID(id) || IsContainerID(id))
    {
        if (IsGenericID(id))
        {
            err = internalWriteChunk(iff, cn, &type, sizeof(type));
            cn->cn_Type = type;
        }
        else
        {
            cn->cn_Type = 0;   /* denote a Container */
        }
    }
    else
    {
        cn->cn_Type = parentType;
    }

    return err;
}


/*****************************************************************************/


/* Pop a chunk off the context stack. Ends the chunk and sets the size (if it
 * has not already been set) by seeking back and fixing it up.  New top chunk
 * gets size updated.
 */
static Err PopChunkW(IFFParser *iff)
{
ContextNode *top;
int64        rsize;
Err          err;
uint64       size;
int8         pad;
bool         unknownSize;
bool         bit64;
uint32       size32;
uint8        padBuf[2];
uint32       alignment, padding, extra;

    if (!(top = GetCurrentContext(iff)))
        return IFF_PARSE_EOF;

    if (top->cn_Size == IFF_SIZE_UNKNOWN_32)
    {
        /* Size is unknown.  The size is therefore however many bytes the
         * client wrote.
         */
        size        = top->cn_CurrentSize;
        bit64       = FALSE;
        unknownSize = TRUE;
    }
    else if (top->cn_Size == IFF_SIZE_UNKNOWN_64)
    {
        /* Size is unknown.  The size is therefore however many bytes the
         * client wrote.
         */
        size        = top->cn_CurrentSize;
        bit64       = TRUE;
        unknownSize = TRUE;
    }
    else
    {
	/* Size is known.  Compare what the client said against what actually
         * happened.
         */
        size = top->cn_Size;
        if (size != top->cn_CurrentSize)
            return IFF_ERR_MANGLED;

        bit64       = FALSE;
        unknownSize = FALSE;
    }

    /* if we're not at the end of the chunk, go there */
    if (top->cn_Offset != top->cn_CurrentSize)
    {
        err = IFFSeek(iff, top->cn_CurrentSize - top->cn_Offset);
        if (err < 0)
            return err;
    }

    /* Add a possible pad byte */
    rsize = (size + 1) & ~1;
    if (rsize > size)
    {
        pad = 0;
        err = IFFWrite(iff, &pad, 1);
        if (err < 0)
            return err;
    }

    /* Remove and deallocate this chunk and any stored properties */
    FreeContextNode(iff, top);

    /* If needed, seek back and fix the chunk size */
    if (unknownSize)
    {
        if (bit64)
        {
            err = IFFSeek(iff, -(rsize + sizeof(size)));
            if (err >= 0)
                err = IFFWrite(iff, &size, sizeof(size));
        }
        else
        {
            if (size >= IFF_SIZE_RESERVED)
                return IFF_ERR_TOOBIG;

            size32 = size;

            err = IFFSeek(iff, -(rsize + sizeof(size32)));
            if (err >= 0)
                err = IFFWrite(iff, &size32, sizeof(size32));
        }

        if (err >= 0)
            err = IFFSeek(iff, rsize);

        if (err < 0)
            return err;
    }

    if (top = GetCurrentContext(iff))
    {
        /* Update parent's count */
        top->cn_Offset      += rsize;
        top->cn_CurrentSize += rsize;
        if ((top->cn_Size != IFF_SIZE_UNKNOWN_32)
         && (top->cn_Size != IFF_SIZE_UNKNOWN_64)
         && (top->cn_Offset > top->cn_Size))
            return IFF_ERR_MANGLED;
    }

    alignment = iff->iff_Alignment;
    if (padding = (rsize % alignment))
    {
        if (8 % alignment)
	{
            padding = alignment - ((8 + padding) % alignment);
	}
        else
        {
            padding = alignment - padding;
        }

        /* Update parent's count */
        top->cn_Offset += padding+8;
        top->cn_CurrentSize += padding+8;
        if ((top->cn_Size != IFF_SIZE_UNKNOWN_32)
	  && (top->cn_Size != IFF_SIZE_UNKNOWN_64)
          && (top->cn_Offset > top->cn_Size))
        {
            return IFF_ERR_MANGLED;
        }

        extra = ID_PAD;
        err = IFFWrite(iff, &extra, 4);
        if (err < 0)
            return err;

        err = IFFWrite(iff, &padding, 4);
        if (err < 0)
            return err;

        padBuf[0] = (uint8)(alignment >> 8);
        padBuf[1] = (uint8)(alignment & 0x00FF);

        err = IFFWrite(iff, padBuf, 2);
        if (err < 0)
            return err;

        padding -= 2;

        padBuf[0] = padBuf[1] = 0;
        while (padding > 0)
	{
            err = IFFWrite(iff, padBuf, 2);
            if (err < 0)
                return err;

            padding -= 2;
	}
    }

    return 0;
}


/*****************************************************************************/


static int32 internalWriteChunk(IFFParser *iff, ContextNode *top,
                                const void *buf, uint32 numBytes)
{
uint32 maxBytes;
int32  result;

    if ((top->cn_Size != IFF_SIZE_UNKNOWN_32)
     && (top->cn_Size != IFF_SIZE_UNKNOWN_64))
    {
        maxBytes = top->cn_Size - top->cn_Offset;

	if (numBytes > maxBytes)
            numBytes = maxBytes;
    }

    if (!numBytes)
        return 0;

    result = IFFWrite(iff, buf, numBytes);
    if (result >= 0)
    {
        top->cn_Offset += (uint32)result;

        if (top->cn_Offset > top->cn_CurrentSize)
            top->cn_CurrentSize = top->cn_Offset;
    }

    return result;
}


/*****************************************************************************/


int32 WriteChunk(IFFParser	*iff, const void *buf, uint32 numBytes)
{
ContextNode *top;

    if (!(top = GetCurrentContext(iff)))
	return IFF_PARSE_EOF;

    if (IsGenericID(top->cn_ID))
    {
        /* can't write into a generic chunk, must have a non-generic chunk there */
        return IFF_ERR_NOTIFF;
    }

    return internalWriteChunk(iff, top, buf, numBytes);
}


/*****************************************************************************/


int32 WriteChunkCompressed(IFFParser *iff, const void *buf, uint32 numBytes)
{
    TOUCH(iff);
    TOUCH(buf);
    TOUCH(numBytes);

    return IFF_ERR_NOTSUPPORTED;
}


/*****************************************************************************/


/* Purge a collection list node and all collection items within its
 * scope (given by First, LastPtr).
 */
static int32 PurgeCollectionList(IFFParser *iff, ContextInfo *ci)
{
CollectionList	*cl;
CollectionChunk	*cc, *nextcc;

    TOUCH(iff);

    cl = (CollectionList *)ci->ci_Data;

    for (cc = cl->cl_First; (cc != cl->cl_LastPtr) && cc; cc = nextcc)
    {
        nextcc = cc->cc_Next;
        FreeMem(cc, sizeof(CollectionChunk) + cc->cc_DataSize);
    }
    FreeContextInfo(ci);

    return 0;
}


/*****************************************************************************/


/* Store contents of a chunk in a buffer and return 0 or error code */
static Err BufferChunk(IFFParser *iff, uint64 size, void *buf)
{
Err result;

    result = ReadChunk(iff, buf, size);
    if (result != size)
    {
        if (result >= 0)
            result = IFF_ERR_PREMATURE_EOF;

        return result;
    }
    return 0;
}


/*****************************************************************************/


/* Encountered a collection property chunk.  Store this at the head of
 * the appropriate list.  If no CollectionList at this level, create one.
 */
static Err HandleCollectionChunk(IFFParser *iff, void *dummy)
{
CollectionList  *cltop;
CollectionChunk *cc;
uint64           size;
Err              err;
CollectionList  *clast;
ContextNode     *cn;
ContextNode     *ctxt;
ContextInfo     *ci;

    TOUCH(dummy);

    /* read the chunk into a buffer and create a CollectionChunk for it */

    cn = GetCurrentContext(iff);
    size = cn->cn_Size;
    if (cc = AllocMem(sizeof(CollectionChunk) + size, MEMTYPE_ANY))
    {
        cc->cc_Container = FindPropContext(iff);
        cc->cc_Data      = &cc[1];
        cc->cc_DataSize  = size;

        err = BufferChunk(iff, cc->cc_DataSize, cc->cc_Data);
        if (err >= 0)
        {
            /* locate current PROP context */
            if (ctxt = cc->cc_Container)
            {
                /* find the top collection list node for this context. */

                ci = FindContextInfo(iff, cn->cn_Type, cn->cn_ID, IFF_CI_COLLECTIONCHUNK);
                if (ci)
                    clast = (CollectionList *)ci->ci_Data;
                else
                    clast = NULL;

                /* If this list item is at the same level as the current chunk,
                 * this is the node for the current chunk and the new buffer
                 * just needs to be linked in.  If this is at a different
                 * level, or there is none at all, a new cl node for this
                 * level needs to be created.
                 */
                if (clast && (clast->cl_Context == ctxt))
                {
                    cltop = clast;
                }
                else
                {
                    if (ci = AllocContextInfo(cn->cn_Type, cn->cn_ID, IFF_CI_COLLECTIONCHUNK,
                                              sizeof(CollectionList), (IFFCallBack)PurgeCollectionList))
                    {
                        cltop             = (CollectionList *)ci->ci_Data;
                        cltop->cl_Context = ctxt;
                        cltop->cl_LastPtr = NULL;

                        if (clast)
                            cltop->cl_LastPtr = clast->cl_First;

                        cltop->cl_First = cltop->cl_LastPtr;

                        err = StoreContextInfo(iff, ci, IFF_CIL_PROP);
                        if (err < 0)
                            FreeContextInfo(ci);
                    }
                    else
                    {
                        err = IFF_ERR_NOMEM;
                        cltop = NULL;
                    }
                }

                if (err >= 0)
                {
                    /* Link new CollectionChunk at head of list. */
                    cc->cc_Next = cltop->cl_First;
                    cltop->cl_First = cc;

                    /* continue parsing */
                    return 1;
                }
            }
            else
            {
                err = IFF_ERR_NOSCOPE;
            }
        }
        FreeMem(cc, sizeof(CollectionChunk) + size);
    }
    else
    {
        err = IFF_ERR_NOMEM;
    }

    return err;
}


/*****************************************************************************/


static Err HandleStopChunk(const IFFParser *iff, void *dummy)
{
    TOUCH(iff);
    TOUCH(dummy);

    /* stop parsing and return to client */
    return 0;
}


/*****************************************************************************/


static Err HandlePropertyChunk(IFFParser *iff, void *dummy)
{
ContextInfo *ci;
PropChunk   *pc;
ContextNode *cn;
Err          err;

    TOUCH(dummy);

    cn = GetCurrentContext(iff);

    ci = AllocContextInfo(cn->cn_Type, cn->cn_ID, IFF_CI_PROPCHUNK,
                          cn->cn_Size + sizeof(PropChunk), NULL);
    if (ci)
    {
	pc              = (PropChunk *)ci->ci_Data;
	pc->pc_DataSize = cn->cn_Size;
	pc->pc_Data     = &pc[1];

	/* Store contents of property chunk in current context */
	err = BufferChunk(iff, cn->cn_Size, pc->pc_Data);
	if (err >= 0)
        {
            err = StoreContextInfo(iff, ci, IFF_CIL_PROP);
            if (err >= 0)
            {
                /* continue parsing */
                return 1;
            }
        }
        FreeContextInfo(ci);
    }
    else
    {
        err = IFF_ERR_NOMEM;
    }

    return err;
}


/*****************************************************************************/


/* Install a handler node into the current context */
static Err InstallHandler(IFFParser *iff, uint32 type, uint32 id, uint32 ident,
                          ContextInfoLocation pos, IFFCallBack cb,
                          const void *userData)
{
ContextInfo  *ci;
ChunkHandler *ch;
Err	      err;

    if (ci = AllocContextInfo(type, id, ident, sizeof(ChunkHandler), NULL))
    {
        ch               = (ChunkHandler *)ci->ci_Data;
        ch->ch_HandlerCB = cb;
        ch->ch_UserData  = userData;

        err = StoreContextInfo(iff, ci, pos);
        if (err < 0)
            FreeContextInfo(ci);

        return err;
    }
    return IFF_ERR_NOMEM;
}


/*****************************************************************************/


/* Read and test generic type ID */
static Err ReadGenericType(IFFParser *iff)
{
ContextNode *top;
Err          result;

    if (top = GetCurrentContext(iff))
    {
        result = ReadChunk(iff, &top->cn_Type, sizeof(PackedID));
        if (result == sizeof(PackedID))
        {
            if (GoodType(top->cn_Type))
                return 0;

            return IFF_ERR_MANGLED;
        }

        if (result >= 0)
            result = IFF_ERR_PREMATURE_EOF;

        return result;
    }

    return IFF_PARSE_EOF;
}


/*****************************************************************************/


Err CreateIFFParser(IFFParser **iffp, bool writeMode, const TagArg tags[])
{
IFFParser   *iff;
Err          result;
TagArg      *tag;
void        *openKey;
IFFIOFuncs  *ioFuncs;

    result = IFF_ERR_NOMEM;

    *iffp = NULL;

    openKey = NULL;
    ioFuncs = NULL;
    while ((tag = NextTagArg(&tags)) != NULL)
    {
        switch (tag->ta_Tag)
        {
            case IFF_TAG_FILE        : openKey = tag->ta_Arg;
                                       ioFuncs = (IFFIOFuncs *)&fileFuncs;
                                       break;

            case IFF_TAG_IOFUNCS     : ioFuncs = (IFFIOFuncs *)tag->ta_Arg;
                                       break;

            case IFF_TAG_IOFUNCS_DATA: openKey = (void *)tag->ta_Arg;
                                       break;

            default                  : return IFF_ERR_BADTAG;
        }
    }

    if (ioFuncs == NULL)
        return IFF_ERR_NOOPENTYPE;

    iff = AllocMem(sizeof(IFFParser), MEMTYPE_FILL);
    if (iff)
    {
        PrepList(&iff->iff_Stack);
        iff->iff_NewIO     = TRUE;
        iff->iff_WriteMode = writeMode;
        iff->iff_IOFuncs   = ioFuncs;
	iff->iff_Alignment = IFF_DEFAULT_ALIGNMENT;

        /* init and attach a surrounding master context */
        PrepList(&iff->iff_TopContext.cn_ContextInfo);
        AddHead(&iff->iff_Stack, (Node *)&iff->iff_TopContext);

        result = IFFOpen(iff, openKey, writeMode);
        if (result >= 0)
        {
            iff->iff_Cookie = iff;
            *iffp = iff;

            return result;
        }
        FreeMem(iff,sizeof(IFFParser));
    }

    return result;
}


/*****************************************************************************/


Err DeleteIFFParser(IFFParser *iff)
{
ContextNode *top;
ContextInfo *ci;
Err          result;

    if (!IsIFFParser(iff))
        return IFF_ERR_BADPTR;

    result = 0;
    while (GetCurrentContext(iff))
    {
        result = PopChunk(iff);
        if (result < 0)
        {
            while (top = GetCurrentContext(iff))
            {
                FreeContextNode(iff, top);
            }
        }
    }

    /* also do the master context */
    while (ci = (ContextInfo *)RemHead(&iff->iff_TopContext.cn_ContextInfo))
    {
        ci->ci.n_Next = NULL;
        PurgeContextInfo(iff, ci);
    }

    if (result >= 0)
        result = IFFClose(iff);
    else
        IFFClose(iff);

    iff->iff_Cookie = NULL;
    FreeMem(iff, sizeof(IFFParser));

    return result;
}


/*****************************************************************************/


/* Test current state and advance to the next state. State is one of:
 *
 *  - Start of file if iff_NewIO is TRUE.
 *    Push initial chunk, get its info and return.
 *
 *  - Poised at end of chunk if iff_Paused is TRUE.
 *    PopChunk() the current context. The top context will then be a generic
 *    or end of file.  Return EOF if the latter. If this chunk is exhausted,
 *    pause again and return, otherwise push new chunk and return.
 *
 *  - Inside a generic chunk.
 *    Push a new context and return.
 *
 *  - Inside a non-generic chunk.
 *    Pause and return.
 */
static Err NextState(IFFParser *iff)
{
ContextNode *top;
Err          err;
PackedID     topid;

    /* deal with the case of the first chunk */
    if (iff->iff_NewIO)
    {
        err = PushChunkR(iff, TRUE);
        iff->iff_NewIO  = FALSE;
        iff->iff_Paused = FALSE;
        if (err < 0)
            return err;

        top = GetCurrentContext(iff);

        if ((top->cn_ID != ID_FORM)
         && (top->cn_ID != ID_CAT)
	 && (top->cn_ID != ID_LIST)
         && (!IsContainerID(top->cn_ID)))
            return IFF_ERR_NOTIFF;

        if (top->cn_Size & 1)
        {
            /* containers must inherently be word-aligned */
            return IFF_ERR_MANGLED;
        }

        return ReadGenericType(iff);
    }

    /* In the PAUSE state, do the deferred PopChunk() */
    if (iff->iff_Paused)
    {
        err = PopChunk(iff);
        if (err < 0)
            return err;

        iff->iff_Paused = FALSE;
    }

    /* If no top context node then the file is exhausted. */
    if (!(top = GetCurrentContext(iff)))
        return IFF_PARSE_EOF;

    topid = top->cn_ID;

    if (IsGenericID(topid) || IsContainerID(topid))
    {
        /* If inside a generic chunk, and not exhausted, push a subchunk. */
        if (top->cn_Offset < top->cn_Size)
        {
            err = PushChunkR(iff, FALSE);
            if (err < 0)
                return err;

            top = GetCurrentContext(iff);

            /* If non-generic, we're done, but if the containing chunk is not
             * FORM or PROP, it's an IFF syntax error.
             */
            if (!IsGenericID(top->cn_ID))
            {
                if ((topid != ID_FORM)
                 && (topid != ID_PROP)
		 && (!IsContainerID(topid)))
                    return IFF_ERR_SYNTAX;

                return 0;
            }

            /* If this new chunk is generic, and is a PROP, test to see if it's
             * in the right place --  containing chunk should be a LIST.
             */
            if ((top->cn_ID == ID_PROP) && (topid != ID_LIST))
                return IFF_ERR_SYNTAX;

            /* since it's an ok generic, get its type and return */
            return ReadGenericType(iff);
        }
        else if (top->cn_Offset != top->cn_Size)
        {
            /* If not exhaused, this is a junky IFF file */
            return IFF_ERR_MANGLED;
        }
    }

    /* If the generic is exhausted, or this is a non-generic chunk,
     * enter the pause state and return flag.
     */
    iff->iff_Paused = TRUE;

    return IFF_PARSE_EOC;
}


/*****************************************************************************/


/* Driver for the parse engine. Loops calling NextState() and acting
 * on the next parse state. NextState() will always cause the parser
 * to enter or exit (really pause before exiting) a chunk. In each case,
 * either an entry handler or an exit handler will be invoked on the
 * current context. A handler can cause the parser to return control
 * to the calling program or it can return an error.
 */
Err ParseIFF(IFFParser *iff, ParseIFFModes control)
{
ContextNode   *top;
ContextInfo   *ci;
ChunkHandler  *ch;
Err            err;
Err            eoc;

    if (!IsIFFParser(iff))
        return IFF_ERR_BADPTR;

    if ((control < 0) || (control > IFF_PARSE_RAWSTEP))
        return IFF_ERR_BADMODE;

    while (TRUE)
    {
        /* advance to next state and store end of chunk info. */
        eoc = NextState(iff);
        if ((eoc < 0) && (eoc != IFF_PARSE_EOC))
            return eoc;

        /* user requesting manual override -- return immediately */
        if (control == IFF_PARSE_RAWSTEP)
            return eoc;

        if (!(top = GetCurrentContext(iff)))
            return IFF_PARSE_EOF;

        /* find chunk handlers for entering or exiting a context. */
        if (eoc == IFF_PARSE_EOC)
            ci = FindContextInfo(iff, top->cn_Type, top->cn_ID, IFF_CI_EXITHANDLER);
        else
            ci = FindContextInfo(iff, top->cn_Type, top->cn_ID, IFF_CI_ENTRYHANDLER);

        if (ci)
        {
            ch = (ChunkHandler *)ci->ci_Data;
            err = (* ch->ch_HandlerCB)(iff, (void *)ch->ch_UserData);
            if (err != 1)
                return err;
        }

        if (control == IFF_PARSE_STEP)
            return eoc;
    }
}


/*****************************************************************************/


Err PushChunk(IFFParser *iff, PackedID type, PackedID id, uint32 size)
{
    if (iff->iff_WriteMode)
	return PushChunkW(iff, type, id, size);

    return PushChunkR(iff, FALSE);
}


/*****************************************************************************/


Err PopChunk(IFFParser *iff)
{
    if (iff->iff_WriteMode)
	return PopChunkW(iff);

    return PopChunkR(iff);
}


/*****************************************************************************/


Err InstallEntryHandler(IFFParser *iff, PackedID type, PackedID id,
                        ContextInfoLocation pos, IFFCallBack cb, const void *userData)
{
    return InstallHandler(iff, type, id, IFF_CI_ENTRYHANDLER, pos, cb, userData);
}


/*****************************************************************************/


Err InstallExitHandler(IFFParser *iff, PackedID type, PackedID id,
                       ContextInfoLocation pos, IFFCallBack cb, const void *userData)
{
    return InstallHandler(iff, type, id, IFF_CI_EXITHANDLER, pos, cb, userData);
}


/*****************************************************************************/


Err RegisterPropChunks(IFFParser *iff, const IFFTypeID typeids[])
{
Err result;

    while (typeids->Type)
    {
        result = InstallEntryHandler(iff, typeids->Type, typeids->ID, IFF_CIL_TOP,
                                     HandlePropertyChunk, NULL);
        if (result < 0)
            return result;

        typeids++;
    }

    return 0;
}


/*****************************************************************************/


Err RegisterStopChunks(IFFParser *iff, const IFFTypeID typeids[])
{
Err result;

    while (typeids->Type)
    {
        result = InstallEntryHandler(iff, typeids->Type, typeids->ID, IFF_CIL_TOP,
                                     (IFFCallBack)HandleStopChunk, NULL);
        if (result < 0)
            return result;

        typeids++;
    }

    return 0;
}


/*****************************************************************************/


Err RegisterCollectionChunks(IFFParser *iff, const IFFTypeID typeids[])
{
Err result;

    while (typeids->Type)
    {
        result = InstallEntryHandler(iff, typeids->Type, typeids->ID, IFF_CIL_TOP,
                                     HandleCollectionChunk, NULL);
        if (result < 0)
            return result;

        typeids++;
    }

    return 0;
}


/*****************************************************************************/


PropChunk *FindPropChunk(const IFFParser *iff, PackedID type, PackedID id)
{
ContextInfo *ci;

    ci = FindContextInfo(iff, type, id, IFF_CI_PROPCHUNK);
    if (ci)
        return (PropChunk *)ci->ci_Data;

    return NULL;
}


/*****************************************************************************/


CollectionChunk *FindCollection(const IFFParser *iff, PackedID type, PackedID id)
{
CollectionList *cl;
ContextInfo    *ci;

    ci = FindContextInfo(iff, type, id, IFF_CI_COLLECTIONCHUNK);
    if (ci)
    {
        cl = (CollectionList *)ci->ci_Data;
        if (cl)
            return cl->cl_First;
    }

    return NULL;
}


/*****************************************************************************/


void AttachContextInfo(IFFParser *iff, ContextNode *to, ContextInfo *new)
{
ContextInfo *ci;
PackedID     ident, id, type;

    ident = new->ci_Ident;
    id    = new->ci_ID;
    type  = new->ci_Type;

    /* nuke duplicate context data */
    ScanList(&to->cn_ContextInfo,ci,ContextInfo)
    {
        if ((ci->ci_Ident == ident)
         && (ci->ci_ID == id)
         && (ci->ci_Type == type))
        {
            RemNode((Node *)ci);
            ci->ci.n_Next = NULL;
            PurgeContextInfo(iff, ci);
            break;
        }
    }

    AddHead(&to->cn_ContextInfo, (Node *)new);
}


/*****************************************************************************/


Err StoreContextInfo(IFFParser *iff, ContextInfo *ci, ContextInfoLocation pos)
{
ContextNode *cn;

    switch (pos)
    {
	case IFF_CIL_BOTTOM: cn = (ContextNode *)LastNode(&iff->iff_Stack);
                             break;

	case IFF_CIL_TOP   : cn = (ContextNode *)FirstNode(&iff->iff_Stack);
                             break;

	case IFF_CIL_PROP  : if (cn = FindPropContext(iff))
                                 break;

                             return IFF_ERR_NOSCOPE;

        default            : return IFF_ERR_BADLOCATION;
    }

    AttachContextInfo(iff, cn, ci);

    return 0;
}


/*****************************************************************************/


void RemoveContextInfo(ContextInfo *ci)
{
    if (ci->ci.n_Next)
    {
        RemNode((Node *)ci);
        ci->ci.n_Next = NULL;
    }
}


/*****************************************************************************/


ContextNode *GetCurrentContext(const IFFParser *iff)
{
ContextNode *cn;

    cn = (ContextNode *)FirstNode(&iff->iff_Stack);
    if (cn->cn.n_Next && cn->cn_ID)
        return cn;

    return NULL;
}


/*****************************************************************************/


ContextNode *GetParentContext(const ContextNode *cn)
{
ContextNode *parent;

    parent = (ContextNode *)NextNode(cn);
    if (parent->cn.n_Next && parent->cn_ID)
        return parent;

    return NULL;
}


/*****************************************************************************/


ContextInfo *AllocContextInfo(PackedID type, PackedID id, PackedID ident,
                              uint32 dataSize, IFFCallBack cb)
{
ContextInfo *ci;

    if (ci = AllocMem(sizeof(ContextInfo) + dataSize, MEMTYPE_FILL))
    {
        ci->ci_ID       = id;
        ci->ci_Type     = type;
        ci->ci_Ident    = ident;
        ci->ci_Data     = &ci[1];
        ci->ci_DataSize = dataSize;
        ci->ci_PurgeCB  = cb;
    }

    return ci;
}


/*****************************************************************************/


void FreeContextInfo(ContextInfo *ci)
{
    if (ci) {
        FreeMem(ci,sizeof(ContextInfo) + ci->ci_DataSize);
    }
}


/*****************************************************************************/


ContextInfo *FindContextInfo(const IFFParser *iff, PackedID type, PackedID id,
                             PackedID ident)
{
ContextNode *cn;
ContextInfo *ci;

    ScanList(&iff->iff_Stack,cn,ContextNode)
    {
        ScanList(&cn->cn_ContextInfo,ci,ContextInfo)
        {
            if ((ci->ci_Ident == ident)
             && (ci->ci_ID == id)
             && (ci->ci_Type == type))
            {
                return ci;
            }
        }
    }

    return NULL;
}


/*****************************************************************************/


ContextNode *FindPropContext(const IFFParser *iff)
{
ContextNode *cn;

    ScanList(&iff->iff_Stack,cn,ContextNode)
    {
        if ((cn->cn_ID == ID_FORM)
         || (cn->cn_ID == ID_LIST))
        {
            if (cn != (ContextNode *)FirstNode(&iff->iff_Stack))
                return cn;
        }
    }

    return NULL;
}
