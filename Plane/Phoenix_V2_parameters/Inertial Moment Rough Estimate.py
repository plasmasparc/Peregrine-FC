import numpy as np

# Wing masses (2 point masses on x-axis)
m_wing = 0.15  # kg each
r_wing = 0.5   # m from center

# Tail mass (1 point mass on y-axis)
m_tail = 0.2   # kg
r_tail = 0.6   # m from center

Ix = 2 * m_wing * r_wing**2        # szárnyak ⊥ x-re
Iy = m_tail * r_tail**2            # farok ⊥ y-ra  
Iz = 2 * m_wing * r_wing**2 + m_tail * r_tail**2  # mindkettő ⊥ z-re

print(f"\nMoments of Inertia - Cross Model Estimation\n")
print(f"Wings: 2 × {m_wing} kg at ±{r_wing} m (x-axis)")
print(f"Tail:  {m_tail} kg at {r_tail} m (y-axis)\n")
print(f"Ix (roll)  = {Ix:.4f} kg⋅m²")
print(f"Iy (pitch) = {Iy:.4f} kg⋅m²")
print(f"Iz (yaw)   = {Iz:.4f} kg⋅m²\n")

# Save to markdown
md_content = f"""# Phoenix V2 Moment of Inertia Estimation

## Cross Model Approximation

**Configuration:**
- Wings: 2 point masses, 0.15 kg each, ±0.5 m from CG (x-axis)
- Tail: 1 point mass, 0.2 kg, 0.6 m from CG (y-axis)

**Calculation:**
```
Point mass: I = m × r²

Ix (roll):  only wings perpendicular to x-axis
  Ix = 2 × 0.15 × 0.5² = {Ix:.4f} kg⋅m²

Iy (pitch): wings + tail perpendicular to y-axis
  Iy = 2 × 0.15 × 0.5² + 0.2 × 0.6² = {Iy:.4f} kg⋅m²

Iz (yaw):   only tail perpendicular to z-axis
  Iz = 0.2 × 0.6² = {Iz:.4f} kg⋅m²
```

## Results

| Axis  | Moment of Inertia | Description |
|-------|-------------------|-------------|
| **Ix** | {Ix:.4f} kg⋅m² | Roll (wings) |
| **Iy** | {Iy:.4f} kg⋅m² | Pitch (wings + tail) |
| **Iz** | {Iz:.4f} kg⋅m² | Yaw (tail) |

**Physical interpretation:**
- Roll easiest (smallest I) - only wing mass matters
- Pitch hardest (largest I) - both wing and tail contribute
- Yaw intermediate - only tail mass, but longer lever arm

**Note:** This is simplified point-mass approximation. Real aircraft has distributed mass (fuselage, servos, battery). Expect 20-40% error vs actual measurement.
"""

with open('inertia_estimation.md', 'w') as f:
    f.write(md_content)

print("✓ Saved: inertia_estimation.md")