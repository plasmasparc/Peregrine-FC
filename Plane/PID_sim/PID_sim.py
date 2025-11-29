import numpy as np
import matplotlib.pyplot as plt
from scipy.integrate import solve_ivp

# ====================================================================================
# SIMULATION PARAMETERS
# ====================================================================================

# Atmospheric conditions
altitude = 340  # m
T_celsius = 15  # °C
T = T_celsius + 273.15  # K
p = 101325 * (1 - 0.0065 * altitude / 288.15) ** 5.255  # Pa
rho = p / (287.05 * T)  # kg/m³

# Moments of inertia (cross model estimation)
Ix = 0.075  # kg⋅m² (roll)
Iy = 0.075  # kg⋅m² (pitch)
Iz = 0.147  # kg⋅m² (yaw)

# Flight velocity
v = 15  # m/s

# Elevator parameters
elevator_area = 0.46 * 0.025  # m²
elevator_lever = 0.60  # m
dCl_ddelta = 2.865  # /rad

# Control limits
max_deflection = 10  # degrees
control_delay = 0.0  # seconds (can be adjusted)

# PID parameters (adjustable)
Kp = 0.5  # Proportional gain
Ki = 0.1  # Integral gain
Kd = 0.2  # Derivative gain

# Target pitch parameters
pitch_amplitude = 10  # degrees
pitch_frequency = 0.2  # Hz

# Simulation time
t_end = 20.0  # seconds
dt = 0.001  # time step for output

print(f"\n{'='*80}")
print(f"PITCH ATTITUDE CONTROL SIMULATION - PHOENIX V2")
print(f"{'='*80}\n")
print(f"Atmospheric Conditions:")
print(f"  Altitude: {altitude} m")
print(f"  Temperature: {T_celsius}°C")
print(f"  Air Density: {rho:.4f} kg/m³\n")
print(f"Aircraft Parameters:")
print(f"  Iy (pitch): {Iy} kg⋅m²")
print(f"  Velocity: {v} m/s\n")
print(f"PID Gains:")
print(f"  Kp = {Kp}")
print(f"  Ki = {Ki}")
print(f"  Kd = {Kd}\n")
print(f"Control Limits:")
print(f"  Max deflection: ±{max_deflection}°")
print(f"  Control delay: {control_delay} s\n")
print(f"Target Signal:")
print(f"  Amplitude: ±{pitch_amplitude}°")
print(f"  Frequency: {pitch_frequency} Hz\n")

# ====================================================================================
# TORQUE CALCULATION FUNCTION
# ====================================================================================

def calculate_pitch_torque(delta_deg, v, rho):
    """Calculate pitch torque from elevator deflection"""
    delta_rad = np.deg2rad(delta_deg)
    q = 0.5 * rho * v**2
    delta_L = dCl_ddelta * delta_rad * q * elevator_area
    torque = delta_L * elevator_lever
    return torque

# ====================================================================================
# DYNAMICS MODEL (4th order RK integration via solve_ivp)
# ====================================================================================

class PitchController:
    def __init__(self, Kp, Ki, Kd):
        self.Kp = Kp
        self.Ki = Ki
        self.Kd = Kd
        self.integral = 0.0
        self.prev_error = 0.0
        
    def update(self, error, dt):
        """PID controller update"""
        self.integral += error * dt
        derivative = (error - self.prev_error) / dt if dt > 0 else 0.0
        self.prev_error = error
        
        output = self.Kp * error + self.Ki * self.integral + self.Kd * derivative
        return output

def target_pitch(t):
    """Target pitch angle as function of time"""
    return pitch_amplitude * np.sin(2 * np.pi * pitch_frequency * t)

def target_pitch_rate(t):
    """Target pitch rate (derivative of target)"""
    return pitch_amplitude * 2 * np.pi * pitch_frequency * np.cos(2 * np.pi * pitch_frequency * t)

# Global controller instance
controller = PitchController(Kp, Ki, Kd)

def dynamics(t, state):
    """
    State vector: [pitch_angle, pitch_rate]
    pitch_angle in degrees
    pitch_rate in deg/s
    """
    theta, theta_dot = state
    
    # Target
    theta_target = target_pitch(t)
    
    # Error
    error = theta_target - theta
    
    # PID control output (elevator deflection in degrees)
    delta = controller.update(error, dt)
    
    # Saturate control
    delta = np.clip(delta, -max_deflection, max_deflection)
    
    # Calculate torque
    M = calculate_pitch_torque(delta, v, rho)
    
    # Angular acceleration (deg/s²)
    theta_ddot = np.rad2deg(M / Iy)
    
    return [theta_dot, theta_ddot]

# ====================================================================================
# SIMULATION
# ====================================================================================

# Initial conditions: [pitch_angle, pitch_rate]
initial_state = [0.0, 0.0]

# Time span
t_span = (0, t_end)
t_eval = np.arange(0, t_end, dt)

# Solve using RK45 (4th-5th order Runge-Kutta)
print(f"Running simulation...")
sol = solve_ivp(dynamics, t_span, initial_state, method='RK45', 
                t_eval=t_eval, max_step=dt)

# Extract results
time = sol.t
pitch_actual = sol.y[0]
pitch_rate = sol.y[1]
pitch_target = np.array([target_pitch(t) for t in time])

# Calculate control signal and error
error_history = pitch_target - pitch_actual
control_signal = []

controller_replay = PitchController(Kp, Ki, Kd)
for i, t in enumerate(time):
    err = error_history[i]
    ctrl = controller_replay.update(err, dt)
    ctrl = np.clip(ctrl, -max_deflection, max_deflection)
    control_signal.append(ctrl)

control_signal = np.array(control_signal)

print(f"✓ Simulation complete")

# ====================================================================================
# PLOTTING
# ====================================================================================

fig, axes = plt.subplots(3, 1, figsize=(14, 10))
fig.suptitle(f'Pitch Attitude Control Simulation - Phoenix V2\n' +
             f'Kp={Kp}, Ki={Ki}, Kd={Kd} | Target: {pitch_amplitude}° @ {pitch_frequency} Hz',
             fontsize=14, fontweight='bold')

# Subplot 1: Pitch angle tracking
ax = axes[0]
ax.plot(time, pitch_target, 'b--', linewidth=2, label='Target Pitch', alpha=0.7)
ax.plot(time, pitch_actual, 'r-', linewidth=2, label='Actual Pitch')
ax.fill_between(time, pitch_target, pitch_actual, alpha=0.2, color='orange')
ax.set_ylabel('Pitch Angle (deg)', fontsize=11, fontweight='bold')
ax.set_title('Pitch Tracking Performance', fontsize=12, fontweight='bold')
ax.grid(True, alpha=0.3)
ax.legend(loc='upper right', fontsize=10)
ax.set_xlim(0, t_end)

# Subplot 2: Tracking error
ax = axes[1]
ax.plot(time, error_history, 'g-', linewidth=2)
ax.axhline(0, color='k', linestyle='--', linewidth=1, alpha=0.5)
ax.fill_between(time, 0, error_history, alpha=0.3, color='green')
ax.set_ylabel('Error (deg)', fontsize=11, fontweight='bold')
ax.set_title('Tracking Error (Target - Actual)', fontsize=12, fontweight='bold')
ax.grid(True, alpha=0.3)
ax.set_xlim(0, t_end)

# Subplot 3: Control signal (elevator deflection)
ax = axes[2]
ax.plot(time, control_signal, 'm-', linewidth=2, label='Elevator Deflection')
ax.axhline(max_deflection, color='r', linestyle='--', linewidth=1.5, 
           alpha=0.7, label=f'Saturation (±{max_deflection}°)')
ax.axhline(-max_deflection, color='r', linestyle='--', linewidth=1.5, alpha=0.7)
ax.axhline(0, color='k', linestyle='-', linewidth=0.8, alpha=0.3)
ax.fill_between(time, -max_deflection, max_deflection, alpha=0.1, color='red')
ax.set_xlabel('Time (s)', fontsize=11, fontweight='bold')
ax.set_ylabel('Deflection (deg)', fontsize=11, fontweight='bold')
ax.set_title('Elevator Control Signal', fontsize=12, fontweight='bold')
ax.grid(True, alpha=0.3)
ax.legend(loc='upper right', fontsize=10)
ax.set_xlim(0, t_end)

plt.tight_layout()
plt.savefig('pitch_control_simulation.png', dpi=300, bbox_inches='tight')
plt.show()
plt.close()

print(f"✓ Saved: pitch_control_simulation.png")

# ====================================================================================
# PERFORMANCE METRICS
# ====================================================================================

# RMS error
rms_error = np.sqrt(np.mean(error_history**2))

# Max error
max_error = np.max(np.abs(error_history))

# Settling time (2% criterion)
settling_threshold = 0.02 * pitch_amplitude
settled_indices = np.where(np.abs(error_history) < settling_threshold)[0]
settling_time = time[settled_indices[0]] if len(settled_indices) > 0 else t_end

# Control effort (integral of squared control signal)
control_effort = np.trapz(control_signal**2, time)

print(f"\n{'='*80}")
print(f"PERFORMANCE METRICS")
print(f"{'='*80}")
print(f"  RMS Error: {rms_error:.4f}°")
print(f"  Max Error: {max_error:.4f}°")
print(f"  Settling Time (2%): {settling_time:.4f} s")
print(f"  Control Effort: {control_effort:.4f}")
print(f"{'='*80}\n")

# ====================================================================================
# EXPORT DATA
# ====================================================================================

with open('pitch_simulation_data.csv', 'w') as f:
    f.write('Time (s),Target Pitch (deg),Actual Pitch (deg),Error (deg),Control (deg)\n')
    for i in range(len(time)):
        f.write(f'{time[i]:.6f},{pitch_target[i]:.6f},{pitch_actual[i]:.6f},')
        f.write(f'{error_history[i]:.6f},{control_signal[i]:.6f}\n')

print(f"✓ Saved: pitch_simulation_data.csv\n")