#ifndef _LDP_MESH_H_
#define _LDP_MESH_H_

//定义自己的mesh结构体
/*
* 保存在文件里面的结构是：
* ldpmesh对象
* vertex
* texcoord
* normals
* tangents
* bitangents
* indexes
*/
struct LdpMesh {
	int vertexCount{ 0 };
	int vertexPos{ 0 };
	int vertexLength{ 0 };

	int texcoordPos{ 0 };
	int texcoordLength{ 0 };

	int normalPos{ 0 };
	int normalLength{ 0 };

	int tangentsPos{ 0 };
	int tangentsLength{ 0 };

	int indexPos{ 0 };
	int indexLength{ 0 };
};

#endif
