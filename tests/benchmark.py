"""Host render CPU comparison (not a CH32 hardware timing measurement).
Usage: python tests/benchmark.py PATH_TO_CHGAME_REPO [--sketch OLD_SKETCH_FOLDER]
"""
from pathlib import Path
import os,sys,argparse,tempfile,subprocess
ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('repo',type=Path);ap.add_argument('--sketch',type=Path);args=ap.parse_args()
root=Path(__file__).resolve().parents[1];repo=args.repo.resolve();sk=(args.sketch or root/'CurseOfAstarok').resolve()
sys.path.insert(0,str(repo/'tools/chsim'));import chsim
os.environ['CHSIM_CXX']='g++';os.environ['CHSIM_FLAGS']='-O2 -fpermissive -include chrono'
gfx=chsim.chgfx_dir();game=chsim.chgame_dir();host=repo/'tools/chsim/host'
with tempfile.TemporaryDirectory() as td:
 td=Path(td);unit=td/'game.cpp';exe=td/'benchmark'
 ino=[sk/'CurseOfAstarok.ino']+sorted(p for p in sk.glob('*.ino') if p.name!='CurseOfAstarok.ino')
 unit.write_text('#include <Arduino.h>\n'+'\n'.join(p.read_text() for p in ino))
 src=[unit,root/'tests/benchmark.cpp']+sorted((sk/'src').rglob('*.cpp'))+sorted(p for p in gfx.glob('*.cpp') if p.name!='CHGfx.cpp')+sorted(game.rglob('*.cpp'))+[host/'chgfx_host.cpp']
 sd=repo/'platform/board/arduino/CHGame/libraries/CHSd'
 src+=sorted((sd/'src').glob('*.cpp'))+[sd/'host/sd_host.cpp']
 chsim.compile_exe(src,exe,includes=[host,sk,gfx,game,sd/'src',sd/'host']);subprocess.run([str(exe)],check=True)
