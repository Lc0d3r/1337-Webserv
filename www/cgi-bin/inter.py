#!/usr/bin/env python3

import cgi
import os

print("<h1>Content-Type: text/plain</h1>")

# Get the QUERY_STRING from environment (for GET requests)
query_string = os.environ.get("QUERY_STRING", "")

print("<h1>Hello from Python CGI!</h1>")
print("<p>Query string: {}</p>".format(query_string))
