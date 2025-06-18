import numpy as np

# Table dimensions
cutoff_steps = 128
resonance_steps = 64
sample_rate = 48000

# Safer cutoff and Q ranges
safe_fc_max = sample_rate * 0.45  # avoid Nyquist edge
safe_q_max = 8.0                  # avoid instability at very high Q

# Generate ranges
cutoff_range = np.linspace(20.0, safe_fc_max, cutoff_steps)
resonance_range = np.linspace(0.1, safe_q_max, resonance_steps)

# Precompute LP12 biquad coefficients
def compute_biquad_lpf_coeffs(fc, q, sr):
    try:
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

        return [round(b0/a0, 7), round(b1/a0, 7), round(b2/a0, 7), round(a1/a0, 7), round(a2/a0, 7)]
    except Exception as e:
        print(f"Warning: fc={fc}, q={q}, err={e}")
        return [0.0, 0.0, 0.0, 0.0, 0.0]

# Generate lookup table
cpp_friendly_table = [
    [
        compute_biquad_lpf_coeffs(fc, q, sample_rate)
        for q in resonance_range
    ]
    for fc in cutoff_range
]

# Format as C++ initializer list
formatted_cpp = "{\n" + ",\n".join(
    "  {" + ", ".join(
        "{" + ", ".join(f"{val}f" for val in coeffs) + "}"
        for coeffs in row
    ) + "}"
    for row in cpp_friendly_table
) + "\n};"

# Print output
print(formatted_cpp)
