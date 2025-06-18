# Notch filter coefficient generator with dynamic Q scaling and inverted cutoff

import numpy as np

# Constants
cutoff_steps = 64
resonance_steps = 64
sample_rate = 48000
safe_fc_max = sample_rate * 0.45

# Inverted cutoff scale: 0 = open (full pass), 1 = closed (deep notch)
cutoff_range = np.logspace(np.log10(20.0), np.log10(safe_fc_max), cutoff_steps)[::-1]

# Soft resonance curve, slightly boosted at high Q values
def q_curve(x):
    q = 1.0 + 31.0 * (x ** 2)
    if x > 0.7:
        q *= 1 + 5 * ((x - 0.7) ** 2)
    return q * 0.5

resonance_range = [q_curve(x) for x in np.linspace(0.0, 1.0, resonance_steps)]

# Q modifier for deepening notch at high cutoff
def dynamic_q(fc, q):
    fc_norm = fc / safe_fc_max
    boost = 1.0 + 2.0 * (fc_norm ** 2)
    return max(0.1, q * boost)

# Compute notch filter biquad coefficients
def compute_biquad_notch_coeffs(fc, q, sr):
    try:
        fc = min(max(fc, 20.0), safe_fc_max * 0.98)
        q = dynamic_q(fc, q)

        omega = 2.0 * np.pi * fc / sr
        sn = np.sin(omega)
        cs = np.cos(omega)
        alpha = sn / (2.0 * q)

        a0 = 1.0 + alpha
        if a0 == 0.0 or np.isnan(a0):
            raise ValueError("Invalid a0")

        b0 = 1.0
        b1 = -2.0 * cs
        b2 = 1.0
        a1 = -2.0 * cs
        a2 = 1.0 - alpha

        b0n = b0 / a0
        b1n = b1 / a0
        b2n = b2 / a0
        a1n = a1 / a0
        a2n = a2 / a0

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

# Generate table
cpp_friendly_table = [
    [compute_biquad_notch_coeffs(fc, q, sample_rate) for q in resonance_range]
    for fc in cutoff_range
]

# Format for C++
formatted_cpp = "{\n" + ",\n".join(
    "  {" + ", ".join(
        "{" + ", ".join(f"{val}f" for val in coeffs) + "}"
        for coeffs in row
    ) + "}"
    for row in cpp_friendly_table
) + "\n};"

# Print result
print(formatted_cpp)
