#!/usr/bin/env python3

import sys
import os

print("Content-Type: text/html\r\n\r\n")
print("<html><body>")

# Affiche la méthode (GET ou POST)
print("<h2>CGI POST Test</h2>")
print(f"<p>Request Method: {os.environ.get('REQUEST_METHOD')}</p>")

# Affiche les données reçues (body en stdin)
if os.environ.get('REQUEST_METHOD') == 'POST':
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    post_data = sys.stdin.read(content_length)
    print(f"<p>POST Data: {post_data}</p>")

print("</body></html>")
