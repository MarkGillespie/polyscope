// Copyright 2017-2019, Nicholas Sharp and the Polyscope contributors. http://polyscope.run.
#include "polyscope/surface_texture_quantity.h"

#include "polyscope/file_helpers.h"
#include "polyscope/polyscope.h"
#include "polyscope/render/materials.h"
#include "polyscope/render/shaders.h"

#include "imgui.h"
#include "stb_image.h"

using std::cout;
using std::endl;

namespace polyscope {

// ==============================================================
// ================  Base Texture  =====================
// ==============================================================

SurfaceTextureQuantity::SurfaceTextureQuantity(std::string name, SurfaceMesh& mesh_, std::vector<glm::vec2> coords_,
                                               DataType type_)
    : SurfaceMeshQuantity(name, mesh_, true), dataType(type_), coords(std::move(coords_)) {
  std::string filename = "../../meshes/spot/spot_texture.png";
  img = stbi_load(filename.c_str(), &imgWidth, &imgHeight, &imgComp, STBI_rgb);
  if (img == nullptr) throw std::logic_error("Failed to load " + filename);
}

void SurfaceTextureQuantity::draw() {
  if (!isEnabled()) return;

  if (program == nullptr) {
    createProgram();
  }

  // Set uniforms
  parent.setTransformUniforms(*program);
  setProgramUniforms(*program);

  program->draw();
}

void SurfaceTextureQuantity::createProgram() {
  // Create the program to draw this quantity

  program = render::engine->generateShaderProgram(
      {render::TEXTURE_SURFACE_VERT_SHADER, render::TEXTURE_SURFACE_FRAG_SHADER}, DrawMode::Triangles);

  // Fill color buffers
  fillColorBuffers(*program);
  parent.fillGeometryBuffers(*program);
  program->setTexture2D("t_image", img, imgWidth, imgHeight, false, false, true);

  render::engine->setMaterial(*program, parent.getMaterial());
}


// Update range uniforms
void SurfaceTextureQuantity::setProgramUniforms(render::ShaderProgram& program) {}

void SurfaceTextureQuantity::buildCustomUI() {
  ImGui::PushItemWidth(100);
  // ImGui::InputText("Filename", filename, IM_ARRAYSIZE(filename));
  // if (ImGui::Button("load")) {
  //   setTexture(filename);
  // }
}


SurfaceTextureQuantity* SurfaceTextureQuantity::setTexture(std::string filename) {
  img = stbi_load(filename.c_str(), &imgWidth, &imgHeight, &imgComp, STBI_rgb);
  if (img == nullptr) throw std::logic_error("Failed to load " + filename);

  program.reset();
  requestRedraw();
  return this;
}

void SurfaceTextureQuantity::geometryChanged() { program.reset(); }

std::string SurfaceTextureQuantity::niceName() { return name + " (corner texture)"; }

void SurfaceTextureQuantity::fillColorBuffers(render::ShaderProgram& p) {
  std::vector<glm::vec2> coordVal;
  coordVal.reserve(3 * parent.nFacesTriangulation());

  size_t cornerCount = 0;
  for (size_t iF = 0; iF < parent.nFaces(); iF++) {
    auto& face = parent.faces[iF];
    size_t D = face.size();

    // implicitly triangulate from root
    size_t cRoot = cornerCount;
    for (size_t j = 1; (j + 1) < D; j++) {
      size_t cB = cornerCount + j;
      size_t cC = cornerCount + ((j + 1) % D);

      coordVal.push_back(coords[cRoot]);
      coordVal.push_back(coords[cB]);
      coordVal.push_back(coords[cC]);
    }

    cornerCount += D;
  }

  // Store data in buffers
  p.setAttribute("a_coord", coordVal);
}

void SurfaceTextureQuantity::buildHalfedgeInfoGUI(size_t heInd) {
  ImGui::TextUnformatted(name.c_str());
  ImGui::NextColumn();
  ImGui::Text("<%g,%g>", coords[heInd].x, coords[heInd].y);
  ImGui::NextColumn();
}

} // namespace polyscope
