#!/usr/bin/env python
#
# Copyright(C) 2005-2014 Simon Howard
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
#
# Converts images into C structures to be inserted in programs
#

import sys

try:
    import Image
except ImportError:
    try:
        from PIL import Image
    except ImportError:
        print("WARNING: Could not update %s.  "
              "Please install the Python Imaging library or Pillow."
              % sys.argv[2])
        sys.exit(0)


def convert_image(filename, output_filename):

    im = Image.open(filename)

    outfile = open(output_filename, "w")

    size = im.size

    outfile.write("int icon_w = %i;\n" % (size[0]))
    outfile.write("int icon_h = %i;\n" % (size[1]))

    outfile.write("\n")
    outfile.write("const unsigned int icon_data[] = {\n")

    elements_on_line = 0

    outfile.write("    ")

    for y in range(size[1]):
        for x in range(size[0]):
            val = im.getpixel((x, y))
            outfile.write("0x%02x%02x%02x%02x, " % val)
            elements_on_line += 1

            if elements_on_line >= 6:
                elements_on_line = 0
                outfile.write("\n")
                outfile.write("    ")

    outfile.write("\n")
    outfile.write("};\n")


convert_image(sys.argv[1], sys.argv[2])
