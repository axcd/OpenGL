#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
unsigned int texture;
glGenTextures(1, &amp;texture);
glActiveTexture(GL_TEXTURE0); // 在绑定纹理之前先激活纹理单元
glBindTexture(GL_TEXTURE_2D, texture);
// 为当前绑定的纹理对象设置环绕、过滤方式
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// 对y轴进行翻转
stbi_set_flip_vertically_on_load(true);
// 加载并生成纹理
int width, height, nrChannels;
unsigned char *data = (char *)stbi_load("container.jpg", &amp;width, &amp;height, &amp;nrChannels, 0);
if (data)
{
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
}
else
{

}
stbi_image_free(data);



glGenTextures(1, &textures[0]); //创建文理名称，这里是创建一个文理，也可以创建多个，第一个参数为文理个数   
glBindTexture(GL_TEXTURE_2D, textures[0]); //绑定纹理，第一个参数为作用目标，在O盆GLES里必须使用GL_TEXTURE_2D,因为OpenGLES只支持这个，第二个为需要绑定的纹理名称，即上面创建的  
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textImageWidth, textImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);//将图像数据加载到纹理中   
free(imageData);//释放内存

/*下面两行为设置纹理相关参数,GL_LINEAR参数能呈现一个平滑的、消除了锯齿的外观*/    
glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  //图像被缩小时GL_TEXTURE_MIN_FILTER,   
glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  //图像被放大时GL_TEXTURE_MAG_FILTER  
glEnable(GL_TEXTURE_2D); //开启2D纹理贴图功能 

//渲染方法\
(void)render{
    static GLfloat rotation = 0.0; //角度    
	static CGFloat spinnySquareVertices[12] = { 
												-0.5f, -0.5f, -1,
												0.5f,  -0.5f, -1,
												-0.5f,  0.5f, -1,
												0.5f,   0.5f, -1,
												};//    
	static CGFloat spinnSquareColor[] = {
											255,255,0,255,//        
											0,255,255,255,//        
											0,0,0,0,//        
											255,0,255,255,//            
											};
											
	const GLshort square[] = {
						        0,0,      
								1,0,      
								0,1,       
								1,1   
							};
								
	glClearColor(0.1f, 0.8f, 1.0f, 1.0f); //设置清屏颜色   
	glClear(GL_COLOR_BUFFER_BIT);  //清屏\n    
	glViewport(0, 0, backingWidth, backingHeight);  //设置视口\n \n   
	glLoadIdentity();  //清空当前矩阵,还原默认矩阵  \n    
	glOrthof(-1.0f, 1.0f, -1.5f, 1.5f, -1.0f, 1.0f);  //正交模式下可视区域 \n    
	glColor4f(1.0, 1.0, 1.0, 1.0);  //绘制的颜色\n    
	glVertexPointer(3, GL_FLOAT, 0, spinnySquareVertices); //确定使用的顶点坐标数列的位置和尺寸\n    
	glEnableClientState(GL_VERTEX_ARRAY);  //启动独立的客户端功能，告诉OpenGL将会使用一个由glVertexPointer定义的定点数组\n//    
	glColorPointer(4, GL_FLOAT, 0, spinnSquareColor);\n//    
	glEnableClientState(GL_COLOR_ARRAY);\n \n    
	glTexCoordPointer(2, GL_SHORT, 0, square); //new //纹理坐标.参数含义跟以上的方法大相迳庭\n    
	glEnableClientState(GL_TEXTURE_COORD_ARRAY); //new\n//    
	glShadeModel(GL_FLOAT);    
	glRotatef(rotation, 0.0, 0.0, 1.0);  //旋转  \n \n    
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); //进行连续不间断的渲染,在渲染缓冲区有了一个准备好的要渲染的图像\n    
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);    
	//rotation += 0.5;   
	//[_context presentRenderbuffer:GL_RENDERBUFFER_OES]; //将渲染缓冲区的内容呈现到屏幕上\n \n}
