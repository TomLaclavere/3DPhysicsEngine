import pandas as pd
import numpy as np
import plotly.graph_objects as go


class MotionCSV:

    def __init__(self, filepath, object_name=None):
        self.filepath = filepath
        self.object_name = object_name
        self.df = pd.read_csv(filepath)

        self.pos = self.df[["pos(x)", "pos(y)", "pos(z)"]].to_numpy()
        self.vel = self.df[["vel(x)", "vel(y)", "vel(z)"]].to_numpy()
        self.acc = self.df[["acc(x)", "acc(y)", "acc(z)"]].to_numpy()

        self.n = len(self.df)
        self.time = self.df["time"].to_numpy()
        
    def time_to_index(self, time):
        idx = np.argmin(np.abs(self.time - time))
        return int(idx)

class MotionPlotting:

    def __init__(self, objectpath, motionpaths):

        self.object_df = pd.read_csv(objectpath)

        self.motions = []
        for i, fp in enumerate(motionpaths):

            object_name = (
                self.object_df.iloc[i]["name"]
                if i < len(self.object_df)
                else None
            )

            self.motions.append(MotionCSV(fp, object_name))

        self.objectpath = objectpath
        self.motionpaths = motionpaths

    # ---------------------------
    # Multi-object plotting
    # ---------------------------

    def plot_positions(self, savepath=None):

        fig = go.Figure()

        for motion in self.motions:

            fig.add_trace(go.Scatter(
                x=motion.time,
                y=motion.pos[:, 0],
                mode="lines",
                name=f"{motion.object_name} x"
            ))

            fig.add_trace(go.Scatter(
                x=motion.time,
                y=motion.pos[:, 1],
                mode="lines",
                name=f"{motion.object_name} y"
            ))

            fig.add_trace(go.Scatter(
                x=motion.time,
                y=motion.pos[:, 2],
                mode="lines",
                name=f"{motion.object_name} z"
            ))

        fig.update_layout(
            title="Positions (x,y,z)",
            xaxis_title="step",
            yaxis_title="position"
        )

        if savepath:
            fig.write_html(savepath)
        else:
            fig.show()

    def plot_positions_z(self, savepath=None):

        fig = go.Figure()

        for motion in self.motions:
            fig.add_trace(go.Scatter(
                x=motion.time,
                y=motion.pos[:, 2],
                mode="lines",
                name=f"{motion.object_name} z"
            ))

        fig.update_layout(
            title="Positions (z)",
            xaxis_title="step",
            yaxis_title="position"
        )

        if savepath:
            fig.write_html(savepath)
        else:
            fig.show()

    def plot_velocities(self, savepath=None):

        fig = go.Figure()

        for motion in self.motions:

            fig.add_trace(go.Scatter(
                x=motion.time,
                y=motion.vel[:, 0],
                mode="lines",
                name=f"{motion.object_name} vx"
            ))

            fig.add_trace(go.Scatter(
                x=motion.time,
                y=motion.vel[:, 1],
                mode="lines",
                name=f"{motion.object_name} vy"
            ))

            fig.add_trace(go.Scatter(
                x=motion.time,
                y=motion.vel[:, 2],
                mode="lines",
                name=f"{motion.object_name} vz"
            ))

        fig.update_layout(
            title="Velocities",
            xaxis_title="step",
            yaxis_title="velocity"
        )

        if savepath:
            fig.write_html(savepath)
        else:
            fig.show()

    def plot_accelerations(self, savepath=None):

        fig = go.Figure()

        for motion in self.motions:

            fig.add_trace(go.Scatter(
                x=motion.time,
                y=motion.acc[:, 0],
                mode="lines",
                name=f"{motion.object_name} ax"
            ))

            fig.add_trace(go.Scatter(
                x=motion.time,
                y=motion.acc[:, 1],
                mode="lines",
                name=f"{motion.object_name} ay"
            ))

            fig.add_trace(go.Scatter(
                x=motion.time,
                y=motion.acc[:, 2],
                mode="lines",
                name=f"{motion.object_name} az"
            ))

        fig.update_layout(
            title="Accelerations",
            xaxis_title="step",
            yaxis_title="acceleration"
        )

        if savepath:
            fig.write_html(savepath)
        else:
            fig.show()

    # ---------------------------
    # Single-object plotting
    # ---------------------------

    def plot_position(self, index, savepath=None):

        motion = self.motions[index]
        fig = go.Figure()

        fig.add_trace(go.Scatter(x=motion.time, y=motion.pos[:, 0], mode="lines", name="x"))
        fig.add_trace(go.Scatter(x=motion.time, y=motion.pos[:, 1], mode="lines", name="y"))
        fig.add_trace(go.Scatter(x=motion.time, y=motion.pos[:, 2], mode="lines", name="z"))

        fig.update_layout(
            title=f"{motion.object_name} position",
            xaxis_title="step",
            yaxis_title="position"
        )

        if savepath:
            fig.write_html(savepath)
        else:
            fig.show()

    def plot_position_z(self, index, savepath=None):

        motion = self.motions[index]
        fig = go.Figure()

        fig.add_trace(go.Scatter(
            x=motion.time,
            y=motion.pos[:, 2],
            mode="lines",
            name="z"
        ))
        
        fig.update_layout(
            title=f"{motion.object_name} position (z)",
            xaxis_title="step",
            yaxis_title="position"
        )

        if savepath:
            fig.write_html(savepath)
        else:
            fig.show()

    def plot_velocity(self, index, savepath=None):

        motion = self.motions[index]
        fig = go.Figure()

        fig.add_trace(go.Scatter(x=motion.time, y=motion.vel[:, 0], mode="lines", name="vx"))
        fig.add_trace(go.Scatter(x=motion.time, y=motion.vel[:, 1], mode="lines", name="vy"))
        fig.add_trace(go.Scatter(x=motion.time, y=motion.vel[:, 2], mode="lines", name="vz"))

        fig.update_layout(
            title=f"{motion.object_name} velocity",
            xaxis_title="step",
            yaxis_title="velocity"
        )

        if savepath:
            fig.write_html(savepath)
        else:
            fig.show()

    def plot_acceleration(self, index, savepath=None):

        motion = self.motions[index]
        fig = go.Figure()

        fig.add_trace(go.Scatter(x=motion.time, y=motion.acc[:, 0], mode="lines", name="ax"))
        fig.add_trace(go.Scatter(x=motion.time, y=motion.acc[:, 1], mode="lines", name="ay"))
        fig.add_trace(go.Scatter(x=motion.time, y=motion.acc[:, 2], mode="lines", name="az"))

        fig.update_layout(
            title=f"{motion.object_name} acceleration",
            xaxis_title="step",
            yaxis_title="acceleration"
        )

        if savepath:
            fig.write_html(savepath)
        else:
            fig.show()
            
    # ---------------------------
    # 3d plotting
    # ---------------------------
    
    def plot_trajectory_3d(self, index, savepath=None):

        fig = go.Figure()

        motion = self.motions[index]

        fig.add_trace(go.Scatter3d(
            x=motion.pos[:, 0],
            y=motion.pos[:, 1],
            z=motion.pos[:, 2],
            mode="lines",
            name=motion.object_name
        ))

        fig.update_layout(
            title="3D trajectories",
            scene=dict(
                xaxis_title="x",
                yaxis_title="y",
                zaxis_title="z"
            )
        )

        if savepath:
            fig.write_html(savepath)
        else:
            fig.show()
            
    def plot_trajectories_3d(self, savepath=None):

        fig = go.Figure()

        for motion in self.motions:

            fig.add_trace(go.Scatter3d(
                x=motion.pos[:, 0],
                y=motion.pos[:, 1],
                z=motion.pos[:, 2],
                mode="lines",
                name=motion.object_name
            ))

        fig.update_layout(
            title="3D trajectories",
            scene=dict(
                xaxis_title="x",
                yaxis_title="y",
                zaxis_title="z"
            )
        )

        if savepath:
            fig.write_html(savepath)
        else:
            fig.show()
            
    def plot_velocity_3d(self, index, savepath=None):

        fig = go.Figure()

        motion = self.motions[index]

        fig.add_trace(go.Scatter3d(
            x=motion.vel[:, 0],
            y=motion.vel[:, 1],
            z=motion.vel[:, 2],
            mode="lines",
            name=motion.object_name
        ))

        fig.update_layout(
            title="3D trajectories",
            scene=dict(
                xaxis_title="x",
                yaxis_title="y",
                zaxis_title="z"
            )
        )

        if savepath:
            fig.write_html(savepath)
        else:
            fig.show()
            
    def plot_velocities_3d(self, savepath=None):

        fig = go.Figure()

        for motion in self.motions:

            fig.add_trace(go.Scatter3d(
                x=motion.vel[:, 0],
                y=motion.vel[:, 1],
                z=motion.vel[:, 2],
                mode="lines",
                name=motion.object_name
            ))

        fig.update_layout(
            title="3D trajectories",
            scene=dict(
                xaxis_title="x",
                yaxis_title="y",
                zaxis_title="z"
            )
        )

        if savepath:
            fig.write_html(savepath)
        else:
            fig.show()