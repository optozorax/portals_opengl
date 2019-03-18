#include <prtl_vis/scene_reader.h>

namespace scene
{

//-----------------------------------------------------------------------------
Scene parseScene(const json& obj) {
	Scene result;
	result.cam_rotate_around = parseVec3(obj["cam_rotate_around"]);
	result.cam_spheric_pos = parseVec3(obj["cam_spheric_pos"]);
	if (obj.find("frames") != obj.end())
		for (const auto& i : obj["frames"])
			result.frames.push_back(parseFrame(i));
	return result;
}

//-----------------------------------------------------------------------------
Frame parseFrame(const json& obj) {
	Frame result;
	if (obj.find("colored_polygons") != obj.end())
		for (const auto& i : obj["colored_polygons"])
			result.colored_polygons.push_back(parseColoredPolygon(i));
	if (obj.find("textured_polygons") != obj.end())
		for (const auto& i : obj["textured_polygons"])
			result.textured_polygons.push_back(parseTexturedPolygon(i));
	if (obj.find("portals") != obj.end())
		for (const auto& i : obj["portals"])
			result.portals.push_back(parsePortal(i));
	if (obj.find("textures") != obj.end())
		for (const auto& i : obj["textures"])
			result.textures.push_back(parseTexture(i));
	if (obj.find("luminaries") != obj.end())
		for (const auto& i : obj["luminaries"])
			result.luminaries.push_back(parseLuminary(i));
	return result;
}

//-----------------------------------------------------------------------------
Luminary parseLuminary(const json& obj) {
	Luminary result;
	result.pos = parseVec3(obj["pos"]);
	result.color = parseVec3(obj["color"]);
	return result;
}

//-----------------------------------------------------------------------------
Texture parseTexture(const json& obj) {
	Texture result;
	result.filename = obj["filename"].get<std::string>();
	result.id = obj["id"];
	return result;
}

//-----------------------------------------------------------------------------
TexturedPolygon parseTexturedPolygon(const json& obj) {
	TexturedPolygon result;
	result.crd = parseCrd3(obj["crd"]);
	result.texture_id = obj["texture_id"];
	if (obj.find("polygon") != obj.end())
		for (const auto& i : obj["polygon"])
			result.polygon.push_back(parseVec2(i));
	if (obj.find("tex_coords") != obj.end())
		for (const auto& i : obj["tex_coords"])
			result.tex_coords.push_back(parseVec2(i));
	return result;
}

//-----------------------------------------------------------------------------
ColoredPolygon parseColoredPolygon(const json& obj) {
	ColoredPolygon result;
	result.crd = parseCrd3(obj["crd"]);
	result.color = parseVec3(obj["color"]);
	if (obj.find("polygon") != obj.end())
		for (const auto& i : obj["polygon"])
			result.polygon.push_back(parseVec2(i));
	return result;
}

//-----------------------------------------------------------------------------
Portal parsePortal(const json& obj) {
	Portal result;
	result.crd1 = parseCrd3(obj["crd1"]);
	result.crd2 = parseCrd3(obj["crd2"]);
	result.color1 = parseVec3(obj["color1"]);
	result.color2 = parseVec3(obj["color2"]);
	if (obj.find("polygon") != obj.end())
		for (const auto& i : obj["polygon"])
			result.polygon.push_back(parseVec2(i));
	return result;
}

//-----------------------------------------------------------------------------
spob::crd3 parseCrd3(const json& obj) {
	spob::crd3 result;
	result.i = parseVec3(obj["i"]);
	result.j = parseVec3(obj["j"]);
	result.k = parseVec3(obj["k"]);
	result.pos = parseVec3(obj["pos"]);
	return result;
}

//-----------------------------------------------------------------------------
spob::vec3 parseVec3(const json& obj) {
	spob::vec3 result;
	result.x = obj[0];
	result.y = obj[1];
	result.z = obj[2];
	return result;
}

//-----------------------------------------------------------------------------
spob::vec2 parseVec2(const json& obj) {
	spob::vec2 result;
	result.x = obj[0];
	result.y = obj[1];
	return result;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
json unparse(const Scene& scene) {
	json result;
	result["cam_rotate_around"] = unparse(scene.cam_rotate_around);
	result["cam_spheric_pos"] = unparse(scene.cam_spheric_pos);
	for (auto& i : scene.frames)
		result["frames"].push_back(unparse(i));
	return result;
}

//-----------------------------------------------------------------------------
json unparse(const Frame& frame) {
	json result;
	result["textured_polygons"] = {};
	result["colored_polygons"] = {};
	result["portals"] = {};
	for (auto& i : frame.textured_polygons)
		result["textured_polygons"].push_back(unparse(i));
	for (auto& i : frame.colored_polygons)
		result["colored_polygons"].push_back(unparse(i));
	for (auto& i : frame.portals)
		result["portals"].push_back(unparse(i));
	for (auto& i : frame.textures)
		result["textures"].push_back(unparse(i));
	for (auto& i : frame.luminaries)
		result["luminaries"].push_back(unparse(i));
	return result;
}

//-----------------------------------------------------------------------------
json unparse(const Luminary& luminary) {
	json result;
	result["pos"] = unparse(luminary.pos);
	result["color"] = unparse(luminary.color);
	return result;
}

//-----------------------------------------------------------------------------
json unparse(const Texture& texture) {
	json result;
	result["filename"] = texture.filename;
	result["id"] = texture.id;
	return result;
}

//-----------------------------------------------------------------------------
json unparse(const TexturedPolygon& textured_polygon) {
	json result;
	result["crd"] = unparse(textured_polygon.crd);
	result["texture_id"] = textured_polygon.texture_id;
	for (auto& i : textured_polygon.polygon)
		result["polygon"].push_back(unparse(i));
	for (auto& i : textured_polygon.tex_coords)
		result["tex_coords"].push_back(unparse(i));
	return result;
}

//-----------------------------------------------------------------------------
json unparse(const ColoredPolygon& colored_polygon) {
	json result;
	result["crd"] = unparse(colored_polygon.crd);
	result["color"] = unparse(colored_polygon.color);
	for (auto& i : colored_polygon.polygon)
		result["polygon"].push_back(unparse(i));
	return result;
}

//-----------------------------------------------------------------------------
json unparse(const Portal& portal) {
	json result;
	result["crd1"] = unparse(portal.crd1);
	result["crd2"] = unparse(portal.crd2);
	result["color1"] = unparse(portal.color1);
	result["color2"] = unparse(portal.color2);
	for (auto& i : portal.polygon)
		result["polygon"].push_back(unparse(i));
	return result;
}

//-----------------------------------------------------------------------------
json unparse(const spob::crd3& crd) {
	json result;
	result["i"] = unparse(crd.i);
	result["j"] = unparse(crd.j);
	result["k"] = unparse(crd.k);
	result["pos"] = unparse(crd.pos);
	return result;
}

//-----------------------------------------------------------------------------
json unparse(const spob::vec3& vec) {
	json result;
	result.push_back(vec.x);
	result.push_back(vec.y);
	result.push_back(vec.z);
	return result;
}

//-----------------------------------------------------------------------------
json unparse(const spob::vec2& vec) {
	json result;
	result.push_back(vec.x);
	result.push_back(vec.y);
	return result;
}

};