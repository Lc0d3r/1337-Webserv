import os

print("Content-Type: text/plain\r\n\r\n")
cookies = os.environ.get("ACCOUNTS", "")
print("Theme cookie found: " + cookies)