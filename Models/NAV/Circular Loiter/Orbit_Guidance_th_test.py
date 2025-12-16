import numpy as np
import matplotlib.pyplot as plt

#plt.rcParams['text.usetex'] = True
#plt.rcParams['font.size'] = 14
#plt.rcParams['font.family'] = 'serif'

# WGS-84
a = 6378137.0
f = 1 / 298.257223563
e2 = f * (2 - f)

def compute_radii(phi):
    sin_phi = np.sin(phi)
    denom = np.sqrt(1 - e2 * sin_phi**2)
    N = a / denom
    M = a * (1 - e2) / denom**3
    return N, M

# Parameters
lam_center_deg, phi_center_deg = 23.5888, 46.7712
lam_center = np.deg2rad(lam_center_deg)
phi_center = np.deg2rad(phi_center_deg)

N_ref, M_ref = compute_radii(phi_center)
offset_x = np.random.uniform(-1000, 1000)
offset_y = np.random.uniform(-1000, 1000)
lam_c = lam_center + offset_x / (N_ref * np.cos(phi_center))
phi_c = phi_center + offset_y / M_ref

R = 500
k = 0.25 * R
lam_orbit = 1
v = 20

box_size = 5000
lam_min = lam_center - box_size / (N_ref * np.cos(phi_center))
lam_max = lam_center + box_size / (N_ref * np.cos(phi_center))
phi_min = phi_center - box_size / M_ref
phi_max = phi_center + box_size / M_ref

# Vector Field
fig1, ax1 = plt.subplots(figsize=(11, 10))

n_grid = 40
lam_grid = np.linspace(lam_min, lam_max, n_grid)
phi_grid = np.linspace(phi_min, phi_max, n_grid)
LAM, PHI = np.meshgrid(lam_grid, phi_grid)

U = np.zeros_like(LAM)
V = np.zeros_like(PHI)

for i in range(n_grid):
    for j in range(n_grid):
        lam = LAM[i, j]
        phi = PHI[i, j]
        
        N, M = compute_radii(phi)
        x = N * np.cos(phi) * lam
        y = M * phi
        x_c = N * np.cos(phi) * lam_c
        y_c = M * phi_c
        
        dx = x - x_c
        dy = y - y_c
        
        r = np.sqrt(dx**2 + dy**2)
        theta = np.arctan2(-dx, -dy)
        
        psi = theta + lam_orbit * (np.arctan((R - r) / k) + np.pi/2)
        # Stange ... -> something fishy with the tetha
        
        U[i, j] = np.sin(psi)
        V[i, j] = np.cos(psi)

LAM_deg = np.rad2deg(LAM)
PHI_deg = np.rad2deg(PHI)
lam_c_deg = np.rad2deg(lam_c)
phi_c_deg = np.rad2deg(phi_c)

ax1.streamplot(LAM_deg, PHI_deg, U, V, 
               density=1.5, color='steelblue', linewidth=1.2, arrowsize=1.5)

circle_lam = lam_c + R * np.cos(np.linspace(0, 2*np.pi, 200)) / (N_ref * np.cos(phi_c))
circle_phi = phi_c + R * np.sin(np.linspace(0, 2*np.pi, 200)) / M_ref
ax1.plot(np.rad2deg(circle_lam), np.rad2deg(circle_phi), 'r-', linewidth=2.5, label='Target Orbit')
ax1.plot(lam_c_deg, phi_c_deg, 'ro', markersize=10, label='Center', zorder=5)

ax1.set_xlabel(r'Longitude [deg]', fontsize=14)
ax1.set_ylabel(r'Latitude [deg]', fontsize=14)
ax1.set_title(r'Vector Field ($k = ' + f'{k:.1f}' + r'$ m, CCW)', fontsize=16, fontweight='bold')
ax1.legend(fontsize=14)
ax1.grid(True, alpha=0.3)
ax1.set_xlim(np.rad2deg(lam_min), np.rad2deg(lam_max))
ax1.set_ylim(np.rad2deg(phi_min), np.rad2deg(phi_max))
ax1.set_aspect('equal')
plt.tight_layout()
plt.savefig('vector_field.png', dpi=150, bbox_inches='tight')

# Trajectories with RK4
fig2, ax2 = plt.subplots(figsize=(11, 10))

np.random.seed(42)
n_traj = 5

for traj_id in range(n_traj):
    if traj_id == 0:
        angle = np.random.uniform(0, 2*np.pi)
        radius = np.random.uniform(50, R * 0.7)
        start_x_local = radius * np.cos(angle)
        start_y_local = radius * np.sin(angle)
        
        lam = lam_c + start_x_local / (N_ref * np.cos(phi_c))
        phi = phi_c + start_y_local / M_ref
    else:
        start_offset_x = np.random.uniform(-4000, 4000)
        start_offset_y = np.random.uniform(-4000, 4000)
        lam = lam_center + start_offset_x / (N_ref * np.cos(phi_center))
        phi = phi_center + start_offset_y / M_ref
    
    lam_traj = [lam]
    phi_traj = [phi]
    
    dt = 0.5
    t_max = 800
    n_steps = int(t_max / dt)
    
    for step in range(n_steps):
        def f(lam_i, phi_i):
            N, M = compute_radii(phi_i)
            x = N * np.cos(phi_i) * lam_i
            y = M * phi_i
            x_c = N * np.cos(phi_i) * lam_c
            y_c = M * phi_i_c
            
            dx = x - x_c
            dy = y - y_c
            r = np.sqrt(dx**2 + dy**2)
            theta = np.arctan2(-dx, -dy)
            
            psi = theta + lam_orbit * (np.arctan((R - r) / k) + np.pi/2)
            
            dlam_dt = v * np.sin(psi) / (N * np.cos(phi_i))
            dphi_dt = v * np.cos(psi) / M
            
            return dlam_dt, dphi_dt
        
        phi_i_c = phi_c
        
        k1_lam, k1_phi = f(lam, phi)
        k2_lam, k2_phi = f(lam + 0.5*dt*k1_lam, phi + 0.5*dt*k1_phi)
        k3_lam, k3_phi = f(lam + 0.5*dt*k2_lam, phi + 0.5*dt*k2_phi)
        k4_lam, k4_phi = f(lam + dt*k3_lam, phi + dt*k3_phi)
        
        lam = lam + (dt/6) * (k1_lam + 2*k2_lam + 2*k3_lam + k4_lam)
        phi = phi + (dt/6) * (k1_phi + 2*k2_phi + 2*k3_phi + k4_phi)
        
        lam_traj.append(lam)
        phi_traj.append(phi)
        
        N, M = compute_radii(phi)
        x = N * np.cos(phi) * lam
        y = M * phi
        x_c = N * np.cos(phi) * lam_c
        y_c = M * phi_c
        r = np.sqrt((x-x_c)**2 + (y-y_c)**2)
        
        if step > 200 and abs(r - R) < 5:
            if step > 400:
                break
    
    color = plt.cm.plasma(traj_id / n_traj)
    ax2.plot(np.rad2deg(lam_traj), np.rad2deg(phi_traj), 
             color=color, linewidth=2, label=f'Traj {traj_id+1}', alpha=0.8)
    ax2.plot(np.rad2deg(lam_traj[0]), np.rad2deg(phi_traj[0]), 
             'o', color=color, markersize=10, markeredgecolor='black', markeredgewidth=1.5)

ax2.plot(np.rad2deg(circle_lam), np.rad2deg(circle_phi), 'r--', linewidth=2.5, label='Target Orbit')
ax2.plot(lam_c_deg, phi_c_deg, 'ro', markersize=10, label='Center', zorder=5)

ax2.set_xlabel(r'Longitude [deg]', fontsize=14)
ax2.set_ylabel(r'Latitude [deg]', fontsize=14)
ax2.set_title(r'Trajectories ($k = ' + f'{k:.1f}' + r'$ m, RK4)', fontsize=16, fontweight='bold')
ax2.legend(fontsize=14)
ax2.grid(True, alpha=0.3)
ax2.set_xlim(np.rad2deg(lam_min), np.rad2deg(lam_max))
ax2.set_ylim(np.rad2deg(phi_min), np.rad2deg(phi_max))
ax2.set_aspect('equal')
plt.tight_layout()
plt.savefig('trajectories.png', dpi=150, bbox_inches='tight')

plt.show()