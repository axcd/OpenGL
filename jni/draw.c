#include <stdio.h>
#include <stdlib.h>
#include <jni.h>
#include <errno.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "log.h"

//获取Texture2D
void getImageTexture2D(AAssetManager* mgr, char* filename, int *width, int *height, int *nrChannels)
{
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

//
static void drawHI()
{
	static GLfloat fv = 4;
	if(fv<-6) fv = 4;
	//fv = 0;
	GLubyte colorArray[] = {
		255, 0, 0, 0,
		255, 255, 0, 0,
		0, 0, 255, 0,
		0, 255, 255, 0,
	};
	
	GLfloat flayout[] = { 
		-0.5+fv, -0.5,
		-0.5+fv, -0.25,
		-0.5+fv,  0,
		-0.5+fv,  0.25,
		-0.5+fv,  0.5,
		-0.25+fv,     0,
		  0+fv,     0,
		 0.25+fv,     0,
		 0.5+fv,  -0.5,
		 0.5+fv,  -0.25,
		 0.5+fv,  0,
		 0.5+fv,  0.25,
		 0.5+fv,  0.5,
		 
		 1.2+fv,  0.5,
		// 0.7+fv,  -0.25,
		 1.2+fv,  0,
		 1.2+fv,  -0.25,
		 1.2+fv,  -0.5,
	};
	
    glEnableClientState(GL_VERTEX_ARRAY);
	//glEnableClientState(GL_COLOR_ARRAY);
	glColor4f(1.0,0.0,0.0,1.0);
	glLineWidth(10);
	glPointSize(100);
	glVertexPointer(2, GL_FLOAT, 0, flayout);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, colorArray);
	
	glDrawArrays(GL_POINTS, 0, 17);
	
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	fv -= 0.05;
}

static void draw()
{
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
	
    glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	
	glLineWidth(10);
	glPointSize(50);
	glVertexPointer(2, GL_FLOAT, 0, flayout);
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, colorArray);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}


void drawTexture(AAssetManager* mgr){
	
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
	getImageTexture2D(mgr, "container.jpg", &width, &height, &nrChannels);
	
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
	glColor4f(1.0,0.0,1.0,1.0);//绘制的颜色
	
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

/**
 *  draw XY
 */
static void drawXY()
{
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
	
    glEnableClientState(GL_VERTEX_ARRAY);
	
	glLineWidth(10);
	glPointSize(50);
	glVertexPointer(2, GL_FLOAT, 0, flayout);
	
	glDrawArrays(GL_LINES, 0, 12);
	
	glDisableClientState(GL_VERTEX_ARRAY);
}

