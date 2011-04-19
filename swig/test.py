import jackaudio
import math

chans = 2

o = jackaudio.BlockingAudioIO("jackpython",chans,chans)

o.start()

for i in range(2):
  o.connectToPhysical(i,i)
  o.connectFromPhysical(i,i)

x = 0
mul = math.pi * 2.0 / o.getSampleRate()

while(True):
  val = math.sin(x * 440.0 * mul) * math.fabs(math.sin(x * 0.5 * mul)) * 0.9
  for i in range(2):
    o.write(i, val)
  x = x + 1

while(True):
  for i in range(2):
    o.write(i, o.read(i))

