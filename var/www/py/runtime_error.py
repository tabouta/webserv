#!/usr/bin/env python3

import sys

try:
    print("Content-Type: text/plain\n")
    raise Exception("To get 502 error on CGI")
except Exception as e:
    print(f"Error: {str(e)}", file=sys.stderr)
    exit(1)
