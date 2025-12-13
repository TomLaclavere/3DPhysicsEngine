import warnings

import numpy as np

warnings.filterwarnings("ignore", category=RuntimeWarning)


class Object:
    def __init__(self, pos, vel, collision_condition, g=np.array([0, 0, -9.81])):
        """Object

        Class to descriebe physical objects following the classical relation of Motion. It is very not general, and should be used carrefully. It's goal is only to solve the motion equation in very simple case, in order to provide analytical value to test with the numerical implementation in C++.
        This class may be updated in the future, if needed.

        The motion equation is the following :

            a*t**2 + b*t + c = x

        Which becomes in free fall case :

            0.5 * g * t**2 + v_0 * t + x_0 = x

        Parameters
        ----------
        pos : array
            3D vector to describe initial object's position.
        vel : array
            3D vector to describe initial object's velocity.
        collision_condition : array
            3D vector to describe position when collision (usually with the ground) occurs.
        g : float, optional
            gravtitational acceleration, by default 9.81
        """
        self.pos = np.array(pos)
        self.vel = np.array(vel)
        self.collision_condition = np.array(collision_condition)
        self.g = g

    def get_a(self):
        return 0.5 * self.g

    def get_b(self):
        return self.vel

    def get_c(self):
        return self.pos - self.collision_condition

    def get_eq(self):
        return self.get_a(), self.get_b(), self.get_c()

    def get_eq_z(self):
        return self.get_a()[2], self.get_b()[2], self.get_c()[2]


class SecondOrderEq:
    def __init__(self, a, b, c):
        self.a = a
        self.b = b
        self.c = c

    def discriminant(self):
        return self.b**2 - 4 * self.a * self.c

    def roots(self):
        r1 = (-self.b - np.sqrt(self.discriminant())) / (2 * self.a)
        r2 = (-self.b + np.sqrt(self.discriminant())) / (2 * self.a)

        return r1, r2

    def get_time_contact_ground(self):
        r1, r2 = self.roots()
        r1 = r1[2]
        r2 = r2[2]

        if r1 < 0:
            r = r2
        else:
            r = r1

        return r

    def get_distance_at_contact_ground(self):
        t = self.get_time_contact_ground()
        return self.a * t**2 + self.b * t + self.c
