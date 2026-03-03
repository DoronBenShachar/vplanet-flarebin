import glob
import os
import shutil
import subprocess as subp
import sys
import pytest

# Ensure tests import the local repository package, not any site-packages
# namespace/module named "vplanet".
REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), os.pardir))
if REPO_ROOT not in sys.path:
    sys.path.insert(0, REPO_ROOT)

loaded_vplanet = sys.modules.get("vplanet")
if loaded_vplanet is not None:
    loaded_file = getattr(loaded_vplanet, "__file__", None)
    if loaded_file is None or not os.path.abspath(loaded_file).startswith(REPO_ROOT + os.sep):
        del sys.modules["vplanet"]

import vplanet
from vplanet import custom_units  # noqa: F401

# Make the `benchmark` script discoverable by the tests
sys.path.insert(1, os.path.abspath(os.path.dirname(__file__)))

# Set to False to keep .log, .forward, etc files
CLEAN_OUTPUTS = False


@pytest.fixture(scope="module")
def vplanet_output(request):
    path = os.path.abspath(os.path.dirname(request.fspath))
    infile = os.path.join(path, "vpl.in")
    output = vplanet.run(infile, quiet=False, clobber=True, C=True)
    yield output
    if CLEAN_OUTPUTS:
        for file in (
            glob.glob(f"{path}/*.log")
            + glob.glob(f"{path}/*.forward")
            + glob.glob(f"{path}/*.backward")
            + glob.glob(f"{path}/*.Climate")
        ):
            os.remove(file)
        for directory in glob.glob(f"{path}/SeasonalClimateFiles"):
            shutil.rmtree(directory)
