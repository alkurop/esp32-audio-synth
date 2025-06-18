import numpy as np

# Table dimensions
cutoff_steps = 64
resonance_steps = 64
sample_rate = 48000

# Safe ranges
cutoff_min = 20.0
cutoff_max = sample_rate * 0.45  # avoid Nyquist
q_min = 0.1
q_max = 8.0

# Logarithmic spacing for perceptual uniformity
cutoff_range = np.geomspace(cutoff_min, cutoff_max, cutoff_steps)
resonance_range = np.geomspace(q_min, q_max, resonance_steps)

# Precompute coefficients for biquad HP12 filter
def compute_biquad_hp12_coeffs(fc, q, sr):
    try:
        omega = 2.0 * np.pi * fc / sr
        sn = np.sin(omega)
        cs = np.cos(omega)
        alpha = sn / (2.0 * q)

        a0 = 1.0 + alpha
        if a0 == 0.0 or np.isnan(a0):
            raise ValueError("Invalid a0")

        b0 = (1.0 + cs) * 0.5
        b1 = -(1.0 + cs)
        b2 = b0
        a1 = -2.0 * cs
        a2 = 1.0 - alpha

        return [round(b0/a0, 7), round(b1/a0, 7), round(b2/a0, 7), round(a1/a0, 7), round(a2/a0, 7)]
    except Exception as e:
        print(f"Warning: fc={fc}, q={q}, err={e}")
        return [0.0, 0.0, 0.0, 0.0, 0.0]  # fallback safe coefficients

# Generate table
cpp_friendly_table = [
    [
        compute_biquad_hp12_coeffs(fc, q, sample_rate)
        for q in resonance_range
    ]
    for fc in cutoff_range
]

# Format as C++ initializer list string
formatted_cpp = "{\n" + ",\n".join(
    "  {" + ", ".join(
        "{" + ", ".join(f"{val}f" for val in coeffs) + "}"
        for coeffs in row
    ) + "}"
    for row in cpp_friendly_table
) + "\n};"

# Output
print(formatted_cpp)
