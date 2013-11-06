/* @(#) autodocs.c 96/04/29 1.23 */

/**
|||	AUTODOC -class IFF -name RegisterCollectionChunks
|||	Defines chunks to be collected during the parse operation.
|||
|||	  Synopsis
|||
|||	    Err RegisterCollectionChunks(IFFParser *iff, const IFFTypeID typeids[]);
|||
|||	  Description
|||
|||	    You supply this function an array of IFFTypeID structures which
|||	    define the chunks that should be stored as they are encountered
|||	    during the parse operation. The array is terminated by a structure
|||	    containing a Type field set to 0.
|||
|||	    The function installs an entry handler for chunks with the given
|||	    type and id so that the contents of those chunks will be stored as
|||	    they are encountered. This is similar to RegisterPropChunks() except
|||	    that more than one chunk of any given type can be stored in lists
|||	    which can be returned by FindCollection(). The storage of these
|||	    chunks still follows the property chunk scoping rules for IFF files
|||	    so that at any given point, stored collection chunks will be valid
|||	    in the current context.
|||
|||	  Arguments
|||
|||	    iff
|||	        Parser handle to operate on.
|||
|||	    typeids
|||	        An array of IFFTypeID structures defining which chunks should
|||	        be collected. The array is terminated by a structure whose Type
|||	        field is set to 0.
|||
|||	  Return Value
|||
|||	    Returns >= 0 for success, or a negative error code for failure.
|||	    Possible error codes currently include:
|||
|||	    IFF_ERR_NOMEM
|||	        There was not enough memory to perform the operation.
|||
|||	  Implementation
|||
|||	    Folio call implemented in IFF folio V27.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    FindCollection(), RegisterPropChunks()
|||
**/

/**
|||	AUTODOC -class IFF -name InstallEntryHandler
|||	Adds an entry handler to the parser.
|||
|||	  Synopsis
|||
|||	    Err InstallEntryHandler(IFFParser *iff, PackedID type, PackedID id,
|||	                            ContextInfoLocation pos, IFFCallBack cb,
|||	                            const void *userData);
|||
|||	  Description
|||
|||	    This function installs an entry handler for a specific type of chunk
|||	    into the context for the given parser handle. The type and id are
|||	    the identifiers for the chunk to handle.
|||
|||	    The handler will be called whenever the parser enters a chunk of
|||	    the given type, so the IFF stream will be positioned to read the
|||	    first data byte in the chunk.
|||
|||	    The value your callback routine returns affects the parser
|||	    in two ways:
|||
|||	    IFF_CB_CONTINUE
|||	        Normal return. The parser will continue through the file.
|||
|||	    <any other value>
|||	        ParseIFF() will stop parsing and return this value directly to
|||	        the caller. Return 0 for a normal return, and negative values
|||	        for errors.
|||
|||	  Arguments
|||
|||	    iff
|||	        The parser handle to affect.
|||
|||	    type
|||	        The ID of the container for the chunks to handle (eg: AIFF).
|||
|||	    id
|||	        The ID value for the chunks to handle.
|||
|||	    pos
|||	        Where the handler should be installed within the context
|||	        stack. Refer to the StoreContextInfo() for a description of
|||	        how this argument is used.
|||
|||	    cb
|||	        The routine to invoke whenever a chunk of the given type and
|||	        id is encountered.
|||
|||	    userData
|||	        A value that is passed straight through to your callback
|||	        routine when it is invoked.
|||
|||	  Return Value
|||
|||	    Returns >= 0 for success, or a negative error code for failure.
|||
|||	  Implementation
|||
|||	    Folio call implemented in IFF folio V27.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    InstallExitHandler()
|||
**/

/**
|||	AUTODOC -class IFF -name InstallExitHandler
|||	Adds an exit handler to the parser.
|||
|||	  Synopsis
|||
|||	    Err InstallExitHandler(IFFParser *iff, PackedID type, PackedID id,
|||	                           ContextInfoLocation pos, IFFCallBack cb,
|||	                           const void *userData);
|||
|||	  Description
|||
|||	    This function installs an exit handler for a specific type of chunk
|||	    into the context for the given parser handle. The type and id are
|||	    the identifiers for the chunk to handle.
|||
|||	    The handler will be called whenever the parser is about to leave a
|||	    chunk of the given type. The position within the stream is
|||	    not constant and must be determined by looking at the cn_Offset
|||	    field of the current context.
|||
|||	    The value your callback routine returns affects the parser
|||	    in two ways:
|||
|||	    IFF_CB_CONTINUE
|||	        Normal return. The parser will continue through the file.
|||
|||	    <any other value>
|||	        ParseIFF() will stop parsing and return this value directly to
|||	        the caller. Return 0 for a normal return, and negative values
|||	        for errors.
|||
|||	  Arguments
|||
|||	    iff
|||	        The parser handle to affect.
|||
|||	    type
|||	        The ID of the container for the chunks to handle (eg: AIFF).
|||
|||	    id
|||	        The ID value for the chunks to handle.
|||
|||	    pos
|||	        Where the handler should be installed within the context
|||	        stack. Refer to the StoreContextInfo() for a description of
|||	        how this argument is used.
|||
|||	    cb
|||	        The routine to invoke whenever a chunk of the given type and
|||	        id is about to be exited.
|||
|||	    userData
|||	        A value that is passed straight through to your callback
|||	        routine when it is invoked.
|||
|||	  Return Value
|||
|||	    Returns >= 0 for success, or a negative error code for failure.
|||
|||	  Implementation
|||
|||	    Folio call implemented in IFF folio V27.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    InstallEntryHandler()
|||
**/

/**
|||	AUTODOC -class IFF -name RegisterPropChunks
|||	Defines property chunks to be stored during the parse operation.
|||
|||	  Synopsis
|||
|||	    Err RegisterPropChunks(IFFParser *iff, const IFFTypeID typeids[]);
|||
|||	  Description
|||
|||	    You supply this function an array of IFFTypeID structures which
|||	    define the chunks that should be stored as they are encountered
|||	    during the parse operation. The array is terminated by a structure
|||	    containing a Type field set to 0.
|||
|||	    The function installs an entry handler for chunks with the given
|||	    type and ID so that the contents of those chunks will be stored as
|||	    they are encountered. The storage of these chunks follows the
|||	    property chunk scoping rules for IFF files so that at any given
|||	    point, a stored property chunk returned by FindPropContext() will
|||	    be the valid property for the current context.
|||
|||	  Arguments
|||
|||	    iff
|||	        Parser handle to operate on.
|||
|||	    typeids
|||	        An array of IFFTypeID structures defining which chunks should
|||	        be stored. The array is terminated by a structure whose Type
|||	        field is set to 0.
|||
|||	  Return Value
|||
|||	    Returns >= 0 for success, or a negative error code for failure.
|||	    Possible error codes currently include:
|||
|||	    IFF_ERR_NOMEM
|||	        There was not enough memory to perform the operation.
|||
|||	  Implementation
|||
|||	    Folio call implemented in IFF folio V27.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    FindPropContext()
|||
**/

/**
|||	AUTODOC -class IFF -name RegisterStopChunks
|||	Defines chunks that cause the parser to stop and return to the caller.
|||
|||	  Synopsis
|||
|||	    Err RegisterStopChunks(IFFParser *iff, const IFFTypeID typeids[]);
|||
|||	  Description
|||
|||	    You supply this function an array of IFFTypeID structures which
|||	    define the chunks that should cause the parser to stop and return
|||	    control to the caller. The array is terminated by a structure
|||	    containing a Type field set to 0.
|||
|||	    The function installs an entry handler for chunks with the given
|||	    type and ID so that the parser will stop as they are encountered.
|||
|||	  Arguments
|||
|||	    iff
|||	        Parser handle to operate on.
|||
|||	    typeids
|||	        An array of IFFTypeID structures defining which chunks should
|||	        cause the parser to stop. The array is terminated by a structure
|||	        whose Type field is set to 0.
|||
|||	  Return Value
|||
|||	    Returns >= 0 for success, or a negative error code for failure.
|||	    Possible failure codes include:
|||
|||	    IFF_ERR_NOMEM
|||	        There was not enough memory to perform the operation.
|||
|||	  Implementation
|||
|||	    Folio call implemented in IFF folio V27.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    ParseIFF()
|||
**/

/**
|||	AUTODOC -class IFF -name AllocContextInfo
|||	Allocates and initializes a ContextInfo structure.
|||
|||	  Synopsis
|||
|||	    ContextInfo *AllocContextInfo(PackedID type, PackedID id,
|||	                                  PackedID ident, uint32 dataSize,
|||	                                  IFFCallBack cb);
|||
|||	  Description
|||
|||	    Allocates and initializes a ContextInfo structure with a specified
|||	    number of bytes of user data. The returned structure contains a
|||	    pointer to the user data buffer in the ci_Data field. The buffer
|||	    is automatically cleared to 0 upon allocation.
|||
|||	    The optional callback function argument sets a client-supplied
|||	    cleanup routine for disposal when the context associated with the
|||	    new ContextInfo is popped. The purge routine will be called when the
|||	    ContextNode containing this structure is popped off the context stack
|||	    and is about to be deleted itself.
|||
|||	    The purge callback you supply is called with its second argument as
|||	    a pointer to the ContextInfo structure being purged. The purge
|||	    routine is responsible for calling FreeContextInfo() on this
|||	    ContextInfo when it is done with it.
|||
|||	  Arguments
|||
|||	    type
|||	        ID of container (eg: AIFF)
|||
|||	    id
|||	        ID of context that will contain this node.
|||
|||	    ident
|||	        Identifier that represents the type of information this node
|||	        describes. This value is later used when searching through
|||	        the list of ContextInfo structures to find a particular node.
|||
|||	    dataSize
|||	        The number of bytes of data to allocate for user use. The
|||	        data area allocated can be found by looking at the ci_Data
|||	        field of the returned ContextInfo structure.
|||
|||	    cb
|||	        The routine to be called when the ContextInfo structure needs
|||	        to be purged. If this is NULL, the folio will simply call
|||	        FreeContextInfo() automatically.
|||
|||	  Return Value
|||
|||	    Returns a pointer to a new ContextInfo structure or NULL if
|||	    there was not enough memory.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    FreeContextInfo(), StoreContextInfo(), AttachContextInfo(),
|||	    RemoveContextInfo(), FindContextInfo()
|||
**/

/**
|||	AUTODOC -class IFF -name CreateIFFParser
|||	Creates a new parser structure and prepares it for use.
|||
|||	  Synopsis
|||
|||	    Err CreateIFFParser(IFFParser **iff, bool writeMode,
|||	                        const TagArg tags[]);
|||
|||	    Err CreateIFFParserVA(IFFParser **iff, bool writeMode,
|||	                          uint32 tag, ...);
|||
|||	  Description
|||
|||	    This function allocates and initializes a new IFFParser structure.
|||	    Once created, you can use the structure to parse IFF streams
|||	    and get at the data in them. The streams can currently either
|||	    be files or a block of memory.
|||
|||	  Arguments
|||
|||	    iff
|||	        A pointer to a variable where a handle to the IFFParser will
|||	        be stored. The value is set to NULL if the parser can't be
|||	        created.
|||
|||	    writeMode
|||	        Specifies how the stream should be opened. TRUE means the
|||	        file will be written to, or FALSE if the stream is to be read.
|||
|||	    tags
|||	        A pointer to an array of tag arguments containing extra data
|||	        for this function. See below for a description of the tags
|||	        supported.
|||
|||	  Tag Arguments
|||
|||	    The following tag arguments may be supplied in array form to this
|||	    function. The array must be terminated with TAG_END.
|||
|||	    IFF_TAG_FILE (const char *)
|||	        This tag lets you specify the name of a file to parse. This
|||	        file will be opened and prepared for use automatically.
|||	        If you supply this tag, you may not supply the IFF_TAG_IOFUNCS
|||	        tag at the same time.
|||
|||	    IFF_TAG_IOFUNCS (IFFIOFuncs *)
|||	        This tag is used in conjunction with the IFF_TAG_IOFUNCS_DATA
|||	        tag and lets you provide custom I/O functions that are used
|||	        as callbacks by the folio to read and write data. This lets
|||	        you do things like parse an in-memory IFF image, or parse a
|||	        data stream coming from some place other than a file. If you
|||	        supply this tag, you may not supply the IFF_TAG_FILE
|||	        tag at the same time. You supply a pointer to a fully
|||	        initialized IFFIOFuncs structure, which contains function
|||	        pointers that the folio can use to perform I/O operations.
|||
|||	    IFF_TAG_IOFUNCS_DATA (void *)
|||	        This tag works in conjunction with the IFF_TAG_IOFUNCS tag
|||	        and lets you specify the parameter that is passed as the
|||	        openKey parameter to the IFFOpenFunc callback function.
|||
|||	  Return Value
|||
|||	    Returns >= 0 for success, or a negative error code if the
|||	    parser could not be created. Possible error codes currently
|||	    include:
|||
|||	    IFF_ERR_NOOPENTYPE
|||	        You didn't supply one of the IFF_TAG_FILE or IFF_TAG_IOFUNCS
|||	        tags. One of these two tags must be supplied in order to tell
|||	        the parser what data to parse.
|||
|||	    IFF_ERR_NOMEM
|||	        There was not enough memory.
|||
|||	  Implementation
|||
|||	    Folio call implemented in IFF folio V27.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    DeleteIFFParser(), ParseIFF()
|||
**/

/**
|||	AUTODOC -class IFF -name DeleteIFFParser
|||	Deletes an IFF parser.
|||
|||	  Synopsis
|||
|||	    Err DeleteIFFParser(IFFParser *iff);
|||
|||	  Description
|||
|||	    Deletes an IFF parser previously created by CreateIFFParser().
|||	    Any data left to be output is sent out, files are closed, and
|||	    all resources are released.
|||
|||	  Arguments
|||
|||	    iff
|||	        An active parser, as obtained from CreateIFFParser(). Once
|||	        this call is made, the parser handle becomes invalid and can
|||	        no longer be used. This value may be NULL, in which case this
|||	        function does nothing.
|||
|||	  Return Value
|||
|||	    Returns >= 0 if successful, or a negative error code if it fails.
|||	    Possible error codes currently include:
|||
|||	    IFF_ERR_BADPTR
|||	        An invalid parser handle was supplied.
|||
|||	  Implementation
|||
|||	    Folio call implemented in IFF folio V27.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    CreateIFFParser(), ParseIFF()
|||
**/

/**
|||	AUTODOC -class IFF -name FindCollection
|||	Gets a pointer to the current list of collection chunks.
|||
|||	  Synopsis
|||
|||	    CollectionChunk *FindCollection(const IFFParser *iff,
|||	                                    PackedID type, PackedID id);
|||
|||	  Description
|||
|||	    This function returns a pointer to a list of CollectionChunk
|||	    structures for each of the collection chunks of the given type
|||	    encountered so far in the course of the parse operation. The
|||	    structures appearing first in the list will be the ones encountered
|||	    most recently.
|||
|||	  Arguments
|||
|||	    iff
|||	        The parsing handle to query.
|||
|||	    type
|||	        Chunk type to search for.
|||
|||	    id
|||	        Chunk id to search for.
|||
|||	  Return Value
|||
|||	    A pointer to the last CollectionChunk structure describing the
|||	    last collection chunk encountered. The structure contains a
|||	    pointer that goes back through all the collection chunks
|||	    encountered during the parse operation. This function returns
|||	    NULL if no collection chunks have been encountered so far.
|||
|||	  Implementation
|||
|||	    Folio call implemented in IFF folio V27.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    RegisterCollectionChunks()
|||
**/

/**
|||	AUTODOC -class IFF -name FindContextInfo
|||	Returns a ContextInfo structure from the context stack.
|||
|||	  Synopsis
|||
|||	    ContextInfo *FindContextInfo(const IFFParser *iff, PackedID type,
|||	                                 PackedID id, PackedID ident);
|||
|||	  Description
|||
|||	    This functions searches through the context stack for a ContextInfo
|||	    structure with matching type, id, and ident values. The search
|||	    starts from the most current context backwards, so that the any
|||	    structure found will be the one with greatest precedence in the
|||	    context stack. The type, id, and ident values correspond to the
|||	    parameters supplied to AllocContextInfo() to create the structure.
|||
|||	  Arguments
|||
|||	    iff
|||	        The parse handle to search in.
|||
|||	    type
|||	        The type value to search for.
|||
|||	    id
|||	        The id value to search for.
|||
|||	    ident
|||	        The ident code to search for.
|||
|||	  Return Value
|||
|||	    Returns a pointer to a ContextInfo structure or NULL if no
|||	    matching structure was found.
|||
|||	  Implementation
|||
|||	    Folio call implemented in IFF folio V27.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    AllocContextInfo(), FreeContextInfo(), StoreContextInfo(),
|||	    AttachContextInfo(), RemoveContextInfo()
|||
**/

/**
|||	AUTODOC -class IFF -name FindPropChunk
|||	Searches for a stored property chunk.
|||
|||	  Synopsis
|||
|||	    PropChunk *FindPropChunk(const IFFParser *iff,
|||	                             PackedID type, PackedID id);
|||
|||	  Description
|||
|||	    This function searches for the stored property chunk which is valid
|||	    in the given context. Property chunks are automatically stored by
|||	    ParseIFF() when pre-declared by RegisterPropChunks(). The pc_Data
|||	    field of the PropChunk structure contains a pointer to the data
|||	    associated with the property chunk.
|||
|||	    The data pointed to by pc_Data is fully writable by your
|||	    task. It can sometimes be handy to write directly to this
|||	    area instead of making a local copy.
|||
|||	  Arguments
|||
|||	    iff
|||	        The parser handler to search in.
|||
|||	    type
|||	        The type code of the property chunk to search for.
|||
|||	    id
|||	        The id code of the property chunk to search for.
|||
|||	  Return Value
|||
|||	    Returns a pointer to a PropChunk structure that describes the
|||	    sought property, or NULL if no such property chunk has been
|||	    encountered so far.
|||
|||	  Implementation
|||
|||	    Folio call implemented in IFF folio V27.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    RegisterPropChunks()
|||
**/

/**
|||	AUTODOC -class IFF -name FindPropContext
|||	Gets the parser's current property context.
|||
|||	  Synopsis
|||
|||	    ContextNode *FindPropContext(const IFFParser *iff);
|||
|||	  Description
|||
|||	    This function locates the context node which would be the
|||	    scoping chunk for properties in the current parsing state. This is
|||	    used for locating the proper scoping context for property chunks
|||	    (i.e. the scope from which a property would apply). This is usually
|||	    the FORM or LIST with the highest precedence in the context stack.
|||
|||	  Arguments
|||
|||	    iff
|||	        The parsing handle to search in.
|||
|||	  Return Value
|||
|||	    Returns a pointer to a ContextNode or NULL if not found.
|||
|||	  Implementation
|||
|||	    Folio call implemented in IFF folio V27.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    GetCurrentContext(), GetParentContext()
|||
**/

/**
|||	AUTODOC -class IFF -name FreeContextInfo
|||	Deallocate a ContextInfo structure.
|||
|||	  Synopsis
|||
|||	    void FreeContextInfo(ContextInfo *ci);
|||
|||	  Description
|||
|||	    This function frees the memory for the ContextInfo structure and
|||	    any associated user memory as allocated with AllocContextInfo().
|||	    User purge vectors should call this function after they have freed
|||	    any other resources associated with this structure.
|||
|||	    Note that this function does NOT call the custom purge vector
|||	    optionally installed through AllocContextInfo(); all it does is
|||	    free the structure.
|||
|||	  Arguments
|||
|||	    ci
|||	        The ContextInfo structure to free. This may be NULL in which
|||	        case this function does nothing.
|||
|||	  Implementation
|||
|||	    Folio call implemented in IFF folio V27.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    AllocContextInfo()
|||
**/

/**
|||	AUTODOC -class IFF -name GetCurrentContext
|||	Gets a pointer to the ContextNode for the current chunk.
|||
|||	  Synopsis
|||
|||	    ContextNode *GetCurrentContext(const IFFParser *iff);
|||
|||	  Description
|||
|||	    Returns the top ContextNode for the given parser handle. The top
|||	    ContextNode corresponds to the chunk most recently pushed on the
|||	    context stack, which is the chunk where the stream is currently
|||	    positioned.
|||
|||	    The ContextNode structure contains information on the type of chunk
|||	    currently being processed, like its size and the current
|||	    position within the chunk.
|||
|||	  Arguments
|||
|||	    iff
|||	        The parser handle to query.
|||
|||	  Return Value
|||
|||	    Returns a pointer to the current ContextNode for the parser.
|||
|||	  Implementation
|||
|||	    Folio call implemented in IFF folio V27.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    PushChunk(), PopChunk(), ParseIFF(), GetParentContext()
|||
**/

/**
|||	AUTODOC -class IFF -name GetParentContext
|||	Gets the nesting ContextNode for the given ContextNode.
|||
|||	  Synopsis
|||
|||	    ContextNode *GetParentContext(ContextNode *cn);
|||
|||	  Description
|||
|||	    This function returns a ContextNode for the chunk containing the
|||	    chunk associated with the supplied ContextNode. This function
|||	    effectively moves down the context stack into previously pushed
|||	    contexts.
|||
|||	  Arguments
|||
|||	    cn
|||	        The ContextNode to obtain the parent of.
|||
|||	  Return Value
|||
|||	    Returns the parent of the supplied context or NULL if the supplied
|||	    context has no parent.
|||
|||	  Implementation
|||
|||	    Folio call implemented in IFF folio V27.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    GetCurrentContext(), FindPropContext()
|||
**/

/**
|||	AUTODOC -class IFF -name ParseIFF
|||	Parses an IFF stream.
|||
|||	  Synopsis
|||
|||	    Err ParseIFF(IFFParser *iff, ParseIFFModes control);
|||
|||	  Description
|||
|||	    This function traverses a stream opened for read by pushing chunks
|||	    onto the context stack and popping them off directed by the generic
|||	    syntax of IFF files. As it pushes each new chunk, it searches the
|||	    context stack for handlers to apply to chunks of that type. If it
|||	    finds an entry handler it will invoke it just after entering the
|||	    chunk. If it finds an exit handler it will invoke it just before
|||	    leaving the chunk. Standard handlers include entry handlers for
|||	    pre-declared property chunks and collection chunks and entry and
|||	    exit handlers for stop chunks - that is, chunks which will cause
|||	    the ParseIFF() function to return control to the client. Client
|||	    programs can also provide their own custom handlers.
|||
|||	    The control argument can have one of three values:
|||
|||	    IFF_PARSE_SCAN
|||	        In this normal mode, ParseIFF() will only return control to
|||	        the caller when either:
|||
|||	        1) an error is encountered, or
|||
|||	        2) a stop chunk is encountered, or
|||
|||	        3) a user handler returns a value other than 1, or
|||
|||	        3) the end of the logical file is reached, in which
|||	           case IFF_PARSE_EOF is returned.
|||
|||	        ParseIFF() will continue pushing and popping chunks until one
|||	        of these conditions occurs. If ParseIFF() is called again
|||	        after returning, it will continue to parse the file where it
|||	        left off.
|||
|||	    IFF_PARSE_STEP and IFF_PARSE_RAWSTEP
|||	        In these two modes, ParseIFF() will return control to the
|||	        caller after every step in the parse, specifically, after
|||	        each push of a context node and just before each pop. If
|||	        returning just before a pop, ParseIFF() will return
|||	        IFF_PARSE_EOC, which is not an error, per se, but is just an
|||	        indication that the most recent context is ending. In STEP
|||	        mode, ParseIFF() will invoke the handlers for chunks, if
|||	        any, before returning. In RAWSTEP mode, ParseIFF() will not
|||	        invoke any handlers and will return right away. In both
|||	        cases the function can be called multiple times to step
|||	        through the parsing of the IFF file.
|||
|||	  Arguments
|||
|||	    iff
|||	        The parsing handle to affect.
|||
|||	    control
|||	        Instructs how to parse.
|||
|||	  Return Value
|||
|||	    Returns >= 0 for success, or a negative error code for failure.
|||
|||	  Implementation
|||
|||	    Folio call implemented in IFF folio V27.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    PushChunk(), PopChunk(), InstallEntryHandler(), InstallExitHandler(),
|||	    RegisterPropChunks(), RegisterCollectionChunks(), RegisterStopChunks()
|||
**/

/**
|||	AUTODOC -class IFF -name PopChunk
|||	Pops the top context node off the context stack.
|||
|||	  Synopsis
|||
|||	    Err PopChunk(IFFParser *iff);
|||
|||	  Description
|||
|||	    This function pops the top context chunk and frees all associated
|||	    ContextInfo structures.  The function is normally only called when
|||	    writing files and signals the end of a chunk.
|||
|||	  Arguments
|||
|||	    iff
|||	        The parsing handle to affect.
|||
|||	  Return Value
|||
|||	    Returns >= 0 for success, or a negative error code for failure.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    PushChunk()
|||
**/

/**
|||	AUTODOC -class IFF -name PushChunk
|||	Pushes a new context node on the context stack.
|||
|||	  Synopsis
|||
|||	    Err PushChunk(IFFParser *iff, PackedID type, PackedID id,
|||	                  uint32 size);
|||
|||	  Description
|||
|||	    This functio pushes a new context node on the context stack by
|||	    reading it from the stream if this is a read stream, or by creating
|||	    it from the passed parameters if this is a write stream. Normally
|||	    this function is only called in write mode, where the type and id
|||	    codes specify the new chunk to create. If this is a leaf chunk
|||	    (i.e. a local chunk inside a FORM or PROP chunk), then the type
|||	    argument is ignored.
|||
|||	    If the size is specified then the chunk writing functions will
|||	    enforce this size. If the size is given as IFF_SIZE_UNKNOWN_32, the
|||	    chunk will expand to accommodate whatever is written into it up to
|||	    a maximum of (IFF_SIZE_RESERVED-1) bytes. If the size is given
|||	    as IFF_SIZE_UNKNOWN_64, then you can write as much data as there
|||	    are atoms in the universe.
|||
|||	  Arguments
|||
|||	    iff
|||	        The parsing handle to affect.
|||
|||	    type
|||	        The ID of the container (eg: AIFF). This is ignored in read mode,
|||	        and for leaf chunks.
|||
|||	    id
|||	        The chunk id speciiier. This is ignored in read mode.
|||
|||	    size
|||	        The size of the chunk to create, or IFF_SIZE_UNKNOWN_32
|||	        or IFF_SIZE_UNKNOWN_64. This parameter is ignored in read mode.
|||
|||	  Return Value
|||
|||	    Returns >= 0 for success, or a negative error code for failure.
|||
|||	  Implementation
|||
|||	    Folio call implemented in IFF folio V27.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    PopChunk(), WriteChunk()
|||
**/

/**
|||	AUTODOC -class IFF -name ReadChunk
|||	Reads bytes from the current chunk into a buffer.
|||
|||	  Synopsis
|||
|||	    int32 ReadChunk(IFFParser *iff, void *buffer, uint32 numBytes);
|||
|||	  Description
|||
|||	    This function reads data from the IFF stream into the buffer for
|||	    the specified number of bytes. Reads are limited to the size of the
|||	    current chunk and attempts to read past the end of the chunk will
|||	    truncate.
|||
|||	  Arguments
|||
|||	    iff
|||	        The parsing handle to read from.
|||
|||	    buffer
|||	        A pointer to where the read data should be put.
|||
|||	    numBytes
|||	        The number of bytes of data to read.
|||
|||	  Return Value
|||
|||	    Returns the actual number of bytes read, or a negative error code
|||	    for failure.
|||
|||	  Implementation
|||
|||	    Folio call implemented in IFF folio V27.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    ParseIFF(), SeekChunk(), WriteChunk()
|||
**/

/**
|||	AUTODOC -disabled -class IFF -name ReadChunkCompressed
|||	Reads and decompresses bytes from the current chunk into a buffer.
|||
|||	  Synopsis
|||
|||	    int32 ReadChunkCompressed(IFFParser *iff, void *buffer,
|||	                              uint32 numBytes);
|||
|||	  Description
|||
|||	    This function reads data from the IFF stream into the buffer for
|||	    the specified number of bytes. Reads are limited to the size of the
|||	    current chunk and attempts to read past the end of the chunk will
|||	    truncate. As the data is read, it is automatically decompressed
|||	    using the compression folio.
|||
|||	  Arguments
|||
|||	    iff
|||	        The parsing handle to read from.
|||
|||	    buffer
|||	        A pointer to where the read data should be put.
|||
|||	    numBytes
|||	        The number of bytes of data to read.
|||
|||	  Return Value
|||
|||	    Returns the actual number of bytes read, or a negative error code
|||	    for failure.
|||
|||	  Implementation
|||
|||	    Folio call implemented in IFF folio V27.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    ParseIFF(), ReadChunk(), SeekChunk(), WriteChunk(),
|||	    WriteChunkCompressed()
|||
**/

/**
|||	AUTODOC -class IFF -name SeekChunk
|||	Moves the current position cursor within the current chunk.
|||
|||	  Synopsis
|||
|||	    Err SeekChunk(IFFParser *iff, int32 position, IFFSeekModes mode);
|||
|||	  Description
|||
|||	    This function moves the position cursor within the current chunk.
|||	    The cursor determines where the next read operation will get its
|||	    data from, and where the next write operation will write its data
|||	    to.
|||
|||	    The cursor can be moved to a position which is relative to the
|||	    start or end of the current chunk, as well as relative to the
|||	    current position. The cursor is never allowed to be less than 0 or
|||	    greater than the number of bytes currently in the chunk. Any attempt
|||	    to do so results in the cursor being positioned to the beginning or
|||	    end of the chunk, depending on which limit was exceeded. This does
|||	    not result in an error.
|||
|||	  Arguments
|||
|||	    iff
|||	        The parsing handle to seek in.
|||
|||	    position
|||	        The position to move the cursor to.
|||
|||	    mode
|||	        Describes what the position argument is relative to.
|||
|||	    The possible seek modes are:
|||
|||	    IFF_SEEK_START
|||	        Indicates that the supplied position is relative to the
|||	        beginning of the chunk. So a position of 10 would put the
|||	        cursor at byte #10 (the eleventh byte) within the chunk.
|||
|||	    IFF_SEEK_CURRENT
|||	        Indicates that the supplied position is relative to the current
|||	        position within the chunk. A positive position value moves the
|||	        cursor forward in the file by that many bytes, while a negative
|||	        value moves the cursor back by that number of bytes.
|||
|||	    IFF_SEEK_END
|||	        Indicates that the supplied position is relative to the end
|||	        of the chunk. The position value should be a negative value.
|||
|||	  Return Value
|||
|||	    Returns >= 0 for success, or a negative error code for failure.
|||
|||	  Caveat
|||
|||	    It is not currently possible to seek more than 2^31 bytes in one
|||	    shot.
|||
|||	  Implementation
|||
|||	    Folio call implemented in IFF folio V27.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    ReadChunk(), WriteChunk()
|||
**/

/**
|||	AUTODOC -class IFF -name GetIFFOffset
|||	Returns the absolute seek position within the current IFF stream.
|||
|||	  Synopsis
|||
|||	    int64 GetIFFOffset(IFFParser *iff);
|||
|||	  Description
|||
|||	    This function returns the count of bytes from the beginning of the
|||	    current IFF stream. This is useful to identify the exact position
|||	    of a chunk or part of a chunk within an IFF file.
|||
|||	  Arguments
|||
|||	    iff
|||	        The parsing handle to query.
|||
|||	  Return Value
|||
|||	    Returns the absolute seek offset from the beginning of the
|||	    IFF stream to the current file cursor position, or a negative
|||	    error code for failure.
|||
|||	  Implementation
|||
|||	    Folio call implemented in IFF folio V27.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
**/

/**
|||	AUTODOC -class IFF -name AttachContextInfo
|||	Attaches a ContextInfo structure to a given ContextNode.
|||
|||	  Synopsis
|||
|||	    void AttachContextInfo(IFFParser *iff, ContextNode *to,
|||	                           ContextInfo *ci);
|||
|||	  Description
|||
|||	    This function adds the ContextInfo structure to the supplied
|||	    ContextNode structure. If another ContextInfo of the same type, id
|||	    and ident codes is already present in the ContextNode, it will be
|||	    purged and replaced with this new structure.
|||
|||	  Arguments
|||
|||	    iff
|||	        The parsing handle to affect.
|||
|||	    to
|||	        The ContextNode to attach the ContextInfo structure to.
|||
|||	    ci
|||	        The ContextInfo structure to attach.
|||
|||	  Implementation
|||
|||	    Folio call implemented in IFF folio V27.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    StoreContextInfo(), RemoveContextInfo(), AllocContextInfo(),
|||	    FreeContextInfo(), FindContextInfo()
|||
**/

/**
|||	AUTODOC -class IFF -name RemoveContextInfo
|||	Removes a ContextInfo structure from wherever it is attached.
|||
|||	  Synopsis
|||
|||	    void RemoveContextInfo(ContextInfo *ci);
|||
|||	  Description
|||
|||	    This function removes the ContextInfo structure from any list it
|||	    might currently be in. If the structure is not currently in a list,
|||	    this function is a NOP.
|||
|||	  Arguments
|||
|||	    ci
|||	        The ContextInfo structure to remove.
|||
|||	  Implementation
|||
|||	    Folio call implemented in IFF folio V28.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    AttachContextInfo(), StoreContextInfo(), AllocContextInfo(),
|||	    FreeContextInfo(), FindContextInfo()
|||
**/

/**
|||	AUTODOC -class IFF -name StoreContextInfo
|||	Inserts a ContextInfo structure into the context stack.
|||
|||	  Synopsis
|||
|||	    Err StoreContextInfo(IFFParser *iff, ContextInfo *ci,
|||	                         ContextInfoLocation pos);
|||
|||	  Description
|||
|||	    This function adds the ContextInfo structure to the list of
|||	    structures for one of the ContextNodes on the context stack and
|||	    purges any other structure in the same context with the same type,
|||	    id and ident codes. The pos argument determines where in the stack
|||	    to add the structure:
|||
|||	    IFF_CIL_BOTTOM
|||	        Add the structure to the ContextNode at the bottom of the stack.
|||
|||	    IFF_CIL_TOP
|||	        Add the structure to the top (current) context node.
|||
|||	    IFF_CIL_PROP
|||	        Add the structure in the top property context. The top
|||	        property context is either the top FORM chunk, or the top LIST
|||	        chunk, whichever is closer to the top of the stack.
|||
|||	  Arguments
|||
|||	    iff
|||	        The parsing stream to affect.
|||
|||	    ci
|||	        The ContextInfo structure to add.
|||
|||	    pos
|||	        The position where the structure should be added.
|||
|||	  Return Value
|||
|||	    Returns >= 0 for success, or a negative error code for failure.
|||
|||	  Implementation
|||
|||	    Folio call implemented in IFF folio V27.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    AllocContextInfo(), FreeContextInfo(), FindContextInfo(),
|||	    AttachContextInfo(), RemoveContextInfo()
|||
**/

/**
|||	AUTODOC -class IFF -name WriteChunk
|||	Writes data from a buffer into the current chunk.
|||
|||	  Synopsis
|||
|||	    int32 WriteChunk(IFFParser *iff, const void *buffer,
|||	                     uint32 numBytes);
|||
|||	  Description
|||
|||	    This function writes the requested number of bytes into the IFF
|||	    stream. If the current chunk was pushed with IFF_SIZE_UNKNOWN_32
|||	    or IFF_SIZE_UNKNOWN_64, the size of the chunk gets increased by the
|||	    size of the buffer written. If the size was specified for this chunk,
|||	    attempts to write past the end of the chunk will be truncated.
|||
|||	  Arguments
|||
|||	    iff
|||	        The parsing handle to write to.
|||
|||	    buffer
|||	        A pointer to the data to write to the stream.
|||
|||	    numBytes
|||	        The number of bytes of data to write.
|||
|||	  Return Value
|||
|||	    Returns the number of bytes written, or a negative error code for
|||	    failure.
|||
|||	  Implementation
|||
|||	    Folio call implemented in IFF folio V27.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    PushChunk(), PopChunk(), ReadChunk()
|||
**/

/**
|||	AUTODOC -disabled -class IFF -name WriteChunkCompressed
|||	Compresses data from a buffer and writes the result to the current
|||	chunk.
|||
|||	  Synopsis
|||
|||	    int32 WriteChunkCompressed(IFFParser *iff, const void *buffer,
|||	                               uint32 numBytes);
|||
|||	  Description
|||
|||	    This function writes the requested number of bytes into the IFF
|||	    stream. If the current chunk was pushed with IFF_SIZE_UNKNOWN_32
|||	    or IFF_SIZE_UNKNOWN_64, the size of the chunk gets increased by the
|||	    size of the buffer written. If the size was specified for this chunk,
|||	    attempts to write past the end of the chunk will be truncated.
|||	    The data is first compressed using the compression folio before
|||	    being written to the chunk.
|||
|||	  Arguments
|||
|||	    iff
|||	        The parsing handle to write to.
|||
|||	    buffer
|||	        A pointer to the data to write to the stream.
|||
|||	    numBytes
|||	        The number of bytes of data to write.
|||
|||	  Return Value
|||
|||	    Returns the number of bytes written, or a negative error code for
|||	    failure.
|||
|||	  Implementation
|||
|||	    Folio call implemented in IFF folio V27.
|||
|||	  Associated Files
|||
|||	    <misc/iff.h>, System.m2/Modules/iff
|||
|||	  See Also
|||
|||	    PushChunk(), PopChunk(), ReadChunk(), ReadChunkCompressed(),
|||	    SeekChunk(), WriteChunk()
|||
**/

/* keep the compiler happy... */
extern int foo;
