#include <prtl_vis/scene_reader.h>

namespace scene
{

//-----------------------------------------------------------------------------
Scene parseScene(const json& obj) {
	Scene result;
	result.cam_rotate_around = parseVec3(obj["cam_rotate_around"]);
	result.cam_spheric_pos = parseVec3(obj["cam_spheric_pos"]);
	for (const auto& i : obj["frames"])
		result.frames.push_back(parseFrame(i));
	return result;
}

//-----------------------------------------------------------------------------
Frame parseFrame(const json& obj) {
	Frame result;
	for (const auto& i : obj["colored_polygons"])
		result.colored_polygons.push_back(parseColoredPolygon(i));
	for (const auto& i : obj["textured_polygons"])
		result.textured_polygons.push_back(parseTexturedPolygon(i));
	for (const auto& i : obj["portals"])
		result.portals.push_back(parsePortal(i));
	return result;
}

//-----------------------------------------------------------------------------
//sTexture parseTexture(const json& obj) {
//}

//-----------------------------------------------------------------------------
TexturedPolygon parseTexturedPolygon(const json& obj) {
	TexturedPolygon result;
	return result;
}

//-----------------------------------------------------------------------------
ColoredPolygon parseColoredPolygon(const json& obj) {
	ColoredPolygon result;
	result.crd = parseCrd3(obj["crd"]);
	result.color = parseVec3(obj["color"]);
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
	// TODO texture
	return result;
}

//-----------------------------------------------------------------------------
/*json unparse(const sTexture& texture) {

}*/

//-----------------------------------------------------------------------------
json unparse(const TexturedPolygon& textured_polygon) {
	json result;
	result["crd"] = unparse(textured_polygon.crd);
	//result["texture"] = texture;
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