import numpy as np
import matplotlib.pyplot as plt
import os

output_dir = "torque_estimate"
os.makedirs(output_dir, exist_ok=True)

# Atmospheric conditions at 340m altitude, 15°C
altitude = 340  # m
T_celsius = 15
T = T_celsius + 273.15  # K
p = 101325 * (1 - 0.0065 * altitude / 288.15) ** 5.255  # Pa
rho = p / (287.05 * T)  # kg/m³

print(f"\nAtmospheric Conditions:")
print(f"  Altitude: {altitude} m")
print(f"  Temperature: {T_celsius}°C ({T:.2f} K)")
print(f"  Pressure: {p:.1f} Pa")
print(f"  Air Density: {rho:.4f} kg/m³\n")

# Control surfaces geometry
aileron = {
    'name': 'Aileron',
    'area': 0.035 * 0.39,  # m² (one aileron)
    'lever': 0.44,  # m from CG
    'count': 2,  # two ailerons working together
    'dCl_ddelta': 2.865  # /rad (≈ 0.05 /deg)
}

elevator = {
    'name': 'Elevator',
    'area': 0.46 * 0.025,  # m²
    'lever': 0.60,  # m from CG
    'count': 1,
    'dCl_ddelta': 2.865  # /rad
}

rudder = {
    'name': 'Rudder',
    'area': 0.05 * 0.18,  # m²
    'lever': 0.65,  # m from CG
    'count': 1,
    'dCl_ddelta': 2.865  # /rad
}

def calculate_torque(surface, delta_deg, v, rho):
    """
    Calculate torque about CG for a control surface
    
    Args:
        surface: dict with area, lever, count, dCl_ddelta
        delta_deg: deflection angle in degrees
        v: velocity in m/s
        rho: air density in kg/m³
    
    Returns:
        torque in N⋅m
    """
    delta_rad = np.deg2rad(delta_deg)
    q = 0.5 * rho * v**2  # dynamic pressure (Pa)
    
    # Lift increment per surface
    delta_L = surface['dCl_ddelta'] * delta_rad * q * surface['area']  # N
    
    # Torque about CG
    torque = surface['count'] * delta_L * surface['lever']  # N⋅m
    
    return torque

# Velocities and deflection range
velocities = [7, 10, 15, 20]  # m/s
deflections = np.linspace(-20, 20, 81)  # degrees

# Calculate torques for all velocities
results = {}
for v in velocities:
    results[v] = {
        'aileron': np.array([calculate_torque(aileron, d, v, rho) for d in deflections]),
        'elevator': np.array([calculate_torque(elevator, d, v, rho) for d in deflections]),
        'rudder': np.array([calculate_torque(rudder, d, v, rho) for d in deflections])
    }

# ====================================================================================
# PLOT 1: Detailed analysis at 7 m/s with 4 subplots
# ====================================================================================

fig, axes = plt.subplots(2, 2, figsize=(15, 11))
fig.suptitle(f'Phoenix V2 Control Surface Torque Analysis at 7 m/s\n' + 
             f'Altitude: {altitude}m, Temperature: {T_celsius}°C, ρ = {rho:.4f} kg/m³', 
             fontsize=14, fontweight='bold')

v = 7
idx_20 = np.where(deflections == 20)[0][0]
idx_m20 = np.where(deflections == -20)[0][0]

# Subplot 1: All three torques
ax = axes[0, 0]
ax.plot(deflections, results[v]['aileron'], 'b-', linewidth=2.5, label='Aileron (Roll)')
ax.plot(deflections, results[v]['elevator'], 'r-', linewidth=2.5, label='Elevator (Pitch)')
ax.plot(deflections, results[v]['rudder'], 'g-', linewidth=2.5, label='Rudder (Yaw)')
ax.axhline(0, color='k', linestyle='-', linewidth=0.8, alpha=0.3)
ax.axvline(0, color='k', linestyle='-', linewidth=0.8, alpha=0.3)
ax.axvline(-20, color='gray', linestyle='--', alpha=0.4)
ax.axvline(20, color='gray', linestyle='--', alpha=0.4)
ax.scatter([20, -20], [results[v]['aileron'][idx_20], results[v]['aileron'][idx_m20]], 
           color='b', s=80, zorder=5, edgecolors='black', linewidth=1)
ax.scatter([20, -20], [results[v]['elevator'][idx_20], results[v]['elevator'][idx_m20]], 
           color='r', s=80, zorder=5, edgecolors='black', linewidth=1)
ax.scatter([20, -20], [results[v]['rudder'][idx_20], results[v]['rudder'][idx_m20]], 
           color='g', s=80, zorder=5, edgecolors='black', linewidth=1)
ax.set_xlabel('Deflection Angle (deg)', fontsize=11, fontweight='bold')
ax.set_ylabel('Torque (N⋅m)', fontsize=11, fontweight='bold')
ax.set_title('Control Surface Torques', fontsize=12, fontweight='bold')
ax.grid(True, alpha=0.3)
ax.legend(loc='upper left', fontsize=10)

# Subplot 2: Aileron only
ax = axes[0, 1]
ax.plot(deflections, results[v]['aileron'], 'b-', linewidth=3)
ax.fill_between(deflections, 0, results[v]['aileron'], alpha=0.2, color='blue')
ax.axhline(0, color='k', linestyle='-', linewidth=0.8, alpha=0.3)
ax.axvline(0, color='k', linestyle='-', linewidth=0.8, alpha=0.3)
ax.axvline(-20, color='gray', linestyle='--', alpha=0.4)
ax.axvline(20, color='gray', linestyle='--', alpha=0.4)
ax.scatter([20, -20], [results[v]['aileron'][idx_20], results[v]['aileron'][idx_m20]], 
           color='b', s=100, zorder=5, edgecolors='black', linewidth=1.5)
ax.text(20, results[v]['aileron'][idx_20], f'  {results[v]["aileron"][idx_20]:.4f} N⋅m', 
        va='bottom', ha='left', fontsize=9, fontweight='bold')
ax.text(-20, results[v]['aileron'][idx_m20], f'{results[v]["aileron"][idx_m20]:.4f} N⋅m  ', 
        va='top', ha='right', fontsize=9, fontweight='bold')
ax.set_xlabel('Deflection Angle (deg)', fontsize=11, fontweight='bold')
ax.set_ylabel('Torque (N⋅m)', fontsize=11, fontweight='bold')
ax.set_title('Aileron Torque (Roll Axis)', fontsize=12, fontweight='bold')
ax.grid(True, alpha=0.3)

# Subplot 3: Elevator only
ax = axes[1, 0]
ax.plot(deflections, results[v]['elevator'], 'r-', linewidth=3)
ax.fill_between(deflections, 0, results[v]['elevator'], alpha=0.2, color='red')
ax.axhline(0, color='k', linestyle='-', linewidth=0.8, alpha=0.3)
ax.axvline(0, color='k', linestyle='-', linewidth=0.8, alpha=0.3)
ax.axvline(-20, color='gray', linestyle='--', alpha=0.4)
ax.axvline(20, color='gray', linestyle='--', alpha=0.4)
ax.scatter([20, -20], [results[v]['elevator'][idx_20], results[v]['elevator'][idx_m20]], 
           color='r', s=100, zorder=5, edgecolors='black', linewidth=1.5)
ax.text(20, results[v]['elevator'][idx_20], f'  {results[v]["elevator"][idx_20]:.4f} N⋅m', 
        va='bottom', ha='left', fontsize=9, fontweight='bold')
ax.text(-20, results[v]['elevator'][idx_m20], f'{results[v]["elevator"][idx_m20]:.4f} N⋅m  ', 
        va='top', ha='right', fontsize=9, fontweight='bold')
ax.set_xlabel('Deflection Angle (deg)', fontsize=11, fontweight='bold')
ax.set_ylabel('Torque (N⋅m)', fontsize=11, fontweight='bold')
ax.set_title('Elevator Torque (Pitch Axis)', fontsize=12, fontweight='bold')
ax.grid(True, alpha=0.3)

# Subplot 4: Summary table
ax = axes[1, 1]
ax.axis('off')

summary_text = f"""
TORQUE SUMMARY AT MAXIMUM DEFLECTION

At +20° deflection:
┌─────────────┬────────────────┬────────────────┐
│ Surface     │ Torque (N⋅m)   │ Lever Arm (cm) │
├─────────────┼────────────────┼────────────────┤
│ Aileron     │ {results[v]['aileron'][idx_20]:>14.4f} │ {aileron['lever']*100:>14.1f} │
│ Elevator    │ {results[v]['elevator'][idx_20]:>14.4f} │ {elevator['lever']*100:>14.1f} │
│ Rudder      │ {results[v]['rudder'][idx_20]:>14.4f} │ {rudder['lever']*100:>14.1f} │
└─────────────┴────────────────┴────────────────┘

At -20° deflection:
┌─────────────┬────────────────┬────────────────┐
│ Surface     │ Torque (N⋅m)   │ Lever Arm (cm) │
├─────────────┼────────────────┼────────────────┤
│ Aileron     │ {results[v]['aileron'][idx_m20]:>14.4f} │ {aileron['lever']*100:>14.1f} │
│ Elevator    │ {results[v]['elevator'][idx_m20]:>14.4f} │ {elevator['lever']*100:>14.1f} │
│ Rudder      │ {results[v]['rudder'][idx_m20]:>14.4f} │ {rudder['lever']*100:>14.1f} │
└─────────────┴────────────────┴────────────────┘

Surface Areas:
  • Aileron:  {aileron['area']*10000:.1f} cm² (×2)
  • Elevator: {elevator['area']*10000:.1f} cm²
  • Rudder:   {rudder['area']*10000:.1f} cm²

Dynamic Pressure: {0.5*rho*v**2:.2f} Pa
"""

ax.text(0.05, 0.95, summary_text, transform=ax.transAxes, 
        fontsize=9.5, verticalalignment='top', fontfamily='monospace',
        bbox=dict(boxstyle='round', facecolor='lightblue', alpha=0.3, pad=1))

plt.tight_layout()
plt.savefig(f'{output_dir}/torque_7ms_detailed.png', dpi=300, bbox_inches='tight')
plt.close()

print(f"✓ Saved: torque_7ms_detailed.png")

# ====================================================================================
# PLOT 2-5: Individual plots for each velocity
# ====================================================================================

for v in velocities:
    fig, ax = plt.subplots(figsize=(12, 8))
    
    ax.plot(deflections, results[v]['aileron'], 'b-', linewidth=3, label='Aileron (Roll)', marker='o', markevery=10)
    ax.plot(deflections, results[v]['elevator'], 'r-', linewidth=3, label='Elevator (Pitch)', marker='s', markevery=10)
    ax.plot(deflections, results[v]['rudder'], 'g-', linewidth=3, label='Rudder (Yaw)', marker='^', markevery=10)
    
    ax.axhline(0, color='k', linestyle='-', linewidth=1, alpha=0.4)
    ax.axvline(0, color='k', linestyle='-', linewidth=1, alpha=0.4)
    ax.axvline(-20, color='gray', linestyle='--', linewidth=1.5, alpha=0.5, label='Max Deflection')
    ax.axvline(20, color='gray', linestyle='--', linewidth=1.5, alpha=0.5)
    
    # Mark max deflection points
    ax.scatter([20], [results[v]['aileron'][idx_20]], color='b', s=120, zorder=5, edgecolors='black', linewidth=2)
    ax.scatter([20], [results[v]['elevator'][idx_20]], color='r', s=120, zorder=5, edgecolors='black', linewidth=2)
    ax.scatter([20], [results[v]['rudder'][idx_20]], color='g', s=120, zorder=5, edgecolors='black', linewidth=2)
    ax.scatter([-20], [results[v]['aileron'][idx_m20]], color='b', s=120, zorder=5, edgecolors='black', linewidth=2)
    ax.scatter([-20], [results[v]['elevator'][idx_m20]], color='r', s=120, zorder=5, edgecolors='black', linewidth=2)
    ax.scatter([-20], [results[v]['rudder'][idx_m20]], color='g', s=120, zorder=5, edgecolors='black', linewidth=2)
    
    ax.set_xlabel('Deflection Angle (deg)', fontsize=13, fontweight='bold')
    ax.set_ylabel('Torque (N⋅m)', fontsize=13, fontweight='bold')
    ax.set_title(f'Phoenix V2 Control Surface Torques at {v} m/s\n' + 
                 f'(Alt: {altitude}m, Temp: {T_celsius}°C, ρ = {rho:.4f} kg/m³, q = {0.5*rho*v**2:.2f} Pa)',
                 fontsize=14, fontweight='bold')
    ax.grid(True, alpha=0.3)
    ax.legend(loc='upper left', fontsize=12)
    
    plt.tight_layout()
    plt.savefig(f'{output_dir}/torque_{v}ms.png', dpi=300, bbox_inches='tight')
    plt.close()
    
    print(f"✓ Saved: torque_{v}ms.png")

# ====================================================================================
# PLOT 6: Comparison across all velocities
# ====================================================================================

fig, axes = plt.subplots(1, 3, figsize=(18, 6))
fig.suptitle(f'Phoenix V2 Control Torques - Velocity Comparison\n' + 
             f'Altitude: {altitude}m, Temperature: {T_celsius}°C, ρ = {rho:.4f} kg/m³',
             fontsize=14, fontweight='bold')

colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728']
linestyles = ['-', '--', '-.', ':']

# Aileron
ax = axes[0]
for i, v in enumerate(velocities):
    ax.plot(deflections, results[v]['aileron'], linewidth=2.5, 
            color=colors[i], linestyle=linestyles[i], label=f'{v} m/s')
ax.axhline(0, color='k', linestyle='-', linewidth=0.8, alpha=0.3)
ax.axvline(0, color='k', linestyle='-', linewidth=0.8, alpha=0.3)
ax.set_xlabel('Deflection Angle (deg)', fontsize=12, fontweight='bold')
ax.set_ylabel('Torque (N⋅m)', fontsize=12, fontweight='bold')
ax.set_title('Aileron (Roll Axis)', fontsize=13, fontweight='bold')
ax.grid(True, alpha=0.3)
ax.legend(fontsize=11)

# Elevator
ax = axes[1]
for i, v in enumerate(velocities):
    ax.plot(deflections, results[v]['elevator'], linewidth=2.5, 
            color=colors[i], linestyle=linestyles[i], label=f'{v} m/s')
ax.axhline(0, color='k', linestyle='-', linewidth=0.8, alpha=0.3)
ax.axvline(0, color='k', linestyle='-', linewidth=0.8, alpha=0.3)
ax.set_xlabel('Deflection Angle (deg)', fontsize=12, fontweight='bold')
ax.set_ylabel('Torque (N⋅m)', fontsize=12, fontweight='bold')
ax.set_title('Elevator (Pitch Axis)', fontsize=13, fontweight='bold')
ax.grid(True, alpha=0.3)
ax.legend(fontsize=11)

# Rudder
ax = axes[2]
for i, v in enumerate(velocities):
    ax.plot(deflections, results[v]['rudder'], linewidth=2.5, 
            color=colors[i], linestyle=linestyles[i], label=f'{v} m/s')
ax.axhline(0, color='k', linestyle='-', linewidth=0.8, alpha=0.3)
ax.axvline(0, color='k', linestyle='-', linewidth=0.8, alpha=0.3)
ax.set_xlabel('Deflection Angle (deg)', fontsize=12, fontweight='bold')
ax.set_ylabel('Torque (N⋅m)', fontsize=12, fontweight='bold')
ax.set_title('Rudder (Yaw Axis)', fontsize=13, fontweight='bold')
ax.grid(True, alpha=0.3)
ax.legend(fontsize=11)

plt.tight_layout()
plt.savefig(f'{output_dir}/torque_comparison_all.png', dpi=300, bbox_inches='tight')
plt.close()

print(f"✓ Saved: torque_comparison_all.png")

# ====================================================================================
# Export CSV
# ====================================================================================

with open(f'{output_dir}/torque_data.csv', 'w') as f:
    f.write('Velocity (m/s),Deflection (deg),Aileron Torque (Nm),Elevator Torque (Nm),Rudder Torque (Nm)\n')
    for v in velocities:
        for i, delta in enumerate(deflections):
            f.write(f'{v},{delta:.2f},{results[v]["aileron"][i]:.6f},')
            f.write(f'{results[v]["elevator"][i]:.6f},{results[v]["rudder"][i]:.6f}\n')

print(f"✓ Saved: torque_data.csv")

# ====================================================================================
# CLI output summary
# ====================================================================================

print(f"\n{'='*80}")
print(f"TORQUE ANALYSIS SUMMARY - PHOENIX V2 GLIDER")
print(f"{'='*80}\n")

for v in velocities:
    print(f"\nVelocity: {v} m/s | Dynamic Pressure: {0.5*rho*v**2:.2f} Pa")
    print(f"{'-'*80}")
    print(f"{'Surface':<12} {'@-20°':<15} {'@0°':<15} {'@+20°':<15}")
    print(f"{'-'*80}")
    print(f"{'Aileron':<12} {results[v]['aileron'][idx_m20]:<15.4f} {results[v]['aileron'][40]:<15.4f} {results[v]['aileron'][idx_20]:<15.4f}")
    print(f"{'Elevator':<12} {results[v]['elevator'][idx_m20]:<15.4f} {results[v]['elevator'][40]:<15.4f} {results[v]['elevator'][idx_20]:<15.4f}")
    print(f"{'Rudder':<12} {results[v]['rudder'][idx_m20]:<15.4f} {results[v]['rudder'][40]:<15.4f} {results[v]['rudder'][idx_20]:<15.4f}")

print(f"\n{'='*80}")
print(f"All files saved to: {output_dir}/")
print(f"{'='*80}\n")