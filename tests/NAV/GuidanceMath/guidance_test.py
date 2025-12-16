import numpy as np
import matplotlib.pyplot as plt
import serial
import time

a = 6378137.0
f = 1 / 298.257223563
e2 = f * (2 - f)

MODE_P2P = 0
MODE_LOITER = 1

def compute_radii(phi):
    sin_phi = np.sin(phi)
    denom = np.sqrt(1 - e2 * sin_phi**2)
    N = a / denom
    M = a * (1 - e2) / denom**3
    return N, M

def rk4_step(lam, phi, psi, v, dt):
    def derivatives(lam_i, phi_i, psi_i):
        N, M = compute_radii(phi_i)
        dlam = v * np.sin(psi_i) / (N * np.cos(phi_i))
        dphi = v * np.cos(psi_i) / M
        return dlam, dphi
    
    k1_lam, k1_phi = derivatives(lam, phi, psi)
    k2_lam, k2_phi = derivatives(lam + 0.5*dt*k1_lam, phi + 0.5*dt*k1_phi, psi)
    k3_lam, k3_phi = derivatives(lam + 0.5*dt*k2_lam, phi + 0.5*dt*k2_phi, psi)
    k4_lam, k4_phi = derivatives(lam + dt*k3_lam, phi + dt*k3_phi, psi)
    
    lam_new = lam + (dt/6) * (k1_lam + 2*k2_lam + 2*k3_lam + k4_lam)
    phi_new = phi + (dt/6) * (k1_phi + 2*k2_phi + 2*k3_phi + k4_phi)
    
    return lam_new, phi_new

PORT = '/dev/ttyACM0'
BAUD = 115200

ser = serial.Serial(PORT, BAUD, timeout=1)
time.sleep(2)

ready = ser.readline().decode().strip()
if ready != "READY":
    print("Arduino not ready")
    exit(1)

lam_center = np.deg2rad(23.5888)
phi_center = np.deg2rad(46.7712)

N_ref, M_ref = compute_radii(phi_center)
pattern_radius = 2000.0

num_waypoints = 89
waypoints = []

for i in range(num_waypoints):
    angle = 2 * np.pi * i / num_waypoints
    x_offset = pattern_radius * np.cos(angle)
    y_offset = pattern_radius * np.sin(angle)
    
    lam_wp = lam_center + x_offset / (N_ref * np.cos(phi_center))
    phi_wp = phi_center + y_offset / M_ref
    
    waypoints.append((lam_wp, phi_wp))

lambda_c = lam_center
phi_c = phi_center
R = 500.0
k = 125.0
lambda_orbit = 1

v = 10.0
dt_integr = 0.05
dt_comm = 0.3
comm_counter = 0
waypoint_threshold = 0.00002

lam = lam_center - 0.00001
phi = phi_center

psi = 0.0

current_waypoint = 0
mode = MODE_P2P

lam_traj = [lam]
phi_traj = [phi]

plt.ion()
fig, ax = plt.subplots(figsize=(12, 12))

max_steps = 50000

for step in range(max_steps):
    if comm_counter % int(dt_comm / dt_integr) == 0:
        
        if mode == MODE_P2P:
            lambda_target, phi_target = waypoints[current_waypoint]
            msg = f"{mode},{lam:.9f},{phi:.9f},{lambda_target:.9f},{phi_target:.9f}\n"
            
            dx = lambda_target - lam
            dy = phi_target - phi
            dist = np.sqrt(dx*dx + dy*dy)
            
            if dist < waypoint_threshold:
                current_waypoint += 1
                if current_waypoint >= len(waypoints):
                    mode = MODE_LOITER
        else:
            msg = f"{mode},{lam:.9f},{phi:.9f},{lambda_c:.9f},{phi_c:.9f},{R},{k},{lambda_orbit}\n"
        
        ser.write(msg.encode())
        
        response = ser.readline().decode().strip()
        if response:
            psi = float(response)
    
    lam, phi = rk4_step(lam, phi, psi, v, dt_integr)
    
    lam_traj.append(lam)
    phi_traj.append(phi)
    
    comm_counter += 1
    
    if step % 40 == 0:
        ax.clear()
        ax.plot(np.rad2deg(lam_traj), np.rad2deg(phi_traj), 'b-', linewidth=1.5, alpha=0.7)
        
        wp_lam = [np.rad2deg(wp[0]) for wp in waypoints]
        wp_phi = [np.rad2deg(wp[1]) for wp in waypoints]
        ax.plot(wp_lam, wp_phi, 'ro', markersize=4, alpha=0.5)
        
        if mode == MODE_LOITER:
            circle_lam = lambda_c + R * np.cos(np.linspace(0, 2*np.pi, 200)) / (N_ref * np.cos(phi_c))
            circle_phi = phi_c + R * np.sin(np.linspace(0, 2*np.pi, 200)) / M_ref
            ax.plot(np.rad2deg(circle_lam), np.rad2deg(circle_phi), 'g--', linewidth=2)
        
        ax.plot(np.rad2deg(lam), np.rad2deg(phi), 'go', markersize=8)
        ax.set_xlabel('Longitude [deg]')
        ax.set_ylabel('Latitude [deg]')
        ax.set_aspect('equal')
        ax.grid(True, alpha=0.3)
        mode_str = "P2P" if mode == MODE_P2P else "LOITER"
        ax.set_title(f'Mission: {num_waypoints} waypoints | Mode: {mode_str} | WP: {current_waypoint}/{len(waypoints)}')
        plt.pause(0.001)

ser.close()
plt.ioff()
plt.show()