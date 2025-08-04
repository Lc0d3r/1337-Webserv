#!/usr/bin/env python3

import os
import sys

def get_post_data():
    content_length = int(os.environ.get("CONTENT_LENGTH", 0))
    return sys.stdin.read(content_length)

def main():
    print("Content-Type: text/plain\r\n")
    print("POST Data Received:")
    print("--------------------")
    post_data = get_post_data()
    print(post_data)

if __name__ == "__main__":
    main()
