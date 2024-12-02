
   /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Open_Face                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Create a face object from a given resource described by            */
  /*    @FT_Open_Args.                                                     */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    library    :: A handle to the library resource.                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    args       :: A pointer to an `FT_Open_Args' structure that must   */
  /*                  be filled by the caller.                             */
  /*                                                                       */
  /*    face_index :: This field holds two different values.  Bits 0-15    */
  /*                  are the index of the face in the font file (starting */
  /*                  with value~0).  Set it to~0 if there is only one     */
  /*                  face in the font file.                               */
  /*                                                                       */
  /*                  [Since 2.6.1] Bits 16-30 are relevant to GX and      */
  /*                  OpenType variation fonts only, specifying the named  */
  /*                  instance index for the current face index (starting  */
  /*                  with value~1; value~0 makes FreeType ignore named    */
  /*                  instances).  For non-variation fonts, bits 16-30 are */
  /*                  ignored.  Assuming that you want to access the third */
  /*                  named instance in face~4, `face_index' should be set */
  /*                  to 0x00030004.  If you want to access face~4 without */
  /*                  variation handling, simply set `face_index' to       */
  /*                  value~4.                                             */
  /*                                                                       */
  /*                  `FT_Open_Face' and its siblings can be used to       */
  /*                  quickly check whether the font format of a given     */
  /*                  font resource is supported by FreeType.  In general, */
  /*                  if the `face_index' argument is negative, the        */
  /*                  function's return value is~0 if the font format is   */
  /*                  recognized, or non-zero otherwise.  The function     */
  /*                  allocates a more or less empty face handle in        */
  /*                  `*aface' (if `aface' isn't NULL); the only two       */
  /*                  useful fields in this special case are               */
  /*                  `face->num_faces' and `face->style_flags'.  For any  */
  /*                  negative value of `face_index', `face->num_faces'    */
  /*                  gives the number of faces within the font file.  For */
  /*                  the negative value `-(N+1)' (with `N' a non-negative */
  /*                  16-bit value), bits 16-30 in `face->style_flags'     */
  /*                  give the number of named instances in face `N' if we */
  /*                  have a variation font (or zero otherwise).  After    */
  /*                  examination, the returned @FT_Face structure should  */
  /*                  be deallocated with a call to @FT_Done_Face.         */
  /*                                                                       */
  /* <Output>                                                              */
  /*    aface      :: A handle to a new face object.  If `face_index' is   */
  /*                  greater than or equal to zero, it must be non-NULL.  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Unlike FreeType 1.x, this function automatically creates a glyph   */
  /*    slot for the face object that can be accessed directly through     */
  /*    `face->glyph'.                                                     */
  /*                                                                       */
  /*    Each new face object created with this function also owns a        */
  /*    default @FT_Size object, accessible as `face->size'.               */
  /*                                                                       */
  /*    One @FT_Library instance can have multiple face objects, this is,  */
  /*    @FT_Open_Face and its siblings can be called multiple times using  */
  /*    the same `library' argument.                                       */
  /*                                                                       */
  /*    See the discussion of reference counters in the description of     */
  /*    @FT_Reference_Face.                                                */
  /*                                                                       */
  /*    To loop over all faces, use code similar to the following snippet  */
  /*    (omitting the error handling).                                     */
  /*                                                                       */
  /*    {                                                                  */
  /*      ...                                                              */
  /*      FT_Face  face;                                                   */
  /*      FT_Long  i, num_faces;                                           */
  /*                                                                       */
  /*                                                                       */
  /*      error = FT_Open_Face( library, args, -1, &face );                */
  /*      if ( error ) { ... }                                             */
  /*                                                                       */
  /*      num_faces = face->num_faces;                                     */
  /*      FT_Done_Face( face );                                            */
  /*                                                                       */
  /*      for ( i = 0; i < num_faces; i++ )                                */
  /*      {                                                                */
  /*        ...                                                            */
  /*        error = FT_Open_Face( library, args, i, &face );               */
  /*        ...                                                            */
  /*        FT_Done_Face( face );                                          */
  /*        ...                                                            */
  /*      }                                                                */
  /*    }                                                                  */
  /*                                                                       */
  /*    To loop over all valid values for `face_index', use something      */
  /*    similar to the following snippet, again without error handling.    */
  /*    The code accesses all faces immediately (thus only a single call   */
  /*    of `FT_Open_Face' within the do-loop), with and without named      */
  /*    instances.                                                         */
  /*                                                                       */
  /*    {                                                                  */
  /*      ...                                                              */
  /*      FT_Face  face;                                                   */
  /*                                                                       */
  /*      FT_Long  num_faces     = 0;                                      */
  /*      FT_Long  num_instances = 0;                                      */
  /*                                                                       */
  /*      FT_Long  face_idx     = 0;                                       */
  /*      FT_Long  instance_idx = 0;                                       */
  /*                                                                       */
  /*                                                                       */
  /*      do                                                               */
  /*      {                                                                */
  /*        FT_Long  id = ( instance_idx << 16 ) + face_idx;               */
  /*                                                                       */
  /*                                                                       */
  /*        error = FT_Open_Face( library, args, id, &face );              */
  /*        if ( error ) { ... }                                           */
  /*                                                                       */
  /*        num_faces     = face->num_faces;                               */
  /*        num_instances = face->style_flags >> 16;                       */
  /*                                                                       */
  /*        ...                                                            */
  /*                                                                       */
  /*        FT_Done_Face( face );                                          */
  /*                                                                       */
  /*        if ( instance_idx < num_instances )                            */
  /*          instance_idx++;                                              */
  /*        else                                                           */
  /*        {                                                              */
  /*          face_idx++;                                                  */
  /*          instance_idx = 0;                                            */
  /*        }                                                              */
  /*                                                                       */
  /*      } while ( face_idx < num_faces )                                 */
  /*    }                                                                  */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Open_Face( FT_Library           library,
                const FT_Open_Args*  args,
                FT_Long              face_index,
                FT_Face             *aface );
				

 /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Open_Args                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure to indicate how to open a new font file or stream.  A  */
  /*    pointer to such a structure can be used as a parameter for the     */
  /*    functions @FT_Open_Face and @FT_Attach_Stream.                     */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    flags       :: A set of bit flags indicating how to use the        */
  /*                   structure.                                          */
  /*                                                                       */
  /*    memory_base :: The first byte of the file in memory.               */
  /*                                                                       */
  /*    memory_size :: The size in bytes of the file in memory.            */
  /*                                                                       */
  /*    pathname    :: A pointer to an 8-bit file pathname.                */
  /*                                                                       */
  /*    stream      :: A handle to a source stream object.                 */
  /*                                                                       */
  /*    driver      :: This field is exclusively used by @FT_Open_Face;    */
  /*                   it simply specifies the font driver to use for      */
  /*                   opening the face.  If set to NULL, FreeType tries   */
  /*                   to load the face with each one of the drivers in    */
  /*                   its list.                                           */
  /*                                                                       */
  /*    num_params  :: The number of extra parameters.                     */
  /*                                                                       */
  /*    params      :: Extra parameters passed to the font driver when     */
  /*                   opening a new face.                                 */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The stream type is determined by the contents of `flags' that      */
  /*    are tested in the following order by @FT_Open_Face:                */
  /*                                                                       */
  /*    If the @FT_OPEN_MEMORY bit is set, assume that this is a           */
  /*    memory file of `memory_size' bytes, located at `memory_address'.   */
  /*    The data are not copied, and the client is responsible for         */
  /*    releasing and destroying them _after_ the corresponding call to    */
  /*    @FT_Done_Face.                                                     */
  /*                                                                       */
  /*    Otherwise, if the @FT_OPEN_STREAM bit is set, assume that a        */
  /*    custom input stream `stream' is used.                              */
  /*                                                                       */
  /*    Otherwise, if the @FT_OPEN_PATHNAME bit is set, assume that this   */
  /*    is a normal file and use `pathname' to open it.                    */
  /*                                                                       */
  /*    If the @FT_OPEN_DRIVER bit is set, @FT_Open_Face only tries to     */
  /*    open the file with the driver whose handler is in `driver'.        */
  /*                                                                       */
  /*    If the @FT_OPEN_PARAMS bit is set, the parameters given by         */
  /*    `num_params' and `params' is used.  They are ignored otherwise.    */
  /*                                                                       */
  /*    Ideally, both the `pathname' and `params' fields should be tagged  */
  /*    as `const'; this is missing for API backward compatibility.  In    */
  /*    other words, applications should treat them as read-only.          */
  /*                                                                       */
  typedef struct  FT_Open_Args_
  {
    FT_UInt         flags;
    const FT_Byte*  memory_base;
    FT_Long         memory_size;
    FT_String*      pathname;
    FT_Stream       stream;
    FT_Module       driver;
    FT_Int          num_params;
    FT_Parameter*   params;

  } FT_Open_Args;

  
/*************************************************************************/
  /*                                                                       */
  /*                  M E M O R Y   M A N A G E M E N T                    */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************
   *
   * @type:
   *   FT_Memory
   *
   * @description:
   *   A handle to a given memory manager object, defined with an
   *   @FT_MemoryRec structure.
   *
   */
  typedef struct FT_MemoryRec_*  FT_Memory;


  /*************************************************************************
   *
   * @functype:
   *   FT_Alloc_Func
   *
   * @description:
   *   A function used to allocate `size' bytes from `memory'.
   *
   * @input:
   *   memory ::
   *     A handle to the source memory manager.
   *
   *   size ::
   *     The size in bytes to allocate.
   *
   * @return:
   *   Address of new memory block.  0~in case of failure.
   *
   */
  typedef void*
  (*FT_Alloc_Func)( FT_Memory  memory,
                    long       size );


  /*************************************************************************
   *
   * @functype:
   *   FT_Free_Func
   *
   * @description:
   *   A function used to release a given block of memory.
   *
   * @input:
   *   memory ::
   *     A handle to the source memory manager.
   *
   *   block ::
   *     The address of the target memory block.
   *
   */
  typedef void
  (*FT_Free_Func)( FT_Memory  memory,
                   void*      block );


  /*************************************************************************
   *
   * @functype:
   *   FT_Realloc_Func
   *
   * @description:
   *   A function used to re-allocate a given block of memory.
   *
   * @input:
   *   memory ::
   *     A handle to the source memory manager.
   *
   *   cur_size ::
   *     The block's current size in bytes.
   *
   *   new_size ::
   *     The block's requested new size.
   *
   *   block ::
   *     The block's current address.
   *
   * @return:
   *   New block address.  0~in case of memory shortage.
   *
   * @note:
   *   In case of error, the old block must still be available.
   *
   */
  typedef void*
  (*FT_Realloc_Func)( FT_Memory  memory,
                      long       cur_size,
                      long       new_size,
                      void*      block );


  /*************************************************************************
   *
   * @struct:
   *   FT_MemoryRec
   *
   * @description:
   *   A structure used to describe a given memory manager to FreeType~2.
   *
   * @fields:
   *   user ::
   *     A generic typeless pointer for user data.
   *
   *   alloc ::
   *     A pointer type to an allocation function.
   *
   *   free ::
   *     A pointer type to an memory freeing function.
   *
   *   realloc ::
   *     A pointer type to a reallocation function.
   *
   */
  struct  FT_MemoryRec_
  {
    void*            user;
    FT_Alloc_Func    alloc;
    FT_Free_Func     free;
    FT_Realloc_Func  realloc;
  };


  /*************************************************************************/
  /*                                                                       */
  /*                       I / O   M A N A G E M E N T                     */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************
   *
   * @type:
   *   FT_Stream
   *
   * @description:
   *   A handle to an input stream.
   *
   * @also:
   *   See @FT_StreamRec for the publicly accessible fields of a given
   *   stream object.
   *
   */
  typedef struct FT_StreamRec_*  FT_Stream;


  /*************************************************************************
   *
   * @struct:
   *   FT_StreamDesc
   *
   * @description:
   *   A union type used to store either a long or a pointer.  This is used
   *   to store a file descriptor or a `FILE*' in an input stream.
   *
   */
  typedef union  FT_StreamDesc_
  {
    long   value;
    void*  pointer;

  } FT_StreamDesc;


  /*************************************************************************
   *
   * @functype:
   *   FT_Stream_IoFunc
   *
   * @description:
   *   A function used to seek and read data from a given input stream.
   *
   * @input:
   *   stream ::
   *     A handle to the source stream.
   *
   *   offset ::
   *     The offset of read in stream (always from start).
   *
   *   buffer ::
   *     The address of the read buffer.
   *
   *   count ::
   *     The number of bytes to read from the stream.
   *
   * @return:
   *   The number of bytes effectively read by the stream.
   *
   * @note:
   *   This function might be called to perform a seek or skip operation
   *   with a `count' of~0.  A non-zero return value then indicates an
   *   error.
   *
   */
  typedef unsigned long
  (*FT_Stream_IoFunc)( FT_Stream       stream,
                       unsigned long   offset,
                       unsigned char*  buffer,
                       unsigned long   count );


  /*************************************************************************
   *
   * @functype:
   *   FT_Stream_CloseFunc
   *
   * @description:
   *   A function used to close a given input stream.
   *
   * @input:
   *  stream ::
   *     A handle to the target stream.
   *
   */
  typedef void
  (*FT_Stream_CloseFunc)( FT_Stream  stream );


  /*************************************************************************
   *
   * @struct:
   *   FT_StreamRec
   *
   * @description:
   *   A structure used to describe an input stream.
   *
   * @input:
   *   base ::
   *     For memory-based streams, this is the address of the first stream
   *     byte in memory.  This field should always be set to NULL for
   *     disk-based streams.
   *
   *   size ::
   *     The stream size in bytes.
   *
   *     In case of compressed streams where the size is unknown before
   *     actually doing the decompression, the value is set to 0x7FFFFFFF.
   *     (Note that this size value can occur for normal streams also; it is
   *     thus just a hint.)
   *
   *   pos ::
   *     The current position within the stream.
   *
   *   descriptor ::
   *     This field is a union that can hold an integer or a pointer.  It is
   *     used by stream implementations to store file descriptors or `FILE*'
   *     pointers.
   *
   *   pathname ::
   *     This field is completely ignored by FreeType.  However, it is often
   *     useful during debugging to use it to store the stream's filename
   *     (where available).
   *
   *   read ::
   *     The stream's input function.
   *
   *   close ::
   *     The stream's close function.
   *
   *   memory ::
   *     The memory manager to use to preload frames.  This is set
   *     internally by FreeType and shouldn't be touched by stream
   *     implementations.
   *
   *   cursor ::
   *     This field is set and used internally by FreeType when parsing
   *     frames.
   *
   *   limit ::
   *     This field is set and used internally by FreeType when parsing
   *     frames.
   *
   */
  typedef struct  FT_StreamRec_
  {
    unsigned char*       base;
    unsigned long        size;
    unsigned long        pos;

    FT_StreamDesc        descriptor;
    FT_StreamDesc        pathname;
    FT_Stream_IoFunc     read;
    FT_Stream_CloseFunc  close;

    FT_Memory            memory;
    unsigned char*       cursor;
    unsigned char*       limit;

  } FT_StreamRec;

  /* */


