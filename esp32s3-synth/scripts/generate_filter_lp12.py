# Full debugged Python script with logging to inspect coefficient variety

import numpy as np

# Constants
cutoff_steps = 64
resonance_steps = 64
sample_rate = 48000
safe_fc_max = sample_rate * 0.45

# Inverted log-scaled cutoff (0=open, 1=closed)
cutoff_range = np.logspace(np.log10(20.0), np.log10(safe_fc_max), cutoff_steps)[::-1]

# Resonance perceptual curve
def q_curve(x):
    return 1.0 + 7.0 * (x ** 2)

resonance_range = [q_curve(x) for x in np.linspace(0.0, 1.0, resonance_steps)]

# Smarter Q clamping to avoid flat response
def dynamic_q(fc, q):
    fc_norm = fc / safe_fc_max
    scale = 0.25 + 0.75 * fc_norm
    return max(0.1, q * scale)

# Biquad LPF computation without gain scaling for debugging
def compute_biquad_lpf_coeffs(fc, q, sr):
    try:
        if fc >= safe_fc_max * 0.99:
            return [1.0, 0.0, 0.0, 0.0, 0.0]  # Bypass

        q = dynamic_q(fc, q)

        omega = 2.0 * np.pi * fc / sr
        sn = np.sin(omega)
        cs = np.cos(omega)
        alpha = sn / (2.0 * q)

        a0 = 1.0 + alpha
        if a0 == 0.0 or np.isnan(a0):
            raise ValueError("Invalid a0")

        b0 = (1.0 - cs) * 0.5
        b1 = 1.0 - cs
        b2 = b0
        a1 = -2.0 * cs
        a2 = 1.0 - alpha

        b0n = b0 / a0
        b1n = b1 / a0
        b2n = b2 / a0
        a1n = a1 / a0
        a2n = a2 / a0

        # Print debug output for a few representative values
        if np.isclose(fc, 100.0, atol=10.0) and np.isclose(q, 1.0, atol=0.1):
            print(f"[DEBUG] fc={fc:.2f}, q={q:.2f} → b0={b0n:.5f}, b1={b1n:.5f}, b2={b2n:.5f}, a1={a1n:.5f}, a2={a2n:.5f}")

        return [
            round(b0n, 7),
            round(b1n, 7),
            round(b2n, 7),
            round(a1n, 7),
            round(a2n, 7)
        ]
    except Exception as e:
        print(f"ERROR: fc={fc}, q={q} → {e}")
        return [0.0, 0.0, 0.0, 0.0, 0.0]

# Generate the filter table
cpp_friendly_table = [
    [compute_biquad_lpf_coeffs(fc, q, sample_rate) for q in resonance_range]
    for fc in cutoff_range
]

# Format as C++ array
formatted_cpp = "{\n" + ",\n".join(
    "  {" + ", ".join(
        "{" + ", ".join(f"{val}f" for val in coeffs) + "}"
        for coeffs in row
    ) + "}"
    for row in cpp_friendly_table
) + "\n};"

# Print full output for use in C++
print(formatted_cpp)
