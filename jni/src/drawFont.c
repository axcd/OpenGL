#define SDLCALL

#define RW_SEEK_SET 0       /**< Seek from the beginning of data */
#define RW_SEEK_CUR 1       /**< Seek relative to current read point */
#define RW_SEEK_END 2       /**< Seek relative to the end of data */

typedef int64_t Sint64;
typedef int32_t Sint32;
typedef uint32_t Uint32;

typedef struct SDL_RWops
{
    /**
     *  Return the size of the file in this rwops, or -1 if unknown
     */
    Sint64 (SDLCALL * size) (struct SDL_RWops * context);

    /**
     *  Seek to \c offset relative to \c whence, one of stdio's whence values:
     *  RW_SEEK_SET, RW_SEEK_CUR, RW_SEEK_END
     *
     *  \return the final offset in the data stream, or -1 on error.
     */
    Sint64 (SDLCALL * seek) (struct SDL_RWops * context, Sint64 offset,
                             int whence);

    /**
     *  Read up to \c maxnum objects each of size \c size from the data
     *  stream to the area pointed at by \c ptr.
     *
     *  \return the number of objects read, or 0 at error or end of file.
     */
    size_t (SDLCALL * read) (struct SDL_RWops * context, void *ptr,
                             size_t size, size_t maxnum);

    /**
     *  Write exactly \c num objects each of size \c size from the area
     *  pointed at by \c ptr to data stream.
     *
     *  \return the number of objects written, or 0 at error or end of file.
     */
    size_t (SDLCALL * write) (struct SDL_RWops * context, const void *ptr,
                              size_t size, size_t num);

    /**
     *  Close and free an allocated SDL_RWops structure.
     *
     *  \return 0 if successful or -1 on write error when flushing data.
     */
    int (SDLCALL * close) (struct SDL_RWops * context);

    Uint32 type;

    union
    {
        struct
        {
            void *asset;
        } androidio;
    } hidden;

} SDL_RWops;


size_t Android_JNI_FileRead(SDL_RWops* ctx, void* buffer,
        size_t size, size_t maxnum)
{
    size_t result;
    AAsset *asset = (AAsset*) ctx->hidden.androidio.asset;
    result = AAsset_read(asset, buffer, size * maxnum);

    if (result > 0) {
        /* Number of chuncks */
        return (result / size);
    } else {
        /* Error or EOF */
        return result;
    }
}



size_t Android_JNI_FileWrite(SDL_RWops *ctx, const void *buffer,
        size_t size, size_t num)
{
    //SDL_SetError("Cannot write to Android package filesystem");
    return 0;
}

Sint64 Android_JNI_FileSize(SDL_RWops *ctx)
{
    off64_t result;
    AAsset *asset = (AAsset*) ctx->hidden.androidio.asset;
    result = AAsset_getLength64(asset);
    return result;
}

Sint64 Android_JNI_FileSeek(SDL_RWops* ctx, Sint64 offset, int whence)
{
    off64_t result;
    AAsset *asset = (AAsset*) ctx->hidden.androidio.asset;
    result = AAsset_seek64(asset, offset, whence);
    return result;
}

int Android_JNI_FileClose(SDL_RWops *ctx)
{
    AAsset *asset = (AAsset*) ctx->hidden.androidio.asset;
    AAsset_close(asset);
    return 0;
}

size_t SDL_RWread(SDL_RWops *context, void *ptr, size_t size, size_t maxnum)
{
    return context->read(context, ptr, size, maxnum);
}

Sint64 SDL_RWtell(SDL_RWops *context)
{
    return context->seek(context, 0, RW_SEEK_CUR);
}

Sint64 SDL_RWseek(SDL_RWops *context, Sint64 offset, int whence)
{
    return context->seek(context, offset, whence);
}

static unsigned long RWread(FT_Stream stream, unsigned long offset, 
		unsigned char *buffer, unsigned long count)
{
    SDL_RWops *src;

    src = (SDL_RWops *)stream->descriptor.pointer;
    SDL_RWseek(src, (int)offset, RW_SEEK_SET);
    if (count == 0) {
        return 0;
    }
    return (unsigned long)SDL_RWread(src, buffer, 1, (int)count);
}

Sint64 SDL_RWsize(SDL_RWops *context)
{
    return context->size(context);
}

/*
 void* FT_Alloc_Func( FT_Memory memory, long size ){
	 memory->user = alloc( size );
 }
					
void FT_Free_Func( FT_Memory memory, void* block ){
	free(memory->user);
}
				   
void* FT_Realloc_Func( FT_Memory memory, long cur_size, long new_size, void* block ){
	realloc(memory->user, new_size);
}

unsigned long FT_Stream_IoFunc( FT_Stream stream, unsigned long offset, unsigned char* buffer, unsigned long count ){
	
}
					   
void FT_Stream_CloseFunc( FT_Stream  stream ){
	
}*/
					  
					 
