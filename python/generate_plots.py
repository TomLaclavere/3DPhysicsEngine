"""Generate plots from simulation CSV data and open them in the browser.

Usage:
    python3 python/generate_plots.py [csv_dir]

Run from the project root. csv_dir defaults to "output/CSV".
"""

import glob
import pathlib
import sys

project_root = pathlib.Path(__file__).resolve().parent.parent
sys.path.insert(0, str(project_root))

from python.utilities.motion_utilities import MotionPlotting  # noqa: E402
from python.utilities.object_utilities import ObjectPlotting  # noqa: E402

csv_dir = sys.argv[1] if len(sys.argv) > 1 else "output/CSV"
objectpath = f"{csv_dir}/objects.csv"
motionpaths = sorted(glob.glob(f"{csv_dir}/motion_*.csv"))

if not pathlib.Path(objectpath).exists():
    print(f"No objects.csv found in {csv_dir}. Run the simulation with CSV saving enabled first.")
    sys.exit(1)

if not motionpaths:
    print(f"No motion_*.csv files found in {csv_dir}.")
    sys.exit(1)

mp = MotionPlotting(objectpath, motionpaths)
op = ObjectPlotting(objectpath, motionpaths)

mp.plot_positions_z()
op.plot_objects_init()
