# Phoenix V2 Moment of Inertia Estimation

## Cross Model Approximation

**Configuration:**
- Wings: 2 point masses, 0.15 kg each, ±0.5 m from CG (x-axis)
- Tail: 1 point mass, 0.2 kg, 0.6 m from CG (y-axis)

**Calculation:**
```
Point mass: I = m × r²

Ix (roll):  only wings perpendicular to x-axis
  Ix = 2 × 0.15 × 0.5² = 0.0750 kg⋅m²

Iy (pitch): wings + tail perpendicular to y-axis
  Iy = 2 × 0.15 × 0.5² + 0.2 × 0.6² = 0.0720 kg⋅m²

Iz (yaw):   only tail perpendicular to z-axis
  Iz = 0.2 × 0.6² = 0.1470 kg⋅m²
```

## Results

| Axis  | Moment of Inertia | Description |
|-------|-------------------|-------------|
| **Ix** | 0.0750 kg⋅m² | Roll (wings) |
| **Iy** | 0.0720 kg⋅m² | Pitch (wings + tail) |
| **Iz** | 0.1470 kg⋅m² | Yaw (tail) |

**Physical interpretation:**
- Roll easiest (smallest I) - only wing mass matters
- Pitch hardest (largest I) - both wing and tail contribute
- Yaw intermediate - only tail mass, but longer lever arm

**Note:** This is simplified point-mass approximation. Real aircraft has distributed mass (fuselage, servos, battery). Expect 20-40% error vs actual measurement.
