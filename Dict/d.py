#!/bin/python3
import json
from random import randint
from os import system

def SEARCH():
    with open('d.json', 'r') as f:
        print('Starting!')
        s = f.read()
        print('Finished reading!')
        j = json.loads(s)
        print('Finished loading json!')
    print(f'Got {len(j)} entries')

    def search(ss):
        print(f'Searching for {ss}')
        for x in j:
            if x['word'] == ss:
                print(x)
                return
        print(f'No matches found!')
    while True:
        # o = randint(0, len(j))
        # system(f"qutebrowser https://en.wiktionary.org/wiki/{j[o]['word']}")
        # input()
        search(input())
# template = {'pos': '', 'word': '', 'sounds': [], 'senses': []}
# sounds = {'ipa: ''}
# stempl = {'links': [], 'glosses': []}
# links = ["einem", "einem#German"], ["'n", "'n#German"]
# glosses = ['', '']

def YES():
    with open('d.json', 'r') as f:
        print('Starting!')
        s = f.read()
        print('Finished reading!')
        j = json.loads(s)
        print('Finished loading json!')
    print(f'Got {len(j)} entries')
    global res
    res = b''
    def adds(s):
        global res
        sb = s.encode('utf-8')
        lsb = len(sb)
        res += lsb.to_bytes(2)
        res += sb

    def ts(x):
        global res
        # print(x)
        adds(x['word'])
        adds(x['pos'])
        res += len(x['sounds']).to_bytes(2)
        for s in x['sounds']:
            adds(s['ipa'])

        res += len(x['senses']).to_bytes(2)
        for s in x['senses']:
            res += len(s['glosses']).to_bytes(2)
            for g in s['glosses']:
                adds(g)

            res += len(s['links']).to_bytes(2)
            for l in s['links']:
                adds(l[0])

        return res
        

    res += len(j).to_bytes(4)
    o = 0
    with open('rdict', 'wb') as f:
        for x in j:
            print(f'At {o} out of {len(j)}            ', end='\r')
            o += 1
            ts(x)
            if len(res) > 1000:
                f.write(res)
                res = b''


        f.write(res)

def FIX():
    with open('deutsch.json', 'r') as f:
        print('Starting!')
        s = f.read()
    with open('deutsch_fixed.json', 'w') as f:
        print('[', file=f, end='')
        can = False
        for line in s.splitlines():
            if can == False:
                can = True
            else:
                print(',', file=f, end='')
            print(line, file=f, end='')
        print(']', file=f, end='')


def NO():
    with open('deutsch_fixed.json', 'r') as f:
        print('Starting!')
        s = f.read()
        print('Finished reading!')
        j = json.loads(s)
        print('Finished loading json!')


    nwords = []

    lj = len(j)
    for o in range(lj):
        print(f'At {o} out of {lj}            ', end='\r')
        template = {'pos': '', 'word': '', 'sounds': [], 'senses': []}
        cw = j[o]
        # print(f'Parsing {cw}!')
        try:
            template['pos'] = cw['pos']
        except:
            pass

        try:
            template['word'] = cw['word']
        except:
            pass

        try:
            for s in cw['sounds']:
                if 'ipa' in s:
                    template['sounds'].append(s)
        except:
            pass

        template['senses'] = []

        for s in cw['senses']:
            stempl = {'links': [], 'glosses': [], 'synonyms': []}
            try:
                stempl['links'] = s['links']
            except:
                pass

            try:
                stempl['glosses'] = s['raw_glosses']
            except:
                try:
                    stempl['glosses'] = s['glosses']
                except:
                    pass

            template['senses'].append(stempl)

        # print(f'Got {template}!')
        nwords.append(template)

# print(nwords)
    with open('d.json', 'w') as f:
        f.write(json.dumps(nwords))

FIX()
NO()
YES()
