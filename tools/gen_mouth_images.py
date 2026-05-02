#!/usr/bin/env python3
"""Generate mouth shape PNG images for different emotions.

Each image is RGBA with transparent background and white mouth line.
Size: 96x40 pixels — larger to prevent arc clipping, thicker lines for visibility.
"""
import os
from PIL import Image, ImageDraw

OUT_DIR = "mouth_pngs"
W, H = 96, 40  # width, height
CX, CY = W // 2, H // 2
COLOR = (255, 255, 255, 255)  # white, fully opaque
LINE_W = 5  # thicker lines for visibility on small display

os.makedirs(OUT_DIR, exist_ok=True)


def new_img():
    return Image.new("RGBA", (W, H), (0, 0, 0, 0))


def save(img, name):
    img.save(os.path.join(OUT_DIR, f"mouth_{name}.png"))


# === Happy: upward arc (U shape / smile) ===
img = new_img()
draw = ImageDraw.Draw(img)
# Arc bounding box stays well within 0..H-1
draw.arc([12, CY - 18, W - 12, CY + 18], start=0, end=180, fill=COLOR, width=LINE_W)
save(img, "happy")

# === Sad: downward arc (inverted U) ===
img = new_img()
draw = ImageDraw.Draw(img)
draw.arc([12, CY - 18, W - 12, CY + 18], start=180, end=0, fill=COLOR, width=LINE_W)
save(img, "sad")

# === Angry: downward V shape ===
img = new_img()
draw = ImageDraw.Draw(img)
pts = [(20, CY - 8), (CX, CY + 10), (W - 20, CY - 8)]
draw.line(pts, fill=COLOR, width=LINE_W)
save(img, "angry")

# === Shocked: round O shape ===
img = new_img()
draw = ImageDraw.Draw(img)
draw.ellipse([CX - 12, CY - 12, CX + 12, CY + 12], outline=COLOR, width=LINE_W)
save(img, "shocked")

# === Neutral: straight horizontal line ===
img = new_img()
draw = ImageDraw.Draw(img)
draw.line([(16, CY), (W - 16, CY)], fill=COLOR, width=LINE_W)
save(img, "neutral")

# === Confused: wavy line ===
img = new_img()
draw = ImageDraw.Draw(img)
pts = [(10, CY - 6), (26, CY + 7), (44, CY - 7), (60, CY + 7), (W - 10, CY - 6)]
draw.line(pts, fill=COLOR, width=LINE_W, joint="curve")
save(img, "confused")

# === Winking: asymmetric smile (one side higher) ===
img = new_img()
draw = ImageDraw.Draw(img)
pts = [(12, CY + 3), (CX - 6, CY - 8), (CX + 6, CY + 10), (W - 12, CY + 4)]
draw.line(pts, fill=COLOR, width=LINE_W, joint="curve")
save(img, "winking")

# === Sleep: small open mouth ===
img = new_img()
draw = ImageDraw.Draw(img)
draw.ellipse([CX - 7, CY - 5, CX + 7, CY + 7], outline=COLOR, width=LINE_W - 1)
save(img, "sleep")

# === Cry: wavy downward ===
img = new_img()
draw = ImageDraw.Draw(img)
pts = [(10, CY - 8), (26, CY + 5), (CX, CY - 6), (66, CY + 5), (W - 10, CY - 8)]
draw.line(pts, fill=COLOR, width=LINE_W, joint="curve")
save(img, "cry")

# === Aloof: slight smirk, flat mostly ===
img = new_img()
draw = ImageDraw.Draw(img)
pts = [(16, CY - 3), (CX - 6, CY - 3), (CX + 6, CY - 8), (W - 16, CY - 8)]
draw.line(pts, fill=COLOR, width=LINE_W, joint="curve")
save(img, "aloof")

# === Cute: small ^ shape, cat-like ===
img = new_img()
draw = ImageDraw.Draw(img)
pts = [(16, CY + 6), (CX - 10, CY - 8), (CX, CY - 3), (CX + 10, CY - 8), (W - 16, CY + 6)]
draw.line(pts, fill=COLOR, width=LINE_W - 1, joint="curve")
save(img, "cute")

# === Talking open: wide open mouth for speech animation ===
img = new_img()
draw = ImageDraw.Draw(img)
draw.ellipse([CX - 10, CY - 10, CX + 10, CY + 10], outline=COLOR, width=LINE_W)
save(img, "talking")

print(f"Generated {len(os.listdir(OUT_DIR))} mouth PNG images in {OUT_DIR}/")
for f in sorted(os.listdir(OUT_DIR)):
    print(f"  {f}")
