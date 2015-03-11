#!/usr/bin/python3

# Copyright (c) 2015, Andrei Danaila. All Rights Reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 and
# only version 2 as published by the Free Software Foundation.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# About: This tiny script creates a directory higherarchy based on when the photos
#        were taken and moves the photos into the higherarchy.
#        The application uses nefarious to parse the NEF header and obtain the date
#        the photo was taken to create the directory structure. The script then copies
#        the photo into the new directory structure

# Use:   To use simply invoke ./sort_nef --in-dir=<source_dir> --out-dir=<destination dir>

# Warning: sort_nef uses nefarious to read in the NEF meta data, exercise the same caution
#          you would when using nefarious.

import getopt, sys
import re, string
import os
import time, datetime
import json
import shutil
from subprocess import Popen, PIPE

def main(argv):
    try:
        opts, args = getopt.getopt(argv,"d:o:", ["in-dir=", "out-dir="])
        if not opts:
            usage()
    except getopt.GetoptError:
        usage()
        sys.exit(2)

    for opt, arg in opts:
        if opt == '-d':
            in_folder = arg
            if not os.path.exists(in_folder):
                print ("ERROR: Input folder does not exist")
                sys.exit(2)
        if opt == '-o':
            out_folder = arg + "/"

    date_pattern = re.compile("(DateTimeDigitized.*)([0-9][0-9][0-9][0-9]:[0-9]+:[0-9]+)")

    file_dict = {}
    proc_img_cntr = 0
    failed_img_cntr = 0
    for root, dirs, files in os.walk(in_folder):
        for filename in files:
            if os.path.splitext(filename)[1] != ".NEF":
                continue
            file_time_string = ""
            print (os.path.join(root, filename))
            process = Popen(["nef-cli", "-i", os.path.join(root, filename)], stdout = PIPE)
            (output, err) = process.communicate()
            exit_code = process.wait()
            if exit_code != 0:
                failed_img_cntr += 1
                continue
            output_string = output.decode("ascii")
            for date_match in re.finditer(date_pattern, output_string):
                file_time_string = date_match.group(2)
                print ("File: %s, Date Taken: %s" % (filename, file_time_string))
                file_timestamp = time.strptime(file_time_string, "%Y:%m:%d")
                file_time_string = file_time_string.replace(":","_")

                append_path(file_dict, file_time_string, os.path.join(root,filename))

                proc_img_cntr += 1
    print ("Processed %d images. Failed to process %d images." % (proc_img_cntr, failed_img_cntr))

    if not os.path.exists(out_folder):
        print ("Creating out directory")
        os.mkdir(out_folder)
    for date in file_dict:
        if not os.path.exists(out_folder + date):
            os.mkdir(out_folder + date)
        for path in file_dict[date]:
            print ("Copying from %s to %s" % (path, out_folder + date))
            shutil.copy2(path, out_folder + date)

def append_path(file_dict, date_key, path_value):
    if date_key in file_dict:
        file_dict[date_key].append(path_value)
    else:
        file_dict[date_key] = [path_value];

def usage():
    print ("To use: sort_nef.py -d <input directory> -o <output directory>")

if __name__ == "__main__": main(sys.argv[1:])
