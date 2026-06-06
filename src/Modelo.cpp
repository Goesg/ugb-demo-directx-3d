// NOMINMAX deve vir antes de qualquer include que puxe windows.h
// Evita conflito entre as macros min/max do Windows e o TinyObjLoader
#define NOMINMAX
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "Modelo.h"
#include <unordered_map>
#include <string>

bool Modelo::carregar(ID3D11Device* device, ID3D11DeviceContext* contexto,
                      const std::string& caminhoObj,
                      const std::string& caminhoTextura) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t>    shapes;
    std::vector<tinyobj::material_t> materials;
    std::string aviso, erro;

    bool ok = tinyobj::LoadObj(&attrib, &shapes, &materials,
                                &aviso, &erro, caminhoObj.c_str());
    if (!ok) return false;

    std::vector<Vertice> vertices;
    std::vector<UINT>    indices;

    // OBJ usa índices separados para posição, normal e UV.
    // Precisamos criar um vértice único para cada combinação usada.
    std::unordered_map<std::string, UINT> mapeamento;

    for (const auto& shape : shapes) {
        for (const auto& idx : shape.mesh.indices) {
            // Criar chave única para esta combinação de índices
            std::string chave = std::to_string(idx.vertex_index)   + "/" +
                                std::to_string(idx.normal_index)   + "/" +
                                std::to_string(idx.texcoord_index);

            if (mapeamento.count(chave) == 0) {
                mapeamento[chave] = static_cast<UINT>(vertices.size());

                Vertice v = {};

                // Posição
                int vi = idx.vertex_index * 3;
                v.posicao = { attrib.vertices[vi],
                              attrib.vertices[vi + 1],
                              attrib.vertices[vi + 2] };

                // Normal (pode estar ausente no OBJ)
                if (idx.normal_index >= 0) {
                    int ni = idx.normal_index * 3;
                    v.normal = { attrib.normals[ni],
                                 attrib.normals[ni + 1],
                                 attrib.normals[ni + 2] };
                }

                // UV (pode estar ausente no OBJ)
                if (idx.texcoord_index >= 0) {
                    int ti = idx.texcoord_index * 2;
                    v.uv = { attrib.texcoords[ti],
                             1.0f - attrib.texcoords[ti + 1] }; // inverter V: OBJ origin=baixo, DX origin=cima
                }

                vertices.push_back(v);
            }

            indices.push_back(mapeamento[chave]);
        }
    }

    if (!malha.criar(device, vertices, indices)) return false;
    if (!caminhoTextura.empty())
        if (!textura.carregar(device, contexto, caminhoTextura)) return false;

    return true;
}
