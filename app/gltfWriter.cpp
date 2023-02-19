#define CGLTF_IMPLEMENTATION
#define CGLTF_WRITE_IMPLEMENTATION
#include "cgltf_write.h"
#include <vector>
#include <array>
#include <iostream>
#include <memory>

std::vector<cgltf_mesh> gMeshes;
std::vector<cgltf_primitive> gSubMeshes; //subMesh
std::vector<cgltf_accessor> gAccessors; //mesh attribute
std::vector<cgltf_buffer_view> gBufferViews;
std::vector<cgltf_material> gMaterials;//only define one material
std::vector<cgltf_attribute> gAttributes;

cgltf_data data;
cgltf_options options;
cgltf_buffer gBuffer;
std::vector<std::pair<void*,std::size_t>> gBufferInfo;

//type : cgltf_attribute_type_position,
//componentType: cgltf_type_vec3 
cgltf_attribute* addAttribute(std::vector<float>& data,cgltf_attribute_type type,cgltf_type componentType){
    std::size_t mySize = sizeof(float)*data.size();
    //define buffer view for vertex data
    cgltf_buffer_view posBv;
    memset((void*)&posBv,0,sizeof(cgltf_buffer_view));
    posBv.name = nullptr;
    posBv.type = cgltf_buffer_view_type_vertices;
    posBv.buffer = &gBuffer;
    posBv.offset = gBuffer.size;
    posBv.size = mySize;
    gBufferViews.emplace_back(posBv);
    gBuffer.size += mySize;
    gBufferInfo.emplace_back((void*)data.data(),mySize);
    //define pos attribute accessor
    cgltf_accessor posAcc;
    memset((void*)&posAcc,0,sizeof(cgltf_accessor));
    posAcc.name = nullptr;
    posAcc.component_type = cgltf_component_type_r_32f;
    posAcc.type = componentType;
    posAcc.buffer_view = &gBufferViews.back();
    int comCounts = componentType;
    if(comCounts>4){
        if(componentType==cgltf_type_mat2){
            comCounts = 4;
        }else if(componentType==cgltf_type_mat3){
            comCounts = 9;
        }else if(componentType==cgltf_type_mat4){
            comCounts = 16;
        }else{
            std::cout<<"error,unknown mesh attribute when write gltf\n";
            return nullptr;
        }
    }
    posAcc.count = data.size()/comCounts;
    gAccessors.emplace_back(posAcc);
    //define pos attribute
    cgltf_attribute posAttribute;
    memset((void*)&posAttribute,0,sizeof(cgltf_attribute));
    posAttribute.name = (char*)"POSITION";
    posAttribute.type = type;
    posAttribute.data = &gAccessors.back();

    gAttributes.emplace_back(posAttribute);

    return &gAttributes.back();
}

cgltf_accessor* addIndices(std::vector<unsigned int>& indices){
    //define buffer view for indices data
    cgltf_buffer_view indexBv{0};
    memset((void*)&indexBv,0,sizeof(cgltf_buffer_view));
    indexBv.name = nullptr;
    indexBv.type = cgltf_buffer_view_type_indices;
    indexBv.buffer = &gBuffer;
    indexBv.offset = gBuffer.size;
    indexBv.size = sizeof(unsigned int)*indices.size();
    gBuffer.size += indexBv.size;
    gBufferViews.emplace_back(indexBv);
    gBufferInfo.emplace_back(indices.data(),indexBv.size);
    //define accessor for indices
    cgltf_accessor indice;
    memset((void*)&indice,0,sizeof(cgltf_accessor));
    indice.name = nullptr;
    indice.component_type = cgltf_component_type_r_32u;
    indice.type = cgltf_type_scalar;
    indice.buffer_view = &gBufferViews.back();
    indice.count = indices.size();
    gAccessors.emplace_back(indice);
    return &gAccessors.back();
}


//atype: cgltf_type_vec3
//type: cgltf_primitive_type_triangles
void addMesh(std::vector<float>& pos,cgltf_type atype,std::vector<unsigned int>& indices,float* pColor,cgltf_primitive_type meshType){
    
    auto* pPosAttr = addAttribute(pos,cgltf_attribute_type_position,atype);
    auto* pIndex = addIndices(indices);

    cgltf_material* pMaterial = &gMaterials[0];
    if(pColor!=nullptr){
        gMaterials.emplace_back();
        cgltf_material& rmaterial = gMaterials.back();
        memset((void*)&rmaterial,0,sizeof(cgltf_material));
        rmaterial.name = nullptr;
        rmaterial.alpha_cutoff = 0.5f;
        rmaterial.has_pbr_metallic_roughness = true;
        rmaterial.pbr_metallic_roughness = {
            .base_color_factor = {pColor[0],pColor[1],pColor[2],pColor[3]},
            .metallic_factor = 0.5,
            .roughness_factor = 0.5
        };
        pMaterial = &gMaterials.back();
    }

    //define a submesh
    cgltf_primitive subMesh;
    memset(&subMesh,0,sizeof(subMesh));
    subMesh.indices = pIndex;
    subMesh.attributes = pPosAttr;
    subMesh.attributes_count = 1;
    subMesh.type = meshType;//triangle line point
    subMesh.material = pMaterial;
    gSubMeshes.emplace_back(subMesh);

    cgltf_mesh gltfMesh;
    memset(&gltfMesh,0,sizeof(gltfMesh));
    gltfMesh.primitives = &gSubMeshes.back();
    gltfMesh.primitives_count = 1;
    gMeshes.emplace_back(gltfMesh);
}

int main(int argc,char** argv){
    //first reserve enough object
    //todo:do not do reserve
    gMeshes.reserve(100);
    gSubMeshes.reserve(100);
    gAccessors.reserve(100);
    gBufferViews.reserve(100);
    gMaterials.reserve(100);

    std::string outFile{"test.glb"};
    memset(&data,0,sizeof(data));
    memset(&options,0,sizeof(options));
    options.type = cgltf_file_type_glb;
    options.json_token_count = 0;
    //before add some submesh,you should figure out how many submesh
    //and than reserve global vectors to that number.

    //create a default material for submesh
    gMaterials.emplace_back();
    cgltf_material& rmaterial = gMaterials.back();
    memset(&rmaterial,0,sizeof(cgltf_material));
    rmaterial.name = (char*)"default";
    rmaterial.alpha_cutoff = 0.5f;
    rmaterial.has_pbr_metallic_roughness = true;
    rmaterial.pbr_metallic_roughness = {
        .base_color_factor = {0.0f,0.7f,0.7f,1.0f},
        .metallic_factor = 0.5,
        .roughness_factor = 0.5
    };

    std::vector<float> pos{
        0,0,0,
        2,0,0,
        1,2,0,
        1,1,2
    };
    std::vector<unsigned int> ind{
        0,2,1,
        0,1,3,
        1,2,3,
        0,3,2
    };

    addMesh(pos,cgltf_type_vec3,ind,nullptr,cgltf_primitive_type_triangles);

    std::vector<cgltf_node> gNodes(gMeshes.size()+1);
    std::vector<cgltf_node*> pgNodes(gMeshes.size()+1);
    pgNodes[0] = &gNodes[0];
    std::vector<std::unique_ptr<char[]>> meshNnames;
    for(int i=1; i<gNodes.size(); ++i){
        meshNnames.emplace_back(new char[64]);
        sprintf(meshNnames.back().get(),"mesh%d",i);
        memset(&gNodes[i],0,sizeof(cgltf_node));
        gNodes[i].name = meshNnames.back().get();
        gNodes[i].parent = pgNodes[0];
        //gNodes[i].children = {};
        gNodes[i].children_count = 0;
        //gNodes[i].skin = {};
        gNodes[i].mesh = &gMeshes.at(i-1);
        pgNodes[i] = &gNodes[i];
    }
    memset(&gNodes[0],0,sizeof(cgltf_node));
    gNodes[0] = {
            .name = (char*)"root",
            .parent = nullptr,
            .children = &pgNodes[1],
            .children_count = pgNodes.size()-1,
            .has_matrix = true,
            .matrix = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
            }
    };

    data.meshes = gMeshes.data();
    data.meshes_count = gMeshes.size();
    data.accessors = gAccessors.data();
    data.accessors_count = gAccessors.size();
    data.buffer_views = gBufferViews.data();
    data.buffer_views_count = gBufferViews.size();
    data.buffers = &gBuffer;
    data.buffers_count = 1;
    data.materials = gMaterials.data();
    data.materials_count = gMaterials.size();

    cgltf_scene gltfScene{};
    memset(&gltfScene,0,sizeof(gltfScene));
    gltfScene.nodes = &pgNodes[0];
    gltfScene.nodes_count = 1;
    std::unique_ptr<char> pBufferData(new char[gBuffer.size]);
    data.bin = (void*)pBufferData.get();
    data.bin_size = gBuffer.size;
    std::size_t offset = 0;
    for(auto& pi:gBufferInfo){
            memcpy(((char*)data.bin + offset),pi.first,pi.second);
            offset += pi.second;
    }
    data.asset.copyright = (char*)"DreamEngine";
    data.asset.version = (char*)"2.0";
    data.nodes = gNodes.data();
    data.nodes_count = gNodes.size();
    data.scene = &gltfScene;
    data.scenes = &gltfScene;
    data.scenes_count = 1;
    cgltf_result result = cgltf_write_file(&options, outFile.c_str(), &data);
    if (result != cgltf_result_success)
    {
            std::cout<<"error to save mesh to glb"<<std::endl;
            return -1;
    }

    return 0;
}