#!/usr/bin/env python

# USAGE: ./mkfont.py FONT_BMP [OUTPUT_FILE]
# FONT_BMP must be a black/white (#000000, #FFFFFF) 128x64px bmp
# if OUTPUT_FILE is omitted, will print to stdout

import sys
from PIL import Image

def printFont(font, dst=sys.stdout):
    for y in range(8):
        for c in range(128):
            for x in range(8):
                    print(font[1024 * y + 8 * c + x], end=',', file=dst)
            print(' ', end='', file=dst)
        print('', file=dst)

def readBmpChar(font, pxs, c):
    cy = c // 16
    cx = c % 16
    pxy = cy * 8
    pxx = cx * 8
    for y in range(8):
        for x in range(8):
            v = 1 if pxs[pxx + x, pxy + y][0] > 0 else 0
            font[y * 1024 + c * 8 + x] = v

def toFont(img):
    pxs = img.load()
    font = [None for _ in range(1024 * 8)]
    for c in range(128):
        readBmpChar(font, pxs, c)
    return font


def readArgs():
    if len(sys.argv) <= 1:
        print("missing font source", file=sys.stderr)

    srcfile = open(sys.argv[1], mode="rb")

    try:
        dstfile = open(sys.argv[2], mode="w")
    except IndexError:
        dstfile = sys.stdout

    return (srcfile, dstfile)

def main():
    src, dst = readArgs()
    with Image.open(src) as img:
        font = toFont(img)
        printFont(font, dst)
    src.close()
    dst.close()


if __name__ == "__main__":
    main()