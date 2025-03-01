#!/usr/bin/env python3

import sys
import os
import requests
from html.parser import HTMLParser
from urllib.parse import unquote
from dateutil.parser import parse

if os.environ.get("SORTSEARCHS"):
  HTTP='https://127.0.0.1:'
else:
  HTTP='http://127.0.0.1:'

URL=HTTP + str(sys.argv[2]) + sys.argv[3]

class MyHTMLParser(HTMLParser):
    def __init__(self):
      super().__init__()
    def handle_starttag(self, tag, attrs):
      if tag == "a":
        for attr in attrs:
          if attr[0] == "href":
            print(unquote(attr[1]), end=" ")
      elif tag == "img":
        for attr in attrs:
          if attr[0] == "src":
            print(unquote(attr[1]), end=" ")

    def handle_data(self, data):
        print(data, end=" ")

    def handle_endtag(self, tag):
      if tag == "tr":
        print("[tr ends]")

pars = {
    0:{'C':'N', 'O':'A'},
    1:{'C':'N', 'O':'D'},
    2:{'C':'M', 'O':'A'},
    3:{'C':'M', 'O':'D'},
    4:{'C':'S', 'O':'A'},
    5:{'C':'S', 'O':'D'}
}

f = open('../auth.txt')
creds = f.read().split(':')
response = requests.get(URL, auth=(creds[0], creds[1]), params=pars[int(sys.argv[1])], verify=False)
parser = MyHTMLParser()
parser.feed(str(response.content))
