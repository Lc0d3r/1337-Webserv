import os

print("Content-Type: text/plain\r\n\r\n")
cookies = os.environ.get("HTTP_COOKIE", "")
if "theme=" in cookies:
    print("Theme cookie found: " + cookies)
else:
    print("Theme cookie not found.")