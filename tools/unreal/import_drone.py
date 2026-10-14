"""Editor-only import of the user-supplied drone; raw assets remain local."""
from pathlib import Path
import unreal

root = Path(__file__).resolve().parents[2]
source = root / "local-assets" / "drone"
mesh_source = source / "source" / "QuadDrone [mesh].fbx"
if not mesh_source.is_file():
    raise RuntimeError("User drone FBX missing from local-assets/drone/source")

tools = unreal.AssetToolsHelpers.get_asset_tools()
assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
destination = "/Game/Drone"
assets.make_directory(destination)

# Reuse Epic's AssetImportTask workflow with explicit automated/save flags.
def import_file(path, name, options=None):
    task = unreal.AssetImportTask()
    task.filename = str(path)
    task.destination_path = destination
    task.destination_name = name
    task.automated = True
    task.replace_existing = True
    task.save = True
    if options:
        task.options = options
        task.factory = unreal.FbxFactory()
    tools.import_asset_tasks([task])
    imported = task.get_objects()
    if not imported:
        raise RuntimeError(f"No asset imported for {path.name}")
    return imported[0]

options = unreal.FbxImportUI()
options.import_as_skeletal = False
options.import_mesh = True
options.import_materials = False
options.import_textures = False
options.automated_import_should_detect_type = False
options.mesh_type_to_import = unreal.FBXImportType.FBXIT_STATIC_MESH
options.static_mesh_import_data.combine_meshes = True
options.static_mesh_import_data.generate_lightmap_u_vs = True
mesh = import_file(mesh_source, "SM_DefaultDrone", options)
if not isinstance(mesh, unreal.StaticMesh):
    raise RuntimeError("Expected a static mesh")
if mesh.get_name() != "SM_DefaultDrone":
    assets.rename_asset(mesh.get_path_name(), destination + "/SM_DefaultDrone")
    mesh = unreal.load_asset(destination + "/SM_DefaultDrone")

maps = {
    "diffuse": "SpaceProbe_[diffuse].png",
    "normal": "SpaceProbe_[normals].png",
    "metal": "SpaceProbe_[metal].png",
    "gloss": "SpaceProbe_[gloss].png",
    "ao": "Spaceprobe_[occlusion].png",
    "emission": "SpaceProbe_[emission-yellow].png",
}
textures = {}
for key, filename in maps.items():
    texture = import_file(source / "textures" / filename, "T_Drone_" + key)
    texture.set_editor_property("srgb", key in ("diffuse", "emission"))
    if key == "normal":
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
    texture.set_editor_property("max_texture_size", 2048)
    assets.save_loaded_asset(texture)
    textures[key] = texture

material_path = destination + "/M_DefaultDrone"
material = unreal.load_asset(material_path)
if not material:
    material = tools.create_asset("M_DefaultDrone", destination, unreal.Material, unreal.MaterialFactoryNew())
lib = unreal.MaterialEditingLibrary
lib.delete_all_material_expressions(material)
properties = {
    "diffuse": unreal.MaterialProperty.MP_BASE_COLOR,
    "normal": unreal.MaterialProperty.MP_NORMAL,
    "metal": unreal.MaterialProperty.MP_METALLIC,
    "ao": unreal.MaterialProperty.MP_AMBIENT_OCCLUSION,
    "emission": unreal.MaterialProperty.MP_EMISSIVE_COLOR,
}
for index, (key, texture) in enumerate(textures.items()):
    node = lib.create_material_expression(material, unreal.MaterialExpressionTextureSample, -400, index * 180)
    node.texture = texture
    if key == "normal":
        node.sampler_type = unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL
    elif key not in ("diffuse", "emission"):
        node.sampler_type = unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR
    if key == "gloss":
        invert = lib.create_material_expression(material, unreal.MaterialExpressionOneMinus, -150, index * 180)
        lib.connect_material_expressions(node, "R", invert, "Input")
        lib.connect_material_property(invert, "", unreal.MaterialProperty.MP_ROUGHNESS)
    else:
        lib.connect_material_property(node, "RGB" if key in ("normal", "diffuse", "emission") else "R", properties[key])
lib.recompile_material(material)
assets.save_loaded_asset(material)
for slot in range(max(1, len(mesh.get_editor_property("static_materials")))):
    mesh.set_material(slot, material)
assets.save_loaded_asset(mesh)
unreal.log("UAVSIM_DRONE_IMPORT_PASS mesh=/Game/Drone/SM_DefaultDrone textures=6")
