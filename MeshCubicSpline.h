#pragma once

#include "Mesh.h"
#include "Rect.h"
/*CubicSpline,使用y=a+bx+cx2+dx3,三次函数插值给定的点，具有c2连续的性质
*给定n+1个顶点，总共有n段曲线，每一段由一个三次函数来确定，n段曲线总共有4n个未知数，需要有4n个方程才能解出来
* 曲线过n+1个顶点，得到n+1个方程，
* n个曲线之间首尾相连，得到n-1个方程
* 曲线具有c2连续，得到2n-2个方程，
* 再设置固定的边界条件：比如起点的一阶导数=0，二阶导数=0等，得到额外两个方程，总共得到4n个方程，然后就是用数值算法，矩阵分解，解方程
*/
class MeshCubicSpline : public Mesh {
public:
	MeshCubicSpline();
	~MeshCubicSpline();
	
	void loadMesh(const std::vector<glm::vec3>& p,int num) override;
	void loadMesh(const std::vector<glm::vec2>& p,int num) override;
protected:
	void draw(int posloc = -1) override;
};
