require 'jackaudio'

chans = 2

o = Jackaudio::BlockingAudioIO.new("jackruby",chans,chans)

o.start
chans.times {|i| 
  o.connectToPhysical(i,i)
  o.connectFromPhysical(i,i)
}

#x = 0
#mul = Math::PI * 2.0 / o.getSampleRate
#loop {
#	val = Math.sin(x * 440.0 * mul) * (Math.sin(x * 0.5 * mul)).abs * 0.9
#  chans.times { |i|
#    o.write(i, val)
#  }
#  x = x + 1
#}

loop {
  2.times { |i|
    o.write(i, o.read(i))
  }
}
