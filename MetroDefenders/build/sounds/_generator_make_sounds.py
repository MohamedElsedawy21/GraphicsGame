"""Generates every sound used by the game (original, synthesized - no copyright issues)."""
import numpy as np, wave, sys, os

SR = 22050
OUT = sys.argv[1]
os.makedirs(OUT, exist_ok=True)
rng = np.random.default_rng(7)


def save(name, x, gain=0.9):
    x = np.asarray(x, dtype=np.float64)
    m = np.max(np.abs(x)) or 1.0
    x = x / m * gain
    data = (x * 32767).astype(np.int16)
    with wave.open(os.path.join(OUT, name + ".wav"), "wb") as w:
        w.setnchannels(1); w.setsampwidth(2); w.setframerate(SR)
        w.writeframes(data.tobytes())


def t(d):
    return np.arange(int(SR * d)) / SR


def env(n, a=0.005, r=None):
    e = np.ones(n)
    na = max(1, int(a * SR)); e[:na] = np.linspace(0, 1, na)
    if r is not None:
        e *= np.exp(-np.arange(n) / (r * SR))
    return e


def tone(f, d, kind="sine", r=0.2):
    tt = t(d)
    ph = 2 * np.pi * np.cumsum(np.full(len(tt), f) if np.isscalar(f) else f) / SR
    if kind == "sine": w = np.sin(ph)
    elif kind == "square": w = np.sign(np.sin(ph)) * 0.6
    elif kind == "saw": w = 2 * ((ph / (2 * np.pi)) % 1) - 1
    return w * env(len(tt), r=r)


def noise(d, r=0.05):
    return rng.uniform(-1, 1, int(SR * d)) * env(int(SR * d), 0.001, r)


def lowpass(x, k=0.2):
    y = np.zeros_like(x); acc = 0.0
    for i, v in enumerate(x):
        acc += k * (v - acc); y[i] = acc
    return y


def pluck(f, d, bright=0.5):
    """Karplus-Strong plucked string (oud-like)."""
    n = int(SR * d); p = max(2, int(SR / f))
    buf = rng.uniform(-1, 1, p) * bright + rng.uniform(-1, 1, p) * (1 - bright) * 0.3
    out = np.zeros(n)
    for i in range(n):
        out[i] = buf[i % p]
        buf[i % p] = 0.5 * (buf[i % p] + buf[(i + 1) % p]) * 0.996
    return out


def sweep(f0, f1, d, kind="sine", r=0.15):
    f = np.linspace(f0, f1, int(SR * d))
    return tone(f, d, kind, r)


def mix(*parts):
    n = max(len(p) for p in parts)
    out = np.zeros(n)
    for p in parts: out[:len(p)] += p
    return out


def place_at(buf, x, start):
    s = int(start * SR); e = min(len(buf), s + len(x))
    buf[s:e] += x[:e - s]


# ------------------------------------------------------------------ SFX
save("select", tone(1200, 0.06, "square", 0.02), 0.5)
save("tick", tone(2200, 0.03, "sine", 0.01), 0.35)
save("place", mix(sweep(300, 900, 0.12, "sine", 0.08), 0.4 * lowpass(noise(0.05, 0.02), 0.4)), 0.7)
save("hit", mix(sweep(180, 60, 0.18, "sine", 0.08), 0.6 * lowpass(noise(0.12, 0.03), 0.3)), 0.8)
save("whack", mix(tone(320, 0.12, "sine", 0.03), 0.8 * lowpass(noise(0.08, 0.015), 0.5), tone(640, 0.08, "sine", 0.02)), 0.7)
save("shoot", mix(sweep(1800, 300, 0.12, "sine", 0.06), 0.3 * noise(0.08, 0.02)), 0.5)
save("clank", mix(tone(1350, 0.3, "sine", 0.07), 0.7 * tone(2210, 0.3, "sine", 0.05), 0.5 * tone(3100, 0.2, "sine", 0.03)), 0.55)
save("crash", mix(lowpass(noise(0.6, 0.18), 0.35), sweep(120, 40, 0.4, "sine", 0.15), 0.4 * tone(1700, 0.25, "sine", 0.05)), 0.85)
save("death", mix(sweep(520, 140, 0.45, "square", 0.25)), 0.45)
coin = np.zeros(int(SR * 0.4)); place_at(coin, tone(988, 0.12, "sine", 0.06), 0); place_at(coin, tone(1319, 0.3, "sine", 0.12), 0.07)
save("coin", coin, 0.6)
honk = mix(tone(415, 0.38, "square", 1.0), tone(523, 0.38, "square", 1.0)) * env(int(SR * 0.38), 0.01)
honk[-int(SR * 0.05):] *= np.linspace(1, 0, int(SR * 0.05))
save("honk", lowpass(honk, 0.35), 0.55)
save("jump", sweep(300, 900, 0.16, "square", 0.1), 0.35)
save("duck", sweep(700, 220, 0.14, "square", 0.08), 0.35)
wt = t(0.6); wf = 2900 + 250 * np.sign(np.sin(2 * np.pi * 28 * wt))
save("whistle", tone(wf, 0.6, "sine", 1.0) * env(len(wt), 0.02) * np.linspace(1, 0.6, len(wt)), 0.45)

# win / lose jingles (maqam hijaz flavour)
D = 293.66
hij = [1, 16 / 15, 5 / 4, 4 / 3, 3 / 2, 8 / 5, 16 / 9, 2]
win = np.zeros(int(SR * 2.2))
for i, k in enumerate([0, 2, 4, 7, 4, 7]):
    place_at(win, pluck(D * hij[k], 0.8), i * 0.16)
place_at(win, mix(pluck(D * 2, 1.2), pluck(D * 1.5, 1.2) * 0.6), 1.0)
save("win", win, 0.8)
lose = np.zeros(int(SR * 2.4))
for i, k in enumerate([7, 5, 4, 2, 1, 0]):
    place_at(lose, pluck(D * hij[k], 0.9), i * 0.25)
place_at(lose, sweep(200, 70, 0.9, "saw", 0.5) * 0.3, 1.4)
save("lose", lose, 0.8)

# ------------------------------------------------------------------ MUSIC
# 16-second loop: oud melody in maqam hijaz over a darbuka "maqsum" rhythm + drone.
bpm = 120; beat = 60 / bpm; bars = 8
L = bars * 4 * beat
music = np.zeros(int(SR * L) + SR)


def doum():
    return mix(sweep(110, 55, 0.35, "sine", 0.12), 0.2 * lowpass(noise(0.05, 0.01), 0.2))


def tek():
    return mix(0.5 * noise(0.06, 0.012), 0.3 * tone(900, 0.04, "sine", 0.01))


# maqsum: D T - T D - T -   (8 eighth notes per bar)
pattern = ["D", "T", None, "T", "D", None, "T", None]
for b in range(bars):
    for i, h in enumerate(pattern):
        st = b * 4 * beat + i * beat / 2
        if h == "D": place_at(music, doum() * 0.9, st)
        elif h == "T": place_at(music, tek() * 0.6, st)
    place_at(music, tek() * 0.25, b * 4 * beat + 3.75 * beat)

# drone on D and A (soft)
dt = t(L)
drone = (np.sin(2 * np.pi * D / 2 * dt) + 0.5 * np.sin(2 * np.pi * D * 0.75 * dt)) * 0.12
drone *= 0.8 + 0.2 * np.sin(2 * np.pi * 0.25 * dt)
music[:len(drone)] += drone

# melody (scale degrees, durations in beats)
mel = [(0, 1), (1, .5), (2, .5), (3, 1), (4, 1), (3, .5), (2, .5), (1, .5), (2, .5), (0, 2),
       (4, 1), (5, .5), (4, .5), (3, 1), (2, 1), (3, .5), (4, .5), (5, .5), (6, .5), (7, 2),
       (7, 1), (6, .5), (5, .5), (4, 1), (5, .5), (4, .5), (3, 1), (2, 1), (1, .5), (2, .5),
       (3, 1), (2, .5), (1, .5), (0, 3)]
pos = 0.0
for deg, dur in mel:
    f = D * hij[deg]
    place_at(music, pluck(f, dur * beat + 0.4, 0.6) * 0.55, pos * beat)
    if dur >= 1:  # oud tremolo flavour on long notes
        place_at(music, pluck(f, 0.3, 0.4) * 0.25, (pos + dur / 2) * beat)
    pos += dur

music = music[:int(SR * L)]
fade = int(0.02 * SR); music[:fade] *= np.linspace(0, 1, fade); music[-fade:] *= np.linspace(1, 0, fade)
save("music", music, 0.7)
print("sounds written to", OUT)
