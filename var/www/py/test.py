#!/usr/bin/env python3
import cgi
import os

print("Content-Type: text/plain\n")

form = cgi.FieldStorage()

if "file" not in form:
    print("No file uploaded")
    exit()

fileitem = form["file"]
filename = os.path.basename(fileitem.filename)

upload_dir = "./uploadstore"
os.makedirs(upload_dir, exist_ok=True)

filepath = os.path.join(upload_dir, filename)

with open(filepath, "wb") as f:
    f.write(fileitem.file.read())

print(f"File '{filename}' uploaded successfully to {filepath}")
