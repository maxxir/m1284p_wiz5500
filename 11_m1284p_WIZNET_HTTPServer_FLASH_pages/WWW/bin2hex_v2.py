# -- coding: utf-8 --
#Reading file in binary & print in HEX AVR PROGCHAR ARRAY every symbol & to ***.h
#Ethercard helper utilites for something like: <bfill.emit_raw_p(index_htm, sizeof(index_htm));>
#(c) Ibragimov M. Russia Togliatty 19/08/2014
# .PS used for build <***.h> from images(*.gif, *.jpg), *.css, static htm(l) pages, javascript etc..
import sys, os
if len(sys.argv) < 2:
    sys.exit('Usage: %s file-name' % sys.argv[0])

file_name = sys.argv[1]
if not os.path.exists(sys.argv[1]):
    sys.exit('ERROR: Filename %s was not found!' % file_name)
else:
	print('File %s is OK!' % file_name)
file_out = file_name.replace(".", "_") + ".h"
print('File_to_write is: %s' % file_out)
fhex = open(file_out, "w")
fhex_str = 'const char %s[] PROGMEM = {' % file_name.replace(".", "_") 
print(fhex_str)
fhex.write(fhex_str + '\n')
with open(file_name, "rb") as f:
    byte = f.read(1)
    i = 0
    fhex_size = 0
    _str = ""
    while byte != "":
		# Do stuff with byte.
		_byte = f.read(1)
		fhex_size = fhex_size + 1
		if _byte != "":
			#print ('%s, ' % hex(ord(byte)))
			_str = _str + "%s," % hex(ord(byte))
		else:
			#Last byte wo <,>
			#print hex(ord(byte))
			_str = _str + "%s" % hex(ord(byte))
			if i < 15:
				print _str
				fhex.write(_str + '\n')
		byte = _byte;
		i = i + 1;
		if i > 15:
			i = 0;
			print _str
			fhex.write(_str + '\n')
			_str = ""
print'};'
fhex.write('};\n')
_str = '%s: %d bytes' % (file_name, fhex_size)
print(_str);
fhex.write('//' + _str + '\n');
fhex.close()
