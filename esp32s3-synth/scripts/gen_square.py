import numpy as np

TABLE_SIZE = 1024
MAX_HARMONICS = 31  # Odd harmonics only: 1, 3, 5, ..., (up to Nyquist)

t = np.linspace(0, 1, TABLE_SIZE, endpoint=False)
table = np.zeros_like(t)

for k in range(1, MAX_HARMONICS + 1, 2):  # Only odd harmonics
    table += (1.0 / k) * np.sin(2 * np.pi * k * t)

# Normalize to -1..1
table *= 4.0 / np.pi
table /= np.max(np.abs(table))

# Output as C++ initializer
print("{")
for i, val in enumerate(table):
    print(f"  {val:.6f},", end="\n" if (i + 1) % 8 == 0 else " ")
print("}")
