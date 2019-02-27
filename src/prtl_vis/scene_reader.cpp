#include <prtl_vis/scene_reader.h>

namespace scene
{

//-----------------------------------------------------------------------------
/*sScene parseScene(const json& obj) {

}

//-----------------------------------------------------------------------------
sFrame parseFrame(const json& obj) {

}

//-----------------------------------------------------------------------------
//sTexture parseTexture(const json& obj) {
//}

//-----------------------------------------------------------------------------
sTexturedPolygon parseTexturedPolygon(const json& obj) {

}

//-----------------------------------------------------------------------------
sColoredPolygon parseColoredPolygon(const json& obj) {

}

//-----------------------------------------------------------------------------
sPortal parsePortal(const json& obj) {

}

//-----------------------------------------------------------------------------
spob::crd3 parseCrd3(const json& obj) {

}

//-----------------------------------------------------------------------------
spob::vec3 parseVec3(const json& obj) {

}*/

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
	for (auto& i : frame.textured_polygons)
		result["textured_polygons"].push_back(unparse(i));
	for (auto& i : frame.colored_polygons)
		result["colored_polygons"].push_back(unparse(i));
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
	result["pos"] = unparse(crd.pos);
	result["i"] = unparse(crd.i);
	result["j"] = unparse(crd.j);
	result["k"] = unparse(crd.k);
	return result;
}

//-----------------------------------------------------------------------------
json unparse(const spob::vec3& vec) {
	json result;
	result["x"] = vec.x;
	result["y"] = vec.y;
	result["z"] = vec.z;
	return result;
}

//-----------------------------------------------------------------------------
json unparse(const spob::vec2& vec) {
	json result;
	result["x"] = vec.x;
	result["y"] = vec.y;
	return result;
}

};