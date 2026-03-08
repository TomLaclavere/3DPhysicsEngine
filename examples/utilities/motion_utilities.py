import pandas as pd
import numpy as np
import matplotlib.pyplot as plt


class MotionCSV:

    def __init__(self, filepath):
        self.filepath = filepath
        self.df = pd.read_csv(filepath)

        self.pos = self.df[["pos(x)", "pos(y)", "pos(z)"]].to_numpy()
        self.vel = self.df[["vel(x)", "vel(y)", "vel(z)"]].to_numpy()
        self.acc = self.df[["acc(x)", "acc(y)", "acc(z)"]].to_numpy()

        self.n = len(self.df)
        self.t = np.arange(self.n)

    # =============================
    # Plot Utilities
    # =============================

    def plot_position(self):
        plt.figure()
        plt.plot(self.t, self.pos[:, 0], label="x")
        plt.plot(self.t, self.pos[:, 1], label="y")
        plt.plot(self.t, self.pos[:, 2], label="z")

        plt.title("Position")
        plt.xlabel("step")
        plt.ylabel("position")
        plt.legend()
        plt.grid()
    
    def plot_position_z(self):
        plt.figure()
        plt.plot(self.t, self.pos[:, 2], label="z")

        plt.title("Position")
        plt.xlabel("step")
        plt.ylabel("position")
        plt.legend()
        plt.grid()

    def plot_velocity(self):
        plt.figure()
        plt.plot(self.t, self.vel[:, 0], label="vx")
        plt.plot(self.t, self.vel[:, 1], label="vy")
        plt.plot(self.t, self.vel[:, 2], label="vz")

        plt.title("Velocity")
        plt.xlabel("step")
        plt.ylabel("velocity")
        plt.legend()
        plt.grid()

    def plot_acceleration(self):
        plt.figure()
        plt.plot(self.t, self.acc[:, 0], label="ax")
        plt.plot(self.t, self.acc[:, 1], label="ay")
        plt.plot(self.t, self.acc[:, 2], label="az")

        plt.title("Acceleration")
        plt.xlabel("step")
        plt.ylabel("acceleration")
        plt.legend()
        plt.grid()

    # =============================
    # Plot norms
    # =============================

    def plot_norms(self):
        plt.figure()

        plt.plot(self.t, self.position_norm(), label="|pos|")
        plt.plot(self.t, self.velocity_norm(), label="|vel|")
        plt.plot(self.t, self.acceleration_norm(), label="|acc|")

        plt.title("Vector norms")
        plt.xlabel("step")
        plt.legend()
        plt.grid()

    # =============================
    # Phase space
    # =============================

    def plot_phase_xy(self):
        plt.figure()
        plt.plot(self.pos[:, 0], self.pos[:, 1])
        plt.xlabel("x")
        plt.ylabel("y")
        plt.title("Phase space (x,y)")
        plt.grid()

    # =============================
    # Summary
    # =============================

    def summary(self):
        print("Number of samples:", self.n)
        print("Position range:", self.pos.min(), "->", self.pos.max())
        print("Velocity range:", self.vel.min(), "->", self.vel.max())
        print("Acceleration range:", self.acc.min(), "->", self.acc.max())
        
        
class MotionCSVManager:
    def __init__(self, filepaths):
        """
        filepaths : list of CSV file paths
        """
        self.motions = [MotionCSV(fp) for fp in filepaths]
        self.filepaths = filepaths

    # =============================
    # Summary for all files
    # =============================
    def summary_all(self):
        for i, motion in enumerate(self.motions):
            print(f"--- File {i}: {self.filepaths[i]} ---")
            motion.summary()
            print()

    # =============================
    # Plotting Utilities
    # =============================
    def plot_positions_all(self):
        for motion in self.motions:
            motion.plot_position()
        plt.show()

    def plot_positions_z_all(self):
        for motion in self.motions:
            motion.plot_position_z()
        plt.show()

    def plot_velocities_all(self):
        for motion in self.motions:
            motion.plot_velocity()
        plt.show()

    def plot_accelerations_all(self):
        for motion in self.motions:
            motion.plot_acceleration()
        plt.show()

    def plot_norms_all(self):
        for motion in self.motions:
            motion.plot_norms()
        plt.show()

    def plot_phase_xy_all(self):
        for motion in self.motions:
            motion.plot_phase_xy()
        plt.show()
