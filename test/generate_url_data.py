import random
import sys

count = int(sys.argv[1])
url = 'https://en.wikipedia.org/wiki/Main_Page'

with open("tc.txt", 'w') as fw:
    for i in range(count):
        ttl = random.randrange(2,10)
        resp = random.choice([200, 201, 401, 403, 500])
        fw.write(f'{url}/{i} {ttl} {resp}\n', )
