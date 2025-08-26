#!/usr/bin/env python3

import os
import sys
import cgi
import cgitb
cgitb.enable()  # Enable debugging

print("Content-Type: text/html")
print()  # End of headers

print("<html><head><title>CGI Test</title></head><body>")
print("<h1>CGI Test Output</h1>")

# Show environment variables
print("<h2>Environment Variables</h2><ul>")
for key, value in os.environ.items():
    print(f"<li><strong>{key}</strong>: {value}</li>")
print("</ul>")

# Show query string
query_string = os.environ.get('QUERY_STRING', '')
print(f"<h2>Query String</h2><p>{query_string}</p>")

# Parse form data
form = cgi.FieldStorage()
if form:
    print("<h2>Form Data</h2><ul>")
    for key in form.keys():
        print(f"<li><strong>{key}</strong>: {form.getvalue(key)}</li>")
    print("</ul>")
else:
    print("<h2>No form data received</h2>")

# If POST, print body
if os.environ.get('REQUEST_METHOD', '') == 'POST':
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    body = sys.stdin.read(content_length)
    print("<h2>POST Body</h2>")
    print(f"<pre>{body}</pre>")

print("</body></html>")
