#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <thread>
#include "core/Shader.h"
#include "core/MeshNurbs.h"
#include "core/MeshBezier.h"
#include "core/MeshCRSpline.h"
#include "core/MeshHermiter.h"
#include "core/Light.h"
#include "core/Scene.h"
#include "core/Node.h"
#include "core/Material.h"
#include "core/Camera.h"
#include "core/Pbo.h"
#include "ui/UiRender.h"
#include "ui/UiManager.h"
#include "ui/View.h"
#include "ui/UiTree.h"
#include "ui/ListView.h"
#include "ui/TreeView.h"
#include "core/Spline.h"
#include "core/MeshLineStrip.h"
#include "core/MeshCubicSpline.h"
#include "core/MeshFilledRect.h"
#include "ui/ListAdapter.h"
#include "ui/NodeListAdapter.h"
#include "core/NodeRoamer.h"
#include "core/Utils.h"
#include "core/helper.h"
//#include "core/MeshLoaderAssimp.h"
#include "core/MeshLoaderGltf.h"
#include "core/Resource.h"
#include "core/aabb.h"
#include "core/AnimationManager.h"
#include <filesystem>
#include <fstream>
#include <glm/gtc/type_ptr.hpp> // value_ptr,make_vec,make_mat
#include "ui/ListViewAdapter.h"
#include "ui/ListViewData.h"

// settings
unsigned int srcWidth = 1024;
unsigned int srcHeight = 1024;

static shared_ptr<NodeRoamer> gpRoamer;
static shared_ptr<Scene> scene;
static shared_ptr<Camera> camera;

//std::shared_ptr<Node> gpBoxNode;

static const string gfontfile("../resource/simfang.ttf");
static const string gmyfontfile("../resource/myfont.data");
static const string gLayout("../resource/layout/layout.xml");

using namespace std::filesystem;
void initResource() {
	Resource::getInstance().loadAllMaterial();
	
	scene = std::make_shared<Scene>();
	camera = scene->createACamera(srcWidth,srcHeight);
	camera->perspective(glm::radians(90.0f), static_cast<float>(srcWidth) / static_cast<float>(srcHeight), 0.1f, 6100.0f);

	
	auto pLight1 = scene->createALight(glm::vec3(1.0f, 10.0f, -5.0f),glm::vec3(200.0f, 100.0f, 0.0f));
	
	auto pLight2 = scene->createALight(glm::vec3(-0.1f, -0.1f, -0.1f), glm::vec3(200.0f, 100.0f, 0.0f),LightType::Directional);//
	pLight2->setCastShadow(true);
	auto pskyNode = scene->createSkybox();
	auto& pSkanNode = scene->getRootDeffered();
	if (Resource::getInstance().parseMeshCfgFile("../resource/mesh/dragon.meshCfg")) {
		AABB aabb;
		auto pNode = Resource::getInstance().getNode("big_gun");//coffee_table
		//gpBoxNode = Resource::getInstance().getNode("box");
		if (pNode) {
			pSkanNode->addChild(pNode);
			//pSkanNode->addChild(gpBoxNode);
			pNode->visitNode([&aabb](Node* pNode) {
				if (pNode) {
					for (const auto& pMesh : pNode->getRenderables()) {
						auto pM = std::dynamic_pointer_cast<Mesh>(pMesh.second);
						if (pM) {
							auto& pab = pM->getAabb();
							if (pab) {
								auto tempAB = *pab;
								tempAB = tempAB * pNode->getWorldMatrix();
								aabb += tempAB;
							}
						}
					}
				}
				});
		}
		auto abCenter = aabb.center();
		auto ablen = aabb.length();
		//auto abCenter = glm::vec3(0.0f,0.0f,0.0f);
		//auto ablen = glm::vec3(0.0f, 0.0f, 5.0f);
		auto eyePos = abCenter + ablen * 2.0f;
		camera->lookAt(glm::vec3(eyePos.x, eyePos.y, eyePos.z),
			glm::vec3(abCenter.x, abCenter.y, abCenter.z), glm::vec3(0.0f, 1.0f, 0.0f));
		

		gpRoamer = make_shared<NodeRoamer>();
		gpRoamer->setTarget(pSkanNode, camera);
	}
	AnimationManager::getInstance().startAnimation(0);
	

	
	
	//auto& ui = UiManager::getInstance();
	//ui.initUi(srcWidth, srcHeight);
	//auto pRootView = ui.loadFromFile(gLayout);
	//if (pRootView) {
	//	auto pTv = pRootView->findViewById("tv");
	//	if (pTv) {
	//		pTv->setOnClickListener([](View* pView) {
	//			if (pView != nullptr) {
	//				Pbo pbo;
	//				pbo.initPbo(srcWidth, srcHeight);
	//				pbo.saveToFile(GL_BACK, "test2.tga");//
	//			}
	//			});
	//	}
	//	auto pv = pRootView->findViewById("nodeTree");
	//	auto pTreeView = std::dynamic_pointer_cast<ListView>(pv);
	//	if (pTreeView) {
	//		auto pAdapter = std::make_shared<ListViewAdapter>(pv);
	//		auto pData = std::make_shared<ListViewData>();
	//		if (pAdapter) {
	//			pAdapter->setData(pData);
	//			pTreeView->setAdapter(pAdapter);
	//		}
	//	}
	//}

	//auto pTree = std::make_shared<UiTree>();
	//pTree->setRootView(pRootView);
	//UiManager::getInstance().setUiTree(pTree);
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mod);
void mouseMoveCallback(GLFWwindow* window, double x, double y);
void keyCallback(GLFWwindow* window, int key,int scancode, int action, int mod);
void processInput(GLFWwindow* window);

int main()
{
    // glfw: initialize and configure
    // ------------------------------
	auto mainThreadId = std::this_thread::get_id();
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(srcWidth, srcHeight, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window,mouseButtonCallback);
	glfwSetCursorPosCallback(window, mouseMoveCallback);
	glfwSetScrollCallback(window, mouseScrollCallback);
	glfwSetKeyCallback(window, keyCallback);
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    // Load OpenGL functions, gladLoadGL returns the loaded version, 0 on error.
    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0)
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }
	initResource();

	auto pMesh = std::make_shared<Mesh>(MeshType::Quad);
	pMesh->loadMesh();
	auto pMaterial = Resource::getInstance().getMaterial("drawQuad");
	if (pMaterial) {
		auto pTex = Resource::getInstance().getOrLoadTextureFromFile("buti.jpg");
		pMaterial->setTextureForSampler("albedoMap", pTex);
		pMesh->setMaterial(pMaterial);
	}
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // render
        // ------
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//default (0,0,0,0), 1.0f
		glEnable(GL_CULL_FACE);//glFrontFace(GL_CCW);glCullFace(GL_BACK);
		glEnable(GL_DEPTH_TEST);//glDepthFunc(GL_LESS);
		glEnable(GL_DITHER);
		glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		
		camera->renderScene();

		//pMesh->draw(nullptr,nullptr);
		//UiManager::getInstance().draw();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();

		AnimationManager::getInstance().updateActiveAnimation();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
	srcWidth = width;
	srcHeight = height;
    glViewport(0, 0, width, height);
	UiManager::getInstance().updateWidthHeight(static_cast<float>(width), 
		static_cast<float>(height));
}

void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	/*
	* mouse middle button scroll up,yoffset>0,scroll down yoffset<0
	*/
	if (gpRoamer) {
		gpRoamer->move(yoffset>0.0);
	}
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mod) {
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	switch (button) {
	case GLFW_MOUSE_BUTTON_LEFT:
		if (action == GLFW_PRESS) {
			UiManager::getInstance().mouseLButtonDown(static_cast<int>(x), static_cast<int>(y));
		}
		else if(action == GLFW_RELEASE) {
			UiManager::getInstance().mouseLButtonUp(static_cast<int>(x), static_cast<int>(y));
		}
		break;
	case GLFW_MOUSE_BUTTON_MIDDLE:
		if (action == GLFW_PRESS) {
			if (gpRoamer) {
				gpRoamer->startRotate(static_cast<int>(x), static_cast<int>(y));
			}
		}
		else if (action == GLFW_RELEASE) {
			if (gpRoamer) {
				gpRoamer->endRotate(static_cast<int>(x), static_cast<int>(y));
			}
		}
		break;
	case GLFW_MOUSE_BUTTON_RIGHT:
		if (action == GLFW_PRESS) {
		}
		else if (action == GLFW_RELEASE) {
		}
		break;
	default:
		break;
	}
}
void mouseMoveCallback(GLFWwindow* window, double x, double y) {
	if (gpRoamer) {
		gpRoamer->rotate(static_cast<int>(x), static_cast<int>(y));
	}
	UiManager::getInstance().mouseMove(static_cast<int>(x), static_cast<int>(y));
}
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mod) {
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, true);
			break;
		case GLFW_KEY_S:
			//gpRoamer->setTarget(gShNode);
			break;
		default:
			break;
		}
	}
}