#include <stdlib.h>
#include <jni.h>
#include <errno.h>
#include <wchar.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#include <ft2build.h>
#include FT_FREETYPE_H  
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "log.h"
#include "drawFont.c"

#define SDL_RWOPS_JNIFILE  3U

AAsset *asset;
Sint64 position;

/**
 * Our saved state data.
 */
struct saved_state {
    float angle;
    int32_t x;
    int32_t y;
};

/**
 * Shared state for our app.
 */
struct engine {
    struct android_app* app;
	
	AAssetManager* aassetManager;   //自己加入代码
    
	ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;

    int animating;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    struct saved_state state;
};

/**
 *  draw XY
 */
static void drawXY(){
	
	static GLfloat rotation=0.0;
	GLfloat flayout[] = {   
	    -0.97,  0,
         0.97,  0,
         0,    -0.97,
         0,     0.9,
         0.97,  0,
         0.93, -0.01,
         0.97,  0,
         0.93,  0.01,
		 0,     0.9,
		-0.02,  0.88,
		 0,      0.9,
		 0.02, 0.88
	};
	
	GLubyte colorArray[] = {
		255, 0, 0, 0,
		0, 255, 0, 0,
		0, 0, 255, 0,
		0, 255, 255, 0,
		255, 0, 255, 0,
		255, 255, 0, 0
	};

	glLoadIdentity();
    
	glLineWidth(10);
	glPointSize(50);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, flayout);
	
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, colorArray);
	
	//glRotatef(rotation,0.0,0.0,1.0);
	glDrawArrays(GL_LINES, 0, 10);
	//rotation += 0.5;
	
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);   
}

//绘制三角形
static void draw(){
	
	static GLfloat rotation=0.0;
	GLubyte colorArray[] = {
		255, 0, 0, 0,
		0, 255, 0, 0,
		0, 0, 255, 0,
		0, 255, 255, 0,
		255, 0, 255, 0,
		255, 255, 0, 0
	};
	
	GLfloat flayout[] = {   
	    -0.6, -0.6,
         0.1, -0.6,
        -0.2,  0.1,
         0.3, -0.3,
         0.1,  0.2,
        -0.6,  0.5
	};
	glLoadIdentity();

	glLineWidth(10);
	glPointSize(50);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, flayout);
	
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, colorArray);
	
	glRotatef(rotation,0.0,0.0,1.0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	rotation += 0.5;
	
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);   
}

//draw A
int drawA(float r, float g, float b){
	
	FT_Library  ft;
	
	wchar_t *wch = L"A";
	wch = L"毛";
	
clean_log();
	
	if (FT_Init_FreeType(&ft)){
		printf("ERROR::FREETYPE: Could not init FreeType Library");
		return -1;
	}

	FT_Face face;
	
	char *file = "/storage/emulated/0/AppProjects/gles/assets/fonts/GB2312.ttf";

	if (FT_New_Face(ft, file, 0, &face)){
		printf( "ERROR::FREETYPE: Failed to load font");
		return -1;
	}
	
	FT_Select_Charmap(face, FT_ENCODING_UNICODE);
	FT_UInt index = FT_Get_Char_Index(face, wch[0]);
	//FT_Set_Pixel_Sizes(face, 500, 500); 
	FT_Set_Char_Size(face, 0, 64*64, 300, 300);
	
	// 字节对齐
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	if (FT_Load_Char(face, index, FT_LOAD_RENDER)){
		printf("ERROR::FREETYTPE: Failed to load Glyph");  
		return -1;
	}
    
	//加载bitmap
	FT_Load_Glyph(face, index, FT_LOAD_DEFAULT);
	FT_Glyph glyph;
	FT_Get_Glyph(face->glyph, &glyph);
	FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph) glyph;
	
    //This reference will make accessing the bitmap easier
    FT_Bitmap &bitmap = bitmap_glyph->bitmap;
	
	static unsigned char* bitmapBuffer = NULL;
	
	if(bitmapBuffer == NULL){
		//把像素有1个字节变成4个字节
		bitmapBuffer = (unsigned char*)calloc(bitmap.rows,bitmap.width*4);	
		for(int i = 0; i < bitmap.rows; i++){
			for(int j = 0; j < bitmap.width; j++){
				if(bitmap.buffer[i*bitmap.width+j] == 0){	
					bitmapBuffer[4*(i*bitmap.width+j)] = 255;      // red
					bitmapBuffer[4*(i*bitmap.width+j)+1] = 225;    // green
					bitmapBuffer[4*(i*bitmap.width+j)+2] = 255;    // blue
					bitmapBuffer[4*(i*bitmap.width+j)+3] = 0;    //alpha
			
				}else{		
					bitmapBuffer[4*(i*bitmap.width+j)] = 0;
					bitmapBuffer[4*(i*bitmap.width+j)+1] = 0;	
					bitmapBuffer[4*(i*bitmap.width+j)+2] = 0;			
					bitmapBuffer[4*(i*bitmap.width+j)+3] = 255;
				}
			}
		}
	}
	
	static GLfloat rotation=0.0;
			
	glLoadIdentity();//清空当前矩阵，还原默认矩阻 
	glOrthof(-1.0f,1.0f,-1.5f,1.5f,-1.5f,1.5f);//正交模式下可视区域
	glColor4f(r, g, b, 0.0);
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
	
    glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RGBA,
                bitmap.width,
                bitmap.rows,
                0,
				GL_RGBA,  
                GL_UNSIGNED_BYTE,
                bitmapBuffer
    );

	GLfloat vx = 0.5f, vy = 0.5f;
	static GLfloat vertices[] = {
								   -vx, -vy,
									vx, -vy,
									-vx,  vy,
									vx,   vy,
								 };    
  
	const GLshort square[] = { 0,1,
							   1,1,
						  	    0,0,      
								1,0, 
								};
		
	glVertexPointer(2, GL_FLOAT, 0, vertices); //确定使用的顶点坐标数列的位置和尺寸
	glEnableClientState(GL_VERTEX_ARRAY);  //启动独立的客户端功能，告诉OpenGL将会使用一个由glVertexPointer定义的定点数组    
	
	glTexCoordPointer(2, GL_SHORT, 0, square);  //纹理坐标.参数含义跟以上的方法大相迳庭
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);  
	
	glRotatef(rotation,0.0,0.0,1.0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); //进行连续不间断的渲染,在渲染缓冲区有了一个准备好的要渲染的图像
	rotation += 0.5;
	
	glDeleteTextures(1, &texture);
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	//free(bitmapBuffer);
	//bitmapBuffer=NULL;
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
	return 0;
}

//draw B
int drawB(float r, float g, float b, AAssetManager* asset_manager){
	
	FT_Library  ft;
	char ch = 'A';
	wchar_t *wch = L"毛";

	asset = AAssetManager_open(asset_manager, "fonts/GB2312.ttf", AASSET_MODE_UNKNOWN);

	static FT_Stream stream = NULL;
	static FT_Open_Args *args = NULL;
	static SDL_RWops *rwops = NULL;

	if(rwops == NULL)
		rwops = (SDL_RWops *)malloc(sizeof *rwops);
	if(stream == NULL)
		stream = (FT_Stream)malloc(sizeof(*stream));
	if(args == NULL)
		args = (FT_Open_Args *)malloc(sizeof(*args));

    memset(stream, 0, sizeof(*stream));
	memset(args, 0, sizeof(*args));

	rwops->hidden.androidio.asset = (void*) asset;
	rwops->size = Android_JNI_FileSize;
    rwops->seek = Android_JNI_FileSeek;
    rwops->read = Android_JNI_FileRead;
    rwops->write = Android_JNI_FileWrite;
    rwops->close = Android_JNI_FileClose;
    rwops->type = SDL_RWOPS_JNIFILE;

	position = SDL_RWtell(rwops);

	stream->read = RWread;
    stream->descriptor.pointer = rwops;
    stream->pos = (unsigned long)position;
    stream->size = (unsigned long)(SDL_RWsize(rwops) - position);

	args->flags = FT_OPEN_STREAM;
	args->stream = stream;
	
	if (FT_Init_FreeType(&ft)){
		printf("ERROR::FREETYPE: Could not init FreeType Library");
		return -1;
	}
	
	FT_Face face;
/*
	if (FT_Open_Face(ft, args, -1, &face)){
		printf( "ERROR::FREETYPE: Failed to load font");
		return -1;
	}
*/
	if (FT_Open_Face(ft, args, 0, &face)){
		printf( "ERROR::FREETYPE: Failed to load font");
		return -1;
	}

	FT_Select_Charmap(face, FT_ENCODING_UNICODE);
	FT_UInt index = FT_Get_Char_Index(face, wch[0]);
	FT_Set_Pixel_Sizes(face, 500, 500); 
	//FT_Set_Char_Size(face, 0, 64*64, 300, 300);

	// 字节对齐
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	if (FT_Load_Char(face, index, FT_LOAD_RENDER)){
		printf("ERROR::FREETYTPE: Failed to load Glyph");  
		return -1;
	}
    
	//加载bitmap
	FT_Load_Glyph(face, index, FT_LOAD_DEFAULT);
	FT_Glyph glyph;
	FT_Get_Glyph(face->glyph, &glyph);
	FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph) glyph;
	
    //This reference will make accessing the bitmap easier
    FT_Bitmap &bitmap = bitmap_glyph->bitmap;
	
	static unsigned char* bitmapBuffer = NULL;
	
	if(bitmapBuffer == NULL){
		//把像素有1个字节变成4个字节
		bitmapBuffer = (unsigned char*)calloc(bitmap.rows,bitmap.width*4);	
		for(int i = 0; i < bitmap.rows; i++){
			for(int j = 0; j < bitmap.width; j++){
				if(bitmap.buffer[i*bitmap.width+j] == 0){	
					bitmapBuffer[4*(i*bitmap.width+j)] = 255;      // red
					bitmapBuffer[4*(i*bitmap.width+j)+1] = 225;    // green
					bitmapBuffer[4*(i*bitmap.width+j)+2] = 255;    // blue
					bitmapBuffer[4*(i*bitmap.width+j)+3] = 0;    //alpha
			
				}else{		
					bitmapBuffer[4*(i*bitmap.width+j)] = 255;
					bitmapBuffer[4*(i*bitmap.width+j)+1] = 0;	
					bitmapBuffer[4*(i*bitmap.width+j)+2] = 0;			
					bitmapBuffer[4*(i*bitmap.width+j)+3] = 0;
				}
			}
		}
	}
	
	static GLfloat rotation=0.0;
			
	glLoadIdentity();//清空当前矩阵，还原默认矩阻 
	glOrthof(-1.0f,1.0f,-1.5f,1.5f,-1.5f,1.5f);//正交模式下可视区域
	glColor4f(r, g, b, 0.0);
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
	
    glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RGBA,
                bitmap.width,
                bitmap.rows,
                0,
				GL_RGBA,  
                GL_UNSIGNED_BYTE,
                bitmapBuffer
    );

	GLfloat vx = 0.5f, vy = 0.5f;
	static GLfloat vertices[] = {
								   -vx, -vy,
									vx, -vy,
									-vx,  vy,
									vx,   vy,
								 };    
  
	const GLshort square[] = { 0,1,
							   1,1,
						  	    0,0,      
								1,0, 
								};
		
	glVertexPointer(2, GL_FLOAT, 0, vertices); //确定使用的顶点坐标数列的位置和尺寸
	glEnableClientState(GL_VERTEX_ARRAY);  //启动独立的客户端功能，告诉OpenGL将会使用一个由glVertexPointer定义的定点数组    
	
	glTexCoordPointer(2, GL_SHORT, 0, square);  //纹理坐标.参数含义跟以上的方法大相迳庭
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);  
	
	glRotatef(rotation,0.0,0.0,1.0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); //进行连续不间断的渲染,在渲染缓冲区有了一个准备好的要渲染的图像
	rotation += 0.5;
	
	glDeleteTextures(1, &texture);
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	//free(bitmapBuffer);
	//bitmapBuffer=NULL;

	free(rwops);
	rwops = NULL;
	free(stream);
	stream = NULL;
	free(args);
	args = NULL;
	
	AAsset_close(asset);
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
	return 0;
}

//获取Texture2D
void getImageTexture2D(AAssetManager* mgr, char* filename, int *width, int *height, int *nrChannels){
	AAsset* asset = AAssetManager_open(mgr, filename, AASSET_MODE_UNKNOWN);
	if(asset==NULL){
		exit(0);
	}
	
	off_t bufferSize = AAsset_getLength(asset);
	char* buffer=(char *)malloc(bufferSize+1);
	int numBytesRead = AAsset_read(asset, buffer, bufferSize);
	
	unsigned char* data = stbi_load_from_memory((const stbi_uc*)buffer, bufferSize, width, height, nrChannels, 0);
	if(data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, *width, *height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }else{
		exit(0);
	}
	
	AAsset_close(asset);
	stbi_image_free(data);
}

//绘制图片
void drawTexture(AAssetManager* mgr, char* filename){
	
	unsigned int texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0); // 在绑定纹理之前先激活纹理单元
	glBindTexture(GL_TEXTURE_2D, texture); // 为当前绑定的纹理对象设置环绕、过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glEnable(GL_TEXTURE_2D); //开启2D纹理贴图功能 
	
	GLfloat vx = 1.0f, vy = 1.0f;
	int width, height, nrChannels;
	getImageTexture2D(mgr, filename, &width, &height, &nrChannels);
	
	if(1.0f*1080/2400 < 1.0f*width/height) vy = 1.0f*1080/2400*height/width;
	else vx = 1.0f*2400/1080*width/height;
	
	static GLfloat rotation=0.0;
	//渲染方法
	static GLfloat vertices[] = {
								   -vx, -vy,
									vx, -vy,
									-vx,  vy,
									vx,   vy,
								};    
	
	const GLshort square[] = {  0,1,
								1,1,
						  	    0,0,      
								1,0,
							};
	glLoadIdentity();//清空当前矩阵，还原默认矩阻 
	
	glOrthof(-1.0f,1.0f,-1.5f,1.5f,-1.5f,1.5f);//正交模式下可视区域
	glColor4f(1.0, 1.0, 1.0, 1.0);
	
	glVertexPointer(2, GL_FLOAT, 0, vertices); //确定使用的顶点坐标数列的位置和尺寸
	glEnableClientState(GL_VERTEX_ARRAY);  //启动独立的客户端功能，告诉OpenGL将会使用一个由glVertexPointer定义的定点数组    
	
	glTexCoordPointer(2, GL_SHORT, 0, square);  //纹理坐标.参数含义跟以上的方法大相迳庭
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);  
	
	glRotatef(rotation,0.0,0.0,1.0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); //进行连续不间断的渲染,在渲染缓冲区有了一个准备好的要渲染的图像
	rotation += 0.5;
	
	glDeleteTextures(1, &texture);
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

