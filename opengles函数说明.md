# 顶点属性：

顶点属性有两种

1. **Constant Vertex Attribute**所有顶点共享

   void **glVertexAttrib4f**(GLuint *index*, GLfloat *x*, GLfloat *y*, GLfloat *z*, GLfloat *w*);

   void **glVertexAttrib4fv**(GLuint *index*, const GLfloat **values*);

2. Vertex arrays 每个顶点都有自己各自的属性：

   void **glVertexAttribPointer**(

   GLuint *index*, 					//顶点shader里面这个属性的location

   GLint *size*, 						//顶点属性的components个数，比如顶点位置属性(x,y,z)有三个，就是3

   GLenum *type*, 				//当个components的类型，比如GL_FLOAT

   GLboolean *normalized*, //通常为GL_FALSE

   GLsizei *stride*, 				//ptr指定的内存里面，顶点n与n+1之间间隔的字节数。

   const void **ptr*)			//可以有两种值：1 顶点属性在内存中的地址，2如果当前绑定了vbo，则是指相对于vbo显存第一个字节的偏移数。





`void **glDrawElements**(`GLenum mode, GLsizei count, GLenum type, const void * indices`)`;

	//第一个参数指定绘制类型，GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP, GL_LINES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN and GL_TRIANGLES
	//第二个参数：是index数组里面的元素的个数，不是字节数，如果index是unsigned short类型 totalbyte/sizeof(unsigned short);
	//第三个参数，单个index的数据类型，GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, or GL_UNSIGNED_INT
	//第四个参数，是一个指针，如果没有绑定GL_ELEMENT_ARRAY_BUFFER，指的是内存中的地址，如果绑定了EAO，指相对于EAO显存第一个字节的偏移量。