import sys
import math

def read_header(in_file):
    width = None
    height = None
    for line in in_file:
        if line.startswith('FONTBOUNDINGBOX'):
            sline = line.split(' ')
            width = int(sline[1])
            height = int(sline[2])
        elif line.startswith('CHARS '):
            break
    return (width, height)

def read_bitmap(in_file, width, height):
    byte_width = math.ceil(width / 8)

    bitmap = [list([0 for _ in range(byte_width)]) for _ in  range(height)]
    i = 0
    for line in in_file:
        if line.startswith('ENDCHAR'):
            break
        n = len(line) // 2
        for j in range(n):
            byte = int(line[j * 2 : j * 2 + 2], base=16)
            bitmap[i][j] = byte

        print(line[:-1])
        print(bitmap[i])
        i += 1
    return bitmap


def read_char(in_file, width, height):
    encoding = None
    bitmap = None
    for line in in_file:
        if line.startswith('ENCODING'):
            encoding = int(line.split(' ')[1])
        elif line.startswith('BITMAP'):
            bitmap = read_bitmap(in_file, width, height)
            break
    return encoding, bitmap


def generate_bmh(in_file, outfile):
    width, height = read_header(in_file)

    raw_chars = []

    while True:
        enc, bitmap = read_char(in_file, width, height)
        if enc is None:
            break
        raw_chars.append((enc, bitmap))

    chars = []
    for enc, raw_bitmap in raw_chars:
        bitmap = []
        for row in range(height // 8):
            for x in range(width):
                b = 0
                for y in range(row * 8, row * 8 + 8):
                    byte = raw_bitmap[y][x // 8]
                    bit = (byte >> (7 - x % 8)) & 0x01
                    b |= bit << (y % 8)
                bitmap.append(b)
        chars.append((enc, bitmap))

    outfile.write("// Header File for SSD1306 characters\n")
    outfile.write("// Font Size: " + str(height) + "\n")
    outfile.write('const static uint8_t FONT[] PROGMEM = {\n')
    outfile.write('    0x00, 0x00, // fixed width font\n')
    outfile.write(f'    {hex(width)}, {hex(height)}, // width, height\n')
    outfile.write('    0x40, // first char ("@")\n')
    outfile.write(f'    {hex(len(chars))}, // char count\n')
    for enc, bitmap in chars:
        arr = ','.join([hex(b) for b in bitmap])
        outfile.write(f'    {arr}, // {chr(enc)}\n')
    outfile.write('};\n')

def main():
    in_file = sys.argv[1]
    out_file = sys.argv[2]

    with open(in_file, "rt") as fi:
        with open(out_file, "w+") as fo:
            generate_bmh(fi, fo)

main()