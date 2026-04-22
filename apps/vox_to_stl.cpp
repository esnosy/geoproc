#include <array>
#include <bitset>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../libs/fast_float.hpp"
#include "../libs/indexed_tri_mesh.hpp"
#include "../libs/ply_io.hpp"
#include "../libs/stl_io.hpp"
#include "../libs/vec3.hpp"

using DICT = std::unordered_map<std::string, std::string>;

static std::string read_string(std::ifstream &ifs) {
  uint32_t buffer_size;
  ifs.read(reinterpret_cast<char *>(&buffer_size), sizeof(uint32_t));
  assert(buffer_size >= 0);
  if (buffer_size == 0) {
    return std::string();
  }
  std::string result(buffer_size, 0);
  ifs.read(result.data(), buffer_size);
  return result;
}

static DICT read_dict(std::ifstream &ifs) {
  uint32_t num_key_value_pairs;
  ifs.read(reinterpret_cast<char *>(&num_key_value_pairs), sizeof(uint32_t));
  assert(num_key_value_pairs >= 0);
  DICT result;
  if (num_key_value_pairs == 0) {
    return result;
  }
  result.reserve(num_key_value_pairs);
  for (size_t i = 0; i < num_key_value_pairs; i++) {
    auto key = read_string(ifs);
    auto value = read_string(ifs);
    result[key] = value;
  }
  return result;
}

static float float_from_str(const std::string &str, float default_value) {
  if (str.empty()) {
    return default_value;
  }
  float result;
  auto [ptr, ec] =
      fast_float::from_chars(str.data(), str.data() + str.size(), result);
  if (ec != std::errc()) {
    return default_value;
  }
  return result;
}

union Matt_Property_Bits {
  uint32_t bits;
  struct {
    uint32_t plastic : 1;
    uint32_t roughness : 1;
    uint32_t specular : 1;
    uint32_t ior : 1;
    uint32_t attenuation : 1;
    uint32_t power : 1;
    uint32_t glow : 1;
    uint32_t is_total_power : 1;
  };
};

union RGBA {
  uint32_t rgba;
  struct {

    uint8_t r, g, b, a;
  };
  RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}
  RGBA(uint32_t rgba) : rgba(rgba) {}
};

struct PBR_Material {
  std::array<float, 3> base_color = {1.0f, 1.0f, 1.0f};
  float metallic = 0.0f;
  float roughness = 0.0f;
  float ior = 1.0f;
  float transmission = 0.0f;
  float emission = 0.0f;
};

struct MATL_chunk {
  uint32_t id;
  DICT props;
};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Expected arguments: /path/to/input.vox" << std::endl;
    return 1;
  }
  const char *filename_vox = argv[1];
  std::ifstream ifs_vox(filename_vox, std::ios::binary);
  if (!ifs_vox) {
    std::cerr << "Failed to open file: " << filename_vox << std::endl;
    return 1;
  }
  char magic[4];
  ifs_vox.read(magic, 4);
  uint32_t version;
  ifs_vox.read(reinterpret_cast<char *>(&version), sizeof(uint32_t));

  size_t model_id = 0;

  std::vector<MATL_chunk> matl_chunks;
  matl_chunks.reserve(256);
  PBR_Material pbr_materials[256];
  RGBA palette[256] = {
      0x00000000, 0xffffffff, 0xffccffff, 0xff99ffff, 0xff66ffff, 0xff33ffff,
      0xff00ffff, 0xffffccff, 0xffccccff, 0xff99ccff, 0xff66ccff, 0xff33ccff,
      0xff00ccff, 0xffff99ff, 0xffcc99ff, 0xff9999ff, 0xff6699ff, 0xff3399ff,
      0xff0099ff, 0xffff66ff, 0xffcc66ff, 0xff9966ff, 0xff6666ff, 0xff3366ff,
      0xff0066ff, 0xffff33ff, 0xffcc33ff, 0xff9933ff, 0xff6633ff, 0xff3333ff,
      0xff0033ff, 0xffff00ff, 0xffcc00ff, 0xff9900ff, 0xff6600ff, 0xff3300ff,
      0xff0000ff, 0xffffffcc, 0xffccffcc, 0xff99ffcc, 0xff66ffcc, 0xff33ffcc,
      0xff00ffcc, 0xffffcccc, 0xffcccccc, 0xff99cccc, 0xff66cccc, 0xff33cccc,
      0xff00cccc, 0xffff99cc, 0xffcc99cc, 0xff9999cc, 0xff6699cc, 0xff3399cc,
      0xff0099cc, 0xffff66cc, 0xffcc66cc, 0xff9966cc, 0xff6666cc, 0xff3366cc,
      0xff0066cc, 0xffff33cc, 0xffcc33cc, 0xff9933cc, 0xff6633cc, 0xff3333cc,
      0xff0033cc, 0xffff00cc, 0xffcc00cc, 0xff9900cc, 0xff6600cc, 0xff3300cc,
      0xff0000cc, 0xffffff99, 0xffccff99, 0xff99ff99, 0xff66ff99, 0xff33ff99,
      0xff00ff99, 0xffffcc99, 0xffcccc99, 0xff99cc99, 0xff66cc99, 0xff33cc99,
      0xff00cc99, 0xffff9999, 0xffcc9999, 0xff999999, 0xff669999, 0xff339999,
      0xff009999, 0xffff6699, 0xffcc6699, 0xff996699, 0xff666699, 0xff336699,
      0xff006699, 0xffff3399, 0xffcc3399, 0xff993399, 0xff663399, 0xff333399,
      0xff003399, 0xffff0099, 0xffcc0099, 0xff990099, 0xff660099, 0xff330099,
      0xff000099, 0xffffff66, 0xffccff66, 0xff99ff66, 0xff66ff66, 0xff33ff66,
      0xff00ff66, 0xffffcc66, 0xffcccc66, 0xff99cc66, 0xff66cc66, 0xff33cc66,
      0xff00cc66, 0xffff9966, 0xffcc9966, 0xff999966, 0xff669966, 0xff339966,
      0xff009966, 0xffff6666, 0xffcc6666, 0xff996666, 0xff666666, 0xff336666,
      0xff006666, 0xffff3366, 0xffcc3366, 0xff993366, 0xff663366, 0xff333366,
      0xff003366, 0xffff0066, 0xffcc0066, 0xff990066, 0xff660066, 0xff330066,
      0xff000066, 0xffffff33, 0xffccff33, 0xff99ff33, 0xff66ff33, 0xff33ff33,
      0xff00ff33, 0xffffcc33, 0xffcccc33, 0xff99cc33, 0xff66cc33, 0xff33cc33,
      0xff00cc33, 0xffff9933, 0xffcc9933, 0xff999933, 0xff669933, 0xff339933,
      0xff009933, 0xffff6633, 0xffcc6633, 0xff996633, 0xff666633, 0xff336633,
      0xff006633, 0xffff3333, 0xffcc3333, 0xff993333, 0xff663333, 0xff333333,
      0xff003333, 0xffff0033, 0xffcc0033, 0xff990033, 0xff660033, 0xff330033,
      0xff000033, 0xffffff00, 0xffccff00, 0xff99ff00, 0xff66ff00, 0xff33ff00,
      0xff00ff00, 0xffffcc00, 0xffcccc00, 0xff99cc00, 0xff66cc00, 0xff33cc00,
      0xff00cc00, 0xffff9900, 0xffcc9900, 0xff999900, 0xff669900, 0xff339900,
      0xff009900, 0xffff6600, 0xffcc6600, 0xff996600, 0xff666600, 0xff336600,
      0xff006600, 0xffff3300, 0xffcc3300, 0xff993300, 0xff663300, 0xff333300,
      0xff003300, 0xffff0000, 0xffcc0000, 0xff990000, 0xff660000, 0xff330000,
      0xff0000ee, 0xff0000dd, 0xff0000bb, 0xff0000aa, 0xff000088, 0xff000077,
      0xff000055, 0xff000044, 0xff000022, 0xff000011, 0xff00ee00, 0xff00dd00,
      0xff00bb00, 0xff00aa00, 0xff008800, 0xff007700, 0xff005500, 0xff004400,
      0xff002200, 0xff001100, 0xffee0000, 0xffdd0000, 0xffbb0000, 0xffaa0000,
      0xff880000, 0xff770000, 0xff550000, 0xff440000, 0xff220000, 0xff110000,
      0xffeeeeee, 0xffdddddd, 0xffbbbbbb, 0xffaaaaaa, 0xff888888, 0xff777777,
      0xff555555, 0xff444444, 0xff222222, 0xff111111};

  char chunk_id[4];
  uint32_t chunk_size;
  uint32_t child_chunk_size;
  while (ifs_vox.read(chunk_id, 4)) {
    ifs_vox.read(reinterpret_cast<char *>(&chunk_size), sizeof(uint32_t));
    ifs_vox.read(reinterpret_cast<char *>(&child_chunk_size), sizeof(uint32_t));
    if (std::string_view(chunk_id, 4) == "XYZI") {
      uint32_t num_voxels;
      ifs_vox.read(reinterpret_cast<char *>(&num_voxels), sizeof(uint32_t));
      uint8_t *voxels = (uint8_t *)malloc(4 * num_voxels);
      ifs_vox.read(reinterpret_cast<char *>(voxels), 4 * num_voxels);

      std::vector<Triangle<double>> tris;
      std::vector<uint8_t> color_indices;

      std::unordered_map<Vec3<double>, uint8_t> dense_voxels;
      dense_voxels.reserve(num_voxels);

      for (size_t i = 0; i < 4 * num_voxels; i += 4) {
        auto x = (double)voxels[i];
        auto y = (double)voxels[i + 1];
        auto z = (double)voxels[i + 2];
        auto color_index = voxels[i + 3];
        assert(color_index > 0);
        dense_voxels.insert({Vec3<double>{x, y, z}, color_index});
      }
      free(voxels);
      std::cout << dense_voxels.size() << " " << num_voxels << std::endl;
      assert(dense_voxels.size() == num_voxels);
      for (const auto &[key, value] : dense_voxels) {
        auto x = key.x;
        auto y = key.y;
        auto z = key.z;
        auto color_index = value;
        // -x
        auto it = dense_voxels.find({x - 1, y, z});
        if (it == dense_voxels.end()) {
          tris.push_back({{x, y, z}, {x, y, z + 1}, {x, y + 1, z}});
          tris.push_back({{x, y + 1, z}, {x, y, z + 1}, {x, y + 1, z + 1}});
          color_indices.push_back(color_index);
          color_indices.push_back(color_index);
        }
        // x
        it = dense_voxels.find({x + 1, y, z});
        if (it == dense_voxels.end()) {
          tris.push_back({{x + 1, y, z}, {x + 1, y + 1, z}, {x + 1, y, z + 1}});
          tris.push_back(
              {{x + 1, y + 1, z}, {x + 1, y + 1, z + 1}, {x + 1, y, z + 1}});
          color_indices.push_back(color_index);
          color_indices.push_back(color_index);
        }
        // -y
        it = dense_voxels.find({x, y - 1, z});
        if (it == dense_voxels.end()) {
          tris.push_back({{x, y, z}, {x + 1, y, z}, {x, y, z + 1}});
          tris.push_back({{x, y, z + 1}, {x + 1, y, z}, {x + 1, y, z + 1}});
          color_indices.push_back(color_index);
          color_indices.push_back(color_index);
        }
        // y
        it = dense_voxels.find({x, y + 1, z});
        if (it == dense_voxels.end()) {
          tris.push_back({{x, y + 1, z}, {x, y + 1, z + 1}, {x + 1, y + 1, z}});
          tris.push_back(
              {{x, y + 1, z + 1}, {x + 1, y + 1, z + 1}, {x + 1, y + 1, z}});
          color_indices.push_back(color_index);
          color_indices.push_back(color_index);
        }
        // -z
        it = dense_voxels.find({x, y, z - 1});
        if (it == dense_voxels.end()) {
          tris.push_back({{x, y, z}, {x, y + 1, z}, {x + 1, y, z}});
          tris.push_back({{x + 1, y, z}, {x, y + 1, z}, {x + 1, y + 1, z}});
          color_indices.push_back(color_index);
          color_indices.push_back(color_index);
        }
        // +z
        it = dense_voxels.find({x, y, z + 1});
        if (it == dense_voxels.end()) {
          tris.push_back({{x, y, z + 1}, {x + 1, y, z + 1}, {x, y + 1, z + 1}});
          tris.push_back(
              {{x + 1, y, z + 1}, {x + 1, y + 1, z + 1}, {x, y + 1, z + 1}});
          color_indices.push_back(color_index);
          color_indices.push_back(color_index);
        }
      }

      auto mesh = Indexed_Tri_Mesh<double>::from_stl_tris(tris);

      std::string filename_vertices =
          "vox" + std::to_string(model_id) + "_vertices.bin";
      std::ofstream ofs_vertices(filename_vertices, std::ios::binary);
      for (const auto &v : mesh.vertices) {
        ofs_vertices.write(reinterpret_cast<const char *>(&v.as<float>()),
                           sizeof(Vec3<float>));
      }

      std::string filename_tris =
          "vox" + std::to_string(model_id) + "_tris.bin";
      std::ofstream ofs_tris(filename_tris, std::ios::binary);
      ofs_tris.write(reinterpret_cast<const char *>(mesh.tris.data()),
                     mesh.tris.size() * sizeof(uint32_t[3]));

      assert(mesh.tris.size() == color_indices.size());
      model_id++;
    } else if (std::string_view(chunk_id, 4) == "MATL") {
      std::cout << "Found MATL chunk!" << std::endl;
      uint32_t material_id;
      auto f1 = ifs_vox.tellg();
      ifs_vox.read(reinterpret_cast<char *>(&material_id), sizeof(uint32_t));
      assert(material_id > 0);
      std::cout << "Material ID: " << material_id << std::endl;
      auto material_properties = read_dict(ifs_vox);
      for (const auto &[key, value] : material_properties) {
        std::cout << "Material property: " << key << " = " << value
                  << std::endl;
      }
      matl_chunks.push_back({material_id, material_properties});
      auto f2 = ifs_vox.tellg();
      assert(f2 - f1 == chunk_size);
    } else if (std::string_view(chunk_id, 4) == "MATT") {

      std::cout << "Found MATT chunk!" << std::endl;
      auto f1 = ifs_vox.tellg();
      uint32_t material_id;
      ifs_vox.read(reinterpret_cast<char *>(&material_id), sizeof(uint32_t));
      assert(material_id > 0);
      std::cout << "Material ID: " << material_id << std::endl;
      uint32_t material_type;
      ifs_vox.read(reinterpret_cast<char *>(&material_type), sizeof(uint32_t));
      float material_weight;
      ifs_vox.read(reinterpret_cast<char *>(&material_weight), sizeof(float));

      Matt_Property_Bits property_bits;
      ifs_vox.read(reinterpret_cast<char *>(&property_bits),
                   sizeof(Matt_Property_Bits));

      auto n = std::bitset<8>(property_bits.bits).count();
      float *properties = (float *)malloc(sizeof(float) * n);
      ifs_vox.read(reinterpret_cast<char *>(properties), sizeof(float) * n);

      // TODO: store material properties and use them to set PBR values for each material.

      auto f2 = ifs_vox.tellg();
      assert(f2 - f1 == chunk_size);
    } else if (std::string_view(chunk_id, 4) == "RGBA") {
      std::cout << "Found RGBA chunk!" << std::endl;
      auto f1 = ifs_vox.tellg();
      ifs_vox.read(reinterpret_cast<char *>(palette + 1), sizeof(RGBA) * 255);
      ifs_vox.seekg(sizeof(RGBA), std::ios::cur);
      auto f2 = ifs_vox.tellg();
      assert(f2 - f1 == chunk_size);
    } else {
      ifs_vox.seekg(chunk_size, std::ios::cur);
    }
  }

  // Set base color from palette;
  for (size_t i = 0; i < 256; i++) {
    auto r = palette[i].r / 255.0f;
    auto g = palette[i].g / 255.0f;
    auto b = palette[i].b / 255.0f;
    r = std::pow(r, 2.2f);
    g = std::pow(g, 2.2f);
    b = std::pow(b, 2.2f);
    pbr_materials[i].base_color = {r, g, b};
  }

  // Set material properties from MATL chunks
  assert(matl_chunks.size() == 256);
  for (auto &matl_chunk : matl_chunks) {
    if (matl_chunk.id == 256) {
      continue;
    }
    auto &pbr_mat = pbr_materials[matl_chunk.id];
    pbr_mat.metallic = float_from_str(matl_chunk.props["_metal"], 0.0f);
    pbr_mat.roughness = float_from_str(matl_chunk.props["_rough"], 0.0f);
    pbr_mat.ior = float_from_str(matl_chunk.props["_ri"], 1.3f);
    pbr_mat.transmission = float_from_str(matl_chunk.props["_trans"], 0.0f);
    pbr_mat.emission = float_from_str(matl_chunk.props["_emit"], 0.0f);
  }

  std::ofstream ofs_pbr("pbr_materials.bin", std::ios::binary);
  ofs_pbr.write(reinterpret_cast<const char *>(pbr_materials),
                sizeof(PBR_Material) * 256);
}