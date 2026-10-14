"""Dependency-free verification; no network or global environment mutation."""
import json
import logging
import pathlib
import sys
import tempfile

logging.basicConfig(level=logging.INFO, format="%(levelname)s: %(message)s")
def main():
    if sys.version_info[:2] != (3, 12) or sys.prefix == sys.base_prefix:
        logging.error("Python 3.12 project virtual environment is required")
        return 1
    with tempfile.TemporaryDirectory() as directory:
        path = pathlib.Path(directory) / "smoke.json"
        path.write_text(json.dumps({"value": 42}), encoding="utf-8")
        if json.loads(path.read_text(encoding="utf-8"))["value"] != 42:
            logging.error("JSON filesystem roundtrip failed")
            return 1
    logging.info("Python %s venv/import/filesystem smoke PASS", sys.version.split()[0])
    return 0

if __name__ == "__main__":
    sys.exit(main())
