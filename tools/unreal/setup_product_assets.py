"""Run the reviewed local model import and reproducible test-level creation."""
from pathlib import Path
import runpy
import unreal

scripts = Path(__file__).resolve().parent
runpy.run_path(str(scripts / "import_drone.py"), run_name="__main__")
runpy.run_path(str(scripts / "create_product_level.py"), run_name="__main__")
unreal.log("UAVSIM_PRODUCT_ASSET_SETUP_PASS")
