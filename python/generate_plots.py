"""Generate plots from simulation CSV data and open them in the browser.

Usage:
    python3 python/generate_plots.py <csv_dir> <mode>

Modes:
    trajectories  -- z-position of all objects vs time
    objects       -- 3D scene at initial and final states
    animation     -- full animated 3D scene

Run from the project root. csv_dir defaults to "output/CSV".
"""

import glob
import pathlib
import sys

project_root = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(project_root))

from python.utilities.motion_utilities import MotionPlotting
from python.utilities.object_utilities import ObjectPlotting

csv_dir = sys.argv[1] if len(sys.argv) > 1 else "output/CSV"
mode = sys.argv[2] if len(sys.argv) > 2 else "trajectories"

objectpath = f"{csv_dir}/objects.csv"
motionpaths = sorted(glob.glob(f"{csv_dir}/motion_*.csv"))

if not pathlib.Path(objectpath).exists():
    print(f"No objects.csv found in {csv_dir}. Run the simulation with save=true first.")
    sys.exit(1)

if not motionpaths:
    print(f"No motion_*.csv files found in {csv_dir}.")
    sys.exit(1)

mp = MotionPlotting(objectpath, motionpaths)
op = ObjectPlotting(objectpath, motionpaths)

if mode == "trajectories":
    mp.plot_positions()
elif mode == "objects":
    op.plot_objects_init()
    op.plot_objects_final()
elif mode == "animation":
    op.animate_objects()
else:
    print(f"Unknown mode '{mode}'. Expected: trajectories, objects, animation.")
    sys.exit(1)
