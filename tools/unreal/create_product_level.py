"""Create the reproducible lightweight product map using installed engine assets."""
import unreal

levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
assets.make_directory("/Game/Maps")
path = "/Game/Maps/UAVSim_Test"
if assets.does_asset_exist(path):
    if not levels.load_level(path):
        raise RuntimeError("Existing product map could not be loaded")
    unreal.log("UAVSIM_LEVEL_EXISTS " + path)
else:
    if not levels.new_level(path):
        raise RuntimeError("Product map creation failed")
    cube = unreal.load_asset("/Engine/BasicShapes/Cube")
    floor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 0, -50))
    floor.set_actor_label("UAVSim flat test ground")
    floor.static_mesh_component.set_static_mesh(cube)
    floor.set_actor_scale3d(unreal.Vector(200, 200, 1))
    # A contrasting landing pad gives altitude and motion visible reference points.
    pad = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 0, 0.5))
    pad.set_actor_label("Landing pad visual")
    pad.static_mesh_component.set_static_mesh(unreal.load_asset("/Engine/BasicShapes/Cylinder"))
    pad.set_actor_scale3d(unreal.Vector(3, 3, 0.01))
    light = actors.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 500), unreal.Rotator(-45, -25, 0))
    light.light_component.set_editor_property("intensity", 4.0)
    actors.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 400))
    actors.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector(0, 0, 0))
    actors.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(0, 0, 10))
    if not levels.save_current_level():
        raise RuntimeError("Product map save failed")
    unreal.log("UAVSIM_LEVEL_CREATE_PASS " + path)
