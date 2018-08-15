#!/usr/bin/python
"""
Generate the Incore Resource:
	1. Read the image information and translate them as the incore resource
	2. use "bin2c" to generate the resource
"""

import os, sys
import time
import re
import traceback
from PIL import Image

USE_PNG2BMP="n"
USE_PNG2GIF="n"

def trace_back():
	try:
		return traceback.print_exc()
	except:
		return ''


def usage():
	print """
		usage gen_incore_res <appollo-path> <out-dir>
	"""


fw_pattern = re.compile("^\s*(\w+)")
def get_first_word(str):
	m = fw_pattern.match(str)
	if m == None:
		return None
	return m.group(1)

imgres_patther = re.compile(".*\(\s*(\w+)\s*,\s*\"([^\"]*)\"\s*\)")
def parer_images(f):
	img_dict = { }
	while True:
		line = f.readline()
		if line == None:
			break
		fw = get_first_word(line)
		if fw == "end_image_res":
			break
		if fw != "image":
			continue
		# read the image information
		m = imgres_patther.match(line)
		if m == None:
			continue
		img_dict[m.group(1)] = m.group(2)
	return img_dict

def png2bmp(f_in, f_out):
	if os.path.isfile(f_in) == False:
		print "File "+f_in+" NOT exists."
		return

	img = Image.open(f_in)

	if len(img.split()) == 4:
	# prevent IOError: cannot write mode RGBA as BMP
		r, g, b, a = img.split()
		img = Image.merge("RGB", (r, g, b))
		img.save(f_out)
	else:
		img.save(f_out)

def png2gif(f_in, f_out):
	if os.path.isfile(f_in) == False:
		print "File "+f_in+" NOT exists."
		return

	img = Image.open(f_in)
	img = img.convert('RGB').convert('P', palette=Image.ADAPTIVE)
	#img = img.convert('RGB').convert('P')
	img.save(f_out)

def parser_res_pkg(f):
	res_infos = { }
	while True:
		line = f.readline()
		if line == None:
			break;
		fw = get_first_word(line)
		if fw == "begin_respkg" or fw == "begin_sys_respkg":
			if fw == "begin_respkg":
				m = re.match("\s*begin_respkg\((\w+).*\)", line)
				if m:
					res_infos["pkgName"] = m.group(1)
			else:
				res_infos["pkgName"] = "sys"

			while True:
				line = f.readline()
				if line == None: break
				fw = get_first_word(line)
				if fw == "begin_image_res":
					res_infos["images"] = parer_images(f)
				elif fw == "end_respkg" or fw == "end_sys_respkg":
					return res_infos
	return res_infos


def str2key(str):
	key = 0;
	l = len(str)
	if l <= 0: return 0
	l = (l)/2
	i = 0
	while i < l:
		w = (ord(str[i<<1])&0xFF) | ((ord(str[(i<<1)+1])<<8)&0xFF00)
		key ^= (w << (i&0xf))
		i += 1
	return key

def get_exten_name(name):
	m = re.match(".*\.(\w+)$", name)
	if m != None:
		return m.group(1)
	return ""

def bin2c(src, dest, dest_name,pkg_name):
	'''
	try:
		fin = open(src, "rb")
	except:
		print("bin2c: open input file %s failed\n"%src)
		return (0, "")
	try:
		fout = open(dest, "w")
	except:
		print("bin2c: open output file %s failed\n"%dest)
		fin.close()
		return (0, "")

	s = fin.read()

	fout.write("/*\n** $Id$\n**\n** " + "%s_%s.cpp"%(pkg_name, dest_name) + " : TODO\n**\n** Copyright (C) 2002 ~ ")
	fout.write(time.strftime('%Y',time.localtime(time.time()))+" Beijing FMSoft Technology Co., Ltd.\n**\n** All rights reserved by FMSoft.\n**\n** Current Maintainer : " + os.uname()[1] + "\n**\n")
	fout.write("** Create Date : "+time.strftime('%Y-%m-%d',time.localtime(time.time()))+"\n*/\n\n")

	fout.write("#include <Apollo.h>\n\n")
	fout.write("#ifdef _APOLLO_INNER_RES\n\n")
	fout.write("#ifdef __cplusplus\nextern \"C\"\n{\n#endif /* __cplusplus */\n\n")
	fout.write("extern const NGUByte _%s_%s_img_data[];\n\n"%(pkg_name, dest_name))
	fout.write("#ifdef __cplusplus\n}\n#endif /* __cplusplus */\n\n")

	fout.write("const NGUByte _%s_%s_img_data[] = {\n  "%(pkg_name, dest_name))
	i = 0
	for c in s:
		fout.write(" 0x%02X,"%ord(c))
		i += 1
		if i == 8:
			fout.write("   ")
		elif i == 16:
			fout.write("\n  ")
			i = 0
	fout.write("\n};\n\n")
	fout.write("#endif   // _APOLLO_INNER_RES\n\n")
	fin.close()
	fout.close()
	return (len(s), "_%s_%s_img_data"%(pkg_name, dest_name))
	'''
	os.system("gen_incore_bitmap " + src + " " + dest + " " + dest_name + " " + pkg_name);
	return (1, "_%s_%s_bitmap"%(pkg_name, dest_name))


def gen_incore_images(prefix, img_prefix, pkg_name, img_dict):
	# make the dir for img dict
	os.system('mkdir -p "%s/%s/images"'%(prefix,pkg_name))
	# open the out put file
	fout = open("%s/%s/images/_img_%s_inner_res.cpp" %(prefix,pkg_name, pkg_name), "w")
	img_list = []
	img_file_list = []

	fout.write("/*\n** $Id$\n**\n** " + "_img_%s_inner_res.cpp"%(pkg_name) + " : TODO\n**\n** Copyright (C) 2002 ~ ")
	fout.write(time.strftime('%Y',time.localtime(time.time()))+" Beijing FMSoft Technology Co., Ltd.\n**\n** All rights reserved by FMSoft.\n**\n** Current Maintainer : " + os.uname()[1] + "\n**\n")
	fout.write("** Create Date : "+time.strftime('%Y-%m-%d',time.localtime(time.time()))+"\n*/\n\n")
	fout.write("#include <Apollo.h>\n\n")
	fout.write('#ifdef _APOLLO_INNER_RES\n\n')

	for item in img_dict.items():
		try:
			if USE_PNG2BMP == "y":
				tmpfile = ".tmpfile_png2bmp.bmp"
				png2bmp("%s/%s"%(img_prefix, item[1]), tmpfile)
				imginfo = bin2c(tmpfile, "%s/%s/images/%s.cpp"%(prefix,pkg_name,pkg_name+"_"+item[0]), item[0], pkg_name)
			elif USE_PNG2GIF == "y":
				tmpfile = ".tmpfile_png2bmp.gif"
				png2gif("%s/%s"%(img_prefix, item[1]), tmpfile)
				imginfo = bin2c(tmpfile, "%s/%s/images/%s.cpp"%(prefix,pkg_name,pkg_name+"_"+item[0]), item[0], pkg_name)
			else:
				imginfo = bin2c("%s/%s"%(img_prefix, item[1]), "%s/%s/images/%s.cpp"%(prefix,pkg_name,pkg_name+"_"+item[0]), item[0], pkg_name)

		except:
			print trace_back()
		if imginfo[0] > 0:
			img_list.append((imginfo[1], imginfo[0], item[1]))
			fout.write('extern const NGUByte %s[];\n'%(imginfo[1]))
			img_file_list.append("%s/images/%s.cpp"%(pkg_name, pkg_name +"_"+ item[0]))
		else:
			print ("gen_incore_images: translate %s/%s failed\n"%(img_prefix, item[1]))

	fout.write("\nstatic INNER_RES_INFO _img_%s_inner_res[] = {\n"%pkg_name)
	for item in img_list:
		fout.write("\t{ \"%s\",\t(NGUByte*)%s,\t%d},//%s\n"%(item[2], item[0], item[1], item[2]))
	fout.write("};\n\n")
	fout.write("#endif   // _APOLLO_INNER_RES\n\n")
	fout.close()
	return img_file_list

def gen_inner_res(res_file, res_prefix, pkg_name, outdir):
	f = open(res_file)
	res_infos = parser_res_pkg(f)
	#print("---- %s"%pkg_name)
	#print(res_infos)
	f.close()
	try:
		return gen_incore_images(outdir, res_prefix, pkg_name, res_infos["images"])
	except:
		return []

def out_cmakelist(inner_res_file_list, outdir):
	f = open(outdir + "/CMakeLists.txt","w")
	f.write("""##############################################################################
## $Id$
##
## CMakeLists.txt : This file is create by gen_incore_res.py, don't edit it
##
""")
	f.write("## Copyright (C) 2002 ~ "+time.strftime('%Y',time.localtime(time.time()))+" Beijing FMSoft Technology Co., Ltd.\n")
	f.write("""##
## All rights reserved by FMSoft.
##
""")
	f.write("## Current Maintainer : " + os.uname()[1] + "\n##\n")
	f.write("## Create Date : "+time.strftime('%Y-%m-%d',time.localtime(time.time())) + "\n")
	f.write("""##############################################################################

# append the source
LIST (APPEND inner_res_sources
""")
	for source in inner_res_file_list:
		f.write("\t%s\n"%source)
	f.write("\tinner_resource.cpp)\n\n")
	f.write("""
mg_add_source_files(${inner_res_sources})

mg_commit_source_files ()
""")
	f.close()


def out_makelist(inner_res_file_list, outdir):
	f = open(outdir + "/list.mk", "w")
	f.write("""##############################################################################
## $Id$
##
## list.mk : The inner resource file list
##
""")
	f.write("## Copyright (C) 2002 ~ "+time.strftime('%Y',time.localtime(time.time()))+" Beijing FMSoft Technology Co., Ltd.\n")
	f.write("""##
## All rights reserved by FMSoft.
##
""")
	f.write("## Current Maintainer : " + os.uname()[1] + "\n##\n")
	f.write("## Create Date : "+time.strftime('%Y-%m-%d',time.localtime(time.time())) + "\n")
	f.write("""##############################################################################

# This file is create by gen_incore_res.py, don't edit it

inner_res_sources = \\
""")
	for source in inner_res_file_list:
		f.write("\t%s \\\n"%source)
	f.write('\tinner_resource.cpp\n')
	f.close()

if __name__ == "__main__":
	#usage()
	applo_path="."
	out_dir="inner-res"
	#usage()
	#applo_path = sys.argv[1]
	#out_dir = sys.argv[2]
	#TODO add the package when you add one application
	pakages=("ime", "bootup", "shutdown", "launcher", "calculator", "countdown", "stopwatch", "unitconvertion", "calendar", "notepad", "phonebook", "videoplayer", "audioplayer", "videorecorder", "camera", "callhistory", "call", "sms", "settings", "profile")
	inner_res_file_list = gen_inner_res(applo_path + "/sysres/sys.res.c", applo_path ,  "sys", out_dir)

	fsource = open(out_dir + "/inner_resource.cpp", "w")

	fsource.write("/*\n** $Id$\n**\n** inner_resource.cpp : the inner resource for applo\n**\n** Copyright (C) 2002 ~ ")
	fsource.write(time.strftime('%Y',time.localtime(time.time()))+" Beijing FMSoft Technology Co., Ltd.\n**\n** All rights reserved by FMSoft.\n**\n** Current Maintainer : " + os.uname()[1] + "\n**\n")
	fsource.write("** Create Date : "+time.strftime('%Y-%m-%d',time.localtime(time.time()))+"\n*/\n\n")

	fsource.write('#include "Apollo.h"\n\n')
	fsource.write("#ifdef _APOLLO_INNER_RES\n\n")
	fsource.write('USE_NGUX_NAMESPACE\n\n\n')
	fsource.write('#include "sys/images/_img_sys_inner_res.cpp"\n')
	array_list = "\tRegisterInnerResource(R_TYPE_IMAGE, _img_sys_inner_res, sizeof(_img_sys_inner_res)/sizeof(INNER_RES_INFO));\n"

	inner_res_file_list = inner_res_file_list + gen_inner_res(applo_path + "/sysres/filetypes.res.c", applo_path ,  "filetypes", out_dir)
	fsource.write('#include "filetypes/images/_img_filetypes_inner_res.cpp"\n')
	array_list = array_list + "\tRegisterInnerResource(R_TYPE_IMAGE, _img_filetypes_inner_res, sizeof(_img_filetypes_inner_res)/sizeof(INNER_RES_INFO));\n"

	for p in pakages:
		inner_res_file_list = inner_res_file_list + gen_inner_res(applo_path + "/%s/src/%s.res.c"%(p,p), applo_path , p, out_dir)
		fsource.write('#include "%s/images/_img_%s_inner_res.cpp"\n'%(p,p))
		array_list = array_list + "\tRegisterInnerResource(R_TYPE_IMAGE, _img_%s_inner_res, sizeof(_img_%s_inner_res)/sizeof(INNER_RES_INFO));\n"%(p,p)

	fsource.write("\n\nvoid Apollo_register_inner_res(void)\n{\n")
	fsource.write(array_list);
	fsource.write("}\n\n")
	fsource.write("#endif   // _APOLLO_INNER_RES\n\n")
	fsource.close()

	out_cmakelist(inner_res_file_list, out_dir)
	out_makelist(inner_res_file_list, out_dir)

