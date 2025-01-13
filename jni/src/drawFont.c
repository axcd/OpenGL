#define SDLCALL

#define RW_SEEK_SET 0       /**< Seek from the beginning of data */
#define RW_SEEK_CUR 1       /**< Seek relative to current read point */
#define RW_SEEK_END 2       /**< Seek relative to the end of data */
#define SDL_RWOPS_JNIFILE  3U

typedef int64_t Sint64;
typedef int32_t Sint32;
typedef uint32_t Uint32;

AAsset *asset;
FT_Library  ft;
FT_Face face;

typedef struct{
	unsigned int rows;
	unsigned int width;
	unsigned char* pchar;
} CharBuffer;

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

int doneCharBuffer( CharBuffer *bch ){
	free( bch->pchar );
	free( bch );
	bch = NULL;
	return 0;
}

int lenWch(wchar_t *wch){
	int i = 0;
	while(memcmp(wch, "\0", 1)!=0){
		wch++;
		i++;
	}
	return i;
}

int getFace(AAssetManager* asset_manager){
	
	asset = AAssetManager_open(asset_manager, "GB2312.ttf", AASSET_MODE_UNKNOWN);
	
	if(asset==NULL){
		return -1;
	}
	
	unsigned char* bitmapBuffer = NULL;
	
	SDL_RWops *rwops = (SDL_RWops *)malloc(sizeof(*rwops));
	FT_Stream stream = (FT_Stream)malloc(sizeof(*stream));
	FT_Open_Args *args = (FT_Open_Args *)malloc(sizeof(*args));
	face = (FT_Face)malloc(sizeof(*face));

	if(rwops == NULL || stream == NULL || args == NULL)
		return -1;

    memset(stream, 0, sizeof(*stream));
	//memset(args, 0, sizeof(*args));

	rwops->hidden.androidio.asset = (void*) asset;
	rwops->size = Android_JNI_FileSize;
    rwops->seek = Android_JNI_FileSeek;
    rwops->read = Android_JNI_FileRead;
    rwops->write = Android_JNI_FileWrite;
    rwops->close = Android_JNI_FileClose;
    rwops->type = SDL_RWOPS_JNIFILE;

	Sint64 position = SDL_RWtell(rwops);

	stream->read = RWread;
    stream->descriptor.pointer = rwops;
    stream->pos = (unsigned long)position;
    stream->size = (unsigned long)(SDL_RWsize(rwops) - position);

	args->flags = FT_OPEN_STREAM;
	args->stream = stream;
	
	if (FT_Init_FreeType(&ft)){
		return -1;
	}
	
	if (FT_Open_Face(ft, args, 0, &face)){
		return -1;
	}
	
	FT_Select_Charmap(face, FT_ENCODING_UNICODE);
	FT_Set_Pixel_Sizes(face, 480, 640); 
	
	free(rwops);
	free(stream);
	free(args);
	
	return 0;
}

CharBuffer* charBuffer( AAssetManager* asset_manager, wchar_t *wch, int i ){
	
	getFace(asset_manager);
	FT_UInt index = FT_Get_Char_Index(face, wch[i]);
	
	//FT_Set_Char_Size(face, 0, 64*64, 300, 300);
	
	// 字节对齐
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	if (FT_Load_Char(face, index, FT_LOAD_RENDER)){ 
		return NULL;
	}
	
	//加载bitmap
	FT_Load_Glyph(face, index, FT_LOAD_DEFAULT);
	FT_Glyph glyph;
	FT_Get_Glyph(face->glyph, &glyph);
	FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph) glyph;
	
    //This reference will make accessing the bitmap easier
    FT_Bitmap &bitmap = bitmap_glyph->bitmap;
	
	CharBuffer *bchar = (CharBuffer *)malloc(sizeof(CharBuffer));
	unsigned char *pch= (unsigned char *)calloc(bitmap.rows, bitmap.width);
	
	if(bchar!=NULL && pch!=NULL){
		memcpy(pch, bitmap.buffer, bitmap.rows*bitmap.width);
		bchar->pchar = pch;
		bchar->rows = bitmap.rows;
		bchar->width = bitmap.width;
	}
	
	if(bchar==NULL || pch==NULL){
		if(bchar!=NULL)free(bchar);
		if(pch!=NULL)free(pch);
	}
	
	return bchar;
}

unsigned char * fontTex(unsigned char *src, unsigned int srows,unsigned int swidth, unsigned int drows,unsigned int dwidth){
	unsigned char *dest = (unsigned char *)calloc(drows, dwidth);
	memset((void *)dest, 0, (drows)*(dwidth));
	unsigned int x1 = 13, x2 = 13;
	unsigned int y1 = 13, y2 = 13;
	unsigned int w = dwidth-x1-x2, h = drows-y1-y2;
	unsigned char *tmp = dest;
	
	tmp += y1*dwidth;
	
	for(int i=0; i<h; i++){
		tmp += x1;
		for(int j=0; j<w; j++){
			*tmp++ = src[i*srows/h*swidth+j*swidth/w];
		}
		tmp += x2;
	}
	return dest;
}

static unsigned int rows;
static unsigned int width;
static unsigned char* bitmapBuffer = NULL;
	
//draw B  从assert文件夹里获取ttf
int drawB0(AAssetManager* asset_manager){
	
	if(bitmapBuffer == NULL){
		
		rows = 2400; 
    	width = 1080; 
		unsigned int  r = 120, w = 85;
		
		//把像素有1个字节变成4个字节
		bitmapBuffer = (unsigned char*)calloc(1080*2400, 4);
		memset(bitmapBuffer, 205, 1080*2400*4);
	
		wchar_t *wch = L"今天天气不错好好休息";
		int n = lenWch(wch);
		
		for(int k=0; k<n; k++){
			unsigned char* p = NULL;
			unsigned char* p1 = bitmapBuffer;
			CharBuffer *bc = charBuffer(asset_manager, wch, k);
			unsigned char* bitbuffer = fontTex(bc->pchar, bc->rows, bc->width, r, w);
			
			p1 += width*2*200;
			for(int i = 0; i < r; i++){
				p = p1;
				p += 200+4*100*k;
				for(int j = 0; j < w; j++){
					if(bitbuffer[i*w+j] == 0){
						*p++ = 255;
						*p++ = 0;
						*p++ = 0;
						*p++ = 0;
					}else{
						*p++ = 0;
						*p++ = 0;
						*p++ = 0;
						*p++ = 0;
					}
				}
				p1 += 4*width;
			}
			doneCharBuffer(bc);
		}
		FT_Done_Face(face);
		FT_Done_FreeType(ft);
	}
	
	return 0;
}

int drawB(float r, float g, float b, AAssetManager* asset_manager){
	static GLfloat rotation=0.0;
			
	glLoadIdentity();//清空当前矩阵，还原默认矩阻 
	//glOrthof(-1.0f,1.0f,-1.5f,1.5f,-1.5f,1.5f);//正交模式下可视区域
	//glColor4f(r, g, b, 0.0);
	//glColor4f(1.0, 0.0, 0.0, 1.0);
	
	GLuint texture;
    glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glEnable(GL_TEXTURE_2D); 
	glEnable(GL_BLEND);
	
    glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RGBA,
                1080,
                2400,
                0,
				GL_RGBA,  
                GL_UNSIGNED_BYTE,
                bitmapBuffer
    );

	GLfloat vx = 1.0f, vy = 1.0f;
	static GLfloat vertices[] = {
								   -vx, -vy,
									vx, -vy,
									-vx,  vy,
									vx,   vy,
								 };    
  
	const GLshort square[] = { 0,1,
							   1,1,
						  	   0,0,      
							   1,0, };
	
	glVertexPointer(2, GL_FLOAT, 0, vertices); //确定使用的顶点坐标数列的位置和尺寸
	glEnableClientState(GL_VERTEX_ARRAY);  //启动独立的客户端功能，告诉OpenGL将会使用一个由glVertexPointer定义的定点数组    
	
	glTexCoordPointer(2, GL_SHORT, 0, square);  //纹理坐标.参数含义跟以上的方法大相迳庭
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);  
	
	glRotatef(rotation,0.0,0.0,0.0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); //进行连续不间断的渲染,在渲染缓冲区有了一个准备好的要渲染的图像
	//rotation += 0.5;
	
	glDeleteTextures(1, &texture);
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	return 0;
}
