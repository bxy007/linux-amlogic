#!/usr/bin/env python3
"""Host UBSan tests for the production DV/HDR10 helpers (requires cc and numpy)."""
import ctypes as C
import hashlib
import json
from pathlib import Path
import random
import subprocess
import tempfile
import numpy as np

ROOT = Path(__file__).resolve().parents[4]
HEADER = ROOT / 'drivers/amlogic/media/enhancement/amdolby_vision/dv_hdr10_encoding.h'
SOURCE = HEADER.with_name('amdolby_vision.c')
U, I, B = C.c_uint32, C.c_int32, C.c_uint8
rng = random.Random(20261103)

def wrap(v):
    return (B*32)(*v)

def block(ver, xy, long=False):
    v = bytearray(32)
    size = 26 if ver == 0 else 15 if long else 12
    v[:6] = bytes([0xe0 | (size-1), 1, 0x46, 0xd0, 0, ver << 5])
    c = list(map(int, xy))
    if ver == 0:
        for k in range(3):
            x,y = c[2*k:2*k+2]; n=6+3*k
            v[n:n+3]=bytes([((x&15)<<4)|(y&15),x>>4,y>>4])
    elif ver == 1 and long:
        v[9:15] = bytes(c)
    elif ver == 1:
        rx,ry,gx,gy,bx,by=c
        v[8]=((bx&7)<<5)|((by&7)<<2)|1
        v[9]=(gx<<1)|(ry&1)
        v[10]=((gy&127)<<1)|((ry>>1)&1)
        v[11]=((rx&31)<<3)|((ry>>2)&7)
    else:
        rx,ry,gx,gy,bx,by=c
        v[8]=gx<<1;v[9]=(gy&127)<<1
        v[10]=((rx&31)<<3)|(bx&7);v[11]=((ry&31)<<3)|(by&7)
    return v,size

def reference(xy):
    p = np.array([[x,y,1-x-y] for x,y in np.array(xy).reshape(3,2)]).T
    w = np.array([.3127/.329,1,.3583/.329])
    return p @ np.diag(np.linalg.solve(p,w))

TARGET_INV = np.linalg.inv(reference([.708,.292,.170,.797,.131,.046]))

def packed(c,s):
    return [(c[i]&65535)|((c[i+1]&65535)<<16) for i in range(0,8,2)]+[(c[8]&65535)|(s<<16)]

with tempfile.TemporaryDirectory(prefix='dv-hdr10-test-') as tmp:
    code = r'''
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
typedef uint8_t u8; typedef uint16_t u16; typedef uint32_t u32;
typedef int16_t s16; typedef int32_t s32; typedef int64_t s64;
static s64 div64_s64(s64 n, s64 d) { return n / d; }
''' + HEADER.read_text() + r'''
static struct dv_hdr10_encoding_cache cache;
int decode(const u8 *v, u32 n, s32 *xy) {return dv_hdr10_primaries(v,n,xy);}
int matrix(const u8 *v, u32 n, s32 *m) {return dv_hdr10_matrix(v,n,m);}
int convert(const u32 *o, const s32 *m, u32 *d) {return dv_hdr10_convert_b2a(o,m,d);}
int update(const u8 *v,u32 n,u32 *dm,int enabled,int scope) {
 return dv_hdr10_update(&cache,dm,enabled,scope,v,n);
}
u32 generation(void) {return cache.generation;}
'''
    path=Path(tmp);(path/'test.c').write_text(code)
    subprocess.run(['cc','-std=c11','-shared','-fPIC','-O2','-Wall','-Wextra','-Werror','-fsanitize=undefined','-fno-sanitize-recover=all',str(path/'test.c'),'-o',str(path/'test.so')],check=True)
    lib=C.CDLL(str(path/'test.so'))
    lib.decode.argtypes=[C.POINTER(B),U,C.POINTER(I)]
    lib.matrix.argtypes=[C.POINTER(B),U,C.POINTER(I)]
    lib.convert.argtypes=[C.POINTER(U),C.POINTER(I),C.POINTER(U)]
    lib.update.argtypes=[C.POINTER(B),U,C.POINTER(U),C.c_int,C.c_int]
    count=valid=0;maximum=0.0
    for ver,long in [(0,False),(1,False),(1,True),(2,False)]:
        for _ in range(2500):
            if ver==0 or long:
                scale=4096 if ver==0 else 256
                # Plausible gamut range; some cases intentionally invalid.
                xy=np.rint(np.array([rng.uniform(.60,.73),rng.uniform(.26,.35),rng.uniform(.10,.32),rng.uniform(.56,.82),rng.uniform(.10,.18),rng.uniform(.025,.10)])*scale).astype(int).tolist()
            else:
                scale=256
                xy=[rng.randrange(160,184),rng.randrange(64,88),rng.randrange(24,84),rng.randrange(140,212),rng.randrange(32,40),rng.randrange(8,16)]
            v,n=block(ver,xy,long);decoded=(I*6)();m=(I*9)()
            ok=lib.decode(wrap(v),n,decoded)
            physical=all(x>0 and y>0 and x+y<=scale for x,y in zip(xy[::2],xy[1::2]))
            assert bool(ok)==physical
            if ok:assert list(decoded)==[c*(4096//scale) for c in xy]
            result=lib.matrix(wrap(v),n,m);count+=1
            if result:
                expected=TARGET_INV@reference(np.array(xy)/scale)
                error=float(np.max(np.abs(np.array(m).reshape(3,3)/2**20-expected)))
                maximum=max(maximum,error)
                assert error<8/2**20,(ver,xy,error)
                assert all(sum(list(m)[i:i+3])==2**20 for i in [0,3,6])
                valid+=1
    for _ in range(20000):
        v=bytes(rng.randrange(256) for _ in range(32));m=(I*9)(*([12345]*9))
        ok=lib.matrix(wrap(v),rng.randrange(40),m)
        if not ok:assert list(m)==[12345]*9
    # Declared/available lengths, bad OUI/version, zero and collinear primaries.
    good,n=block(2,[175,80,56,176,38,15])
    for v,size in [(good,i) for i in range(12)]+[(good,33)]:
        assert not lib.matrix(wrap(v),size,(I*9)())
    for offset,value in [(0,0),(1,0),(2,0),(5,0xe0)]:
        v=bytearray(good);v[offset]=value;assert not lib.matrix(wrap(v),n,(I*9)())
    for xy in [[0]*6,[1024,1024]*3,[3000,3000,1000,2000,200,300]]:
        v,size=block(0,xy);assert not lib.matrix(wrap(v),size,(I*9)())
    # Identity, rounding, overflow, unsupported scale; original inputs immutable.
    for _ in range(15000):
        c=[rng.randrange(-32768,32768) for _ in range(9)];s=rng.randrange(19)
        m=[rng.randrange(-9*2**20,9*2**20) for _ in range(9)]
        if _%4==0:m=[2**20 if i%4==0 else 0 for i in range(9)]
        orig=packed(c,s);dst=(U*5)();input_=(U*5)(*orig)
        ret=lib.convert(input_,(I*9)(*m),dst)
        expected=[]
        for row in range(3):
            for col in range(3):
                x=sum(m[row*3+k]*c[k*3+col] for k in range(3))
                expected.append((1 if x>=0 else -1)*((abs(x)+2**19)//2**20))
        ok=s<=15 and all(abs(x)<=8*2**20 for x in m) and all(-32768<=x<=32767 for x in expected)
        assert ret==ok and list(input_)==orig
        assert list(dst)==(packed(expected,s) if ok else orig)
    # Cache follows actual full VSVDB; non-color change never produces stale -1.
    dm_words=[0]*5+packed([8192 if i%4==0 else 0 for i in range(9)],13)+[42]*16
    def update(v=good,size=n,enabled=1,scope=1):
        dm=(U*26)(*dm_words);status=lib.update(wrap(v),size,dm,enabled,scope)
        assert list(dm)[:5]==dm_words[:5] and list(dm)[10:]==dm_words[10:]
        if status!=1:assert list(dm)==dm_words
        return status,list(dm)
    assert update()[0]==1;g=lib.generation();baseline=update()[1]
    for _ in range(100):assert update()[1]==baseline
    assert lib.generation()==g
    changed=bytearray(good);changed[7]=0x9f
    assert update(changed)[1]==baseline and lib.generation()==g+1
    changed[10]^=8
    assert update(changed)[0]==1 and update(changed)[1]!=baseline
    assert update(bytes(32))[0]==-1
    assert update()[0]==1
    for enabled,scope in [(0,0),(0,1),(1,0)]:assert update(enabled=enabled,scope=scope)[0]==0
    assert update()[1]==baseline
    source=SOURCE.read_text()
    assert 'dv_hdr10_encoding_enabled = true;' in source
    assert 'module_param_cb(dv_hdr10_encoding_config, &dv_hdr10_encoding_config_ops, NULL, 0400)' in source
    assert 'index >= 5 && index <= 9' in source
    result={'parsed_gamuts':count,'valid_gamuts':valid,'max_coefficient_error':maximum,'malformed_fuzz':20000,'arithmetic_cases':15000,'cache_transitions':'passed','ubsan':'passed','source_sha256':hashlib.sha256(SOURCE.read_bytes()).hexdigest(),'header_sha256':hashlib.sha256(HEADER.read_bytes()).hexdigest()}
    print(json.dumps(result,indent=2))
