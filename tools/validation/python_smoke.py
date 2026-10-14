#!/usr/bin/env python3
import platform
import sys

assert sys.version_info >= (3, 9), sys.version
print(f"python={platform.python_version()} arch={platform.machine()}")
print("python-smoke: PASS")
