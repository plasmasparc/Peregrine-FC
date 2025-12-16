#!/usr/bin/env python3
# WGS-84 local mapping: (v, heading) -> (dlon/dt, dlat/dt)
# Inputs: lon_deg, lat_deg in degrees; heading_deg in [0, 360); speed_mps in m/s
# Output: (dlon_dt_deg_s, dlat_dt_deg_s) in degrees/second

from __future__ import annotations
import math
from dataclasses import dataclass
from typing import Tuple

# --- WGS-84 constants ---
_A = 6378137.0  # semi-major axis [m]
_F = 1.0 / 298.257223563
_E2 = _F * (2.0 - _F)

# Default: Romania (approx. Cluj-Napoca)
DEFAULT_LON_DEG = 23.5940
DEFAULT_LAT_DEG = 46.7712


def lonlat_derivatives_deg_s(
    lon_deg: float = DEFAULT_LON_DEG,
    lat_deg: float = DEFAULT_LAT_DEG,
    speed_mps: float = 10.0,
    heading_deg: float = 0.0,
) -> Tuple[float, float]:
    """
    Compute (dlon/dt, dlat/dt) from local horizontal speed and heading.

    Conventions:
      - heading_deg is COG/heading (true), measured clockwise from North:
          0° = North, 90° = East, 180° = South, 270° = West
      - speed_mps is horizontal ground speed in m/s
      - lon_deg, lat_deg are geodetic coordinates in degrees (WGS-84)

    Returns:
      (dlon_dt_deg_s, dlat_dt_deg_s) in degrees/second
    """
    if speed_mps < 0:
        raise ValueError("speed_mps must be non-negative")
    # Normalize heading to [0, 360)
    heading_deg = heading_deg % 360.0

    phi = math.radians(lat_deg)      # latitude [rad]
    psi = math.radians(heading_deg)  # heading [rad]

    sin_phi = math.sin(phi)
    cos_phi = math.cos(phi)

    # Radii of curvature (WGS-84 ellipsoid)
    denom = 1.0 - _E2 * sin_phi * sin_phi
    sqrt_denom = math.sqrt(denom)

    N = _A / sqrt_denom
    M = _A * (1.0 - _E2) / (denom * sqrt_denom)  # = a(1-e^2)/(1-e^2 sin^2)^(3/2)

    # Local N/E components (ENU)
    v_N = speed_mps * math.cos(psi)
    v_E = speed_mps * math.sin(psi)

    # Angular rates [rad/s]
    dphi_dt = v_N / M
    if abs(cos_phi) < 1e-15:
        # near poles longitude rate is ill-defined
        raise ValueError("Latitude too close to ±90°, dlon/dt becomes ill-defined.")
    dlambda_dt = v_E / (N * cos_phi)

    # Convert to deg/s
    rad2deg = 180.0 / math.pi
    dlat_dt_deg_s = dphi_dt * rad2deg
    dlon_dt_deg_s = dlambda_dt * rad2deg

    return dlon_dt_deg_s, dlat_dt_deg_s


if __name__ == "__main__":
    # Example: 10 m/s North and 10 m/s West at default Romania coords
    for heading in (0.0, 270.0, 90.0, 180.0, 45.0):
        dlon, dlat = lonlat_derivatives_deg_s(speed_mps=10.0, heading_deg=heading)
        print(f"heading={heading:6.1f} deg  ->  dlon/dt={dlon:+.9e} deg/s,  dlat/dt={dlat:+.9e} deg/s")
