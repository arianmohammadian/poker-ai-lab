#!/usr/bin/env python3
"""Deterministic, original (non-downloaded) Poker AI Arena artwork.
Requires Pillow and NumPy only when REGENERATING the supplied PNG files.
Font files are never copied or redistributed.
"""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont, ImageFilter
import numpy as np
import math

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'assets'/'arena'
OUT.mkdir(parents=True,exist_ok=True)
RNG=np.random.default_rng(20261106)
FONT_REG='/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf'
FONT_SER='/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf'
FONT_BOLD='/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf'

def ft(size,bold=False,serif=False):
    return ImageFont.truetype(FONT_SER if serif else (FONT_BOLD if bold else FONT_REG),size)

def centered(d,xy,text,font,fill,stroke_width=0,stroke_fill=None):
    bb=d.textbbox((0,0),text,font=font,stroke_width=stroke_width)
    d.text((int(xy[0]-(bb[2]-bb[0])/2),int(xy[1]-(bb[3]-bb[1])/2)-bb[1]),text,font=font,fill=fill,
           stroke_width=stroke_width,stroke_fill=stroke_fill)

def ellipse_mask(w,h,bounds,rad):
    m=Image.new('L',(w,h)); ImageDraw.Draw(m).rounded_rectangle(bounds,radius=rad,fill=255); return m

def use_mask(img,m,color):
    layer=Image.new('RGBA',img.size,color); layer.putalpha(m); img.alpha_composite(layer)

# Atmospheric backdrop, neutral and restrained, no expensive shaders at runtime.
w,h=1600,960
y,x=np.mgrid[0:h,0:w].astype('float32')
center=((x-700)/1280)**2+((y-460)/900)**2
halo=np.exp(-center*3.6)
noise=RNG.normal(0,1.4,(h,w))
arr=np.zeros((h,w,3),dtype=np.uint8)
for i,(base,lit) in enumerate(zip((7,12,17),(10,18,19))):
    arr[:,:,i]=np.clip(base+lit*halo+noise,0,255).astype('uint8')
bg=Image.fromarray(arr,'RGB').convert('RGBA')
d=ImageDraw.Draw(bg,'RGBA')
for k in range(10):
    a=int(11-0.8*k)
    d.ellipse((230-40*k,260-18*k,1380+40*k,860+20*k),outline=(90,132,124,a),width=1)
bg.save(OUT/'backdrop.png',optimize=True)

# Realistic top-down table; one pre-baked texture, zero per-frame pixel processing.
w,h=1250,540
cx,cy=w/2,h/2
x0,y0,x1,y1=35,30,1215,507
im=Image.new('RGBA',(w,h))
# Drop shadow blurred in precomputation.
shadow=Image.new('RGBA',(w,h));sd=ImageDraw.Draw(shadow)
sd.rounded_rectangle((x0+9,y0+19,x1+9,y1+19),radius=235,fill=(0,0,0,200))
shadow=shadow.filter(ImageFilter.GaussianBlur(19))
im.alpha_composite(shadow)
# dark wood/leather outer body with progressive rings
D=ImageDraw.Draw(im,'RGBA')
for b,c in [((x0,y0,x1,y1),(13,17,16,255)),((x0+2,y0+1,x1-2,y1-6),(45,48,40,255)),
            ((x0+10,y0+8,x1-10,y1-14),(81,68,47,255)),((x0+15,y0+13,x1-15,y1-18),(178,132,69,255)),
            ((x0+18,y0+16,x1-18,y1-22),(27,32,28,255)),((x0+38,y0+34,x1-38,y1-41),(7,49,40,255))]:
    D.rounded_rectangle(b,radius=240,fill=c)
# felt mask, clipped pixel gradient and dense cloth micro-grain
feltbox=(x0+43,y0+38,x1-43,y1-45)
mask=ellipse_mask(w,h,feltbox,225)
y,x=np.mgrid[0:h,0:w].astype('float32')
r2=((x-cx)/570)**2+((y-cy)/240)**2
spot=np.maximum(0,1-r2)
texture=RNG.normal(0,2.0,(h,w)).astype('float32')
weave=((x.astype('int32')%3==0).astype('float32')*0.7-(y.astype('int32')%3==0).astype('float32')*0.55)
vals=np.dstack([np.clip(9+10*spot+texture+weave,0,255),np.clip(60+39*spot+texture+weave,0,255),np.clip(46+24*spot+texture+weave,0,255)]).astype('uint8')
fr=Image.fromarray(vals,'RGB').convert('RGBA');fr.putalpha(mask);im.alpha_composite(fr)
D=ImageDraw.Draw(im,'RGBA')
# inset fine stitch + rail reflections
D.rounded_rectangle((x0+50,y0+45,x1-50,y1-52),radius=222,outline=(127,180,142,90),width=2)
D.line((242, y0+19, 1008, y0+19),fill=(245,209,149,48),width=2)
D.line((268, y1-22, 984, y1-22),fill=(0,0,0,60),width=2)
# betting line - subtle dashed ring (drawn as dots horizontally inside felt)
for i in range(75):
    xx=185+i*12
    if xx<=1070:
        D.line((xx,111,xx+5,111),fill=(170,215,190,26),width=1)
        D.line((xx,422,xx+5,422),fill=(170,215,190,22),width=1)
# casino embroidery, barely visible
# No center watermark: it distracts from the board and bet labels.
im.save(OUT/'table.png',optimize=True)

# Cards: color-accurate modern deck with a complete set of numerical pips.
CW,CH=196,282
ranks=list(range(2,15)); suits=['clubs','diamonds','hearts','spades']
red=(180,40,57,255); black=(33,43,52,255); gold=(165,118,60,255)

def suit_shape(layer,suit,cx,cy,size,color):
    # Use DejaVu Serif Unicode symbols for smooth, authentic suit contours.
    c={'clubs':'♣','diamonds':'♦','hearts':'♥','spades':'♠'}[suit]
    d=ImageDraw.Draw(layer,'RGBA')
    centered(d,(cx,cy),c,ft(int(size),serif=True),color)

def pip_positions(n):
    # Positions normalized in central panel (x between ~44 and ~152, y 85-243).
    if n == 1: return [(0.5,0.52)]
    if n == 2: return [(0.5,0.22),(0.5,0.78)]
    if n == 3: return [(0.5,0.22),(0.5,0.50),(0.5,0.78)]
    if n == 4: return [(0.25,0.23),(0.75,0.23),(0.25,0.77),(0.75,0.77)]
    if n == 5: return pip_positions(4)+[(0.5,0.5)]
    if n == 6: return [(v,z) for z in (0.23,0.5,0.77) for v in (0.25,0.75)]
    if n == 7: return pip_positions(6)+[(0.5,0.36)]
    if n == 8: return pip_positions(6)+[(0.5,0.36),(0.5,0.64)]
    if n == 9: return [(v,z) for z in (0.2,0.4,0.6,0.8) for v in (0.25,0.75)]+[(0.5,0.5)]
    if n == 10: return [(v,z) for z in (0.17,0.34,0.5,0.66,0.83) for v in (0.25,0.75)]
    return []

def make_card(rank,suit):
    im=Image.new('RGBA',(CW,CH),(0,0,0,0)); D=ImageDraw.Draw(im,'RGBA')
    D.rounded_rectangle((2,2,CW-3,CH-3),radius=14,fill=(249,248,242,255),outline=(204,205,199,255),width=2)
    D.rounded_rectangle((9,9,CW-10,CH-10),radius=9,outline=(223,218,207,255),width=1)
    # a thin gradient-like vertical stripe
    D.line((16,14,16,CH-15),fill=(255,255,255,170),width=1)
    co=red if suit in ('diamonds','hearts') else black
    rn=str(rank) if rank<=10 else {11:'J',12:'Q',13:'K',14:'A'}[rank]
    D.text((17,10),rn,font=ft(39,bold=True),fill=co,stroke_width=0)
    suit_shape(im,suit,32,73,34,co)
    # Mirrored tiny corner at bottom right using rotation
    lower=Image.new('RGBA',(75,105),(0,0,0,0)); dl=ImageDraw.Draw(lower,'RGBA')
    dl.text((8,6),rn,font=ft(34,bold=True),fill=co)
    suit_shape(lower,suit,23,62,29,co)
    im.alpha_composite(lower.rotate(180,expand=False),(CW-74,CH-105))
    if rank<=10:
        poss=pip_positions(rank) if rank<14 else [(0.5,0.5)]
        size=47 if rank<=6 else 35
        for px,py in poss:
            xx=49+px*100; yy=66+py*160
            suit_shape(im,suit,int(xx),int(yy),size,co)
    elif rank == 14:
        suit_shape(im,suit,CW//2,CH//2+2,115,co)
        D=ImageDraw.Draw(im,'RGBA')
        D.line((58,206,138,206),fill=(218,195,150,180),width=2)
    else:
        # Modern Art Deco-inspired court cards, original graphic design.
        D=ImageDraw.Draw(im,'RGBA')
        court=(35,68,CW-35,CH-55)
        D.rounded_rectangle(court,radius=8,fill=(239,234,222,255),outline=(181,146,85,255),width=3)
        D.rectangle((court[0]+8,court[1]+8,court[2]-8,court[3]-8),outline=(209,187,144,255),width=1)
        # geometric diagonal sash, decorative gold chevrons
        for i in range(6):
            y=103+i*21
            D.line((53,y,144,y+20),fill=(187,159,109,80),width=2)
        for offset in (0,8):
            D.polygon([(98,86+offset),(111,96+offset),(98,106+offset),(85,96+offset)],outline=(153,112,62,255),fill=(222,197,148,230))
        centered(D,(98,156),rn,ft(91,serif=True),co,stroke_width=1,stroke_fill=(255,255,255,100))
        suit_shape(im,suit,98,205,43,co)
    return im
atlas=Image.new('RGBA',(CW*13,CH*4),(0,0,0,0))
for sy,suit in enumerate(suits):
    for ri,rank in enumerate(ranks):
        atlas.alpha_composite(make_card(rank,suit),(ri*CW,sy*CH))
atlas.save(OUT/'cards_atlas.png',optimize=True)

# Deep sapphire playing-card reverse with linework and metallic emblem.
b=Image.new('RGBA',(CW,CH)); d=ImageDraw.Draw(b,'RGBA')
d.rounded_rectangle((2,2,CW-3,CH-3),radius=15,fill=(16,37,57,255),outline=(217,185,120,255),width=3)
d.rounded_rectangle((9,9,CW-10,CH-10),radius=12,outline=(157,175,168,185),width=2)
d.rounded_rectangle((17,17,CW-18,CH-18),radius=6,outline=(185,145,81,220),width=2)
for i in range(-250,350,19):
    for j in range(-200,350,19):
        x=i+((j//19)%2)*10
        if 22<x<CW-22 and 23<j<CH-23:
            d.line((x,j-5,x+5,j,x,j+5,x-5,j-5),fill=(84,127,143,96),width=1)
# dark inner seal
d.ellipse((42,83,154,197),fill=(9,27,44,244),outline=(226,193,133,255),width=3)
d.ellipse((49,90,147,190),outline=(145,170,173,170),width=1)
suit_shape(b,'spades',98,136,86,(222,189,118,255))
d=ImageDraw.Draw(b,'RGBA')
centered(d,(98,226),'P O K E R  A I',ft(12,bold=True),(207,181,128,255))
b.save(OUT/'card_back.png',optimize=True)
print('Assets:',[(p.name,p.stat().st_size) for p in sorted(OUT.glob('*.png'))])
