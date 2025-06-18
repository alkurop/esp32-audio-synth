import numpy as np

# Constants
cutoff_steps = 64
resonance_steps = 64
sample_rate = 48000
safe_fc_max = sample_rate * 0.45

# Inverted log-scaled cutoff (0=open, 1=closed)
cutoff_range = np.logspace(np.log10(20.0), np.log10(safe_fc_max), cutoff_steps)

# Perceptually scaled and softened resonance
def q_curve(x):
    q = 1.0 + 31.0 * (x ** 2)
    if x > 0.7:
        q *= 1 + 5 * ((x - 0.7) ** 2)
    return q * 0.5

resonance_range = [q_curve(x) for x in np.linspace(0.0, 1.0, resonance_steps)]

# Dynamic Q clamping based on cutoff
def dynamic_q(fc, q):
    fc_norm = fc / safe_fc_max
    scale = 0.5 + 0.5 * (1.0 - abs(0.5 - fc_norm) * 2.0)  # Emphasize middle
    return max(0.1, q * scale)

# Biquad LPF
def compute_biquad_lpf_coeffs(fc, q, sr):
    fc = min(fc, safe_fc_max * 0.98)
    q = dynamic_q(fc, q)
    omega = 2.0 * np.pi * fc / sr
    sn = np.sin(omega)
    cs = np.cos(omega)
    alpha = sn / (2.0 * q)
    a0 = 1.0 + alpha
    if a0 == 0.0 or np.isnan(a0): raise ValueError("Invalid a0")
    b0 = (1.0 - cs) * 0.5
    b1 = 1.0 - cs
    b2 = b0
    a1 = -2.0 * cs
    a2 = 1.0 - alpha
    return [b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0]

# Biquad HPF
def compute_biquad_hpf_coeffs(fc, q, sr):
    fc = max(fc, 20.0)
    q = dynamic_q(fc, q)
    omega = 2.0 * np.pi * fc / sr
    sn = np.sin(omega)
    cs = np.cos(omega)
    alpha = sn / (2.0 * q)
    a0 = 1.0 + alpha
    if a0 == 0.0 or np.isnan(a0): raise ValueError("Invalid a0")
    b0 = (1.0 + cs) * 0.5
    b1 = -(1.0 + cs)
    b2 = b0
    a1 = -2.0 * cs
    a2 = 1.0 - alpha
    return [b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0]

# Blend LPF and HPF to simulate BP
def compute_biquad_bp_approx(fc, q, sr):
    lpf = compute_biquad_lpf_coeffs(fc, q, sr)
    hpf = compute_biquad_hpf_coeffs(fc, q, sr)
    return [(l + h) / 2.0 for l, h in zip(lpf, hpf)]

# Generate table
cpp_friendly_table = []
for fc in cutoff_range:
    row = []
    for q in resonance_range:
        try:
            coeffs = compute_biquad_bp_approx(fc, q, sample_rate)
            row.append([round(v, 7) for v in coeffs])
        except:
            row.append([0.0, 0.0, 0.0, 0.0, 0.0])
    cpp_friendly_table.append(row)

# Format C++
formatted_cpp = "{\n" + ",\n".join(
    "  {" + ", ".join(
        "{" + ", ".join(f"{val}f" for val in coeffs) + "}"
        for coeffs in row
    ) + "}"
    for row in cpp_friendly_table
) + "\n};"

print(formatted_cpp)  # Preview first 1000 chars
