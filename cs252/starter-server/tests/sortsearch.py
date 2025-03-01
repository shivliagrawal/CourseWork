import sys
import os
import requests
from html.parser import HTMLParser
from urllib.parse import unquote

if os.environ.get("SORTSEARCHS"):
  HTTP='https://127.0.0.1:'
else:
  HTTP='http://127.0.0.1:'

URL=HTTP + str(sys.argv[2]) + sys.argv[3]

class MyHTMLParser(HTMLParser):
    def __init__(self):
      super().__init__()
    def handle_starttag(self, tag, attrs):
      if "a" == tag and len(attrs[0]) == 2 and "href" == attrs[0][0] and "C=" not in attrs[0][1] and attrs[0][1] not in URL:
        att = attrs[0][1]
        if not os.environ.get("KEEPSLASH") and att[-1] == '/':
          att = attrs[0][1][:-1]
        print(unquote(att))

pars = {
    0:{'C':'N', 'O':'A'},
    1:{'C':'N', 'O':'D'},
    2:{'C':'M', 'O':'A'},
    3:{'C':'M', 'O':'D'},
    4:{'C':'S', 'O':'A'},
    5:{'C':'S', 'O':'D'},
    6:{}
}

f = open('../auth.txt')
creds = f.read().split(':')
response = requests.get(URL, auth=(creds[0], creds[1]), params=pars[int(sys.argv[1])], verify=False)
parser = MyHTMLParser()
parser.feed(str(response.content))
