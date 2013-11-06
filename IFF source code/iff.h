#ifndef __MISC_IFF_H
#define __MISC_IFF_H


/******************************************************************************
**
**  @(#) iff.h 96/04/29 1.27
**
**  IFF file parser.
**
******************************************************************************/


#ifndef __KERNEL_TYPES_H
#include <kernel/types.h>
#endif

#ifndef __KERNEL_LIST_H
#include <kernel/list.h>
#endif

#ifndef __KERNEL_ITEM_H
#include <kernel/item.h>
#endif

#ifndef __KERNEL_OPERROR_H
#include <kernel/operror.h>
#endif


/*****************************************************************************/


/* kernel interface definitions */
#define IFF_FOLIONAME "iff"


/*****************************************************************************/


/* There is one ContextNode structure created for every scoping level
 * during a parse operation. The nodes are kept in a LIFO queue. As new
 * scopes are entered, new nodes get added to the queue. As contexts are
 * exited, the nodes get removed.
 *
 * The cn_ID, cn_Type, and cn_Size fields describe the associated chunk.
 * cn_Offset indicates the current offset within this chunk where
 * ReadChunk() and WriteChunk() are.
 */
typedef struct ContextNode
{
    MinNode  cn;              /* Linkage in the LIFO queue */
    PackedID cn_Type;         /* container ID (eg: AIFF)   */
    PackedID cn_ID;           /* chunk ID                  */
#ifdef NO_64_BIT_SCALARS
    uint32   cn_SizeHi;
    uint32   cn_Size;
    uint32   cn_OffsetHi;
    uint32   cn_Offset;
#else
    uint64   cn_Size;         /* chunk size                  */
    uint64   cn_Offset;	      /* current offset within chunk */
#endif
#ifndef EXTERNAL_RELEASE
    List     cn_ContextInfo;
#ifdef NO_64_BIT_SCALARS
    uint32   cn_CurrentSizeHi;
    uint32   cn_CurrentSize;
#else
    uint64   cn_CurrentSize;
#endif
#endif
} ContextNode;


/*****************************************************************************/


#ifdef EXTERNAL_RELEASE
/* a parsing context */
typedef struct IFFParser IFFParser;
#else
typedef struct IFFParser
{
    List               iff_Stack;
    ContextNode        iff_TopContext;
    void              *iff_IOContext;
    struct IFFIOFuncs *iff_IOFuncs;
    bool               iff_WriteMode;
    bool               iff_NewIO;
    bool               iff_Paused;
    uint16             iff_Alignment;
    void              *iff_Cookie;
} IFFParser;
#endif

/* call back functions */
typedef Err (* IFFCallBack)(IFFParser *iff, void *userData);

/* information local to an active ContextNode */
typedef struct ContextInfo
{
    MinNode      ci;
#ifdef NO_64_BIT_SCALARS
    uint32       ci_DataSizeHi;
    uint32       ci_DataSize;
#else
    uint64       ci_DataSize;
#endif
    void        *ci_Data;
#ifndef EXTERNAL_RELEASE

    PackedID     ci_ID;      /* chunk ID     */
    PackedID     ci_Type;    /* container ID */
    PackedID     ci_Ident;
    IFFCallBack  ci_PurgeCB;
#endif
} ContextInfo;


/*****************************************************************************/


/* These are used for custom IO processing */
typedef Err (* IFFOpenFunc)(void **userData, void *openKey, bool writeMode);
typedef Err (* IFFCloseFunc)(void *userData);
typedef int32 (* IFFReadFunc)(void *userData, void *buffer, uint32 numBytes);
typedef int32 (* IFFWriteFunc)(void *userData, const void *buffer, uint32 numBytes);
typedef int32 (* IFFSeekFunc)(void *userData, int32 position);

typedef struct IFFIOFuncs
{
    IFFOpenFunc  io_Open;
    IFFCloseFunc io_Close;
    IFFReadFunc  io_Read;
    IFFWriteFunc io_Write;
    IFFSeekFunc  io_Seek;
} IFFIOFuncs;


/*****************************************************************************/


/* to specify chunks */
typedef struct IFFTypeID
{
    PackedID Type;   /* container ID (eg: AIFF) */
    PackedID ID;     /* chunk ID (eg: SSND)     */
} IFFTypeID;


/*****************************************************************************/


/* Describes the data associated with a previously encountered property
 * chunk.
 */
typedef struct PropChunk
{
#ifdef NO_64_BIT_SCALARS
    uint32  pc_DataSizeHi;
    uint32  pc_DataSize;
#else
    uint64  pc_DataSize;
#endif
    void   *pc_Data;
} PropChunk;


/*****************************************************************************/


/* A node in a collection list. The next pointers cross a context boundaries so
 * that the complete list is accessible.
 */
typedef struct CollectionChunk
{
    struct CollectionChunk *cc_Next;
    ContextNode            *cc_Container;
    void                   *cc_Data;
#ifdef NO_64_BIT_SCALARS
    uint32                  cc_DataSizeHi;
    uint32                  cc_DataSize;
#else
    uint64                  cc_DataSize;
#endif
} CollectionChunk;


/*****************************************************************************/


/* Error codes */
#define MakeIFFErr(svr,class,err) MakeErr(ER_FOLI,ER_IFF,svr,ER_E_SSTM,class,err)

/* No memory */
#define IFF_ERR_NOMEM         MakeIFFErr(ER_SEVERE,ER_C_STND,ER_NoMem)

/* Unknown tag supplied */
#define IFF_ERR_BADTAG        MakeIFFErr(ER_SEVERE,ER_C_STND,ER_BadTagArg)

/* Bad IFFParser parameter */
#define IFF_ERR_BADPTR        MakeIFFErr(ER_SEVERE,ER_C_STND,ER_BadPtr)

/* Operation not supported at this time */
#define IFF_ERR_NOTSUPPORTED  MakeIFFErr(ER_SEVERE,ER_C_STND,ER_NotSupported)

/* No valid scope for property */
#define IFF_ERR_NOSCOPE       MakeIFFErr(ER_SEVERE,ER_C_NSTND,1)

/* Hit the end of file too soon */
#define IFF_ERR_PREMATURE_EOF MakeIFFErr(ER_SEVERE,ER_C_NSTND,2)

/* Data in file is corrupt */
#define IFF_ERR_MANGLED       MakeIFFErr(ER_SEVERE,ER_C_NSTND,3)

/* IFF syntax error */
#define IFF_ERR_SYNTAX        MakeIFFErr(ER_SEVERE,ER_C_NSTND,4)

/* Not an IFF file */
#define IFF_ERR_NOTIFF        MakeIFFErr(ER_SEVERE,ER_C_NSTND,5)

/* No open type specified */
#define IFF_ERR_NOOPENTYPE    MakeIFFErr(ER_SEVERE,ER_C_NSTND,6)

/* Bad storage location given */
#define IFF_ERR_BADLOCATION   MakeIFFErr(ER_SEVERE,ER_C_NSTND,7)

/* Bad storage location given */
#define IFF_ERR_TOOBIG        MakeIFFErr(ER_SEVERE,ER_C_NSTND,8)

/* Bad mode parameter */
#define IFF_ERR_BADMODE       MakeIFFErr(ER_SEVERE,ER_C_NSTND,9)


/* Not really errors, but they look like 'em */

/* reached logical end of file */
#define IFF_PARSE_EOF         MakeIFFErr(ER_INFO,ER_C_NSTND,10)

/* about to leave context */
#define IFF_PARSE_EOC         MakeIFFErr(ER_INFO,ER_C_NSTND,11)


/*****************************************************************************/


/* Universal IFF identifiers */
#define ID_FORM  MAKE_ID('F','O','R','M')
#define ID_LIST  MAKE_ID('L','I','S','T')
#define ID_CAT   MAKE_ID('C','A','T',' ')
#define ID_PROP  MAKE_ID('P','R','O','P')
#define ID_NULL  MAKE_ID(' ',' ',' ',' ')

/* Identifier codes for universally recognized ContextInfo nodes */
#define IFF_CI_PROPCHUNK        MAKE_ID('p','r','o','p')
#define IFF_CI_COLLECTIONCHUNK  MAKE_ID('c','o','l','l')
#define IFF_CI_ENTRYHANDLER     MAKE_ID('e','n','h','d')
#define IFF_CI_EXITHANDLER      MAKE_ID('e','x','h','d')


/*****************************************************************************/


/* Control modes for ParseIFF() */
typedef enum ParseIFFModes
{
    IFF_PARSE_SCAN,
    IFF_PARSE_STEP,
    IFF_PARSE_RAWSTEP
} ParseIFFModes;


/*****************************************************************************/


/* value that entry or exit handlers should return to keep ParseIFF() running */
#define IFF_CB_CONTINUE 1


/*****************************************************************************/


/* different types of seek operations */
typedef enum IFFSeekModes
{
    IFF_SEEK_START,         /* relative to start of chunk   */
    IFF_SEEK_CURRENT,       /* relative to current position */
    IFF_SEEK_END            /* relative to end of chunk     */
} IFFSeekModes;


/*****************************************************************************/


/* Control modes for StoreContextInfo() */
typedef enum ContextInfoLocation
{
    IFF_CIL_BOTTOM,  /* store in default context      */
    IFF_CIL_TOP,     /* store in current context      */
    IFF_CIL_PROP     /* store in topmost FORM or LIST */
} ContextInfoLocation;


/*****************************************************************************/


/* If you pass this value as a size to PushChunk() when writing a file, the
 * parser will figure out the size of the chunk for you. If you know the size,
 * it is better to provide it as it makes things faster.
 *
 * This particular value will cause a 32-bit IFF file to be generated, which
 * is backwards compatible with IFF-85 readers. If you try to write more
 * than 0xffffffef bytes to this file, you will get an error.
 */
#define IFF_SIZE_UNKNOWN_32  0xffffffff

/* If you pass this value as a size to PushChunk() when writing a file, the
 * parser will figure out the size of the chunk for you. If you know the size,
 * it is better to provide it as it makes things faster.
 *
 * This particular value will cause a 64-bit IFF file to be generated, which
 * is not backwards compatible with IFF-85 readers, but allows darn big files
 * to be created.
 */
#define IFF_SIZE_UNKNOWN_64  0xfffffffe

/* Specifies that the actual size of the chunk is stored as a 64-bit
 * number following the normal location of the 32-bit size field.
 */
#define IFF_SIZE_64BIT    0xfffffffd

/* Reserved size values are from this number on up to 0xffffffff. No chunk
 * can have a 32-bit size field in this range.
 */
#define IFF_SIZE_RESERVED 0xfffffff0


/* default byte alignment enforced by the folio on writes */
#define IFF_DEFAULT_ALIGNMENT 4


/*****************************************************************************/


/* for use with CreateIFFParser() */
typedef enum IFFParserTags
{
    IFF_TAG_FILE = TAG_ITEM_LAST+1,  /* name of file to parse             */
    IFF_TAG_IOFUNCS,                 /* callback functions to do IO       */
    IFF_TAG_IOFUNCS_DATA             /* openKey parameter for IFFOpenFunc */
} IFFParserTags;


/*****************************************************************************/


#ifdef  __cplusplus
extern "C" {
#endif  /* __cplusplus */


/* folio management */
Err OpenIFFFolio(void);
Err CloseIFFFolio(void);

/* parser control */
Err CreateIFFParser(IFFParser **iff, bool writeMode, const TagArg tags[]);
Err CreateIFFParserVA(IFFParser **iff, bool writeMode, uint32 tag, ...);
Err DeleteIFFParser(IFFParser *iff);
Err ParseIFF(IFFParser *iff, ParseIFFModes control);

/* chunk data IO */
int32 ReadChunk(IFFParser *iff, void *buffer, uint32 numBytes);
int32 WriteChunk(IFFParser *iff, const void *buffer, uint32 numBytes);
Err SeekChunk(IFFParser *iff, int32 position, IFFSeekModes mode);

#ifdef NO_64_BIT_SCALARS
int32 GetIFFOffset(IFFParser *iff);
#else
int64 GetIFFOffset(IFFParser *iff);
#endif

/* context entry and exit */
Err PushChunk(IFFParser *iff, PackedID type, PackedID id, uint32 size);
Err PopChunk(IFFParser *iff);

/* built-in chunk and property handlers */
Err RegisterPropChunks(IFFParser *iff, const IFFTypeID typeids[]);
Err RegisterStopChunks(IFFParser *iff, const IFFTypeID typeids[]);
Err RegisterCollectionChunks(IFFParser *iff, const IFFTypeID typeids[]);

/* context utilities */
PropChunk *FindPropChunk(const IFFParser *iff, PackedID type, PackedID id);
CollectionChunk *FindCollection(const IFFParser *iff, PackedID type, PackedID id);
ContextNode *FindPropContext(const IFFParser *iff);
ContextNode *GetCurrentContext(const IFFParser *iff);
ContextNode *GetParentContext(const ContextNode *contextNode);

/* ContextInfo utilities */
ContextInfo *AllocContextInfo(PackedID type, PackedID id, PackedID ident, uint32 dataSize, IFFCallBack cb);
void FreeContextInfo(ContextInfo *ci);
ContextInfo *FindContextInfo(const IFFParser *iff, PackedID type, PackedID id, PackedID ident);
Err StoreContextInfo(IFFParser *iff, ContextInfo *ci, ContextInfoLocation pos);
void AttachContextInfo(IFFParser *iff, ContextNode *to, ContextInfo *ci);
void RemoveContextInfo(ContextInfo *ci);

/* low-level handler installation */
Err InstallEntryHandler(IFFParser *iff, PackedID type, PackedID id, ContextInfoLocation pos,
                        IFFCallBack cb, const void *userData);
Err InstallExitHandler(IFFParser *iff, PackedID type, PackedID id, ContextInfoLocation pos,
                       IFFCallBack cb, const void *userData);


#ifdef  __cplusplus
}
#endif  /* __cplusplus */


/*****************************************************************************/


#endif /* __MISC_IFF_H */
