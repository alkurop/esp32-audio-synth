import numpy as np

# Table dimensions
cutoff_steps = 128
resonance_steps = 64
sample_rate = 48000

# Cutoff range: 20Hz to Nyquist (~24kHz), resonance Q: 0.1 to 10.0
cutoff_range = np.linspace(20.0, sample_rate / 2, cutoff_steps)
resonance_range = np.linspace(0.1, 10.0, resonance_steps)

# Precompute coefficients for biquad LPF
def compute_biquad_lpf_coeffs(fc, q, sr):
    omega = 2.0 * np.pi * fc / sr
    sn = np.sin(omega)
    cs = np.cos(omega)
    alpha = sn / (2.0 * q)

    b0 = (1.0 - cs) * 0.5
    b1 = 1.0 - cs
    b2 = b0
    a0 = 1.0 + alpha
    a1 = -2.0 * cs
    a2 = 1.0 - alpha

    # Normalize
    return [round(b0/a0, 7), round(b1/a0, 7), round(b2/a0, 7), round(a1/a0, 7), round(a2/a0, 7)]

# Generate table
cpp_friendly_table = [
    [
        compute_biquad_lpf_coeffs(fc, q, sample_rate)
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

# Show just the beginning for preview
print(formatted_cpp)
