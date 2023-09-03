#!/bin/python3
import json
from random import randint

with open('wiktdump.json', 'r') as f:
    print('Starting!')
    s = f.read()
    print('Finished reading!')
    j = json.loads(s)
    print('Finished loading json!')


# template = {'pos': '', 'word': '', 'sounds': [], 'senses': []}
# sounds = {'ipa: ''}
# stempl = {'links': [], 'glosses': []}
# links = ["einem", "einem#German"], ["'n", "'n#German"]
# glosses = ['', '']

def adds(s):
    sb = s.encode('utf-8')
    lsb = len(sb)
    res = b''
    res += lsb.to_bytes(2)
    res += sb
    return res

def ts(x):
    # pjint(x)
    res = b''
    res += adds(x['word'])
    res += adds(x['pos'])
    res += len(x['sounds']).to_bytes(2)
    for s in x['sounds']:
        res += adds(s['ipa'])

    res += len(x['senses']).to_bytes(2)
    for s in x['senses']:
        res += len(s['glosses']).to_bytes(2)
        for g in s['glosses']:
            res += adds(g)

        res += len(s['links']).to_bytes(2)
        for l in s['links']:
            res += adds(l[0])

    return res
    

tims = []
tims.append(len(j).to_bytes(4))
o = 0
for x in j:
    print(f'At {o} out of {len(j)}            ', end='\r')
    o += 1
    tims.append(ts(x))

with open('rdict', 'wb') as f:
    f.write(b''.join(tims))
