# 绘制的一般流程：

1. 创建vbo，把内存中各种顶点属性数据传到显存

2. 创建shader，编译连接成program

3. 创建vao，把shader里面的顶点属性，指向相应的vbo，打开ebo的过程打包到vao

   glBindVertexArray(mVAO);

   glBindBuffer(GL_ARRAY_BUFFER, mNorVbo);
   glEnableVertexAttribArray(norloc);
   glVertexAttribPointer(norloc, 3, GL_FLOAT, GL_FALSE, 0, 0); //上面这三个函数是一对，将shader里面顶点属性指向vbo

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexVbo);

   glBindVertexArray(0);

4. bind VAO,绘制图形

   glBindVertexArray(mVAO);
   glDrawElements(GL_TRIANGLES, mIndexByteSize/sizeof(GLushort), GL_UNSIGNED_SHORT, (const void*)0);
   glBindVertexArray(0);

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

   GLsizei *stride*, 				//ptr指定的内存或显存里面，顶点n与n+1之间间隔的字节数。

   const void **ptr*)			//可以有两种值：1 顶点属性在内存中的地址，2如果当前绑定了vbo，则是指相对于vbo显存第一个字节的偏移数。





void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void * indices);

	//第一个参数指定绘制类型，GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP, GL_LINES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN and GL_TRIANGLES
	//第二个参数：是index数组里面的元素的个数，不是字节数，如果index是unsigned short类型 totalbyte/sizeof(unsigned short);
	//第三个参数，单个index的数据类型，GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, or GL_UNSIGNED_INT
	//第四个参数，是一个指针，如果没有绑定GL_ELEMENT_ARRAY_BUFFER，指的是内存中的地址，如果绑定了EAO，指相对于EAO显存第一个字节的偏移量。

# 点精灵GL_POINTS

点精灵（GL_POINTS primitive）其实是个和屏幕对齐的小正方形

绘制点精灵的时候，gl_PointSize是顶点shader里面一个内置的变量，顶点shader里面也要 output gl_PointSize，不然会有绘制错误。

gles3.0的坐标原点是屏幕的左下角，但是绘制点精灵的时候，坐标原点是左上角。



在绘制点精灵的时候，vec2 gl_PointCoord是片段shader的内置变量，范围为【0，1】

# 顶点shader到片段shader之间的操作：

<img src="\doc\顶点shader到片段shader渲染流程.png"/>

#### clip操作：

​	把各种图元，与视锥的上下左右前后六个面做裁剪，剪掉不在视锥里面的部分。

#### 透视除操作：

​	就是x/=w, y/=w, z/=w，这样x,y,z的范围都在[-1，1]之间了，就是normalized坐标了

#### 窗口坐标：

1. normalized后的xy被转换为真实的屏幕坐标，normalized后的z值根据glDepthRangef指定的值来转换为相应的窗口坐标。最后这个Z值是不是就是深度缓存里面的值？归一化坐标转换为窗口坐标，与这个函数有关：**glViewport**(GLint *x,* GLint *y,* GLsizei *w,* GLsizei *h*)，

![](\doc\窗口坐标变换.png)

2. 公式里面 Ox = *x* + *w*/2 and Oy = *y* + *h*/2。n，f的值由void **glDepthRangef**(GLclampf *n,* GLclampf *f*)，这个函数来指定的。

#### 正反面剔除：

​	void **glFrontFace**(GLenum *dir*)；GL_CW or GL_CCW，default value is GL_CCW（逆时针）

​	void **glCullFace**(GLenum *mode*)

​	void **glEnable**(GLenum *cap*)

​	void **glDisable**(GLenum *cap*)   GL_CULL_FACE

#### 最后一步Rasterization：

![](\doc\rasterization.png)

1. 各种顶点属性都会被光栅化，光栅化后的属性都赋值给了fragment

2. Polygon Offset，两个距离很近的三角形，由于深度的精度原因，可能会出现artifact，使用多边形偏移来解决

   glEnable(GL_POLYGON_OFFSET_FILL)，

   glPolygonOffset(GLfloat *factor*, GLfloat *units*)

   depth offset = *m \* factor* + r * units

# 顶点Shader：

顶点shader可以使用Samplers

1. 顶点shader内置变量：

   gl_VertexID，输入变量，顶点的索引，整数，高精度

   gl_InstanceID，输入变量，

   gl_Position，输出变量，以clip coordinates输出

   gl_PointSize，输出变量，highp float以像素为单位，指定点精灵的size，clamped to the aliased point size range

   gl_FrontFacing，bool类型，

   gl_DepthRange，

   ```
   struct gl_DepthRangeParameters
   
   {
   
    highp float near; // near Z
   
    highp float far; // far Z
   
    highp float diff; // far – near
   
   }
   
   uniform gl_DepthRangeParameters gl_DepthRange;
   ```

2. 顶点shader内置常量：

   gl_MaxVertexAttribs，最少16

   gl_MaxVertexUniformVectors，最少256个vec4，shader里面uniform，与常量的总数

   gl_MaxVertexOutputVectors，最少16

   gl_MaxVertexTextureImageUnits ，最少16

   gl_MaxCombinedTextureImageUnits，（vs+fs最大能用的纹理个数）最少32

   ```c++
   glGetIntegerv ( GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs );
   
   glGetIntegerv ( GL_MAX_VERTEX_UNIFORM_VECTORS,&maxVertexUniforms );
   
   glGetIntegerv ( GL_MAX_VARYING_VECTORS,&maxVaryings );
   
   glGetIntegerv ( GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,&maxVertexTextureUnits );
   
   glGetIntegerv ( GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,&maxCombinedTextureUnits );
   ```

   程序里面出现的1.0，2.0，3等立即数，最好用const int a这种形式声明，然后int(a)来引用，应为有gl_MaxVertexUniformVectors这个限制。

3. 输入、输出、精度指示符：in highp，out lowp，mediump

   precision highp float;

   precision mediump int; 这个用于指定默认的精度，如果没有precision，默认都是highp