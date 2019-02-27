#pragma once

#include <string>
#include <vector>
#include <spob/spob.h>
#include <json.hpp>

using json = nlohmann::json;

namespace scene
{
	struct Scene;
	struct Frame;
	struct TexturedPolygon;
	struct ColoredPolygon;
	struct Portal;

	struct Portal
	{
		spob::space3 crd1, crd2;
		std::vector<spob::vec2> polygon;
		spob::vec3 color1, color2;
	};

	struct ColoredPolygon
	{
		spob::plane3 crd;
		std::vector<spob::vec2> polygon;
		spob::vec3 color;
	};

	struct TexturedPolygon
	{
		spob::plane3 crd;
		std::vector<spob::vec2> polygon;
		std::vector<spob::vec2> tex_coords;
		int texture;
	};

	/*struct sTexture
	{
	private:
		sTexture(std::string )
		~sTexture();

		int id;
	private:
		void* data;
	};*/

	struct Frame
	{
		//std::vector<Textures> ;
		std::vector<TexturedPolygon> textured_polygons;
		std::vector<ColoredPolygon> colored_polygons;
		std::vector<Portal> portals;
	};

	struct Scene
	{
		spob::vec3 cam_rotate_around, cam_spheric_pos;
		std::vector<Frame> frames;
	};

	Scene parseScene(const json& obj);
	Frame parseFrame(const json& obj);
	//sTexture parseTexture(const json& obj);
	TexturedPolygon parseTexturedPolygon(const json& obj);
	ColoredPolygon parseColoredPolygon(const json& obj);
	Portal parsePortal(const json& obj);
	spob::crd3 parseCrd3(const json& obj);
	spob::vec3 parseVec3(const json& obj);
	spob::vec2 parseVec2(const json& obj);

	json unparse(const Scene& scene);
	json unparse(const Frame& frame);
	//json unparse(const sTexture& texture);
	json unparse(const TexturedPolygon& textured_polygon);
	json unparse(const ColoredPolygon& colored_polygon);
	json unparse(const Portal& portal);
	json unparse(const spob::crd3& crd);
	json unparse(const spob::vec3& vec);
	json unparse(const spob::vec2& vec);
};