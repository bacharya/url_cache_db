import random

url = 'https://stackoverflow.com/questions/14295980/md5-reference-error'

with open("tc.txt", 'w') as fw:
    for i in range(50000):
        ttl = random.randrange(2,10)
        resp = random.choice([200, 201, 401, 403, 500])
        fw.write(f'{url}/{i} {ttl} {resp}\n', )