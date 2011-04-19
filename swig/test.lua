require("jackaudio")

chans = 2
o = jackaudio.BlockingAudioIO("jacklua",chans,chans)

--start our jack client
o:start()

--connect our two outputs to the soundcard outputs
--connect our two inputs to the soundcard inputs
for i=0,chans-1 do
	o:connectToPhysical(i,i)
	o:connectFromPhysical(i,i)
end

--make a sine wave
x = 0
mul = math.pi * 2.0 / o:getSampleRate()
while true do
	val = math.sin(x * 440.0 * mul) * math.abs(math.sin(x * 0.5 * mul)) * 0.9
	for i=0,chans - 1 do
		o:write(i, val)
	end
	x = x + 1
end

--copy input to output
while true do
	for i=0,chans - 1 do
		o:write(i,o:read(i))
	end
end

