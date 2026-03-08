import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

class MotionCSV:
    def __init__(self, filepath, object_name=None):
        self.filepath = filepath
        self.object_name = object_name
        self.df = pd.read_csv(filepath)

        self.pos = self.df[["pos(x)", "pos(y)", "pos(z)"]].to_numpy()
        self.vel = self.df[["vel(x)", "vel(y)", "vel(z)"]].to_numpy()
        self.acc = self.df[["acc(x)", "acc(y)", "acc(z)"]].to_numpy()

        self.n = len(self.df)
        self.t = np.arange(self.n)



class MotionCSVManager:
    def __init__(self, objectpath, motionpaths):
        self.object_df = pd.read_csv(objectpath)

        self.motions = []
        for i, fp in enumerate(motionpaths):
            object_name = self.object_df.iloc[i]["name"] if i < len(self.object_df) else None
            print(object_name)
            self.motions.append(MotionCSV(fp, object_name))

        self.objectpath = objectpath
        self.motionpaths = motionpaths

    # ----------------------------
    # Multi-object plotting
    # ----------------------------
    def plot_positions(self, savepath=None):
        plt.figure()
        for motion in self.motions:
            plt.plot(motion.t, motion.pos[:, 0], label=f"{motion.object_name} x")
            plt.plot(motion.t, motion.pos[:, 1], label=f"{motion.object_name} y")
            plt.plot(motion.t, motion.pos[:, 2], label=f"{motion.object_name} z")

        plt.title("Positions (x,y,z)")
        plt.xlabel("step")
        plt.ylabel("position")
        plt.legend()
        plt.grid()
        if savepath:
            plt.savefig(savepath)
            plt.close()
        else:
            plt.show()
            
    def plot_positions_z(self, savepath=None):
        plt.figure()
        for motion in self.motions:
            plt.plot(motion.t, motion.pos[:, 2], label=f"{motion.object_name} (z)")

        plt.title("Positions (z)")
        plt.xlabel("step")
        plt.ylabel("position")
        plt.legend()
        plt.grid()
        if savepath:
            plt.savefig(savepath)
            plt.close()
        else:
            plt.show()

    def plot_velocities(self, savepath=None):
        plt.figure()
        for motion in self.motions:
            plt.plot(motion.t, motion.vel[:, 0], label=f"{motion.object_name} vx")
            plt.plot(motion.t, motion.vel[:, 1], label=f"{motion.object_name} vy")
            plt.plot(motion.t, motion.vel[:, 2], label=f"{motion.object_name} vz")

        plt.title("Velocities (vx,vy,vz)")
        plt.xlabel("step")
        plt.ylabel("velocity")
        plt.legend()
        plt.grid()
        if savepath:
            plt.savefig(savepath)
            plt.close()
        else:
            plt.show()

    def plot_accelerations(self, savepath=None):
        plt.figure()
        for motion in self.motions:
            plt.plot(motion.t, motion.acc[:, 0], label=f"{motion.object_name} ax")
            plt.plot(motion.t, motion.acc[:, 1], label=f"{motion.object_name} ay")
            plt.plot(motion.t, motion.acc[:, 2], label=f"{motion.object_name} az")

        plt.title("Accelerations (ax,ay,az)")
        plt.xlabel("step")
        plt.ylabel("acceleration")
        plt.legend()
        plt.grid()
        if savepath:
            plt.savefig(savepath)
            plt.close()
        else:
            plt.show()

