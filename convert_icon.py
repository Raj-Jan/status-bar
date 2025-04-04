import argparse
import struct
import zlib
import os
import re

def sanitize_filename(filename):
	"""Convert filename to a valid C identifier by replacing invalid characters."""
	name = os.path.splitext(os.path.basename(filename))[0]  # Remove directory & extension
	name = re.sub(r'\W+', '_', name)  # Replace non-alphanumeric characters with '_'
	return name

def paeth_predictor(a, b, c):
	"""Paeth predictor filter function."""
	p = a + b - c
	pa = abs(p - a)
	pb = abs(p - b)
	pc = abs(p - c)
	if pa <= pb and pa <= pc:
		return a
	elif pb <= pc:
		return b
	else:
		return c

def apply_png_filter(filter_type, scanline, prev_scanline, bpp):
	"""Applies PNG filtering to reconstruct raw pixel data."""
	result = bytearray(scanline)
	
	if filter_type == 0:  # None
		return result
	elif filter_type == 1:  # Sub
		for i in range(bpp, len(scanline)):
			result[i] = (scanline[i] + result[i - bpp]) & 0xFF
	elif filter_type == 2:  # Up
		for i in range(len(scanline)):
			result[i] = (scanline[i] + prev_scanline[i]) & 0xFF
	elif filter_type == 3:  # Average
		for i in range(len(scanline)):
			left = result[i - bpp] if i >= bpp else 0
			above = prev_scanline[i]
			result[i] = (scanline[i] + (left + above) // 2) & 0xFF
	elif filter_type == 4:  # Paeth
		for i in range(len(scanline)):
			left = result[i - bpp] if i >= bpp else 0
			above = prev_scanline[i]
			upper_left = prev_scanline[i - bpp] if i >= bpp else 0
			result[i] = (scanline[i] + paeth_predictor(left, above, upper_left)) & 0xFF
	return result

def read_png(filename):
	"""Reads a PNG file and extracts raw pixel data in RGBA format."""
	with open(filename, 'rb') as f:
		png = f.read()

	# Verify PNG signature
	if png[:8] != b'\x89PNG\r\n\x1a\n':
		raise ValueError("Not a valid PNG file")

	pos = 8  # Skip signature
	width, height, bit_depth, color_type = (None, None, None, None)
	idat_data = b''

	while pos < len(png):
		chunk_len = struct.unpack(">I", png[pos:pos+4])[0]
		chunk_type = png[pos+4:pos+8]
		chunk_data = png[pos+8:pos+8+chunk_len]
		pos += 8 + chunk_len + 4  # Move to next chunk

		if chunk_type == b'IHDR':
			width, height, bit_depth, color_type, _, _, _ = struct.unpack(">IIBBBBB", chunk_data)
			if color_type != 6 or bit_depth != 8:
				raise ValueError("Only 8-bit RGBA PNGs are supported")
		elif chunk_type == b'IDAT':
			idat_data += chunk_data

	if width is None or height is None:
		raise ValueError("Invalid PNG file (missing IHDR)")

	# Decompress IDAT data
	decompressed = zlib.decompress(idat_data)

	# PNG filter-based decoding
	stride = width * 4  # Each pixel = 4 bytes (RGBA)
	image_data = bytearray()
	prev_scanline = bytearray(stride)

	i = 0
	for _ in range(height):
		filter_type = decompressed[i]
		scanline = bytearray(decompressed[i+1:i+1+stride])
		i += 1 + stride

		# Apply the correct filter
		filtered = apply_png_filter(filter_type, scanline, prev_scanline, 4)
		image_data.extend(filtered)
		prev_scanline = filtered  # Save for next row

	return width, height, bytes(image_data)

def convert_image_to_c_array(input_image, output_file):
	"""Converts PNG to a C header file with raw RGBA data."""
	width, height, image_data = read_png(input_image)
	var_name = sanitize_filename(input_image)

	# Format data as a C array
	c_array = ', '.join(f'0x{b:02x}' for b in image_data)

	# Write to output file
	with open(output_file, "w") as f:
		f.write(f"const unsigned char icon_{var_name}_data[] = {{ {c_array} }};\n")

	print(f"Conversion complete! Data saved to {output_file}")

if __name__ == "__main__":
	parser = argparse.ArgumentParser(description="Convert PNG to C byte array without external libraries.")
	parser.add_argument("input_image", help="Path to the input PNG image")
	parser.add_argument("output_file", help="Path to the output C header file")
	args = parser.parse_args()

	convert_image_to_c_array(args.input_image, args.output_file)
